# -*- coding: utf-8 -*-

"""
***************************************************************************
    ResultsDialog.py
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

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from processing.core.ProcessingResults import ProcessingResults

from processing.ui.ui_DlgResults import Ui_DlgResults

class ResultsDialog(QDialog, Ui_DlgResults):
    def __init__(self):
        QDialog.__init__(self)
        self.setupUi(self)

        self.keyIcon = QIcon()
        self.keyIcon.addPixmap(self.style().standardPixmap(QStyle.SP_FileIcon))

        self.tree.itemClicked.connect(self.changeResult)

        self.fillTree()

        if self.lastUrl:
            self.webView.load(self.lastUrl)

    def fillTree(self):
        elements = ProcessingResults.getResults()
        if len(elements) == 0:
            self.lastUrl = None
            return
        for element in elements:
            item = TreeResultItem(element)
            item.setIcon(0, self.keyIcon)
            self.tree.addTopLevelItem(item)
        self.lastUrl = QUrl(elements[-1].filename)

    def changeResult(self):
        item = self.tree.currentItem()
        if isinstance(item, TreeResultItem):
            url = QUrl(item.filename)
            self.webView.load(url)

class TreeResultItem(QTreeWidgetItem):
    def __init__(self, result):
        QTreeWidgetItem.__init__(self)
        self.filename = result.filename
        self.setText(0, result.name)
