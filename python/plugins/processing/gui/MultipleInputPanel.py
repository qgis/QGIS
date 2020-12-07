# -*- coding: utf-8 -*-

"""
***************************************************************************
    MultipleInputPanel.py
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

import os
import warnings

from qgis.core import QgsProcessing
from qgis.PyQt import uic
from qgis.PyQt.QtCore import pyqtSignal

''
from processing.gui.MultipleInputDialog import MultipleInputDialog
from processing.gui.MultipleFileInputDialog import MultipleFileInputDialog

pluginPath = os.path.split(os.path.dirname(__file__))[0]

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(
        os.path.join(pluginPath, 'ui', 'widgetBaseSelector.ui'))


class MultipleInputPanel(BASE, WIDGET):
    selectionChanged = pyqtSignal()

    def __init__(self, options=None, datatype=None):
        super(MultipleInputPanel, self).__init__(None)
        self.setupUi(self)

        self.leText.setEnabled(False)
        self.leText.setText(self.tr('0 elements selected'))

        self.btnSelect.clicked.connect(self.showSelectionDialog)

        self.options = options
        self.datatype = datatype
        self.selectedoptions = []

    def setSelectedItems(self, selected):
        # No checking is performed!
        self.selectedoptions = selected
        self.leText.setText(
            self.tr('{0} elements selected').format(len(self.selectedoptions)))

    def showSelectionDialog(self):
        if self.datatype == QgsProcessing.TypeFile:
            dlg = MultipleFileInputDialog(self.selectedoptions)
        else:
            dlg = MultipleInputDialog(self.options, self.selectedoptions, datatype=self.datatype)
        dlg.exec_()
        if dlg.selectedoptions is not None:
            self.selectedoptions = dlg.selectedoptions
            self.leText.setText(
                self.tr('{0} elements selected').format(len(self.selectedoptions)))
            self.selectionChanged.emit()

    def updateForOptions(self, options):
        selectedoptions = []
        selected = [self.options[i] if isinstance(i, int) else i for i in self.selectedoptions]
        for sel in selected:
            if not isinstance(sel, int):
                try:
                    idx = options.index(sel)
                    selectedoptions.append(idx)
                except ValueError:
                    pass
            else:
                selectedoptions.append(sel)
        self.options = options
        self.setSelectedItems(selectedoptions)
