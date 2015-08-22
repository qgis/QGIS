# -*- coding: utf-8 -*-

"""
***************************************************************************
    GetScriptsAndModels.py
    ---------------------
    Date                 : June 2014
    Copyright            : (C) 2014 by Victor Olaya
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
__date__ = 'June 2014'
__copyright__ = '(C) 201, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import json
import urllib2
from urllib2 import HTTPError

from PyQt4 import uic
from PyQt4.QtCore import Qt, QCoreApplication
from PyQt4.QtGui import QIcon, QMessageBox, QCursor, QApplication, QTreeWidgetItem

from qgis.utils import iface

from processing.gui.ToolboxAction import ToolboxAction
from processing.script.ScriptUtils import ScriptUtils
from processing.algs.r.RUtils import RUtils
from processing.modeler.ModelerUtils import ModelerUtils
from processing.gui import Help2Html
from processing.gui.Help2Html import getDescription, ALG_DESC, ALG_VERSION, ALG_CREATOR

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'DlgGetScriptsAndModels.ui'))


class GetScriptsAction(ToolboxAction):

    def __init__(self):
        self.name = self.tr('Get scripts from on-line scripts collection', 'GetScriptsAction')
        self.group = self.tr('Tools', 'GetScriptsAction')

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'script.png'))

    def execute(self):
        try:
            dlg = GetScriptsAndModelsDialog(GetScriptsAndModelsDialog.SCRIPTS)
            dlg.exec_()
            if dlg.updateToolbox:
                self.toolbox.updateProvider('script')
        except HTTPError:
            QMessageBox.critical(iface.mainWindow(),
                                 self.tr('Connection problem', 'GetScriptsAction'),
                                 self.tr('Could not connect to scripts/models repository', 'GetScriptsAction'))


class GetRScriptsAction(ToolboxAction):

    def __init__(self):
        self.name = self.tr('Get R scripts from on-line scripts collection', 'GetRScriptsAction')
        self.group = self.tr('Tools', 'GetRScriptsAction')

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'r.png'))

    def execute(self):
        try:
            dlg = GetScriptsAndModelsDialog(GetScriptsAndModelsDialog.RSCRIPTS)
            dlg.exec_()
            if dlg.updateToolbox:
                self.toolbox.updateProvider('r')
        except HTTPError:
            QMessageBox.critical(iface.mainWindow(),
                                 self.tr('Connection problem', 'GetRScriptsAction'),
                                 self.tr('Could not connect to scripts/models repository', 'GetRScriptsAction'))


class GetModelsAction(ToolboxAction):

    def __init__(self):
        self.name = self.tr('Get models from on-line scripts collection', 'GetModelsAction')
        self.group = self.tr('Tools', 'GetModelsAction')

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'model.png'))

    def execute(self):
        try:
            dlg = GetScriptsAndModelsDialog(GetScriptsAndModelsDialog.MODELS)
            dlg.exec_()
            if dlg.updateToolbox:
                self.toolbox.updateProvider('model')
        except (HTTPError, URLError):
            QMessageBox.critical(iface.mainWindow(),
                                 self.tr('Connection problem', 'GetModelsAction'),
                                 self.tr('Could not connect to scripts/models repository', 'GetModelsAction'))


def readUrl(url):
    try:
        QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
        return urllib2.urlopen(url).read()
    finally:
        QApplication.restoreOverrideCursor()


class GetScriptsAndModelsDialog(BASE, WIDGET):

    HELP_TEXT = QCoreApplication.translate('GetScriptsAndModelsDialog',
                                           '<h3> Processing resources manager </h3>'
                                           '<p>Check/uncheck algorithms in the tree to select the ones that you '
                                           'want to install or remove</p>'
                                           '<p>Algorithms are divided in 3 groups:</p>'
                                           '<ul><li><b>Installed:</b> Algorithms already in your system, with '
                                           'the latest version available</li>'
                                           '<li><b>Updatable:</b> Algorithms already in your system, but with '
                                           'a newer version available in the server</li>'
                                           '<li><b>Not installed:</b> Algorithms not installed in your '
                                           'system</li></ul>')
    MODELS = 0
    SCRIPTS = 1
    RSCRIPTS = 2

    def __init__(self, resourceType):
        super(GetScriptsAndModelsDialog, self).__init__(iface.mainWindow())
        self.setupUi(self)

        self.resourceType = resourceType
        if self.resourceType == self.MODELS:
            self.folder = ModelerUtils.modelsFolder()
            self.urlBase = 'https://raw.githubusercontent.com/qgis/QGIS-Processing/master/models/'
            self.icon = QIcon(os.path.join(pluginPath, 'images', 'model.png'))
        elif self.resourceType == self.SCRIPTS:
            self.folder = ScriptUtils.scriptsFolder()
            self.urlBase = 'https://raw.githubusercontent.com/qgis/QGIS-Processing/master/scripts/'
            self.icon = QIcon(os.path.join(pluginPath, 'images', 'script.png'))
        else:
            self.folder = RUtils.RScriptsFolder()
            self.urlBase = 'https://raw.githubusercontent.com/qgis/QGIS-Processing/master/rscripts/'
            self.icon = QIcon(os.path.join(pluginPath, 'images', 'r.png'))

        self.lastSelectedItem = None
        self.updateToolbox = False
        self.populateTree()
        self.buttonBox.accepted.connect(self.okPressed)
        self.buttonBox.rejected.connect(self.cancelPressed)
        self.tree.currentItemChanged .connect(self.currentItemChanged)

    def populateTree(self):
        self.uptodateItem = QTreeWidgetItem()
        self.uptodateItem.setText(0, self.tr('Installed'))
        self.toupdateItem = QTreeWidgetItem()
        self.toupdateItem.setText(0, self.tr('Updatable'))
        self.notinstalledItem = QTreeWidgetItem()
        self.notinstalledItem.setText(0, self.tr('Not installed'))
        self.toupdateItem.setIcon(0, self.icon)
        self.uptodateItem.setIcon(0, self.icon)
        self.notinstalledItem.setIcon(0, self.icon)
        resources = readUrl(self.urlBase + 'list.txt').splitlines()
        resources = [r.split(',') for r in resources]
        self.resources = {f: (v, n) for f, v, n in resources}
        for filename, version, name in resources:
            treeBranch = self.getTreeBranchForState(filename, float(version))
            item = TreeItem(filename, name, self.icon)
            treeBranch.addChild(item)
            if treeBranch != self.notinstalledItem:
                item.setCheckState(0, Qt.Checked)

        self.tree.addTopLevelItem(self.toupdateItem)
        self.tree.addTopLevelItem(self.notinstalledItem)
        self.tree.addTopLevelItem(self.uptodateItem)

        self.webView.setHtml(self.HELP_TEXT)

    def currentItemChanged(self, item, prev):
        if isinstance(item, TreeItem):
            try:
                url = self.urlBase + item.filename.replace(' ', '%20') + '.help'
                helpContent = readUrl(url)
                descriptions = json.loads(helpContent)
                html = '<h2>%s</h2>' % item.name
                html += self.tr('<p><b>Description:</b> %s</p>') % getDescription(ALG_DESC, descriptions)
                html += self.tr('<p><b>Created by:</b> %s') % getDescription(ALG_CREATOR, descriptions)
                html += self.tr('<p><b>Version:</b> %s') % getDescription(ALG_VERSION, descriptions)
            except HTTPError as e:
                html = self.tr('<h2>No detailed description available for this script</h2>')
            self.webView.setHtml(html)
        else:
            self.webView.setHtml(self.HELP_TEXT)

    def getTreeBranchForState(self, filename, version):
        if not os.path.exists(os.path.join(self.folder, filename)):
            return self.notinstalledItem
        else:
            helpFile = os.path.join(self.folder, filename + '.help')
            try:
                with open(helpFile) as f:
                    helpContent = json.load(f)
                    currentVersion = float(helpContent[Help2Html.ALG_VERSION])
            except Exception:
                currentVersion = 0
            if version > currentVersion:
                return self.toupdateItem
            else:
                return self.uptodateItem

    def cancelPressed(self):
        self.close()

    def okPressed(self):
        toDownload = []
        for i in xrange(self.toupdateItem.childCount()):
            item = self.toupdateItem.child(i)
            if item.checkState(0) == Qt.Checked:
                toDownload.append(item.filename)
        for i in xrange(self.notinstalledItem.childCount()):
            item = self.notinstalledItem.child(i)
            if item.checkState(0) == Qt.Checked:
                toDownload.append(item.filename)

        if toDownload:
            self.progressBar.setMaximum(len(toDownload))
            for i, filename in enumerate(toDownload):
                QCoreApplication.processEvents()
                url = self.urlBase + filename.replace(' ', '%20')
                try:
                    code = readUrl(url)
                    path = os.path.join(self.folder, filename)
                    with open(path, 'w') as f:
                        f.write(code)
                except HTTPError:
                    QMessageBox.critical(iface.mainWindow(),
                                         self.tr('Connection problem'),
                                         self.tr('Could not download file: %s') % filename)
                    return
                url += '.help'
                try:
                    html = readUrl(url)
                except HTTPError:
                    html = '{"ALG_VERSION" : %s}' % self.resources[filename][0]

                path = os.path.join(self.folder, filename + '.help')
                with open(path, 'w') as f:
                    f.write(html)
                self.progressBar.setValue(i + 1)

        toDelete = []
        for i in xrange(self.uptodateItem.childCount()):
            item = self.uptodateItem.child(i)
            if item.checkState(0) == Qt.Unchecked:
                toDelete.append(item.filename)
        for filename in toDelete:
            path = os.path.join(self.folder, filename)
            os.remove(path)

        self.updateToolbox = len(toDownload) + len(toDelete) > 0
        self.close()


class TreeItem(QTreeWidgetItem):

    def __init__(self, filename, name, icon):
        QTreeWidgetItem.__init__(self)
        self.name = name
        self.filename = filename
        self.setText(0, name)
        self.setIcon(0, icon)
        self.setCheckState(0, Qt.Unchecked)
