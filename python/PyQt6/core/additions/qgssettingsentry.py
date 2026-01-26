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
from enum import IntFlag, Flag
from qgis.core import (
    QgsSettings,
    QgsSettingsTree,
    QgsSettingsEntryBase,
    QgsLogger,
    Qgis,
)
import qgis  # required to get base class of enums


class PyQgsSettingsEntryEnumFlag(QgsSettingsEntryBase):
    """
    class PyQgsSettingsEntryEnumFlag
    An enum settings entry.
    since QGIS 3.20
    """

    def __init__(
        self,
        key,
        pluginName,
        defaultValue,
        description="",
        options=Qgis.SettingsOptions(),
    ):
        """
        Constructor for PyQgsSettingsEntryEnumFlag.

        :param key: argument specifies the final part of the settings key.
        :param pluginName: argument is either the plugin name or the settings tree parent element
        :param defaultValue: argument specifies the default value for the settings entry.
        :param description: argument specifies a description for the settings entry.
        """

        # TODO QGIS 5: rename pluginName arg to parent and key to name

        self.options = options
        self.__enum_class = defaultValue.__class__

        if issubclass(self.__enum_class, (IntFlag, Flag)):
            defaultValueStr = "|".join(
                [e.name for e in self.__enum_class if defaultValue & e]
            )
        else:
            defaultValueStr = defaultValue.name

        if type(pluginName) is str:
            parent = QgsSettingsTree.createPluginTreeNode(pluginName)
        else:
            parent = pluginName
        super().__init__(key, parent, defaultValueStr, description, options)

    def typeId(self):
        """
        Defines a custom id since this class has not the same API as the cpp implementation
        """
        return "py-enumflag"

    def value(self, dynamicKeyPart=None):
        """
        Get settings value.

        :param dynamicKeyPart: argument specifies the dynamic part of the settings key.
        """
        v = QgsSettings().value(self.key(dynamicKeyPart), self.defaultValue())
        if issubclass(self.__enum_class, (IntFlag, Flag)):
            if isinstance(v, str):
                flag_val = self.__enum_class(0)
                for part in v.split("|"):
                    try:
                        flag_val |= getattr(self.__enum_class, part)
                    except AttributeError:
                        QgsLogger.debug(f"Invalid enum/flag key/s '{v}'.")
                        return -1
                return flag_val
            elif isinstance(v, int):
                return self.__enum_class(v)
        else:
            if isinstance(v, str):
                try:
                    flag_val = getattr(self.__enum_class, v)
                except AttributeError:
                    QgsLogger.debug(f"Invalid enum/flag key/s '{v}'.")
                    return -1
                return flag_val
            elif isinstance(v, int):
                return self.__enum_class(v)

    def valueWithDefaultOverride(self, defaultValueOverride, dynamicKeyPart=None):
        """
        Get settings value with a default value override.

        :param defaultValueOverride: argument if valid is used instead of the normal default value.
        :param dynamicKeyPart: argument specifies the dynamic part of the settings key.
        """
        if self.__metaEnum.isFlag():
            return QgsSettings().flagValue(
                self.key(dynamicKeyPart), defaultValueOverride
            )
        else:
            return QgsSettings().enumValue(
                self.key(dynamicKeyPart), defaultValueOverride
            )

    def defaultValue(self):
        """
        Get settings default value.
        """
        defaultValueString = self.defaultValueAsVariant()

        if issubclass(self.__enum_class, (IntFlag, Flag)):
            flag_val = self.__enum_class(0)
            for part in defaultValueString.split("|"):
                try:
                    flag_val |= getattr(self.__enum_class, part)
                except AttributeError:
                    QgsLogger.debug(f"Invalid enum/flag key/s '{part}'.")
                    return -1
            return flag_val
        else:
            try:
                return getattr(self.__enum_class, defaultValueString)
            except AttributeError:
                QgsLogger.debug(
                    f"Invalid enum/flag key/s '{self.defaultValueAsVariant()}'."
                )
                return -1

    def setValue(self, value, dynamicKeyPart: (list, str) = None):
        """
        Set settings value.

        :param value: the value to set for the setting.
        :param dynamicKeyPart: argument specifies the dynamic part of the settings key (a single one a string, or several as a list)
        """
        if self.options & Qgis.SettingsOption.SaveEnumFlagAsInt:
            enum_flag_key = value.value
        else:
            if issubclass(self.__enum_class, (IntFlag, Flag)):
                enum_flag_key = "|".join(
                    [e.name for e in self.__enum_class if value & e]
                )
            else:
                if isinstance(value, int):
                    try:
                        enum_flag_key = [
                            e.name for e in self.__enum_class if value == e.value
                        ][0]
                    except IndexError:
                        return False
                else:
                    enum_flag_key = value.name

        if type(dynamicKeyPart) is str:
            dynamicKeyPart = [dynamicKeyPart]
        elif dynamicKeyPart is None:
            dynamicKeyPart = []

        return super().setVariantValue(enum_flag_key, dynamicKeyPart)

    def settingsType(self):
        """
        Get the settings entry type.
        """
        return self.SettingsType.EnumFlag
