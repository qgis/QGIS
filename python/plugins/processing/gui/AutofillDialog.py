# -*- coding: utf-8 -*-

"""
***************************************************************************
    AutofillDialog.py
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

from PyQt4 import uic
from PyQt4.QtGui import QDialog

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'DlgAutofill.ui'))


class AutofillDialog(BASE, WIDGET):

    DO_NOT_AUTOFILL = 0
    FILL_WITH_NUMBERS = 1
    FILL_WITH_PARAMETER = 2

    def __init__(self, alg):
        super(AutofillDialog, self).__init__(None)
        self.setupUi(self)

        self.cmbFillType.currentIndexChanged.connect(self.toggleParameters)

        for param in alg.parameters:
            self.cmbParameters.addItem(param.description)

    def toggleParameters(self, index):
        if index == self.FILL_WITH_PARAMETER:
            self.lblParameters.setEnabled(True)
            self.cmbParameters.setEnabled(True)
        else:
            self.lblParameters.setEnabled(False)
            self.cmbParameters.setEnabled(False)

    def accept(self):
        self.mode = self.cmbFillType.currentIndex()
        self.param = self.cmbParameters.currentIndex()
        QDialog.accept(self)

    def reject(self):
        self.mode = None
        self.param = None
        QDialog.reject(self)
