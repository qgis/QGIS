# -*- coding: utf-8 -*-

"""
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

from PyQt5.QtCore import QCoreApplication
from qgis.core import (QgsProcessing,
                       QgsFeatureSink,
                       QgsProcessingException,
                       QgsProcessingAlgorithm,
                       QgsProcessingParameterField,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink)
import processing

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

class DeleteDuplicateFields(QgisAlgorithm):

    INPUT = 'INPUT'
    UNIQUE_FIELD = 'UNIQUE_FIELD'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()
    
    def createInstance(self):
        return DeleteDuplicateFields()

    def name(self):
        return 'deleteduplicatefields'

    def displayName(self):
        return self.tr('Delete Duplicates Features')

    def group(self):
        return self.tr('Vector table')

    def groupId(self):
        return 'vectortable'

    def shortHelpString(self):
        return self.tr("""Remove features based on a given unique field. 
                        If the values of the given fields are duplicated, the algorithms only keep the first feature. Others fields are unchanged.""")

    def initAlgorithm(self, config=None):
        self.addParameter(
            QgsProcessingParameterFeatureSource(
                self.INPUT,
                self.tr('Input layer'),
                [QgsProcessing.TypeVectorAnyGeometry]
            )
        )
        
        self.addParameter(
            QgsProcessingParameterField(
                self.UNIQUE_FIELD,
                self.tr('Unique field'),
                parentLayerParameterName=self.INPUT
            )
        )

        self.addParameter(
            QgsProcessingParameterFeatureSink(
                self.OUTPUT,
                self.tr('Output layer')
            )
        )

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(
            parameters,
            self.INPUT,
            context
        )
        unique_field = self.parameterAsFields(parameters,self.UNIQUE_FIELD,context)[0]

        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        (sink, dest_id) = self.parameterAsSink(
            parameters,
            self.OUTPUT,
            context,
            source.fields(),
            source.wkbType(),
            source.sourceCrs()
        )

        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        total = 100.0 / source.featureCount() if source.featureCount() else 0
        features = source.getFeatures()

        vals=set()

        for current, feature in enumerate(features):
            # Stop the algorithm if cancel button has been clicked
            if feedback.isCanceled():
                break
            current_val=feature.attribute(unique_field)
            if current_val in vals :
                continue
            else :
                vals.add(current_val)
                sink.addFeature(feature, QgsFeatureSink.FastInsert)

            # Update the progress bar
            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
