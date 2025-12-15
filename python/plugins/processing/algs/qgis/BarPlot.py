"""
***************************************************************************
    BarPlot.py
    ---------------------
    Date                 : March 2015
    Copyright            : (C) 2017 by Matteo Ghetta
    Email                : matteo dot ghetta at gmail dot com
***************************************************************************
* *
* This program is free software; you can redistribute it and/or modify  *
* it under the terms of the GNU General Public License as published by  *
* the Free Software Foundation; either version 2 of the License, or     *
* (at your option) any later version.                                   *
* *
***************************************************************************
"""

__author__ = "Matteo Ghetta"
__date__ = "March 2017"
__copyright__ = "(C) 2017, Matteo Ghetta"

import warnings

from qgis.core import (
    QgsFeatureRequest,
    QgsProcessingException,
    QgsProcessingParameterEnum,
    QgsProcessingParameterFeatureSource,
    QgsProcessingParameterField,
    QgsProcessingParameterFileDestination,
    QgsProcessingParameterString,
)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class BarPlot(QgisAlgorithm):
    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    NAME_FIELD = "NAME_FIELD"
    VALUE_FIELD = "VALUE_FIELD"
    TITLE = "TITLE"
    XAXIS_TITLE = "XAXIS_TITLE"
    YAXIS_TITLE = "YAXIS_TITLE"

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
            QgsProcessingParameterString(self.TITLE, self.tr("Title"), optional=True)
        )

        self.addParameter(
            QgsProcessingParameterString(
                self.XAXIS_TITLE, self.tr("X-axis title"), optional=True
            )
        )

        self.addParameter(
            QgsProcessingParameterString(
                self.YAXIS_TITLE, self.tr("Y-axis title"), optional=True
            )
        )

        self.addParameter(
            QgsProcessingParameterFileDestination(
                self.OUTPUT, self.tr("Bar plot"), self.tr("HTML files (*.html)")
            )
        )

    def name(self):
        return "barplot"

    def displayName(self):
        return self.tr("Bar plot")

    def shortDescription(self):
        return self.tr("Creates a bar plot from a category and a layer field.")

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

        namefieldname = self.parameterAsString(parameters, self.NAME_FIELD, context)
        valuefieldname = self.parameterAsString(parameters, self.VALUE_FIELD, context)

        title = self.parameterAsString(parameters, self.TITLE, context)
        xaxis_title = self.parameterAsString(parameters, self.XAXIS_TITLE, context)
        yaxis_title = self.parameterAsString(parameters, self.YAXIS_TITLE, context)

        if title.strip() == "":
            title = None
        if xaxis_title == "":
            xaxis_title = namefieldname
        elif xaxis_title == " ":
            xaxis_title = None
        if yaxis_title.strip() == "":
            yaxis_title = valuefieldname
        elif yaxis_title == " ":
            yaxis_title = None

        output = self.parameterAsFileOutput(parameters, self.OUTPUT, context)

        x_data = []
        y_data = []

        name_index = source.fields().lookupField(namefieldname)
        value_index = source.fields().lookupField(valuefieldname)

        # Optimize: Only fetch the 2 fields we need, and skip geometry
        req = QgsFeatureRequest().setFlags(QgsFeatureRequest.Flag.NoGeometry)
        req.setSubsetOfAttributes([name_index, value_index])

        for f in source.getFeatures(req):
            n_val = f[namefieldname]
            x_data.append(n_val if n_val is not None else "<NULL>")
            y_data.append(f[valuefieldname])

        data = [go.Bar(x=x_data, y=y_data)]

        fig = go.Figure(
            data=data,
            layout_title_text=title,
            layout_xaxis_title=xaxis_title,
            layout_yaxis_title=yaxis_title,
        )

        fig.write_html(output)

        return {self.OUTPUT: output}
