# -*- coding: utf-8 -*-

"""
***************************************************************************
    SagaAlgorithm.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
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
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

import os
import shutil
import importlib
from qgis.core import (Qgis,
                       QgsApplication,
                       QgsProcessingUtils,
                       QgsProcessingException,
                       QgsMessageLog,
                       QgsProcessing,
                       QgsProcessingAlgorithm,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterMultipleLayers,
                       QgsProcessingParameterMatrix,
                       QgsProcessingParameterString,
                       QgsProcessingParameterField,
                       QgsProcessingParameterFile,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterRasterDestination,
                       QgsProcessingParameterVectorDestination)
from processing.core.ProcessingConfig import ProcessingConfig
from processing.tools.system import getTempFilename
from .SagaNameDecorator import decoratedAlgorithmName, decoratedGroupName
from .SagaParameters import Parameters
from . import SagaUtils
from .SagaAlgorithmBase import SagaAlgorithmBase
from .shorthelp import shortHelp

pluginPath = os.path.normpath(os.path.join(
    os.path.split(os.path.dirname(__file__))[0], os.pardir))

sessionExportedLayers = {}


class SagaAlgorithm(SagaAlgorithmBase):
    OUTPUT_EXTENT = 'OUTPUT_EXTENT'

    def __init__(self, descriptionfile):
        super().__init__()
        self.hardcoded_strings = []
        self.allow_nonmatching_grid_extents = False
        self.description_file = descriptionfile
        self.undecorated_group = None
        self._name = ''
        self._display_name = ''
        self._group = ''
        self._groupId = ''
        self.params = []
        self.known_issues = False
        self.defineCharacteristicsFromFile()

    def createInstance(self):
        return SagaAlgorithm(self.description_file)

    def initAlgorithm(self, config=None):
        for p in self.params:
            self.addParameter(p)

    def name(self):
        return self._name

    def displayName(self):
        return self._display_name

    def group(self):
        return self._group

    def groupId(self):
        return self._groupId

    def shortHelpString(self):
        return shortHelp.get(self.id(), None)

    def icon(self):
        return QgsApplication.getThemeIcon("/providerSaga.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerSaga.svg")

    def flags(self):
        # TODO - maybe it's safe to background thread this?
        f = super().flags() | QgsProcessingAlgorithm.FlagNoThreading
        if self.known_issues:
            f = f | QgsProcessingAlgorithm.FlagKnownIssues
        return f

    def defineCharacteristicsFromFile(self):
        with open(self.description_file, encoding="utf-8") as lines:
            line = lines.readline().strip('\n').strip()

            self._name = line
            if '|' in self._name:
                tokens = self._name.split('|')
                self._name = tokens[0]
                # cmdname is the name of the algorithm in SAGA, that is, the name to use to call it in the console
                self.cmdname = tokens[1]

            else:
                self.cmdname = self._name
                self._display_name = self.tr(str(self._name))
            self._name = decoratedAlgorithmName(self._name)
            self._display_name = self.tr(str(self._name))

            self._name = self._name.lower()
            validChars = \
                'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:'
            self._name = ''.join(c for c in self._name if c in validChars)

            line = lines.readline().strip('\n').strip()
            if line == '##known_issues':
                self.known_issues = True
                line = lines.readline().strip('\n').strip()

            self.undecorated_group = line
            self._group = self.tr(decoratedGroupName(self.undecorated_group))

            validChars = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:'
            grpName = decoratedGroupName(self.undecorated_group).lower()
            self._groupId = ''.join(c for c in grpName if c in validChars)
            line = lines.readline().strip('\n').strip()
            while line != '':
                if line.startswith('Hardcoded'):
                    self.hardcoded_strings.append(line[len('Hardcoded|'):])
                elif Parameters.is_parameter_line(line):
                    self.params.append(Parameters.create_parameter_from_line(line))
                elif line.startswith('AllowUnmatching'):
                    self.allow_nonmatching_grid_extents = True
                else:
                    pass  # TODO
                    # self.addOutput(getOutputFromString(line))
                line = lines.readline().strip('\n').strip()

    def processAlgorithm(self, parameters, context, feedback):
        commands = []
        self.exportedLayers = {}

        self.preProcessInputs()
        extent = None
        crs = None

        # 1: Export rasters to sgrd and vectors to shp
        # Tables must be in dbf format. We check that.
        for param in self.parameterDefinitions():
            if isinstance(param, QgsProcessingParameterRasterLayer):
                if param.name() not in parameters or parameters[param.name()] is None:
                    continue

                if isinstance(parameters[param.name()], str):
                    if parameters[param.name()].lower().endswith('sdat'):
                        self.exportedLayers[param.name()] = parameters[param.name()][:-4] + 'sgrd'
                    elif parameters[param.name()].lower().endswith('sgrd'):
                        self.exportedLayers[param.name()] = parameters[param.name()]
                    else:
                        layer = self.parameterAsRasterLayer(parameters, param.name(), context)
                        exportCommand = self.exportRasterLayer(param.name(), layer)
                        if exportCommand is not None:
                            commands.append(exportCommand)
                else:
                    if parameters[param.name()].source().lower().endswith('sdat'):
                        self.exportedLayers[param.name()] = parameters[param.name()].source()[:-4] + 'sgrd'
                    if parameters[param.name()].source().lower().endswith('sgrd'):
                        self.exportedLayers[param.name()] = parameters[param.name()].source()
                    else:
                        exportCommand = self.exportRasterLayer(param.name(), parameters[param.name()])
                        if exportCommand is not None:
                            commands.append(exportCommand)
            elif isinstance(param, QgsProcessingParameterFeatureSource):
                if param.name() not in parameters or parameters[param.name()] is None:
                    continue

                if not crs:
                    source = self.parameterAsSource(parameters, param.name(), context)
                    if source is None:
                        raise QgsProcessingException(self.invalidSourceError(parameters, param.name()))

                    crs = source.sourceCrs()

                layer_path = self.parameterAsCompatibleSourceLayerPath(parameters, param.name(), context, ['shp'], 'shp', feedback=feedback)
                if layer_path:
                    self.exportedLayers[param.name()] = layer_path
                else:
                    raise QgsProcessingException(
                        self.tr('Unsupported file format'))
            elif isinstance(param, QgsProcessingParameterMultipleLayers):
                if param.name() not in parameters or parameters[param.name()] is None:
                    continue

                layers = self.parameterAsLayerList(parameters, param.name(), context)
                if layers is None or len(layers) == 0:
                    continue
                if param.layerType() == QgsProcessing.TypeRaster:
                    files = []
                    for i, layer in enumerate(layers):
                        if layer.source().lower().endswith('sdat'):
                            files.append(layer.source()[:-4] + 'sgrd')
                        elif layer.source().lower().endswith('sgrd'):
                            files.append(layer.source())
                        else:
                            exportCommand = self.exportRasterLayer(param.name(), layer)
                            files.append(self.exportedLayers[param.name()])
                            if exportCommand is not None:
                                commands.append(exportCommand)

                    self.exportedLayers[param.name()] = files
                else:
                    for layer in layers:
                        temp_params = {param.name(): layer}
                        if not crs:
                            source = self.parameterAsSource(temp_params, param.name(), context)
                            if source is None:
                                raise QgsProcessingException(self.invalidSourceError(parameters, param.name()))

                            crs = source.sourceCrs()

                        layer_path = self.parameterAsCompatibleSourceLayerPath(temp_params, param.name(), context, ['shp'], 'shp',
                                                                               feedback=feedback)
                        if layer_path:
                            if param.name() in self.exportedLayers:
                                self.exportedLayers[param.name()].append(layer_path)
                            else:
                                self.exportedLayers[param.name()] = [layer_path]
                        else:
                            raise QgsProcessingException(
                                self.tr('Unsupported file format'))

        # 2: Set parameters and outputs
        command = self.undecorated_group + ' "' + self.cmdname + '"'
        command += ' ' + ' '.join(self.hardcoded_strings)

        for param in self.parameterDefinitions():
            if param.name() not in parameters or parameters[param.name()] is None:
                continue
            if param.isDestination():
                continue

            if isinstance(param, (QgsProcessingParameterRasterLayer, QgsProcessingParameterFeatureSource)):
                command += ' -{} "{}"'.format(param.name(), self.exportedLayers[param.name()])
            elif isinstance(param, QgsProcessingParameterMultipleLayers):
                if parameters[param.name()]:  # parameter may have been an empty list
                    command += ' -{} "{}"'.format(param.name(), ';'.join(self.exportedLayers[param.name()]))
            elif isinstance(param, QgsProcessingParameterBoolean):
                if self.parameterAsBoolean(parameters, param.name(), context):
                    command += ' -{} true'.format(param.name().strip())
                else:
                    command += ' -{} false'.format(param.name().strip())
            elif isinstance(param, QgsProcessingParameterMatrix):
                tempTableFile = getTempFilename('txt')
                with open(tempTableFile, 'w') as f:
                    f.write('\t'.join(param.headers()) + '\n')
                    values = self.parameterAsMatrix(parameters, param.name(), context)
                    for i in range(0, len(values), 3):
                        s = '{}\t{}\t{}\n'.format(values[i], values[i + 1], values[i + 2])
                        f.write(s)
                command += ' -{} "{}"'.format(param.name(), tempTableFile)
            elif isinstance(param, QgsProcessingParameterExtent):
                # 'We have to subtract/add half cell size, since SAGA is
                # center based, not corner based
                halfcell = self.getOutputCellsize(parameters, context) / 2
                offset = [halfcell, -halfcell, halfcell, -halfcell]
                rect = self.parameterAsExtent(parameters, param.name(), context)

                values = [
                    rect.xMinimum(),
                    rect.xMaximum(),
                    rect.yMinimum(),
                    rect.yMaximum(),
                ]

                for i in range(4):
                    command += ' -{} {}'.format(param.name().split(' ')[i], float(values[i]) + offset[i])
            elif isinstance(param, QgsProcessingParameterNumber):
                if param.dataType() == QgsProcessingParameterNumber.Integer:
                    command += ' -{} {}'.format(param.name(), self.parameterAsInt(parameters, param.name(), context))
                else:
                    command += ' -{} {}'.format(param.name(), self.parameterAsDouble(parameters, param.name(), context))
            elif isinstance(param, QgsProcessingParameterEnum):
                command += ' -{} {}'.format(param.name(), self.parameterAsEnum(parameters, param.name(), context))
            elif isinstance(param, (QgsProcessingParameterString, QgsProcessingParameterFile)):
                command += ' -{} "{}"'.format(param.name(), self.parameterAsFile(parameters, param.name(), context))
            elif isinstance(param, (QgsProcessingParameterString, QgsProcessingParameterField)):
                command += ' -{} "{}"'.format(param.name(), self.parameterAsString(parameters, param.name(), context))

        output_layers = []
        output_files = {}
        # If the user has entered an output file that has non-ascii chars, we use a different path with only ascii chars
        output_files_nonascii = {}
        for out in self.destinationParameterDefinitions():
            filePath = self.parameterAsOutputLayer(parameters, out.name(), context)
            if isinstance(out, (QgsProcessingParameterRasterDestination, QgsProcessingParameterVectorDestination)):
                output_layers.append(filePath)
                try:
                    filePath.encode('ascii')
                except UnicodeEncodeError:
                    nonAsciiFilePath = filePath
                    filePath = QgsProcessingUtils.generateTempFilename(out.name() + os.path.splitext(filePath)[1])
                    output_files_nonascii[filePath] = nonAsciiFilePath

            output_files[out.name()] = filePath
            command += ' -{} "{}"'.format(out.name(), filePath)
        commands.append(command)

        # special treatment for RGB algorithm
        # TODO: improve this and put this code somewhere else
        if self.cmdname == 'RGB Composite':
            for out in self.destinationParameterDefinitions():
                if isinstance(out, QgsProcessingParameterRasterDestination):
                    filename = self.parameterAsOutputLayer(parameters, out.name(), context)
                    filename2 = os.path.splitext(filename)[0] + '.sgrd'
                    commands.append('io_grid_image 0 -COLOURING 4 -GRID:"{}" -FILE:"{}"'.format(filename2, filename))

        # 3: Run SAGA
        commands = self.editCommands(commands)
        SagaUtils.createSagaBatchJobFileFromSagaCommands(commands)
        loglines = [self.tr('SAGA execution commands')]
        for line in commands:
            feedback.pushCommandInfo(line)
            loglines.append(line)
        if ProcessingConfig.getSetting(SagaUtils.SAGA_LOG_COMMANDS):
            QgsMessageLog.logMessage('\n'.join(loglines), self.tr('Processing'), Qgis.Info)
        SagaUtils.executeSaga(feedback)

        if crs is not None:
            for out in output_layers:
                prjFile = os.path.splitext(out)[0] + '.prj'
                with open(prjFile, 'wt', encoding='utf-8') as f:
                    f.write(crs.toWkt())

        for old, new in output_files_nonascii.items():
            oldFolder = os.path.dirname(old)
            newFolder = os.path.dirname(new)
            newName = os.path.splitext(os.path.basename(new))[0]
            files = [f for f in os.listdir(oldFolder)]
            for f in files:
                ext = os.path.splitext(f)[1]
                newPath = os.path.join(newFolder, newName + ext)
                oldPath = os.path.join(oldFolder, f)
                shutil.move(oldPath, newPath)

        return {
            o.name(): output_files[o.name()]
            for o in self.outputDefinitions()
            if o.name() in output_files
        }

    def preProcessInputs(self):
        name = self.name().replace('.', '_')
        try:
            module = importlib.import_module('processing.algs.saga.ext.' + name)
        except ImportError:
            return
        if hasattr(module, 'preProcessInputs'):
            func = getattr(module, 'preProcessInputs')
            func(self)

    def editCommands(self, commands):
        try:
            module = importlib.import_module('processing.algs.saga.ext.' + self.name())
        except ImportError:
            return commands
        if hasattr(module, 'editCommands'):
            func = getattr(module, 'editCommands')
            return func(commands)
        else:
            return commands

    def getOutputCellsize(self, parameters, context):
        """Tries to guess the cell size of the output, searching for
        a parameter with an appropriate name for it.
        :param parameters:
        """

        cellsize = 0
        for param in self.parameterDefinitions():
            if param.name() in parameters and param.name() == 'USER_SIZE':
                cellsize = self.parameterAsDouble(parameters, param.name(), context)
                break
        return cellsize

    def exportRasterLayer(self, parameterName, layer):
        global sessionExportedLayers
        if layer.source() in sessionExportedLayers:
            exportedLayer = sessionExportedLayers[layer.source()]
            if os.path.exists(exportedLayer):
                self.exportedLayers[parameterName] = exportedLayer
                return None
            else:
                del sessionExportedLayers[layer.source()]

        if layer:
            filename = layer.name()
        else:
            filename = os.path.basename(layer.source())

        validChars = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:'
        filename = ''.join(c for c in filename if c in validChars)

        if len(filename) == 0:
            filename = 'layer'

        destFilename = QgsProcessingUtils.generateTempFilename(filename + '.sgrd')
        sessionExportedLayers[layer.source()] = destFilename
        self.exportedLayers[parameterName] = destFilename

        return 'io_gdal 0 -TRANSFORM 1 -RESAMPLING 3 -GRIDS "{}" -FILES "{}"'.format(destFilename, layer.source())

    def checkParameterValues(self, parameters, context):
        """
        We check that there are no multiband layers, which are not
        supported by SAGA, and that raster layers have the same grid extent
        """
        extent = None
        raster_layer_params = []
        for param in self.parameterDefinitions():
            if param not in parameters or parameters[param.name()] is None:
                continue

            if isinstance(param, QgsProcessingParameterRasterLayer):
                raster_layer_params.append(param.name())
            elif (isinstance(param, QgsProcessingParameterMultipleLayers)
                    and param.layerType() == QgsProcessing.TypeRaster):
                raster_layer_params.extend(param.name())

        for layer_param in raster_layer_params:
            layer = self.parameterAsRasterLayer(parameters, layer_param, context)

            if layer is None:
                continue
            if layer.bandCount() > 1:
                return False, self.tr('Input layer {0} has more than one band.\n'
                                      'Multiband layers are not supported by SAGA').format(layer.name())
            if not self.allow_nonmatching_grid_extents:
                if extent is None:
                    extent = (layer.extent(), layer.height(), layer.width())
                else:
                    extent2 = (layer.extent(), layer.height(), layer.width())
                    if extent != extent2:
                        return False, self.tr("Input layers do not have the same grid extent.")
        return super(SagaAlgorithm, self).checkParameterValues(parameters, context)
