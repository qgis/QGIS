# -*- coding: utf-8 -*-

"""
***************************************************************************
    RAlgorithm.py
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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import subprocess
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtGui
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import \
        GeoAlgorithmExecutionException
from processing.core.ProcessingLog import ProcessingLog
from processing.gui.Help2Html import getHtmlFromHelpFile
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterTable
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterMultipleInput
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterTableField
from processing.core.parameters import ParameterExtent
from processing.core.parameters import ParameterFile
from processing.core.outputs import OutputTable
from processing.core.outputs import OutputVector
from processing.core.outputs import OutputRaster
from processing.core.outputs import OutputHTML
from processing.core.outputs import OutputFile
from processing.tools.system import *
from processing.script.WrongScriptException import WrongScriptException
from RUtils import RUtils


class RAlgorithm(GeoAlgorithm):

    R_CONSOLE_OUTPUT = 'R_CONSOLE_OUTPUT'
    RPLOTS = 'RPLOTS'

    def getCopy(self):
        newone = RAlgorithm(self.descriptionFile)
        newone.provider = self.provider
        return newone

    def __init__(self, descriptionFile, script=None):
        GeoAlgorithm.__init__(self)
        self.script = script
        self.descriptionFile = descriptionFile
        if script is not None:
            self.defineCharacteristicsFromScript()
        if descriptionFile is not None:
            self.defineCharacteristicsFromFile()

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + '/../../images/r.png')

    def defineCharacteristicsFromScript(self):
        lines = self.script.split('\n')
        self.name = '[Unnamed algorithm]'
        self.group = 'User R scripts'
        self.parseDescription(iter(lines))

    def defineCharacteristicsFromFile(self):
        filename = os.path.basename(self.descriptionFile)
        self.name = filename[:filename.rfind('.')].replace('_', ' ')
        self.group = 'User R scripts'
        with open(self.descriptionFile, 'r') as f:
            lines = [line.strip() for line in f]
        self.parseDescription(iter(lines))

    def parseDescription(self, lines):
        self.script = ''
        self.commands = []
        self.showPlots = False
        self.showConsoleOutput = False
        self.useRasterPackage = True
        self.passFileNames = False
        self.verboseCommands = []
        ender = 0
        line = lines.next().strip('\n').strip('\r')
        while ender < 10:
            if line.startswith('##'):
                try:
                    self.processParameterLine(line)
                except Exception:
                    raise WrongScriptException(
                        self.tr('Could not load R script: %s.\n Problem with line %s' % (self.descriptionFile, line)))
            elif line.startswith('>'):
                self.commands.append(line[1:])
                self.verboseCommands.append(line[1:])
                if not self.showConsoleOutput:
                    self.addOutput(OutputHTML(RAlgorithm.R_CONSOLE_OUTPUT,
                        self.tr('R Console Output')))
                self.showConsoleOutput = True
            else:
                if line == '':
                    ender += 1
                else:
                    ender = 0
                self.commands.append(line)
            self.script += line + '\n'
            try:
                line = lines.next().strip('\n').strip('\r')
            except:
                break

    def getVerboseCommands(self):
        return self.verboseCommands

    def createDescriptiveName(self, s):
        return s.replace('_', ' ')

    def processParameterLine(self, line):
        param = None
        out = None
        line = line.replace('#', '')
        if line.lower().strip().startswith('showplots'):
            self.showPlots = True
            self.addOutput(OutputHTML(RAlgorithm.RPLOTS, 'R Plots'))
            return
        if line.lower().strip().startswith('dontuserasterpackage'):
            self.useRasterPackage = False
            return
        if line.lower().strip().startswith('passfilenames'):
            self.passFileNames = True
            return
        tokens = line.split('=')
        desc = self.createDescriptiveName(tokens[0])
        if tokens[1].lower().strip() == 'group':
            self.group = tokens[0]
            return
        if tokens[1].lower().strip().startswith('raster'):
            param = ParameterRaster(tokens[0], desc, False)
        elif tokens[1].lower().strip() == 'vector':
            param = ParameterVector(tokens[0], desc,
                                    [ParameterVector.VECTOR_TYPE_ANY])
        elif tokens[1].lower().strip() == 'table':
            param = ParameterTable(tokens[0], desc, False)
        elif tokens[1].lower().strip().startswith('multiple raster'):
            param = ParameterMultipleInput(tokens[0], desc,
                    ParameterMultipleInput.TYPE_RASTER)
            param.optional = False
        elif tokens[1].lower().strip() == 'multiple vector':
            param = ParameterMultipleInput(tokens[0], desc,
                    ParameterMultipleInput.TYPE_VECTOR_ANY)
            param.optional = False
        elif tokens[1].lower().strip().startswith('selection'):
            options = tokens[1].strip()[len('selection'):].split(';')
            param = ParameterSelection(tokens[0], desc, options)
        elif tokens[1].lower().strip().startswith('boolean'):
            default = tokens[1].strip()[len('boolean') + 1:]
            param = ParameterBoolean(tokens[0], desc, default)
        elif tokens[1].lower().strip().startswith('number'):
            try:
                default = float(tokens[1].strip()[len('number') + 1:])
                param = ParameterNumber(tokens[0], desc, default=default)
            except:
                raise WrongScriptException(
                    self.tr('Could not load R script: %s.\n Problem with line %s' % (self.descriptionFile, line)))
        elif tokens[1].lower().strip().startswith('field'):
            field = tokens[1].strip()[len('field') + 1:]
            found = False
            for p in self.parameters:
                if p.name == field:
                    found = True
                    break
            if found:
                param = ParameterTableField(tokens[0], tokens[0], field)
        elif tokens[1].lower().strip() == 'extent':
            param = ParameterExtent(tokens[0], desc)
        elif tokens[1].lower().strip() == 'file':
            param = ParameterFile(tokens[0], desc, False)
        elif tokens[1].lower().strip() == 'folder':
            param = ParameterFile(tokens[0], desc, True)
        elif tokens[1].lower().strip().startswith('string'):
            default = tokens[1].strip()[len('string') + 1:]
            param = ParameterString(tokens[0], desc, default)
        elif tokens[1].lower().strip().startswith('output raster'):
            out = OutputRaster()
        elif tokens[1].lower().strip().startswith('output vector'):
            out = OutputVector()
        elif tokens[1].lower().strip().startswith('output table'):
            out = OutputTable()
        elif tokens[1].lower().strip().startswith('output file'):
            out = OutputFile()

        if param is not None:
            self.addParameter(param)
        elif out is not None:
            out.name = tokens[0]
            out.description = tokens[0]
            self.addOutput(out)
        else:
            raise WrongScriptException(
                self.tr('Could not load R script: %s.\n Problem with line %s' % (self.descriptionFile, line)))

    def processAlgorithm(self, progress):
        if isWindows():
            path = RUtils.RFolder()
            if path == '':
                raise GeoAlgorithmExecutionException(
                    self.tr('R folder is not configured.\nPlease configure it '
                            'before running R scripts.'))
        loglines = []
        loglines.append(self.tr('R execution commands'))
        loglines += self.getFullSetOfRCommands()
        for line in loglines:
            progress.setCommand(line)
        ProcessingLog.addToLog(ProcessingLog.LOG_INFO, loglines)
        RUtils.executeRAlgorithm(self, progress)
        if self.showPlots:
            htmlfilename = self.getOutputValue(RAlgorithm.RPLOTS)
            f = open(htmlfilename, 'w')
            f.write('<html><img src="' + self.plotsFilename + '"/></html>')
            f.close()
        if self.showConsoleOutput:
            htmlfilename = self.getOutputValue(RAlgorithm.R_CONSOLE_OUTPUT)
            f = open(htmlfilename, 'w')
            f.write(RUtils.getConsoleOutput())
            f.close()

    def getFullSetOfRCommands(self):
        commands = []
        commands += self.getImportCommands()
        commands += self.getRCommands()
        commands += self.getExportCommands()

        return commands

    def getExportCommands(self):
        commands = []
        for out in self.outputs:
            if isinstance(out, OutputRaster):
                value = out.value
                value = value.replace('\\', '/')
                if self.useRasterPackage or self.passFileNames:
                    commands.append('writeRaster(' + out.name + ',"' + value
                                    + '", overwrite=TRUE)')
                else:
                    if not value.endswith('tif'):
                        value = value + '.tif'
                    commands.append('writeGDAL(' + out.name + ',"' + value
                                    + '")')
            if isinstance(out, OutputVector):
                value = out.value
                if not value.endswith('shp'):
                    value = value + '.shp'
                value = value.replace('\\', '/')
                filename = os.path.basename(value)
                filename = filename[:-4]
                commands.append('writeOGR(' + out.name + ',"' + value + '","'
                                + filename + '", driver="ESRI Shapefile")')

        if self.showPlots:
            commands.append('dev.off()')

        return commands

    def getImportCommands(self):
        commands = []

        # Just use main mirror
        commands.append('options("repos"="http://cran.at.r-project.org/")')

        # Try to install packages if needed
        packages = RUtils.getRequiredPackages(self.script)
        packages.extend(['rgdal', 'raster'])
        for p in packages:
            commands.append('tryCatch(find.package("' + p
                            + '"), error=function(e) install.packages("' + p
                            + '", dependencies=TRUE))')
        commands.append('library("raster")')
        commands.append('library("rgdal")')

        for param in self.parameters:
            if isinstance(param, ParameterRaster):
                value = param.value
                value = value.replace('\\', '/')
                if self.passFileNames:
                    commands.append(param.name + ' = "' + value + '"')
                elif self.useRasterPackage:
                    commands.append(param.name + ' = ' + 'brick("' + value
                                    + '")')
                else:
                    commands.append(param.name + ' = ' + 'readGDAL("' + value
                                    + '")')
            if isinstance(param, ParameterVector):
                value = param.getSafeExportedLayer()
                value = value.replace('\\', '/')
                filename = os.path.basename(value)
                filename = filename[:-4]
                folder = os.path.dirname(value)
                if self.passFileNames:
                    commands.append(param.name + ' = "' + value + '"')
                else:
                    commands.append(param.name + ' = readOGR("' + folder
                                    + '",layer="' + filename + '")')
            if isinstance(param, ParameterTable):
                value = param.value
                if not value.lower().endswith('csv'):
                    raise GeoAlgorithmExecutionException(
                            'Unsupported input file format.\n' + value)
                if self.passFileNames:
                    commands.append(param.name + ' = "' + value + '"')
                else:
                    commands.append(param.name + ' <- read.csv("' + value
                                    + '", head=TRUE, sep=",")')
            elif isinstance(param, (ParameterTableField, ParameterString,
                            ParameterFile)):
                commands.append(param.name + '="' + param.value + '"')
            elif isinstance(param, (ParameterNumber, ParameterSelection)):
                commands.append(param.name + '=' + str(param.value))
            elif isinstance(param, ParameterBoolean):
                if param.value:
                    commands.append(param.name + '=TRUE')
                else:
                    commands.append(param.name + '=FALSE')
            elif isinstance(param, ParameterMultipleInput):
                iLayer = 0
                if param.datatype == ParameterMultipleInput.TYPE_RASTER:
                    layers = param.value.split(';')
                    for layer in layers:
                        layer = layer.replace('\\', '/')
                        if self.passFileNames:
                            commands.append('tempvar' + str(iLayer) + ' <- "'
                                    + layer + '"')
                        elif self.useRasterPackage:
                            commands.append('tempvar' + str(iLayer) + ' <- '
                                    + 'brick("' + layer + '")')
                        else:
                            commands.append('tempvar' + str(iLayer) + ' <- '
                                    + 'readGDAL("' + layer + '")')
                        iLayer += 1
                else:
                    exported = param.getSafeExportedLayers()
                    layers = exported.split(';')
                    for layer in layers:
                        if not layer.lower().endswith('shp') \
                            and not self.passFileNames:
                            raise GeoAlgorithmExecutionException(
                                    'Unsupported input file format.\n' + layer)
                        layer = layer.replace('\\', '/')
                        filename = os.path.basename(layer)
                        filename = filename[:-4]
                        if self.passFileNames:
                            commands.append('tempvar' + str(iLayer) + ' <- "'
                                    + layer + '"')
                        else:
                            commands.append('tempvar' + str(iLayer) + ' <- '
                                    + 'readOGR("' + layer + '",layer="'
                                    + filename + '")')
                        iLayer += 1
                s = ''
                s += param.name
                s += ' = c('
                iLayer = 0
                for layer in layers:
                    if iLayer != 0:
                        s += ','
                    s += 'tempvar' + str(iLayer)
                    iLayer += 1
                s += ')\n'
                commands.append(s)

        if self.showPlots:
            htmlfilename = self.getOutputValue(RAlgorithm.RPLOTS)
            self.plotsFilename = htmlfilename + '.png'
            self.plotsFilename = self.plotsFilename.replace('\\', '/')
            commands.append('png("' + self.plotsFilename + '")')

        return commands

    def getRCommands(self):
        return self.commands

    def help(self):
        helpfile = unicode(self.descriptionFile) + '.help'
        if os.path.exists(helpfile):
            return True, getHtmlFromHelpFile(self, helpfile)
        else:
            return False, None

    def checkBeforeOpeningParametersDialog(self):
        msg = RUtils.checkRIsInstalled()
        if msg is not None:
            html = self.tr(
                '<p>This algorithm requires R to be run. Unfortunately it '
                'seems that R is not installed in your system or it is not '
                'correctly configured to be used from QGIS</p>'
                '<p><a href="http://docs.qgis.org/testing/en/docs/user_manual/processing/3rdParty.html">Click here</a> '
                'to know more about how to install and configure R to be used with QGIS</p>')
            return html

    def getPostProcessingErrorMessage(self, wrongLayers):
        html = GeoAlgorithm.getPostProcessingErrorMessage(self, wrongLayers)
        msg = RUtils.checkRIsInstalled(True)
        html += self.tr(
            '<p>This algorithm requires R to be run. A test to check if '
            'R is correctly installed and configured in your system has '
            'been performed, with the following result:</p><ul><i>')
        if msg is None:
            html += self.tr(
                'R seems to be correctly installed and configured</i></li></ul>'
                '<p>The script you have executed needs the following packages:</p><ul>')
            packages = RUtils.getRequiredPackages(self.script)
            for p in packages:
                html += '<li>' + p + '</li>'
            html += self.tr(
                '</ul><p>Make sure they are installed in your R '
                'environment before trying to execute this script.</p>')
        else:
            html += msg + '</i></li></ul>'
            html += self.tr(
                '<p><a href= "http://docs.qgis.org/testing/en/docs/user_manual/processing/3rdParty.html">Click here</a> '
                'to know more about how to install and configure R to be used with QGIS</p>')

        return html
