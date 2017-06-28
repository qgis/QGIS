# -*- coding: utf-8 -*-

"""
***************************************************************************
    CreateAttributeIndex.py
    -----------------------
    Date                 : November 2016
    Copyright            : (C) 2016 by Nyall Dawson
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
__date__ = 'November 2016'
__copyright__ = '(C) 2016, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsVectorDataProvider,
                       QgsFields,
                       QgsApplication,
                       QgsProcessingParameterVectorLayer,
                       QgsProcessingParameterField,
                       QgsProcessingParameterDefinition,
                       QgsProcessingOutputVectorLayer)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class CreateAttributeIndex(QgisAlgorithm):

    INPUT = 'INPUT'
    FIELD = 'FIELD'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector general tools')

    def __init__(self):
        super().__init__()
        self.addParameter(QgsProcessingParameterVectorLayer(self.INPUT,
                                                            self.tr('Input Layer')))
        self.addParameter(QgsProcessingParameterField(self.FIELD,
                                                      self.tr('Attribute to index'), None, self.INPUT))
        self.addOutput(QgsProcessingOutputVectorLayer(self.OUTPUT, self.tr('Indexed layer')))

        self.layer = None
        self.field = None

    def name(self):
        return 'createattributeindex'

    def displayName(self):
        return self.tr('Create attribute index')

    def prepareAlgorithm(self, parameters, context, feedback):
        self.layer = self.parameterAsVectorLayer(parameters, self.INPUT, context)
        self.field = self.parameterAsString(parameters, self.FIELD, context)
        return True

    def processAlgorithm(self, context, feedback):
        provider = self.layer.dataProvider()

        field_index = self.layer.fields().lookupField(self.field)
        if field_index < 0 or self.layer.fields().fieldOrigin(field_index) != QgsFields.OriginProvider:
            feedback.pushInfo(self.tr('Can not create attribute index on "{}"').format(self.field))
        else:
            provider_index = self.layer.fields().fieldOriginIndex(field_index)
            if provider.capabilities() & QgsVectorDataProvider.CreateAttributeIndex:
                if not provider.createAttributeIndex(provider_index):
                    feedback.pushInfo(self.tr('Could not create attribute index'))
            else:
                feedback.pushInfo(self.tr("Layer's data provider does not support "
                                          "creating attribute indexes"))
        return True

    def postProcessAlgorithm(self, context, feedback):
        return {self.OUTPUT: self.layer.id()}
