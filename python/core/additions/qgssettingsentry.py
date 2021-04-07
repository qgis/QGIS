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
from qgis.core import QgsSettings, QgsSettingsEntryBase, QgsLogger
import qgis  # required to get base class of enums


class PyQgsSettingsEntryEnum(QgsSettingsEntryBase):
    """ class QgsSettingsEntryEnum
        ingroup core
        An enum settings entry.
        since QGIS 3.20
    """

    def __init__(self, key, pluginName, defaultValue, description=str()):
        """ Constructor for QgsSettingsEntryEnum.

            :param self: the QgsSettingsEntryEnum object
            :param key: argument specifies the final part of the settings key.
            :param pluginName: argument is inserted in the key after the section.
            :param defaultValue: argument specifies the default value for the settings entry.
            :param description: argument specifies a description for the settings entry.

            .. note::  The enum needs to be declared with Q_ENUM.
        """

        defaultValueStr = str()
        self.__metaEnum = metaEnumFromValue(defaultValue)
        if self.__metaEnum is None or not self.__metaEnum.isValid():
            QgsLogger.debug("Invalid metaenum. Enum probably misses Q_ENUM or Q_FLAG declaration.")
        else:
            defaultValueStr = self.__metaEnum.valueToKey(defaultValue)
            self.__enumClass = defaultValue.__class__

        super().__init__(key, pluginName, defaultValueStr, description)

    def value(self, dynamicKeyPart=str()):
        """ Get settings value.

            :param self: the QgsSettingsEntryEnum object
            :param dynamicKeyPart: argument specifies the dynamic part of the settings key.
        """

        return QgsSettings().enumValue(self.key(dynamicKeyPart),
                                       self.defaultValue(),
                                       self.section())

    def defaultValue(self):
        """ Get settings default value.

            :param self: the QgsSettingsEntryEnum object
        """

        if self.__metaEnum is None or not self.__metaEnum.isValid():
            QgsLogger.debug("Invalid metaenum. Enum probably misses Q_ENUM or Q_FLAG declaration.")
            return -1

        defaultValueString = self.defaultValueAsVariant()
        (defaultValue, ok) = self.__metaEnum.keyToValue(defaultValueString)
        if not ok:
            QgsLogger.debug("Invalid enum key '{0}'.".format(self.defaultValueAsVariant()))
            return -1

        # cast to the enum class
        defaultValue = self.__enumClass(defaultValue)
        return defaultValue

    def setValue(self, value, dynamicKeyPart=str()):
        """ Set settings value.

            :param self: the QgsSettingsEntryEnum object
            :param dynamicKeyPart: argument specifies the dynamic part of the settings key.
         """

        if self.__metaEnum is None or not self.__metaEnum.isValid():
            QgsLogger.debug("Invalid metaenum. Enum probably misses Q_ENUM or Q_FLAG declaration.")
            return False

        enumKey = str()
        enumKey = self.__metaEnum.valueToKey(value)
        if not enumKey:
            QgsLogger.debug("Invalid enum value '{0}'.".format(value))
            return False

        super().setValue(enumKey, dynamicKeyPart)
        return True

    def settingsType(self):
        """ Get the settings entry type.

            :param self: the QgsSettingsEntryEnum object
        """

        return self.Enum


class PyQgsSettingsEntryFlag(QgsSettingsEntryBase):
    """ class QgsSettingsEntryFlag
        ingroup core
        A flag settings entry.
        since QGIS 3.20
    """

    def __init__(self, key, pluginName, defaultValue, description=str()):
        """ Constructor for QgsSettingsEntryFlag.

            :param self: the QgsSettingsEntryFlag object
            :param key: argument specifies the final part of the settings key.
            :param pluginName: argument is inserted in the key after the section.
            :param defaultValue: argument specifies the default value for the settings entry.
            :param description: argument specifies a description for the settings entry.

            .. note::  The flag needs to be declared with Q_FLAG (not Q_FLAGS).
        """

        defaultValueStr = str()
        self.__metaEnum = metaEnumFromValue(defaultValue)
        if self.__metaEnum is None or not self.__metaEnum.isValid():
            QgsLogger.debug("Invalid metaenum. Enum probably misses Q_ENUM or Q_FLAG declaration.")
        else:
            defaultValueStr = self.__metaEnum.valueToKeys(defaultValue)
            self.__flagClass = defaultValue.__class__

        super().__init__(key, pluginName, defaultValueStr, description)

    def value(self, dynamicKeyPart=str()):
        """ Get settings value.

            :param self: the QgsSettingsEntryFlag object
            :param dynamicKeyPart: argument specifies the dynamic part of the settings key.
        """

        return QgsSettings().flagValue(self.key(dynamicKeyPart),
                                       self.defaultValue(),
                                       self.section())

    def defaultValue(self):
        """ Get settings default value.

            :param self: the QgsSettingsEntryFlag object
        """

        if self.__metaEnum is None or not self.__metaEnum.isValid():
            QgsLogger.debug("Invalid metaenum. Flag probably misses Q_ENUM or Q_FLAG declaration.")
            return -1

        defaultValueString = self.defaultValueAsVariant()
        (defaultValue, ok) = self.__metaEnum.keysToValue(defaultValueString)
        if not ok:
            QgsLogger.debug("Invalid flag keys '{0}'.".format(self.defaultValueAsVariant()))
            return -1

        # cast to the flag class
        defaultValue = self.__flagClass(defaultValue)
        return defaultValue

    def setValue(self, value, dynamicKeyPart=str()):
        """ Set settings value.

            :param self: the QgsSettingsEntryFlag object
            :param dynamicKeyPart: argument specifies the dynamic part of the settings key.
         """

        if self.__metaEnum is None or not self.__metaEnum.isValid():
            QgsLogger.debug("Invalid metaenum. Flag probably misses Q_ENUM or Q_FLAG declaration.")
            return False

        flagKeys = str()
        flagKeys = self.__metaEnum.valueToKeys(value)
        if not flagKeys:
            QgsLogger.debug("Invalid flag value '{0}'.".format(value))
            return False

        super().setValue(flagKeys, dynamicKeyPart)
        return True

    def settingsType(self):
        """ Get the settings entry type.

            :param self: the QgsSettingsEntryFlag object
        """

        return self.Flag
