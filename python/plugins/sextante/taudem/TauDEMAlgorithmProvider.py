# -*- coding: utf-8 -*-

"""
***************************************************************************
    TauDEMAlgorithmProvider.py
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

from PyQt4.QtGui import *

from sextante.core.AlgorithmProvider import AlgorithmProvider
from sextante.core.SextanteConfig import SextanteConfig
from sextante.core.SextanteConfig import Setting
from sextante.core.SextanteLog import SextanteLog

from sextante.taudem.TauDEMAlgorithm import TauDEMAlgorithm
from sextante.taudem.TauDEMUtils import TauDEMUtils

from sextante.taudem.peukerdouglas import PeukerDouglas
from sextante.taudem.slopearea import SlopeArea
from sextante.taudem.lengtharea import LengthArea
from sextante.taudem.dropanalysis import DropAnalysis
from sextante.taudem.dinfdistdown import DinfDistDown
from sextante.taudem.dinfdistup import DinfDistUp
from sextante.taudem.gridnet import GridNet
from sextante.taudem.dinftranslimaccum import DinfTransLimAccum
from sextante.taudem.dinftranslimaccum2 import DinfTransLimAccum2

class TauDEMAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.activate = False
        self.createAlgsList()

    def getDescription(self):
        return "TauDEM (hydrologic analysis)"

    def getName(self):
        return "taudem"

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + "/../images/taudem.png")

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)
        SextanteConfig.addSetting(Setting(self.getDescription(), TauDEMUtils.TAUDEM_FOLDER, "TauDEM command line tools folder", TauDEMUtils.taudemPath()))
        SextanteConfig.addSetting(Setting(self.getDescription(), TauDEMUtils.MPIEXEC_FOLDER, "MPICH2/OpenMPI bin directory", TauDEMUtils.mpiexecPath()))
        SextanteConfig.addSetting(Setting(self.getDescription(), TauDEMUtils.MPI_PROCESSES, "Number of MPI parallel processes to use", 2))

    def unload(self):
        AlgorithmProvider.unload(self)
        SextanteConfig.removeSetting(TauDEMUtils.TAUDEM_FOLDER)
        SextanteConfig.removeSetting(TauDEMUtils.MPIEXEC_FOLDER)
        SextanteConfig.removeSetting(TauDEMUtils.MPI_PROCESSES)

    def _loadAlgorithms(self):
        self.algs = self.preloadedAlgs

    def createAlgsList(self):
        self.preloadedAlgs = []
        folder = TauDEMUtils.taudemDescriptionPath()
        for descriptionFile in os.listdir(folder):
            if descriptionFile.endswith("txt"):
                try:
                    alg = TauDEMAlgorithm(os.path.join(folder, descriptionFile))
                    if alg.name.strip() != "":
                        self.preloadedAlgs.append(alg)
                    else:
                        SextanteLog.addToLog(SextanteLog.LOG_ERROR, "Could not open TauDEM algorithm: " + descriptionFile)
                except Exception, e:
                    SextanteLog.addToLog(SextanteLog.LOG_ERROR, "Could not open TauDEM algorithm: " + descriptionFile)

        self.preloadedAlgs.append(PeukerDouglas())
        self.preloadedAlgs.append(SlopeArea())
        self.preloadedAlgs.append(LengthArea())
        self.preloadedAlgs.append(DropAnalysis())
        self.preloadedAlgs.append(DinfDistDown())
        self.preloadedAlgs.append(DinfDistUp())
        self.preloadedAlgs.append(GridNet())
        self.preloadedAlgs.append(DinfTransLimAccum())
        self.preloadedAlgs.append(DinfTransLimAccum2())
