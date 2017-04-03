# -*- coding: utf-8 -*-

"""
***************************************************************************
    shortestpathpointtopoint.py
    ---------------------
    Date                 : February 2017
    Copyright            : (C) 2017 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'February 2017'
__copyright__ = '(C) 2017, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from osgeo import gdal, gnm, ogr

from qgis.PyQt.QtCore import QVariant

from qgis.core import (QgsCoordinateReferenceSystem,
                       QgsGeometry,
                       QgsFeature,
                       QgsFields,
                       QgsField,
                       QgsWkbTypes)

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import (ParameterFile,
                                        ParameterNumber,
                                        ParameterString)

from processing.core.outputs import OutputVector

from processing.algs.gdal.GdalUtils import GdalUtils


class ShortestPathPointToPoint(GeoAlgorithm):

    NETWORK = 'NETWORK'
    NUMBER_OF_PATHS = 'NUMBER_OF_PATHS'
    START_POINT = 'START_POINT'
    END_POINT = 'END_POINT'
    BLOCKED_POINTS = 'BLOCKED_POINTS'
    SHORTEST_PATHS = 'SHORTEST_PATHS'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Shortest paths (point to point)')
        self.group, self.i18n_group = self.trAlgorithm('Network analysis')

        self.addParameter(ParameterFile(
            self.NETWORK,
            self.tr('Directory with network'),
            isFolder=True,
            optional=False))
        self.addParameter(ParameterNumber(
            self.NUMBER_OF_PATHS,
            self.tr('Number of paths to calculate'),
            1, 99, 1))
        self.addParameter(ParameterNumber(
            self.START_POINT,
            self.tr('GFID of the start node (value of the "gnm_fid" field)'),
            default=0))
        self.addParameter(ParameterNumber(
            self.END_POINT,
            self.tr('GFID of the end node (value of the "gnm_fid" field)'),
            default=0))
        self.addParameter(ParameterString(
            self.BLOCKED_POINTS,
            self.tr('Comma-separated GFIDs of the blocked nodes'),
            '',
            optional=True))

        self.addOutput(OutputVector(
            self.SHORTEST_PATHS,
            self.tr('Shortest path(s)')))

    def processAlgorithm(self, feedback):
        networkPath = self.getParameterValue(self.NETWORK)
        pathsNumber = self.getParameterValue(self.NUMBER_OF_PATHS)
        gfidStart = self.getParameterValue(self.START_POINT)
        gfidEnd = self.getParameterValue(self.END_POINT)
        gfidsBlocked = self.getParameterValue(self.BLOCKED_POINTS)
        outputPath = self.getOutputValue(self.SHORTEST_PATHS)

        if gfidStart == gfidEnd:
            raise GeoAlgorithmExecutionException(
                self.tr('Start and end points should be different.'))

        if gfidsBlocked is not None:
            gfidsBlocked = [int(gfid.strip()) for gfid in gfidsBlocked.split(',')]

            if gfidStart in gfidsBlocked:
                raise GeoAlgorithmExecutionException(
                    self.tr('Start point can not be blocked.'))

            if gfidEnd in gfidsBlocked:
                raise GeoAlgorithmExecutionException(
                    self.tr('End point can not be blocked.'))

        # load network
        ds = gdal.OpenEx(networkPath)
        network = gnm.CastToGenericNetwork(ds)
        if network is None:
            raise GeoAlgorithmExecutionException(
                self.tr('Can not open generic network dataset.'))

        # block nodes if necessary
        if gfidsBlocked is not None:
            for gfid in gfidsBlocked:
                network.ChangeBlockState(gfid, True)

        # calculate shortest paths
        if pathsNumber == 1:
            layer = network.GetPath(gfidStart, gfidEnd, gnm.GATDijkstraShortestPath)
        else:
            options = ['num_paths={}'.format(pathsNumber)]
            layer = network.GetPath(gfidStart, gfidEnd, gnm.GATKShortestPath, options)

        # unblock previously blocked nodes
        if gfidsBlocked is not None:
            for gfid in gfidsBlocked:
                network.ChangeBlockState(gfid, False)

        if layer is None:
            raise GeoAlgorithmExecutionException(
                self.tr('Error occurred during shortest path calculation.'))

        if layer.GetFeatureCount() == 0:
            feedback.pushInfo(
                self.tr('There is no path from the start point to the end point.'))

        # copy features to the output layer
        networkCrs = network.GetProjectionRef()
        crs = QgsCoordinateReferenceSystem(networkCrs)

        fields = QgsFields()
        fields.append(QgsField('gfid', QVariant.Int, '', 10, 0))
        fields.append(QgsField('ogrlayer', QVariant.String, '', 254))
        fields.append(QgsField('path_num', QVariant.Int, '', 10, 0))
        fields.append(QgsField('type', QVariant.String, '', 254))

        writer = self.getOutputFromName(
            self.SHORTEST_PATHS).getVectorWriter(
                fields,
                QgsWkbTypes.LineString,
                crs)

        feat = QgsFeature()
        feat.setFields(fields)
        geom = QgsGeometry()

        layer.ResetReading()
        f = layer.GetNextFeature()
        while f is not None:
            g = f.GetGeometryRef()
            if g.GetGeometryType() == ogr.wkbLineString:
                wkb = g.ExportToWkb()
                geom.fromWkb(wkb)
                feat.setGeometry(geom)
                feat['gfid'] = f.GetFieldAsInteger64(0)
                feat['ogrlayer'] = f.GetFieldAsString(1)
                feat['path_num'] = f.GetFieldAsInteger64(2)
                feat['type'] = f.GetFieldAsString(3)
                writer.addFeature(feat)
            f = layer.GetNextFeature()

        del writer

        network.ReleaseResultSet(layer)
        network = None
        ds = None
