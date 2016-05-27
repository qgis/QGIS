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
__copyright__ = '(C) 2014, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import json
from functools import partial

from qgis.PyQt import uic
from qgis.PyQt.QtCore import Qt, QCoreApplication, QUrl
from qgis.PyQt.QtGui import QIcon, QCursor
from qgis.PyQt.QtWidgets import QApplication, QTreeWidgetItem, QPushButton
from qgis.PyQt.QtNetwork import QNetworkReply, QNetworkRequest

from qgis.utils import iface, show_message_log
from qgis.core import QgsNetworkAccessManager, QgsMessageLog
from qgis.gui import QgsMessageBar

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
        self.name, self.i18n_name = self.trAction('Get scripts from on-line scripts collection')
        self.group, self.i18n_group = self.trAction('Tools')

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'script.png'))

    def execute(self):
        dlg = GetScriptsAndModelsDialog(GetScriptsAndModelsDialog.SCRIPTS)
        dlg.exec_()
        if dlg.updateProvider:
            algList.reloadProvider('script')


class GetRScriptsAction(ToolboxAction):

    def __init__(self):
        self.name, self.i18n_name = self.trAction('Get R scripts from on-line scripts collection')
        self.group, self.i18n_group = self.trAction('Tools')

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'r.svg'))

    def execute(self):
        dlg = GetScriptsAndModelsDialog(GetScriptsAndModelsDialog.RSCRIPTS)
        dlg.exec_()
        if dlg.updateProvider:
            self.toolbox.updateProvider('r')


