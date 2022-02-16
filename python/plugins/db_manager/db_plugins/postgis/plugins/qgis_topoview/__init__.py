# -*- coding: utf-8 -*-

"""
/***************************************************************************
Name                 : TopoViewer plugin for DB Manager
Description          : Create a project to display topology schema on Qgis
Date                 : Sep 23, 2011
copyright            : (C) 2011 by Giuseppe Sucameli
                       (C) 2019 by Sandro Santilli
email                : strk@kbt.io

Based on qgis_pgis_topoview by Sandro Santilli <strk@kbt.io>
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
from builtins import str

from qgis.PyQt.QtWidgets import QAction
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtGui import QIcon
from qgis.core import Qgis, QgsProject, QgsVectorLayer, QgsWkbTypes, QgsLayerTreeGroup
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
    res = db.executeSql(sql)
    if res is None or len(res) < 1 or int(res[0][0]) <= 0:
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
        mainwindow.infoBar.pushMessage("Invalid topology", u'Select a topology schema to continue.', Qgis.Info,
                                       mainwindow.iface.messageTimeout())
        return False

    if item.schema() is not None:
        sql = u"SELECT srid FROM topology.topology WHERE name = %s" % quoteStr(item.schema().name)
        res = db.executeSql(sql)
        isTopoSchema = len(res) > 0

    if not isTopoSchema:
        mainwindow.infoBar.pushMessage("Invalid topology",
                                       u'Schema "{0}" is not registered in topology.topology.'.format(
                                           item.schema().name), Qgis.Warning,
                                       mainwindow.iface.messageTimeout())
        return False

    if (res[0][0] < 0):
        mainwindow.infoBar.pushMessage("WARNING", u'Topology "{0}" is registered as having a srid of {1} in topology.topology, we will assume 0 (for unknown)'.format(item.schema().name, res[0]), Qgis.Warning, mainwindow.iface.messageTimeout())
        toposrid = '0'
    else:
        toposrid = str(res[0][0])

    # load layers into the current project
    toponame = item.schema().name
    template_dir = os.path.join(current_path, 'templates')

    # do not refresh the canvas until all the layers are added
    wasFrozen = iface.mapCanvas().isFrozen()
    iface.mapCanvas().freeze()
    try:
        provider = db.dbplugin().providerName()
        uri = db.uri()

        # Force use of estimated metadata (topologies can be big)
        uri.setUseEstimatedMetadata(True)

        # FACES

        # face mbr
        uri.setDataSource(toponame, 'face', 'mbr', '', 'face_id')
        uri.setSrid(toposrid)
        uri.setWkbType(QgsWkbTypes.Polygon)
        layerFaceMbr = QgsVectorLayer(uri.uri(False), u'%s.face_mbr' % toponame, provider)
        layerFaceMbr.loadNamedStyle(os.path.join(template_dir, 'face_mbr.qml'))

        face_extent = layerFaceMbr.extent()

        # face geometry
        sql = u'SELECT face_id, mbr, topology.ST_GetFaceGeometry(%s,' \
              'face_id)::geometry(polygon, %s) as geom ' \
              'FROM %s.face WHERE face_id > 0' % \
              (quoteStr(toponame), toposrid, quoteId(toponame))
        uri.setDataSource('', u'(%s\n)' % sql, 'geom', '', 'face_id')
        uri.setParam('bbox', 'mbr')
        uri.setParam('checkPrimaryKeyUnicity', '0')
        uri.setSrid(toposrid)
        uri.setWkbType(QgsWkbTypes.Polygon)
        layerFaceGeom = QgsVectorLayer(uri.uri(False), u'%s.face' % toponame, provider)
        layerFaceGeom.setExtent(face_extent)
        layerFaceGeom.loadNamedStyle(os.path.join(template_dir, 'face.qml'))

        # face_seed
        sql = u'SELECT face_id, mbr, ST_PointOnSurface(' \
              'topology.ST_GetFaceGeometry(%s,' \
              'face_id))::geometry(point, %s) as geom ' \
              'FROM %s.face WHERE face_id > 0' % \
              (quoteStr(toponame), toposrid, quoteId(toponame))
        uri.setDataSource('', u'(%s)' % sql, 'geom', '', 'face_id')
        uri.setParam('bbox', 'mbr')
        uri.setParam('checkPrimaryKeyUnicity', '0')
        uri.setSrid(toposrid)
        uri.setWkbType(QgsWkbTypes.Point)
        layerFaceSeed = QgsVectorLayer(uri.uri(False), u'%s.face_seed' % toponame, provider)
        layerFaceSeed.setExtent(face_extent)
        layerFaceSeed.loadNamedStyle(os.path.join(template_dir, 'face_seed.qml'))

        # TODO: add polygon0, polygon1 and polygon2 ?

        # NODES

        # node
        uri.setDataSource(toponame, 'node', 'geom', '', 'node_id')
        uri.removeParam('bbox')
        uri.setSrid(toposrid)
        uri.setWkbType(QgsWkbTypes.Point)
        layerNode = QgsVectorLayer(uri.uri(False), u'%s.node' % toponame, provider)
        layerNode.loadNamedStyle(os.path.join(template_dir, 'node.qml'))
        node_extent = layerNode.extent()

        # node labels
        uri.setDataSource(toponame, 'node', 'geom', '', 'node_id')
        uri.setSrid(toposrid)
        uri.setWkbType(QgsWkbTypes.Point)
        uri.removeParam('bbox')
        layerNodeLabel = QgsVectorLayer(uri.uri(False), u'%s.node_id' % toponame, provider)
        layerNodeLabel.setExtent(node_extent)
        layerNodeLabel.loadNamedStyle(os.path.join(template_dir, 'node_label.qml'))

        # EDGES

        # edge
        uri.setDataSource(toponame, 'edge_data', 'geom', '', 'edge_id')
        uri.setSrid(toposrid)
        uri.setWkbType(QgsWkbTypes.LineString)
        uri.removeParam('bbox')
        layerEdge = QgsVectorLayer(uri.uri(False), u'%s.edge' % toponame, provider)
        edge_extent = layerEdge.extent()

        # directed edge
        uri.setDataSource(toponame, 'edge_data', 'geom', '', 'edge_id')
        uri.setSrid(toposrid)
        uri.setWkbType(QgsWkbTypes.LineString)
        uri.removeParam('bbox')
        layerDirectedEdge = QgsVectorLayer(uri.uri(False), u'%s.directed_edge' % toponame, provider)
        layerDirectedEdge.setExtent(edge_extent)
        layerDirectedEdge.loadNamedStyle(os.path.join(template_dir, 'edge.qml'))

        # edge labels
        uri.setDataSource(toponame, 'edge_data', 'geom', '', 'edge_id')
        uri.setSrid(toposrid)
        uri.setWkbType(QgsWkbTypes.LineString)
        uri.removeParam('bbox')
        layerEdgeLabel = QgsVectorLayer(uri.uri(False), u'%s.edge_id' % toponame, provider)
        layerEdgeLabel.setExtent(edge_extent)
        layerEdgeLabel.loadNamedStyle(os.path.join(template_dir, 'edge_label.qml'))

        # face_left
        uri.setDataSource(toponame, 'edge_data', 'geom', '', 'edge_id')
        uri.setSrid(toposrid)
        uri.setWkbType(QgsWkbTypes.LineString)
        uri.removeParam('bbox')
        layerFaceLeft = QgsVectorLayer(uri.uri(False), u'%s.face_left' % toponame, provider)
        layerFaceLeft.setExtent(edge_extent)
        layerFaceLeft.loadNamedStyle(os.path.join(template_dir, 'face_left.qml'))

        # face_right
        uri.setDataSource(toponame, 'edge_data', 'geom', '', 'edge_id')
        uri.setSrid(toposrid)
        uri.setWkbType(QgsWkbTypes.LineString)
        uri.removeParam('bbox')
        layerFaceRight = QgsVectorLayer(uri.uri(False), u'%s.face_right' % toponame, provider)
        layerFaceRight.setExtent(edge_extent)
        layerFaceRight.loadNamedStyle(os.path.join(template_dir, 'face_right.qml'))

        # next_left
        uri.setDataSource(toponame, 'edge_data', 'geom', '', 'edge_id')
        uri.setSrid(toposrid)
        uri.setWkbType(QgsWkbTypes.LineString)
        uri.removeParam('bbox')
        layerNextLeft = QgsVectorLayer(uri.uri(False), u'%s.next_left' % toponame, provider)
        layerNextLeft.setExtent(edge_extent)
        layerNextLeft.loadNamedStyle(os.path.join(template_dir, 'next_left.qml'))

        # next_right
        uri.setDataSource(toponame, 'edge_data', 'geom', '', 'edge_id')
        uri.setSrid(toposrid)
        uri.setWkbType(QgsWkbTypes.LineString)
        uri.removeParam('bbox')
        layerNextRight = QgsVectorLayer(uri.uri(False), u'%s.next_right' % toponame, provider)
        layerNextRight.setExtent(edge_extent)
        layerNextRight.loadNamedStyle(os.path.join(template_dir, 'next_right.qml'))

        # Add layers to the layer tree

        faceLayers = [layerFaceMbr, layerFaceGeom, layerFaceSeed]
        nodeLayers = [layerNode, layerNodeLabel]
        edgeLayers = [layerEdge, layerDirectedEdge, layerEdgeLabel, layerFaceLeft, layerFaceRight, layerNextLeft, layerNextRight]

        QgsProject.instance().addMapLayers(faceLayers, False)
        QgsProject.instance().addMapLayers(nodeLayers, False)
        QgsProject.instance().addMapLayers(edgeLayers, False)

        # Organize layers in groups

        groupFaces = QgsLayerTreeGroup(u'Faces')
        for layer in faceLayers:
            nodeLayer = groupFaces.addLayer(layer)
            nodeLayer.setItemVisibilityChecked(False)
            nodeLayer.setExpanded(False)

        groupNodes = QgsLayerTreeGroup(u'Nodes')
        for layer in nodeLayers:
            nodeLayer = groupNodes.addLayer(layer)
            nodeLayer.setItemVisibilityChecked(False)
            nodeLayer.setExpanded(False)

        groupEdges = QgsLayerTreeGroup(u'Edges')
        for layer in edgeLayers:
            nodeLayer = groupEdges.addLayer(layer)
            nodeLayer.setItemVisibilityChecked(False)
            nodeLayer.setExpanded(False)

        supergroup = QgsLayerTreeGroup(u'Topology "%s"' % toponame)
        supergroup.insertChildNodes(-1, [groupFaces, groupNodes, groupEdges])

        layerTree = QgsProject.instance().layerTreeRoot()

        layerTree.addChildNode(supergroup)

        # Set layers rendering order

        order = layerTree.layerOrder()

        order.insert(0, order.pop(order.index(layerFaceMbr)))
        order.insert(0, order.pop(order.index(layerFaceGeom)))
        order.insert(0, order.pop(order.index(layerEdge)))
        order.insert(0, order.pop(order.index(layerDirectedEdge)))

        order.insert(0, order.pop(order.index(layerNode)))
        order.insert(0, order.pop(order.index(layerFaceSeed)))

        order.insert(0, order.pop(order.index(layerNodeLabel)))
        order.insert(0, order.pop(order.index(layerEdgeLabel)))

        order.insert(0, order.pop(order.index(layerNextLeft)))
        order.insert(0, order.pop(order.index(layerNextRight)))
        order.insert(0, order.pop(order.index(layerFaceLeft)))
        order.insert(0, order.pop(order.index(layerFaceRight)))

        layerTree.setHasCustomLayerOrder(True)
        layerTree.setCustomLayerOrder(order)

    finally:

        # Set canvas extent to topology extent, if not yet initialized
        canvas = iface.mapCanvas()
        if (canvas.fullExtent().isNull()):
            ext = node_extent
            ext.combineExtentWith(edge_extent)
            # Grow by 1/20 of largest side
            ext = ext.buffered(max(ext.width(), ext.height()) / 20)
            canvas.setExtent(ext)

        # restore canvas render flag
        if not wasFrozen:
            iface.mapCanvas().freeze(False)

    return True
