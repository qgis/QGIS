# -*- coding: utf-8 -*-

"""
***************************************************************************
    TauDEMAlgorithm.py
    ---------------------
    Date                 : October 2012
    Copyright            : (C) 2012 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""


__author__ = 'Alexander Bruy'
__date__ = 'October 2012'
__copyright__ = '(C) 2012, Alexander Bruy'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import *

from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.SextanteLog import SextanteLog
from sextante.core.SextanteUtils import SextanteUtils
from sextante.core.SextanteConfig import SextanteConfig
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException

from sextante.parameters.ParameterFactory import ParameterFactory
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.parameters.ParameterString import ParameterString
from sextante.parameters.ParameterNumber import ParameterNumber

from sextante.outputs.OutputFactory import OutputFactory
from sextante.outputs.OutputRaster import OutputRaster
from sextante.outputs.OutputVector import OutputVector
from sextante.outputs.OutputFile import OutputFile

from sextante.taudem.TauDEMUtils import TauDEMUtils

class TauDEMAlgorithm(GeoAlgorithm):

    def __init__(self, descriptionfile):
        GeoAlgorithm.__init__(self)
        self.descriptionFile = descriptionfile
        self.defineCharacteristicsFromFile()

    def getCopy(self):
        newone = TauDEMAlgorithm(self.descriptionFile)
        newone.provider = self.provider
        return newone

    def getIcon(self):
        return  QIcon(os.path.dirname(__file__) + "/../images/taudem.png")

    def defineCharacteristicsFromFile(self):
        lines = open(self.descriptionFile)
        line = lines.readline().strip("\n").strip()
        self.name = line
        line = lines.readline().strip("\n").strip()
        self.cmdName = line
        line = lines.readline().strip("\n").strip()
        self.group = line
        while line != "":
          try:
              line = line.strip("\n").strip()
              if line.startswith("Parameter"):
                  param = ParameterFactory.getFromString(line)
                  self.addParameter(param)
              else:
                  self.addOutput(OutputFactory.getFromString(line))
              line = lines.readline().strip("\n").strip()
          except Exception, e:
              SextanteLog.addToLog(SextanteLog.LOG_ERROR, "Could not load TauDEM algorithm: " + self.descriptionFile + "\n" + line)
              raise e
        lines.close()

    def processAlgorithm(self, progress):
        commands = []
        commands.append(os.path.join(TauDEMUtils.mpiexecPath(), "mpiexec"))

        processNum = SextanteConfig.getSetting(TauDEMUtils.MPI_PROCESSES)
        if processNum <= 0:
          raise GeoAlgorithmExecutionException("Wrong number of MPI processes used.\nPlease set correct number before running TauDEM algorithms.")

        commands.append("-n")
        commands.append(str(processNum))
        commands.append(os.path.join(TauDEMUtils.taudemPath(), self.cmdName))

        for param in self.parameters:
            if param.value == None or param.value == "":
                continue
            if isinstance(param, ParameterNumber):
                commands.append(param.name)
                commands.append(str(param.value))
            if isinstance(param, (ParameterRaster, ParameterVector)):
                commands.append(param.name)
                commands.append(param.value)
            elif isinstance(param, ParameterBoolean):
                if param.value and str(param.value).lower() == "false":
                    commands.append(param.name)
            elif isinstance(param, ParameterString):
                commands.append(param.name)
                commands.append(str(param.value))

        for out in self.outputs:
            commands.append(out.name)
            commands.append(out.value)

        loglines = []
        loglines.append("TauDEM execution command")
        for line in commands:
            loglines.append(line)
        SextanteLog.addToLog(SextanteLog.LOG_INFO, loglines)
        TauDEMUtils.executeTauDEM(commands, progress)

    #def helpFile(self):
    #    return os.path.join(os.path.dirname(__file__), "help", self.cmdName + ".html")
