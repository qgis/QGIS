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

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from sextante.core.SextanteConfig import SextanteConfig

from sextante.ui.ui_DlgConfig import Ui_DlgConfig

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

    def fillTree(self):
        self.items = {}
        self.tree.clear()
        text = str(self.searchBox.text())
        settings = SextanteConfig.getSettings()
        for group in settings.keys():
            groupItem = QTreeWidgetItem()
            groupItem.setText(0,group)
            icon = SextanteConfig.getGroupIcon(group)
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

        self.tree.sortItems(0, Qt.AscendingOrder)
        self.tree.resizeColumnToContents(0)
        self.tree.resizeColumnToContents(1)

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
            SextanteConfig.addSetting(setting)
        SextanteConfig.saveSettings()
        self.toolbox.updateTree()

        QDialog.accept(self)

class TreeSettingItem(QTreeWidgetItem):

    def __init__(self, setting, icon):
        QTreeWidgetItem.__init__(self)
        self.setting = setting
        self.setText(0, setting.description)
        self.setFlags(self.flags() | Qt.ItemIsEditable)
        if isinstance(setting.value,bool):
            if setting.value:
                self.setCheckState(1, Qt.Checked)
            else:
                self.setCheckState(1, Qt.Unchecked)
        else:
            self.setText(1, unicode(setting.value))
        self.setIcon(0, icon)
