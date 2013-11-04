# -*- coding: utf-8 -*-

"""
***************************************************************************
    MultipleInputDialog.py
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

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from processing.ui.ui_DlgMultipleSelection import Ui_DlgMultipleSelection


class MultipleInputDialog(QDialog, Ui_DlgMultipleSelection):

    def __init__(self, options, selectedoptions=None):
        QDialog.__init__(self)
        self.setupUi(self)

        self.options = options
        self.selectedoptions = selectedoptions

        # Additional buttons
        self.btnSelectAll = QPushButton(self.tr('Select all'))
        self.buttonBox.addButton(self.btnSelectAll,
                                 QDialogButtonBox.ActionRole)
        self.btnClearSelection = QPushButton(self.tr('Clear selection'))
        self.buttonBox.addButton(self.btnClearSelection,
                                 QDialogButtonBox.ActionRole)
        self.btnToggleSelection = QPushButton(self.tr('Toggle selection'))
        self.buttonBox.addButton(self.btnToggleSelection,
                                 QDialogButtonBox.ActionRole)

        self.btnSelectAll.clicked.connect(self.selectAll)
        self.btnClearSelection.clicked.connect(self.lstLayers.clearSelection)
        self.btnToggleSelection.clicked.connect(self.toggleSelection)

        self.populateList()

    def populateList(self):
        self.lstLayers.clear()
        self.lstLayers.addItems(self.options)
        selModel = self.lstLayers.selectionModel()
        self.lstLayers.blockSignals(True)
        for i in xrange(self.lstLayers.count()):
            item = self.lstLayers.item(i)
            if self.lstLayers.indexFromItem(item).row() in self.selectedoptions:
                selModel.select(self.lstLayers.indexFromItem(item),
                                QItemSelectionModel.Select)
        self.lstLayers.blockSignals(False)

    def accept(self):
        self.selectedoptions = []
        for i in self.lstLayers.selectedItems():
            self.selectedoptions.append(self.lstLayers.indexFromItem(i).row())
        QDialog.accept(self)

    def reject(self):
        self.selectedoptions = None
        QDialog.reject(self)

    def selectAll(self):
        selModel = self.lstLayers.selectionModel()
        self.lstLayers.blockSignals(True)
        for i in xrange(self.lstLayers.count()):
            item = self.lstLayers.item(i)
            selModel.select(self.lstLayers.indexFromItem(item),
                            QItemSelectionModel.Select)
        self.lstLayers.blockSignals(False)

    def toggleSelection(self):
        selModel = self.lstLayers.selectionModel()
        self.lstLayers.blockSignals(True)
        for i in xrange(self.lstLayers.count()):
            item = self.lstLayers.item(i)
            selModel.select(self.lstLayers.indexFromItem(item),
                            QItemSelectionModel.Toggle)
        self.lstLayers.blockSignals(False)
