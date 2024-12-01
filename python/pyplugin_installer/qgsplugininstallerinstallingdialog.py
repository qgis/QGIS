"""
/***************************************************************************
                           qgsplugininstallerinstallingdialog.py
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
from qgis.PyQt.QtCore import QDir, QUrl, QFile, QCoreApplication
from qgis.PyQt.QtWidgets import QDialog
from qgis.PyQt.QtNetwork import QNetworkRequest, QNetworkReply

from qgis.core import (
    QgsNetworkAccessManager,
    QgsApplication,
    QgsNetworkRequestParameters,
)
from qgis.utils import HOME_PLUGIN_PATH

from .installer_data import removeDir, repositories
from .unzip import unzip

Ui_QgsPluginInstallerInstallingDialogBase, _ = uic.loadUiType(
    Path(__file__).parent / "qgsplugininstallerinstallingbase.ui"
)


class QgsPluginInstallerInstallingDialog(
    QDialog, Ui_QgsPluginInstallerInstallingDialogBase
):
    # ----------------------------------------- #

    def __init__(self, parent, plugin, stable=True):
        QDialog.__init__(self, parent)
        self.setupUi(self)
        self.plugin = plugin
        self.mResult = ""
        self.progressBar.setRange(0, 0)
        self.progressBar.setFormat("%p%")
        self.labelName.setText(plugin["name"])
        self.buttonBox.clicked.connect(self.abort)

        self.url = QUrl(
            plugin["download_url_stable"]
            if stable
            else plugin["download_url_experimental"]
        )
        self.redirectionCounter = 0

        fileName = plugin["filename"]
        tmpDir = QDir.tempPath()
        tmpPath = QDir.cleanPath(tmpDir + "/" + fileName)
        self.file = QFile(tmpPath)

        self.requestDownloading()

    def requestDownloading(self):
        self.request = QNetworkRequest(self.url)
        self.request.setAttribute(
            QNetworkRequest.Attribute(
                QgsNetworkRequestParameters.RequestAttributes.AttributeInitiatorClass
            ),
            "QgsPluginInstallerInstallingDialog",
        )
        authcfg = repositories.all()[self.plugin["zip_repository"]]["authcfg"]
        if authcfg and isinstance(authcfg, str):
            if not QgsApplication.authManager().updateNetworkRequest(
                self.request, authcfg.strip()
            ):
                self.mResult = self.tr(
                    "Update of network request with authentication "
                    "credentials FAILED for configuration '{0}'"
                ).format(authcfg)
                self.request = None

        if self.request is not None:
            self.reply = QgsNetworkAccessManager.instance().get(self.request)
            self.reply.downloadProgress.connect(self.readProgress)
            self.reply.finished.connect(self.requestFinished)

            self.stateChanged(4)

    def exec(self):
        if self.request is None:
            return QDialog.DialogCode.Rejected

        QDialog.exec(self)

    # ----------------------------------------- #
    def result(self):
        return self.mResult

    # ----------------------------------------- #
    def stateChanged(self, state):
        messages = [
            QCoreApplication.translate(
                "QgsPluginInstallerInstallingDialog", "Installing…"
            ),
            QCoreApplication.translate(
                "QgsPluginInstallerInstallingDialog", "Resolving host name…"
            ),
            QCoreApplication.translate(
                "QgsPluginInstallerInstallingDialog", "Connecting…"
            ),
            QCoreApplication.translate(
                "QgsPluginInstallerInstallingDialog", "Host connected. Sending request…"
            ),
            QCoreApplication.translate(
                "QgsPluginInstallerInstallingDialog", "Downloading data…"
            ),
            self.tr("Idle"),
            QCoreApplication.translate(
                "QgsPluginInstallerInstallingDialog", "Closing connection…"
            ),
            self.tr("Error"),
        ]
        self.labelState.setText(messages[state])

    # ----------------------------------------- #
    def readProgress(self, done, total):
        if total > 0:
            self.progressBar.setMaximum(total)
            self.progressBar.setValue(done)

    # ----------------------------------------- #
    def requestFinished(self):
        reply = self.sender()
        self.buttonBox.setEnabled(False)
        if reply.error() != QNetworkReply.NetworkError.NoError:
            self.mResult = reply.errorString()
            if reply.error() == QNetworkReply.NetworkError.OperationCanceledError:
                self.mResult += "<br/><br/>" + QCoreApplication.translate(
                    "QgsPluginInstaller",
                    "If you haven't canceled the download manually, it might be caused by a timeout. In this case consider increasing the connection timeout value in QGIS options.",
                )
            self.reject()
            reply.deleteLater()
            return
        elif reply.attribute(QNetworkRequest.Attribute.HttpStatusCodeAttribute) in (
            301,
            302,
        ):
            redirectionUrl = reply.attribute(
                QNetworkRequest.Attribute.RedirectionTargetAttribute
            )
            self.redirectionCounter += 1
            if self.redirectionCounter > 4:
                self.mResult = QCoreApplication.translate(
                    "QgsPluginInstaller", "Too many redirections"
                )
                self.reject()
                reply.deleteLater()
                return
            else:
                if redirectionUrl.isRelative():
                    redirectionUrl = reply.url().resolved(redirectionUrl)
                # Fire a new request and exit immediately in order to quietly destroy the old one
                self.url = redirectionUrl
                self.requestDownloading()
                reply.deleteLater()
                return

        self.file.open(QFile.OpenModeFlag.WriteOnly)
        self.file.write(reply.readAll())
        self.file.close()
        self.stateChanged(0)
        reply.deleteLater()
        pluginDir = HOME_PLUGIN_PATH
        tmpPath = self.file.fileName()
        # make sure that the parent directory exists
        if not QDir(pluginDir).exists():
            QDir().mkpath(pluginDir)
        # if the target directory already exists as a link, remove the link without resolving:
        QFile(pluginDir + str(QDir.separator()) + self.plugin["id"]).remove()
        try:
            unzip(
                str(tmpPath), str(pluginDir)
            )  # test extract. If fails, then exception will be raised and no removing occurs
            # removing old plugin files if exist
            removeDir(
                QDir.cleanPath(pluginDir + "/" + self.plugin["id"])
            )  # remove old plugin if exists
            unzip(str(tmpPath), str(pluginDir))  # final extract.
        except:
            self.mResult = (
                self.tr(
                    "Failed to unzip the plugin package. Probably it's broken or missing from the repository. You may also want to make sure that you have write permission to the plugin directory:"
                )
                + "\n"
                + pluginDir
            )
            self.reject()
            return
        try:
            # cleaning: removing the temporary zip file
            QFile(tmpPath).remove()
        except:
            pass
        self.close()

    # ----------------------------------------- #
    def abort(self):
        if self.reply.isRunning():
            self.reply.finished.disconnect()
            self.reply.abort()
            del self.reply
        self.mResult = self.tr("Aborted by user")
        self.reject()
