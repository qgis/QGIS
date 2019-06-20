# -*- coding: utf-8 -*-

"""
***************************************************************************
    ResultsDock.py
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
import time
import warnings

from qgis.PyQt import uic
from qgis.PyQt.QtCore import (QUrl,
                              QFileInfo,
                              QDir)
from qgis.gui import QgsDockWidget
from qgis.PyQt.QtGui import QDesktopServices
from qgis.PyQt.QtWidgets import QTreeWidgetItem

from processing.core.ProcessingResults import resultsList

pluginPath = os.path.split(os.path.dirname(__file__))[0]

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(
        os.path.join(pluginPath, 'ui', 'resultsdockbase.ui'))


class ResultsDock(QgsDockWidget, WIDGET):

    def __init__(self):
        super(ResultsDock, self).__init__(None)
        self.setupUi(self)

        resultsList.resultAdded.connect(self.addResult)

        self.treeResults.currentItemChanged.connect(self.updateDescription)
        self.treeResults.itemDoubleClicked.connect(self.openResult)

        self.txtDescription.setOpenLinks(False)
        self.txtDescription.anchorClicked.connect(self.openLink)

        self.fillTree()

    def addResult(self):
        self.fillTree()

        # Automatically open the panel for users to see output
        self.setUserVisible(True)
        self.treeResults.setCurrentItem(self.treeResults.topLevelItem(0))

    def fillTree(self):
        self.treeResults.blockSignals(True)
        self.treeResults.clear()
        elements = resultsList.getResults()
        for element in elements:
            item = TreeResultItem(element)
            self.treeResults.insertTopLevelItem(0, item)
        self.treeResults.blockSignals(False)

    def updateDescription(self, current, previous):
        if isinstance(current, TreeResultItem):
            html = '<b>Algorithm</b>: {}<br><b>File path</b>: <a href="{}">{}</a>'.format(current.algorithm, QUrl.fromLocalFile(current.filename).toString(), QDir.toNativeSeparators(current.filename))
            self.txtDescription.setHtml(html)

    def openLink(self, url):
        QDesktopServices.openUrl(url)

    def openResult(self, item, column):
        QDesktopServices.openUrl(QUrl.fromLocalFile(item.filename))


class TreeResultItem(QTreeWidgetItem):

    def __init__(self, result):
        QTreeWidgetItem.__init__(self)
        self.setIcon(0, result.icon)
        self.setText(0, '{0} [{1}]'.format(result.name, time.strftime('%I:%M:%S%p', result.timestamp)))
        self.algorithm = result.name
        self.filename = result.filename
