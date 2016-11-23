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

from qgis.core import QgsFeature
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class DeleteColumn(GeoAlgorithm):

    INPUT = 'INPUT'
    COLUMNS = 'COLUMN'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Delete column')
        self.group, self.i18n_group = self.trAlgorithm('Vector table tools')
        self.tags = self.tr('drop,delete,remove,fields,columns,attributes')

        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input layer')))
        self.addParameter(ParameterTableField(self.COLUMNS,
                                              self.tr('Fields to delete'), self.INPUT, multiple=True))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Output layer')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT))

        fields_to_delete = self.getParameterValue(self.COLUMNS).split(';')
        fields = layer.fields()
        field_indices = []
        # loop through twice - first we need to build up a list of original attribute indices
        for f in fields_to_delete:
            index = fields.lookupField(f)
            field_indices.append(index)

        # important - make sure we remove from the end so we aren't changing used indices as we go
        field_indices.sort(reverse=True)

        # this second time we make a cleaned version of the fields
        for index in field_indices:
            fields.remove(index)

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fields,
                                                                     layer.wkbType(), layer.crs())

        features = vector.features(layer)
        total = 100.0 / len(features)

        for current, f in enumerate(features):
            attributes = f.attributes()
            for index in field_indices:
                del attributes[index]
            f.setAttributes(attributes)
            writer.addFeature(f)

            progress.setPercentage(int(current * total))

        del writer
