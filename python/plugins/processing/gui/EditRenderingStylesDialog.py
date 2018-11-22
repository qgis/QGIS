# -*- coding: utf-8 -*-

"""
***************************************************************************
    EditRenderingStylesDialog.py
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
import warnings

from qgis.PyQt import uic
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtWidgets import QDialog, QHeaderView, QTableWidgetItem

from qgis.core import (QgsProcessingOutputRasterLayer,
                       QgsProcessingOutputVectorLayer)

from processing.gui.RenderingStyles import RenderingStyles
from processing.gui.RenderingStyleFilePanel import RenderingStyleFilePanel

pluginPath = os.path.split(os.path.dirname(__file__))[0]

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(
        os.path.join(pluginPath, 'ui', 'DlgRenderingStyles.ui'))


class EditRenderingStylesDialog(BASE, WIDGET):

    def __init__(self, alg):
        super(EditRenderingStylesDialog, self).__init__(None)
        self.setupUi(self)

        self.alg = alg

        self.tblStyles.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        self.setWindowTitle(self.alg.displayName())

        self.valueItems = {}
        self.dependentItems = {}
        self.setTableContent()

    def setTableContent(self):
        numOutputs = 0
        for output in self.alg.outputDefinitions():
            if isinstance(output, (QgsProcessingOutputVectorLayer, QgsProcessingOutputRasterLayer)):
                numOutputs += 1
        self.tblStyles.setRowCount(numOutputs)

        i = 0
        for output in self.alg.outputDefinitions():
            if isinstance(output, (QgsProcessingOutputVectorLayer, QgsProcessingOutputRasterLayer)):
                item = QTableWidgetItem(output.description() + '<' +
                                        output.__class__.__name__ + '>')
                item.setFlags(Qt.ItemIsEnabled)
                self.tblStyles.setItem(i, 0, item)
                item = RenderingStyleFilePanel()
                style = \
                    RenderingStyles.getStyle(self.alg.id(),
                                             output.name())
                if style:
                    item.setText(str(style))
                self.valueItems[output.name()] = item
                self.tblStyles.setCellWidget(i, 1, item)
                self.tblStyles.setRowHeight(i, 22)
            i += 1

    def accept(self):
        styles = {}
        for key in list(self.valueItems.keys()):
            styles[key] = str(self.valueItems[key].getValue())
        RenderingStyles.addAlgStylesAndSave(self.alg.id(), styles)

        QDialog.accept(self)

    def reject(self):
        QDialog.reject(self)
