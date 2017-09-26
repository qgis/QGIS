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
from builtins import str
from builtins import range

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt import uic
from qgis.PyQt.QtCore import Qt, QEvent
from qgis.PyQt.QtWidgets import (QFileDialog,
                                 QStyle,
                                 QMessageBox,
                                 QStyledItemDelegate,
                                 QLineEdit,
                                 QWidget,
                                 QToolButton,
                                 QHBoxLayout,
                                 QComboBox,
                                 QPushButton,
                                 QApplication)
from qgis.PyQt.QtGui import (QIcon,
                             QStandardItemModel,
                             QStandardItem,
                             QCursor)

from qgis.gui import (QgsDoubleSpinBox,
                      QgsSpinBox,
                      QgsOptionsPageWidget)
from qgis.core import NULL, QgsApplication, QgsSettings
from qgis.utils import OverrideCursor

from processing.core.ProcessingConfig import (ProcessingConfig,
                                              settingsWatcher,
                                              Setting)
from processing.core.Processing import Processing
from processing.gui.DirectorySelectorDialog import DirectorySelectorDialog
from processing.gui.menus import defaultMenuEntries, menusSettingsGroup


pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'DlgConfig.ui'))


class ConfigOptionsPage(QgsOptionsPageWidget):

    def __init__(self, parent):
        super(ConfigOptionsPage, self).__init__(parent)
        self.config_widget = ConfigDialog()
        layout = QHBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setMargin(0)
        self.setLayout(layout)
        layout.addWidget(self.config_widget)
        self.setObjectName('processingOptions')

    def apply(self):
        self.config_widget.accept()

    def helpKey(self):
        return 'processing/index.html'


