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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

import os
import json
import warnings

from qgis.PyQt import uic
from qgis.PyQt.QtWidgets import QDialog, QTreeWidgetItem

from qgis.core import (Qgis,
                       QgsMessageLog,
                       QgsProcessingUtils,
                       QgsProcessingParameterDefinition,
                       QgsProcessingModelAlgorithm)

pluginPath = os.path.split(os.path.dirname(__file__))[0]

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(
        os.path.join(pluginPath, 'ui', 'DlgHelpEdition.ui'))


class HelpEditionDialog(BASE, WIDGET):
    ALG_DESC = 'ALG_DESC'
    ALG_CREATOR = 'ALG_CREATOR'
    ALG_HELP_CREATOR = 'ALG_HELP_CREATOR'
    ALG_VERSION = 'ALG_VERSION'
    SHORT_DESCRIPTION = 'SHORT_DESCRIPTION'
    HELP_URL = 'HELP_URL'

    def __init__(self, alg):
        super(HelpEditionDialog, self).__init__(None)
        self.setupUi(self)

        self.alg = alg
        self.descriptions = {}
        if isinstance(self.alg, QgsProcessingModelAlgorithm):
            self.descriptions = self.alg.helpContent()
        else:
            if self.alg.descriptionFile is not None:
                helpfile = alg.descriptionFile + '.help'
                if os.path.exists(helpfile):
                    try:
                        with open(helpfile) as f:
                            self.descriptions = json.load(f)
                    except Exception:
                        QgsMessageLog.logMessage(self.tr('Cannot open help file: {0}').format(helpfile), self.tr('Processing'), Qgis.Warning)

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
        self.descriptions[self.currentName] = str(self.text.toPlainText())
        QDialog.accept(self)

    def getHtml(self):
        s = '<p>' + self.getDescription(self.ALG_DESC) + '</p>\n'
        inputs = ""
        for param in self.alg.parameterDefinitions():
            if param.flags() & QgsProcessingParameterDefinition.FlagHidden or param.isDestination():
                continue

            if self.getDescription(param.name()):
                inputs += '<h3>' + param.description() + '</h3>\n'
                inputs += '<p>' + self.getDescription(param.name()) + '</p>\n'
        if inputs:
            s += '<h2>' + self.tr('Input parameters') + '</h2>\n' + inputs
        outputs = ""
        for out in self.alg.outputDefinitions():
            if self.getDescription(param.name()):
                outputs += '<h3>' + out.description() + '</h3>\n'
                outputs += '<p>' + self.getDescription(out.name()) + '</p>\n'
        if outputs:
            s += '<h2>' + self.tr('Outputs') + '</h2>\n' + outputs
        s += '<br>'
        if self.getDescription(self.ALG_CREATOR):
            s += '<p align=\"right\">' + self.tr('Algorithm author:') + ' ' + self.getDescription(self.ALG_CREATOR) + '</p>'
        if self.getDescription(self.ALG_HELP_CREATOR):
            s += '<p align=\"right\">' + self.tr('Help author:') + ' ' + self.getDescription(self.ALG_HELP_CREATOR) + '</p>'
        if self.getDescription(self.ALG_VERSION):
            s += '<p align=\"right\">' + self.tr('Algorithm version:') + ' ' + self.getDescription(self.ALG_VERSION) + '</p>'
        return s

    def fillTree(self):
        item = TreeDescriptionItem(self.tr('Algorithm description'), self.ALG_DESC)
        self.tree.addTopLevelItem(item)
        item = TreeDescriptionItem(self.tr('Short description'), self.SHORT_DESCRIPTION)
        self.tree.addTopLevelItem(item)
        parametersItem = TreeDescriptionItem(self.tr('Input parameters'), None)
        self.tree.addTopLevelItem(parametersItem)
        for param in self.alg.parameterDefinitions():
            if param.flags() & QgsProcessingParameterDefinition.FlagHidden or param.isDestination():
                continue

            item = TreeDescriptionItem(param.description(), param.name())
            parametersItem.addChild(item)
        outputsItem = TreeDescriptionItem(self.tr('Outputs'), None)
        self.tree.addTopLevelItem(outputsItem)
        for out in self.alg.outputDefinitions():
            item = TreeDescriptionItem(out.description(), out.name())
            outputsItem.addChild(item)
        item = TreeDescriptionItem(self.tr('Algorithm author'), self.ALG_CREATOR)
        self.tree.addTopLevelItem(item)
        item = TreeDescriptionItem(self.tr('Help author'), self.ALG_HELP_CREATOR)
        self.tree.addTopLevelItem(item)
        item = TreeDescriptionItem(self.tr('Algorithm version'), self.ALG_VERSION)
        self.tree.addTopLevelItem(item)
        item = TreeDescriptionItem(self.tr('Documentation help URL (for help button)'), self.HELP_URL)
        self.tree.addTopLevelItem(item)

    def changeItem(self):
        item = self.tree.currentItem()
        if isinstance(item, TreeDescriptionItem):
            if self.currentName:
                self.descriptions[self.currentName] = str(self.text.toPlainText())
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
        self.txtPreview.setHtml(self.getHtml())

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
