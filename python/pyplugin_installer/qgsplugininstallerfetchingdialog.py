"""
/***************************************************************************
                           qgsplugininstallerfetchingdialog.py
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

from pathlib import Path

from qgis.PyQt import uic
from qgis.PyQt.QtCore import Qt, QCoreApplication
from qgis.PyQt.QtWidgets import QDialog, QTreeWidgetItem

from .installer_data import repositories

from qgis.gui import QgsGui

Ui_QgsPluginInstallerFetchingDialogBase, _ = uic.loadUiType(
    Path(__file__).parent / "qgsplugininstallerfetchingbase.ui"
)


class QgsPluginInstallerFetchingDialog(
    QDialog, Ui_QgsPluginInstallerFetchingDialogBase
):
    # ----------------------------------------- #

    def __init__(self, parent):
        QDialog.__init__(self, parent)
        self.setupUi(self)
        QgsGui.instance().enableAutoGeometryRestore(self)

        self.progressBar.setRange(0, len(repositories.allEnabled()) * 100)
        self.itemProgress = {}
        self.item = {}
        for key in repositories.allEnabled():
            self.item[key] = QTreeWidgetItem(self.treeWidget)
            self.item[key].setText(0, key)
            if repositories.all()[key]["state"] > 1:
                self.itemProgress[key] = 100
                self.displayState(key, 0)
            else:
                self.itemProgress[key] = 0
                self.displayState(key, 2)
        self.treeWidget.resizeColumnToContents(0)
        self.treeWidget.setCursor(Qt.CursorShape.WaitCursor)
        repositories.repositoryFetched.connect(self.repositoryFetched)
        repositories.anythingChanged.connect(self.displayState)

    # ----------------------------------------- #
    def displayState(self, key, state, state2=None):
        messages = [
            self.tr("Success"),
            QCoreApplication.translate(
                "QgsPluginInstallerFetchingDialog", "Resolving host name…"
            ),
            QCoreApplication.translate(
                "QgsPluginInstallerFetchingDialog", "Connecting…"
            ),
            QCoreApplication.translate(
                "QgsPluginInstallerFetchingDialog", "Host connected. Sending request…"
            ),
            QCoreApplication.translate(
                "QgsPluginInstallerFetchingDialog", "Downloading data…"
            ),
            self.tr("Idle"),
            QCoreApplication.translate(
                "QgsPluginInstallerFetchingDialog", "Closing connection…"
            ),
            self.tr("Error"),
        ]
        message = messages[state]
        if state2:
            message += " (%s%%)" % state2
        self.item[key].setText(1, message)

        if state == 4 and state2:
            self.itemProgress[key] = state2
        totalProgress = sum(self.itemProgress.values())
        self.progressBar.setValue(totalProgress)

    # ----------------------------------------- #
    def repositoryFetched(self, repoName):
        self.itemProgress[repoName] = 100
        if repositories.all()[repoName]["state"] == 2:
            self.displayState(repoName, 0)
        else:
            self.displayState(repoName, 7)
        if not repositories.fetchingInProgress():
            self.close()