class ConfigDialog(BASE, WIDGET):

    def __init__(self):
        super(ConfigDialog, self).__init__(None)
        self.setupUi(self)

        self.groupIcon = QIcon()
        self.groupIcon.addPixmap(self.style().standardPixmap(
            QStyle.SP_DirClosedIcon), QIcon.Normal, QIcon.Off)
        self.groupIcon.addPixmap(self.style().standardPixmap(
            QStyle.SP_DirOpenIcon), QIcon.Normal, QIcon.On)

        if hasattr(self.searchBox, 'setPlaceholderText'):
            self.searchBox.setPlaceholderText(self.tr('Search...'))

        self.model = QStandardItemModel()
        self.tree.setModel(self.model)

        self.delegate = SettingDelegate()
        self.tree.setItemDelegateForColumn(1, self.delegate)

        self.searchBox.textChanged.connect(self.textChanged)

        self.fillTree()

        self.saveMenus = False
        self.tree.expanded.connect(self.itemExpanded)

    def textChanged(self):
        text = str(self.searchBox.text().lower())
        self._filterItem(self.model.invisibleRootItem(), text)
        if text:
            self.tree.expandAll()
        else:
            self.tree.collapseAll()

    def _filterItem(self, item, text):
        if item.hasChildren():
            show = False
            for i in range(item.rowCount()):
                child = item.child(i)
                showChild = self._filterItem(child, text)
                show = (showChild or show)
            self.tree.setRowHidden(item.row(), item.index().parent(), not show)
            return show

        elif isinstance(item, QStandardItem):
            hide = bool(text) and (text not in item.text().lower())
            self.tree.setRowHidden(item.row(), item.index().parent(), hide)
            return not hide

    def fillTree(self):
        self.fillTreeUsingProviders()

    def fillTreeUsingProviders(self):
        self.items = {}
        self.model.clear()
        self.model.setHorizontalHeaderLabels([self.tr('Setting'),
                                              self.tr('Value')])

        settings = ProcessingConfig.getSettings()

        rootItem = self.model.invisibleRootItem()

        """
        Filter 'General', 'Models' and 'Scripts' items
        """
        priorityKeys = [self.tr('General'), self.tr('Models'), self.tr('Scripts')]
        for group in priorityKeys:
            groupItem = QStandardItem(group)
            icon = ProcessingConfig.getGroupIcon(group)
            groupItem.setIcon(icon)
            groupItem.setEditable(False)
            emptyItem = QStandardItem()
            emptyItem.setEditable(False)

            rootItem.insertRow(0, [groupItem, emptyItem])
            if not group in settings:
                continue

            # add menu item only if it has any search matches
            for setting in settings[group]:
                if setting.hidden or setting.name.startswith("MENU_"):
                    continue

                labelItem = QStandardItem(setting.description)
                labelItem.setIcon(icon)
                labelItem.setEditable(False)
                self.items[setting] = SettingItem(setting)
                groupItem.insertRow(0, [labelItem, self.items[setting]])

        """
        Filter 'Providers' items
        """
        providersItem = QStandardItem(self.tr('Providers'))
        icon = QgsApplication.getThemeIcon("/processingAlgorithm.svg")
        providersItem.setIcon(icon)
        providersItem.setEditable(False)
        emptyItem = QStandardItem()
        emptyItem.setEditable(False)

        rootItem.insertRow(0, [providersItem, emptyItem])
        for group in list(settings.keys()):
            if group in priorityKeys or group == menusSettingsGroup:
                continue

            groupItem = QStandardItem(group)
            icon = ProcessingConfig.getGroupIcon(group)
            groupItem.setIcon(icon)
            groupItem.setEditable(False)

            for setting in settings[group]:
                if setting.hidden:
                    continue

                labelItem = QStandardItem(setting.description)
                labelItem.setIcon(icon)
                labelItem.setEditable(False)
                self.items[setting] = SettingItem(setting)
                groupItem.insertRow(0, [labelItem, self.items[setting]])

            emptyItem = QStandardItem()
            emptyItem.setEditable(False)
            providersItem.appendRow([groupItem, emptyItem])

        """
        Filter 'Menus' items
        """
        self.menusItem = QStandardItem(self.tr('Menus'))
        icon = QIcon(os.path.join(pluginPath, 'images', 'menu.png'))
        self.menusItem.setIcon(icon)
        self.menusItem.setEditable(False)
        emptyItem = QStandardItem()
        emptyItem.setEditable(False)

        rootItem.insertRow(0, [self.menusItem, emptyItem])

        button = QPushButton(self.tr('Reset to defaults'))
        button.clicked.connect(self.resetMenusToDefaults)
        layout = QHBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addWidget(button)
        layout.addStretch()
        widget = QWidget()
        widget.setLayout(layout)
        self.tree.setIndexWidget(emptyItem.index(), widget)

        for provider in QgsApplication.processingRegistry().providers():
            providerDescription = provider.name()
            groupItem = QStandardItem(providerDescription)
            icon = provider.icon()
            groupItem.setIcon(icon)
            groupItem.setEditable(False)

            for alg in provider.algorithms():
                algItem = QStandardItem(alg.displayName())
                algItem.setIcon(icon)
                algItem.setEditable(False)
                try:
                    settingMenu = ProcessingConfig.settings["MENU_" + alg.id()]
                    settingButton = ProcessingConfig.settings["BUTTON_" + alg.id()]
                    settingIcon = ProcessingConfig.settings["ICON_" + alg.id()]
                except:
                    continue
                self.items[settingMenu] = SettingItem(settingMenu)
                self.items[settingButton] = SettingItem(settingButton)
                self.items[settingIcon] = SettingItem(settingIcon)
                menuLabelItem = QStandardItem("Menu path")
                menuLabelItem.setEditable(False)
                buttonLabelItem = QStandardItem("Add button in toolbar")
                buttonLabelItem.setEditable(False)
                iconLabelItem = QStandardItem("Icon")
                iconLabelItem.setEditable(False)
                emptyItem = QStandardItem()
                emptyItem.setEditable(False)
                algItem.insertRow(0, [menuLabelItem, self.items[settingMenu]])
                algItem.insertRow(0, [buttonLabelItem, self.items[settingButton]])
                algItem.insertRow(0, [iconLabelItem, self.items[settingIcon]])
                groupItem.insertRow(0, [algItem, emptyItem])

            emptyItem = QStandardItem()
            emptyItem.setEditable(False)

            self.menusItem.appendRow([groupItem, emptyItem])

        self.tree.sortByColumn(0, Qt.AscendingOrder)
        self.adjustColumns()

    def resetMenusToDefaults(self):
        for provider in QgsApplication.processingRegistry().providers():
            for alg in provider.algorithms():
                d = defaultMenuEntries.get(alg.id(), "")
                setting = ProcessingConfig.settings["MENU_" + alg.id()]
                item = self.items[setting]
                item.setData(d, Qt.EditRole)
        self.saveMenus = True

    def accept(self):
        qsettings = QgsSettings()
        for setting in list(self.items.keys()):
            if setting.group != menusSettingsGroup or self.saveMenus:
                if isinstance(setting.value, bool):
                    setting.setValue(self.items[setting].checkState() == Qt.Checked)
                else:
                    try:
                        setting.setValue(str(self.items[setting].text()))
                    except ValueError as e:
                        QMessageBox.warning(self, self.tr('Wrong value'),
                                            self.tr('Wrong value for parameter "{0}":\n\n{1}').format(setting.description, str(e)))
                        return
                setting.save(qsettings)

        with OverrideCursor(Qt.WaitCursor):
            for p in QgsApplication.processingRegistry().providers():
                p.refreshAlgorithms()

        settingsWatcher.settingsChanged.emit()

    def itemExpanded(self, idx):
        if idx == self.menusItem.index():
            self.saveMenus = True
        self.adjustColumns()

    def adjustColumns(self):
        self.tree.resizeColumnToContents(0)
        self.tree.resizeColumnToContents(1)


class SettingItem(QStandardItem):

    def __init__(self, setting):
        QStandardItem.__init__(self)
        self.setting = setting
        self.setData(setting, Qt.UserRole)
        if isinstance(setting.value, bool):
            self.setCheckable(True)
            self.setEditable(False)
            if setting.value:
                self.setCheckState(Qt.Checked)
            else:
                self.setCheckState(Qt.Unchecked)
        else:
            self.setData(setting.value, Qt.EditRole)


