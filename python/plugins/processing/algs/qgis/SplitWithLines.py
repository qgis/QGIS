# -*- coding: utf-8 -*-

"""
***************************************************************************
    SplitWithLines.py
    ---------------------
    Date                 : November 2014
    Revised              : November 2016
    Copyright            : (C) 2014 by Bernhard Ströbl
    Email                : bernhard dot stroebl at jena dot de
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Bernhard Ströbl'
__date__ = 'November 2014'
__copyright__ = '(C) 2014, Bernhard Ströbl'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import QgsFeatureRequest, QgsFeature, QgsGeometry, QgsSpatialIndex, QgsWkbTypes, QgsPoint
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.core.ProcessingLog import ProcessingLog
from processing.tools import dataobjects
from processing.tools import vector


class SplitWithLines(GeoAlgorithm):

    INPUT_A = 'INPUT_A'
    INPUT_B = 'INPUT_B'

    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Split with lines')
        self.group, self.i18n_group = self.trAlgorithm('Vector overlay tools')
        self.addParameter(ParameterVector(self.INPUT_A,
                                          self.tr('Input layer, single geometries only'), [dataobjects.TYPE_VECTOR_POLYGON,
                                                                                           dataobjects.TYPE_VECTOR_LINE]))
        self.addParameter(ParameterVector(self.INPUT_B,
                                          self.tr('Split layer'), [dataobjects.TYPE_VECTOR_LINE]))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Split')))

    def processAlgorithm(self, progress):
        layerA = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT_A))
        splitLayer = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT_B))

        sameLayer = self.getParameterValue(self.INPUT_A) == self.getParameterValue(self.INPUT_B)
        fieldList = layerA.fields()

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fieldList,
                        QgsWkbTypes.multiType(layerA.wkbType()), layerA.crs())

        spatialIndex = QgsSpatialIndex()
        splitGeoms = {}
        request = QgsFeatureRequest()
        request.setSubsetOfAttributes([])

        for aSplitFeature in vector.features(splitLayer, request):
            splitGeoms[aSplitFeature.id()] = aSplitFeature.geometry()
            spatialIndex.insertFeature(aSplitFeature)
            # honor the case that user has selection on split layer and has setting "use selection"

        outFeat = QgsFeature()
        features = vector.features(layerA)

        if len(features) == 0:
            total = 100
        else:
            total = 100.0 / float(len(features))

        for current, inFeatA in enumerate(features):
            inGeom = inFeatA.geometry()
            attrsA = inFeatA.attributes()
            outFeat.setAttributes(attrsA)

            if inGeom.isMultipart():
                inGeoms = []

                for g in inGeom.asGeometryCollection():
                    inGeoms.append(g)
            else:
                inGeoms = [inGeom]

            lines = spatialIndex.intersects(inGeom.boundingBox())

            if len(lines) > 0:  # has intersection of bounding boxes
                splittingLines = []

                engine = QgsGeometry.createGeometryEngine(inGeom.geometry())
                engine.prepareGeometry()

                for i in lines:
                    try:
                        splitGeom = splitGeoms[i]
                    except:
                        continue

                    # check if trying to self-intersect
                    if sameLayer:
                        if inFeatA.id() == i:
                            continue

                    if engine.intersects(splitGeom.geometry()):
                        splittingLines.append(splitGeom)

                if len(splittingLines) > 0:
                    for splitGeom in splittingLines:
                        splitterPList = None
                        outGeoms = []

                        split_geom_engine = QgsGeometry.createGeometryEngine(splitGeom.geometry())
                        split_geom_engine.prepareGeometry()

                        while len(inGeoms) > 0:
                            inGeom = inGeoms.pop()

                            if inGeom.geometry() == None: # this has been encountered and created a run-time error
                                continue

                            if split_geom_engine.intersects(inGeom.geometry()):
                                inPoints = vector.extractPoints(inGeom)
                                if splitterPList == None:
                                    splitterPList = vector.extractPoints(splitGeom)

                                try:
                                    result, newGeometries, topoTestPoints = inGeom.splitGeometry(splitterPList, False)
                                except:
                                    ProcessingLog.addToLog(ProcessingLog.LOG_WARNING,
                                                           self.tr('Geometry exception while splitting'))
                                    result = 1

                                # splitGeometry: If there are several intersections
                                # between geometry and splitLine, only the first one is considered.
                                if result == 0:  # split occurred
                                    if inPoints == vector.extractPoints(inGeom):
                                        # bug in splitGeometry: sometimes it returns 0 but
                                        # the geometry is unchanged
                                        outGeoms.append(inGeom)
                                    else:
                                        inGeoms.append(inGeom)

                                        for aNewGeom in newGeometries:
                                            inGeoms.append(aNewGeom)
                                else:
                                    outGeoms.append(inGeom)
                            else:
                                outGeoms.append(inGeom)

                        inGeoms = outGeoms

            parts = []

            for aGeom in inGeoms:
                passed = True

                if QgsWkbTypes.geometryType( aGeom.wkbType() )  == QgsWkbTypes.LineGeometry:
                    numPoints = aGeom.geometry().numPoints()

                    if numPoints <= 2:
                        if numPoints == 2:
                            passed = not aGeom.geometry().isClosed() # tests if vertex 0 = vertex 1
                        else:
                            passed = False
                            # sometimes splitting results in lines of zero length

                if passed:
                    parts.append(aGeom)

            if len(parts) > 0:
                outFeat.setGeometry(QgsGeometry.collectGeometry(parts))
                writer.addFeature(outFeat)

            progress.setPercentage(int(current * total))
        del writer
