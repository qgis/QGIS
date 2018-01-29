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
from qgis.core import (QgsExpressionContextUtils,
                       QgsExpressionContext,
                       QgsProcessingAlgorithm,
                       QgsProject,
                       QgsApplication,
                       QgsMessageLog,
                       QgsProcessingUtils,
                       QgsProcessingAlgorithm)

from qgis.PyQt.QtCore import (QCoreApplication)

from processing.gui.Help2Html import getHtmlFromHelpFile
from processing.core.parameters import getParameterFromString
from processing.core.outputs import getOutputFromString
from processing.script.WrongScriptException import WrongScriptException

pluginPath = os.path.split(os.path.dirname(__file__))[0]


class ScriptAlgorithm(QgsProcessingAlgorithm):

    def __init__(self, descriptionFile, script=None):
        """The script parameter can be used to directly pass the code
        of the script without a file.

        This is to be used from the script edition dialog, but should
        not be used in other cases.
        """

        super().__init__()
        self._icon = QgsApplication.getThemeIcon("/processingScript.svg")
        self._name = ''
        self._display_name = ''
        self._group = ''
        self._groupId = ''
        self._flags = None

        self.script = script
        self.allowEdit = True
        self.noCRSWarning = False
        self.descriptionFile = descriptionFile
        if script is not None:
            self.defineCharacteristicsFromScript()
        if descriptionFile is not None:
            self.defineCharacteristicsFromFile()

        self.ns = {}
        self.cleaned_script = None
        self.results = {}

    def createInstance(self):
        if self.descriptionFile is not None:
            return ScriptAlgorithm(self.descriptionFile)
        else:
            return ScriptAlgorithm(descriptionFile=None, script=self.script)

    def initAlgorithm(self, config=None):
        pass

    def icon(self):
        return self._icon

    def name(self):
        return self._name

    def displayName(self):
        return self._display_name

    def group(self):
        return self._group

    def flags(self):
        if self._flags is not None:
            return QgsProcessingAlgorithm.Flags(self._flags)
        else:
            return QgsProcessingAlgorithm.flags(self)

    def svgIconPath(self):
        return QgsApplication.iconPath("processingScript.svg")

    def defineCharacteristicsFromFile(self):
        self.error = None
        self.script = ''
        filename = os.path.basename(self.descriptionFile)
        self._name = filename[:filename.rfind('.')].replace('_', ' ')
        self._display_name = self._name
        self._group = self.tr('User scripts', 'ScriptAlgorithm')
        self._groupId = 'userscripts'
        with open(self.descriptionFile) as lines:
            line = lines.readline()
            while line != '':
                if line.startswith('##'):
                    try:
                        self.processParameterLine(line.strip('\n'))
                    except:
                        self.error = self.tr('This script has a syntax error.\n'
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
        self._groupId = 'userscripts'
        for line in lines:
            if line.startswith('##'):
                try:
                    self.processParameterLine(line.strip('\n'))
                except:
                    pass

    def canExecute(self):
        return not self.error, self.error

    def validateInputCrs(self, parameters, context):
        if self.noCRSWarning:
            return True
        else:
            return QgsProcessingAlgorithm.validateInputCrs(self, parameters, context)

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
            validChars = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:'
            self._groupId = ''.join(c for c in tokens[0].lower() if c in validChars)
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
            out.setName(tokens[0])
            out.setDescription(desc)
            self.addOutput(out)
        else:
            raise WrongScriptException(
                self.tr('Could not load script: {0}.\n'
                        'Problem with line "{1}"', 'ScriptAlgorithm').format(self.descriptionFile or '', line))

    def prepareAlgorithm(self, parameters, context, feedback):
        for param in self.parameterDefinitions():
            method = None
            if param.type() == "boolean":
                method = self.parameterAsBool
            elif param.type() == "crs":
                method = self.parameterAsCrs
            elif param.type() == "layer":
                method = self.parameterAsLayer
            elif param.type() == "extent":
                method = self.parameterAsExtent
            elif param.type() == "point":
                method = self.parameterAsPoint
            elif param.type() == "file":
                method = self.parameterAsFile
            elif param.type() == "matrix":
                method = self.parameterAsMatrix
            elif param.type() == "multilayer":
                method = self.parameterAsLayerList
            elif param.type() == "number":
                method = self.parameterAsDouble
            elif param.type() == "range":
                method = self.parameterAsRange
            elif param.type() == "raster":
                method = self.parameterAsRasterLayer
            elif param.type() == "enum":
                method = self.parameterAsEnum
            elif param.type() == "string":
                method = self.parameterAsString
            elif param.type() == "expression":
                method = self.parameterAsString
            elif param.type() == "vector":
                method = self.parameterAsVectorLayer
            elif param.type() == "field":
                if param.allowMultiple():
                    method = self.parameterAsFields
                else:
                    method = self.parameterAsString
            elif param.type() == "source":
                method = self.parameterAsSource

            if method:
                self.ns[param.name()] = method(parameters, param.name(), context)

        self.ns['scriptDescriptionFile'] = self.descriptionFile
        for out in self.outputDefinitions():
            self.ns[out.name()] = None

        self.ns['self'] = self
        self.ns['parameters'] = parameters

        expression_context = self.createExpressionContext(parameters, context)
        variables = re.findall('@[a-zA-Z0-9_]*', self.script)
        script = 'import processing\n'
        script += self.script
        for var in variables:
            varname = var[1:]
            if context.hasVariable(varname):
                script = script.replace(var, context.variable(varname))
            else:
                # messy - it's probably NOT a variable, and instead an email address or some other string containing '@'
                QgsMessageLog.logMessage(self.tr('Cannot find variable: {0}').format(varname), self.tr('Processing'), QgsMessageLog.WARNING)
        self.cleaned_script = script

        return True

    def processAlgorithm(self, parameters, context, feedback):
        self.ns['feedback'] = feedback
        self.ns['context'] = context

        exec((self.cleaned_script), self.ns)
        self.results = {}
        for out in self.outputDefinitions():
            self.results[out.name()] = self.ns[out.name()]
        del self.ns
        return self.results

    def helpUrl(self):
        if self.descriptionFile is None:
            return None
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
                    for param in self.parameterDefinitions():
                        if param.name() in descriptions:
                            descs[param.name()] = str(descriptions[param.name()])
                except:
                    return descs
        return descs

    def tr(self, string, context=''):
        if context == '':
            context = self.__class__.__name__
        return QCoreApplication.translate(context, string)
