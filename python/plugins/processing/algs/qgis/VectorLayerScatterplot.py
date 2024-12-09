"""
***************************************************************************
    EquivalentNumField.py
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
    QgsProcessingParameterString,
    QgsProcessingParameterBoolean,
    QgsProcessingParameterExpression,
    QgsExpression,
    QgsExpressionContext,
    QgsExpressionContextUtils,
    QgsProcessingUtils,
    QgsFeatureRequest,
)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import vector

from qgis.PyQt.QtCore import QCoreApplication


class VectorLayerScatterplot(QgisAlgorithm):
    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    XFIELD = "XFIELD"
    YFIELD = "YFIELD"
    HOVERTEXT = "HOVERTEXT"
    TITLE = "TITLE"
    XAXIS_TITLE = "XAXIS_TITLE"
    YAXIS_TITLE = "YAXIS_TITLE"
    XAXIS_LOG = "XAXIS_LOG"
    YAXIS_LOG = "YAXIS_LOG"

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
                self.XFIELD,
                self.tr("X attribute"),
                parentLayerParameterName=self.INPUT,
                type=QgsProcessingParameterField.DataType.Numeric,
            )
        )
        self.addParameter(
            QgsProcessingParameterField(
                self.YFIELD,
                self.tr("Y attribute"),
                parentLayerParameterName=self.INPUT,
                type=QgsProcessingParameterField.DataType.Numeric,
            )
        )

        self.addParameter(
            QgsProcessingParameterExpression(
                self.HOVERTEXT,
                self.tr("Hover text"),
                parentLayerParameterName=self.INPUT,
                optional=True,
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
            QgsProcessingParameterBoolean(
                self.XAXIS_LOG,
                self.tr("Use logarithmic scale for x-axis"),
                defaultValue=False,
                optional=True,
            )
        )

        self.addParameter(
            QgsProcessingParameterBoolean(
                self.YAXIS_LOG,
                self.tr("Use logarithmic scale for y-axis"),
                defaultValue=False,
                optional=True,
            )
        )

        self.addParameter(
            QgsProcessingParameterFileDestination(
                self.OUTPUT, self.tr("Scatterplot"), self.tr("HTML files (*.html)")
            )
        )

    def name(self):
        return "vectorlayerscatterplot"

    def displayName(self):
        return self.tr("Vector layer scatterplot")

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
                    "VectorLayerScatterplot",
                    "This algorithm requires the Python “plotly” library. Please install this library and try again.",
                )
            )

        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(
                self.invalidSourceError(parameters, self.INPUT)
            )

        xfieldname = self.parameterAsString(parameters, self.XFIELD, context)
        yfieldname = self.parameterAsString(parameters, self.YFIELD, context)

        title = self.parameterAsString(parameters, self.TITLE, context)
        xaxis_title = self.parameterAsString(parameters, self.XAXIS_TITLE, context)
        yaxis_title = self.parameterAsString(parameters, self.YAXIS_TITLE, context)
        xaxis_log = self.parameterAsBool(parameters, self.XAXIS_LOG, context)
        yaxis_log = self.parameterAsBool(parameters, self.YAXIS_LOG, context)

        if title.strip() == "":
            title = None
        if xaxis_title == "":
            xaxis_title = xfieldname
        elif xaxis_title == " ":
            xaxis_title = None
        if yaxis_title == "":
            yaxis_title = yfieldname
        elif yaxis_title == " ":
            yaxis_title = None

        output = self.parameterAsFileOutput(parameters, self.OUTPUT, context)

        hoverexpression = self.parameterAsExpression(
            parameters, self.HOVERTEXT, context
        )
        if hoverexpression.strip():
            exp_context = self.createExpressionContext(parameters, context, source)
            hoverexpression = QgsExpression(hoverexpression)
            if hoverexpression.hasParserError():
                feedback.reportError(
                    f"Expression evaluation error: {hoverexpression.evalErrorString()}"
                )
                hoverexpression = None
        else:
            hoverexpression = None

        if xfieldname == yfieldname:
            fields = [xfieldname]
        else:
            fields = [xfieldname, yfieldname]

        values = {}
        hovertext = []

        for field in fields:
            values[field] = []

        request = QgsFeatureRequest().setFlags(QgsFeatureRequest.Flag.NoGeometry)
        for feature in source.getFeatures(request):
            for field in fields:
                try:
                    v = float(feature[field])
                except ValueError:
                    v = None
                values[field].append(v)
            if hoverexpression:
                exp_context.setFeature(feature)
                txt = str(hoverexpression.evaluate(exp_context))
                if hoverexpression.hasEvalError():
                    txt = ""
                    feedback.reportError(
                        f"Expression evaluation error: {hoverexpression.evalErrorString()}"
                    )
                hovertext.append(str(txt))

        data = [go.Scatter(x=values[xfieldname], y=values[yfieldname], mode="markers")]
        fig = go.Figure(
            data=data,
            layout_title_text=title,
            layout_xaxis_title=xaxis_title,
            layout_yaxis_title=yaxis_title,
        )

        if xaxis_log:
            fig.update_xaxes(type="log")

        if yaxis_log:
            fig.update_yaxes(type="log")

        if hoverexpression:
            fig.update_traces(text=hovertext)

        fig.write_html(output)

        return {self.OUTPUT: output}
