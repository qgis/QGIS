"""
***************************************************************************
    grass_algorithm.py
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

from typing import (
    Dict,
    Optional
)
import sys
import os
import uuid
import math
import importlib
from pathlib import Path

from qgis.PyQt.QtCore import QCoreApplication, QUrl

from qgis.core import (Qgis,
                       QgsMapLayer,
                       QgsRasterLayer,
                       QgsApplication,
                       QgsMapLayerType,
                       QgsCoordinateReferenceSystem,
                       QgsProcessingUtils,
                       QgsProcessing,
                       QgsMessageLog,
                       QgsVectorFileWriter,
                       QgsProcessingContext,
                       QgsProcessingAlgorithm,
                       QgsProcessingParameterDefinition,
                       QgsProcessingException,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterString,
                       QgsProcessingParameterField,
                       QgsProcessingParameterPoint,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterRange,
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

import warnings

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    from osgeo import ogr

from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.parameters import getParameterFromString

from grassprovider.parsed_description import ParsedDescription
from grassprovider.grass_utils import GrassUtils

from processing.tools.system import isWindows, getTempFilename

pluginPath = os.path.normpath(os.path.join(
    os.path.split(os.path.dirname(__file__))[0], os.pardir))


class GrassAlgorithm(QgsProcessingAlgorithm):
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
    QGIS_OUTPUT_TYPES = {QgsProcessing.SourceType.TypeVectorAnyGeometry: 'auto',
                         QgsProcessing.SourceType.TypeVectorPoint: 'point',
                         QgsProcessing.SourceType.TypeVectorLine: 'line',
                         QgsProcessing.SourceType.TypeVectorPolygon: 'area'}

    def __init__(self,
                 description_file: Optional[Path] = None,
                 json_definition: Optional[Dict] = None,
                 description_folder: Optional[Path] = None
                 ):
        super().__init__()
        self._name = ''
        self._display_name = ''
        self._short_description = ''
        self._group = ''
        self._groupId = ''
        self.grass_name = ''
        self.params = []
        self.hardcodedStrings = []
        self.inputLayers = []
        self.commands = []
        self.outputCommands = []
        self.exportedLayers = {}
        self.fileOutputs = {}
        self._description_file: Optional[Path] = description_file
        self._json_definition: Optional[Dict] = json_definition
        self._description_folder: Optional[Path] = description_folder

        # Default GRASS parameters
        self.region = None
        self.cellSize = None
        self.snapTolerance = None
        self.outputType = None
        self.minArea = None
        self.alignToResolution = None

        # destination Crs for combineLayerExtents, will be set from layer or mapSettings
        self.destination_crs = QgsCoordinateReferenceSystem()

        # Load parameters from a description file
        if self._description_file is not None:
            self._define_characteristics_from_file()
        else:
            self._define_characteristics_from_json()

        self.numExportedLayers = 0
        # Do we need this anymore?
        self.uniqueSuffix = str(uuid.uuid4()).replace('-', '')

        # Use the ext mechanism
        self.module = None
        try:
            extpath = None
            ext_name = None
            if self._description_file:
                ext_name = self.name().replace('.', '_')
                extpath = self._description_file.parents[1].joinpath('ext', ext_name + '.py')
            elif self._json_definition.get('ext_path'):
                ext_name = self._json_definition['ext_path']
                extpath = self._description_folder.parents[0].joinpath(
                    'ext', ext_name + '.py')

            # this check makes it a bit faster
            if extpath and extpath.exists():
                spec = importlib.util.spec_from_file_location(
                    'grassprovider.ext.' + ext_name, extpath)
                self.module = importlib.util.module_from_spec(spec)
                spec.loader.exec_module(self.module)

        except Exception as e:
            QgsMessageLog.logMessage(self.tr('Failed to load: {0}\n{1}').format(extpath, e), 'Processing', Qgis.MessageLevel.Critical)
            pass

    def createInstance(self):
        return self.__class__(
            description_file=self._description_file,
            json_definition=self._json_definition,
            description_folder=self._description_folder)

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
        return super().flags() | QgsProcessingAlgorithm.Flag.FlagNoThreading | QgsProcessingAlgorithm.Flag.FlagDisplayNameIsLiteral

    def tr(self, string, context=''):
        if context == '':
            context = self.__class__.__name__
        return QCoreApplication.translate(context, string)

    def helpUrl(self):
        helpPath = GrassUtils.grassHelpPath()
        if helpPath == '':
            return None

        if os.path.exists(helpPath):
            return QUrl.fromLocalFile(os.path.join(helpPath, '{}.html'.format(self.grass_name))).toString()
        else:
            return helpPath + '{}.html'.format(self.grass_name)

    def initAlgorithm(self, config=None):
        """
        Algorithm initialization
        """
        for p in self.params:
            # We use createOutput argument for automatic output creation
            self.addParameter(p, True)

    def _define_characteristics_from_file(self):
        """
        Create algorithm parameters and outputs from a text file.
        """
        results = ParsedDescription.parse_description_file(
            self._description_file)
        self._define_characteristics_from_parsed_description(
            results
        )

    def _define_characteristics_from_json(self):
        """
        Create algorithm parameters and outputs from JSON definition.
        """
        results = ParsedDescription.from_dict(
            self._json_definition)
        self._define_characteristics_from_parsed_description(
            results
        )

    def _define_characteristics_from_parsed_description(
            self,
            description: ParsedDescription):
        """
        Create algorithm parameters and outputs from parsed description
        """
        self.grass_name = description.grass_command
        self._name = description.name
        self._short_description = description.short_description
        self._display_name = description.display_name
        self._group = description.group
        self._groupId = description.group_id
        self.hardcodedStrings = description.hardcoded_strings[:]

        self.params = []

        has_raster_input: bool = False
        has_vector_input: bool = False

        has_raster_output: bool = False
        has_vector_outputs: bool = False

        for param_string in description.param_strings:
            try:
                parameter = getParameterFromString(param_string, "GrassAlgorithm")
            except Exception as e:
                QgsMessageLog.logMessage(
                    QCoreApplication.translate("GrassAlgorithm",
                                               'Could not open GRASS GIS algorithm: {0}').format(
                        self._name),
                    QCoreApplication.translate("GrassAlgorithm",
                                               'Processing'),
                    Qgis.MessageLevel.Critical)
                raise e

            if parameter is None:
                continue

            self.params.append(parameter)
            if isinstance(parameter, (
                    QgsProcessingParameterVectorLayer,
                    QgsProcessingParameterFeatureSource)):
                has_vector_input = True
            elif isinstance(parameter,
                            QgsProcessingParameterRasterLayer):
                has_raster_input = True
            elif isinstance(parameter,
                            QgsProcessingParameterMultipleLayers):
                if parameter.layerType() < 3 or parameter.layerType() == 5:
                    has_vector_input = True
                elif parameter.layerType() == 3:
                    has_raster_input = True
            elif isinstance(parameter,
                            QgsProcessingParameterVectorDestination):
                has_vector_outputs = True
            elif isinstance(parameter,
                            QgsProcessingParameterRasterDestination):
                has_raster_output = True

        param = QgsProcessingParameterExtent(
            self.GRASS_REGION_EXTENT_PARAMETER,
            self.tr('GRASS GIS region extent'),
            optional=True
        )
        param.setFlags(param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced)
        self.params.append(param)

        if has_raster_output or has_raster_input:
            # Add a cellsize parameter
            param = QgsProcessingParameterNumber(
                self.GRASS_REGION_CELLSIZE_PARAMETER,
                self.tr('GRASS GIS region cellsize (leave 0 for default)'),
                type=QgsProcessingParameterNumber.Type.Double,
                minValue=0.0, maxValue=sys.float_info.max + 1, defaultValue=0.0
            )
            param.setFlags(param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced)
            self.params.append(param)

        if has_raster_output:
            # Add a createopt parameter for format export
            param = QgsProcessingParameterString(
                self.GRASS_RASTER_FORMAT_OPT,
                self.tr('Output Rasters format options (createopt)'),
                multiLine=True, optional=True
            )
            param.setFlags(param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced)
            param.setHelp(self.tr('Creation options should be comma separated'))
            self.params.append(param)

            # Add a metadata parameter for format export
            param = QgsProcessingParameterString(
                self.GRASS_RASTER_FORMAT_META,
                self.tr('Output Rasters format metadata options (metaopt)'),
                multiLine=True, optional=True
            )
            param.setFlags(param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced)
            param.setHelp(self.tr('Metadata options should be comma separated'))
            self.params.append(param)

        if has_vector_input:
            param = QgsProcessingParameterNumber(self.GRASS_SNAP_TOLERANCE_PARAMETER,
                                                 self.tr('v.in.ogr snap tolerance (-1 = no snap)'),
                                                 type=QgsProcessingParameterNumber.Type.Double,
                                                 minValue=-1.0, maxValue=sys.float_info.max + 1,
                                                 defaultValue=-1.0)
            param.setFlags(param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced)
            self.params.append(param)
            param = QgsProcessingParameterNumber(self.GRASS_MIN_AREA_PARAMETER,
                                                 self.tr('v.in.ogr min area'),
                                                 type=QgsProcessingParameterNumber.Type.Double,
                                                 minValue=0.0, maxValue=sys.float_info.max + 1,
                                                 defaultValue=0.0001)
            param.setFlags(param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced)
            self.params.append(param)

        if has_vector_outputs:
            # Add an optional output type
            param = QgsProcessingParameterEnum(self.GRASS_OUTPUT_TYPE_PARAMETER,
                                               self.tr('v.out.ogr output type'),
                                               self.OUTPUT_TYPES,
                                               defaultValue=0)
            param.setFlags(param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced)
            self.params.append(param)

            # Add a DSCO parameter for format export
            param = QgsProcessingParameterString(
                self.GRASS_VECTOR_DSCO,
                self.tr('v.out.ogr output data source options (dsco)'),
                multiLine=True, optional=True
            )
            param.setFlags(param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced)
            self.params.append(param)

            # Add a LCO parameter for format export
            param = QgsProcessingParameterString(
                self.GRASS_VECTOR_LCO,
                self.tr('v.out.ogr output layer options (lco)'),
                multiLine=True, optional=True
            )
            param.setFlags(param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced)
            self.params.append(param)

            # Add a -c flag for export
            param = QgsProcessingParameterBoolean(
                self.GRASS_VECTOR_EXPORT_NOCAT,
                self.tr('Also export features without category (not labeled). Otherwise only features with category are exported'),
                False
            )
            param.setFlags(param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced)
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
        self.alignToResolution = self.parameterAsBoolean(parameters,
                                                         self.GRASS_REGION_ALIGN_TO_RESOLUTION,
                                                         context)

    def processAlgorithm(self, original_parameters, context, feedback):
        if isWindows():
            path = GrassUtils.grassPath()
            if path == '':
                raise QgsProcessingException(
                    self.tr('GRASS GIS folder is not configured. Please '
                            'configure it before running GRASS GIS algorithms.'))

        # make a copy of the original parameters dictionary - it gets modified by grass algorithms
        parameters = {k: v for k, v in original_parameters.items()}

        # Create brand new commands lists
        self.commands = []
        self.outputCommands = []
        self.exportedLayers = {}
        self.fileOutputs = {}

        # If GRASS session has been created outside of this algorithm then
        # get the list of layers loaded in GRASS otherwise start a new
        # session
        existingSession = GrassUtils.sessionRunning
        if existingSession:
            self.exportedLayers = GrassUtils.getSessionLayers()
        else:
            GrassUtils.startGrassSession()

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
        loglines = [self.tr('GRASS GIS execution commands')]
        for line in self.commands:
            feedback.pushCommandInfo(line)
            loglines.append(line)
        if ProcessingConfig.getSetting(GrassUtils.GRASS_LOG_COMMANDS):
            QgsMessageLog.logMessage("\n".join(loglines), self.tr('Processing'), Qgis.MessageLevel.Info)

        GrassUtils.executeGrass(self.commands, feedback, self.outputCommands)

        # If the session has been created outside of this algorithm, add
        # the new GRASS GIS layers to it otherwise finish the session
        if existingSession:
            GrassUtils.addSessionLayers(self.exportedLayers)
        else:
            GrassUtils.endGrassSession()

        # Return outputs map
        outputs = {}
        for out in self.outputDefinitions():
            outName = out.name()
            if outName in parameters:
                if outName in self.fileOutputs:
                    outputs[outName] = self.fileOutputs[outName]
                else:
                    outputs[outName] = parameters[outName]
                if isinstance(out, QgsProcessingOutputHtml):
                    if self.module and hasattr(self.module, 'convertToHtml'):
                        func = getattr(self.module, 'convertToHtml')
                        func(self, self.fileOutputs[outName], outputs)
                    else:
                        self.convertToHtml(self.fileOutputs[outName])
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
                    if QgsProcessing.SourceType.TypeFile in param.dataTypes():
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
                        self.loadRasterLayer(layerName, layer, context)
                    # Add a vector layer
                    elif layer.type() == QgsMapLayerType.VectorLayer:
                        self.loadVectorLayer(layerName, layer, context, external=None, feedback=feedback)

        self.postInputs(context)

    def postInputs(self, context: QgsProcessingContext):
        """
        After layer imports, we need to update some internal parameters
        """
        # If projection has not already be set, use the project
        self.setSessionProjectionFromProject(context)

        # Build GRASS region
        if self.region.isEmpty():
            self.region = QgsProcessingUtils.combineLayerExtents(self.inputLayers, self.destination_crs, context)
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

        QgsMessageLog.logMessage(self.tr('processInputs end. Commands: {}').format(self.commands), 'Grass', Qgis.MessageLevel.Info)

    def processCommand(self, parameters, context, feedback, delOutputs=False):
        """
        Prepare the GRASS algorithm command
        :param parameters:
        :param context:
        :param delOutputs: do not add outputs to commands.
        """
        noOutputs = [o for o in self.parameterDefinitions() if o not in self.destinationParameterDefinitions()]
        command = '{} '.format(self.grass_name)
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
                if self.parameterAsBoolean(parameters, paramName, context):
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
            elif isinstance(param, QgsProcessingParameterRange):
                v = self.parameterAsRange(parameters, paramName, context)
                if (param.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional) and (math.isnan(v[0]) or math.isnan(v[1])):
                    continue
                else:
                    value = '{},{}'.format(v[0], v[1])
            elif isinstance(param, QgsProcessingParameterCrs):
                if self.parameterAsCrs(parameters, paramName, context):
                    # TODO: ideally we should be exporting to WKT here, but it seems not all grass algorithms
                    # will accept a wkt string for a crs value (e.g. r.tileset)
                    value = '"{}"'.format(self.parameterAsCrs(parameters, paramName, context).toProj())
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
                if out.flags() & QgsProcessingParameterDefinition.Flag.FlagHidden:
                    continue
                outName = out.name()
                # For File destination
                if isinstance(out, QgsProcessingParameterFileDestination):
                    if outName in parameters and parameters[outName] is not None:
                        outPath = self.parameterAsFileOutput(parameters, outName, context)
                        self.fileOutputs[outName] = outPath
                        # for HTML reports, we need to redirect stdout
                        if out.defaultFileExtension().lower() == 'html':
                            if outName == 'html':
                                # for "fake" outputs redirect command stdout
                                command += ' > "{}"'.format(outPath)
                            else:
                                # for real outputs only output itself should be redirected
                                command += ' {}=- > "{}"'.format(outName, outPath)
                        else:
                            command += ' {}="{}"'.format(outName, outPath)
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
        QgsMessageLog.logMessage(self.tr('processCommands end. Commands: {}').format(self.commands), 'Grass', Qgis.MessageLevel.Info)

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

    def loadRasterLayerFromParameter(self, name, parameters,
                                     context: QgsProcessingContext,
                                     external=None, band=1):
        """
        Creates a dedicated command to load a raster into
        the temporary GRASS DB.
        :param name: name of the parameter.
        :param parameters: algorithm parameters dict.
        :param context: algorithm context.
        :param external: use r.external if True, r.in.gdal otherwise.
        :param band: imports only specified band. None for all bands.
        """
        layer = self.parameterAsRasterLayer(parameters, name, context)
        self.loadRasterLayer(name, layer, context, external, band)

    def loadRasterLayer(self, name, layer, context: QgsProcessingContext,
                        external=None, band=1, destName=None):
        """
        Creates a dedicated command to load a raster into
        the temporary GRASS DB.
        :param name: name of the parameter.
        :param layer: QgsMapLayer for the raster layer.
        :param context: Processing context
        :param external: use r.external if True, r.in.gdal if False.
        :param band: imports only specified band. None for all bands.
        :param destName: force the destination name of the raster.
        """
        if external is None:
            external = ProcessingConfig.getSetting(GrassUtils.GRASS_USE_REXTERNAL)
        self.inputLayers.append(layer)
        self.setSessionProjectionFromLayer(layer, context)
        if not destName:
            destName = 'rast_{}'.format(os.path.basename(getTempFilename(context=context)))
        self.exportedLayers[name] = destName
        command = '{} input="{}" {}output="{}" --overwrite -o'.format(
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
        outFormat = GrassUtils.getRasterFormatFromFilename(fileName)
        createOpt = self.parameterAsString(parameters, self.GRASS_RASTER_FORMAT_OPT, context)
        metaOpt = self.parameterAsString(parameters, self.GRASS_RASTER_FORMAT_META, context)
        self.exportRasterLayer(grassName, fileName, colorTable, outFormat, createOpt, metaOpt)

        self.fileOutputs[name] = fileName

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
        createOpt = createOpt or GrassUtils.GRASS_RASTER_FORMATS_CREATEOPTS.get(outFormat)

        for cmd in [self.commands, self.outputCommands]:
            # Adjust region to layer before exporting
            cmd.append('g.region raster={}'.format(grassName))
            cmd.append(
                'r.out.gdal -t -m{} input="{}" output="{}" format="{}" {}{} --overwrite'.format(
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
                cmd.append("for /F %%r IN ('g.list type^=rast pattern^=\"{}*\"') do r.out.gdal -m{} input=%%r output={}/%%r.tif {}".format(
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
            self.loadVectorLayer(name, ogr_layer, context, external=external, feedback=feedback)
        else:
            # already an ogr disk based layer source
            self.loadVectorLayer(name, layer, context, external=external, feedback=feedback)

    def loadVectorLayer(self, name, layer,
                        context: QgsProcessingContext,
                        external=False, feedback=None):
        """
        Creates a dedicated command to load a vector into
        temporary GRASS DB.
        :param name: name of the parameter
        :param layer: QgsMapLayer for the vector layer.
        :param context: Processing context
        :param external: use v.external (v.in.ogr if False).
        :param feedback: feedback object
        """
        # TODO: support multiple input formats
        if external is None:
            external = ProcessingConfig.getSetting(
                GrassUtils.GRASS_USE_VEXTERNAL)

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
        self.setSessionProjectionFromLayer(layer, context)
        destFilename = 'vector_{}'.format(os.path.basename(getTempFilename(context=context)))
        self.exportedLayers[name] = destFilename
        command = '{}{}{} input="{}"{} output="{}" --overwrite -o'.format(
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
        exportnocat = self.parameterAsBoolean(parameters, self.GRASS_VECTOR_EXPORT_NOCAT, context)
        self.exportVectorLayer(grassName, fileName, layer, nocats, dataType, outFormat, dsco, lco, exportnocat)

        self.fileOutputs[name] = fileName

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
                'v.out.ogr{} type="{}" input="{}" output="{}" format="{}" {}{}{}{} --overwrite'.format(
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
        self.loadAttributeTable(name, table, context)

    def loadAttributeTable(self, name, layer, context: QgsProcessingContext,
                           destName=None):
        """
        Creates a dedicated command to load an attribute table
        into the temporary GRASS DB.
        :param name: name of the input parameter.
        :param layer: a layer object to import from.
        :param context: processing context
        :param destName: force the name for the table into GRASS DB.
        """
        self.inputLayers.append(layer)
        if not destName:
            destName = 'table_{}'.format(os.path.basename(getTempFilename(context=context)))
        self.exportedLayers[name] = destName
        command = 'db.in.ogr --overwrite input="{}" output="{}"'.format(
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
                'db.out.ogr input="{}" output="{}" layer={} format={} --overwrite'.format(
                    grassName, fileName, layer, outFormat
                )
            )

    def setSessionProjectionFromProject(self, context: QgsProcessingContext):
        """
        Set the projection from the project.
        We create a WKT definition which is transmitted to Grass
        """
        if not GrassUtils.projectionSet and iface:
            self.setSessionProjection(
                iface.mapCanvas().mapSettings().destinationCrs(),
                context
            )

    def setSessionProjectionFromLayer(self, layer: QgsMapLayer, context: QgsProcessingContext):
        """
        Set the projection from a QgsVectorLayer.
        We create a WKT definition which is transmitted to Grass
        """
        if not GrassUtils.projectionSet:
            self.setSessionProjection(layer.crs(), context)

    def setSessionProjection(self, crs, context: QgsProcessingContext):
        """
        Set the session projection to the specified CRS
        """
        self.destination_crs = crs
        file_name = GrassUtils.exportCrsWktToFile(crs, context)
        command = 'g.proj -c wkt="{}"'.format(file_name)
        self.commands.append(command)
        GrassUtils.projectionSet = True

    def convertToHtml(self, fileName):
        # Read HTML contents
        lines = []
        with open(fileName, encoding='utf-8') as f:
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
        message = GrassUtils.checkGrassIsInstalled()
        return not message, message

    def checkParameterValues(self, parameters, context):
        grass_parameters = {k: v for k, v in parameters.items()}
        if self.module:
            if hasattr(self.module, 'checkParameterValuesBeforeExecuting'):
                func = getattr(self.module, 'checkParameterValuesBeforeExecuting')
                return func(self, grass_parameters, context)
        return super().checkParameterValues(grass_parameters, context)
