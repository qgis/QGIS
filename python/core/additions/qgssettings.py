# -*- coding: utf-8 -*-

"""
***************************************************************************
    qgssettings.py
    ---------------------
    Date                 : May 2018
    Copyright            : (C) 2018 by Denis Rouzaud
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

from .metaenum import metaEnumFromValue
from qgis.core import QgsSettings
import qgis  # required to get base class of enums


def _qgssettings_enum_value(self, key, enumDefaultValue, section=QgsSettings.NoSection):
    """
    Return the setting value for a setting based on an enum.
    This forces the output to be a valid and existing entry of the enum.
    Hence if the setting value is incorrect, the given default value is returned.

    :param self: the QgsSettings object
    :param key: the setting key
    :param enumDefaultValue: the default value as an enum value
    :param section: optional section
    :return: the setting value

     .. note::  The enum needs to be declared with Q_ENUM.

    """

    meta_enum = metaEnumFromValue(enumDefaultValue)
    if meta_enum is None or not meta_enum.isValid():
        # this should not happen
        raise ValueError("could not get the meta enum for given enum default value (type: {})"
                         .format(enumDefaultValue.__class__))

    str_val = self.value(key, meta_enum.valueToKey(enumDefaultValue), str, section)
    # need a new meta enum as QgsSettings.value is making a copy and leads to seg fault (probably a PyQt issue)
    meta_enum_2 = metaEnumFromValue(enumDefaultValue)
    (enu_val, ok) = meta_enum_2.keyToValue(str_val)

    if not ok:
        enu_val = enumDefaultValue
    else:
        # cast to the enum class
        enu_val = enumDefaultValue.__class__(enu_val)

    return enu_val


def _qgssettings_set_enum_value(self, key, enumValue, section=QgsSettings.NoSection):
    """
    Save the setting value for a setting based on an enum.
    This forces the output to be a valid and existing entry of the enum.
    The entry is saved as a string.

    :param self: the QgsSettings object
    :param key: the setting key
    :param enumValue: the value to be saved
    :param section: optional section
    :return: the setting value

     .. note::  The enum needs to be declared with Q_ENUM.

    """
    meta_enum = metaEnumFromValue(enumValue)
    if meta_enum is None or not meta_enum.isValid():
        # this should not happen
        raise ValueError("could not get the meta enum for given enum default value (type: {})".format(type(enumValue)))

    self.setValue(key, meta_enum.valueToKey(enumValue), section)


def _qgssettings_flag_value(self, key, flagDefaultValue, section=QgsSettings.NoSection):
    """
    Return the setting value for a setting based on a flag.
    This forces the output to be a valid and existing entry of the enum.
    Hence if the setting value is incorrect, the given default value is returned.

    :param self: the QgsSettings object
    :param key: the setting key
    :param flagDefaultValue: the default value as a flag value
    :param section: optional section
    :return: the setting value

     .. note::  The flag needs to be declared with Q_FLAG (not Q_FLAGS).

    """

    # There is an issue in SIP, flags.__class__ does not return the proper class
    # (e.g. Filters instead of QgsMapLayerProxyModel.Filters)
    # dirty hack to get the parent class
    __import__(flagDefaultValue.__module__)
    baseClass = None
    exec("baseClass={module}.{flag_class}".format(module=flagDefaultValue.__module__.replace('_', ''),
                                                  flag_class=flagDefaultValue.__class__.__name__))

    meta_enum = metaEnumFromValue(flagDefaultValue, baseClass)
    if meta_enum is None or not meta_enum.isValid():
        # this should not happen
        raise ValueError("could not get the meta enum for given enum default value (type: {})".format(type(flagDefaultValue)))

    str_val = self.value(key, meta_enum.valueToKey(flagDefaultValue), str, section)
    # need a new meta enum as QgsSettings.value is making a copy and leads to seg fault (probably a PyQt issue)
    meta_enum_2 = metaEnumFromValue(flagDefaultValue)
    (flag_val, ok) = meta_enum_2.keysToValue(str_val)

    if not ok:
        flag_val = flagDefaultValue
    else:
        flag_val = flagDefaultValue.__class__(flag_val)

    return flag_val
