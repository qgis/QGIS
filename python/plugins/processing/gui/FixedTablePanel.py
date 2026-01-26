"""
***************************************************************************
    FixedTablePanel.py
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

__author__ = "Victor Olaya"
__date__ = "August 2012"
__copyright__ = "(C) 2012, Victor Olaya"

import os
import warnings

from qgis.PyQt import uic

from processing.gui.FixedTableDialog import FixedTableDialog

pluginPath = os.path.split(os.path.dirname(__file__))[0]

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(
        os.path.join(pluginPath, "ui", "widgetBaseSelector.ui")
    )


class FixedTablePanel(BASE, WIDGET):

    def __init__(self, param, parent=None):
        super().__init__(parent)
        self.setupUi(self)

        self.leText.setEnabled(False)

        self.param = param

        # NOTE - table IS squashed to 1-dimensional!
        self.table = []
        for row in range(param.numberRows()):
            for col in range(len(param.headers())):
                self.table.append("0")

        self.leText.setText(
            self.tr("Fixed table {0}x{1}").format(
                param.numberRows(), len(param.headers())
            )
        )

        self.btnSelect.clicked.connect(self.showFixedTableDialog)

    def updateSummaryText(self):
        self.leText.setText(
            self.tr("Fixed table {0}x{1}").format(
                len(self.table) // len(self.param.headers()), len(self.param.headers())
            )
        )

    def setValue(self, value):
        self.table = value
        self.updateSummaryText()

    def showFixedTableDialog(self):
        dlg = FixedTableDialog(self.param, self.table)
        dlg.exec()
        if dlg.rettable is not None:
            self.setValue(dlg.rettable)
        dlg.deleteLater()
