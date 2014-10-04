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

from PyQt4 import QtGui


class RangePanel(QtGui.QWidget):

    def __init__(self, param):
        super(RangePanel, self).__init__(None)
        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.labelmin = QtGui.QLabel()
        self.labelmin.setText(self.tr('Min'))
        self.textmin = QtGui.QLineEdit()
        self.textmin.setSizePolicy(QtGui.QSizePolicy.Expanding,
                                   QtGui.QSizePolicy.Expanding)
        self.labelmax = QtGui.QLabel()
        self.labelmax.setText(self.tr('Max'))
        self.textmax = QtGui.QLineEdit()
        self.textmin.setText(param.default.split(',')[0])
        self.textmax.setText(param.default.split(',')[1])
        self.textmax.setSizePolicy(QtGui.QSizePolicy.Expanding,
                                   QtGui.QSizePolicy.Expanding)
        self.horizontalLayout.addWidget(self.labelmin)
        self.horizontalLayout.addWidget(self.textmin)
        self.horizontalLayout.addWidget(self.labelmax)
        self.horizontalLayout.addWidget(self.textmax)
        self.setLayout(self.horizontalLayout)

    def getValue(self):
        return self.textmin.text() + ',' + self.textmax.text()
