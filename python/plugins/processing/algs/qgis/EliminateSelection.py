# -*- coding: utf-8 -*-

"""
***************************************************************************
    EliminateSelection.py
    ---------------------
    Date                 : January 2017
    Copyright         : (C) 2017 by Bernhard Ströbl
    Email                : bernhard.stroebl@jena.de
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
from builtins import range

__author__ = 'Bernhard Ströbl'
__date__ = 'January 2017'
__copyright__ = '(C) 2017, Bernhard Ströbl'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import QgsFeatureRequest, QgsFeature, QgsGeometry

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.ProcessingLog import ProcessingLog
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class EliminateSelection(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    MODE = 'MODE'

    MODE_LARGEST_AREA = 0
    MODE_SMALLEST_AREA = 1
    MODE_BOUNDARY = 2

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'eliminate.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Eliminate selected polygons')
        self.group, self.i18n_group = self.trAlgorithm('Vector geometry tools')

        self.modes = [self.tr('Largest area'),
                      self.tr('Smallest Area'),
                      self.tr('Largest common boundary')]

        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input layer'), [dataobjects.TYPE_VECTOR_POLYGON]))
        self.addParameter(ParameterSelection(self.MODE,
                                             self.tr('Merge selection with the neighbouring polygon with the'),
                                             self.modes))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Eliminated'), datatype=[dataobjects.TYPE_VECTOR_POLYGON]))

    def processAlgorithm(self, feedback):
        inLayer = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT))
        boundary = self.getParameterValue(self.MODE) == self.MODE_BOUNDARY
        smallestArea = self.getParameterValue(self.MODE) == self.MODE_SMALLEST_AREA

        if inLayer.selectedFeatureCount() == 0:
            ProcessingLog.addToLog(ProcessingLog.LOG_WARNING,
                                   self.tr('{0}: (No selection in input layer "{1}")').format(self.commandLineName(), self.getParameterValue(self.INPUT)))

        featToEliminate = []
        selFeatIds = inLayer.selectedFeatureIds()
        output = self.getOutputFromName(self.OUTPUT)
        writer = output.getVectorWriter(inLayer.fields(),
                                        inLayer.wkbType(), inLayer.crs())

        for aFeat in inLayer.getFeatures():
            if aFeat.id() in selFeatIds:
                # Keep references to the features to eliminate
                featToEliminate.append(aFeat)
            else:
                # write the others to output
                writer.addFeature(aFeat)

        # Delete all features to eliminate in processLayer
        processLayer = output.layer
        processLayer.startEditing()

        # ANALYZE
        if len(featToEliminate) > 0:  # Prevent zero division
            start = 20.00
            add = 80.00 / len(featToEliminate)
        else:
            start = 100

        feedback.setProgress(start)
        madeProgress = True

        # We go through the list and see if we find any polygons we can
        # merge the selected with. If we have no success with some we
        # merge and then restart the whole story.
        while madeProgress:  # Check if we made any progress
            madeProgress = False
            featNotEliminated = []

            # Iterate over the polygons to eliminate
            for i in range(len(featToEliminate)):
                feat = featToEliminate.pop()
                geom2Eliminate = feat.geometry()
                bbox = geom2Eliminate.boundingBox()
                fit = processLayer.getFeatures(
                    QgsFeatureRequest().setFilterRect(bbox).setSubsetOfAttributes([]))
                mergeWithFid = None
                mergeWithGeom = None
                max = 0
                min = -1
                selFeat = QgsFeature()

                # use prepared geometries for faster intersection tests
                engine = QgsGeometry.createGeometryEngine(geom2Eliminate.geometry())
                engine.prepareGeometry()

                while fit.nextFeature(selFeat):
                    selGeom = selFeat.geometry()

                    if engine.intersects(selGeom.geometry()):
                        # We have a candidate
                        iGeom = geom2Eliminate.intersection(selGeom)

                        if not iGeom:
                            continue

                        if boundary:
                            selValue = iGeom.length()
                        else:
                            # area. We need a common boundary in
                            # order to merge
                            if 0 < iGeom.length():
                                selValue = selGeom.area()
                            else:
                                selValue = -1

                        if -1 != selValue:
                            useThis = True

                            if smallestArea:
                                if -1 == min:
                                    min = selValue
                                else:
                                    if selValue < min:
                                        min = selValue
                                    else:
                                        useThis = False
                            else:
                                if selValue > max:
                                    max = selValue
                                else:
                                    useThis = False

                            if useThis:
                                mergeWithFid = selFeat.id()
                                mergeWithGeom = QgsGeometry(selGeom)
                # End while fit

                if mergeWithFid is not None:
                    # A successful candidate
                    newGeom = mergeWithGeom.combine(geom2Eliminate)

                    if processLayer.changeGeometry(mergeWithFid, newGeom):
                        madeProgress = True
                    else:
                        raise GeoAlgorithmExecutionException(
                            self.tr('Could not replace geometry of feature with id {0}').format(mergeWithFid))

                    start = start + add
                    feedback.setProgress(start)
                else:
                    featNotEliminated.append(feat)

            # End for featToEliminate

            featToEliminate = featNotEliminated

        # End while
        if not processLayer.commitChanges():
            raise GeoAlgorithmExecutionException(self.tr('Could not commit changes'))

        for feature in featNotEliminated:
            writer.addFeature(feature)
