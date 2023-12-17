# -*- coding: utf-8 -*-

"""
***************************************************************************
    metaenum.py
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

META_OBJECT_BY_ENUM_CLASS = {}
META_ENUM_BY_ENUM_CLASS = {}


def metaEnumFromValue(enumValue, baseClass=None, raiseException=True):
    """
    Returns the QMetaEnum for an enum value.
    The enum must have declared using the Q_ENUM macro

    :param enumValue: the enum value
    :param baseClass: the enum base class. If not given, it will try to get it by using `enumValue.__class__.baseClass`
    :param raiseException: if False, no exception will be raised and None will be return in case of failure
    :return: the QMetaEnum if it succeeds, None otherwise
    """
    return metaEnumFromType(enumValue.__class__, baseClass, raiseException)


def metaEnumFromType(enumClass, baseClass=None, raiseException=True):
    """
    Returns the QMetaEnum for an enum type.
    The enum must have declared using the Q_ENUM macro

    :param enumClass: the enum class
    :param baseClass: the enum base class. If not given, it will try to get it by using `enumValue.__class__.baseClass`
    :param raiseException: if False, no exception will be raised and None will be return in case of failure
    :return: the QMetaEnum if it succeeds, None otherwise
    """
    global META_OBJECT_BY_ENUM_CLASS
    global META_ENUM_BY_ENUM_CLASS
    if enumClass in META_ENUM_BY_ENUM_CLASS:
        return META_ENUM_BY_ENUM_CLASS[enumClass]

    if enumClass == int:
        if raiseException:
            raise TypeError("enumClass is an int, while it should be an enum")
        else:
            return None

    if baseClass is None:
        try:
            baseClass = enumClass.baseClass
            return metaEnumFromType(enumClass, baseClass, raiseException)
        except AttributeError:
            if raiseException:
                raise ValueError("Enum type does not implement baseClass method. Provide the base class as argument.")

    try:
        meta_object = baseClass.staticMetaObject
        META_OBJECT_BY_ENUM_CLASS[enumClass] = meta_object
        idx = meta_object.indexOfEnumerator(enumClass.__name__)
        meta_enum = meta_object.enumerator(idx)
        META_ENUM_BY_ENUM_CLASS[enumClass] = meta_enum
    except AttributeError:
        if raiseException:
            raise TypeError("could not get the metaEnum for {}".format(enumClass.__name__))
        meta_enum = None

    return meta_enum
