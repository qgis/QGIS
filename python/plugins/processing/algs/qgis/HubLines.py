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

__author__ = 'Michael Minn'
__date__ = 'May 2010'
__copyright__ = '(C) 2010, Michael Minn'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import QGis, QgsFeature, QgsGeometry, QgsPoint
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputVector

from processing.tools import dataobjects, vector

class HubLines(GeoAlgorithm):
    HUBS = 'HUBS'
    HUB_FIELD = 'HUB_FIELD'
    SPOKES = 'SPOKES'
    SPOKE_FIELD = 'SPOKE_FIELD'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name = 'Hub lines'
        self.group = 'Vector analysis tools'

        self.addParameter(ParameterVector(self.HUBS,
            self.tr('Hub point layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterTableField(self.HUB_FIELD,
            self.tr('Hub ID field'), self.HUBS))
        self.addParameter(ParameterVector(self.SPOKES,
            self.tr('Spoke point layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterTableField(self.SPOKE_FIELD,
            self.tr('Spoke ID field'), self.SPOKES))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Hub lines')))

    def processAlgorithm(self, progress):
        layerHub = dataobjects.getObjectFromUri(
            self.getParameterValue(self.HUBS))
        layerSpoke = dataobjects.getObjectFromUri(
            self.getParameterValue(self.SPOKES))

        fieldHub = self.getParameterValue(self.HUB_FIELD)
        fieldSpoke = self.getParameterValue(self.SPOKE_FIELD)

        if layerHub.source() == layerSpoke.source():
            raise GeoAlgorithmExecutionException(
                self.tr('Same layer given for both hubs and spokes'))

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            layerSpoke.pendingFields(), QGis.WKBLineString, layerSpoke.crs())

        spokes = vector.features(layerSpoke)
        hubs = vector.features(layerHub)

        count = len(spokes)
        total = 100.0 / float(count)

        for count, spokepoint in enumerate(spokes):
            p = spokepoint.geometry().boundingBox().center()
            spokeX = p.x()
            spokeY = p.y()
            spokeId = unicode(spokepoint[fieldSpoke])

            for hubpoint in hubs:
                hubId = unicode(hubpoint[fieldHub])
                if hubId == spokeId:
                    p = hubpoint.geometry().boundingBox().center()
                    hubX = p.x()
                    hubY = p.y()

                    f = QgsFeature()
                    f.setAttributes(spokepoint.attributes())
                    f.setGeometry(QgsGeometry.fromPolyline(
                        [QgsPoint(spokeX, spokeY), QgsPoint(hubX, hubY)]))
                    writer.addFeature(f)

                    break

            progress.setPercentage(int(count * total))

        del writer
