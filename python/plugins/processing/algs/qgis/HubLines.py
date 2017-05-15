# -*- coding: utf-8 -*-

"""
***************************************************************************
    Gridify.py
    ---------------------
    Date                 : May 2010
    Copyright            : (C) 2010 by Michael Minn
    Email                : pyqgis at michaelminn dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""
from builtins import str

__author__ = 'Michael Minn'
__date__ = 'May 2010'
__copyright__ = '(C) 2010, Michael Minn'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsFeature,
                       QgsGeometry,
                       QgsPointXY,
                       QgsWkbTypes,
                       QgsApplication,
                       QgsProcessingUtils)
from processing.algs.qgis import QgisAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputVector

from processing.tools import dataobjects


class HubLines(QgisAlgorithm):
    HUBS = 'HUBS'
    HUB_FIELD = 'HUB_FIELD'
    SPOKES = 'SPOKES'
    SPOKE_FIELD = 'SPOKE_FIELD'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def group(self):
        return self.tr('Vector analysis tools')

    def __init__(self):
        super().__init__()
        self.addParameter(ParameterVector(self.HUBS,
                                          self.tr('Hub layer')))
        self.addParameter(ParameterTableField(self.HUB_FIELD,
                                              self.tr('Hub ID field'), self.HUBS))
        self.addParameter(ParameterVector(self.SPOKES,
                                          self.tr('Spoke layer')))
        self.addParameter(ParameterTableField(self.SPOKE_FIELD,
                                              self.tr('Spoke ID field'), self.SPOKES))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Hub lines'), datatype=[dataobjects.TYPE_VECTOR_LINE]))

    def name(self):
        return 'hublines'

    def displayName(self):
        return self.tr('Hub lines')

    def processAlgorithm(self, context, feedback):
        layerHub = QgsProcessingUtils.mapLayerFromString(self.getParameterValue(self.HUBS), context)
        layerSpoke = QgsProcessingUtils.mapLayerFromString(self.getParameterValue(self.SPOKES), context)

        fieldHub = self.getParameterValue(self.HUB_FIELD)
        fieldSpoke = self.getParameterValue(self.SPOKE_FIELD)

        if layerHub.source() == layerSpoke.source():
            raise GeoAlgorithmExecutionException(
                self.tr('Same layer given for both hubs and spokes'))

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(layerSpoke.fields(), QgsWkbTypes.LineString,
                                                                     layerSpoke.crs(), context)

        spokes = QgsProcessingUtils.getFeatures(layerSpoke, context)
        hubs = QgsProcessingUtils.getFeatures(layerHub, context)
        total = 100.0 / QgsProcessingUtils.featureCount(layerSpoke, context)

        for current, spokepoint in enumerate(spokes):
            p = spokepoint.geometry().boundingBox().center()
            spokeX = p.x()
            spokeY = p.y()
            spokeId = str(spokepoint[fieldSpoke])

            for hubpoint in hubs:
                hubId = str(hubpoint[fieldHub])
                if hubId == spokeId:
                    p = hubpoint.geometry().boundingBox().center()
                    hubX = p.x()
                    hubY = p.y()

                    f = QgsFeature()
                    f.setAttributes(spokepoint.attributes())
                    f.setGeometry(QgsGeometry.fromPolyline(
                        [QgsPointXY(spokeX, spokeY), QgsPointXY(hubX, hubY)]))
                    writer.addFeature(f)

                    break

            feedback.setProgress(int(current * total))

        del writer