class GetModelsAction(ToolboxAction):

    def __init__(self):
        self.name, self.i18n_name = self.trAction('Get models from on-line scripts collection')
        self.group, self.i18n_group = self.trAction('Tools')

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'model.png'))

    def execute(self):
        dlg = GetScriptsAndModelsDialog(GetScriptsAndModelsDialog.MODELS)
        dlg.exec_()
        if dlg.updateProvider:
            algList.reloadProvider('model')


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

    tr_disambiguation = {0: 'GetModelsAction',
                         1: 'GetScriptsAction',
                         2: 'GetRScriptsAction'}

    def __init__(self, resourceType):
        super(GetScriptsAndModelsDialog, self).__init__(iface.mainWindow())
        self.setupUi(self)
        self.manager = QgsNetworkAccessManager.instance()

        self.resourceType = resourceType
        if self.resourceType == self.MODELS:
            self.folder = ModelerUtils.defaultModelsFolder()
            self.urlBase = 'https://raw.githubusercontent.com/qgis/QGIS-Processing/master/models/'
            self.icon = QIcon(os.path.join(pluginPath, 'images', 'model.png'))
        elif self.resourceType == self.SCRIPTS:
            self.folder = ScriptUtils.defaultScriptsFolder()
            self.urlBase = 'https://raw.githubusercontent.com/qgis/QGIS-Processing/master/scripts/'
            self.icon = QIcon(os.path.join(pluginPath, 'images', 'script.png'))
        else:
            self.folder = RUtils.defaultRScriptsFolder()
            self.urlBase = 'https://raw.githubusercontent.com/qgis/QGIS-Processing/master/rscripts/'
            self.icon = QIcon(os.path.join(pluginPath, 'images', 'r.svg'))

        self.lastSelectedItem = None
        self.updateProvider = False
        self.populateTree()
        self.buttonBox.accepted.connect(self.okPressed)
        self.buttonBox.rejected.connect(self.cancelPressed)
        self.tree.currentItemChanged.connect(self.currentItemChanged)

    def popupError(self, error=None, url=None):
        """Popups an Error message bar for network errors."""
        disambiguation = self.tr_disambiguation[self.resourceType]
        widget = iface.messageBar().createMessage(self.tr('Connection problem', disambiguation),
                                                  self.tr('Could not connect to scripts/models repository', disambiguation))
        if error and url:
            QgsMessageLog.logMessage(self.tr(u"Network error code: {} on URL: {}").format(error, url), self.tr(u"Processing"), QgsMessageLog.CRITICAL)
            button = QPushButton(QCoreApplication.translate("Python", "View message log"), pressed=show_message_log)
            widget.layout().addWidget(button)

        iface.messageBar().pushWidget(widget, level=QgsMessageBar.CRITICAL, duration=5)

    def grabHTTP(self, url, loadFunction, arguments=None):
        """Grab distant content via QGIS internal classes and QtNetwork."""
        QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
        request = QUrl(url)
        reply = self.manager.get(QNetworkRequest(request))
        if arguments:
            reply.finished.connect(partial(loadFunction, reply, arguments))
        else:
            reply.finished.connect(partial(loadFunction, reply))

        while not reply.isFinished():
            QCoreApplication.processEvents()

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
        self.grabHTTP(self.urlBase + 'list.txt', self.treeLoaded)

    def treeLoaded(self, reply):
        """
        update the tree of scripts/models whenever
        HTTP request is finished
        """
        QApplication.restoreOverrideCursor()
        if reply.error() != QNetworkReply.NoError:
            self.popupError(reply.error(), reply.request().url().toString())
        else:
            resources = unicode(reply.readAll()).splitlines()
            resources = [r.split(',') for r in resources]
            self.resources = {f: (v, n) for f, v, n in resources}
            for filename, version, name in sorted(resources, key=lambda kv: kv[2].lower()):
                treeBranch = self.getTreeBranchForState(filename, float(version))
                item = TreeItem(filename, name, self.icon)
                treeBranch.addChild(item)
                if treeBranch != self.notinstalledItem:
                    item.setCheckState(0, Qt.Checked)

        reply.deleteLater()
        self.tree.addTopLevelItem(self.toupdateItem)
        self.tree.addTopLevelItem(self.notinstalledItem)
        self.tree.addTopLevelItem(self.uptodateItem)

        self.txtHelp.setHtml(self.HELP_TEXT)

    def setHelp(self, reply, item):
        """Change the HTML content"""
        QApplication.restoreOverrideCursor()
        if reply.error() != QNetworkReply.NoError:
            html = self.tr('<h2>No detailed description available for this script</h2>')
        else:
            content = unicode(reply.readAll())
            descriptions = json.loads(content)
            html = '<h2>%s</h2>' % item.name
            html += self.tr('<p><b>Description:</b> %s</p>') % getDescription(ALG_DESC, descriptions)
            html += self.tr('<p><b>Created by:</b> %s') % getDescription(ALG_CREATOR, descriptions)
            html += self.tr('<p><b>Version:</b> %s') % getDescription(ALG_VERSION, descriptions)
        reply.deleteLater()
        self.txtHelp.setHtml(html)

    def currentItemChanged(self, item, prev):
        if isinstance(item, TreeItem):
            url = self.urlBase + item.filename.replace(' ', '%20') + '.help'
            self.grabHTTP(url, self.setHelp, item)
        else:
            self.txtHelp.setHtml(self.HELP_TEXT)

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
        super(GetScriptsAndModelsDialog, self).reject()

    def storeFile(self, reply, filename):
        """store a script/model that has been downloaded"""
        QApplication.restoreOverrideCursor()
        if reply.error() != QNetworkReply.NoError:
            if os.path.splitext(filename)[1].lower() == '.help':
                content = '{"ALG_VERSION" : %s}' % self.resources[filename[:-5]][0]
            else:
                self.popupError(reply.error(), reply.request().url().toString())
                content = None
        else:
            content = reply.readAll()

        reply.deleteLater()
        if content:
            path = os.path.join(self.folder, filename)
            with open(path, 'w') as f:
                f.write(content)

        self.progressBar.setValue(self.progressBar.value() + 1)

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
            self.progressBar.setMaximum(len(toDownload) * 2)
            for i, filename in enumerate(toDownload):
                QCoreApplication.processEvents()
                url = self.urlBase + filename.replace(' ', '%20')
                self.grabHTTP(url, self.storeFile, filename)

                url += '.help'
                self.grabHTTP(url, self.storeFile, filename + '.help')

        toDelete = []
        for i in xrange(self.uptodateItem.childCount()):
            item = self.uptodateItem.child(i)
            if item.checkState(0) == Qt.Unchecked:
                toDelete.append(item.filename)

        # Remove py and help files if they exist
        for filename in toDelete:
            for pathname in (filename, filename + u".help"):
                path = os.path.join(self.folder, pathname)
                if os.path.exists(path):
                    os.remove(path)

        self.updateProvider = len(toDownload) + len(toDelete) > 0
        super(GetScriptsAndModelsDialog, self).accept()


class TreeItem(QTreeWidgetItem):

    def __init__(self, filename, name, icon):
        QTreeWidgetItem.__init__(self)
        self.name = name
        self.filename = filename
        self.setText(0, name)
        self.setIcon(0, icon)
        self.setCheckState(0, Qt.Unchecked)
