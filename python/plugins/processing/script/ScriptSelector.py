# -*- coding: utf-8 -*-

"""
***************************************************************************
    ScriptSelector.py
    ---------------------
    Date                 : May 2016
    Copyright            : (C) 2016 by Victor Olaya
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
__date__ = 'May 2016'
__copyright__ = '(C) 2016, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive
from PyQt4 import QtGui, uic, QtCore
import os
from collections import defaultdict
from processing.core.alglist import algList

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'DlgConfig.ui'))


class ScriptSelector(BASE, WIDGET):

    def __init__(self):
        QtGui.QDialog.__init__(self)
        self.setupUi(self)

        self.scripts = None

        allScripts = defaultdict(list)
        alglist = algList.getProviderFromName("script").algs
        for script in alglist:
            allScripts[script.group].append(script)

        for group, groupScripts in allScripts.iteritems():
            groupItem = QtGui.QTreeWidgetItem()
            groupItem.setText(0, group)
            groupItem.setFlags(groupItem.flags() | QtCore.Qt.ItemIsTristate)
            for script in groupScripts:
                scriptItem = QtGui.QTreeWidgetItem()
                scriptItem.setFlags(scriptItem.flags() | QtCore.Qt.ItemIsUserCheckable)
                scriptItem.setCheckState(0, QtCore.Qt.Checked)
                scriptItem.script = script
                scriptItem.setText(0, script.name)
                groupItem.addChild(scriptItem)
            self.scriptsTree.addTopLevelItem(groupItem)

        self.scriptsTree.expandAll()

        self.selectAllLabel.linkActivated.connect(lambda: self.checkScripts(True))
        self.unselectAllLabel.linkActivated.connect(lambda: self.checkScripts(False))

        self.folderButton.clicked.connect(self.selectFolder)

        self.buttonBox.accepted.connect(self.okPressed)
        self.buttonBox.rejected.connect(self.cancelPressed)

    def selectFolder(self):
        folder = QtGui.QFileDialog.getExistingDirectory(self, 'Select folder')
        if folder:
            self.folderBox.setText(folder)

    def checkScripts(self, b):
        state = QtCore.Qt.Checked if b else QtCore.Qt.Unchecked
        for i in xrange(self.scriptsTree.topLevelItemCount()):
            item = self.scriptsTree.topLevelItem(i)
            for j in xrange(item.childCount()):
                child = item.child(j)
                child.setCheckState(0, state)

    def cancelPressed(self):
        self.close()

    def _getValue(self, textBox):
        textBox.setStyleSheet("QLineEdit{background: white}")
        value = textBox.text()
        if value:
            return value
        textBox.setStyleSheet("QLineEdit{background: yellow}")
        raise Exception("wrong parameter value")

    def okPressed(self):
        self.scripts = []
        for i in xrange(self.scriptsTree.topLevelItemCount()):
            groupItem = self.scriptsTree.topLevelItem(i)
            for j in xrange(groupItem.childCount()):
                scriptItem = groupItem.child(j)
                if scriptItem.checkState(0) == QtCore.Qt.Checked:
                    self.scripts.append(scriptItem.script)
        self.folder = self._getValue(self.folderBox)
        try:
            self.name = self._getValue(self.nameBox)
            self.description = self._getValue(self.descriptionBox)
            self.author = self._getValue(self.authorBox)
            self.email = self._getValue(self.emailBox)
        except:
            return
        self.close()
