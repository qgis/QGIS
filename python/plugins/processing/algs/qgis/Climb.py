"""
/***************************************************************************
 Climb
        begin                : 2019-05-15
        copyright            : (C) 2019 by Håvard Tveite
        email                : havard.tveite@nmbu.no
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""

__author__ = "Håvard Tveite"
__date__ = "2019-03-01"
__copyright__ = "(C) 2019 by Håvard Tveite"

import os
import math

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QMetaType

from qgis.core import (
    QgsProcessing,
    QgsFeatureSink,
    QgsProcessingAlgorithm,
    QgsProcessingParameterFeatureSource,
    QgsProcessingParameterFeatureSink,
    QgsProcessingOutputNumber,
    QgsProcessingException,
    QgsProcessingUtils,
    QgsWkbTypes,
    QgsFields,
    QgsField,
)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class Climb(QgisAlgorithm):
    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    TOTALCLIMB = "TOTALCLIMB"
    TOTALDESCENT = "TOTALDESCENT"
    MINELEVATION = "MINELEVATION"
    MAXELEVATION = "MAXELEVATION"
    CLIMBATTRIBUTE = "climb"
    DESCENTATTRIBUTE = "descent"
    MINELEVATTRIBUTE = "minelev"
    MAXELEVATTRIBUTE = "maxelev"

    def name(self):
        return "climbalongline"

    def displayName(self):
        return self.tr("Climb along line")

    def group(self):
        return self.tr("Vector analysis")

    def groupId(self):
        return "vectoranalysis"

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(
            QgsProcessingParameterFeatureSource(
                self.INPUT,
                self.tr("Line layer"),
                [QgsProcessing.SourceType.TypeVectorLine],
            )
        )

        self.addParameter(
            QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr("Climb layer"))
        )

        self.addOutput(
            QgsProcessingOutputNumber(self.TOTALCLIMB, self.tr("Total climb"))
        )

        self.addOutput(
            QgsProcessingOutputNumber(self.TOTALDESCENT, self.tr("Total descent"))
        )

        self.addOutput(
            QgsProcessingOutputNumber(self.MINELEVATION, self.tr("Minimum elevation"))
        )

        self.addOutput(
            QgsProcessingOutputNumber(self.MAXELEVATION, self.tr("Maximum elevation"))
        )

    def processAlgorithm(self, parameters, context, feedback):

        source = self.parameterAsSource(parameters, self.INPUT, context)

        fcount = source.featureCount()
        source_fields = source.fields()

        hasZ = QgsWkbTypes.hasZ(source.wkbType())

        if not hasZ:
            raise QgsProcessingException(
                self.tr(
                    "The layer does not have Z values. If you have a DEM, use the Drape algorithm to extract Z values."
                )
            )

        thefields = QgsFields()
        climbindex = -1
        descentindex = -1
        minelevindex = -1
        maxelevindex = -1
        fieldnumber = 0

        # Create new fields for climb and descent
        thefields.append(QgsField(self.CLIMBATTRIBUTE, QMetaType.Type.Double))
        thefields.append(QgsField(self.DESCENTATTRIBUTE, QMetaType.Type.Double))
        thefields.append(QgsField(self.MINELEVATTRIBUTE, QMetaType.Type.Double))
        thefields.append(QgsField(self.MAXELEVATTRIBUTE, QMetaType.Type.Double))

        # combine all the vector fields
        out_fields = QgsProcessingUtils.combineFields(thefields, source_fields)

        layerwithz = source

        (sink, dest_id) = self.parameterAsSink(
            parameters,
            self.OUTPUT,
            context,
            out_fields,
            layerwithz.wkbType(),
            source.sourceCrs(),
        )

        # get features from source (with z values)
        features = layerwithz.getFeatures()
        totalclimb = 0
        totaldescent = 0
        minelevation = float("Infinity")
        maxelevation = float("-Infinity")

        no_z_nodes = []
        no_geometry = []

        for current, feature in enumerate(features):
            if feedback.isCanceled():
                break
            climb = 0
            descent = 0
            minelev = float("Infinity")
            maxelev = float("-Infinity")
            # In case of multigeometries we need to do the parts
            parts = feature.geometry().constParts()
            if not feature.hasGeometry():
                no_geometry.append(self.tr(f"Feature: {feature.id()}"))
            for partnumber, part in enumerate(parts):
                # Calculate the climb
                first = True
                zval = 0
                for idx, v in enumerate(part.vertices()):
                    zval = v.z()
                    if math.isnan(zval):
                        no_z_nodes.append(
                            self.tr(
                                "Feature: {feature_id}, part: {part_id}, point: {point_id}".format(
                                    feature_id=feature.id(),
                                    part_id=partnumber,
                                    point_id=idx,
                                )
                            )
                        )
                        continue
                    if first:
                        prevz = zval
                        minelev = zval
                        maxelev = zval
                        first = False
                    else:
                        diff = zval - prevz
                        if diff > 0:
                            climb += diff
                        else:
                            descent -= diff
                        minelev = min(minelev, zval)
                        maxelev = max(maxelev, zval)
                    prevz = zval
                totalclimb += climb
                totaldescent += descent
            # Set the attribute values
            # Append the attributes to the end of the existing ones
            attrs = [climb, descent, minelev, maxelev, *feature.attributes()]
            # Set the final attribute list
            feature.setAttributes(attrs)
            # Add a feature to the sink
            sink.addFeature(feature, QgsFeatureSink.Flag.FastInsert)
            minelevation = min(minelevation, minelev)
            maxelevation = max(maxelevation, maxelev)
            # Update the progress bar
            if fcount > 0:
                feedback.setProgress(int(100 * current / fcount))

        feedback.pushInfo(
            self.tr(
                "The following features do not have geometry: {no_geometry_report}".format(
                    no_geometry_report=(", ".join(no_geometry))
                )
            )
        )

        feedback.pushInfo(
            self.tr(
                "The following points do not have Z values: {no_z_report}".format(
                    no_z_report=(", ".join(no_z_nodes))
                )
            )
        )

        sink.finalize()

        # Return the results
        return {
            self.OUTPUT: dest_id,
            self.TOTALCLIMB: totalclimb,
            self.TOTALDESCENT: totaldescent,
            self.MINELEVATION: minelevation,
            self.MAXELEVATION: maxelevation,
        }
