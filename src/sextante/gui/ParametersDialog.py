#!/usr/bin/env python
# -*- coding: latin-1 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtCore, QtGui, QtWebKit
from sextante.core.QGisLayers import QGisLayers
from sextante.gui.SextantePostprocessing import SextantePostprocessing
from sextante.gui.AlgorithmExecutionDialog import AlgorithmExecutionDialog
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.parameters.ParameterMultipleInput import ParameterMultipleInput
from sextante.parameters.ParameterFixedTable import ParameterFixedTable
from sextante.parameters.ParameterTableField import ParameterTableField
from sextante.parameters.ParameterTable import ParameterTable
from sextante.gui.AlgorithmExecutor import AlgorithmExecutor
from sextante.core.SextanteLog import SextanteLog
#~ from sextante.gui.SextantePostprocessing import SextantePostprocessing
from sextante.parameters.ParameterRange import ParameterRange
from sextante.parameters.ParameterNumber import ParameterNumber

from sextante.gui.ParametersPanel import ParametersPanel
from sextante.parameters.ParameterFile import ParameterFile
from sextante.parameters.ParameterCrs import ParameterCrs
from sextante.core.SextanteConfig import SextanteConfig
from sextante.parameters.ParameterExtent import ParameterExtent
from sextante.outputs.OutputHTML import OutputHTML
from sextante.outputs.OutputRaster import OutputRaster
from sextante.outputs.OutputVector import OutputVector
from sextante.outputs.OutputTable import OutputTable
from sextante.core.WrongHelpFileException import WrongHelpFileException
import os
from sextante.gui.UnthreadedAlgorithmExecutor import UnthreadedAlgorithmExecutor

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class ParametersDialog(AlgorithmExecutionDialog):

    NOT_SELECTED = "[Not selected]"
    '''the default parameters dialog, to be used when an algorithm is called from the toolbox'''
    def __init__(self, alg):
        self.paramTable = ParametersPanel(self, alg)        
        AlgorithmExecutionDialog.__init__(self, alg, self.paramTable)
        self.executed = False
