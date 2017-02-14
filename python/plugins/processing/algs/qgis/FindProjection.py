# -*- coding: utf-8 -*-

"""
***************************************************************************
    FindProjection.py
    -----------------
    Date                 : February 2017
    Copyright            : (C) 2017 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nyall Dawson'
__date__ = 'February 2017'
__copyright__ = '(C) 2017, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import codecs

from qgis.core import (QgsGeometry,
                       QgsRectangle,
                       QgsCoordinateReferenceSystem,
                       QgsCoordinateTransform)

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterCrs
from processing.core.parameters import ParameterExtent
from processing.core.outputs import OutputHTML
from processing.tools import dataobjects


pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class FindProjection(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    TARGET_AREA = 'TARGET_AREA'
    TARGET_AREA_CRS = 'TARGET_AREA_CRS'
    OUTPUT_HTML_FILE = 'OUTPUT_HTML_FILE'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Find projection')
        self.group, self.i18n_group = self.trAlgorithm('Vector general tools')
        self.tags = self.tr('crs,srs,coordinate,reference,system,guess,estimate,finder,determine')

        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input layer')))
        extent_parameter = ParameterExtent(self.TARGET_AREA,
                                           self.tr('Target area for layer'),
                                           self.INPUT_LAYER)
        extent_parameter.skip_crs_check = True
        self.addParameter(extent_parameter)
        self.addParameter(ParameterCrs(self.TARGET_AREA_CRS, 'Target area CRS'))

        self.addOutput(OutputHTML(self.OUTPUT_HTML_FILE,
                                  self.tr('Candidates')))

    def processAlgorithm(self, feedback):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT_LAYER))

        extent = self.getParameterValue(self.TARGET_AREA).split(',')
        target_crs = QgsCoordinateReferenceSystem(self.getParameterValue(self.TARGET_AREA_CRS))

        target_geom = QgsGeometry.fromRect(QgsRectangle(float(extent[0]), float(extent[2]),
                                                        float(extent[1]), float(extent[3])))

        output_file = self.getOutputValue(self.OUTPUT_HTML_FILE)

        # make intersection tests nice and fast
        engine = QgsGeometry.createGeometryEngine(target_geom.geometry())
        engine.prepareGeometry()

        layer_bounds = QgsGeometry.fromRect(layer.extent())

        results = []

        for srs_id in QgsCoordinateReferenceSystem.validSrsIds():
            candidate_crs = QgsCoordinateReferenceSystem.fromSrsId(srs_id)
            if not candidate_crs.isValid():
                continue

            transform_candidate = QgsCoordinateTransform(candidate_crs, target_crs)
            transformed_bounds = QgsGeometry(layer_bounds)
            try:
                if not transformed_bounds.transform(transform_candidate) == 0:
                    continue
            except:
                continue

            if engine.intersects(transformed_bounds.geometry()):
                results.append(candidate_crs.authid())

        self.createHTML(output_file, results)

    def createHTML(self, outputFile, candidates):
        with codecs.open(outputFile, 'w', encoding='utf-8') as f:
            f.write('<html><head>\n')
            f.write('<meta http-equiv="Content-Type" content="text/html; \
                    charset=utf-8" /></head><body>\n')
            for c in candidates:
                f.write('<p>' + c + '</p>\n')
            f.write('</body></html>\n')
