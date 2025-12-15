"""
***************************************************************************
    PolarPlot.py
    ---------------------
    Date                 : January 2013
    Copyright            : (C) 2013 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
* *
* This program is free software; you can redistribute it and/or modify  *
* it under the terms of the GNU General Public License as published by  *
* the Free Software Foundation; either version 2 of the License, or     *
* (at your option) any later version.                                   *
* *
***************************************************************************
"""

__author__ = "Victor Olaya"
__date__ = "January 2013"
__copyright__ = "(C) 2013, Victor Olaya"

import warnings

from qgis.core import (
    QgsFeatureRequest,
    QgsProcessingException,
    QgsProcessingParameterFeatureSource,
    QgsProcessingParameterField,
    QgsProcessingParameterFileDestination,
)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class PolarPlot(QgisAlgorithm):
    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    VALUE_FIELD = "VALUE_FIELD"

    def group(self):
        return self.tr("Plots")

    def groupId(self):
        return "plots"

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(
            QgsProcessingParameterFeatureSource(self.INPUT, self.tr("Input layer"))
        )
        self.addParameter(
            QgsProcessingParameterField(
                self.VALUE_FIELD,
                self.tr("Value field"),
                parentLayerParameterName=self.INPUT,
                type=QgsProcessingParameterField.DataType.Numeric,
            )
        )

        self.addParameter(
            QgsProcessingParameterFileDestination(
                self.OUTPUT, self.tr("Polar plot"), self.tr("HTML files (*.html)")
            )
        )

    def name(self):
        return "polarplot"

    def displayName(self):
        return self.tr("Polar plot")

    def shortDescription(self):
        return self.tr(
            "Generates a polar plot based on the value of an input vector layer."
        )

    def processAlgorithm(self, parameters, context, feedback):
        try:
            # importing plotly throws Python warnings from within the library - filter these out
            with warnings.catch_warnings():
                warnings.filterwarnings("ignore", category=ResourceWarning)
                warnings.filterwarnings("ignore", category=ImportWarning)
                import plotly as plt
                import plotly.graph_objs as go
        except ImportError:
            raise QgsProcessingException(
                self.tr(
                    "This algorithm requires the Python “plotly” library. Please install this library and try again."
                )
            )

        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(
                self.invalidSourceError(parameters, self.INPUT)
            )

        valuefieldname = self.parameterAsString(parameters, self.VALUE_FIELD, context)
        output = self.parameterAsFileOutput(parameters, self.OUTPUT, context)

        # Optimize: Only fetch the field we need, and skip geometry
        value_index = source.fields().lookupField(valuefieldname)
        req = QgsFeatureRequest().setFlags(QgsFeatureRequest.Flag.NoGeometry)
        req.setSubsetOfAttributes([value_index])

        values = [f[valuefieldname] for f in source.getFeatures(req)]

        # Calculate angles without numpy (standard Python logic)
        count = len(values)
        if count > 0:
            step = 360.0 / count
            theta = [i * step for i in range(count)]
        else:
            theta = []

        data = [
            go.Barpolar(
                r=values,
                theta=theta,
            )
        ]

        fig = go.Figure(data=data)
        fig.write_html(output)

        return {self.OUTPUT: output}
