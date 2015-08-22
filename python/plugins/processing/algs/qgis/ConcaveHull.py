# -*- coding: utf-8 -*-

"""
***************************************************************************
    ConcaveHull.py
    ---------------------
    Date                 : May 2014
    Copyright            : (C) 2012 by Piotr Pociask
    Email                : piotr dot pociask at gis-support dot pl
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Piotr Pociask'
__date__ = 'May 2014'
__copyright__ = '(C) 2014, Piotr Pociask'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import QGis, QgsFeatureRequest, QgsFeature, QgsGeometry
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputVector
from processing.tools import dataobjects
import processing
from math import sqrt


class ConcaveHull(GeoAlgorithm):

    INPUT = 'INPUT'
    ALPHA = 'ALPHA'
    HOLES = 'HOLES'
    NO_MULTIGEOMETRY = 'NO_MULTIGEOMETRY'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Concave hull')
        self.group, self.i18n_group = self.trAlgorithm('Vector geometry tools')
        self.addParameter(ParameterVector(ConcaveHull.INPUT,
                                          self.tr('Input point layer'), [ParameterVector.VECTOR_TYPE_POINT]))
        self.addParameter(ParameterNumber(self.ALPHA,
                                          self.tr('Threshold (0-1, where 1 is equivalent with Convex Hull)'),
                                          0, 1, 0.3))
        self.addParameter(ParameterBoolean(self.HOLES,
                                           self.tr('Allow holes'), True))
        self.addParameter(ParameterBoolean(self.NO_MULTIGEOMETRY,
                                           self.tr('Split multipart geometry into singleparts geometries'), False))
        self.addOutput(OutputVector(ConcaveHull.OUTPUT, self.tr('Concave hull')))

    def processAlgorithm(self, progress):
        #get parameters
        layer = dataobjects.getObjectFromUri(self.getParameterValue(ConcaveHull.INPUT))
        alpha = self.getParameterValue(self.ALPHA)
        holes = self.getParameterValue(self.HOLES)
        no_multigeom = self.getParameterValue(self.NO_MULTIGEOMETRY)
        #Delaunay triangulation from input point layer
        progress.setText(self.tr('Creating Delaunay triangles...'))
        delone_triangles = processing.runalg("qgis:delaunaytriangulation", layer, None)['OUTPUT']
        delaunay_layer = processing.getObject(delone_triangles)
        #get max edge length from Delaunay triangles
        progress.setText(self.tr('Computing edges max length...'))
        features = delaunay_layer.getFeatures()
        counter = 50. / delaunay_layer.featureCount()
        lengths = []
        edges = {}
        for feat in features:
            line = feat.geometry().asPolygon()[0]
            for i in range(len(line) - 1):
                lengths.append(sqrt(line[i].sqrDist(line[i + 1])))
            edges[feat.id()] = max(lengths[-3:])
            progress.setPercentage(feat.id() * counter)
        max_length = max(lengths)
        #get features with longest edge longer than alpha*max_length
        progress.setText(self.tr('Removing features...'))
        counter = 50. / len(edges)
        i = 0
        ids = []
        for id, max_len in edges.iteritems():
            if max_len > alpha * max_length:
                ids.append(id)
            progress.setPercentage(50 + i * counter)
            i += 1
        #remove features
        delaunay_layer.setSelectedFeatures(ids)
        delaunay_layer.startEditing()
        delaunay_layer.deleteSelectedFeatures()
        delaunay_layer.commitChanges()
        #dissolve all Delaunay triangles
        progress.setText(self.tr('Dissolving Delaunay triangles...'))
        dissolved = processing.runalg("qgis:dissolve", delaunay_layer,
                                      True, '', None)['OUTPUT']
        dissolved_layer = processing.getObject(dissolved)
        #save result
        progress.setText(self.tr('Saving data...'))
        feat = QgsFeature()
        dissolved_layer.getFeatures(QgsFeatureRequest().setFilterFid(0)).nextFeature(feat)
        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            layer.pendingFields().toList(), QGis.WKBPolygon, layer.crs())
        geom = feat.geometry()
        if no_multigeom and geom.isMultipart():
            #only singlepart geometries are allowed
            geom_list = geom.asMultiPolygon()
            for single_geom_list in geom_list:
                single_feature = QgsFeature()
                single_geom = QgsGeometry.fromPolygon(single_geom_list)
                if not holes:
                    #delete holes
                    deleted = True
                    while deleted:
                        deleted = single_geom.deleteRing(1)
                single_feature.setGeometry(single_geom)
                writer.addFeature(single_feature)
        else:
            #multipart geometries are allowed
            if not holes:
                #delete holes
                deleted = True
                while deleted:
                    deleted = geom.deleteRing(1)
            writer.addFeature(feat)
        del writer
