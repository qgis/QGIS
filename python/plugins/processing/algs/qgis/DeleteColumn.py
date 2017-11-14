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

from qgis.core import (QgsProcessingParameterField,
                       QgsProcessing)
from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm


class DeleteColumn(QgisFeatureBasedAlgorithm):

    COLUMNS = 'COLUMN'

    def tags(self):
        return self.tr('drop,delete,remove,fields,columns,attributes').split(',')

    def group(self):
        return self.tr('Vector table')

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
        return self.tr('Fields dropped')

    def prepareAlgorithm(self, parameters, context, feedback):
        self.fields_to_delete = self.parameterAsFields(parameters, self.COLUMNS, context)
        return True

    def outputFields(self, input_fields):
        # loop through twice - first we need to build up a list of original attribute indices
        for f in self.fields_to_delete:
            index = input_fields.lookupField(f)
            self.field_indices.append(index)

        # important - make sure we remove from the end so we aren't changing used indices as we go
        self.field_indices.sort(reverse=True)

        # this second time we make a cleaned version of the fields
        for index in self.field_indices:
            input_fields.remove(index)
        return input_fields

    def processFeature(self, feature, feedback):
        attributes = feature.attributes()
        for index in self.field_indices:
            del attributes[index]
        feature.setAttributes(attributes)
        return feature
