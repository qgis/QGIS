# -*- coding:utf-8 -*-
"""
/***************************************************************************
                           qgsplugininstallerrepositorydialog.py
                           Plugin Installer module
                             -------------------
    Date                 : June 2013
    Copyright            : (C) 2013 by Borys Jurgiel
    Email                : info at borysjurgiel dot pl

    This module is based on former plugin_installer plugin:
      Copyright (C) 2007-2008 Matthew Perry
      Copyright (C) 2008-2013 Borys Jurgiel

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

from qgis.gui import QgsAuthConfigSelect
from qgis.PyQt.QtWidgets import QDialog, QDialogButtonBox, QVBoxLayout
from qgis.PyQt.QtCore import Qt

from .ui_qgsplugininstallerrepositorybase import Ui_QgsPluginInstallerRepositoryDetailsDialogBase


class QgsPluginInstallerRepositoryDialog(QDialog, Ui_QgsPluginInstallerRepositoryDetailsDialogBase):
    # ----------------------------------------- #

    def __init__(self, parent=None):
        QDialog.__init__(self, parent)
        self.setupUi(self)
        self.editURL.setText("http://")
        self.editName.textChanged.connect(self.textChanged)
        self.editURL.textChanged.connect(self.textChanged)
        self.btnClearAuthCfg.clicked.connect(self.editAuthCfg.clear)
        self.btnEditAuthCfg.clicked.connect(self.editAuthCfgId)
        self.textChanged(None)

    # ----------------------------------------- #
    def textChanged(self, string):
        enable = (len(self.editName.text()) > 0 and len(self.editURL.text()) > 0)
        self.buttonBox.button(QDialogButtonBox.Ok).setEnabled(enable)

    def editAuthCfgId(self):
        dlg = QDialog(self)
        dlg.setWindowModality(Qt.WindowModal)
        layout = QVBoxLayout()
        selector = QgsAuthConfigSelect(self)
        if self.editAuthCfg.text():
            selector.setConfigId(self.editAuthCfg.text())
        layout.addWidget(selector)
        buttonBox = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Close)
        buttonBox.accepted.connect(dlg.accept)
        buttonBox.rejected.connect(dlg.reject)
        layout.addWidget(buttonBox)
        dlg.setLayout(layout)
        if dlg.exec_():
            self.editAuthCfg.setText(selector.configId())
        del dlg
