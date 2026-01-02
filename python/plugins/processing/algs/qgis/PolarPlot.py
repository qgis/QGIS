"""
***************************************************************************
    PolarPlot.py
    ---------------------
    Date                 : January 2013
    Copyright            : (C) 2013 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Victor Olaya"
__date__ = "January 2013"
__copyright__ = "(C) 2013, Victor Olaya"

import warnings

from qgis.core import (
    QgsProcessingException,
    QgsProcessingParameterFeatureSource,
    QgsProcessingParameterField,
    QgsProcessingParameterFileDestination,
    QgsFeatureRequest,
    QgsProcessingParameterString,
)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import vector

from qgis.PyQt.QtCore import QCoreApplication


class PolarPlot(QgisAlgorithm):
    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    NAME_FIELD = "NAME_FIELD"
    VALUE_FIELD = "VALUE_FIELD"
    TITLE = "TITLE"

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
                self.NAME_FIELD,
                self.tr("Category name field"),
                parentLayerParameterName=self.INPUT,
                type=QgsProcessingParameterField.DataType.Any,
            )
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
                self.OUTPUT, self.tr("Polar bar plot"), self.tr("HTML files (*.html)")
            )
        )
        self.addParameter(
            QgsProcessingParameterString(self.TITLE, self.tr("Title"), optional=True)
        )

    def name(self):
        return "polarplot"

    def displayName(self):
        return self.tr("Polar bar plot")

    def shortDescription(self):
        return self.tr(
            "Creates a polar bar plot based on values grouped by a category field."
        )

    def shortHelpString(self):
        return self.tr(
            "This algorithm creates a polar bar plot based on values grouped by a category field."
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
                QCoreApplication.translate(
                    "PolarPlot",
                    "This algorithm requires the Python “plotly” library. Please install this library and try again.",
                )
            )

        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(
                self.invalidSourceError(parameters, self.INPUT)
            )

        namefieldname = self.parameterAsString(parameters, self.NAME_FIELD, context)
        valuefieldname = self.parameterAsString(parameters, self.VALUE_FIELD, context)
        title = self.parameterAsString(parameters, self.TITLE, context)

        if title.strip() == "":
            title = None

        output = self.parameterAsFileOutput(parameters, self.OUTPUT, context)

        values = vector.values(source, valuefieldname)

        # Load a vector of categories
        category_index = source.fields().lookupField(namefieldname)
        categories = vector.convert_nulls(
            [
                i[namefieldname]
                for i in source.getFeatures(
                    QgsFeatureRequest()
                    .setFlags(QgsFeatureRequest.Flag.NoGeometry)
                    .setSubsetOfAttributes([category_index])
                )
            ],
            "<NULL>",
        )
        # Sum up values by category
        category_sums = { category: 0 for category in set(categories) }
        for idx in range(len(categories)):
            category_sums[categories[idx]] += values[valuefieldname][idx]

        data = [
            go.Barpolar(
                r=list(category_sums.values()),
                theta=list(category_sums.keys()),
            )
        ]

        fig = go.Figure(
            data=data,
            layout_title_text=title,
        )

        fig.write_html(output)

        return {self.OUTPUT: output}
