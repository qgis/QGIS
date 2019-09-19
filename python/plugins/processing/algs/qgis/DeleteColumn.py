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

from qgis.PyQt.QtCore import QCoreApplication

from qgis.core import (QgsProcessingParameterField,
                       QgsProcessing,
                       QgsProcessingAlgorithm,
                       QgsProcessingFeatureSource)
from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm


class DeleteColumn(QgisFeatureBasedAlgorithm):

    COLUMNS = 'COLUMN'

    def flags(self):
        return super().flags() & ~QgsProcessingAlgorithm.FlagSupportsInPlaceEdits

    def tags(self):
        return self.tr('drop,delete,remove,fields,columns,attributes').split(',')

    def group(self):
        return self.tr('Vector table')

    def groupId(self):
        return 'vectortable'

    def __init__(self):
        super().__init__()
        self.fields_to_delete = []
        self.field_indices = []

    def initParameters(self, config=None):
        self.addParameter(QgsProcessingParameterField(self.COLUMNS,
                                                      self.tr('Fields to drop'),
                                                      None, 'INPUT', QgsProcessingParameterField.Any, True))

    def inputLayerTypes(self):
        return [QgsProcessing.TypeVector]

    def name(self):
        return 'deletecolumn'

    def displayName(self):
        return self.tr('Drop field(s)')

    def outputName(self):
        return self.tr('Remaining fields')

    def prepareAlgorithm(self, parameters, context, feedback):
        self.fields_to_delete = self.parameterAsFields(parameters, self.COLUMNS, context)

        source = self.parameterAsSource(parameters, 'INPUT', context)
        if source is not None:
            for f in self.fields_to_delete:
                index = source.fields().lookupField(f)
                if index < 0:
                    feedback.pushInfo(QCoreApplication.translate('DeleteColumn', 'Field “{}” does not exist in input layer').format(f))

        return super().prepareAlgorithm(parameters, context, feedback)

    def outputFields(self, input_fields):
        # loop through twice - first we need to build up a list of original attribute indices
        for f in self.fields_to_delete:
            index = input_fields.lookupField(f)
            if index >= 0:
                self.field_indices.append(index)

        # important - make sure we remove from the end so we aren't changing used indices as we go
        self.field_indices.sort(reverse=True)

        # this second time we make a cleaned version of the fields
        for index in self.field_indices:
            input_fields.remove(index)
        return input_fields

    def sourceFlags(self):
        return QgsProcessingFeatureSource.FlagSkipGeometryValidityChecks

    def processFeature(self, feature, context, feedback):
        attributes = feature.attributes()
        for index in self.field_indices:
            del attributes[index]
        feature.setAttributes(attributes)
        return [feature]
