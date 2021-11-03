# -*- coding: utf-8 -*-

"""
***************************************************************************
    DefineProjection.py
    ---------------------
    Date                 : January 2016
    Copyright            : (C) 2016 by Alexander Bruy
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
__date__ = 'January 2016'
__copyright__ = '(C) 2016, Alexander Bruy'

import os
import re

from qgis.core import (QgsProcessing,
                       QgsProcessingAlgorithm,
                       QgsProcessingParameterVectorLayer,
                       QgsProcessingParameterCrs,
                       QgsProcessingOutputVectorLayer,
                       QgsCoordinateReferenceSystem,
                       QgsProjUtils)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class DefineProjection(QgisAlgorithm):
    INPUT = 'INPUT'
    CRS = 'CRS'

    def group(self):
        return self.tr('Vector general')

    def groupId(self):
        return 'vectorgeneral'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterVectorLayer(self.INPUT,
                                                            self.tr('Input Shapefile'), types=[QgsProcessing.TypeVectorAnyGeometry]))
        self.addParameter(QgsProcessingParameterCrs(self.CRS, 'CRS'))
        self.addOutput(QgsProcessingOutputVectorLayer(self.INPUT,
                                                      self.tr('Layer with projection')))

    def name(self):
        return 'definecurrentprojection'

    def displayName(self):
        return self.tr('Define Shapefile projection')

    def tags(self):
        return self.tr('layer,shp,prj,qpj,change,alter').split(',')

    def shortDescription(self):
        return self.tr('Changes a Shapefile\'s projection to a new CRS without reprojecting features')

    def flags(self):
        return super().flags() | QgsProcessingAlgorithm.FlagNoThreading

    def processAlgorithm(self, parameters, context, feedback):
        layer = self.parameterAsVectorLayer(parameters, self.INPUT, context)
        crs = self.parameterAsCrs(parameters, self.CRS, context)

        provider = layer.dataProvider()
        ds = provider.dataSourceUri()
        p = re.compile(r'\|.*')
        dsPath = p.sub('', ds)

        if dsPath.lower().endswith('.shp'):
            dsPath = dsPath[:-4]

            wkt = crs.toWkt(QgsCoordinateReferenceSystem.WKT1_ESRI)
            with open(dsPath + '.prj', 'wt', encoding='utf-8') as f:
                f.write(wkt)

            qpjFile = dsPath + '.qpj'
            if os.path.exists(qpjFile):
                os.remove(qpjFile)
        else:
            feedback.pushConsoleInfo(self.tr("Data source isn't a Shapefile, skipping .prj/.qpj creation"))

        layer.setCrs(crs)
        layer.triggerRepaint()

        return {self.INPUT: layer}
