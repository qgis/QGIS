# -*- coding: utf-8 -*-

"""
***************************************************************************
    Grass7Algorithm.py
    ---------------------
    Date                 : February 2015
    Copyright            : (C) 2014-2015 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'February 2015'
__copyright__ = '(C) 2012-2015, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import sys
import os
import re
import uuid
import importlib

from qgis.PyQt.QtCore import QCoreApplication, QUrl

from qgis.core import (Qgis,
                       QgsRasterLayer,
                       QgsApplication,
                       QgsMapLayer,
                       QgsProcessingUtils,
                       QgsProcessing,
                       QgsMessageLog,
                       QgsVectorFileWriter,
                       QgsProcessingAlgorithm,
                       QgsProcessingParameterDefinition,
                       QgsProcessingException,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterString,
                       QgsProcessingParameterField,
                       QgsProcessingParameterPoint,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterVectorLayer,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterMultipleLayers,
                       QgsProcessingParameterVectorDestination,
                       QgsProcessingParameterRasterDestination,
                       QgsProcessingParameterFileDestination,
                       QgsProcessingParameterFile,
                       QgsProcessingParameterFolderDestination,
                       QgsProcessingOutputHtml,
                       QgsVectorLayer,
                       QgsProviderRegistry)
from qgis.utils import iface
from osgeo import ogr

from processing.core.ProcessingConfig import ProcessingConfig

from processing.core.parameters import getParameterFromString

from .Grass7Utils import Grass7Utils

from processing.tools.system import isWindows, getTempFilename

pluginPath = os.path.normpath(os.path.join(
    os.path.split(os.path.dirname(__file__))[0], os.pardir))


class Grass7Algorithm(QgsProcessingAlgorithm):

    GRASS_OUTPUT_TYPE_PARAMETER = 'GRASS_OUTPUT_TYPE_PARAMETER'
    GRASS_MIN_AREA_PARAMETER = 'GRASS_MIN_AREA_PARAMETER'
    GRASS_SNAP_TOLERANCE_PARAMETER = 'GRASS_SNAP_TOLERANCE_PARAMETER'
    GRASS_REGION_EXTENT_PARAMETER = 'GRASS_REGION_PARAMETER'
    GRASS_REGION_CELLSIZE_PARAMETER = 'GRASS_REGION_CELLSIZE_PARAMETER'
    GRASS_REGION_ALIGN_TO_RESOLUTION = 'GRASS_REGION_ALIGN_TO_RESOLUTION'
    GRASS_RASTER_FORMAT_OPT = 'GRASS_RASTER_FORMAT_OPT'
    GRASS_RASTER_FORMAT_META = 'GRASS_RASTER_FORMAT_META'
    GRASS_VECTOR_DSCO = 'GRASS_VECTOR_DSCO'
    GRASS_VECTOR_LCO = 'GRASS_VECTOR_LCO'
    GRASS_VECTOR_EXPORT_NOCAT = 'GRASS_VECTOR_EXPORT_NOCAT'

    OUTPUT_TYPES = ['auto', 'point', 'line', 'area']
    QGIS_OUTPUT_TYPES = {QgsProcessing.TypeVectorAnyGeometry: 'auto',
                         QgsProcessing.TypeVectorPoint: 'point',
                         QgsProcessing.TypeVectorLine: 'line',
                         QgsProcessing.TypeVectorPolygon: 'area'}

    def __init__(self, descriptionfile):
        super().__init__()
        self._name = ''
        self._display_name = ''
        self._short_description = ''
        self._group = ''
        self._groupId = ''
        self.groupIdRegex = re.compile(r'^[^\s\(]+')
        self.grass7Name = ''
        self.params = []
        self.hardcodedStrings = []
        self.inputLayers = []
        self.commands = []
        self.outputCommands = []
        self.exportedLayers = {}
        self.descriptionFile = descriptionfile

        # Default GRASS parameters
        self.region = None
        self.cellSize = None
        self.snapTolerance = None
        self.outputType = None
        self.minArea = None
        self.alignToResolution = None

        # Load parameters from a description file
        self.defineCharacteristicsFromFile()
        self.numExportedLayers = 0
        # Do we need this anymore?
        self.uniqueSuffix = str(uuid.uuid4()).replace('-', '')

        # Use the ext mechanism
        name = self.name().replace('.', '_')
        try:
            self.module = importlib.import_module(
                'processing.algs.grass7.ext.{}'.format(name))
        except ImportError:
            self.module = None

    def createInstance(self):
        return self.__class__(self.descriptionFile)

    def name(self):
        return self._name

    def displayName(self):
        return self._display_name

    def shortDescription(self):
        return self._short_description

    def group(self):
        return self._group

    def groupId(self):
        return self._groupId

    def icon(self):
        return QgsApplication.getThemeIcon("/providerGrass.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerGrass.svg")

    def flags(self):
        # TODO - maybe it's safe to background thread this?
        return super().flags() | QgsProcessingAlgorithm.FlagNoThreading | QgsProcessingAlgorithm.FlagDisplayNameIsLiteral

    def tr(self, string, context=''):
        if context == '':
            context = self.__class__.__name__
        return QCoreApplication.translate(context, string)

    def helpUrl(self):
        helpPath = Grass7Utils.grassHelpPath()
        if helpPath == '':
            return None

        if os.path.exists(helpPath):
            return QUrl.fromLocalFile(os.path.join(helpPath, '{}.html'.format(self.grass7Name))).toString()
        else:
            return helpPath + '{}.html'.format(self.grass7Name)

    def initAlgorithm(self, config=None):
        """
        Algorithm initialization
        """
        for p in self.params:
            # We use createOutput argument for automatic output creation
            self.addParameter(p, True)

    def defineCharacteristicsFromFile(self):
        """
        Create algorithm parameters and outputs from a text file.
        """
        with open(self.descriptionFile) as lines:
            # First line of the file is the Grass algorithm name
            line = lines.readline().strip('\n').strip()
            self.grass7Name = line
            # Second line if the algorithm name in Processing
            line = lines.readline().strip('\n').strip()
            self._short_description = line
            if " - " not in line:
                self._name = self.grass7Name
            else:
                self._name = line[:line.find(' ')].lower()
            self._short_description = QCoreApplication.translate("GrassAlgorithm", line)
            self._display_name = self._name
            # Read the grass group
            line = lines.readline().strip('\n').strip()
            self._group = QCoreApplication.translate("GrassAlgorithm", line)
            self._groupId = self.groupIdRegex.search(line).group(0).lower()
            hasRasterOutput = False
            hasRasterInput = False
            hasVectorInput = False
            vectorOutputs = False
            # Then you have parameters/output definition
            line = lines.readline().strip('\n').strip()
            while line != '':
                try:
                    line = line.strip('\n').strip()
                    if line.startswith('Hardcoded'):
                        self.hardcodedStrings.append(line[len('Hardcoded|'):])
                    parameter = getParameterFromString(line, "GrassAlgorithm")
                    if parameter is not None:
                        self.params.append(parameter)
                        if isinstance(parameter, (QgsProcessingParameterVectorLayer, QgsProcessingParameterFeatureSource)):
                            hasVectorInput = True
                        elif isinstance(parameter, QgsProcessingParameterRasterLayer):
                            hasRasterInput = True
                        elif isinstance(parameter, QgsProcessingParameterMultipleLayers):
                            if parameter.layerType() < 3 or parameter.layerType() == 5:
                                hasVectorInput = True
                            elif parameter.layerType() == 3:
                                hasRasterInput = True
                        elif isinstance(parameter, QgsProcessingParameterVectorDestination):
                            vectorOutputs = True
                        elif isinstance(parameter, QgsProcessingParameterRasterDestination):
                            hasRasterOutput = True
                    line = lines.readline().strip('\n').strip()
                except Exception as e:
                    QgsMessageLog.logMessage(self.tr('Could not open GRASS GIS 7 algorithm: {0}\n{1}').format(self.descriptionFile, line), self.tr('Processing'), Qgis.Critical)
                    raise e

        param = QgsProcessingParameterExtent(
            self.GRASS_REGION_EXTENT_PARAMETER,
            self.tr('GRASS GIS 7 region extent'),
            optional=True
        )
        param.setFlags(param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.params.append(param)

        if hasRasterOutput or hasRasterInput:
            # Add a cellsize parameter
            param = QgsProcessingParameterNumber(
                self.GRASS_REGION_CELLSIZE_PARAMETER,
                self.tr('GRASS GIS 7 region cellsize (leave 0 for default)'),
                type=QgsProcessingParameterNumber.Double,
                minValue=0.0, maxValue=sys.float_info.max + 1, defaultValue=0.0
            )
            param.setFlags(param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
            self.params.append(param)

        if hasRasterOutput:
            # Add a createopt parameter for format export
            param = QgsProcessingParameterString(
                self.GRASS_RASTER_FORMAT_OPT,
                self.tr('Output Rasters format options (createopt)'),
                multiLine=True, optional=True
            )
            param.setFlags(param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
            self.params.append(param)

            # Add a metadata parameter for format export
            param = QgsProcessingParameterString(
                self.GRASS_RASTER_FORMAT_META,
                self.tr('Output Rasters format metadata options (metaopt)'),
                multiLine=True, optional=True
            )
            param.setFlags(param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
            self.params.append(param)

        if hasVectorInput:
            param = QgsProcessingParameterNumber(self.GRASS_SNAP_TOLERANCE_PARAMETER,
                                                 self.tr('v.in.ogr snap tolerance (-1 = no snap)'),
                                                 type=QgsProcessingParameterNumber.Double,
                                                 minValue=-1.0, maxValue=sys.float_info.max + 1,
                                                 defaultValue=-1.0)
            param.setFlags(param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
            self.params.append(param)
            param = QgsProcessingParameterNumber(self.GRASS_MIN_AREA_PARAMETER,
                                                 self.tr('v.in.ogr min area'),
                                                 type=QgsProcessingParameterNumber.Double,
                                                 minValue=0.0, maxValue=sys.float_info.max + 1,
                                                 defaultValue=0.0001)
            param.setFlags(param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
            self.params.append(param)

        if vectorOutputs:
            # Add an optional output type
            param = QgsProcessingParameterEnum(self.GRASS_OUTPUT_TYPE_PARAMETER,
                                               self.tr('v.out.ogr output type'),
                                               self.OUTPUT_TYPES)
            param.setFlags(param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
            self.params.append(param)

            # Add a DSCO parameter for format export
            param = QgsProcessingParameterString(
                self.GRASS_VECTOR_DSCO,
                self.tr('v.out.ogr output data source options (dsco)'),
                multiLine=True, optional=True
            )
            param.setFlags(param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
            self.params.append(param)

            # Add a LCO parameter for format export
            param = QgsProcessingParameterString(
                self.GRASS_VECTOR_LCO,
                self.tr('v.out.ogr output layer options (lco)'),
                multiLine=True, optional=True
            )
            param.setFlags(param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
            self.params.append(param)

            # Add a -c flag for export
            param = QgsProcessingParameterBoolean(
                self.GRASS_VECTOR_EXPORT_NOCAT,
                self.tr('Also export features without category (not labeled). Otherwise only features with category are exported'),
                False
            )
            param.setFlags(param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
            self.params.append(param)

    def getDefaultCellSize(self):
        """
        Determine a default cell size from all the raster layers.
        """
        cellsize = 0.0
        layers = [l for l in self.inputLayers if isinstance(l, QgsRasterLayer)]

        for layer in layers:
            cellsize = max(layer.rasterUnitsPerPixelX(), cellsize)

        if cellsize == 0.0:
            cellsize = 100.0

        return cellsize

    def grabDefaultGrassParameters(self, parameters, context):
        """
        Imports default GRASS parameters (EXTENT, etc) into
        object attributes for faster retrieving.
        """
        # GRASS region extent
        self.region = self.parameterAsExtent(parameters,
                                             self.GRASS_REGION_EXTENT_PARAMETER,
                                             context)
        # GRASS cell size
        if self.parameterDefinition(self.GRASS_REGION_CELLSIZE_PARAMETER):
            self.cellSize = self.parameterAsDouble(parameters,
                                                   self.GRASS_REGION_CELLSIZE_PARAMETER,
                                                   context)
        # GRASS snap tolerance
        self.snapTolerance = self.parameterAsDouble(parameters,
                                                    self.GRASS_SNAP_TOLERANCE_PARAMETER,
                                                    context)
        # GRASS min area
        self.minArea = self.parameterAsDouble(parameters,
                                              self.GRASS_MIN_AREA_PARAMETER,
                                              context)
        # GRASS output type
        self.outputType = self.parameterAsString(parameters,
                                                 self.GRASS_OUTPUT_TYPE_PARAMETER,
                                                 context)
        # GRASS align to resolution
        self.alignToResolution = self.parameterAsBool(parameters,
                                                      self.GRASS_REGION_ALIGN_TO_RESOLUTION,
                                                      context)

    def processAlgorithm(self, original_parameters, context, feedback):
        if isWindows():
            path = Grass7Utils.grassPath()
            if path == '':
                raise QgsProcessingException(
                    self.tr('GRASS GIS 7 folder is not configured. Please '
                            'configure it before running GRASS GIS 7 algorithms.'))

        # make a copy of the original parameters dictionary - it gets modified by grass algorithms
        parameters = {k: v for k, v in original_parameters.items()}

        # Create brand new commands lists
        self.commands = []
        self.outputCommands = []
        self.exportedLayers = {}

        # If GRASS session has been created outside of this algorithm then
        # get the list of layers loaded in GRASS otherwise start a new
        # session
        existingSession = Grass7Utils.sessionRunning
        if existingSession:
            self.exportedLayers = Grass7Utils.getSessionLayers()
        else:
            Grass7Utils.startGrassSession()

        # Handle default GRASS parameters
        self.grabDefaultGrassParameters(parameters, context)

        # Handle ext functions for inputs/command/outputs
        for fName in ['Inputs', 'Command', 'Outputs']:
            fullName = 'process{}'.format(fName)
            if self.module and hasattr(self.module, fullName):
                getattr(self.module, fullName)(self, parameters, context, feedback)
            else:
                getattr(self, fullName)(parameters, context, feedback)

        # Run GRASS
        loglines = []
        loglines.append(self.tr('GRASS GIS 7 execution commands'))
        for line in self.commands:
            feedback.pushCommandInfo(line)
            loglines.append(line)
        if ProcessingConfig.getSetting(Grass7Utils.GRASS_LOG_COMMANDS):
            QgsMessageLog.logMessage("\n".join(loglines), self.tr('Processing'), Qgis.Info)

        Grass7Utils.executeGrass(self.commands, feedback, self.outputCommands)

        # If the session has been created outside of this algorithm, add
        # the new GRASS GIS 7 layers to it otherwise finish the session
        if existingSession:
            Grass7Utils.addSessionLayers(self.exportedLayers)
        else:
            Grass7Utils.endGrassSession()

        # Return outputs map
        outputs = {}
        for out in self.outputDefinitions():
            outName = out.name()
            if outName in parameters:
                outputs[outName] = parameters[outName]
                if isinstance(out, QgsProcessingOutputHtml):
                    self.convertToHtml(parameters[outName])

        return outputs

    def processInputs(self, parameters, context, feedback):
        """Prepare the GRASS import commands"""
        inputs = [p for p in self.parameterDefinitions()
                  if isinstance(p, (QgsProcessingParameterVectorLayer,
                                    QgsProcessingParameterFeatureSource,
                                    QgsProcessingParameterRasterLayer,
                                    QgsProcessingParameterMultipleLayers))]
        for param in inputs:
            paramName = param.name()
            if paramName not in parameters:
                continue
            # Handle Null parameter
            if parameters[paramName] is None:
                continue
            elif isinstance(parameters[paramName], str) and len(parameters[paramName]) == 0:
                continue

            # Raster inputs needs to be imported into temp GRASS DB
            if isinstance(param, QgsProcessingParameterRasterLayer):
                if paramName not in self.exportedLayers:
                    self.loadRasterLayerFromParameter(
                        paramName, parameters, context)
            # Vector inputs needs to be imported into temp GRASS DB
            elif isinstance(param, (QgsProcessingParameterFeatureSource, QgsProcessingParameterVectorLayer)):
                if paramName not in self.exportedLayers:
                    # Attribute tables are also vector inputs
                    if QgsProcessing.TypeFile in param.dataTypes():
                        self.loadAttributeTableFromParameter(
                            paramName, parameters, context)
                    else:
                        self.loadVectorLayerFromParameter(
                            paramName, parameters, context, external=None, feedback=feedback)
            # For multiple inputs, process each layer
            elif isinstance(param, QgsProcessingParameterMultipleLayers):
                layers = self.parameterAsLayerList(parameters, paramName, context)
                for idx, layer in enumerate(layers):
                    layerName = '{}_{}'.format(paramName, idx)
                    # Add a raster layer
                    if layer.type() == QgsMapLayerType.RasterLayer:
                        self.loadRasterLayer(layerName, layer)
                    # Add a vector layer
                    elif layer.type() == QgsMapLayerType.VectorLayer:
                        self.loadVectorLayer(layerName, layer, external=None, feedback=feedback)

        self.postInputs()

    def postInputs(self):
        """
        After layer imports, we need to update some internal parameters
        """
        # If projection has not already be set, use the project
        self.setSessionProjectionFromProject()

        # Build GRASS region
        if self.region.isEmpty():
            self.region = QgsProcessingUtils.combineLayerExtents(self.inputLayers)
        command = 'g.region n={} s={} e={} w={}'.format(
            self.region.yMaximum(), self.region.yMinimum(),
            self.region.xMaximum(), self.region.xMinimum()
        )
        # Handle cell size
        if self.parameterDefinition(self.GRASS_REGION_CELLSIZE_PARAMETER):
            if self.cellSize:
                cellSize = self.cellSize
            else:
                cellSize = self.getDefaultCellSize()
            command += ' res={}'.format(cellSize)

        # Handle align to resolution
        if self.alignToResolution:
            command += ' -a'

        # Add the default parameters commands
        self.commands.append(command)

        QgsMessageLog.logMessage(self.tr('processInputs end. Commands: {}').format(self.commands), 'Grass7', Qgis.Info)

    def processCommand(self, parameters, context, feedback, delOutputs=False):
        """
        Prepare the GRASS algorithm command
        :param parameters:
        :param context:
        :param delOutputs: do not add outputs to commands.
        """
        noOutputs = [o for o in self.parameterDefinitions() if o not in self.destinationParameterDefinitions()]
        command = '{} '.format(self.grass7Name)
        command += '{}'.join(self.hardcodedStrings)

        # Add algorithm command
        for param in noOutputs:
            paramName = param.name()
            value = None

            # Exclude default GRASS parameters
            if paramName in [self.GRASS_REGION_CELLSIZE_PARAMETER,
                             self.GRASS_REGION_EXTENT_PARAMETER,
                             self.GRASS_MIN_AREA_PARAMETER,
                             self.GRASS_SNAP_TOLERANCE_PARAMETER,
                             self.GRASS_OUTPUT_TYPE_PARAMETER,
                             self.GRASS_REGION_ALIGN_TO_RESOLUTION,
                             self.GRASS_RASTER_FORMAT_OPT,
                             self.GRASS_RASTER_FORMAT_META,
                             self.GRASS_VECTOR_DSCO,
                             self.GRASS_VECTOR_LCO,
                             self.GRASS_VECTOR_EXPORT_NOCAT]:
                continue

            # Raster and vector layers
            if isinstance(param, (QgsProcessingParameterRasterLayer,
                                  QgsProcessingParameterVectorLayer,
                                  QgsProcessingParameterFeatureSource)):
                if paramName in self.exportedLayers:
                    value = self.exportedLayers[paramName]
                else:
                    value = self.parameterAsCompatibleSourceLayerPath(
                        parameters, paramName, context,
                        QgsVectorFileWriter.supportedFormatExtensions()
                    )
            # MultipleLayers
            elif isinstance(param, QgsProcessingParameterMultipleLayers):
                layers = self.parameterAsLayerList(parameters, paramName, context)
                values = []
                for idx in range(len(layers)):
                    layerName = '{}_{}'.format(paramName, idx)
                    values.append(self.exportedLayers[layerName])
                value = ','.join(values)
            # For booleans, we just add the parameter name
            elif isinstance(param, QgsProcessingParameterBoolean):
                if self.parameterAsBool(parameters, paramName, context):
                    command += ' {}'.format(paramName)
            # For Extents, remove if the value is null
            elif isinstance(param, QgsProcessingParameterExtent):
                if self.parameterAsExtent(parameters, paramName, context):
                    value = self.parameterAsString(parameters, paramName, context)
            # For enumeration, we need to grab the string value
            elif isinstance(param, QgsProcessingParameterEnum):
                # Handle multiple values
                if param.allowMultiple():
                    indexes = self.parameterAsEnums(parameters, paramName, context)
                else:
                    indexes = [self.parameterAsEnum(parameters, paramName, context)]
                if indexes:
                    value = '"{}"'.format(','.join([param.options()[i] for i in indexes]))
            # For strings, we just translate as string
            elif isinstance(param, QgsProcessingParameterString):
                data = self.parameterAsString(parameters, paramName, context)
                # if string is empty, we don't add it
                if len(data) > 0:
                    value = '"{}"'.format(
                        self.parameterAsString(parameters, paramName, context)
                    )
            # For fields, we just translate as string
            elif isinstance(param, QgsProcessingParameterField):
                value = ','.join(
                    self.parameterAsFields(parameters, paramName, context)
                )
            elif isinstance(param, QgsProcessingParameterFile):
                if self.parameterAsString(parameters, paramName, context):
                    value = '"{}"'.format(
                        self.parameterAsString(parameters, paramName, context)
                    )
            elif isinstance(param, QgsProcessingParameterPoint):
                if self.parameterAsString(parameters, paramName, context):
                    # parameter specified, evaluate as point
                    # TODO - handle CRS transform
                    point = self.parameterAsPoint(parameters, paramName, context)
                    value = '{},{}'.format(point.x(), point.y())
            # For numbers, we translate as a string
            elif isinstance(param, (QgsProcessingParameterNumber,
                                    QgsProcessingParameterPoint)):
                value = self.parameterAsString(parameters, paramName, context)
            # For everything else, we assume that it is a string
            else:
                value = '"{}"'.format(
                    self.parameterAsString(parameters, paramName, context)
                )
            if value:
                command += ' {}={}'.format(paramName.replace('~', ''), value)

        # Handle outputs
        if not delOutputs:
            for out in self.destinationParameterDefinitions():
                # We exclude hidden parameters
                if out.flags() & QgsProcessingParameterDefinition.FlagHidden:
                    continue
                outName = out.name()
                # For File destination
                if isinstance(out, QgsProcessingParameterFileDestination):
                    if outName in parameters and parameters[outName] is not None:
                        # for HTML reports, we need to redirect stdout
                        if out.defaultFileExtension().lower() == 'html':
                            command += ' {}=- > "{}"'.format(
                                outName,
                                self.parameterAsFileOutput(parameters, outName, context))
                        else:
                            command += ' {}="{}"'.format(
                                outName,
                                self.parameterAsFileOutput(parameters, outName, context))
                # For folders destination
                elif isinstance(out, QgsProcessingParameterFolderDestination):
                    # We need to add a unique temporary basename
                    uniqueBasename = outName + self.uniqueSuffix
                    command += ' {}={}'.format(outName, uniqueBasename)
                else:
                    if outName in parameters and parameters[outName] is not None:
                        # We add an output name to make sure it is unique if the session
                        # uses this algorithm several times.
                        uniqueOutputName = outName + self.uniqueSuffix
                        command += ' {}={}'.format(outName, uniqueOutputName)

                        # Add output file to exported layers, to indicate that
                        # they are present in GRASS
                        self.exportedLayers[outName] = uniqueOutputName

        command += ' --overwrite'
        self.commands.append(command)
        QgsMessageLog.logMessage(self.tr('processCommands end. Commands: {}').format(self.commands), 'Grass7', Qgis.Info)

    def vectorOutputType(self, parameters, context):
        """Determine vector output types for outputs"""
        self.outType = 'auto'
        if self.parameterDefinition(self.GRASS_OUTPUT_TYPE_PARAMETER):
            typeidx = self.parameterAsEnum(parameters,
                                           self.GRASS_OUTPUT_TYPE_PARAMETER,
                                           context)
            self.outType = ('auto' if typeidx
                            is None else self.OUTPUT_TYPES[typeidx])

    def processOutputs(self, parameters, context, feedback):
        """Prepare the GRASS v.out.ogr commands"""
        # Determine general vector output type
        self.vectorOutputType(parameters, context)

        for out in self.destinationParameterDefinitions():
            outName = out.name()
            if outName not in parameters:
                # skipped output
                continue

            if isinstance(out, QgsProcessingParameterRasterDestination):
                self.exportRasterLayerFromParameter(outName, parameters, context)
            elif isinstance(out, QgsProcessingParameterVectorDestination):
                self.exportVectorLayerFromParameter(outName, parameters, context)
            elif isinstance(out, QgsProcessingParameterFolderDestination):
                self.exportRasterLayersIntoDirectory(outName, parameters, context)

    def loadRasterLayerFromParameter(self, name, parameters, context, external=True, band=1):
        """
        Creates a dedicated command to load a raster into
        the temporary GRASS DB.
        :param name: name of the parameter.
        :param parameters: algorithm parameters dict.
        :param context: algorithm context.
        :param external: True if using r.external.
        :param band: imports only specified band. None for all bands.
        """
        layer = self.parameterAsRasterLayer(parameters, name, context)
        self.loadRasterLayer(name, layer, external, band)

    def loadRasterLayer(self, name, layer, external=True, band=1, destName=None):
        """
        Creates a dedicated command to load a raster into
        the temporary GRASS DB.
        :param name: name of the parameter.
        :param layer: QgsMapLayer for the raster layer.
        :param external: True if using r.external.
        :param band: imports only specified band. None for all bands.
        :param destName: force the destination name of the raster.
        """
        self.inputLayers.append(layer)
        self.setSessionProjectionFromLayer(layer)
        if not destName:
            destName = 'rast_{}'.format(os.path.basename(getTempFilename()))
        self.exportedLayers[name] = destName
        command = '{0} input="{1}" {2}output="{3}" --overwrite -o'.format(
            'r.external' if external else 'r.in.gdal',
            os.path.normpath(layer.source()),
            'band={} '.format(band) if band else '',
            destName)
        self.commands.append(command)

    def exportRasterLayerFromParameter(self, name, parameters, context, colorTable=True):
        """
        Creates a dedicated command to export a raster from
        temporary GRASS DB into a file via gdal.
        :param name: name of the parameter.
        :param parameters: Algorithm parameters dict.
        :param context: Algorithm context.
        :param colorTable: preserve color Table.
        """
        fileName = self.parameterAsOutputLayer(parameters, name, context)
        if not fileName:
            return

        fileName = os.path.normpath(fileName)
        grassName = '{}{}'.format(name, self.uniqueSuffix)
        outFormat = Grass7Utils.getRasterFormatFromFilename(fileName)
        createOpt = self.parameterAsString(parameters, self.GRASS_RASTER_FORMAT_OPT, context)
        metaOpt = self.parameterAsString(parameters, self.GRASS_RASTER_FORMAT_META, context)
        self.exportRasterLayer(grassName, fileName, colorTable, outFormat, createOpt, metaOpt)

    def exportRasterLayer(self, grassName, fileName,
                          colorTable=True, outFormat='GTiff',
                          createOpt=None,
                          metaOpt=None):
        """
        Creates a dedicated command to export a raster from
        temporary GRASS DB into a file via gdal.
        :param grassName: name of the raster to export.
        :param fileName: file path of raster layer.
        :param colorTable: preserve color Table.
        :param outFormat: file format for export.
        :param createOpt: creation options for format.
        :param metaOpt: metadata options for export.
        """
        if not createOpt:
            if outFormat in Grass7Utils.GRASS_RASTER_FORMATS_CREATEOPTS:
                createOpt = Grass7Utils.GRASS_RASTER_FORMATS_CREATEOPTS[outFormat]

        for cmd in [self.commands, self.outputCommands]:
            # Adjust region to layer before exporting
            cmd.append('g.region raster={}'.format(grassName))
            cmd.append(
                'r.out.gdal -t -m{0} input="{1}" output="{2}" format="{3}" {4}{5} --overwrite'.format(
                    '' if colorTable else ' -c',
                    grassName, fileName,
                    outFormat,
                    ' createopt="{}"'.format(createOpt) if createOpt else '',
                    ' metaopt="{}"'.format(metaOpt) if metaOpt else ''
                )
            )

    def exportRasterLayersIntoDirectory(self, name, parameters, context, colorTable=True, wholeDB=False):
        """
        Creates a dedicated loop command to export rasters from
        temporary GRASS DB into a directory via gdal.
        :param name: name of the output directory parameter.
        :param parameters: Algorithm parameters dict.
        :param context: Algorithm context.
        :param colorTable: preserve color Table.
        :param wholeDB: export every raster layer from the GRASSDB
        """
        # Grab directory name and temporary basename
        outDir = os.path.normpath(
            self.parameterAsString(parameters, name, context))
        basename = ''
        if not wholeDB:
            basename = name + self.uniqueSuffix

        # Add a loop export from the basename
        for cmd in [self.commands, self.outputCommands]:
            # TODO Format/options support
            if isWindows():
                cmd.append("if not exist {0} mkdir {0}".format(outDir))
                cmd.append("for /F %%r IN ('g.list type^=rast pattern^=\"{0}*\"') do r.out.gdal -m{1} input=%%r output={2}/%%r.tif {3}".format(
                    basename,
                    ' -t' if colorTable else '',
                    outDir,
                    '--overwrite -c createopt="TFW=YES,COMPRESS=LZW"'
                ))
            else:
                cmd.append("for r in $(g.list type=rast pattern='{}*'); do".format(basename))
                cmd.append("  r.out.gdal -m{0} input=${{r}} output={1}/${{r}}.tif {2}".format(
                    ' -t' if colorTable else '', outDir,
                    '--overwrite -c createopt="TFW=YES,COMPRESS=LZW"'
                ))
                cmd.append("done")

    def loadVectorLayerFromParameter(self, name, parameters, context, feedback, external=False):
        """
        Creates a dedicated command to load a vector into
        the temporary GRASS DB.
        :param name: name of the parameter
        :param parameters: Parameters of the algorithm.
        :param context: Processing context
        :param external: use v.external (v.in.ogr if False).
        """
        layer = self.parameterAsVectorLayer(parameters, name, context)

        is_ogr_disk_based_layer = layer is not None and layer.dataProvider().name() == 'ogr'
        if is_ogr_disk_based_layer:
            # we only support direct reading of disk based ogr layers -- not ogr postgres layers, etc
            source_parts = QgsProviderRegistry.instance().decodeUri('ogr', layer.source())
            if not source_parts.get('path'):
                is_ogr_disk_based_layer = False
            elif source_parts.get('layerId'):
                # no support for directly reading layers by id in grass
                is_ogr_disk_based_layer = False

        if not is_ogr_disk_based_layer:
            # parameter is not a vector layer or not an OGR layer - try to convert to a source compatible with
            # grass OGR inputs and extract selection if required
            path = self.parameterAsCompatibleSourceLayerPath(parameters, name, context,
                                                             QgsVectorFileWriter.supportedFormatExtensions(),
                                                             feedback=feedback)
            ogr_layer = QgsVectorLayer(path, '', 'ogr')
            self.loadVectorLayer(name, ogr_layer, external=external, feedback=feedback)
        else:
            # already an ogr disk based layer source
            self.loadVectorLayer(name, layer, external=external, feedback=feedback)

    def loadVectorLayer(self, name, layer, external=False, feedback=None):
        """
        Creates a dedicated command to load a vector into
        temporary GRASS DB.
        :param name: name of the parameter
        :param layer: QgsMapLayer for the vector layer.
        :param external: use v.external (v.in.ogr if False).
        :param feedback: feedback object
        """
        # TODO: support multiple input formats
        if external is None:
            external = ProcessingConfig.getSetting(
                Grass7Utils.GRASS_USE_VEXTERNAL)

        source_parts = QgsProviderRegistry.instance().decodeUri('ogr', layer.source())
        file_path = source_parts.get('path')
        layer_name = source_parts.get('layerName')

        # safety check: we can only use external for ogr layers which support random read
        if external:
            if feedback is not None:
                feedback.pushInfo('Attempting to use v.external for direct layer read')
            ds = ogr.Open(file_path)
            if ds is not None:
                ogr_layer = ds.GetLayer()
                if ogr_layer is None or not ogr_layer.TestCapability(ogr.OLCRandomRead):
                    if feedback is not None:
                        feedback.reportError('Cannot use v.external: layer does not support random read')
                    external = False
            else:
                if feedback is not None:
                    feedback.reportError('Cannot use v.external: error reading layer')
                external = False

        self.inputLayers.append(layer)
        self.setSessionProjectionFromLayer(layer)
        destFilename = 'vector_{}'.format(os.path.basename(getTempFilename()))
        self.exportedLayers[name] = destFilename
        command = '{0}{1}{2} input="{3}"{4} output="{5}" --overwrite -o'.format(
            'v.external' if external else 'v.in.ogr',
            ' min_area={}'.format(self.minArea) if not external else '',
            ' snap={}'.format(self.snapTolerance) if not external else '',
            os.path.normpath(file_path),
            ' layer="{}"'.format(layer_name) if layer_name else '',
            destFilename)
        self.commands.append(command)

    def exportVectorLayerFromParameter(self, name, parameters, context, layer=None, nocats=False):
        """
        Creates a dedicated command to export a vector from
        a QgsProcessingParameter.
        :param name: name of the parameter.
        :param context: parameters context.
        :param layer: for vector with multiples layers, exports only one layer.
        :param nocats: do not export GRASS categories.
        """
        fileName = os.path.normpath(
            self.parameterAsOutputLayer(parameters, name, context))
        grassName = '{}{}'.format(name, self.uniqueSuffix)

        # Find if there is a dataType
        dataType = self.outType
        if self.outType == 'auto':
            parameter = self.parameterDefinition(name)
            if parameter:
                layerType = parameter.dataType()
                if layerType in self.QGIS_OUTPUT_TYPES:
                    dataType = self.QGIS_OUTPUT_TYPES[layerType]

        outFormat = QgsVectorFileWriter.driverForExtension(os.path.splitext(fileName)[1]).replace(' ', '_')
        dsco = self.parameterAsString(parameters, self.GRASS_VECTOR_DSCO, context)
        lco = self.parameterAsString(parameters, self.GRASS_VECTOR_LCO, context)
        exportnocat = self.parameterAsBool(parameters, self.GRASS_VECTOR_EXPORT_NOCAT, context)
        self.exportVectorLayer(grassName, fileName, layer, nocats, dataType, outFormat, dsco, lco, exportnocat)

    def exportVectorLayer(self, grassName, fileName, layer=None, nocats=False, dataType='auto',
                          outFormat=None, dsco=None, lco=None, exportnocat=False):
        """
        Creates a dedicated command to export a vector from
        temporary GRASS DB into a file via OGR.
        :param grassName: name of the vector to export.
        :param fileName: file path of vector layer.
        :param dataType: export only this type of data.
        :param layer: for vector with multiples layers, exports only one layer.
        :param nocats: do not export GRASS categories.
        :param outFormat: file format for export.
        :param dsco: datasource creation options for format.
        :param lco: layer creation options for format.
        :param exportnocat: do not export features without categories.
        """
        if outFormat is None:
            outFormat = QgsVectorFileWriter.driverForExtension(os.path.splitext(fileName)[1]).replace(' ', '_')

        for cmd in [self.commands, self.outputCommands]:
            cmd.append(
                'v.out.ogr{0} type="{1}" input="{2}" output="{3}" format="{4}" {5}{6}{7}{8} --overwrite'.format(
                    '' if nocats else '',
                    dataType, grassName, fileName,
                    outFormat,
                    'layer={}'.format(layer) if layer else '',
                    ' dsco="{}"'.format(dsco) if dsco else '',
                    ' lco="{}"'.format(lco) if lco else '',
                    ' -c' if exportnocat else ''
                )
            )

    def loadAttributeTableFromParameter(self, name, parameters, context):
        """
        Creates a dedicated command to load an attribute table
        into the temporary GRASS DB.
        :param name: name of the parameter
        :param parameters: Parameters of the algorithm.
        :param context: Processing context
        """
        table = self.parameterAsVectorLayer(parameters, name, context)
        self.loadAttributeTable(name, table)

    def loadAttributeTable(self, name, layer, destName=None):
        """
        Creates a dedicated command to load an attribute table
        into the temporary GRASS DB.
        :param name: name of the input parameter.
        :param layer: a layer object to import from.
        :param destName: force the name for the table into GRASS DB.
        """
        self.inputLayers.append(layer)
        if not destName:
            destName = 'table_{}'.format(os.path.basename(getTempFilename()))
        self.exportedLayers[name] = destName
        command = 'db.in.ogr --overwrite input="{0}" output="{1}"'.format(
            os.path.normpath(layer.source()), destName)
        self.commands.append(command)

    def exportAttributeTable(self, grassName, fileName, outFormat='CSV', layer=1):
        """
        Creates a dedicated command to export an attribute
        table from the temporary GRASS DB into a file via ogr.
        :param grassName: name of the parameter.
        :param fileName: file path of raster layer.
        :param outFormat: file format for export.
        :param layer: In GRASS a vector can have multiple layers.
        """
        for cmd in [self.commands, self.outputCommands]:
            cmd.append(
                'db.out.ogr input="{0}" output="{1}" layer={2} format={3} --overwrite'.format(
                    grassName, fileName, layer, outFormat
                )
            )

    def setSessionProjectionFromProject(self):
        """
        Set the projection from the project.
        We creates a PROJ4 definition which is transmitted to Grass
        """
        if not Grass7Utils.projectionSet and iface:
            proj4 = iface.mapCanvas().mapSettings().destinationCrs().toProj4()
            command = 'g.proj -c proj4="{}"'.format(proj4)
            self.commands.append(command)
            Grass7Utils.projectionSet = True

    def setSessionProjectionFromLayer(self, layer):
        """
        Set the projection from a QgsVectorLayer.
        We creates a PROJ4 definition which is transmitted to Grass
        """
        if not Grass7Utils.projectionSet:
            proj4 = str(layer.crs().toProj4())
            command = 'g.proj -c proj4="{}"'.format(proj4)
            self.commands.append(command)
            Grass7Utils.projectionSet = True

    def convertToHtml(self, fileName):
        # Read HTML contents
        lines = []
        with open(fileName, 'r', encoding='utf-8') as f:
            lines = f.readlines()

        if len(lines) > 1 and '<html>' not in lines[0]:
            # Then write into the HTML file
            with open(fileName, 'w', encoding='utf-8') as f:
                f.write('<html><head>')
                f.write('<meta http-equiv="Content-Type" content="text/html; charset=utf-8" /></head>')
                f.write('<body><p>')
                for line in lines:
                    f.write('{}</br>'.format(line))
                f.write('</p></body></html>')

    def canExecute(self):
        message = Grass7Utils.checkGrassIsInstalled()
        return not message, message

    def checkParameterValues(self, parameters, context):
        grass_parameters = {k: v for k, v in parameters.items()}
        if self.module:
            if hasattr(self.module, 'checkParameterValuesBeforeExecuting'):
                func = getattr(self.module, 'checkParameterValuesBeforeExecuting')
                return func(self, grass_parameters, context)
        return super().checkParameterValues(grass_parameters, context)
