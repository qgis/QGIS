# -*- coding: utf-8 -*-

"""
***************************************************************************
    ExportGeometryInfo.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4.QtCore import *

from qgis.core import *

from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.QGisLayers import QGisLayers

from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterSelection import ParameterSelection

from sextante.outputs.OutputVector import OutputVector

from sextante.algs.ftools import FToolsUtils as utils

class ExportGeometryInfo(GeoAlgorithm):

    INPUT = "INPUT"
    METHOD = "CALC_METHOD"
    OUTPUT = "OUTPUT"

    CALC_METHODS = ["Layer CRS",
                    "Project CRS",
                    "Ellipsoidal"
                   ]

    #===========================================================================
    # def getIcon(self):
    #    return QtGui.QIcon(os.path.dirname(__file__) + "/icons/export_geometry.png")
    #===========================================================================

    def defineCharacteristics(self):
        self.name = "Export/Add geometry columns"
        self.group = "Vector table tools"

        self.addParameter(ParameterVector(self.INPUT, "Input layer", [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterSelection(self.METHOD, "Calculate using", self.CALC_METHODS, 0))

        self.addOutput(OutputVector(self.OUTPUT, "Output layer"))

    def processAlgorithm(self, progress):
        layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.INPUT))
        method = self.getParameterValue(self.METHOD)

        geometryType = layer.geometryType()

        idx1 = -1
        idx2 = -1
        fields = layer.pendingFields()

        if geometryType == QGis.Polygon:
            idx1, fields = utils.findOrCreateField(layer, fields, "area", 21, 6)
            idx2, fields = utils.findOrCreateField(layer, fields, "perimeter", 21, 6)
        elif geometryType == QGis.Line:
            idx1, fields = utils.findOrCreateField(layer, fields, "length", 21, 6)
            idx2 = idx1
        else:
            idx1, fields = utils.findOrCreateField(layer, fields, "xcoord", 21, 6)
            idx2, fields = utils.findOrCreateField(layer, fields, "ycoord", 21, 6)

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fields.toList(),
                     layer.dataProvider().geometryType(), layer.crs())

        print idx1, idx2

        ellips = None
        crs = None
        coordTransform = None

        # calculate with:
        # 0 - layer CRS
        # 1 - project CRS
        # 2 - ellipsoidal
        if method == 2:
            ellips = QgsProject.instance().readEntry("Measure", "/Ellipsoid", GEO_NONE)[0]
            crs = layer.crs().srsid()
        elif method == 1:
            mapCRS = QGisLayers.iface.mapCanvas().mapRenderer().destinationCrs()
            layCRS = layer.crs()
            coordTransform = QgsCoordinateTransform(layCRS, mapCRS)

        outFeat = QgsFeature()
        inGeom = QgsGeometry()

        outFeat.initAttributes(len(fields))
        outFeat.setFields(fields)

        current = 0
        features = QGisLayers.features(layer)
        total = 100.0 / float(len(features))
        for f in features:
            inGeom = f.geometry()

            if method == 1:
                inGeom.transform(coordTransform)

            (attr1, attr2) = utils.simpleMeasure(inGeom, method, ellips, crs)

            outFeat.setGeometry(inGeom)
            attrs = f.attributes()
            attrs.insert(idx1, attr1)
            if attr2 is not None:
                attrs.insert(idx2, attr2)
            outFeat.setAttributes(attrs)
            writer.addFeature(outFeat)

            current += 1
            progress.setPercentage(int(current * total))

        del writer
