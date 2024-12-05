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

__author__ = "Victor Olaya"
__date__ = "January 2013"
__copyright__ = "(C) 2013, Victor Olaya"

from qgis.PyQt.QtWidgets import (
    QComboBox,
    QPlainTextEdit,
    QSizePolicy,
    QVBoxLayout,
    QWidget,
)


class MultilineTextPanel(QWidget):
    USE_TEXT = 0

    def __init__(self, options, parent=None):
        super().__init__(parent)
        self.options = options
        self.verticalLayout = QVBoxLayout(self)
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.combo = QComboBox()
        self.combo.addItem(self.tr("[Use text below]"))
        for option in options:
            self.combo.addItem(option[0], option[1])
        self.combo.setSizePolicy(
            QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding
        )
        self.verticalLayout.addWidget(self.combo)
        self.textBox = QPlainTextEdit()
        self.verticalLayout.addWidget(self.textBox)
        self.setLayout(self.verticalLayout)

    def setText(self, text):
        self.textBox.setPlainText(text)

    def getOption(self):
        return self.combo.currentIndex()

    def getValue(self):
        if self.combo.currentIndex() == 0:
            return str(self.textBox.toPlainText())
        else:
            return self.combo.currentData()

    def setValue(self, value):
        items = [self.combo.itemData(i) for i in range(1, self.combo.count())]
        for idx, item in enumerate(items):
            if item == value:
                self.combo.setCurrentIndex(idx)
                return
        self.combo.setCurrentIndex(0)
        if value:
            self.textBox.setPlainText(value)
