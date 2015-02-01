# -*- coding: utf-8 -*-

"""
***************************************************************************
    HelpEditionDialog.py
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
from processing.modeler.ModelerAlgorithm import ModelerAlgorithm


__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import json

from PyQt4.QtGui import QDialog, QMessageBox, QTreeWidgetItem

from processing.ui.ui_DlgHelpEdition import Ui_DlgHelpEdition
from processing.core.ProcessingLog import ProcessingLog


class HelpEditionDialog(QDialog, Ui_DlgHelpEdition):

    ALG_DESC = 'ALG_DESC'
    ALG_CREATOR = 'ALG_CREATOR'
    ALG_HELP_CREATOR = 'ALG_HELP_CREATOR'
    ALG_VERSION = 'ALG_VERSION'

    def __init__(self, alg):
        QDialog.__init__(self)
        self.setupUi(self)
        self.alg = alg
        self.descriptions = {}
        if isinstance(self.alg, ModelerAlgorithm):
            self.descriptions = self.alg.helpContent
        else:
            if self.alg.descriptionFile is not None:
                helpfile = alg.descriptionFile + '.help'
                if os.path.exists(helpfile):
                    try:
                        with open(helpfile) as f:
                            self.descriptions = json.load(f)
                    except Exception, e:
                        ProcessingLog.addToLog(ProcessingLog.LOG_WARNING,
                            self.tr('Cannot open help file: %s') % helpfile)

        self.currentName = self.ALG_DESC
        if self.ALG_DESC in self.descriptions:
            self.text.setText(self.descriptions[self.ALG_DESC])
        self.tree.itemClicked.connect(self.changeItem)

        self.fillTree()
        self.updateHtmlView()

    def reject(self):
        self.descriptions = None
        QDialog.reject(self)

    def accept(self):
        self.descriptions[self.currentName] = unicode(self.text.toPlainText())
        if isinstance(self.alg, ModelerAlgorithm):
            self.alg.helpContent = self.descriptions
        else:
            if self.alg.descriptionFile is not None:
                try:
                    with open(self.alg.descriptionFile + '.help', 'w') as f:
                        json.dump(self.descriptions, f)
                except Exception, e:
                    QMessageBox.warning(self, self.tr('Error saving help file'),
                        self.tr('Help file could not be saved.\n'
                                'Check that you have permission to modify the help\n'
                                'file. You might not have permission if you are \n'
                                'editing an example model or script, since they \n'
                                'are stored on the installation folder'))

        QDialog.accept(self)

    def getHtml(self):
        s = self.tr('<h2>Algorithm description</h2>\n')
        s += '<p>' + self.getDescription(self.ALG_DESC) + '</p>\n'
        s += self.tr('<h2>Input parameters</h2>\n')
        for param in self.alg.parameters:
            s += '<h3>' + param.description + '</h3>\n'
            s += '<p>' + self.getDescription(param.name) + '</p>\n'
        s += self.tr('<h2>Outputs</h2>\n')
        for out in self.alg.outputs:
            s += '<h3>' + out.description + '</h3>\n'
            s += '<p>' + self.getDescription(out.name) + '</p>\n'
        return s

    def fillTree(self):
        item = TreeDescriptionItem(self.tr('Algorithm description'), self.ALG_DESC)
        self.tree.addTopLevelItem(item)
        parametersItem = TreeDescriptionItem(self.tr('Input parameters'), None)
        self.tree.addTopLevelItem(parametersItem)
        for param in self.alg.parameters:
            item = TreeDescriptionItem(param.description, param.name)
            parametersItem.addChild(item)
        outputsItem = TreeDescriptionItem(self.tr('Outputs'), None)
        self.tree.addTopLevelItem(outputsItem)
        for out in self.alg.outputs:
            item = TreeDescriptionItem(out.description, out.name)
            outputsItem.addChild(item)
        item = TreeDescriptionItem(self.tr('Algorithm created by'), self.ALG_CREATOR)
        self.tree.addTopLevelItem(item)
        item = TreeDescriptionItem(self.tr('Algorithm help written by'),
                                   self.ALG_HELP_CREATOR)
        self.tree.addTopLevelItem(item)
        item = TreeDescriptionItem(self.tr('Algorithm version'),
                                   self.ALG_VERSION)
        self.tree.addTopLevelItem(item)

    def changeItem(self):
        item = self.tree.currentItem()
        if isinstance(item, TreeDescriptionItem):
            if self.currentName:
                self.descriptions[self.currentName] = unicode(self.text.toPlainText())
            name = item.name
            if name:
                self.text.setEnabled(True)
                self.updateHtmlView()
                self.currentName = name
                if name in self.descriptions:
                    self.text.setText(self.descriptions[name])
                else:
                    self.text.clear()
            else:
                self.currentName = None
                self.text.clear()
                self.text.setEnabled(False)
                self.updateHtmlView()

    def updateHtmlView(self):
        self.webView.setHtml(self.getHtml())

    def getDescription(self, name):
        if name in self.descriptions:
            return self.descriptions[name].replace('\n', '<br>')
        else:
            return ''


class TreeDescriptionItem(QTreeWidgetItem):

    def __init__(self, description, name):
        QTreeWidgetItem.__init__(self)
        self.name = name
        self.description = description
        self.setText(0, description)
