"""
***************************************************************************
    SelectByExpression.py
    ---------------------
    Date                 : July 2014
    Copyright            : (C) 2014 by Michael Douchin
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Michael Douchin"
__date__ = "July 2014"
__copyright__ = "(C) 2014, Michael Douchin"

from qgis.core import (
    QgsExpression,
    QgsProcessing,
    QgsVectorLayer,
    QgsProcessingAlgorithm,
    QgsProcessingException,
    QgsProcessingParameterVectorLayer,
    QgsProcessingParameterExpression,
    QgsProcessingParameterEnum,
    QgsProcessingOutputVectorLayer,
)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class SelectByExpression(QgisAlgorithm):
    INPUT = "INPUT"
    EXPRESSION = "EXPRESSION"
    OUTPUT = "OUTPUT"
    METHOD = "METHOD"

    def group(self):
        return self.tr("Vector selection")

    def groupId(self):
        return "vectorselection"

    def __init__(self):
        super().__init__()

    def flags(self):
        return (
            super().flags()
            | QgsProcessingAlgorithm.Flag.FlagNoThreading
            | QgsProcessingAlgorithm.Flag.FlagNotAvailableInStandaloneTool
        )

    def initAlgorithm(self, config=None):
        self.methods = [
            self.tr("creating new selection"),
            self.tr("adding to current selection"),
            self.tr("removing from current selection"),
            self.tr("selecting within current selection"),
        ]

        self.addParameter(
            QgsProcessingParameterVectorLayer(
                self.INPUT,
                self.tr("Input layer"),
                types=[QgsProcessing.SourceType.TypeVector],
            )
        )

        self.addParameter(
            QgsProcessingParameterExpression(
                self.EXPRESSION,
                self.tr("Expression"),
                parentLayerParameterName=self.INPUT,
            )
        )
        self.addParameter(
            QgsProcessingParameterEnum(
                self.METHOD,
                self.tr("Modify current selection by"),
                self.methods,
                defaultValue=0,
            )
        )

        self.addOutput(
            QgsProcessingOutputVectorLayer(self.OUTPUT, self.tr("Selected (attribute)"))
        )

    def name(self):
        return "selectbyexpression"

    def displayName(self):
        return self.tr("Select by expression")

    def processAlgorithm(self, parameters, context, feedback):
        layer = self.parameterAsVectorLayer(parameters, self.INPUT, context)

        method = self.parameterAsEnum(parameters, self.METHOD, context)
        if method == 0:
            behavior = QgsVectorLayer.SelectBehavior.SetSelection
        elif method == 1:
            behavior = QgsVectorLayer.SelectBehavior.AddToSelection
        elif method == 2:
            behavior = QgsVectorLayer.SelectBehavior.RemoveFromSelection
        elif method == 3:
            behavior = QgsVectorLayer.SelectBehavior.IntersectSelection

        expression = self.parameterAsString(parameters, self.EXPRESSION, context)
        qExp = QgsExpression(expression)
        if qExp.hasParserError():
            raise QgsProcessingException(qExp.parserErrorString())

        expression_context = self.createExpressionContext(parameters, context)

        layer.selectByExpression(expression, behavior, expression_context)

        return {self.OUTPUT: parameters[self.INPUT]}
