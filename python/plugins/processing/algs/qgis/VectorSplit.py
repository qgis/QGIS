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
                       QgsProcessingParameterFolderOutput,
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
        return self.tr('Vector general tools')

    def __init__(self):
        super().__init__()

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))

        self.addParameter(QgsProcessingParameterField(self.FIELD,
                                                      self.tr('Unique ID field'), None, self.INPUT))

        self.addParameter(QgsProcessingParameterFolderOutput(self.OUTPUT,
                                                             self.tr('Output directory')))

        self.addOutput(QgsProcessingOutputFolder(self.OUTPUT, self.tr('Output directory')))

        self.source = None
        self.fieldName = None
        self.directory = None
        self.uniqueValues = None
        self.sinks = {}

    def name(self):
        return 'splitvectorlayer'

    def displayName(self):
        return self.tr('Split vector layer')

    def prepareAlgorithm(self, parameters, context, feedback):
        self.source = self.parameterAsSource(parameters, self.INPUT, context)
        self.fieldName = self.parameterAsString(parameters, self.FIELD, context)
        self.directory = self.parameterAsString(parameters, self.OUTPUT, context)
        mkdir(self.directory)

        fieldIndex = self.source.fields().lookupField(self.fieldName)
        self.uniqueValues = self.source.uniqueValues(fieldIndex)

        baseName = os.path.join(self.directory, '{0}'.format(self.fieldName))
        self.sinks = {}
        for current, i in enumerate(self.uniqueValues):
            if feedback.isCanceled():
                break
            fName = u'{0}_{1}.shp'.format(baseName, str(i).strip())
            feedback.pushInfo(self.tr('Creating layer: {}').format(fName))
            sink, dest = QgsProcessingUtils.createFeatureSink(fName, context, self.source.fields, self.source.wkbType(), self.source.sourceCrs())
            self.sinks[i] = sink
        return True

    def processAlgorithm(self, context, feedback):
        total = 100.0 / len(self.uniqueValues) if self.uniqueValues else 1
        for current, i in enumerate(self.uniqueValues):
            if feedback.isCanceled():
                break

            sink = self.sinks[i]

            filter = '{} = {}'.format(QgsExpression.quotedColumnRef(self.fieldName), QgsExpression.quotedValue(i))
            req = QgsFeatureRequest().setFilterExpression(filter)

            count = 0
            for f in self.source.getFeatures(req):
                if feedback.isCanceled():
                    break
                sink.addFeature(f, QgsFeatureSink.FastInsert)
                count += 1
            feedback.pushInfo(self.tr('Added {} features to layer').format(count))
            del sink

            feedback.setProgress(int(current * total))
        return True

    def postProcessAlgorithm(self, context, feedback):
        return {self.OUTPUT: self.directory}
