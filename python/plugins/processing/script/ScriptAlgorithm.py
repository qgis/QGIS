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
from PyQt4 import QtGui
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
from processing.core.parameters import ParameterExtent
from processing.core.parameters import ParameterFile
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
from processing.script.WrongScriptException import WrongScriptException

class ScriptAlgorithm(GeoAlgorithm):

    _icon = QtGui.QIcon(os.path.dirname(__file__) + '/../images/script.png')

    def __init__(self, descriptionFile, script=None):
        """The script parameter can be used to directly pass the code
        of the script without a file.

        This is to be used from the script edition dialog, but should
        not be used in other cases.
        """

        GeoAlgorithm.__init__(self)
        self.script = script
        self.allowEdit = True
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
        self.script = ''
        self.silentOutputs = []
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
                    raise WrongScriptException(
                        self.tr('Could not load script: %s\n'
                                'Problem with line: %d', 'ScriptAlgorithm') % (self.descriptionFile, line))
            self.script += line
            line = lines.readline()
        lines.close()
        if self.group == self.tr('[Test scripts]', 'ScriptAlgorithm'):
            self.showInModeler = False
            self.showInToolbox = False

    def defineCharacteristicsFromScript(self):
        lines = self.script.split('\n')
        self.silentOutputs = []
        self.name = self.tr('[Unnamed algorithm]', 'ScriptAlgorithm')
        self.group = self.tr('User scripts', 'ScriptAlgorithm')
        for line in lines:
            if line.startswith('##'):
                try:
                    self.processParameterLine(line.strip('\n'))
                except:
                    pass

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
        tokens = line.split('=', 1)
        desc = self.createDescriptiveName(tokens[0])
        if tokens[1].lower().strip() == 'group':
            self.group = tokens[0]
            return
        if tokens[1].lower().strip() == 'name':
            self.name = tokens[0]
            return
        if tokens[1].lower().strip() == 'raster':
            param = ParameterRaster(tokens[0], desc, False)
        elif tokens[1].lower().strip() == 'vector':
            param = ParameterVector(tokens[0], desc,
                                    [ParameterVector.VECTOR_TYPE_ANY])
        elif tokens[1].lower().strip() == 'vector point':
            param = ParameterVector(tokens[0], desc,
                                    [ParameterVector.VECTOR_TYPE_POINT])
        elif tokens[1].lower().strip() == 'vector line':
            param = ParameterVector(tokens[0], desc,
                                    [ParameterVector.VECTOR_TYPE_LINE])
        elif tokens[1].lower().strip() == 'vector polygon':
            param = ParameterVector(tokens[0], desc,
                                    [ParameterVector.VECTOR_TYPE_POLYGON])
        elif tokens[1].lower().strip() == 'table':
            param = ParameterTable(tokens[0], desc, False)
        elif tokens[1].lower().strip() == 'multiple raster':
            param = ParameterMultipleInput(tokens[0], desc,
                    ParameterMultipleInput.TYPE_RASTER)
            param.optional = False
        elif tokens[1].lower().strip() == 'multiple vector':
            param = ParameterMultipleInput(tokens[0], desc,
                    ParameterMultipleInput.TYPE_VECTOR_ANY)
            param.optional = False
        elif tokens[1].lower().strip().startswith('selection'):
            options = tokens[1].strip()[len('selection '):].split(';')
            param = ParameterSelection(tokens[0], desc, options)
        elif tokens[1].lower().strip().startswith('boolean'):
            default = tokens[1].strip()[len('boolean') + 1:]
            param = ParameterBoolean(tokens[0], desc, default)
        elif tokens[1].lower().strip() == 'extent':
            param = ParameterExtent(tokens[0], desc)
        elif tokens[1].lower().strip() == 'file':
            param = ParameterFile(tokens[0], desc, False)
        elif tokens[1].lower().strip() == 'folder':
            param = ParameterFile(tokens[0], desc, True)
        elif tokens[1].lower().strip().startswith('number'):
            default = tokens[1].strip()[len('number') + 1:]
            param = ParameterNumber(tokens[0], desc, default=default)
        elif tokens[1].lower().strip().startswith('field'):
            field = tokens[1].strip()[len('field') + 1:]
            found = False
            for p in self.parameters:
                if p.name == field:
                    found = True
                    break
            if found:
                param = ParameterTableField(tokens[0], desc, field)
        elif tokens[1].lower().strip().startswith('string'):
            default = tokens[1].strip()[len('string') + 1:]
            param = ParameterString(tokens[0], desc, default)
        elif tokens[1].lower().strip().startswith('longstring'):
            default = tokens[1].strip()[len('longstring') + 1:]
            param = ParameterString(tokens[0], desc, default, multiline = True)
        elif tokens[1].lower().strip().startswith('crs'):
            default = tokens[1].strip()[len('crs') + 1:]
            if not default:
                default = 'EPSG:4326'
            param = ParameterCrs(tokens[0], desc, default)
        elif tokens[1].lower().strip().startswith('output raster'):
            out = OutputRaster()
        elif tokens[1].lower().strip().startswith('output vector'):
            out = OutputVector()
        elif tokens[1].lower().strip().startswith('output table'):
            out = OutputTable()
        elif tokens[1].lower().strip().startswith('output html'):
            out = OutputHTML()
        elif tokens[1].lower().strip().startswith('output file'):
            out = OutputFile()
            subtokens = tokens[1].split(' ')
            if len(subtokens) > 2:
                out.ext = subtokens[2]
        elif tokens[1].lower().strip().startswith('output directory'):
            out = OutputDirectory()
        elif tokens[1].lower().strip().startswith('output number'):
            out = OutputNumber()
        elif tokens[1].lower().strip().startswith('output string'):
            out = OutputString()

        if param is not None:
            self.addParameter(param)
        elif out is not None:
            out.name = tokens[0]
            out.description = desc
            self.addOutput(out)
        else:
            raise WrongScriptException(
                self.tr('Could not load script: %s.\n'
                        'Problem with line %d', 'ScriptAlgorithm') % (self.descriptionFile or '', line))

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

        script = 'import processing\n'

        ns = {}
        ns['progress'] = progress
        ns['scriptDescriptionFile'] = self.descriptionFile

        for param in self.parameters:
            ns[param.name] = param.value

        for out in self.outputs:
            ns[out.name] = out.value

        script += self.script
        exec(script) in ns
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
