# -*- coding: utf-8 -*-

"""
***************************************************************************
    FilterEdit.py
    ---------------------
    Date                 : October 2012
    Copyright            : (C) 2012 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'October 2012'
__copyright__ = '(C) 2012, Alexander Bruy'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4.QtCore import *
from PyQt4.QtGui import *

import sextante.resources_rc

class FilterLineEdit(QLineEdit):
    def __init__(self, parent=None, placeholder="Filter"):
        QLineEdit.__init__(self, parent)

        if hasattr(self, "setPlaceholderText"):
            self.setPlaceholderText(placeholder)

        self.btnClear = QToolButton(self)
        self.btnClear.setIcon(QIcon(":/sextante/images/clear.png"))
        self.btnClear.setCursor(Qt.ArrowCursor)
        self.btnClear.setStyleSheet("QToolButton { border: none; padding: 0px; }")
        self.btnClear.hide()

        self.btnClear.clicked.connect(self.clear)
        self.textChanged.connect(self.updateClearButton)

        frameWidth = self.style().pixelMetric(QStyle.PM_DefaultFrameWidth)
        self.setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").arg(self.btnClear.sizeHint().width() + frameWidth + 1))
        msz = self.minimumSizeHint()
        self.setMinimumSize(max(msz.width(), self.btnClear.sizeHint().height() + frameWidth * 2 + 2),
                            max(msz.height(), self.btnClear.sizeHint().height() + frameWidth * 2 + 2))

    def resizeEvent(self, event):
        sz = self.btnClear.sizeHint()
        frameWidth = self.style().pixelMetric(QStyle.PM_DefaultFrameWidth)
        self.btnClear.move(self.rect().right() - frameWidth - sz.width(),
                           (self.rect().bottom() + 1 - sz.height())/2)

    def updateClearButton(self, text):
        self.btnClear.setVisible(not text.isEmpty())
