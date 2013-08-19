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
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.parameters.ParameterTable import ParameterTable
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterMultipleInput import ParameterMultipleInput
from sextante.script.WrongScriptException import WrongScriptException
from sextante.outputs.OutputTable import OutputTable
from sextante.outputs.OutputVector import OutputVector
from sextante.outputs.OutputRaster import OutputRaster
from sextante.outputs.OutputNumber import OutputNumber
from sextante.outputs.OutputString import OutputString
from sextante.parameters.ParameterString import ParameterString
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.parameters.ParameterTableField import ParameterTableField
from sextante.outputs.OutputHTML import OutputHTML
from sextante.parameters.ParameterExtent import ParameterExtent
from sextante.parameters.ParameterFile import ParameterFile
from sextante.outputs.OutputFile import OutputFile
from sextante.parameters.ParameterFactory import ParameterFactory
from sextante.outputs.OutputFactory import OutputFactory
import sys
from sextante.gui.Help2Html import Help2Html

class ScriptAlgorithm(GeoAlgorithm):

    def __init__(self, descriptionFile, script=None):
        '''The script parameter can be used to directly pass the code of the script without a file.
        This is to be used from the script edition dialog, but should not be used in other cases'''
        GeoAlgorithm.__init__(self)
        self.script = script
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
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/script.png")

    def defineCharacteristicsFromFile(self):
        self.script=""
        self.silentOutputs = []
        filename = os.path.basename(self.descriptionFile)
        self.name = filename[:filename.rfind(".")].replace("_", " ")
        self.group = "User scripts"
        lines = open(self.descriptionFile)
        line = lines.readline()
        while line != "":
            if line.startswith("##"):
                try:
                    self.processParameterLine(line.strip("\n"))
                except:
                    raise  WrongScriptException("Could not load script: " + self.descriptionFile +"\n" + "Problem with line: " + line)
            self.script += line
            line = lines.readline()
        lines.close()

    def defineCharacteristicsFromScript(self):
        lines = self.script.split("\n")
        self.silentOutputs = []
        self.name = "[Unnamed algorithm]"
        self.group = "User scripts"
        for line in lines:
            if line.startswith("##"):
                try:
                    self.processParameterLine(line.strip("\n"))
                except:
                    pass

    def createDescriptiveName(self, s):
        return s.replace("_", " ")

    def processParameterLine(self,line):
        param = None
        out = None
        line = line.replace("#", "");
        # If the line is in the format of the text description files for normal algorithms,
        # then process it using parameter and output factories
        if '|' in line:
            self.processDescriptionParameterLine(line)
            return
        tokens = line.split("=");
        desc = self.createDescriptiveName(tokens[0])
        if tokens[1].lower().strip() == "group":
            self.group = tokens[0]
            return
        if tokens[1].lower().strip() == "name":
            self.name = tokens[0]
            return
        if tokens[1].lower().strip() == "raster":
            param = ParameterRaster(tokens[0], desc, False)
        elif tokens[1].lower().strip() == "vector":
            param = ParameterVector(tokens[0],  desc, [ParameterVector.VECTOR_TYPE_ANY])
        elif tokens[1].lower().strip() == "table":
            param = ParameterTable(tokens[0], desc, False)
        elif tokens[1].lower().strip() == "multiple raster":
            param = ParameterMultipleInput(tokens[0], desc, ParameterMultipleInput.TYPE_RASTER)
            param.optional = False
        elif tokens[1].lower().strip() == "multiple vector":
            param = ParameterMultipleInput(tokens[0], desc, ParameterMultipleInput.TYPE_VECTOR_ANY)
            param.optional = False
        elif tokens[1].lower().strip().startswith("selection"):
            options = tokens[1].strip()[len("selection"):].split(";")
            param = ParameterSelection(tokens[0],  desc, options);
        elif tokens[1].lower().strip().startswith("boolean"):
            default = tokens[1].strip()[len("boolean")+1:]
            param = ParameterBoolean(tokens[0],  desc, default)
        elif tokens[1].lower().strip() == "extent":
            param = ParameterExtent(tokens[0],  desc)
        elif tokens[1].lower().strip() == "file":
            param = ParameterFile(tokens[0],  desc, False)
        elif tokens[1].lower().strip() == "folder":
            param = ParameterFile(tokens[0],  desc, True)
        elif tokens[    1].lower().strip().startswith("number"):
            default = tokens[1].strip()[len("number")+1:]
            param = ParameterNumber(tokens[0],  desc, default=default)
        elif tokens[1].lower().strip().startswith("field"):
            field = tokens[1].strip()[len("field")+1:]
            found = False
            for p in self.parameters:
                if p.name == field:
                    found = True
                    break
            if found:
                param = ParameterTableField(tokens[0],  tokens[0], field)
        elif tokens[1].lower().strip().startswith("string"):
            default = tokens[1].strip()[len("string")+1:]
            param = ParameterString(tokens[0],  desc, default)
        elif tokens[1].lower().strip().startswith("output raster"):
            out = OutputRaster()
        elif tokens[1].lower().strip().startswith("output vector"):
            out = OutputVector()
        elif tokens[1].lower().strip().startswith("output table"):
            out = OutputTable()
        elif tokens[1].lower().strip().startswith("output html"):
            out = OutputHTML()
        elif tokens[1].lower().strip().startswith("output file"):
            out = OutputFile()
        elif tokens[1].lower().strip().startswith("output number"):
            out = OutputNumber()
        elif tokens[1].lower().strip().startswith("output string"):
            out = OutputString()

        if param != None:
            self.addParameter(param)
        elif out != None:
            out.name = tokens[0]
            out.description = tokens[0]
            self.addOutput(out)
        else:
            raise WrongScriptException("Could not load script:" + self.descriptionFile + ".\n Problem with line \"" + line + "\"")

    def processDescriptionParameterLine(self, line):
        try:
            if line.startswith("Parameter"):
                self.addParameter(ParameterFactory.getFromString(line))
            elif line.startswith("*Parameter"):
                param = ParameterFactory.getFromString(line[1:])
                param.isAdvanced = True
                self.addParameter(param)
            else:
                self.addOutput(OutputFactory.getFromString(line))
        except Exception:
            raise WrongScriptException("Could not load script:" + self.descriptionFile + ".\n Problem with line \"" + line + "\"")

    def processAlgorithm(self, progress):

        script = "import sextante\n"

        ns = {}
        ns['progress'] = progress

        for param in self.parameters:
            ns[param.name] = param.value

        for out in self.outputs:
            ns[out.name] = out.value

        script+=self.script
        exec(script) in ns
        for out in self.outputs:
            out.setValue(ns[out.name])

    def helpFile(self):
        helpfile = self.descriptionFile + ".help"
        if os.path.exists(helpfile):
            h2h = Help2Html()
            return h2h.getHtmlFile(self, helpfile)
        else:
            return None

