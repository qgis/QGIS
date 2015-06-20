# -*- coding: utf-8 -*-

"""
***************************************************************************
    NumberInputPanel.py
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

from processing.gui.NumberInputDialog import NumberInputDialog

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'widgetNumberSelector.ui'))


class NumberInputPanel(BASE, WIDGET):

    def __init__(self, number, minimum, maximum, isInteger):
        super(NumberInputPanel, self).__init__(None)
        self.setupUi(self)

        self.isInteger = isInteger
        if self.isInteger:
            self.spnValue.setDecimals(0)
            if maximum:
                self.spnValue.setMaximum(maximum)
            else:
                self.spnValue.setMaximum(99999999)
            if minimum:
                self.spnValue.setMinimum(minimum)
            else:
                self.spnValue.setMinimum(-99999999)

        self.spnValue.setValue(float(number))

        self.btnCalc.clicked.connect(self.showNumberInputDialog)

    def showNumberInputDialog(self):
        dlg = NumberInputDialog(self.isInteger)
        dlg.exec_()
        if dlg.value is not None:
            self.spnValue.setValue(dlg.value)

    def getValue(self):
        return self.spnValue.value()
