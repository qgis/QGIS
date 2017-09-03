# -*- coding: utf-8 -*-

"""
***************************************************************************
    NearestNeighbourAnalysis.py
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
from builtins import next
from builtins import str

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import math
import codecs

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsFeatureRequest,
                       QgsDistanceArea,
                       QgsProject,
                       QgsProcessing,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFileDestination,
                       QgsProcessingOutputHtml,
                       QgsProcessingOutputNumber,
                       QgsSpatialIndex)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class NearestNeighbourAnalysis(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT_HTML_FILE = 'OUTPUT_HTML_FILE'
    OBSERVED_MD = 'OBSERVED_MD'
    EXPECTED_MD = 'EXPECTED_MD'
    NN_INDEX = 'NN_INDEX'
    POINT_COUNT = 'POINT_COUNT'
    Z_SCORE = 'Z_SCORE'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'neighbour.png'))

    def group(self):
        return self.tr('Vector analysis')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'), [QgsProcessing.TypeVectorPoint]))

        self.addParameter(QgsProcessingParameterFileDestination(self.OUTPUT_HTML_FILE, self.tr('Nearest neighbour'), self.tr('HTML files (*.html)'), None, True))
        self.addOutput(QgsProcessingOutputHtml(self.OUTPUT_HTML_FILE, self.tr('Nearest neighbour')))

        self.addOutput(QgsProcessingOutputNumber(self.OBSERVED_MD,
                                                 self.tr('Observed mean distance')))
        self.addOutput(QgsProcessingOutputNumber(self.EXPECTED_MD,
                                                 self.tr('Expected mean distance')))
        self.addOutput(QgsProcessingOutputNumber(self.NN_INDEX,
                                                 self.tr('Nearest neighbour index')))
        self.addOutput(QgsProcessingOutputNumber(self.POINT_COUNT,
                                                 self.tr('Number of points')))
        self.addOutput(QgsProcessingOutputNumber(self.Z_SCORE, self.tr('Z-Score')))

    def name(self):
        return 'nearestneighbouranalysis'

    def displayName(self):
        return self.tr('Nearest neighbour analysis')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        output_file = self.parameterAsFileOutput(parameters, self.OUTPUT_HTML_FILE, context)

        spatialIndex = QgsSpatialIndex(source, feedback)

        distance = QgsDistanceArea()
        distance.setSourceCrs(source.sourceCrs())
        distance.setEllipsoid(context.project().ellipsoid())

        sumDist = 0.00
        A = source.sourceExtent()
        A = float(A.width() * A.height())

        features = source.getFeatures()
        count = source.featureCount()
        total = 100.0 / count if count else 1
        for current, feat in enumerate(features):
            if feedback.isCanceled():
                break

            neighbourID = spatialIndex.nearestNeighbor(
                feat.geometry().asPoint(), 2)[1]
            request = QgsFeatureRequest().setFilterFid(neighbourID).setSubsetOfAttributes([])
            neighbour = next(source.getFeatures(request))
            sumDist += distance.measureLine(neighbour.geometry().asPoint(),
                                            feat.geometry().asPoint())

            feedback.setProgress(int(current * total))

        do = float(sumDist) / count
        de = float(0.5 / math.sqrt(count / A))
        d = float(do / de)
        SE = float(0.26136 / math.sqrt(count ** 2 / A))
        zscore = float((do - de) / SE)

        results = {}
        results[self.OBSERVED_MD] = do
        results[self.EXPECTED_MD] = de
        results[self.NN_INDEX] = d
        results[self.POINT_COUNT] = count
        results[self.Z_SCORE] = zscore

        if output_file:
            data = []
            data.append('Observed mean distance: ' + str(do))
            data.append('Expected mean distance: ' + str(de))
            data.append('Nearest neighbour index: ' + str(d))
            data.append('Number of points: ' + str(count))
            data.append('Z-Score: ' + str(zscore))
            self.createHTML(output_file, data)
            results[self.OUTPUT_HTML_FILE] = output_file

        return results

    def createHTML(self, outputFile, algData):
        with codecs.open(outputFile, 'w', encoding='utf-8') as f:
            f.write('<html><head>')
            f.write('<meta http-equiv="Content-Type" content="text/html; \
                    charset=utf-8" /></head><body>')
            for s in algData:
                f.write('<p>' + str(s) + '</p>')
            f.write('</body></html>')
