# -*- coding: utf-8 -*-

"""
***************************************************************************
    SextanteAlgorithmProvider.py
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

from sextante.algs.EquivalentNumField import EquivalentNumField
from sextante.core.AlgorithmProvider import AlgorithmProvider
from sextante.algs.AddTableField import AddTableField
from PyQt4 import QtGui
import os
from sextante.algs.FieldsCalculator import FieldsCalculator
from sextante.algs.SaveSelectedFeatures import SaveSelectedFeatures
from sextante.algs.Explode import Explode
from sextante.algs.AutoincrementalField import AutoincrementalField
from sextante.algs.FieldPyculator import FieldsPyculator

class SextanteAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.alglist = [AddTableField(), FieldsCalculator(), SaveSelectedFeatures(),
                        AutoincrementalField(), Explode(), FieldsPyculator(), EquivalentNumField()]

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)


    def unload(self):
        AlgorithmProvider.unload(self)


    def getName(self):
        return "sextante"

    def getDescription(self):
        return "SEXTANTE geoalgorithms"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/toolbox.png")

    def _loadAlgorithms(self):
        self.algs = self.alglist

    def supportsNonFileBasedOutput(self):
        return True