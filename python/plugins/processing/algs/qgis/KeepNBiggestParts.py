# -*- coding: utf-8 -*-

"""
***************************************************************************
    KeepNBiggestParts.py
    ---------------------
    Date                 : July 2014
    Copyright            : (C) 2014 by Victor Olaya
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
__date__ = 'July 2014'
__copyright__ = '(C) 2014, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from operator import itemgetter


from qgis.core import (QgsGeometry,
                       QgsFeatureSink,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterNumber,
                       )

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class KeepNBiggestParts(QgisAlgorithm):

    POLYGONS = 'POLYGONS'
    PARTS = 'PARTS'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector geometry')

    def groupId(self):
        return 'vectorgeometry'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.POLYGONS,
                                                              self.tr('Polygons'), [QgsProcessing.TypeVectorPolygon]))
        self.addParameter(QgsProcessingParameterNumber(self.PARTS,
                                                       self.tr('Parts to keep'),
                                                       QgsProcessingParameterNumber.Integer,
                                                       1, False, 1))
        self.addParameter(
            QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Parts'), QgsProcessing.TypeVectorPolygon))

    def name(self):
        return 'keepnbiggestparts'

    def displayName(self):
        return self.tr('Keep N biggest parts')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.POLYGONS, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.POLYGONS))

        parts = self.parameterAsInt(parameters, self.PARTS, context)

        fields = source.fields()
        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), source.wkbType(), source.sourceCrs())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0
        for current, feat in enumerate(features):
            if feedback.isCanceled():
                break

            geom = feat.geometry()
            if geom.isMultipart():
                out_feature = feat
                geoms = geom.asGeometryCollection()
                geom_area = [(i, geoms[i].area()) for i in range(len(geoms))]
                geom_area.sort(key=itemgetter(1))
                if parts == 1:
                    out_feature.setGeometry(geoms[geom_area[-1][0]])
                elif parts > len(geoms):
                    out_feature.setGeometry(geom)
                else:
                    out_feature.setGeometry(geom)
                    geomres = [geoms[i].asPolygon() for i, a in geom_area[-1 * parts:]]
                    out_feature.setGeometry(QgsGeometry.fromMultiPolygonXY(geomres))
                sink.addFeature(out_feature, QgsFeatureSink.FastInsert)
            else:
                sink.addFeature(feat, QgsFeatureSink.FastInsert)

            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
