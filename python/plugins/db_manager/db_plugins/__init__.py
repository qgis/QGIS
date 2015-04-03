# -*- coding: utf-8 -*-

"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QGIS
Date                 : May 23, 2011
copyright            : (C) 2011 by Giuseppe Sucameli
email                : brush.tyler@gmail.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""


class NotSupportedDbType(Exception):
    def __init__(self, dbtype):
        self.msg = self.tr("%s is not supported yet") % dbtype
        Exception(self, self.msg)

    def __str__(self):
        return self.msg.encode('utf-8')


def initDbPluginList():
    import os

    current_dir = os.path.dirname(__file__)
    for name in os.listdir(current_dir):
        if not os.path.isdir(os.path.join(current_dir, name)):
            continue

        try:
            exec ( u"from .%s import plugin as mod" % name )
        except ImportError, e:
            DBPLUGIN_ERRORS.append(u"%s: %s" % (name, e.message))
            continue

        pluginclass = mod.classFactory()
        SUPPORTED_DBTYPES[pluginclass.typeName()] = pluginclass

    return len(SUPPORTED_DBTYPES) > 0


def supportedDbTypes():
    return sorted(SUPPORTED_DBTYPES.keys())


def getDbPluginErrors():
    return DBPLUGIN_ERRORS


def createDbPlugin(dbtype, conn_name=None):
    if dbtype not in SUPPORTED_DBTYPES:
        raise NotSupportedDbType(dbtype)
    dbplugin = SUPPORTED_DBTYPES[dbtype]
    return dbplugin if conn_name is None else dbplugin(conn_name)


# initialize the plugin list
SUPPORTED_DBTYPES = {}
DBPLUGIN_ERRORS = []
initDbPluginList()
