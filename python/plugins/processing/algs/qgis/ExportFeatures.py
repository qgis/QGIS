"""
***************************************************************************
    ExportFeatures.py
    ---------------------
    Date                 : March 2024
    Copyright            : (C) 2024 by Joona Laine
    Email                : joona dot laine at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Joona Laine'
__date__ = 'March 2024'
__copyright__ = '(C) 2024, Joona Laine'


from qgis.core import (
    QgsProcessingException,
    QgsProcessingParameterVectorLayer,
    QgsVectorDataProvider,
    QgsVectorLayerUtils,
)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class ExportFeatures(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def initAlgorithm(
        self, config=None
    ) -> None:
        self.addParameter(
            QgsProcessingParameterVectorLayer(
                name=self.INPUT,
                description=self.tr('Input layer'),
            )
        )
        self.addParameter(
            QgsProcessingParameterVectorLayer(
                name=self.OUTPUT,
                description=self.tr('Output layer'),
            )
        )

    def name(self) -> str:
        return 'exportfeatures'

    def displayName(self) -> str:
        return self.tr('Export features to existing layer')

    def group(self):
        return self.tr('Vector general')

    def groupId(self):
        return 'vectorgeneral'

    def processAlgorithm(
        self,
        parameters,
        context,
        model_feedback,
    ) -> dict:
        inLayer = self.parameterAsVectorLayer(parameters, self.INPUT, context)
        outLayer = self.parameterAsVectorLayer(parameters, self.OUTPUT, context)

        # Layer has to be in edit mode in order to
        if not outLayer.isEditable():
            raise QgsProcessingException(
                self.tr('Layer {} is not in edit mode. Start editing for layer and try again', outLayer.name())
            )
        if not outLayer.dataProvider().capabilities() & QgsVectorDataProvider.AddFeatures:
            raise QgsProcessingException(
                self.tr(
                    'Provider of layer {} does not allow adding features', outLayer.name()
                )
            )

        outLayer.addFeatures(
            QgsVectorLayerUtils.makeFeaturesCompatible(
                features=inLayer.getFeatures(),
                layer=outLayer,
            )
        )

        return {}
