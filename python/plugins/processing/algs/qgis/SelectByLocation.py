# -*- coding: utf-8 -*-

"""
***************************************************************************
    SelectByLocation.py
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

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsGeometry,
                       QgsFeatureRequest,
                       QgsProcessing,
                       QgsProcessingParameterVectorLayer,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterEnum,
                       QgsProcessingOutputVectorLayer,
                       QgsVectorLayer)


from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class SelectByLocation(QgisAlgorithm):

    INPUT = 'INPUT'
    INTERSECT = 'INTERSECT'
    PREDICATE = 'PREDICATE'
    METHOD = 'METHOD'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'select_location.png'))

    def group(self):
        return self.tr('Vector selection')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.predicates = (
            ('intersects', self.tr('intersects')),
            ('contains', self.tr('contains')),
            ('disjoint', self.tr('is disjoint')),
            ('isEqual', self.tr('equals')),
            ('touches', self.tr('touches')),
            ('overlaps', self.tr('overlaps')),
            ('within', self.tr('within')),
            ('crosses', self.tr('crosses')))

        self.reversed_predicates = {'intersects': 'intersects',
                                    'contains': 'within',
                                    'disjoint': 'disjoint',
                                    'isEqual': 'isEqual',
                                    'touches': 'touches',
                                    'overlaps': 'overlaps',
                                    'within': 'contains',
                                    'crosses': 'crosses'}

        self.methods = [self.tr('creating new selection'),
                        self.tr('adding to current selection'),
                        self.tr('select within current selection'),
                        self.tr('removing from current selection')]

        self.addParameter(QgsProcessingParameterVectorLayer(self.INPUT,
                                                            self.tr('Select features from'), types=[QgsProcessing.TypeVectorAnyGeometry]))
        self.addParameter(QgsProcessingParameterEnum(self.PREDICATE,
                                                     self.tr('Where the features are (geometric predicate)'),
                                                     options=[p[1] for p in self.predicates],
                                                     allowMultiple=True, defaultValue=[0]))
        self.addParameter(QgsProcessingParameterFeatureSource(self.INTERSECT,
                                                              self.tr('By comparing to the features from'), types=[QgsProcessing.TypeVectorAnyGeometry]))
        self.addParameter(QgsProcessingParameterEnum(self.METHOD,
                                                     self.tr('Modify current selection by'),
                                                     options=self.methods, defaultValue=0))

        self.addOutput(QgsProcessingOutputVectorLayer(self.OUTPUT, self.tr('Selected (by location)')))

    def name(self):
        return 'selectbylocation'

    def displayName(self):
        return self.tr('Select by location')

    def processAlgorithm(self, parameters, context, feedback):
        select_layer = self.parameterAsVectorLayer(parameters, self.INPUT, context)
        method = QgsVectorLayer.SelectBehavior(self.parameterAsEnum(parameters, self.METHOD, context))
        intersect_source = self.parameterAsSource(parameters, self.INTERSECT, context)
        # build a list of 'reversed' predicates, because in this function
        # we actually test the reverse of what the user wants (allowing us
        # to prepare geometries and optimise the algorithm)
        predicates = [self.reversed_predicates[self.predicates[i][0]] for i in self.parameterAsEnums(parameters, self.PREDICATE, context)]

        if 'disjoint' in predicates:
            disjoint_set = select_layer.allFeatureIds()
        else:
            disjoint_set = None

        selected_set = set()
        request = QgsFeatureRequest().setSubsetOfAttributes([]).setDestinationCrs(select_layer.crs())
        features = intersect_source.getFeatures(request)
        total = 100.0 / intersect_source.featureCount() if intersect_source.featureCount() else 0
        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            if not f.hasGeometry():
                continue

            engine = QgsGeometry.createGeometryEngine(f.geometry().geometry())
            engine.prepareGeometry()
            bbox = f.geometry().boundingBox()

            request = QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry).setFilterRect(bbox).setSubsetOfAttributes([])
            for test_feat in select_layer.getFeatures(request):
                if feedback.isCanceled():
                    break

                if test_feat in selected_set:
                    # already added this one, no need for further tests
                    continue

                for predicate in predicates:
                    if predicate == 'disjoint':
                        if test_feat.geometry().intersects(f.geometry()):
                            try:
                                disjoint_set.remove(test_feat.id())
                            except:
                                pass  # already removed
                    else:
                        if getattr(engine, predicate)(test_feat.geometry().geometry()):
                            selected_set.add(test_feat.id())
                            break

            feedback.setProgress(int(current * total))

        if 'disjoint' in predicates:
            selected_set = list(selected_set) + disjoint_set

        select_layer.selectByIds(list(selected_set), method)
        return {self.OUTPUT: parameters[self.INPUT]}
