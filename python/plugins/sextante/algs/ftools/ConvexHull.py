# -*- coding: utf-8 -*-

"""
***************************************************************************
    ConvexHull.py
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
from PyQt4.QtGui import *

from qgis.core import *

from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.QGisLayers import QGisLayers
from sextante.core.SextanteLog import SextanteLog

from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterTableField import ParameterTableField
from sextante.parameters.ParameterSelection import ParameterSelection

from sextante.outputs.OutputVector import OutputVector

from sextante.algs.ftools import FToolsUtils as utils

class ConvexHull(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    FIELD = "FIELD"
    METHOD = "METHOD"
    METHODS = ["Create single minimum convex hull",
               "Create convex hulls based on field"
              ]

    #===========================================================================
    # def getIcon(self):
    #    return QtGui.QIcon(os.path.dirname(__file__) + "/icons/convex_hull.png")
    #===========================================================================

    def defineCharacteristics(self):
        self.name = "Convex hull"
        self.group = "Vector geometry tools"
        self.addParameter(ParameterVector(ConvexHull.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterTableField(ConvexHull.FIELD, "Field (optional, only used if creating convex hulls by classes)", ConvexHull.INPUT, optional = True))
        self.addParameter(ParameterSelection(ConvexHull.METHOD, "Method", ConvexHull.METHODS))
        self.addOutput(OutputVector(ConvexHull.OUTPUT, "Convex hull"))

    def processAlgorithm(self, progress):
        useField = (self.getParameterValue(ConvexHull.METHOD) == 1)
        fieldName = self.getParameterValue(ConvexHull.FIELD)
        layer = QGisLayers.getObjectFromUri(self.getParameterValue(ConvexHull.INPUT))

        GEOS_EXCEPT = True
        FEATURE_EXCEPT = True

        
        f = QgsField("value")
        f.setType(QVariant.String)
        f.setLength(255)
        if useField:
            index = layer.fieldNameIndex(fieldName)
            fType = layer.pendingFields()[index].type()
            if fType == QVariant.Int:
                f.setType(QVariant.Int)
                f.setLength(20)
            elif fType == QVariant.Double:
                f.setType(QVariant.Double)
                f.setLength(20)
                f.setPrecision(6)
            else:
                f.setType(QVariant.String)
                f.setLength(255)

        fields = [QgsField("id", QVariant.Int, "", 20),
                  f,
                  QgsField("area", QVariant.Double, "", 20, 6),
                  QgsField("perim", QVariant.Double, "", 20, 6)
                 ]

        writer = self.getOutputFromName(ConvexHull.OUTPUT).getVectorWriter(fields, QGis.WKBPolygon, layer.dataProvider().crs())

        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        outGeom = QgsGeometry()

        current = 0

        fid = 0
        val = ""
        if useField:
            unique = layer.uniqueValues(index)
            total = 100.0 / float(layer.featureCount() * len (unique))

            for i in unique:
                hull = []
                first = True
                features = QGisLayers.features(layer)
                for f in features:
                    idVar = f[fieldName]
                    if idVar.toString().trimmed() == i.toString().trimmed():
                        if first:
                            val = idVar
                            first = False
                        inGeom = QgsGeometry(f.geometry())
                        points = utils.extractPoints(inGeom)
                        hull.extend(points)
                    current += 1
                    progress.setPercentage(int(current * total))

                if len(hull) >= 3:
                    tmpGeom = QgsGeometry(outGeom.fromMultiPoint(hull))
                    try:
                        outGeom = tmpGeom.convexHull()
                        (area, perim) = utils.simpleMeasure(outGeom)
                        outFeat.setGeometry(outGeom)
                        outFeat.setAttributes([QVariant(fid),
                                               QVariant(val),
                                               QVariant(area),
                                               QVariant(perim)
                                             ])
                        writer.addFeature(outFeat)
                    except:
                        GEOS_EXCEPT = False
                        continue
                fid += 1
        else:
          hull = []
          total = 100.0 / float(layer.featureCount())
          features = QGisLayers.features(layer)
          for f in features:
              inGeom = QgsGeometry(f.geometry())
              points = utils.extractPoints(inGeom)
              hull.extend(points)
              current += 1
              progress.setPercentage(int(current * total))

          tmpGeom = QgsGeometry(outGeom.fromMultiPoint(hull))
          try:
              outGeom = tmpGeom.convexHull()
              (area, perim) = utils.simpleMeasure(outGeom)
              outFeat.setGeometry(outGeom)
              outFeat.setAttributes([QVariant(0),
                                     QVariant("all"),
                                     QVariant(area),
                                     QVariant(perim)
                                   ])
              writer.addFeature(outFeat)
          except:
              GEOS_EXCEPT = False

        del writer

        if not GEOS_EXCEPT:
            SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Geometry exception while computing convex hull")
        if not FEATURE_EXCEPT:
            SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Feature exception while computing convex hull")