class SettingDelegate(QStyledItemDelegate):

    def __init__(self, parent=None):
        QStyledItemDelegate.__init__(self, parent)

    def createEditor(self, parent, options, index):
        setting = index.model().data(index, Qt.UserRole)
        if setting.valuetype == Setting.FOLDER:
            return FileDirectorySelector(parent)
        elif setting.valuetype == Setting.FILE:
            return FileDirectorySelector(parent, True)
        elif setting.valuetype == Setting.SELECTION:
            combo = QComboBox(parent)
            combo.addItems(setting.options)
            return combo
        elif setting.valuetype == Setting.MULTIPLE_FOLDERS:
            return MultipleDirectorySelector(parent)
        else:
            value = self.convertValue(index.model().data(index, Qt.EditRole))
            if isinstance(value, int):
                spnBox = QgsSpinBox(parent)
                spnBox.setRange(-999999999, 999999999)
                return spnBox
            elif isinstance(value, float):
                spnBox = QgsDoubleSpinBox(parent)
                spnBox.setRange(-999999999.999999, 999999999.999999)
                spnBox.setDecimals(6)
                return spnBox
            elif isinstance(value, str):
                return QLineEdit(parent)

    def setEditorData(self, editor, index):
        value = self.convertValue(index.model().data(index, Qt.EditRole))
        setting = index.model().data(index, Qt.UserRole)
        if setting.valuetype == Setting.SELECTION:
            editor.setCurrentIndex(editor.findText(value))
        else:
            editor.setText(value)

    def setModelData(self, editor, model, index):
        value = self.convertValue(index.model().data(index, Qt.EditRole))
        setting = index.model().data(index, Qt.UserRole)
        if setting.valuetype == Setting.SELECTION:
            model.setData(index, editor.currentText(), Qt.EditRole)
        else:
            if isinstance(value, str):
                model.setData(index, editor.text(), Qt.EditRole)
            else:
                model.setData(index, editor.value(), Qt.EditRole)

    def sizeHint(self, option, index):
        return QgsSpinBox().sizeHint()

    def eventFilter(self, editor, event):
        if event.type() == QEvent.FocusOut and hasattr(editor, 'canFocusOut'):
            if not editor.canFocusOut:
                return False
        return QStyledItemDelegate.eventFilter(self, editor, event)

    def convertValue(self, value):
        if value is None or value == NULL:
            return ""
        try:
            return int(value)
        except:
            try:
                return float(value)
            except:
                return str(value)


class FileDirectorySelector(QWidget):

    def __init__(self, parent=None, selectFile=False):
        QWidget.__init__(self, parent)

        # create gui
        self.btnSelect = QToolButton()
        self.btnSelect.setText('…')
        self.lineEdit = QLineEdit()
        self.hbl = QHBoxLayout()
        self.hbl.setMargin(0)
        self.hbl.setSpacing(0)
        self.hbl.addWidget(self.lineEdit)
        self.hbl.addWidget(self.btnSelect)

        self.setLayout(self.hbl)

        self.canFocusOut = False
        self.selectFile = selectFile

        self.setFocusPolicy(Qt.StrongFocus)
        self.btnSelect.clicked.connect(self.select)

    def select(self):
        lastDir = ''
        if not self.selectFile:
            selectedPath = QFileDialog.getExistingDirectory(None,
                                                            self.tr('Select directory'), lastDir,
                                                            QFileDialog.ShowDirsOnly)
        else:
            selectedPath, selected_filter = QFileDialog.getOpenFileName(None,
                                                                        self.tr('Select file'), lastDir, self.tr('All files (*.*)')
                                                                        )

        if not selectedPath:
            return

        self.lineEdit.setText(selectedPath)
        self.canFocusOut = True

    def text(self):
        return self.lineEdit.text()

    def setText(self, value):
        self.lineEdit.setText(value)


class MultipleDirectorySelector(QWidget):

    def __init__(self, parent=None):
        QWidget.__init__(self, parent)

        # create gui
        self.btnSelect = QToolButton()
        self.btnSelect.setText('…')
        self.lineEdit = QLineEdit()
        self.hbl = QHBoxLayout()
        self.hbl.setMargin(0)
        self.hbl.setSpacing(0)
        self.hbl.addWidget(self.lineEdit)
        self.hbl.addWidget(self.btnSelect)

        self.setLayout(self.hbl)

        self.canFocusOut = False

        self.setFocusPolicy(Qt.StrongFocus)
        self.btnSelect.clicked.connect(self.select)

    def select(self):
        text = self.lineEdit.text()
        if text != '':
            items = text.split(';')

        dlg = DirectorySelectorDialog(None, items)
        if dlg.exec_():
            text = dlg.value()
            self.lineEdit.setText(text)

        self.canFocusOut = True

    def text(self):
        return self.lineEdit.text()

    def setText(self, value):
        self.lineEdit.setText(value)
