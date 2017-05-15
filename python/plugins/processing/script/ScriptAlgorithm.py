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
from builtins import str

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import re
import json
from qgis.core import (QgsExpressionContextUtils,
                       QgsExpressionContext,
                       QgsProcessingAlgorithm,
                       QgsProject,
                       QgsApplication,
                       QgsMessageLog,
                       QgsProcessingUtils)

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.gui.Help2Html import getHtmlFromHelpFile
from processing.core.parameters import getParameterFromString
from processing.core.outputs import getOutputFromString
from processing.script.WrongScriptException import WrongScriptException

pluginPath = os.path.split(os.path.dirname(__file__))[0]


class ScriptAlgorithm(GeoAlgorithm):

    def __init__(self, descriptionFile, script=None):
        """The script parameter can be used to directly pass the code
        of the script without a file.

        This is to be used from the script edition dialog, but should
        not be used in other cases.
        """

        GeoAlgorithm.__init__(self)
        self._icon = QgsApplication.getThemeIcon("/processingScript.svg")
        self._name = ''
        self._display_name = ''
        self._group = ''
        self._flags = 0

        self.script = script
        self.allowEdit = True
        self.noCRSWarning = False
        self.descriptionFile = descriptionFile
        if script is not None:
            self.defineCharacteristicsFromScript()
        if descriptionFile is not None:
            self.defineCharacteristicsFromFile()

    def icon(self):
        return self._icon

    def name(self):
        return self._name

    def displayName(self):
        return self._display_name

    def group(self):
        return self._group

    def flags(self):
        return self._flags

    def svgIconPath(self):
        return QgsApplication.iconPath("processingScript.svg")

    def defineCharacteristicsFromFile(self):
        self.error = None
        self.script = ''
        filename = os.path.basename(self.descriptionFile)
        self._name = filename[:filename.rfind('.')].replace('_', ' ')
        self._group = self.tr('User scripts', 'ScriptAlgorithm')
        with open(self.descriptionFile) as lines:
            line = lines.readline()
            while line != '':
                if line.startswith('##'):
                    try:
                        self.processParameterLine(line.strip('\n'))
                    except:
                        self.error = self.tr('This script has a syntax errors.\n'
                                             'Problem with line: {0}', 'ScriptAlgorithm').format(line)
                self.script += line
                line = lines.readline()
        if self._group == self.tr('[Test scripts]', 'ScriptAlgorithm'):
            self._flags = QgsProcessingAlgorithm.FlagHideFromToolbox | QgsProcessingAlgorithm.FlagHideFromModeler

    def defineCharacteristicsFromScript(self):
        lines = self.script.split('\n')
        self._name = '[Unnamed algorithm]'
        self._display_name = self.tr('[Unnamed algorithm]', 'ScriptAlgorithm')
        self._group = self.tr('User scripts', 'ScriptAlgorithm')
        for line in lines:
            if line.startswith('##'):
                try:
                    self.processParameterLine(line.strip('\n'))
                except:
                    pass

    def canExecute(self):
        return not self.error, self.error

    def checkInputCRS(self):
        if self.noCRSWarning:
            return True
        else:
            return GeoAlgorithm.checkInputCRS(self)

    def createDescriptiveName(self, s):
        return s.replace('_', ' ')

    def processParameterLine(self, line):
        param = None
        line = line.replace('#', '')

        if line == "nomodeler":
            self._flags = self._flags | QgsProcessingAlgorithm.FlagHideFromModeler
            return
        if line == "nocrswarning":
            self.noCRSWarning = True
            return
        tokens = line.split('=', 1)
        desc = self.createDescriptiveName(tokens[0])
        if tokens[1].lower().strip() == 'group':
            self._group = tokens[0]
            return
        if tokens[1].lower().strip() == 'name':
            self._name = self._display_name = tokens[0]
            return

        out = getOutputFromString(line)
        if out is None:
            param = getParameterFromString(line)

        if param is not None:
            self.addParameter(param)
        elif out is not None:
            out.name = tokens[0]
            out.description = desc
            self.addOutput(out)
        else:
            raise WrongScriptException(
                self.tr('Could not load script: {0}.\n'
                        'Problem with line "{1}"', 'ScriptAlgorithm').format(self.descriptionFile or '', line))

    def processAlgorithm(self, parameters, context, feedback):
        ns = {}
        ns['feedback'] = feedback
        ns['scriptDescriptionFile'] = self.descriptionFile
        ns['context'] = context

        for param in self.parameters:
            ns[param.name] = param.value

        for out in self.outputs:
            ns[out.name] = out.value

        variables = re.findall('@[a-zA-Z0-9_]*', self.script)
        script = 'import processing\n'
        script += self.script

        context = QgsExpressionContext()
        context.appendScope(QgsExpressionContextUtils.globalScope())
        context.appendScope(QgsExpressionContextUtils.projectScope(QgsProject.instance()))
        for var in variables:
            varname = var[1:]
            if context.hasVariable(varname):
                script = script.replace(var, context.variable(varname))
            else:
                QgsMessageLog.logMessage(self.tr('Cannot find variable: {0}').format(varname), self.tr('Processing'), QgsMessageLog.WARNING)

        exec((script), ns)
        for out in self.outputs:
            out.setValue(ns[out.name])

    def helpString(self):
        if self.descriptionFile is None:
            return False, None
        helpfile = self.descriptionFile + '.help'
        if os.path.exists(helpfile):
            return getHtmlFromHelpFile(self, helpfile)
        else:
            return None

    def shortHelpString(self):
        if self.descriptionFile is None:
            return None
        helpFile = str(self.descriptionFile) + '.help'
        if os.path.exists(helpFile):
            with open(helpFile) as f:
                try:
                    descriptions = json.load(f)
                    if 'ALG_DESC' in descriptions:
                        return str(descriptions['ALG_DESC'])
                except:
                    return None
        return None

    def getParameterDescriptions(self):
        descs = {}
        if self.descriptionFile is None:
            return descs
        helpFile = str(self.descriptionFile) + '.help'
        if os.path.exists(helpFile):
            with open(helpFile) as f:
                try:
                    descriptions = json.load(f)
                    for param in self.parameters:
                        if param.name in descriptions:
                            descs[param.name] = str(descriptions[param.name])
                except:
                    return descs
        return descs
