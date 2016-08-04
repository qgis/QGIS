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
import json

from qgis.PyQt.QtGui import QIcon

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
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
from processing.core.parameters import ParameterTableMultipleField
from processing.core.parameters import ParameterExtent
from processing.core.parameters import ParameterCrs
from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterPoint
from processing.core.outputs import OutputTable
from processing.core.outputs import OutputVector
from processing.core.outputs import OutputRaster
from processing.core.outputs import OutputHTML
from processing.core.outputs import OutputFile
from processing.tools.system import isWindows
from processing.script.WrongScriptException import WrongScriptException
from .RUtils import RUtils


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
        return QIcon(os.path.dirname(__file__) + '/../../images/r.svg')

    def defineCharacteristicsFromScript(self):
        lines = self.script.split('\n')
        self.name, self.i18n_name = self.trAlgorithm('[Unnamed algorithm]')
        self.group, self.i18n_group = self.trAlgorithm('User R scripts')
        self.parseDescription(iter(lines))

    def defineCharacteristicsFromFile(self):
        filename = os.path.basename(self.descriptionFile)
        self.name = filename[:filename.rfind('.')].replace('_', ' ')
        self.group, self.i18n_group = self.trAlgorithm('User R scripts')
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
            self.group = self.i18n_group = tokens[0]
            return
        if tokens[1].lower().strip() == 'name':
            self.name = self.i18n_name = tokens[0]
            return

        if tokens[1].lower().strip().startswith('output'):
            outToken = tokens[1].strip()[len('output') + 1:]
            out = self.processOutputParameterToken(outToken)

        elif tokens[1].lower().strip().startswith('optional'):
            optToken = tokens[1].strip()[len('optional') + 1:]
            param = self.processInputParameterToken(optToken, tokens[0])
            if param:
                param.optional = True

        else:
            param = self.processInputParameterToken(tokens[1], tokens[0])

        if param is not None:
            self.addParameter(param)
        elif out is not None:
            out.name = tokens[0]
            out.description = tokens[0]
            self.addOutput(out)
        else:
            raise WrongScriptException(
                self.tr('Could not load R script: %s.\n Problem with line %s' % (self.descriptionFile, line)))

    def processInputParameterToken(self, token, name):
        param = None

        desc = self.createDescriptiveName(name)

        if token.lower().strip().startswith('raster'):
            param = ParameterRaster(name, desc, False)
        elif token.lower().strip() == 'vector':
            param = ParameterVector(name, desc,
                                    [ParameterVector.VECTOR_TYPE_ANY])
        elif token.lower().strip() == 'vector point':
            param = ParameterVector(name, desc,
                                    [ParameterVector.VECTOR_TYPE_POINT])
        elif token.lower().strip() == 'vector line':
            param = ParameterVector(name, desc,
                                    [ParameterVector.VECTOR_TYPE_LINE])
        elif token.lower().strip() == 'vector polygon':
            param = ParameterVector(name, desc,
                                    [ParameterVector.VECTOR_TYPE_POLYGON])
        elif token.lower().strip() == 'table':
            param = ParameterTable(name, desc, False)
        elif token.lower().strip().startswith('multiple raster'):
            param = ParameterMultipleInput(name, desc,
                                           ParameterMultipleInput.TYPE_RASTER)
            param.optional = False
        elif token.lower().strip() == 'multiple vector':
            param = ParameterMultipleInput(name, desc,
                                           ParameterMultipleInput.TYPE_VECTOR_ANY)
            param.optional = False
        elif token.lower().strip().startswith('selection'):
            options = token.strip()[len('selection'):].split(';')
            param = ParameterSelection(name, desc, options)
        elif token.lower().strip().startswith('boolean'):
            default = token.strip()[len('boolean') + 1:]
            if default:
                param = ParameterBoolean(name, desc, default)
            else:
                param = ParameterBoolean(name, desc)
        elif token.lower().strip().startswith('number'):
            default = token.strip()[len('number') + 1:]
            if default:
                param = ParameterNumber(name, desc, default=default)
            else:
                param = ParameterNumber(name, desc)
        elif token.lower().strip().startswith('field'):
            field = token.strip()[len('field') + 1:]
            found = False
            for p in self.parameters:
                if p.name == field:
                    found = True
                    break
            if found:
                param = ParameterTableField(name, desc, field)
        elif token.lower().strip().startswith('multiple field'):
            field = token.strip()[len('multiple field') + 1:]
            found = False
            for p in self.parameters:
                if p.name == field:
                    found = True
                    break
            if found:
                param = ParameterTableMultipleField(token, desc, field)
        elif token.lower().strip() == 'extent':
            param = ParameterExtent(name, desc)
        elif token.lower().strip() == 'point':
            param = ParameterPoint(name, desc)
        elif token.lower().strip() == 'file':
            param = ParameterFile(name, desc, False)
        elif token.lower().strip() == 'folder':
            param = ParameterFile(name, desc, True)
        elif token.lower().strip().startswith('string'):
            default = token.strip()[len('string') + 1:]
            if default:
                param = ParameterString(name, desc, default)
            else:
                param = ParameterString(name, desc)
        elif token.lower().strip().startswith('longstring'):
            default = token.strip()[len('longstring') + 1:]
            if default:
                param = ParameterString(name, desc, default, multiline=True)
            else:
                param = ParameterString(name, desc, multiline=True)
        elif token.lower().strip() == 'crs':
            default = token.strip()[len('crs') + 1:]
            if default:
                param = ParameterCrs(name, desc, default)
            else:
                param = ParameterCrs(name, desc)

        return param

    def processOutputParameterToken(self, token):
        out = None

        if token.lower().strip().startswith('raster'):
            out = OutputRaster()
        elif token.lower().strip().startswith('vector'):
            out = OutputVector()
        elif token.lower().strip().startswith('table'):
            out = OutputTable()
        elif token.lower().strip().startswith('file'):
            out = OutputFile()

        return out

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
            elif isinstance(out, OutputVector):
                value = out.value
                if not value.endswith('shp'):
                    value = value + '.shp'
                value = value.replace('\\', '/')
                filename = os.path.basename(value)
                filename = filename[:-4]
                commands.append('writeOGR(' + out.name + ',"' + value + '","'
                                + filename + '", driver="ESRI Shapefile")')
            elif isinstance(out, OutputTable):
                value = out.value
                value = value.replace('\\', '/')
                commands.append('write.csv(' + out.name + ',"' + value + '")')

        if self.showPlots:
            commands.append('dev.off()')

        return commands

    def getImportCommands(self):
        commands = []

        # Just use main mirror
        commands.append('options("repos"="http://cran.at.r-project.org/")')

        # Try to install packages if needed
        if isWindows():
            commands.append('.libPaths(\"' + unicode(RUtils.RLibs()).replace('\\', '/') + '\")')
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
                if param.value is None:
                    commands.append(param.name + '= NULL')
                else:
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
            elif isinstance(param, ParameterVector):
                if param.value is None:
                    commands.append(param.name + '= NULL')
                else:
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
            elif isinstance(param, ParameterTable):
                if param.value is None:
                    commands.append(param.name + '= NULL')
                else:
                    value = param.value
                    if not value.lower().endswith('csv'):
                        raise GeoAlgorithmExecutionException(
                            'Unsupported input file format.\n' + value)
                    if self.passFileNames:
                        commands.append(param.name + ' = "' + value + '"')
                    else:
                        commands.append(param.name + ' <- read.csv("' + value
                                        + '", head=TRUE, sep=",")')
            elif isinstance(param, ParameterExtent):
                if param.value:
                    tokens = unicode(param.value).split(',')
                    # Extent from raster package is "xmin, xmax, ymin, ymax" like in Processing
                    # http://www.inside-r.org/packages/cran/raster/docs/Extent
                    commands.append(param.name + ' = extent(' + tokens[0] + ',' + tokens[1] + ',' + tokens[2] + ',' + tokens[3] + ')')
                else:
                    commands.append(param.name + ' = NULL')
            elif isinstance(param, ParameterCrs):
                if param.value is None:
                    commands.append(param.name + '= NULL')
                else:
                    commands.append(param.name + ' = "' + param.value + '"')
            elif isinstance(param, (ParameterTableField, ParameterTableMultipleField, ParameterString,
                                    ParameterFile)):
                if param.value is None:
                    commands.append(param.name + '= NULL')
                else:
                    commands.append(param.name + '="' + param.value + '"')
            elif isinstance(param, (ParameterNumber, ParameterSelection)):
                if param.value is None:
                    commands.append(param.name + '= NULL')
                else:
                    commands.append(param.name + '=' + unicode(param.value))
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
                            commands.append('tempvar' + unicode(iLayer) + ' <- "'
                                            + layer + '"')
                        elif self.useRasterPackage:
                            commands.append('tempvar' + unicode(iLayer) + ' <- '
                                            + 'brick("' + layer + '")')
                        else:
                            commands.append('tempvar' + unicode(iLayer) + ' <- '
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
                            commands.append('tempvar' + unicode(iLayer) + ' <- "'
                                            + layer + '"')
                        else:
                            commands.append('tempvar' + unicode(iLayer) + ' <- '
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
                    s += 'tempvar' + unicode(iLayer)
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

    def shortHelp(self):
        if self.descriptionFile is None:
            return None
        helpFile = unicode(self.descriptionFile) + '.help'
        if os.path.exists(helpFile):
            with open(helpFile) as f:
                try:
                    descriptions = json.load(f)
                    if 'ALG_DESC' in descriptions:
                        return self._formatHelp(unicode(descriptions['ALG_DESC']))
                except:
                    return None
        return None

    def getParameterDescriptions(self):
        descs = {}
        if self.descriptionFile is None:
            return descs
        helpFile = unicode(self.descriptionFile) + '.help'
        if os.path.exists(helpFile):
            with open(helpFile) as f:
                try:
                    descriptions = json.load(f)
                    for param in self.parameters:
                        if param.name in descriptions:
                            descs[param.name] = unicode(descriptions[param.name])
                except:
                    return descs
        return descs

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
