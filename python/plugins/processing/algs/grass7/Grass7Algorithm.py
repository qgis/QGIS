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
                       QgsProcessingUtils,
                       QgsMessageLog,
                       QgsVectorFileWriter,
                       QgsProcessingAlgorithm,
                       QgsProcessingParameterDefinition,
                       QgsProcessingException,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterString,
                       QgsProcessingParameterPoint,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterVectorLayer,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterMultipleLayers,
                       QgsProcessingParameterVectorDestination,
                       QgsProcessingParameterRasterDestination,
                       QgsProcessingParameterFileDestination,
                       QgsProcessingOutputVectorLayer,
                       QgsProcessingOutputRasterLayer,
                       QgsProcessingOutputHtml)
from qgis.utils import iface

from processing.core.ProcessingConfig import ProcessingConfig

from processing.core.parameters import (getParameterFromString)

from .Grass7Utils import Grass7Utils

from processing.tools import dataobjects, system

pluginPath = os.path.normpath(os.path.join(
    os.path.split(os.path.dirname(__file__))[0], os.pardir))


class Grass7Algorithm(QgsProcessingAlgorithm):

    GRASS_OUTPUT_TYPE_PARAMETER = 'GRASS_OUTPUT_TYPE_PARAMETER'
    GRASS_MIN_AREA_PARAMETER = 'GRASS_MIN_AREA_PARAMETER'
    GRASS_SNAP_TOLERANCE_PARAMETER = 'GRASS_SNAP_TOLERANCE_PARAMETER'
    GRASS_REGION_EXTENT_PARAMETER = 'GRASS_REGION_PARAMETER'
    GRASS_REGION_CELLSIZE_PARAMETER = 'GRASS_REGION_CELLSIZE_PARAMETER'
    GRASS_REGION_ALIGN_TO_RESOLUTION = '-a_r.region'

    OUTPUT_TYPES = ['auto', 'point', 'line', 'area']

    def __init__(self, descriptionfile):
        super().__init__()
        self._name = ''
        self._display_name = ''
        self._group = ''
        self.grass7Name = ''
        self.params = []
        self.hardcodedStrings = []
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
            self.module = importlib.import_module('processing.algs.grass7.ext.' + name)
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

    def getParameterDescriptions(self):
        descs = {}
        _, helpfile = self.help()
        try:
            with open(helpfile) as infile:
                lines = infile.readlines()
                for i in range(len(lines)):
                    if lines[i].startswith('<DT><b>'):
                        for param in self.parameterDefinitions():
                            searchLine = '<b>' + param.name() + '</b>'
                            if searchLine in lines[i]:
                                i += 1
                                descs[param.name()] = (lines[i])[4:-6]
                                break

        except Exception:
            pass
        return descs
    
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
                        elif isinstance(parameter, QgsProcessingParameterMultipleLayers) \
                           and parameter.layerType() < 3:
                            hasVectorInput = True
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
        
        if hasRasterOutput:
            param = QgsProcessingParameterNumber(
                self.GRASS_REGION_CELLSIZE_PARAMETER,
                self.tr('GRASS GIS 7 region cellsize (leave 0 for default)'),
                type=QgsProcessingParameterNumber.Double,
                minValue=0.0, maxValue=sys.float_info.max + 1, defaultValue=0.0
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

    def getDefaultCellSize(self, parameters, context):
        cellsize = 0
        for param in self.parameterDefinitions():
            if param.name() in parameters:
                value = parameters[param.name()]
                if isinstance(param, QgsProcessingParameterRasterLayer):
                    layer =  self.parameterAsExtent(parameters, paramName, context)
                    #if isinstance(value, QgsRasterLayer):
                    #    layer = value
                    #else:
                    #    layer = QgsProcessingUtils.mapLayerFromString(param.value, context)
                    cellsize = max(cellsize, (layer.extent().xMaximum() -
                                              layer.extent().xMinimum()) /
                                   layer.width())
                elif isinstance(param, QgsProcessingParameterMultipleLayers):
                    # TODO: finish
                    layers = value.split(';')
                    for layername in layers:
                        layer = QgsProcessingUtils.mapLayerFromString(layername, context)
                        if isinstance(layer, QgsRasterLayer):
                            cellsize = max(cellsize, (
                                layer.extent().xMaximum() -
                                layer.extent().xMinimum()) /
                                layer.width()
                            )

        if cellsize == 0:
            cellsize = 100
        return cellsize

    def grabDefaultGrassParameters(self, parameters, context):
        """
        Imports default GRASS parameters (EXTENT, etc) into
        object attributes for faster retrieving.
        """
        # GRASS region extent
        self.region =  self.parameterAsExtent(parameters,
                                              self.GRASS_REGION_EXTENT_PARAMETER,
                                              context)
        QgsMessageLog.logMessage('processAlgorithm self.region: {}'.format(self.region.isEmpty()), 'Grass7', QgsMessageLog.INFO)                
        # GRASS cell size
        self.cellSize = self.parameterAsString(parameters,
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
        if system.isWindows():
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
            Grass7Utils.startGrass7Session()

        # Handle default GRASS parameters
        self.grabDefaultGrassParameters(parameters, context)
        
        # Handle ext functions for inputs/command/outputs
        if self.module:
            if hasattr(self.module, 'processInputs'):
                func = getattr(self.module, 'processInputs')
                func(self)
            else:
                self.processInputs(parameters, context)

            if hasattr(self.module, 'processCommand'):
                func = getattr(self.module, 'processCommand')
                func(self)
            else:
                self.processCommand()

            if hasattr(self.module, 'processOutputs'):
                func = getattr(self.module, 'processOutputs')
                func(self)
            else:
                self.processOutputs()
        else:
            self.processInputs(parameters, context)
            self.processCommand(parameters, context)
            self.processOutputs(parameters, context)

        # Run GRASS
        loglines = []
        loglines.append(self.tr('GRASS GIS 7 execution commands'))
        for line in self.commands:
            feedback.pushCommandInfo(line)
            loglines.append(line)
        if ProcessingConfig.getSetting(Grass7Utils.GRASS_LOG_COMMANDS):
            QgsMessageLog.logMessage("\n".join(loglines), self.tr('Processing'), QgsMessageLog.INFO)

        Grass7Utils.executeGrass7(self.commands, feedback, self.outputCommands)

        # TODO handle and detect outputHtml
        #for out in self.destinationParameterDefinitions():
        #    if isinstance(out, QgsProcessingParameterOutputHTML):
        #        with open(self.getOutputFromName("rawoutput").value) as f:
        #            rawOutput = "".join(f.readlines())
        #        with open(out.value, "w") as f:
        #            f.write("<pre>%s</pre>" % rawOutput)

        # If the session has been created outside of this algorithm, add
        # the new GRASS GIS 7 layers to it otherwise finish the session
        if existingSession:
            Grass7Utils.addSessionLayers(self.exportedLayers)
        else:
            Grass7Utils.endGrass7Session()

        # Return outputs map
        outputs = {}
        for outName in [o.name() for o in self.outputDefinitions()]:
            if outName in parameters:
                outputs[outName] = parameters[outName]
        QgsMessageLog.logMessage('processAlgorithm end. outputs: {}'.format(outputs), 'Grass7', QgsMessageLog.INFO)                
        return outputs

    def processInputs(self, parameters, context):
        """Prepare the GRASS import commands"""
        layers=[]
        inputs = [p for p in self.parameterDefinitions()
                  if isinstance(p, (QgsProcessingParameterVectorLayer,
                                    QgsProcessingParameterRasterLayer,
                                    QgsProcessingParameterMultipleLayers))]
        for param in inputs:
            paramName = param.name()
            # Raster inputs needs to be imported into temp GRASS DB
            if isinstance(param, QgsProcessingParameterRasterLayer):
                if not paramName in parameters:
                    continue
                layer = self.parameterAsRasterLayer(parameters, paramName, context)
                layerSrc = parameters[paramName]
                # Check if the layer hasn't already been exported in, for
                # example, previous GRASS calls in this session
                if paramName in self.exportedLayers:
                    continue
                else:
                    layers.append(layer)
                    self.setSessionProjectionFromLayer(layer)
                    self.commands.append(self.exportRasterLayer(paramName, layerSrc))
            # Vector inputs needs to be imported into temp GRASS DB
            if isinstance(param, QgsProcessingParameterVectorLayer):
                if not paramName in parameters:
                    continue
                layer = self.parameterAsVectorLayer(parameters, paramName, context)
                layerSrc = self.parameterAsCompatibleSourceLayerPath(
                    parameters, paramName, context,
                    self.provider().supportedOutputVectorLayerExtensions()
                )
                                                   
                if paramName in self.exportedLayers:
                    continue
                else:
                    layers.append(layer)
                    self.setSessionProjectionFromLayer(layer)
                    self.commands.append(self.exportVectorLayer(paramName, layerSrc))
            # TODO: find the best replacement for ParameterTable
            #if isinstance(param, ParameterTable):
            #    pass
            if isinstance(param, QgsProcessingParameterMultipleLayers):
                if not param.name() in parameters:
                    continue
                value = parameters[param.name()]
                layers = value.split(';')
                if layers is None or len(layers) == 0:
                    continue
                if param.datatype == dataobjects.TYPE_RASTER:
                    for layer in layers:
                        if layer in list(self.exportedLayers.keys()):
                            continue
                        else:
                            self.setSessionProjectionFromLayer(layer, self.commands)
                            self.commands.append(self.exportRasterLayer(layer))
                elif param.datatype in [dataobjects.TYPE_VECTOR_ANY,
                                        dataobjects.TYPE_VECTOR_LINE,
                                        dataobjects.TYPE_VECTOR_POLYGON,
                                        dataobjects.TYPE_VECTOR_POINT]:
                    for layer in layers:
                        if layer in list(self.exportedLayers.keys()):
                            continue
                        else:
                            self.setSessionProjectionFromLayer(layer)
                            self.commands.append(self.exportVectorLayer(layer))

        # If projection has not already be set, use the project
        self.setSessionProjectionFromProject()

        # Build GRASS region
        if self.region.isEmpty():
            self.region = QgsProcessingUtils.combineLayerExtents(layers)
        command = 'g.region n={} s={} e={} w={}'.format(
            self.region.yMaximum(), self.region.yMinimum(),
            self.region.xMaximum(), self.region.xMinimum()
        )
        # TODO Handle cell size
        #if not self.cellSize:
        #    self.cellSize = self.getDefaultCellSize(parameters, context)
        #command += ' res={}'.format(self.cellSize)
        if self.alignToResolution:
            command += ' -a'

        # Add the default parameters commands
        self.commands.append(command)
        QgsMessageLog.logMessage('processInputs end. Commands: {}'.format(self.commands), 'Grass7', QgsMessageLog.INFO)

    def processCommand(self, parameters, context):
        """Prepare the GRASS algorithm command
        :param parameters:
        """
        noOutputs = [o for o in self.parameterDefinitions() if o not in self.destinationParameterDefinitions()]
        QgsMessageLog.logMessage('processCommand', 'Grass7', QgsMessageLog.INFO)
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
                             self.GRASS_REGION_ALIGN_TO_RESOLUTION]:
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

            # TODO: handle multipleLayers!
            #elif isinstance(param, QgsProcessingParameterMultipleLayers):
            #    s = param.value
            #    for layer in list(self.exportedLayers.keys()):
            #        s = s.replace(layer, self.exportedLayers[layer])
            #    s = s.replace(';', ',')
            #    command += ' ' + param.name() + '=' + s
            # For booleans, we just add the parameter name
            elif isinstance(param, QgsProcessingParameterBoolean):
                if self.parameterAsBool(parameters, paramName, context):
                    value = paramName
            # For enumeration, we need to grab the string value
            elif isinstance(param, QgsProcessingParameterEnum):
                idx = self.parameterAsEnum(parameters, paramName, context)
                value = '"{}"'.format(param.options()[idx])
            # For strings, we just translate as string
            elif isinstance(param, QgsProcessingParameterString):
                value = '"{}"'.format(
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
        for out in self.destinationParameterDefinitions():
            outName = out.name()
            if isinstance(out, QgsProcessingParameterFileDestination):
                command += ' > {}'.format(self.parameterAsFileOutput(parameters, outName, context))
            # TODO: handle OutputHTML
            #elif not isinstance(out, OutputHTML):
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
        
    def processOutputs(self, parameters, context):
        """Prepare the GRASS v.out.ogr commands"""
        # TODO: use outputDefinitions() or destionationParametersDefinitions() ?
        # TODO: support multiple raster formats.
        # TODO: support multiple vector formats.
        for out in self.destinationParameterDefinitions():
            outName = out.name()
            fileName = self.parameterAsOutputLayer(parameters, outName, context)
            uniqName = '{}{}'.format(outName, self.uniqueSuffix)
            if isinstance(out, QgsProcessingParameterRasterDestination):
                # Raster layer output: adjust region to layer before
                # exporting
                self.commands.append('g.region raster={}'.format(uniqName))
                self.outputCommands.append('g.region raster={}'.format(uniqName))
                # TODO: move this hack into ext/
                if self.grass7Name == 'r.statistics':
                    # r.statistics saves its results in a non-qgis compatible
                    # way. Post-process them with r.mapcalc.
                    calcExpression = 'correctedoutput' + self.uniqueSuffix
                    calcExpression += '=@{}'.format(uniqName)
                    command = 'r.mapcalc expression="' + calcExpression + '"'
                    self.commands.append(command)
                    self.outputCommands.append(command)

                    command = 'r.out.gdal --overwrite -c createopt="TFW=YES,COMPRESS=LZW"'
                    command += ' input='
                    command += 'correctedoutput' + self.uniqueSuffix
                    command += ' output="' + fileName + '"'
                else:
                    command = 'r.out.gdal --overwrite -c createopt="TFW=YES,COMPRESS=LZW"'
                    command += ' input='

                if self.grass7Name == 'r.horizon':
                    command += '{}_0'.format(uniqName)
                elif self.grass7Name == 'r.statistics':
                    self.commands.append(command)
                    self.outputCommands.append(command)
                else:
                    command += '{} output="{}"'.format(uniqName, fileName)
                    self.commands.append(command)
                    self.outputCommands.append(command)

            if isinstance(out, QgsProcessingParameterVectorDestination):
                typeidx = self.parameterAsEnum(parameters, self.GRASS_OUTPUT_TYPE_PARAMETER, context)
                outType = ('auto' if typeidx
                           is None else self.OUTPUT_TYPES[typeidx])
                # TODO: move this hack into ext/
                if self.grass7Name == 'r.flow':
                    command = 'v.out.ogr type=line layer=0 -s -e input={}'.format(uniqName)
                elif self.grass7Name == 'v.voronoi':
                    if '-l' in self.commands[-1]:
                        command = 'v.out.ogr type=line layer=0 -s -e input={}'.format(uniqName)
                    else:
                        command = 'v.out.ogr -s -e input={} type={}'.format(uniqName, outType)
                elif self.grass7Name == 'v.sample':
                    command = 'v.out.ogr type=point -s -e input={}'.format(uniqName)
                else:
                    command = 'v.out.ogr -s -e input={} type={}'.format(uniqName, outType)
                command += ' output="{}"'.format(os.path.dirname(fileName))
                command += ' format=ESRI_Shapefile'
                command += ' output_layer={} --overwrite'.format(os.path.basename(fileName)[:-4])
                self.commands.append(command)
                self.outputCommands.append(command)
        QgsMessageLog.logMessage('processOutputs. Commands: {}'.format(self.commands), 'Grass7', QgsMessageLog.INFO)
        
    def exportVectorLayer(self, layerKey, layerSrc):
        # TODO: learn about ProcessingContexts
        #context = dataobjects.createContext()

        # TODO: improve this. We are now exporting if it is not a shapefile,
        # but the functionality of v.in.ogr could be used for this.
        # We also export if there is a selection
        #if not os.path.exists(orgFilename) or not orgFilename.endswith('shp'):
        #    layer = QgsProcessingUtils.mapLayerFromString(orgFilename, context, False)
        #    if layer:
        #        filename = dataobjects.exportVectorLayer(layer)
        #else:
        #    layer = QgsProcessingUtils.mapLayerFromString(orgFilename, context, False)
        #    if layer:
        #        #TODO
        #        #useSelection = \
        #        #    ProcessingConfig.getSetting(ProcessingConfig.USE_SELECTED)
        #        if useSelection and layer.selectedFeatureCount() != 0:
        #            filename = dataobjects.exportVectorLayer(layer)
        #        else:
        #            filename = orgFilename
        #    else:
        #        filename = orgFilename
        # TODO handle selection with a where_clause?
        # TODO use v.external as an option!
        destFileName = 'a' + os.path.basename(self.getTempFilename())
        self.exportedLayers[layerKey] = destFileName
        layerFileName= os.path.basename(layerSrc)
        layerName = os.path.splitext(layerFileName)[0]
        command = 'v.in.ogr min_area={0} snap={1} input="{2}" layer={3} output={4} --overwrite -o'.format(
            self.minArea, self.snapTolerance,
            os.path.dirname(layerSrc),
            layerName,
            destFileName)
        return command

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
        context = dataobjects.createContext()
        if not Grass7Utils.projectionSet:
            proj4 = str(layer.crs().toProj4())
            command = 'g.proj -c proj4="{}"'.format(proj4)
            self.commands.append(command)
            Grass7Utils.projectionSet = True

    def exportRasterLayer(self, layerKey, layerSrc):
        """
        Creates a dedicated command to load a raster into
        temporary GRASS DB.
        """
        # TODO: handle multiple bands
        destFilename = 'a' + os.path.basename(self.getTempFilename())
        self.exportedLayers[layerKey] = destFilename
        command = 'r.external input="{}" band=1 output={} --overwrite -o'.format(
            layerSrc, destFilename)
        return command

    def getTempFilename(self):
        # TODO Replace with QgsProcessingUtils generateTempFilename
        return system.getTempFilename()

    def canExecute(self):
        message = Grass7Utils.checkGrass7IsInstalled()
        return not message, message

    def checkParameterValues(self, parameters, context):
        if self.module:
            if hasattr(self.module, 'checkParameterValuesBeforeExecuting'):
                func = getattr(self.module, 'checkParameterValuesBeforeExecuting')
                return func(self), None
        return super(Grass7Algorithm, self).checkParameterValues(parameters, context)
