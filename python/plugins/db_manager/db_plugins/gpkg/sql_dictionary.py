"""
***************************************************************************
    sql_dictionary.py
    ---------------------
    Date                 : April 2012
    Copyright            : (C) 2012 by Giuseppe Sucameli
    Email                : brush dot tyler at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""


def getSqlDictionary(spatial=True):
    from ..spatialite.sql_dictionary import getSqlDictionary

    return getSqlDictionary(spatial)


def getQueryBuilderDictionary():
    from ..spatialite.sql_dictionary import getQueryBuilderDictionary

    return getQueryBuilderDictionary()
