# -*- coding: utf-8 -*-

"""
***************************************************************************
    ScriptAlgorithm.py
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
import re
import json
from qgis.core import QgsExpressionContextUtils, QgsExpressionContext
from qgis.PyQt.QtGui import QIcon
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.gui.Help2Html import getHtmlFromHelpFile
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterTable
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterMultipleInput
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterCrs
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterTableField
from processing.core.parameters import ParameterTableMultipleField
from processing.core.parameters import ParameterExtent
from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterPoint
from processing.core.parameters import getParameterFromString
from processing.core.outputs import OutputTable
from processing.core.outputs import OutputVector
from processing.core.outputs import OutputRaster
from processing.core.outputs import OutputNumber
from processing.core.outputs import OutputString
from processing.core.outputs import OutputHTML
from processing.core.outputs import OutputFile
from processing.core.outputs import OutputDirectory
from processing.core.outputs import getOutputFromString
from processing.core.ProcessingLog import ProcessingLog
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.script.WrongScriptException import WrongScriptException

from processing.tools import dataobjects

pluginPath = os.path.split(os.path.dirname(__file__))[0]


class ScriptAlgorithm(GeoAlgorithm):

    def __init__(self, descriptionFile, script=None):
        """The script parameter can be used to directly pass the code
        of the script without a file.

        This is to be used from the script edition dialog, but should
        not be used in other cases.
        """

        GeoAlgorithm.__init__(self)
        self._icon = QIcon(os.path.join(pluginPath, 'images', 'script.png'))

        self.script = script
        self.allowEdit = True
        self.noCRSWarning = False
        self.descriptionFile = descriptionFile
        if script is not None:
            self.defineCharacteristicsFromScript()
        if descriptionFile is not None:
            self.defineCharacteristicsFromFile()

    def getCopy(self):
        newone = ScriptAlgorithm(self.descriptionFile)
        newone.provider = self.provider
        return newone

    def getIcon(self):
        return self._icon

    def defineCharacteristicsFromFile(self):
        self.error = None
        self.script = ''
        filename = os.path.basename(self.descriptionFile)
        self.name = filename[:filename.rfind('.')].replace('_', ' ')
        self.group = self.tr('User scripts', 'ScriptAlgorithm')
        lines = open(self.descriptionFile)
        line = lines.readline()
        while line != '':
            if line.startswith('##'):
                try:
                    self.processParameterLine(line.strip('\n'))
                except:
                    self.error = self.tr('This script has a syntax errors.\n'
                                         'Problem with line: %s', 'ScriptAlgorithm') % line
            self.script += line
            line = lines.readline()
        lines.close()
        if self.group == self.tr('[Test scripts]', 'ScriptAlgorithm'):
            self.showInModeler = False
            self.showInToolbox = False

    def defineCharacteristicsFromScript(self):
        lines = self.script.split('\n')
        self.name, self.i18n_name = self.trAlgorithm('[Unnamed algorithm]', 'ScriptAlgorithm')
        self.group, self.i18n_group = self.trAlgorithm('User scripts', 'ScriptAlgorithm')
        for line in lines:
            if line.startswith('##'):
                try:
                    self.processParameterLine(line.strip('\n'))
                except:
                    pass

    def checkBeforeOpeningParametersDialog(self):
        return self.error

    def checkInputCRS(self):
        if self.noCRSWarning:
            return True
        else:
            return GeoAlgorithm.checkInputCRS(self)

    def createDescriptiveName(self, s):
        return s.replace('_', ' ')

    def processParameterLine(self, line):
        param = None
        out = None
        line = line.replace('#', '')

        if line == "nomodeler":
            self.showInModeler = False
            return
        if line == "nocrswarning":
            self.noCRSWarning = True
            return
        tokens = line.split('=', 1)
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
        else:
            param = getParameterFromString(line)

        if param is not None:
            self.addParameter(param)
        elif out is not None:
            out.name = tokens[0]
            out.description = desc
            self.addOutput(out)
        else:
            raise WrongScriptException(
                self.tr('Could not load script: %s.\n'
                        'Problem with line "%s"', 'ScriptAlgorithm') % (self.descriptionFile or '', line))

    def processInputParameterLine(self, line):
        for paramClass in paramClasses:
            param  = paramClass.fromScriptCode(line)
            if param is not None:
                return param

    def processOutputParameterToken(self, token):
        out = None

        if token.lower().strip().startswith('raster'):
            out = OutputRaster()
        elif token.lower().strip() == 'vector':
            out = OutputVector()
        elif token.lower().strip() == 'vector point':
            out = OutputVector(datatype=[dataobjects.TYPE_VECTOR_POINT])
        elif token.lower().strip() == 'vector line':
            out = OutputVector(datatype=[OutputVector.TYPE_VECTOR_LINE])
        elif token.lower().strip() == 'vector polygon':
            out = OutputVector(datatype=[OutputVector.TYPE_VECTOR_POLYGON])
        elif token.lower().strip().startswith('table'):
            out = OutputTable()
        elif token.lower().strip().startswith('html'):
            out = OutputHTML()
        elif token.lower().strip().startswith('file'):
            out = OutputFile()
            subtokens = token.split(' ')
            if len(subtokens) > 2:
                out.ext = subtokens[2]
        elif token.lower().strip().startswith('directory'):
            out = OutputDirectory()
        elif token.lower().strip().startswith('number'):
            out = OutputNumber()
        elif token.lower().strip().startswith('string'):
            out = OutputString()
        elif token.lower().strip().startswith('extent'):
            out = OutputExtent()

        return out

    def processDescriptionParameterLine(self, line):
        try:
            if line.startswith('Parameter'):
                self.addParameter(getParameterFromString(line))
            elif line.startswith('*Parameter'):
                param = getParameterFromString(line[1:])
                param.isAdvanced = True
                self.addParameter(param)
            else:
                self.addOutput(getOutputFromString(line))
        except Exception:
            raise WrongScriptException(
                self.tr('Could not load script: %s.\n'
                        'Problem with line %d', 'ScriptAlgorithm') % (self.descriptionFile or '', line))

    def processAlgorithm(self, progress):
        ns = {}
        ns['progress'] = progress
        ns['scriptDescriptionFile'] = self.descriptionFile

        for param in self.parameters:
            ns[param.name] = param.value

        for out in self.outputs:
            ns[out.name] = out.value

        variables = re.findall('@[a-zA-Z0-9_]*', self.script)
        script = 'import processing\n'
        script += self.script

        context = QgsExpressionContext()
        context.appendScope(QgsExpressionContextUtils.globalScope())
        context.appendScope(QgsExpressionContextUtils.projectScope())
        for var in variables:
            varname = var[1:]
            if context.hasVariable(varname):
                script = script.replace(var, context.variable(varname))
            else:
                ProcessingLog.addToLog(ProcessingLog.LOG_WARNING, 'Cannot find variable: %s' % varname)

        exec((script), ns)
        for out in self.outputs:
            out.setValue(ns[out.name])

    def help(self):
        if self.descriptionFile is None:
            return False, None
        helpfile = self.descriptionFile + '.help'
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
