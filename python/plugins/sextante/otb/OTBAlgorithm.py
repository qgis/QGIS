# -*- coding: utf-8 -*-

"""
***************************************************************************
    OTBAlgorithm.py
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
from qgis.core import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.parameters.ParameterMultipleInput import ParameterMultipleInput
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.core.SextanteLog import SextanteLog
from sextante.core.SextanteUtils import SextanteUtils
from sextante.core.WrongHelpFileException import WrongHelpFileException
from sextante.parameters.ParameterFactory import ParameterFactory
from sextante.outputs.OutputFactory import OutputFactory
from sextante.otb.OTBUtils import OTBUtils
from sextante.parameters.ParameterExtent import ParameterExtent

class OTBAlgorithm(GeoAlgorithm):

    REGION_OF_INTEREST = "ROI"

    def __init__(self, descriptionfile):
        GeoAlgorithm.__init__(self)
        self.roiFile = None
        self.descriptionFile = descriptionfile
        self.defineCharacteristicsFromFile()
        self.numExportedLayers = 0
        self.hasROI = None;

    def getCopy(self):
        newone = OTBAlgorithm(self.descriptionFile)
        newone.provider = self.provider
        return newone

    def getIcon(self):
        return  QIcon(os.path.dirname(__file__) + "/../images/otb.png")

    def helpFile(self):
        folder = os.path.join( OTBUtils.otbDescriptionPath(), 'doc' )
        helpfile = os.path.join( str(folder), self.appkey + ".html")
        if os.path.exists(helpfile):
            return helpfile
        else:
            raise WrongHelpFileException("Could not find help file for this algorithm. \nIf you have it put it in: \n"+str(folder))

    def defineCharacteristicsFromFile(self):
        lines = open(self.descriptionFile)
        line = lines.readline().strip("\n").strip()
        self.appkey = line
        line = lines.readline().strip("\n").strip()
        self.cliName = line
        line = lines.readline().strip("\n").strip()
        self.name = line
        line = lines.readline().strip("\n").strip()
        self.group = line
        while line != "":
            try:
                line = line.strip("\n").strip()
                if line.startswith("Parameter"):
                    param = ParameterFactory.getFromString(line)
                    # Hack for initializing the elevation parameters from Sextante configuration
                    if param.name == "-elev.dem.path" or param.name == "-elev.dem":
                        param.default = OTBUtils.otbSRTMPath()
                    elif param.name == "-elev.dem.geoid" or param.name == "-elev.geoid":
                        param.default = OTBUtils.otbGeoidPath()
                    self.addParameter(param)
                elif line.startswith("*Parameter"):
                    param = ParameterFactory.getFromString(line[1:])
                    param.isAdvanced = True
                    self.addParameter(param)
                elif line.startswith("Extent"):
                    self.addParameter(ParameterExtent(self.REGION_OF_INTEREST, "Region of interest", "0,1,0,1"))
                    self.hasROI = True
                else:
                    self.addOutput(OutputFactory.getFromString(line))
                line = lines.readline().strip("\n").strip()
            except Exception,e:
                SextanteLog.addToLog(SextanteLog.LOG_ERROR, "Could not open OTB algorithm: " + self.descriptionFile + "\n" + line)
                raise e
        lines.close()


    def processAlgorithm(self, progress):
        path = OTBUtils.otbPath()
        libpath = OTBUtils.otbLibPath()
        if path == "" or libpath == "":
            raise GeoAlgorithmExecutionException("OTB folder is not configured.\nPlease configure it before running OTB algorithms.")

        commands = []
        commands.append(path + os.sep + self.cliName)

        self.roiVectors = {}
        self.roiRasters = {}
        for param in self.parameters:
            if param.value == None or param.value == "":
                continue
            if isinstance(param, ParameterVector):
                commands.append(param.name)
                if self.hasROI:
                    roiFile = SextanteUtils.getTempFilename('shp')
                    commands.append(roiFile)
                    self.roiVectors[param.value] = roiFile
                else:
                    commands.append("\"" + param.value+ "\"")
            elif isinstance(param, ParameterRaster):
                commands.append(param.name)
                if self.hasROI:
                    roiFile = SextanteUtils.getTempFilename('tif')
                    commands.append(roiFile)
                    self.roiRasters[param.value] = roiFile
                else:
                    commands.append("\"" + param.value+ "\"")
            elif isinstance(param, ParameterMultipleInput):
                commands.append(param.name)
                files = str(param.value).split(";")
                paramvalue = " ".join(["\"" + f + "\"" for f in files])                
                commands.append(paramvalue)
            elif isinstance(param, ParameterSelection):
                commands.append(param.name)
                idx = int(param.value)
                commands.append(str(param.options[idx]))
            elif isinstance(param, ParameterBoolean):
                if param.value:
                    commands.append(param.name)
                    commands.append(str(param.value).lower())
            elif isinstance(param, ParameterExtent):
                self.roiValues = param.value.split(",")
            else:
                commands.append(param.name)
                commands.append(str(param.value))

        for out in self.outputs:
            commands.append(out.name)
            commands.append('"' + out.value + '"')
        for roiInput, roiFile in self.roiRasters.items():
            startX, startY = float(self.roiValues[0]), float(self.roiValues[1])
            sizeX = float(self.roiValues[2]) - startX
            sizeY = float(self.roiValues[3]) - startY
            helperCommands = [
                    "otbcli_ExtractROI",
                    "-in",       roiInput,
                    "-out",      roiFile,
                    "-startx",   str(startX),
                    "-starty",   str(startY),
                    "-sizex",    str(sizeX),
                    "-sizey",    str(sizeY)]
            SextanteLog.addToLog(SextanteLog.LOG_INFO, helperCommands)
            progress.setCommand(helperCommands)
            OTBUtils.executeOtb(helperCommands, progress)

        if self.roiRasters:
            supportRaster = self.roiRasters.itervalues().next()
            for roiInput, roiFile in self.roiVectors.items():
                helperCommands = [
                        "otbcli_VectorDataExtractROIApplication",
                        "-vd.in",           roiInput,
                        "-io.in",           supportRaster,
                        "-io.out",          roiFile,
                        "-elev.dem.path",   OTBUtils.otbSRTMPath()]
                SextanteLog.addToLog(SextanteLog.LOG_INFO, helperCommands)
                progress.setCommand(helperCommands)
                OTBUtils.executeOtb(helperCommands, progress)

        loglines = []
        loglines.append("OTB execution command")
        for line in commands:
            loglines.append(line)
            progress.setCommand(line)

        SextanteLog.addToLog(SextanteLog.LOG_INFO, loglines)

        OTBUtils.executeOtb(commands, progress)
