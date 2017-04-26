# -*- coding: utf-8 -*-

"""
***************************************************************************
    SplitLinesWithLines.py
    DEPRECATED, replaced by SplitWithLines.py
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

from qgis.core import (QgsProcessingAlgorithm,
                       QgsApplication,
                       QgsFeatureRequest,
                       QgsFeature,
                       QgsGeometry,
                       QgsWkbTypes,
                       QgsMessageLog,
                       QgsProcessingUtils)
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.tools import dataobjects
from processing.tools import vector


class SplitLinesWithLines(GeoAlgorithm):

    INPUT_A = 'INPUT_A'
    INPUT_B = 'INPUT_B'

    OUTPUT = 'OUTPUT'

    def flags(self):
        # this algorithm is deprecated - use SplitWithLines instead
        return QgsProcessingAlgorithm.FlagDeprecated

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def group(self):
        return self.tr('Vector overlay tools')

    def name(self):
        return 'splitlineswithlines'

    def displayName(self):
        return self.tr('Split lines with lines')

    def defineCharacteristics(self):
        self.addParameter(ParameterVector(self.INPUT_A,
                                          self.tr('Input layer'), [dataobjects.TYPE_VECTOR_LINE]))
        self.addParameter(ParameterVector(self.INPUT_B,
                                          self.tr('Split layer'), [dataobjects.TYPE_VECTOR_LINE]))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Split'), datatype=[dataobjects.TYPE_VECTOR_LINE]))

    def processAlgorithm(self, context, feedback):
        layerA = dataobjects.getLayerFromString(self.getParameterValue(self.INPUT_A))
        layerB = dataobjects.getLayerFromString(self.getParameterValue(self.INPUT_B))

        sameLayer = self.getParameterValue(self.INPUT_A) == self.getParameterValue(self.INPUT_B)
        fieldList = layerA.fields()

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fieldList,
                                                                     QgsWkbTypes.LineString, layerA.crs())

        spatialIndex = vector.spatialindex(layerB)

        outFeat = QgsFeature()
        features = QgsProcessingUtils.getFeatures(layerA, context)
        total = 100.0 / QgsProcessingUtils.featureCount(layerA, context)

        for current, inFeatA in enumerate(features):
            inGeom = inFeatA.geometry()
            attrsA = inFeatA.attributes()
            outFeat.setAttributes(attrsA)
            inLines = [inGeom]
            lines = spatialIndex.intersects(inGeom.boundingBox())

            engine = None
            if len(lines) > 0:
                # use prepared geometries for faster intersection tests
                engine = QgsGeometry.createGeometryEngine(inGeom.geometry())
                engine.prepareGeometry()

            if len(lines) > 0:  # hasIntersections
                splittingLines = []

                request = QgsFeatureRequest().setFilterFids(lines).setSubsetOfAttributes([])
                for inFeatB in layerB.getFeatures(request):
                    # check if trying to self-intersect
                    if sameLayer:
                        if inFeatA.id() == inFeatB.id():
                            continue

                    splitGeom = inFeatB.geometry()

                    if engine.intersects(splitGeom.geometry()):
                        splittingLines.append(splitGeom)

                if len(splittingLines) > 0:
                    for splitGeom in splittingLines:
                        splitterPList = vector.extractPoints(splitGeom)
                        outLines = []

                        split_geom_engine = QgsGeometry.createGeometryEngine(splitGeom.geometry())
                        split_geom_engine.prepareGeometry()

                        while len(inLines) > 0:
                            inGeom = inLines.pop()
                            inPoints = vector.extractPoints(inGeom)

                            if split_geom_engine.intersects(inGeom.geometry()):
                                try:
                                    result, newGeometries, topoTestPoints = inGeom.splitGeometry(splitterPList, False)
                                except:
                                    QgsMessageLog.logMessage(self.tr('Geometry exception while splitting'),
                                                             self.tr('Processing'), QgsMessageLog.WARNING)
                                    result = 1

                                # splitGeometry: If there are several intersections
                                # between geometry and splitLine, only the first one is considered.
                                if result == 0:  # split occurred

                                    if inPoints == vector.extractPoints(inGeom):
                                        # bug in splitGeometry: sometimes it returns 0 but
                                        # the geometry is unchanged
                                        outLines.append(inGeom)
                                    else:
                                        inLines.append(inGeom)

                                        for aNewGeom in newGeometries:
                                            inLines.append(aNewGeom)
                                else:
                                    outLines.append(inGeom)
                            else:
                                outLines.append(inGeom)

                        inLines = outLines

            for aLine in inLines:
                if len(aLine.asPolyline()) > 2 or \
                        (len(aLine.asPolyline()) == 2 and
                         aLine.asPolyline()[0] != aLine.asPolyline()[1]):
                    # sometimes splitting results in lines of zero length
                    outFeat.setGeometry(aLine)
                    writer.addFeature(outFeat)

            feedback.setProgress(int(current * total))

        del writer
