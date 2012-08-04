#!/usr/bin/env python
# -*- coding: latin-1 -*-
from sextante.gui.ParametersPanel import ParametersPanel
from sextante.gui.AlgorithmExecutionDialog import AlgorithmExecutionDialog
from PyQt4 import QtCore


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
