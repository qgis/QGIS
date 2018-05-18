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


def metaEnumFromValue(enumValue, raiseException=True, baseClass=None):
    return metaEnumFromType(enumValue.__class__, raiseException, baseClass)


def metaEnumFromType(enumClass, raiseException=True, baseClass=None):
    if enumClass == int:
        if raiseException:
            raise TypeError("enumClass is an int, while it should be an enum")
        else:
            return None

    if baseClass is None:
        try:
            baseClass = enumClass.baseClass
            return metaEnumFromType(enumClass, raiseException, baseClass)
        except AttributeError:
            if raiseException:
                raise ValueError("Enum type does not implement baseClass method. Provide the base class as argument.")

    try:
        idx = baseClass.staticMetaObject.indexOfEnumerator(enumClass.__name__)
        meta_enum = baseClass.staticMetaObject.enumerator(idx)
    except AttributeError:
        if raiseException:
            raise TypeError("could not get the metaEnum for {}".format(enumClass.__name__))
        meta_enum = None

    return meta_enum
