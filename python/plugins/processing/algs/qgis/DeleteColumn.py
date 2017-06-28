# -*- coding: utf-8 -*-

"""
***************************************************************************
    DeleteColumn.py
    ---------------------
    Date                 : May 2010
    Copyright            : (C) 2010 by Michael Minn
    Email                : pyqgis at michaelminn dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Michael Minn'
__date__ = 'May 2010'
__copyright__ = '(C) 2010, Michael Minn'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsApplication,
                       QgsFeatureSink,
                       QgsProcessingUtils,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterField,
                       QgsProcessingOutputVectorLayer)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class DeleteColumn(QgisAlgorithm):

    INPUT = 'INPUT'
    COLUMNS = 'COLUMN'
    OUTPUT = 'OUTPUT'

    def tags(self):
        return self.tr('drop,delete,remove,fields,columns,attributes').split(',')

    def group(self):
        return self.tr('Vector table tools')

    def __init__(self):
        super().__init__()

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT, self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterField(self.COLUMNS,
                                                      self.tr('Fields to drop'),
                                                      None, self.INPUT, QgsProcessingParameterField.Any, True))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Output layer')))
        self.addOutput(QgsProcessingOutputVectorLayer(self.OUTPUT, self.tr("Output layer")))

        self.source = None
        self.fields_to_delete = None
        self.sink = None
        self.dest_id = None
        self.field_indices = []

    def name(self):
        return 'deletecolumn'

    def displayName(self):
        return self.tr('Drop field(s)')

    def prepareAlgorithm(self, parameters, context, feedback):
        self.source = self.parameterAsSource(parameters, self.INPUT, context)
        self.fields_to_delete = self.parameterAsFields(parameters, self.COLUMNS, context)

        fields = self.source.fields()
        # loop through twice - first we need to build up a list of original attribute indices
        for f in self.fields_to_delete:
            index = fields.lookupField(f)
            self.field_indices.append(index)

        # important - make sure we remove from the end so we aren't changing used indices as we go
        self.field_indices.sort(reverse=True)

        # this second time we make a cleaned version of the fields
        for index in self.field_indices:
            fields.remove(index)

        (self.sink, self.dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                                         fields, self.source.wkbType(), self.source.sourceCrs())
        return True

    def processAlgorithm(self, context, feedback):
        features = self.source.getFeatures()
        total = 100.0 / self.source.featureCount() if self.source.featureCount() else 0

        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            attributes = f.attributes()
            for index in self.field_indices:
                del attributes[index]
            f.setAttributes(attributes)
            self.sink.addFeature(f, QgsFeatureSink.FastInsert)

            feedback.setProgress(int(current * total))

        return True

    def postProcessAlgorithm(self, context, feedback):
        return {self.OUTPUT: self.dest_id}
