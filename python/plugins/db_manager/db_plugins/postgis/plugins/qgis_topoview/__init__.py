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

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *

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
        db.connector._execute( c, sql )
        res = db.connector._fetchone( c )
        if res == None or int(res[0]) <= 0:
                return

        # add the action to the DBManager menu
        action = QAction( QIcon(), "&TopoViewer", db )
        mainwindow.registerAction( action, "&Schema", run )


# The run function is called once the user clicks on the action TopoViewer
# (look above at the load function) from the DBManager menu/toolbar.
# @param item is the selected db item (either db, schema or table)
# @param action is the clicked action on the DBManager menu/toolbar
# @param mainwindow is the DBManager mainwindow
def run(item, action, mainwindow):
        db = item.database()
        uri = db.uri()
        conninfo = uri.connectionInfo()
        iface = mainwindow.iface

        quoteId = db.connector.quoteId
        quoteStr = db.connector.quoteString

        # check if the selected item is a topology schema
        isTopoSchema = False

        if not hasattr(item, 'schema'):
                QMessageBox.critical(mainwindow, "Invalid topology", u'Select a topology schema to continue.')
                return False

        if item.schema() != None:
                sql = u"SELECT srid FROM topology.topology WHERE name = %s" % quoteStr(item.schema().name)
                c = db.connector._get_cursor()
                db.connector._execute( c, sql )
                res = db.connector._fetchone( c )
                isTopoSchema = res != None

        if not isTopoSchema:
                QMessageBox.critical(mainwindow, "Invalid topology", u'Schema "%s" is not registered in topology.topology.' % item.schema().name)
                return False

        toposrid = str(res[0])

        # load layers into the current project
        toponame = item.schema().name
        template_dir = os.path.join(current_path, 'templates')
        registry = QgsMapLayerRegistry.instance()
        legend = iface.legendInterface()

        # do not refresh the canvas until all the layers are added
        prevRenderFlagState = iface.mapCanvas().renderFlag()
        iface.mapCanvas().setRenderFlag( False )
        try:
                supergroup = legend.addGroup(u'Topology "%s"' % toponame, False)
                # should not be needed: http://hub.qgis.org/issues/6938
                legend.setGroupVisible(supergroup, False)

                provider = db.dbplugin().providerName()
                uri = db.uri();

                # FACES
                group = legend.addGroup(u'Faces', False, supergroup)
                # should not be needed: http://hub.qgis.org/issues/6938
                legend.setGroupVisible(group, False)

          # face mbr
                uri.setDataSource(toponame, 'face', 'mbr', '', 'face_id')
                uri.setSrid( toposrid )
                uri.setWkbType( QGis.WKBPolygon )
                layer = QgsVectorLayer(uri.uri(), u'%s.face_mbr' % toponame, provider)
                layer.loadNamedStyle(os.path.join(template_dir, 'face_mbr.qml'))
                registry.addMapLayers([layer])
                legend.setLayerVisible(layer, False)
                legend.setLayerExpanded(layer, False)
                legend.moveLayer(layer, group)
                face_extent = layer.extent()

          # face geometry
                sql = u'SELECT face_id, topology.ST_GetFaceGeometry(%s, face_id) as geom ' \
                       'FROM %s.face WHERE face_id > 0' % (quoteStr(toponame), quoteId(toponame))
                uri.setDataSource('', u'(%s\n)' % sql, 'geom', '', 'face_id')
                uri.setSrid( toposrid )
                uri.setWkbType( QGis.WKBPolygon )
                layer = QgsVectorLayer(uri.uri(), u'%s.face' % toponame, provider)
                layer.setExtent(face_extent)
                layer.loadNamedStyle(os.path.join(template_dir, 'face.qml'))
                registry.addMapLayers([layer])
                legend.setLayerVisible(layer, False)
                legend.setLayerExpanded(layer, False)
                legend.moveLayer(layer, group)

          # face_seed
                sql = u'SELECT face_id, ST_PointOnSurface(topology.ST_GetFaceGeometry(%s, face_id)) as geom ' \
                       'FROM %s.face WHERE face_id > 0' % (quoteStr(toponame), quoteId(toponame))
                uri.setDataSource('', u'(%s)' % sql, 'geom', '', 'face_id')
                uri.setSrid( toposrid )
                uri.setWkbType( QGis.WKBPoint )
                layer = QgsVectorLayer(uri.uri(), u'%s.face_seed' % toponame, provider)
                layer.setExtent(face_extent)
                layer.loadNamedStyle(os.path.join(template_dir, 'face_seed.qml'))
                registry.addMapLayers([layer])
                legend.setLayerVisible(layer, False)
                legend.setLayerExpanded(layer, False)
                legend.moveLayer(layer, group)

          # TODO: add polygon0, polygon1 and polygon2 ?


                # NODES
                group = legend.addGroup(u'Nodes', False, supergroup)
                # should not be needed: http://hub.qgis.org/issues/6938
                legend.setGroupVisible(group, False)

          # node
                uri.setDataSource(toponame, 'node', 'geom', '', 'node_id')
                uri.setSrid( toposrid )
                uri.setWkbType( QGis.WKBPoint )
                layer = QgsVectorLayer(uri.uri(), u'%s.node' % toponame, provider)
                layer.loadNamedStyle(os.path.join(template_dir, 'node.qml'))
                registry.addMapLayers([layer])
                legend.setLayerVisible(layer, False)
                legend.setLayerExpanded(layer, False)
                legend.moveLayer(layer, group)
                node_extent = layer.extent()

          # node labels
                uri.setDataSource(toponame, 'node', 'geom', '', 'node_id')
                uri.setSrid( toposrid )
                uri.setWkbType( QGis.WKBPoint )
                layer = QgsVectorLayer(uri.uri(), u'%s.node_id' % toponame, provider)
                layer.setExtent(node_extent)
                layer.loadNamedStyle(os.path.join(template_dir, 'node_label.qml'))
                registry.addMapLayers([layer])
                legend.setLayerVisible(layer, False)
                legend.setLayerExpanded(layer, False)
                legend.moveLayer(layer, group)

                # EDGES
                group = legend.addGroup(u'Edges', False, supergroup)
                # should not be needed: http://hub.qgis.org/issues/6938
                legend.setGroupVisible(group, False)

          # edge
                uri.setDataSource(toponame, 'edge_data', 'geom', '', 'edge_id')
                uri.setSrid( toposrid )
                uri.setWkbType( QGis.WKBLineString )
                layer = QgsVectorLayer(uri.uri(), u'%s.edge' % toponame, provider)
                registry.addMapLayers([layer])
                legend.setLayerVisible(layer, False)
                legend.setLayerExpanded(layer, False)
                legend.moveLayer(layer, group)
                edge_extent = layer.extent()

          # directed edge
                uri.setDataSource(toponame, 'edge_data', 'geom', '', 'edge_id')
                uri.setSrid( toposrid )
                uri.setWkbType( QGis.WKBLineString )
                layer = QgsVectorLayer(uri.uri(), u'%s.directed_edge' % toponame, provider)
                layer.setExtent(edge_extent)
                layer.loadNamedStyle(os.path.join(template_dir, 'edge.qml'))
                registry.addMapLayers([layer])
                legend.setLayerVisible(layer, False)
                legend.setLayerExpanded(layer, False)
                legend.moveLayer(layer, group)


          # edge labels
                uri.setDataSource(toponame, 'edge_data', 'geom', '', 'edge_id')
                uri.setSrid( toposrid )
                uri.setWkbType( QGis.WKBLineString )
                layer = QgsVectorLayer(uri.uri(), u'%s.edge_id' % toponame, provider)
                layer.setExtent(edge_extent)
                layer.loadNamedStyle(os.path.join(template_dir, 'edge_label.qml'))
                registry.addMapLayers([layer])
                legend.setLayerVisible(layer, False)
                legend.setLayerExpanded(layer, False)
                legend.moveLayer(layer, group)

          # face_left
                uri.setDataSource(toponame, 'edge_data', 'geom', '', 'edge_id')
                uri.setSrid( toposrid )
                uri.setWkbType( QGis.WKBLineString )
                layer = QgsVectorLayer(uri.uri(), u'%s.face_left' % toponame, provider)
                layer.setExtent(edge_extent)
                layer.loadNamedStyle(os.path.join(template_dir, 'face_left.qml'))
                registry.addMapLayers([layer])
                legend.setLayerVisible(layer, False)
                legend.setLayerExpanded(layer, False)
                legend.moveLayer(layer, group)

          # face_right
                uri.setDataSource(toponame, 'edge_data', 'geom', '', 'edge_id')
                uri.setSrid( toposrid )
                uri.setWkbType( QGis.WKBLineString )
                layer = QgsVectorLayer(uri.uri(), u'%s.face_right' % toponame, provider)
                layer.setExtent(edge_extent)
                layer.loadNamedStyle(os.path.join(template_dir, 'face_right.qml'))
                registry.addMapLayers([layer])
                legend.setLayerVisible(layer, False)
                legend.setLayerExpanded(layer, False)
                legend.moveLayer(layer, group)

          # next_left
                uri.setDataSource(toponame, 'edge_data', 'geom', '', 'edge_id')
                uri.setSrid( toposrid )
                uri.setWkbType( QGis.WKBLineString )
                layer = QgsVectorLayer(uri.uri(), u'%s.next_left' % toponame, provider)
                layer.setExtent(edge_extent)
                layer.loadNamedStyle(os.path.join(template_dir, 'next_left.qml'))
                registry.addMapLayers([layer])
                legend.setLayerVisible(layer, False)
                legend.setLayerExpanded(layer, False)
                legend.moveLayer(layer, group)

          # next_right
                uri.setDataSource(toponame, 'edge_data', 'geom', '', 'edge_id')
                uri.setSrid( toposrid )
                uri.setWkbType( QGis.WKBLineString )
                layer = QgsVectorLayer(uri.uri(), u'%s.next_right' % toponame, provider)
                layer.setExtent(edge_extent)
                layer.loadNamedStyle(os.path.join(template_dir, 'next_right.qml'))
                registry.addMapLayers([layer])
                legend.setLayerVisible(layer, False)
                legend.setLayerExpanded(layer, False)
                legend.moveLayer(layer, group)

        finally:
                # restore canvas render flag
                iface.mapCanvas().setRenderFlag( prevRenderFlagState )

        return True

