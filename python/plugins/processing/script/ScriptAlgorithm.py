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
from processing.script.WrongScriptException import WrongScriptException
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException

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

        # If the line is in the format of the text description files for
        # normal algorithms, then process it using parameter and output
        # factories
        if '|' in line:
            self.processDescriptionParameterLine(line)
            return
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
            out.description = desc
            self.addOutput(out)
        else:
            raise WrongScriptException(
                self.tr('Could not load script: %s.\n'
                        'Problem with line "%s"', 'ScriptAlgorithm') % (self.descriptionFile or '', line))

    def processInputParameterToken(self, token, name):
        param = None

        descName = self.createDescriptiveName(name)

        if token.lower().strip() == 'raster':
            param = ParameterRaster(name, descName, False)
        elif token.lower().strip() == 'vector':
            param = ParameterVector(name, descName,
                                    [ParameterVector.VECTOR_TYPE_ANY])
        elif token.lower().strip() == 'vector point':
            param = ParameterVector(name, descName,
                                    [ParameterVector.VECTOR_TYPE_POINT])
        elif token.lower().strip() == 'vector line':
            param = ParameterVector(name, descName,
                                    [ParameterVector.VECTOR_TYPE_LINE])
        elif token.lower().strip() == 'vector polygon':
            param = ParameterVector(name, descName,
                                    [ParameterVector.VECTOR_TYPE_POLYGON])
        elif token.lower().strip() == 'table':
            param = ParameterTable(name, descName, False)
        elif token.lower().strip() == 'multiple raster':
            param = ParameterMultipleInput(name, descName,
                                           ParameterMultipleInput.TYPE_RASTER)
            param.optional = False
        elif token.lower().strip() == 'multiple vector':
            param = ParameterMultipleInput(name, descName,
                                           ParameterMultipleInput.TYPE_VECTOR_ANY)
            param.optional = False
        elif token.lower().strip().startswith('selectionfromfile'):
            options = token.strip()[len('selectionfromfile '):].split(';')
            param = ParameterSelection(name, descName, options, isSource=True)
        elif token.lower().strip().startswith('selection'):
            options = token.strip()[len('selection '):].split(';')
            param = ParameterSelection(name, descName, options)
        elif token.lower().strip().startswith('boolean'):
            default = token.strip()[len('boolean') + 1:]
            if default:
                param = ParameterBoolean(name, descName, default)
            else:
                param = ParameterBoolean(name, descName)
        elif token.lower().strip() == 'extent':
            param = ParameterExtent(name, descName)
        elif token.lower().strip() == 'point':
            param = ParameterPoint(name, descName)
        elif token.lower().strip() == 'file':
            param = ParameterFile(name, descName, False)
        elif token.lower().strip() == 'folder':
            param = ParameterFile(name, descName, True)
        elif token.lower().strip().startswith('number'):
            default = token.strip()[len('number') + 1:]
            if default:
                param = ParameterNumber(name, descName, default=default)
            else:
                param = ParameterNumber(name, descName)
        elif token.lower().strip().startswith('field'):
            if token.lower().strip().startswith('field number'):
                field = token.strip()[len('field number') + 1:]
                datatype = ParameterTableField.DATA_TYPE_NUMBER
            elif token.lower().strip().startswith('field string'):
                field = token.strip()[len('field string') + 1:]
                datatype = ParameterTableField.DATA_TYPE_STRING
            else:
                field = token.strip()[len('field') + 1:]
                datatype = ParameterTableField.DATA_TYPE_ANY
            found = False
            for p in self.parameters:
                if p.name == field:
                    found = True
                    break
            if found:
                param = ParameterTableField(
                    name=name,
                    description=descName,
                    parent=field,
                    datatype=datatype
                )
        elif token.lower().strip().startswith('multiple field'):
            if token.lower().strip().startswith('multiple field number'):
                field = token.strip()[len('multiple field number') + 1:]
                datatype = ParameterTableMultipleField.DATA_TYPE_NUMBER
            elif token.lower().strip().startswith('multiple field string'):
                field = token.strip()[len('multiple field string') + 1:]
                datatype = ParameterTableMultipleField.DATA_TYPE_STRING
            else:
                field = token.strip()[len('multiple field') + 1:]
                datatype = ParameterTableMultipleField.DATA_TYPE_ANY
            found = False
            for p in self.parameters:
                if p.name == field:
                    found = True
                    break
            if found:
                param = ParameterTableMultipleField(
                    name=name,
                    description=descName,
                    parent=field,
                    datatype=datatype
                )
        elif token.lower().strip().startswith('string'):
            default = token.strip()[len('string') + 1:]
            if default:
                param = ParameterString(name, descName, default)
            else:
                param = ParameterString(name, descName)
        elif token.lower().strip().startswith('longstring'):
            default = token.strip()[len('longstring') + 1:]
            if default:
                param = ParameterString(name, descName, default, multiline=True)
            else:
                param = ParameterString(name, descName, multiline=True)
        elif token.lower().strip().startswith('crs'):
            default = token.strip()[len('crs') + 1:]
            if default:
                param = ParameterCrs(name, descName, default)
            else:
                param = ParameterCrs(name, descName)

        return param

    def processOutputParameterToken(self, token):
        out = None

        if token.lower().strip().startswith('raster'):
            out = OutputRaster()
        elif token.lower().strip().startswith('vector'):
            out = OutputVector()
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
