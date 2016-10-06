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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt import uic
from qgis.PyQt.QtCore import pyqtSignal

from processing.gui.MultipleInputDialog import MultipleInputDialog
from processing.gui.MultipleFileInputDialog import MultipleFileInputDialog

pluginPath = os.path.split(os.path.dirname(__file__))[0]
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
            self.tr('%d elements selected') % len(self.selectedoptions))

    def showSelectionDialog(self):
        if self.datatype is None:
            dlg = MultipleInputDialog(self.options, self.selectedoptions)
        else:
            dlg = MultipleFileInputDialog(self.selectedoptions)
        dlg.exec_()
        if dlg.selectedoptions is not None:
            self.selectedoptions = dlg.selectedoptions
            self.leText.setText(
                self.tr('%d elements selected') % len(self.selectedoptions))
            self.selectionChanged.emit()

    def updateForOptions(self, options):
        selectedoptions = []
        selected = [self.options[i] for i in self.selectedoptions]
        for sel in selected:
            try:
                idx = options.index(sel)
                selectedoptions.append(idx)
            except ValueError:
                pass
        self.options = options
        self.setSelectedItems(selectedoptions)
