# -*- coding: utf-8 -*-

"""
/***************************************************************************
Name                 : TopoViewer plugin for DB Manager
Description          : Create a project to display topology schema on QGis
Date                 : Sep 23, 2011
copyright            : (C) 2011 by Giuseppe Sucameli
email                : brush.tyler@gmail.com

Based on qgis_pgis_topoview by Sandro Santilli <strk@keybit.net>
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

from PyQt4.QtGui import QAction, QIcon
from qgis.core import QgsMapLayerRegistry, QgsVectorLayer, QGis
from qgis.gui import QgsMessageBar

import os

current_path = os.path.dirname(__file__)


# The load function is called when the "db" database or either one of its
# children db objects (table o schema) is selected by the user.
# @param db is the selected database
# @param mainwindow is the DBManager mainwindow
def load(db, mainwindow):
    # check whether the selected database supports topology
    # (search for topology.topology)
    sql = u"""SELECT count(*)
                FROM pg_class AS cls JOIN pg_namespace AS nsp ON nsp.oid = cls.relnamespace
                WHERE cls.relname = 'topology' AND nsp.nspname = 'topology'"""
    c = db.connector._get_cursor()
    db.connector._execute(c, sql)
    res = db.connector._fetchone(c)
    if res is None or int(res[0]) <= 0:
        return

    # add the action to the DBManager menu
    action = QAction(QIcon(), "&TopoViewer", db)
    mainwindow.registerAction(action, "&Schema", run)


# The run function is called once the user clicks on the action TopoViewer
# (look above at the load function) from the DBManager menu/toolbar.
# @param item is the selected db item (either db, schema or table)
# @param action is the clicked action on the DBManager menu/toolbar
# @param mainwindow is the DBManager mainwindow
def run(item, action, mainwindow):
    db = item.database()
    uri = db.uri()
    iface = mainwindow.iface

    quoteId = db.connector.quoteId
    quoteStr = db.connector.quoteString

    # check if the selected item is a topology schema
    isTopoSchema = False

    if not hasattr(item, 'schema'):
        mainwindow.infoBar.pushMessage("Invalid topology", u'Select a topology schema to continue.', QgsMessageBar.INFO,
                                       mainwindow.iface.messageTimeout())
        return False

    if item.schema() is not None:
        sql = u"SELECT srid FROM topology.topology WHERE name = %s" % quoteStr(item.schema().name)
        c = db.connector._get_cursor()
        db.connector._execute(c, sql)
        res = db.connector._fetchone(c)
        isTopoSchema = res is not None

    if not isTopoSchema:
        mainwindow.infoBar.pushMessage("Invalid topology",
                                       u'Schema "{0}" is not registered in topology.topology.'.format(
                                           item.schema().name), QgsMessageBar.WARNING,
                                       mainwindow.iface.messageTimeout())
        return False

    if (res[0] < 0):
        mainwindow.infoBar.pushMessage("WARNING", u'Topology "{0}" is registered as having a srid of {1} in topology.topology, we will assume 0 (for unknown)'.format(item.schema().name, res[0]), QgsMessageBar.WARNING, mainwindow.iface.messageTimeout())
        toposrid = '0'
    else:
        toposrid = unicode(res[0])

    # load layers into the current project
    toponame = item.schema().name
    template_dir = os.path.join(current_path, 'templates')
    registry = QgsMapLayerRegistry.instance()
    legend = iface.legendInterface()

    # do not refresh the canvas until all the layers are added
    prevRenderFlagState = iface.mapCanvas().renderFlag()
    iface.mapCanvas().setRenderFlag(False)
    try:
        supergroup = legend.addGroup(u'Topology "%s"' % toponame, False)
        provider = db.dbplugin().providerName()
        uri = db.uri()

        # FACES
        group = legend.addGroup(u'Faces', False, supergroup)

        # face mbr
        uri.setDataSource(toponame, 'face', 'mbr', '', 'face_id')
        uri.setSrid(toposrid)
        uri.setWkbType(QGis.WKBPolygon)
        layer = QgsVectorLayer(uri.uri(), u'%s.face_mbr' % toponame, provider)
        layer.loadNamedStyle(os.path.join(template_dir, 'face_mbr.qml'))
        registry.addMapLayers([layer])
        legend.moveLayer(layer, group)
        legend.setLayerVisible(layer, False)
        legend.setLayerExpanded(layer, False)
        face_extent = layer.extent()

        # face geometry
        sql = u'SELECT face_id, topology.ST_GetFaceGeometry(%s, face_id) as geom ' \
              'FROM %s.face WHERE face_id > 0' % (quoteStr(toponame), quoteId(toponame))
        uri.setDataSource('', u'(%s\n)' % sql, 'geom', '', 'face_id')
        uri.setSrid(toposrid)
        uri.setWkbType(QGis.WKBPolygon)
        layer = QgsVectorLayer(uri.uri(), u'%s.face' % toponame, provider)
        layer.setExtent(face_extent)
        layer.loadNamedStyle(os.path.join(template_dir, 'face.qml'))
        registry.addMapLayers([layer])
        legend.moveLayer(layer, group)
        legend.setLayerVisible(layer, False)
        legend.setLayerExpanded(layer, False)

        # face_seed
        sql = u'SELECT face_id, ST_PointOnSurface(topology.ST_GetFaceGeometry(%s, face_id)) as geom ' \
              'FROM %s.face WHERE face_id > 0' % (quoteStr(toponame), quoteId(toponame))
        uri.setDataSource('', u'(%s)' % sql, 'geom', '', 'face_id')
        uri.setSrid(toposrid)
        uri.setWkbType(QGis.WKBPoint)
        layer = QgsVectorLayer(uri.uri(), u'%s.face_seed' % toponame, provider)
        layer.setExtent(face_extent)
        layer.loadNamedStyle(os.path.join(template_dir, 'face_seed.qml'))
        registry.addMapLayers([layer])
        legend.moveLayer(layer, group)
        legend.setLayerVisible(layer, False)
        legend.setLayerExpanded(layer, False)

        # TODO: add polygon0, polygon1 and polygon2 ?

        # NODES
        group = legend.addGroup(u'Nodes', False, supergroup)

        # node
        uri.setDataSource(toponame, 'node', 'geom', '', 'node_id')
        uri.setSrid(toposrid)
        uri.setWkbType(QGis.WKBPoint)
        layer = QgsVectorLayer(uri.uri(), u'%s.node' % toponame, provider)
        layer.loadNamedStyle(os.path.join(template_dir, 'node.qml'))
        registry.addMapLayers([layer])
        legend.moveLayer(layer, group)
        legend.setLayerVisible(layer, False)
        legend.setLayerExpanded(layer, False)
        node_extent = layer.extent()

        # node labels
        uri.setDataSource(toponame, 'node', 'geom', '', 'node_id')
        uri.setSrid(toposrid)
        uri.setWkbType(QGis.WKBPoint)
        layer = QgsVectorLayer(uri.uri(), u'%s.node_id' % toponame, provider)
        layer.setExtent(node_extent)
        layer.loadNamedStyle(os.path.join(template_dir, 'node_label.qml'))
        registry.addMapLayers([layer])
        legend.moveLayer(layer, group)
        legend.setLayerVisible(layer, False)
        legend.setLayerExpanded(layer, False)

        # EDGES
        group = legend.addGroup(u'Edges', False, supergroup)

        # edge
        uri.setDataSource(toponame, 'edge_data', 'geom', '', 'edge_id')
        uri.setSrid(toposrid)
        uri.setWkbType(QGis.WKBLineString)
        layer = QgsVectorLayer(uri.uri(), u'%s.edge' % toponame, provider)
        registry.addMapLayers([layer])
        legend.moveLayer(layer, group)
        legend.setLayerVisible(layer, False)
        legend.setLayerExpanded(layer, False)
        edge_extent = layer.extent()

        # directed edge
        uri.setDataSource(toponame, 'edge_data', 'geom', '', 'edge_id')
        uri.setSrid(toposrid)
        uri.setWkbType(QGis.WKBLineString)
        layer = QgsVectorLayer(uri.uri(), u'%s.directed_edge' % toponame, provider)
        layer.setExtent(edge_extent)
        layer.loadNamedStyle(os.path.join(template_dir, 'edge.qml'))
        registry.addMapLayers([layer])
        legend.moveLayer(layer, group)
        legend.setLayerVisible(layer, False)
        legend.setLayerExpanded(layer, False)

        # edge labels
        uri.setDataSource(toponame, 'edge_data', 'geom', '', 'edge_id')
        uri.setSrid(toposrid)
        uri.setWkbType(QGis.WKBLineString)
        layer = QgsVectorLayer(uri.uri(), u'%s.edge_id' % toponame, provider)
        layer.setExtent(edge_extent)
        layer.loadNamedStyle(os.path.join(template_dir, 'edge_label.qml'))
        registry.addMapLayers([layer])
        legend.moveLayer(layer, group)
        legend.setLayerVisible(layer, False)
        legend.setLayerExpanded(layer, False)

        # face_left
        uri.setDataSource(toponame, 'edge_data', 'geom', '', 'edge_id')
        uri.setSrid(toposrid)
        uri.setWkbType(QGis.WKBLineString)
        layer = QgsVectorLayer(uri.uri(), u'%s.face_left' % toponame, provider)
        layer.setExtent(edge_extent)
        layer.loadNamedStyle(os.path.join(template_dir, 'face_left.qml'))
        registry.addMapLayers([layer])
        legend.moveLayer(layer, group)
        legend.setLayerVisible(layer, False)
        legend.setLayerExpanded(layer, False)

        # face_right
        uri.setDataSource(toponame, 'edge_data', 'geom', '', 'edge_id')
        uri.setSrid(toposrid)
        uri.setWkbType(QGis.WKBLineString)
        layer = QgsVectorLayer(uri.uri(), u'%s.face_right' % toponame, provider)
        layer.setExtent(edge_extent)
        layer.loadNamedStyle(os.path.join(template_dir, 'face_right.qml'))
        registry.addMapLayers([layer])
        legend.moveLayer(layer, group)
        legend.setLayerVisible(layer, False)
        legend.setLayerExpanded(layer, False)

        # next_left
        uri.setDataSource(toponame, 'edge_data', 'geom', '', 'edge_id')
        uri.setSrid(toposrid)
        uri.setWkbType(QGis.WKBLineString)
        layer = QgsVectorLayer(uri.uri(), u'%s.next_left' % toponame, provider)
        layer.setExtent(edge_extent)
        layer.loadNamedStyle(os.path.join(template_dir, 'next_left.qml'))
        registry.addMapLayers([layer])
        legend.moveLayer(layer, group)
        legend.setLayerVisible(layer, False)
        legend.setLayerExpanded(layer, False)

        # next_right
        uri.setDataSource(toponame, 'edge_data', 'geom', '', 'edge_id')
        uri.setSrid(toposrid)
        uri.setWkbType(QGis.WKBLineString)
        layer = QgsVectorLayer(uri.uri(), u'%s.next_right' % toponame, provider)
        layer.setExtent(edge_extent)
        layer.loadNamedStyle(os.path.join(template_dir, 'next_right.qml'))
        registry.addMapLayers([layer])
        legend.moveLayer(layer, group)
        legend.setLayerVisible(layer, False)
        legend.setLayerExpanded(layer, False)

    finally:

        # Set canvas extent to topology extent, if not yet initialized
        canvas = iface.mapCanvas()
        if (canvas.fullExtent().isNull()):
            ext = node_extent
            ext.combineExtentWith(edge_extent)
            # Grow by 1/20 of largest side
            ext = ext.buffer(max(ext.width(), ext.height()) / 20)
            canvas.setExtent(ext)

        # restore canvas render flag
        iface.mapCanvas().setRenderFlag(prevRenderFlagState)

    return True
