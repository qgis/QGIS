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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import codecs

from qgis.PyQt import uic
from qgis.PyQt.QtCore import QUrl
from qgis.PyQt.QtGui import QDesktopServices
from qgis.PyQt.QtWidgets import QTreeWidgetItem

from processing.core.ProcessingResults import resultsList

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'resultsdockbase.ui'))


class ResultsDock(BASE, WIDGET):

    def __init__(self):
        super(ResultsDock, self).__init__(None)
        self.setupUi(self)

        self.treeResults.currentItemChanged.connect(self.updateDescription)
        self.treeResults.itemDoubleClicked.connect(self.openResult)

        self.fillTree()

    def fillTree(self):
        self.treeResults.blockSignals(True)
        self.treeResults.clear()
        elements = resultsList.getResults()
        for element in elements:
            item = TreeResultItem(element)
            self.treeResults.addTopLevelItem(item)
        self.treeResults.blockSignals(False)

    def updateDescription(self, current, previous):
        if isinstance(current, TreeResultItem):
            html = '<b>Algorithm</b>: {}<br><b>File path</b>: {}'.format(current.text(0), current.filename)
            self.txtDescription.setHtml(html)

    def openResult(self, item, column):
        QDesktopServices.openUrl(QUrl.fromLocalFile(item.filename))


class TreeResultItem(QTreeWidgetItem):

    def __init__(self, result):
        QTreeWidgetItem.__init__(self)
        self.setIcon(0, result.icon)
        self.setText(0, result.name)
        self.filename = result.filename
