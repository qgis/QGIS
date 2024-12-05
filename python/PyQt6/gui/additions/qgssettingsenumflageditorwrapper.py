"""
***************************************************************************
    qgssettingsenumflageditorwrapper.py
    ---------------------
    Date                 : October 2024
    Copyright            : (C) 2021 by Denis Rouzaud
    Email                : denis@opengis.ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

from qgis.PyQt.QtWidgets import QComboBox

from qgis.core import QgsSettingsEntryBase
from qgis.gui import QgsSettingsEditorWidgetWrapper


class PyQgsSettingsEnumEditorWidgetWrapper(QgsSettingsEditorWidgetWrapper):
    """
    A settings editor widget wrapper for enum settings as PyQgsSettingsEntryEnumFlag
    """

    def __init__(
        self, parent=None, editor=None, setting=None, displayStrings: dict = None
    ):
        self.setting = setting
        self.editor = editor
        self.displayStrings = {}
        super().__init__(parent)
        if editor is not None and setting is not None:
            if displayStrings:
                self.displayStrings = displayStrings
            self.configureEditor(editor, setting)

    def id(self):
        return "py-enum"

    def createWrapper(self, parent=None):
        return PyQgsSettingsEnumEditorWidgetWrapper(parent)

    def setWidgetFromSetting(self):
        if self.setting:
            return self.setWidgetFromVariant(
                self.setting.valueAsVariant(self.dynamicKeyPartList())
            )
        return False

    def setSettingFromWidget(self):
        if self.editor:
            self.setting.setVariantValue(
                self.variantValueFromWidget(), self.dynamicKeyPartList()
            )
            return True
        else:
            return False

    def variantValueFromWidget(self):
        if self.editor:
            return self.editor.currentData()
        return None

    def setWidgetFromVariant(self, value):
        if self.editor and value is not None:
            if isinstance(value, int):
                value = self.setting.metaEnum().valueToKey(value)
            idx = self.editor.findData(value)
            self.editor.setCurrentIndex(idx)
            return idx >= 0
        return False

    def createEditorPrivate(self, parent=None):
        return QComboBox(parent)

    def configureEditorPrivate(self, editor: QComboBox, setting: QgsSettingsEntryBase):
        self.setting = setting
        if isinstance(editor, QComboBox):
            self.editor = editor
            for i in range(self.setting.metaEnum().keyCount()):
                value = self.setting.metaEnum().value(i)
                key = self.setting.metaEnum().key(i)
                text = self.displayStrings.get(value, key)
                self.editor.addItem(text, key)
            return True
        else:
            return False

    def enableAutomaticUpdatePrivate(self):
        self.editor.currentIndexChanged.connect(
            lambda: self.setting.setValue(
                self.editor.currentData(), self.dynamicKeyPartList()
            )
        )
