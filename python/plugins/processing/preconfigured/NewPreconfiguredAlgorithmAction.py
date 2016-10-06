# -*- coding: utf-8 -*-

"""
***************************************************************************
    NewPreconfiguredAlgorithmAction.py
    ---------------------
    Date                 : April 2016
    Copyright            : (C) 2016 by Victor Olaya
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
__date__ = 'April 2016'
__copyright__ = '(C) 2016, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from processing.gui.ContextAction import ContextAction
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.preconfigured.PreconfiguredAlgorithmDialog import PreconfiguredAlgorithmDialog
from processing.preconfigured.PreconfiguredAlgorithm import PreconfiguredAlgorithm


class NewPreconfiguredAlgorithmAction(ContextAction):

    def __init__(self):
        self.name = self.tr('Create preconfigured algorithm', 'NewPreconfiguredAlgorithmAction')

    def isEnabled(self):
        return (isinstance(self.itemData, GeoAlgorithm) and
                not isinstance(self.itemData, PreconfiguredAlgorithm))

    def execute(self):
        alg = self.itemData.getCopy()  # make copy so we do not taint the original one in the dialog
        dlg = PreconfiguredAlgorithmDialog(alg, self.toolbox)
        dlg.exec_()
