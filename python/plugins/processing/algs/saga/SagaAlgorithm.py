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
from processing.gui.Help2Html import getHtmlFromRstFile

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import importlib
from qgis.core import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.ProcessingLog import ProcessingLog
from processing.core.GeoAlgorithmExecutionException import \
        GeoAlgorithmExecutionException
from processing.core.parameters import *
from processing.core.outputs import *
from SagaUtils import SagaUtils
from SagaGroupNameDecorator import SagaGroupNameDecorator
from processing.tools import dataobjects
from processing.tools.system import *

sessionExportedLayers = {}


class SagaAlgorithm(GeoAlgorithm):

    OUTPUT_EXTENT = 'OUTPUT_EXTENT'

    def __init__(self, descriptionfile):
        GeoAlgorithm.__init__(self)
        self.hardcodedStrings = []
        self.allowUnmatchingGridExtents = False
        self.descriptionFile = descriptionfile
        self.defineCharacteristicsFromFile()

    def getCopy(self):
        newone = SagaAlgorithm(self.descriptionFile)
        newone.provider = self.provider
        return newone

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + '/../../images/saga.png')

    def defineCharacteristicsFromFile(self):
        lines = open(self.descriptionFile)
        line = lines.readline().strip('\n').strip()
        self.name = line
        if '|' in self.name:
            tokens = self.name.split('|')
            self.name = tokens[0]
            self.cmdname = tokens[1]
        else:
            self.cmdname = self.name
            self.name = self.name[0].upper() + self.name[1:].lower()
        line = lines.readline().strip('\n').strip()
        self.undecoratedGroup = line
        self.group = SagaGroupNameDecorator.getDecoratedName(
                self.undecoratedGroup)
        line = lines.readline().strip('\n').strip()
        while line != '':
            if line.startswith('Hardcoded'):
                self.hardcodedStrings.append(line[len('Harcoded|') + 1:])
            elif line.startswith('Parameter'):
                self.addParameter(getParameterFromString(line))
            elif line.startswith('AllowUnmatching'):
                self.allowUnmatchingGridExtents = True
            elif line.startswith('Extent'):
                # An extent parameter that wraps 4 SAGA numerical parameters
                self.extentParamNames = line[6:].strip().split(' ')
                self.addParameter(ParameterExtent(self.OUTPUT_EXTENT,
                                  'Output extent', '0,1,0,1'))
            else:
                self.addOutput(getOutputFromString(line))
            line = lines.readline().strip('\n').strip()
        lines.close()


    def processAlgorithm(self, progress):
        if isWindows():
            path = SagaUtils.sagaPath()
            if path == '':
                raise GeoAlgorithmExecutionException(
                        'SAGA folder is not configured.\nPlease configure \
                        it before running SAGA algorithms.')
        commands = list()
        self.exportedLayers = {}

        self.preProcessInputs()

        # 1: Export rasters to sgrd and vectors to shp
        # Tables must be in dbf format. We check that.
        for param in self.parameters:
            if isinstance(param, ParameterRaster):
                if param.value is None:
                    continue
                value = param.value
                if not value.endswith('sgrd'):
                    exportCommand = self.exportRasterLayer(value)
                    if exportCommand is not None:
                        commands.append(exportCommand)
            if isinstance(param, ParameterVector):
                if param.value is None:
                    continue
                layer = dataobjects.getObjectFromUri(param.value, False)
                if layer:
                    filename = dataobjects.exportVectorLayer(layer)
                    self.exportedLayers[param.value] = filename
                elif not param.value.endswith('shp'):
                    raise GeoAlgorithmExecutionException(
                            'Unsupported file format')
            if isinstance(param, ParameterTable):
                if param.value is None:
                    continue
                table = dataobjects.getObjectFromUri(param.value, False)
                if table:
                    filename = dataobjects.exportTable(table)
                    self.exportedLayers[param.value] = filename
                elif not param.value.endswith('shp'):
                    raise GeoAlgorithmExecutionException(
                            'Unsupported file format')
            if isinstance(param, ParameterMultipleInput):
                if param.value is None:
                    continue
                layers = param.value.split(';')
                if layers is None or len(layers) == 0:
                    continue
                if param.datatype == ParameterMultipleInput.TYPE_RASTER:
                    for layerfile in layers:
                        if not layerfile.endswith('sgrd'):
                            exportCommand = self.exportRasterLayer(layerfile)
                            if exportCommand is not None:
                                commands.append(exportCommand)
                elif param.datatype == ParameterMultipleInput.TYPE_VECTOR_ANY:
                    for layerfile in layers:
                        layer = dataobjects.getObjectFromUri(layerfile, False)
                        if layer:
                            filename = dataobjects.exportVectorLayer(layer)
                            self.exportedLayers[layerfile] = filename
                        elif not layerfile.endswith('shp'):
                            raise GeoAlgorithmExecutionException(
                                    'Unsupported file format')

        # 2: Set parameters and outputs
        saga208 = SagaUtils.isSaga208()
        if isWindows() or isMac() or not saga208:
            command = self.undecoratedGroup + ' "' + self.cmdname + '"'
        else:
            command = 'lib' + self.undecoratedGroup + ' "' + self.cmdname + '"'

        if self.hardcodedStrings:
            for s in self.hardcodedStrings:
                command += ' ' + s

        for param in self.parameters:
            if param.value is None:
                continue
            if isinstance(param, (ParameterRaster, ParameterVector,
                          ParameterTable)):
                value = param.value
                if value in self.exportedLayers.keys():
                    command += ' -' + param.name + ' "' \
                        + self.exportedLayers[value] + '"'
                else:
                    command += ' -' + param.name + ' "' + value + '"'
            elif isinstance(param, ParameterMultipleInput):
                s = param.value
                for layer in self.exportedLayers.keys():
                    s = s.replace(layer, self.exportedLayers[layer])
                command += ' -' + param.name + ' "' + s + '"'
            elif isinstance(param, ParameterBoolean):
                if param.value:
                    command += ' -' + param.name
            elif isinstance(param, ParameterFixedTable):
                tempTableFile = getTempFilename('txt')
                f = open(tempTableFile, 'w')
                f.write('\t'.join([col for col in param.cols]) + '\n')
                values = param.value.split(',')
                for i in range(0, len(values), 3):
                    s = values[i] + '\t' + values[i + 1] + '\t' + values[i
                            + 2] + '\n'
                    f.write(s)
                f.close()
                command += ' -' + param.name + ' "' + tempTableFile + '"'
            elif isinstance(param, ParameterExtent):
                # 'We have to substract/add half cell size, since SAGA is
                # center based, not corner based
                halfcell = self.getOutputCellsize() / 2
                offset = [halfcell, -halfcell, halfcell, -halfcell]
                values = param.value.split(',')
                for i in range(4):
                    command += ' -' + self.extentParamNames[i] + ' ' \
                        + str(float(values[i]) + offset[i])
            elif isinstance(param, (ParameterNumber, ParameterSelection)):
                command += ' -' + param.name + ' ' + str(param.value)
            else:
                command += ' -' + param.name + ' "' + str(param.value) + '"'

        for out in self.outputs:
            if isinstance(out, OutputRaster):
                filename = out.getCompatibleFileName(self)
                filename += '.sgrd'
                command += ' -' + out.name + ' "' + filename + '"'
            if isinstance(out, OutputVector):
                filename = out.getCompatibleFileName(self)
                command += ' -' + out.name + ' "' + filename + '"'
            if isinstance(out, OutputTable):
                filename = out.getCompatibleFileName(self)
                command += ' -' + out.name + ' "' + filename + '"'

        commands.append(command)

        # 3: Export resulting raster layers
        optim = ProcessingConfig.getSetting(
                SagaUtils.SAGA_IMPORT_EXPORT_OPTIMIZATION)
        for out in self.outputs:
            if isinstance(out, OutputRaster):
                filename = out.getCompatibleFileName(self)
                filename2 = filename + '.sgrd'
                formatIndex = (4 if not saga208 and isWindows() else 1)
                sessionExportedLayers[filename] = filename2
                dontExport = True

                # Do not export is the output is not a final output
                # of the model
                #if self.model is not None and optim:
                #    for subalg in self.model.algOutputs:
                #        if out.name in subalg:
                #            if subalg[out.name] is not None:
                #                dontExport = False
                #                break
                #    if dontExport:
                #        continue

                if self.cmdname == 'RGB Composite':
                    if isWindows() or isMac() or not saga208:
                        commands.append('io_grid_image 0 -IS_RGB -GRID:"' + filename2
                                	+ '" -FILE:"' + filename
                                	+ '"')
                    else:
                        commands.append('libio_grid_image 0 -IS_RGB -GRID:"' + filename2
                                	+ '" -FILE:"' + filename
                                	+ '"')
                else:
                    if isWindows() or isMac() or not saga208:
                        commands.append('io_gdal 1 -GRIDS "' + filename2
	                                    + '" -FORMAT ' + str(formatIndex)
	                                    + ' -TYPE 0 -FILE "' + filename + '"')
                    else:
                        commands.append('libio_gdal 1 -GRIDS "' + filename2
	                                    + '" -FORMAT 1 -TYPE 0 -FILE "' + filename
	                                    + '"')

        # 4: Run SAGA
        commands = self.editCommands(commands)
        SagaUtils.createSagaBatchJobFileFromSagaCommands(commands)
        loglines = []
        loglines.append('SAGA execution commands')
        for line in commands:
            progress.setCommand(line)
            loglines.append(line)
        if ProcessingConfig.getSetting(SagaUtils.SAGA_LOG_COMMANDS):
            ProcessingLog.addToLog(ProcessingLog.LOG_INFO, loglines)
        SagaUtils.executeSaga(progress)

    def preProcessInputs(self):
        name = self.commandLineName().replace('.', '_')[len('saga:'):]
        try:
            module = importlib.import_module('processing.algs.saga.ext.' + name)
        except ImportError:
            return
        if hasattr(module, 'preProcessInputs'):
            func = getattr(module, 'preProcessInputs')
            func(self)

    def editCommands(self, commands):
        name = self.commandLineName()[len('saga:'):]
        try:
            module = importlib.import_module('processing.algs.saga.ext.' + name)
        except ImportError:
            return commands
        if hasattr(module, 'editCommands'):
            func = getattr(module, 'editCommands')
            return func(commands)
        else:
            return commands

    def getOutputCellsize(self):
        """Tries to guess the cellsize of the output, searching for
        a parameter with an appropriate name for it.
        """

        cellsize = 0
        for param in self.parameters:
            if param.value is not None and param.name == 'USER_SIZE':
                cellsize = float(param.value)
                break
        return cellsize


    def exportRasterLayer(self, source):
        global sessionExportedLayers
        if source in sessionExportedLayers:
            self.exportedLayers[source] = sessionExportedLayers[source]
            return None
        layer = dataobjects.getObjectFromUri(source, False)
        if layer:
            filename = str(layer.name())
        else:
            filename = os.path.basename(source)
        validChars = \
            'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:'
        filename = ''.join(c for c in filename if c in validChars)
        if len(filename) == 0:
            filename = 'layer'
        destFilename = getTempFilenameInTempFolder(filename + '.sgrd')
        self.exportedLayers[source] = destFilename
        sessionExportedLayers[source] = destFilename
        saga208 = SagaUtils.isSaga208()
        if saga208:
            if isWindows() or isMac():
                return 'io_gdal 0 -GRIDS "' + destFilename + '" -FILES "' + source \
					+ '"'
            else:
                return 'libio_gdal 0 -GRIDS "' + destFilename + '" -FILES "' \
					+ source + '"'
        else:
            return 'io_gdal 0 -TRANSFORM -INTERPOL 0 -GRIDS "' + destFilename + '" -FILES "' + source \
                + '"'

    def checkBeforeOpeningParametersDialog(self):
        msg = SagaUtils.checkSagaIsInstalled()
        if msg is not None:
            print msg
            html = '<p>This algorithm requires SAGA to be run.Unfortunately, \
                   it seems that SAGA is not installed in your system, or it \
                   is not correctly configured to be used from QGIS</p>'
            html += '<p><a href= "http://docs.qgis.org/2.0/en/docs/user_manual/processing/3rdParty.html">\
                    Click here</a> to know more about how to install and configure SAGA to be used with QGIS</p>'
            return html

    def checkParameterValuesBeforeExecuting(self):
        """
        We check that there are no multiband layers, which are not
        supported by SAGA, and that raster layers have the same grid extent
        """
        extent = None
        for param in self.parameters:
            files = []
            if isinstance(param, ParameterRaster):
                files = [param.value]
            elif isinstance(param, ParameterMultipleInput) and param.datatype == ParameterMultipleInput.TYPE_RASTER:
                if param.value is not None:
                    files = param.value.split(";")
            for f in files:
                layer = dataobjects.getObjectFromUri(f)
                if layer is None:
                    continue
                if layer.bandCount() > 1:
                    return 'Input layer ' + str(layer.name()) \
                        + ' has more than one band.\n' \
                        + 'Multiband layers are not supported by SAGA'
                if not self.allowUnmatchingGridExtents:
                    if extent is None:
                        extent = (layer.extent(), layer.height(), layer.width())
                    else:
                        extent2 = (layer.extent(), layer.height(), layer.width())
                        if extent != extent2:
                            return "Input layers do not have the same grid extent."



    def help(self):
        name = self.cmdname.lower()
        validChars = 'abcdefghijklmnopqrstuvwxyz'
        name = ''.join(c for c in name if c in validChars)
        html = getHtmlFromRstFile(os.path.join(os.path.dirname(__file__), 'help',
                            name + '.rst'))
        if html is None:
            return True, None
        imgpath = os.path.join(os.path.dirname(__file__),os.pardir, os.pardir, 'images', 'saga100x100.jpg')
        html = ('<img src="%s"/>' % imgpath) + html
        return True, html

    def getPostProcessingErrorMessage(self, wrongLayers):
        html = GeoAlgorithm.getPostProcessingErrorMessage(self, wrongLayers)
        msg = SagaUtils.checkSagaIsInstalled(True)
        html += '<p>This algorithm requires SAGA to be run. A test to check \
                 if SAGA is correctly installed and configured in your system \
                 has been performed, with the following result:</p><ul><i>'
        if msg is None:
            html += 'SAGA seems to be correctly installed and \
                     configured</li></ul>'
        else:
            html += msg + '</i></li></ul>'
            html += '<p><a href= "http://docs.qgis.org/2.0/en/docs/user_manual/processing/3rdParty.html">Click here</a> to know more about how to install and configure SAGA to be used with QGIS</p>'

        return html
