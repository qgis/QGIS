# -*- coding: utf-8 -*-

"""
***************************************************************************
    qgssettingsentry.py
    ---------------------
    Date                 : April 2021
    Copyright            : (C) 2021 by Damiano Lombardi
    Email                : damiano@opengis.ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

from .metaenum import metaEnumFromValue
from qgis.core import QgsSettings, QgsSettingsTree, QgsSettingsEntryBase, QgsLogger, Qgis
import qgis  # required to get base class of enums


class PyQgsSettingsEntryEnumFlag(QgsSettingsEntryBase):
    """
    class PyQgsSettingsEntryEnumFlag
    An enum settings entry.
    since QGIS 3.20
    """

    def __init__(self, key, pluginName, defaultValue, description=str(), options=Qgis.SettingsOptions()):
        """
        Constructor for PyQgsSettingsEntryEnumFlag.

        :param key: argument specifies the final part of the settings key.
        :param pluginName: argument is either the plugin name or the settings tree parent element
        :param defaultValue: argument specifies the default value for the settings entry.
        :param description: argument specifies a description for the settings entry.
        """

        # TODO QGIS 4: rename pluginName arg to parent and key to name

        self.options = options
        defaultValueStr = str()
        self.__metaEnum = metaEnumFromValue(defaultValue)
        if self.__metaEnum is None or not self.__metaEnum.isValid():
            QgsLogger.debug("Invalid metaenum. Enum/Flag probably misses Q_ENUM/Q_FLAG declaration. Settings key: '{0}'".format(self.key()))
        else:
            if self.__metaEnum.isFlag():
                defaultValueStr = self.__metaEnum.valueToKeys(defaultValue)
            else:
                defaultValueStr = self.__metaEnum.valueToKey(defaultValue)
            self.__enumFlagClass = defaultValue.__class__

        if isinstance(pluginName, str):
            parent = QgsSettingsTree.createPluginTreeNode(pluginName)
        else:
            parent = pluginName
        super().__init__(key, parent, defaultValueStr, description, options)

    def value(self, dynamicKeyPart=None):
        """
        Get settings value.

        :param dynamicKeyPart: argument specifies the dynamic part of the settings key.
        """
        if self.__metaEnum.isFlag():
            return QgsSettings().flagValue(self.key(dynamicKeyPart), self.defaultValue())
        else:
            return QgsSettings().enumValue(self.key(dynamicKeyPart), self.defaultValue())

    def valueWithDefaultOverride(self, defaultValueOverride, dynamicKeyPart=None):
        """
        Get settings value with a default value override.

        :param defaultValueOverride: argument if valid is used instead of the normal default value.
        :param dynamicKeyPart: argument specifies the dynamic part of the settings key.
        """
        if self.__metaEnum.isFlag():
            return QgsSettings().flagValue(self.key(dynamicKeyPart), defaultValueOverride)
        else:
            return QgsSettings().enumValue(self.key(dynamicKeyPart), defaultValueOverride)

    def defaultValue(self):
        """
        Get settings default value.
        """

        if self.__metaEnum is None or not self.__metaEnum.isValid():
            QgsLogger.debug("Invalid metaenum. Enum/Flag probably misses Q_ENUM/Q_FLAG declaration. Settings key: '{0}'".format(self.key()))
            return -1

        defaultValueString = self.defaultValueAsVariant()
        if self.__metaEnum.isFlag():
            (defaultValue, ok) = self.__metaEnum.keysToValue(defaultValueString)
        else:
            (defaultValue, ok) = self.__metaEnum.keyToValue(defaultValueString)
        if not ok:
            QgsLogger.debug("Invalid enum/flag key/s '{0}'.".format(self.defaultValueAsVariant()))
            return -1

        # cast to the enum class
        defaultValue = self.__enumFlagClass(defaultValue)
        return defaultValue

    def setValue(self, value, dynamicKeyPart: (list, str) = None):
        """
        Set settings value.

        :param value: the value to set for the setting.
        :param dynamicKeyPart: argument specifies the dynamic part of the settings key (a single one a string, or several as a list)
        """

        if self.__metaEnum is None or not self.__metaEnum.isValid():
            QgsLogger.debug("Invalid metaenum. Enum/Flag probably misses Q_ENUM/Q_FLAG declaration. Settings key: '{0}'".format(self.key()))
            return False

        if self.options & Qgis.SettingsOption.SaveEnumFlagAsInt:
            enum_flag_key = int(value)
        else:
            if self.__metaEnum.isFlag():
                enum_flag_key = self.__metaEnum.valueToKeys(value)
            else:
                enum_flag_key = self.__metaEnum.valueToKey(value)
            if not enum_flag_key:
                QgsLogger.debug("Invalid enum/flag value '{0}'.".format(value))
                return False

        if isinstance(dynamicKeyPart, str):
            dynamicKeyPart = [dynamicKeyPart]
        elif dynamicKeyPart is None:
            dynamicKeyPart = []

        return super().setVariantValue(enum_flag_key, dynamicKeyPart)

    def settingsType(self):
        """
        Get the settings entry type.
        """

        return self.SettingsType.EnumFlag
