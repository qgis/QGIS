# -*- coding: utf-8 -*-

"""
***************************************************************************
    MultilineTextPanel.py
    ---------------------
    Date                 : January 2013
    Copyright            : (C) 2013 by Victor Olaya
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
__date__ = 'January 2013'
__copyright__ = '(C) 2013, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class MultilineTextPanel(QtGui.QWidget):

    USE_TEXT = 0

    def __init__(self, options, model, parent = None):
        super(MultilineTextPanel, self).__init__(parent)
        self.options = options
        self.model = model
        self.verticalLayout = QtGui.QVBoxLayout(self)
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.combo = QtGui.QComboBox()
        self.combo.addItem("[Use text below]")
        for option in options:
            self.combo.addItem(option.name(), option)
        self.combo.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        self.verticalLayout.addWidget(self.combo)
        self.textBox = QtGui.QPlainTextEdit()
        self.verticalLayout.addWidget(self.textBox)
        self.setLayout(self.verticalLayout)

    def setText(self, text):
        self.textBox.setPlainText(text)

    def getOption(self):
        return self.combo.currentIndex()

    def getValue(self):
        if self.combo.currentIndex() == 0:
            return unicode(self.textBox.toPlainText())
        else:
            return self.combo.itemData(self.combo.currentIndex()).toPyObject()

    def setValue(self, value):
        items = [self.combo.itemData(i).toPyObject() for i in range(1,self.combo.count())]
        idx = 0
        for item in items:
            idx += 1
            if item and value:
                if item.alg == value.alg and item.param == value.param:
                    self.combo.setCurrentIndex(idx)
                    return
        self.combo.setCurrentIndex(0)
        value = self.model.getValueFromAlgorithmAndParameter(value)
        if value:
            self.textBox.setPlainText(str(value))
