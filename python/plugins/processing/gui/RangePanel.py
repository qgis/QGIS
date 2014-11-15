# -*- coding: utf-8 -*-

"""
***************************************************************************
    RangePanel.py
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

from PyQt4.QtGui import *

from processing.ui.ui_widgetRangeSelector import Ui_Form


class RangePanel(QWidget, Ui_Form):

    def __init__(self, param):
        QWidget.__init__(self)
        self.setupUi(self)

        self.isInteger = param.isInteger
        if self.isInteger:
            self.spnMin.setDecimals(0)
            self.spnMax.setDecimals(0)

        values = param.default.split(',')
        minVal = float(values[0])
        maxVal = float(values[1])
        self.spnMin.setValue(minVal)
        self.spnMax.setValue(maxVal)

        self.spnMin.setMaximum(maxVal)
        self.spnMin.setMinimum(minVal)

        self.spnMax.setMaximum(maxVal)
        self.spnMax.setMinimum(minVal)

    def getValue(self):
        return '{},{}'.format(self.spnMin.value(), self.spnMax.value())
