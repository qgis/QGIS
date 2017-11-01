# -*- coding: utf-8 -*-

"""
***************************************************************************
    VectorSplit.py
    ---------------------
    Date                 : September 2014
    Copyright            : (C) 2014 by Alexander Bruy
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
from builtins import str

__author__ = 'Alexander Bruy'
__date__ = 'September 2014'
__copyright__ = '(C) 2014, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.core import (QgsProcessingUtils,
                       QgsFeatureSink,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterField,
                       QgsProcessingParameterFolderDestination,
                       QgsProcessingOutputFolder,
                       QgsExpression,
                       QgsFeatureRequest)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools.system import mkdir

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class VectorSplit(QgisAlgorithm):

    INPUT = 'INPUT'
    FIELD = 'FIELD'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector general')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))

        self.addParameter(QgsProcessingParameterField(self.FIELD,
                                                      self.tr('Unique ID field'), None, self.INPUT))

        self.addParameter(QgsProcessingParameterFolderDestination(self.OUTPUT,
                                                                  self.tr('Output directory')))

        self.addOutput(QgsProcessingOutputFolder(self.OUTPUT, self.tr('Output directory')))

    def name(self):
        return 'splitvectorlayer'

    def displayName(self):
        return self.tr('Split vector layer')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        fieldName = self.parameterAsString(parameters, self.FIELD, context)
        directory = self.parameterAsString(parameters, self.OUTPUT, context)

        mkdir(directory)

        fieldIndex = source.fields().lookupField(fieldName)
        uniqueValues = source.uniqueValues(fieldIndex)
        baseName = os.path.join(directory, '{0}'.format(fieldName))

        fields = source.fields()
        crs = source.sourceCrs()
        geomType = source.wkbType()

        total = 100.0 / len(uniqueValues) if uniqueValues else 1

        for current, i in enumerate(uniqueValues):
            if feedback.isCanceled():
                break
            fName = u'{0}_{1}.shp'.format(baseName, str(i).strip())
            feedback.pushInfo(self.tr('Creating layer: {}').format(fName))

            sink, dest = QgsProcessingUtils.createFeatureSink(fName, context, fields, geomType, crs)

            filter = '{} = {}'.format(QgsExpression.quotedColumnRef(fieldName), QgsExpression.quotedValue(i))
            req = QgsFeatureRequest().setFilterExpression(filter)

            count = 0
            for f in source.getFeatures(req):
                if feedback.isCanceled():
                    break
                sink.addFeature(f, QgsFeatureSink.FastInsert)
                count += 1
            feedback.pushInfo(self.tr('Added {} features to layer').format(count))
            del sink

            feedback.setProgress(int(current * total))

        return {self.OUTPUT: directory}
