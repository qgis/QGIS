# -*- coding: utf-8 -*-
"""
/***************************************************************************
 ManageRepositoryDialog
                                 A QGIS plugin
 Download colllections shared by other users
                             -------------------
        begin                : 2016-05-29
        git sha              : $Format:%H$
        copyright            : (C) 2016 by Akbar Gumbira
        email                : akbargumbira@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""

from qgis.PyQt import uic
from qgis.PyQt.QtCore import Qt
try:
    from qgis.PyQt.QtGui import QDialog, QVBoxLayout, QDialogButtonBox
except ImportError:
    from qgis.PyQt.QtWidgets import QDialog, QVBoxLayout, QDialogButtonBox

from resource_sharing.utilities import ui_path, qgis_version

FORM_CLASS, _ = uic.loadUiType(ui_path('manage_repository.ui'))


class ManageRepositoryDialog(QDialog, FORM_CLASS):
    def __init__(self, parent=None):
        """Create the dialog and configure the UI."""
        super(ManageRepositoryDialog, self).__init__(parent)
        self.setupUi(self)
        self.line_edit_url.setText('http://')
        self.buttonBox.button(QDialogButtonBox.Ok).setEnabled(False)
        self.line_edit_name.textChanged.connect(self.form_changed)
        self.line_edit_url.textChanged.connect(self.form_changed)
        self.button_add_auth.clicked.connect(self.add_authentication)
        self.button_clear_auth.clicked.connect(self.line_edit_auth_id.clear)

        if qgis_version() < 21200:
            self.disable_authentication()

    def disable_authentication(self):
        """Disable adding authentication by hiding all the widgets."""
        self.label_auth.hide()
        self.line_edit_auth_id.hide()
        self.button_add_auth.hide()
        self.button_clear_auth.hide()

    def form_changed(self):
        """Slot for when the form changed."""
        is_enabled = (len(self.line_edit_name.text()) > 0 and
                      len(self.line_edit_url.text()) > 0)
        self.buttonBox.button(QDialogButtonBox.Ok).setEnabled(is_enabled)

    def add_authentication(self):
        """Slot for when the add auth button is clicked."""
        if qgis_version() >= 21200:
            from qgis.gui import QgsAuthConfigSelect

            dlg = QDialog(self)
            dlg.setWindowTitle(self.tr("Select Authentication"))
            layout = QVBoxLayout(dlg)

            acs = QgsAuthConfigSelect(dlg)
            if self.line_edit_auth_id.text():
                acs.setConfigId(self.line_edit_auth_id.text())
            layout.addWidget(acs)

            button_box = QDialogButtonBox(
                QDialogButtonBox.Ok | QDialogButtonBox.Cancel,
                Qt.Horizontal,
                dlg)
            layout.addWidget(button_box)
            button_box.accepted.connect(dlg.accept)
            button_box.rejected.connect(dlg.close)

            dlg.setLayout(layout)
            dlg.setWindowModality(Qt.WindowModal)
            if dlg.exec_():
                self.line_edit_auth_id.setText(acs.configId())
            del dlg
