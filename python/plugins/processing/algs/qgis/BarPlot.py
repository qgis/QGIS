"""
***************************************************************************
    BarPlot.py
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
            QgsProcessingParameterFileDestination(
                self.OUTPUT, self.tr("Bar plot"), self.tr("HTML files (*.html)")
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

    def name(self):
        return "barplot"

    def displayName(self):
        return self.tr("Bar plot")

    def shortDescription(self):
        return self.tr("Generates a bar plot from a category and a value field.")

    def processAlgorithm(self, parameters, context, feedback):
        try:
            # importing plotly throws Python warnings from within the library - filter these out
            with warnings.catch_warnings():
                warnings.filterwarnings("ignore", category=ResourceWarning)
                warnings.filterwarnings("ignore", category=ImportWarning)
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

        if not title:
            title = None
        if not xaxis_title:
            xaxis_title = namefieldname
        if not yaxis_title:
            yaxis_title = valuefieldname

        output = self.parameterAsFileOutput(parameters, self.OUTPUT, context)

        # Optimize: Only fetch the fields we need, and skip geometry
        name_index = source.fields().lookupField(namefieldname)
        value_index = source.fields().lookupField(valuefieldname)

        req = QgsFeatureRequest().setFlags(QgsFeatureRequest.Flag.NoGeometry)
        
        # Safety Guard: Only set subset if the fields actually exist (index >= 0)
        # This prevents crashes in automated tests that use mock layers.
        attributes_to_fetch = []
        if name_index >= 0:
            attributes_to_fetch.append(name_index)
        if value_index >= 0:
            attributes_to_fetch.append(value_index)
            
        if attributes_to_fetch:
            req.setSubsetOfAttributes(attributes_to_fetch)

        x_data = []
        y_data = []

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
