# -*- coding: utf-8 -*-

"""
***************************************************************************
    NumberInputPanel.py
    ---------------------
    Date                 : August 2016
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
__date__ = 'August 2016'
__copyright__ = '(C) 2016, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt import uic
from qgis.PyQt.QtCore import pyqtSignal
from qgis.PyQt.QtWidgets import QDialog

from qgis.core import (QgsDataSourceUri,
                       QgsCredentials,
                       QgsExpression,
                       QgsRasterLayer)
from qgis.gui import QgsEncodingFileDialog, QgsExpressionBuilderDialog
from qgis.utils import iface

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'widgetBaseSelector.ui'))


class StringInputPanel(BASE, WIDGET):

    hasChanged = pyqtSignal()

    def __init__(self, param):
        super(StringInputPanel, self).__init__(None)
        self.setupUi(self)

        self.param = param
        self.text = param.default

        self.btnSelect.clicked.connect(self.showExpressionsBuilder)
        self.leText.textChanged.connect(lambda: self.hasChanged.emit())

    def showExpressionsBuilder(self):
        context = self.param.expressionContext()
        dlg = QgsExpressionBuilderDialog(None, self.leText.text(), self, 'generic', context)
        dlg.setWindowTitle(self.tr('Expression based input'))
        if dlg.exec_() == QDialog.Accepted:
            exp = QgsExpression(dlg.expressionText())
            if not exp.hasParserError():
                self.setValue(dlg.expressionText())

    
    def getValue(self):
        return self.leText.text()

    def setValue(self, value):
        self.leText.setText(unicode(value))
