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
        self.btnSelectAll = QPushButton(self.tr('(de)Select all'))
        self.buttonBox.addButton(self.btnSelectAll,
                                 QDialogButtonBox.ActionRole)

        self.btnSelectAll.clicked.connect(self.toggleSelection)

        self.setTableContent()

    def setTableContent(self):
        self.tblLayers.setRowCount(len(self.options))
        for i in range(len(self.options)):
            item = QCheckBox()
            item.setText(self.options[i])
            if i in self.selectedoptions:
                item.setChecked(True)
            self.tblLayers.setCellWidget(i, 0, item)

    def accept(self):
        self.selectedoptions = []
        for i in range(len(self.options)):
            widget = self.tblLayers.cellWidget(i, 0)
            if widget.isChecked():
                self.selectedoptions.append(i)
        QDialog.accept(self)

    def reject(self):
        self.selectedoptions = None
        QDialog.reject(self)

    def toggleSelection(self):
        checked = False
        for i in range(len(self.options)):
            widget = self.tblLayers.cellWidget(i, 0)
            if not widget.isChecked():
                checked = True
                break
        for i in range(len(self.options)):
            widget = self.tblLayers.cellWidget(i, 0)
            widget.setChecked(checked)
