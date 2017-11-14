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
from builtins import str
from builtins import range

__author__ = 'Victor Olaya'
__date__ = 'February 2015'
__copyright__ = '(C) 2012-2015, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import sys
import os
import uuid
import importlib

from qgis.PyQt.QtCore import QCoreApplication, QUrl

from qgis.core import (QgsRasterLayer,
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
                       QgsProcessingParameterVectorLayer,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterMultipleLayers,
                       QgsProcessingParameterVectorDestination,
                       QgsProcessingParameterRasterDestination,
                       QgsProcessingParameterFileDestination,
                       QgsProcessingParameterFolderDestination,
                       QgsProcessingOutputFolder,
                       QgsProcessingOutputVectorLayer,
                       QgsProcessingOutputRasterLayer,
                       QgsProcessingOutputHtml,
                       QgsProcessingUtils)
from qgis.utils import iface

from processing.core.ProcessingConfig import ProcessingConfig

from processing.core.parameters import (getParameterFromString)

from .Grass7Utils import Grass7Utils

#from processing.tools import dataobjects, system
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

    OUTPUT_TYPES = ['auto', 'point', 'line', 'area']
    QGIS_OUTPUT_TYPES = {QgsProcessing.TypeVectorAnyGeometry: 'auto',
                         QgsProcessing.TypeVectorPoint: 'point',
                         QgsProcessing.TypeVectorLine: 'line',
                         QgsProcessing.TypeVectorPolygon: 'area'}

    def __init__(self, descriptionfile):
        super().__init__()
        self._name = ''
        self._display_name = ''
        self._group = ''
        self.grass7Name = ''
        self.params = []
        self.hardcodedStrings = []
        self.inputLayers = []
        self.descriptionFile = descriptionfile

        # Default GRASS parameters
        self.region = None
        self.cellSize = None
        self.snaptTolerance = None
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

    def group(self):
        return self._group

    def icon(self):
        return QgsApplication.getThemeIcon("/providerGrass.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerGrass.svg")

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
            res = self.addParameter(p, True)
            # File destinations are not automatically added as outputs
            if (isinstance(p, QgsProcessingParameterFileDestination)
                    and p.defaultFileExtension().lower() == 'html'):
                self.addOutput(QgsProcessingOutputHtml(p.name(), p.description()))

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
            self._name = line
            self._display_name = QCoreApplication.translate("GrassAlgorithm", line)
            if " - " not in self._name:
                self._name = self.grass7Name + " - " + self._name
                self._display_name = self.grass7Name + " - " + self._display_name

            self._name = self._name[:self._name.find(' ')].lower()
            # Read the grass group
            line = lines.readline().strip('\n').strip()
            self._group = QCoreApplication.translate("GrassAlgorithm", line)
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
                    parameter = getParameterFromString(line)
                    if parameter is not None:
                        self.params.append(parameter)
                        if isinstance(parameter, QgsProcessingParameterVectorLayer):
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
                    QgsMessageLog.logMessage(self.tr('Could not open GRASS GIS 7 algorithm: {0}\n{1}').format(self.descriptionFile, line), self.tr('Processing'), QgsMessageLog.CRITICAL)
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
            param = QgsProcessingParameterEnum(self.GRASS_OUTPUT_TYPE_PARAMETER,
                                               self.tr('v.out.ogr output type'),
                                               self.OUTPUT_TYPES)
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

    def processAlgorithm(self, parameters, context, feedback):
        if isWindows():
            path = Grass7Utils.grassPath()
            if path == '':
                raise QgsProcessingException(
                    self.tr('GRASS GIS 7 folder is not configured. Please '
                            'configure it before running GRASS GIS 7 algorithms.'))

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
                getattr(self.module, fullName)(self, parameters, context)
            else:
                getattr(self, fullName)(parameters, context)

        # Run GRASS
        loglines = []
        loglines.append(self.tr('GRASS GIS 7 execution commands'))
        for line in self.commands:
            feedback.pushCommandInfo(line)
            loglines.append(line)
        if ProcessingConfig.getSetting(Grass7Utils.GRASS_LOG_COMMANDS):
            QgsMessageLog.logMessage("\n".join(loglines), self.tr('Processing'), QgsMessageLog.INFO)

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

    def processInputs(self, parameters, context):
        """Prepare the GRASS import commands"""
        inputs = [p for p in self.parameterDefinitions()
                  if isinstance(p, (QgsProcessingParameterVectorLayer,
                                    QgsProcessingParameterRasterLayer,
                                    QgsProcessingParameterMultipleLayers))]
        for param in inputs:
            paramName = param.name()
            if not paramName in parameters:
                continue
            if isinstance(parameters[paramName], str) and len(parameters[paramName]) == 0:
                continue
            # Raster inputs needs to be imported into temp GRASS DB
            if isinstance(param, QgsProcessingParameterRasterLayer):
                if paramName not in self.exportedLayers:
                    self.loadRasterLayerFromParameter(
                        paramName, parameters, context)
            # Vector inputs needs to be imported into temp GRASS DB
            elif isinstance(param, QgsProcessingParameterVectorLayer):
                if paramName not in self.exportedLayers:
                    self.loadVectorLayerFromParameter(
                        paramName, parameters, context)
            # TODO: find the best replacement for ParameterTable
            #if isinstance(param, ParameterTable):
            #    pass
            # For multiple inputs, process each layer
            elif isinstance(param, QgsProcessingParameterMultipleLayers):
                layers = self.parameterAsLayerList(parameters, paramName, context)
                for idx, layer in enumerate(layers):
                    layerName = '{}_{}'.format(paramName, idx)
                    # Add a raster layer
                    if layer.type() == QgsMapLayer.RasterLayer:
                        self.loadRasterLayer(layerName, layer)
                    # Add a vector layer
                    elif layer.type() == QgsMapLayer.VectorLayer:
                        self.loadVectorLayer(layerName, layer)

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

        QgsMessageLog.logMessage('processInputs end. Commands: {}'.format(self.commands), 'Grass7', QgsMessageLog.INFO)

    def processCommand(self, parameters, context, delOutputs=False):
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
                             self.GRASS_RASTER_FORMAT_META]:
                continue

            # Raster and vector layers
            if isinstance(param, (QgsProcessingParameterRasterLayer,
                                  QgsProcessingParameterVectorLayer)):
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
            # For enumeration, we need to grab the string value
            elif isinstance(param, QgsProcessingParameterEnum):
                idx = self.parameterAsEnum(parameters, paramName, context)
                value = '"{}"'.format(param.options()[idx])
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
                value = '{}'.format(
                    self.parameterAsString(parameters, paramName, context)
                )
            # For numbers and points, we translate as a string
            elif isinstance(param, (QgsProcessingParameterNumber,
                                    QgsProcessingParameterPoint)):
                value = self.parameterAsString(parameters, paramName, context)
            # For everything else, we assume that it is a string
            else:
                value = '"{}"'.format(
                    self.parameterAsString(parameters, paramName, context)
                )
            if value:
                command += ' {}={}'.format(paramName, value)

        # Handle outputs
        if not delOutputs:
            for out in self.destinationParameterDefinitions():
                outName = out.name()
                # For File destination
                if isinstance(out, QgsProcessingParameterFileDestination):
                    # for HTML reports, we need to redirect stdout
                    if out.defaultFileExtension().lower() == 'html':
                        command += ' > "{}"'.format(
                            self.parameterAsFileOutput(
                                parameters, outName, context)
                        )
                    else:
                        command += ' {}="{}"'.format(
                            outName,
                            self.parameterAsFileOutput(
                                parameters, outName, context))
                # For folders destination
                elif isinstance(out, QgsProcessingParameterFolderDestination):
                    # We need to add a unique temporary basename
                    uniqueBasename = outName + self.uniqueSuffix
                    command += ' {}={}'.format(outName, uniqueBasename)
                else:
                    # We add an output name to make sure it is unique if the session
                    # uses this algorithm several times.
                    #value = self.parameterAsOutputLayer(parameters, outName, context)
                    uniqueOutputName = outName + self.uniqueSuffix
                    command += ' {}={}'.format(outName, uniqueOutputName)

                    # Add output file to exported layers, to indicate that
                    # they are present in GRASS
                    self.exportedLayers[outName] = uniqueOutputName

        command += ' --overwrite'
        self.commands.append(command)
        QgsMessageLog.logMessage('processCommands end. Commands: {}'.format(self.commands), 'Grass7', QgsMessageLog.INFO)

    def vectorOutputType(self, parameters, context):
        """Determine vector output types for outputs"""
        self.outType = 'auto'
        if self.parameterDefinition(self.GRASS_OUTPUT_TYPE_PARAMETER):
            typeidx = self.parameterAsEnum(parameters,
                                           self.GRASS_OUTPUT_TYPE_PARAMETER,
                                           context)
            self.outType = ('auto' if typeidx
                            is None else self.OUTPUT_TYPES[typeidx])

    def processOutputs(self, parameters, context):
        """Prepare the GRASS v.out.ogr commands"""
        # TODO: support multiple raster formats.
        # TODO: support multiple vector formats.

        # Determine general vector output type
        self.vectorOutputType(parameters, context)

        for out in self.destinationParameterDefinitions():
            outName = out.name()
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

    def loadRasterLayer(self, name, layer, external=True, band=1):
        """
        Creates a dedicated command to load a raster into
        the temporary GRASS DB.
        :param name: name of the parameter.
        :param layer: QgsMapLayer for the raster layer.
        :param external: True if using r.external.
        :param band: imports only specified band. None for all bands.
        """
        self.inputLayers.append(layer)
        self.setSessionProjectionFromLayer(layer)
        destFilename = 'a' + os.path.basename(getTempFilename())
        self.exportedLayers[name] = destFilename
        command = '{0} input="{1}" {2}output="{3}" --overwrite -o'.format(
            'r.external' if external else 'r.in.gdal',
            os.path.normpath(layer.source()),
            'band={} '.format(band) if band else '',
            destFilename)
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
        fileName = os.path.normpath(
            self.parameterAsOutputLayer(parameters, name, context))
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
        :param metatOpt: metadata options for export.
        """
        if not createOpt:
            if outFormat in Grass7Utils.GRASS_RASTER_FORMATS_CREATEOPTS:
                createOpt = Grass7Utils.GRASS_RASTER_FORMATS_CREATEOPTS[outFormat]

        for cmd in [self.commands, self.outputCommands]:
            # Adjust region to layer before exporting
            cmd.append('g.region raster={}'.format(grassName))
            cmd.append(
                'r.out.gdal -c -m{0} input="{1}" output="{2}" format="{3}" {4}{5} --overwrite'.format(
                    ' -t' if colorTable else '',
                    grassName, fileName,
                    outFormat,
                    ' createopt="{}"'.format(createOpt) if createOpt else '',
                    ' metaopt="{}"'.format(metaOpt) if metaOpt else ''
                )
            )

    def exportRasterLayersIntoDirectory(self, name, parameters, context, colorTable=True):
        """
        Creates a dedicated loop command to export rasters from
        temporary GRASS DB into a directory via gdal.
        :param name: name of the output directory parameter.
        :param parameters: Algorithm parameters dict.
        :param context: Algorithm context.
        :param colorTable: preserve color Table.
        """
        # Grab directory name and temporary basename
        outDir = os.path.normpath(
            self.parameterAsString(parameters, name, context))
        basename = name + self.uniqueSuffix

        # Add a loop export from the basename
        for cmd in [self.commands, self.outputCommands]:
            # Adjust region to layer before exporting
            # TODO: Does-it works under MS-Windows or MacOSX?
            cmd.append("for r in $(g.list type=rast pattern='{}*'); do".format(basename))
            cmd.append("  r.out.gdal -m{0} input=${{r}} output={1}/${{r}}.tif {2}".format(
                ' -t' if colorTable else '', outDir,
                '--overwrite -c createopt="TFW=YES,COMPRESS=LZW"'
            )
            )
            cmd.append("done")

    def loadVectorLayerFromParameter(self, name, parameters, context, external=None):
        """
        Creates a dedicated command to load a vector into
        the temporary GRASS DB.
        :param name: name of the parameter
        :param parameters: Parameters of the algorithm.
        :param context: Processing context
        :param external: use v.external (v.in.ogr if False).
        """
        layer = self.parameterAsVectorLayer(parameters, name, context)
        self.loadVectorLayer(name, layer, external)

    def loadVectorLayer(self, name, layer, external=None):
        """
        Creates a dedicated command to load a vector into
        temporary GRASS DB.
        :param name: name of the parameter
        :param layer: QgsMapLayer for the vector layer.
        :param external: use v.external (v.in.ogr if False).
        """
        # TODO: support selections
        # TODO: support multiple input formats
        if external is None:
            external = ProcessingConfig.getSetting(
                Grass7Utils.GRASS_USE_VEXTERNAL)
        self.inputLayers.append(layer)
        self.setSessionProjectionFromLayer(layer)
        destFilename = 'a' + os.path.basename(getTempFilename())
        self.exportedLayers[name] = destFilename
        command = '{0}{1}{2} input="{3}" output="{4}" --overwrite -o'.format(
            'v.external' if external else 'v.in.ogr',
            ' min_area={}'.format(self.minArea) if not external else '',
            ' snap={}'.format(self.snapTolerance) if not external else '',
            os.path.normpath(layer.source()),
            destFilename)
        self.commands.append(command)

    def exportVectorLayerFromParameter(self, name, parameters, context):
        """
        Creates a dedicated command to export a raster from
        temporary GRASS DB into a file via gdal.
        :param grassName: name of the parameter
        :param fileName: file path of raster layer
        :param colorTable: preserve color Table.
        """
        fileName = os.path.normpath(
            self.parameterAsOutputLayer(parameters, name, context))
        # Find if there is a dataType
        dataType = self.outType
        if self.outType == 'auto':
            parameter = self.parameterDefinition(name)
            if parameter:
                layerType = parameter.dataType()
                if layerType in self.QGIS_OUTPUT_TYPES:
                    dataType = self.QGIS_OUTPUT_TYPES[layerType]

        grassName = '{}{}'.format(name, self.uniqueSuffix)
        self.exportVectorLayer(grassName, fileName, dataType)

    def exportVectorLayer(self, grassName, fileName, dataType='auto', layer=None, nocats=False):
        """
        Creates a dedicated command to export a vector from
        temporary GRASS DB into a file via ogr.
        :param grassName: name of the parameter
        :param fileName: file path of raster layer
        """
        for cmd in [self.commands, self.outputCommands]:
            cmd.append(
                'v.out.ogr{0} type={1} {2} input="{3}" output="{4}" {5}'.format(
                    '' if nocats else ' -c',
                    dataType,
                    'layer={}'.format(layer) if layer else '',
                    grassName,
                    fileName,
                    'format=ESRI_Shapefile --overwrite'
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
        html = False
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
        if self.module:
            if hasattr(self.module, 'checkParameterValuesBeforeExecuting'):
                func = getattr(self.module, 'checkParameterValuesBeforeExecuting')
                #return func(self, parameters, context), None
                return None, func(self, parameters, context)
        return super(Grass7Algorithm, self).checkParameterValues(parameters, context)
