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

from PyQt4.QtGui import QIcon

from processing.core.AlgorithmProvider import AlgorithmProvider
from processing.core.ProcessingConfig import ProcessingConfig, Setting
from processing.core.ProcessingLog import ProcessingLog

from TauDEMAlgorithm import TauDEMAlgorithm
from TauDEMMultifileAlgorithm import TauDEMMultifileAlgorithm
from TauDEMUtils import TauDEMUtils

from peukerdouglas import PeukerDouglas
from slopearea import SlopeArea
from lengtharea import LengthArea
from dropanalysis import DropAnalysis
from dinfdistdown import DinfDistDown
from dinfdistup import DinfDistUp
from gridnet import GridNet
from dinftranslimaccum import DinfTransLimAccum
from dinftranslimaccum2 import DinfTransLimAccum2

from peukerdouglas_multi import PeukerDouglasMulti
from slopearea_multi import SlopeAreaMulti
from lengtharea_multi import LengthAreaMulti
from dropanalysis_multi import DropAnalysisMulti
from dinfdistdown_multi import DinfDistDownMulti
from dinfdistup_multi import DinfDistUpMulti
from gridnet_multi import GridNetMulti
from dinftranslimaccum_multi import DinfTransLimAccumMulti
from dinftranslimaccum2_multi import DinfTransLimAccum2Multi


class TauDEMAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.activate = False

    def getDescription(self):
        return self.tr('TauDEM (hydrologic analysis)')

    def getName(self):
        return 'taudem'

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + '/../../images/taudem.png')

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)

        ProcessingConfig.addSetting(Setting(self.getDescription(),
            TauDEMUtils.TAUDEM_FOLDER,
            self.tr('TauDEM command line tools folder'),
            TauDEMUtils.taudemPath(), valuetype=Setting.FOLDER))
        ProcessingConfig.addSetting(Setting(self.getDescription(),
            TauDEMUtils.TAUDEM_MULTIFILE_FOLDER,
            self.tr('TauDEM multifile command line tools folder'),
            TauDEMUtils.taudemMultifilePath(), valuetype=Setting.FOLDER))
        ProcessingConfig.addSetting(Setting(self.getDescription(),
            TauDEMUtils.TAUDEM_USE_SINGLEFILE,
            self.tr('Enable singlefile TauDEM tools'), True))
        ProcessingConfig.addSetting(Setting(self.getDescription(),
            TauDEMUtils.TAUDEM_USE_MULTIFILE,
            self.tr('Enable multifile TauDEM tools'), False))
        ProcessingConfig.addSetting(Setting(self.getDescription(),
            TauDEMUtils.MPIEXEC_FOLDER,
            self.tr('MPICH2/OpenMPI bin directory'),
            TauDEMUtils.mpiexecPath(), valuetype=Setting.FOLDER))
        ProcessingConfig.addSetting(Setting(self.getDescription(),
            TauDEMUtils.MPI_PROCESSES,
            self.tr('Number of MPI parallel processes to use'), 2))

    def unload(self):
        AlgorithmProvider.unload(self)

        ProcessingConfig.removeSetting(TauDEMUtils.TAUDEM_FOLDER)
        ProcessingConfig.removeSetting(TauDEMUtils.TAUDEM_MULTIFILE_FOLDER)
        ProcessingConfig.removeSetting(TauDEMUtils.TAUDEM_USE_SINGLEFILE)
        ProcessingConfig.removeSetting(TauDEMUtils.TAUDEM_USE_MULTIFILE)
        ProcessingConfig.removeSetting(TauDEMUtils.MPIEXEC_FOLDER)
        ProcessingConfig.removeSetting(TauDEMUtils.MPI_PROCESSES)

    def _loadAlgorithms(self):
        self.algs = []
        basePath = TauDEMUtils.taudemDescriptionPath()

        if ProcessingConfig.getSetting(TauDEMUtils.TAUDEM_USE_SINGLEFILE):
            folder = os.path.join(basePath, 'single')

            for descriptionFile in os.listdir(folder):
                if descriptionFile.endswith('txt'):
                    descriptionFile = os.path.join(folder, descriptionFile)
                    self._algFromDescription(descriptionFile)

            self.algs.append(PeukerDouglas())
            self.algs.append(SlopeArea())
            self.algs.append(LengthArea())
            self.algs.append(DropAnalysis())
            self.algs.append(DinfDistDown())
            self.algs.append(DinfDistUp())
            self.algs.append(GridNet())
            self.algs.append(DinfTransLimAccum())
            self.algs.append(DinfTransLimAccum2())

        if ProcessingConfig.getSetting(TauDEMUtils.TAUDEM_USE_MULTIFILE):
            folder = os.path.join(basePath, 'multi')

            for descriptionFile in os.listdir(folder):
                if descriptionFile.endswith('txt'):
                    descriptionFile = os.path.join(folder, descriptionFile)
                    self._algFromDescription(descriptionFile, True)

            self.algs.append(PeukerDouglasMulti())
            self.algs.append(SlopeAreaMulti())
            self.algs.append(LengthAreaMulti())
            self.algs.append(DropAnalysisMulti())
            self.algs.append(DinfDistDownMulti())
            self.algs.append(DinfDistUpMulti())
            self.algs.append(GridNetMulti())
            self.algs.append(DinfTransLimAccumMulti())
            self.algs.append(DinfTransLimAccum2Multi())

    def _algFromDescription(self, descriptionFile, multifile=False):
        try:
            if multifile:
                alg = TauDEMMultifileAlgorithm(descriptionFile)
            else:
                alg = TauDEMAlgorithm(descriptionFile)
            if alg.name.strip() != '':
                self.algs.append(alg)
            else:
                ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
                    self.tr('Could not open TauDEM algorithm: %s' % descriptionFile))
        except Exception, e:
            ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
                self.tr('Could not open TauDEM algorithm %s:\n%s' % (descriptionFile, str(e))))
