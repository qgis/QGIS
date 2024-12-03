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
    QgsProcessingParameterFeatureSource,
    QgsProcessingParameterField,
    QgsProcessingParameterFileDestination,
    QgsProcessingException,
    QgsProcessingParameterString,
)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

from processing.tools import vector

from qgis.PyQt.QtCore import QCoreApplication


class VectorLayerScatterplot3D(QgisAlgorithm):
    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    XFIELD = "XFIELD"
    YFIELD = "YFIELD"
    ZFIELD = "ZFIELD"
    TITLE = "TITLE"
    XAXIS_TITLE = "XAXIS_TITLE"
    YAXIS_TITLE = "YAXIS_TITLE"
    ZAXIS_TITLE = "ZAXIS_TITLE"

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
            QgsProcessingParameterField(
                self.ZFIELD,
                self.tr("Z attribute"),
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
            QgsProcessingParameterString(
                self.ZAXIS_TITLE, self.tr("Z-axis title"), optional=True
            )
        )

        self.addParameter(
            QgsProcessingParameterFileDestination(
                self.OUTPUT, self.tr("Scatterplot 3D"), self.tr("HTML files (*.html)")
            )
        )

    def name(self):
        return "scatter3dplot"

    def displayName(self):
        return self.tr("Vector layer scatterplot 3D")

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
                    "VectorLayerScatterplot3D",
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
        zfieldname = self.parameterAsString(parameters, self.ZFIELD, context)

        title = self.parameterAsString(parameters, self.TITLE, context)
        xaxis_title = self.parameterAsString(parameters, self.XAXIS_TITLE, context)
        yaxis_title = self.parameterAsString(parameters, self.YAXIS_TITLE, context)
        zaxis_title = self.parameterAsString(parameters, self.ZAXIS_TITLE, context)

        if title.strip() == "":
            title = None
        # Passing None for axis titles to Plotly does not work in this case: Unlike in 2D charts, Plotly would use x, y, z as axis titles.
        # If our users enter space, we still get invisible axes titles, resulting in the same behavior as for the other algs.
        if xaxis_title == "":
            xaxis_title = xfieldname
        if yaxis_title == "":
            yaxis_title = yfieldname
        if zaxis_title == "":
            zaxis_title = zfieldname

        output = self.parameterAsFileOutput(parameters, self.OUTPUT, context)

        values = vector.values(source, xfieldname, yfieldname, zfieldname)

        data = [
            go.Scatter3d(
                x=values[xfieldname],
                y=values[yfieldname],
                z=values[zfieldname],
                mode="markers",
            )
        ]

        fig = go.Figure(
            data=data,
            layout_title_text=title,
            layout_scene_xaxis_title=xaxis_title,
            layout_scene_yaxis_title=yaxis_title,
            layout_scene_zaxis_title=zaxis_title,
        )

        fig.write_html(output)

        return {self.OUTPUT: output}
