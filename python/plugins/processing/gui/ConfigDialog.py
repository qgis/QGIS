# -*- coding: utf-8 -*-

"""
***************************************************************************
    ConfigDialog.py
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
from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from processing.core.ProcessingConfig import ProcessingConfig
from processing.ui.ui_DlgConfig import Ui_DlgConfig

class ConfigDialog(QDialog, Ui_DlgConfig):
    def __init__(self, toolbox):
        QDialog.__init__(self)
        self.setupUi(self)
        self.toolbox = toolbox
        self.groupIcon = QIcon()
        self.groupIcon.addPixmap(self.style().standardPixmap(QStyle.SP_DirClosedIcon),
                                 QIcon.Normal, QIcon.Off)
        self.groupIcon.addPixmap(self.style().standardPixmap(QStyle.SP_DirOpenIcon),
                                 QIcon.Normal, QIcon.On)

        if hasattr(self.searchBox, 'setPlaceholderText'):
            self.searchBox.setPlaceholderText(self.tr("Search..."))

        self.searchBox.textChanged.connect(self.fillTree)
        self.fillTree()
        self.tree.itemClicked.connect(self.edit)
        self.tree.itemDoubleClicked.connect(self.edit)

    def edit(self, item, column):
        if column > 0:
            self.tree.editItem(item, column)

    def fillTree(self):
        self.items = {}
        self.tree.clear()
        text = unicode(self.searchBox.text())
        settings = ProcessingConfig.getSettings()
        priorityKeys = ['General', "Models", "Scripts"]
        for group in priorityKeys:
            groupItem = QTreeWidgetItem()
            groupItem.setText(0,group)
            icon = ProcessingConfig.getGroupIcon(group)
            groupItem.setIcon(0, icon)
            for setting in settings[group]:
                if setting.hidden:
                    continue
                if text =="" or text.lower() in setting.description.lower():
                    settingItem = TreeSettingItem(setting, icon)
                    self.items[setting]=settingItem
                    groupItem.addChild(settingItem)
            self.tree.addTopLevelItem(groupItem)
            if text != "":
                groupItem.setExpanded(True)

        providersItem = QTreeWidgetItem()
        providersItem.setText(0, "Providers")
        icon = QtGui.QIcon(os.path.dirname(__file__) + "/../images/alg.png")
        providersItem.setIcon(0, icon)
        for group in settings.keys():
            if group in priorityKeys:
                continue
            groupItem = QTreeWidgetItem()
            groupItem.setText(0,group)
            icon = ProcessingConfig.getGroupIcon(group)
            groupItem.setIcon(0, icon)
            for setting in settings[group]:
                if setting.hidden:
                    continue
                if text =="" or text.lower() in setting.description.lower():
                    settingItem = TreeSettingItem(setting, icon)
                    self.items[setting]=settingItem
                    groupItem.addChild(settingItem)
            if text != "":
                groupItem.setExpanded(True)
            providersItem.addChild(groupItem)
        self.tree.addTopLevelItem(providersItem)

        self.tree.sortItems(0, Qt.AscendingOrder)
        self.tree.setColumnWidth(0, 400)

    def accept(self):
        for setting in self.items.keys():
            if isinstance(setting.value,bool):
                setting.value = (self.items[setting].checkState(1) == Qt.Checked)
            elif isinstance(setting.value, (float,int, long)):
                value = str(self.items[setting].text(1))
                try:
                    value = float(value)
                    setting.value = value
                except ValueError:
                    QMessageBox.critical(self,
                                         self.tr("Wrong value"),
                                         self.tr("Wrong parameter value:\n%1").arg(value)
                                        )
                    return
            else:
                setting.value = str(self.items[setting].text(1))
            ProcessingConfig.addSetting(setting)
        ProcessingConfig.saveSettings()
        self.toolbox.updateTree()

        QDialog.accept(self)

class TreeSettingItem(QTreeWidgetItem):

    def __init__(self, setting, icon):
        QTreeWidgetItem.__init__(self)
        self.setting = setting
        self.setText(0, setting.description)
        if isinstance(setting.value,bool):
            if setting.value:
                self.setCheckState(1, Qt.Checked)
            else:
                self.setCheckState(1, Qt.Unchecked)
        else:
            self.setFlags(self.flags() | Qt.ItemIsEditable)
            self.setText(1, unicode(setting.value))
        self.setIcon(0, icon)
