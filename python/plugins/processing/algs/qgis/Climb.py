# -*- coding: utf-8 -*-

"""
/***************************************************************************
 Climb
                                 A QGIS plugin

                              -------------------
        begin                : 2019-03-01
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

__author__ = 'Håvard Tveite'
__date__ = '2019-03-01'
__copyright__ = '(C) 2019 by Håvard Tveite'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import math

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant

from qgis.core import (QgsProcessing,
                       QgsFeatureSink,
                       QgsProcessingAlgorithm,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingOutputNumber,
                       QgsWkbTypes,
                       QgsFields,
                       QgsField)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class Climb(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    TOTALCLIMB = 'TOTALCLIMB'
    TOTALDESCENT = 'TOTALDESCENT'
    MINELEVATION = 'MINELEVATION'
    MAXELEVATION = 'MAXELEVATION'
    CLIMBATTRIBUTE = 'climb'
    DESCENTATTRIBUTE = 'descent'
    MINELEVATTRIBUTE = 'minelev'
    MAXELEVATTRIBUTE = 'maxelev'

    def name(self):
        return 'climbalongline'

    def displayName(self):
        return self.tr('Climb Along Line')

    def group(self):
        return self.tr('Vector analysis')

    def groupId(self):
        return 'vectoranalysis'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(
            QgsProcessingParameterFeatureSource(
                self.INPUT,
                self.tr('Input (line) layer'),
                [QgsProcessing.TypeVectorLine]
            )
        )

        self.addParameter(
            QgsProcessingParameterFeatureSink(
                self.OUTPUT,
                self.tr('Climb layer')
            )
        )

        self.addOutput(
            QgsProcessingOutputNumber(
                self.TOTALCLIMB,
                self.tr('Total climb')
            )
        )

        self.addOutput(
            QgsProcessingOutputNumber(
                self.TOTALDESCENT,
                self.tr('Total descent')
            )
        )

        self.addOutput(
            QgsProcessingOutputNumber(
                self.MINELEVATION,
                self.tr('Minimum elevation')
            )
        )

        self.addOutput(
            QgsProcessingOutputNumber(
                self.MAXELEVATION,
                self.tr('Maximum elevation')
            )
        )

    def processAlgorithm(self, parameters, context, feedback):

        source = self.parameterAsSource(
            parameters,
            self.INPUT,
            context
        )

        fcount = source.featureCount()

        hasZ = QgsWkbTypes.hasZ(source.wkbType())

        if not hasZ:
            feedback.reportError(self.tr('The layer does not have Z values. If you have a DEM, use the Drape algorithm to extract Z values.'))
            return

        thefields = QgsFields()
        climbindex = -1
        descentindex = -1
        minelevindex = -1
        maxelevindex = -1
        fieldnumber = 0

        # Skip fields with names that are equal to the generated ones
        for field in source.fields():
            if field.name() == self.CLIMBATTRIBUTE:
                feedback.pushInfo(self.tr(
                    '{attr_name} attribute found and removed'.format(
                        attr_name=self.CLIMBATTRIBUTE)
                )
                )
                climbindex = fieldnumber
            elif field.name() == self.DESCENTATTRIBUTE:
                feedback.pushInfo(self.tr(
                    '{attr_name} attribute found and removed'.format(
                        attr_name=self.DESCENTATTRIBUTE)
                )
                )
                descentindex = fieldnumber
            elif field.name() == self.MINELEVATTRIBUTE:
                feedback.pushInfo(self.tr(
                    '{attr_name} attribute found and removed'.format(
                        attr_name=self.MINELEVATTRIBUTE)
                )
                )
                minelevindex = fieldnumber
            elif field.name() == self.MAXELEVATTRIBUTE:
                feedback.pushInfo(self.tr(
                    '{attr_name} attribute found and removed'.format(
                        attr_name=self.MAXELEVATTRIBUTE)
                )
                )
                maxelevindex = fieldnumber
            else:
                thefields.append(field)
            fieldnumber = fieldnumber + 1

        # Create new fields for climb and descent
        thefields.append(QgsField(self.CLIMBATTRIBUTE, QVariant.Double))
        thefields.append(QgsField(self.DESCENTATTRIBUTE, QVariant.Double))
        thefields.append(QgsField(self.MINELEVATTRIBUTE, QVariant.Double))
        thefields.append(QgsField(self.MAXELEVATTRIBUTE, QVariant.Double))

        layerwithz = source

        (sink, dest_id) = self.parameterAsSink(parameters,
                                               self.OUTPUT,
                                               context, thefields,
                                               layerwithz.wkbType(),
                                               source.sourceCrs())

        # get features from source (with z values)
        features = layerwithz.getFeatures()
        totalclimb = 0
        totaldescent = 0
        minelevation = float('Infinity')
        maxelevation = float('-Infinity')

        no_z_nodes = []

        for current, feature in enumerate(features):
            if feedback.isCanceled():
                break
            climb = 0
            descent = 0
            minelev = float('Infinity')
            maxelev = float('-Infinity')
            # In case of multigeometries we need to do the parts
            parts = feature.geometry().constParts()
            partnumber = 0
            for part in parts:
                # Calculate the climb
                first = True
                zval = 0
                for idx, v in enumerate(part.vertices()):
                    zval = v.z()
                    if math.isnan(zval):
                        no_z_nodes.append(self.tr(
                            'Feature: {feature_id}, part: {part_id}, point: {point_id}'.format(
                                feature_id=feature.id(),
                                part_id=partnumber,
                                point_id=idx)
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
                            climb = climb + diff
                        else:
                            descent = descent - diff
                        if minelev > zval:
                            minelev = zval
                        if maxelev < zval:
                            maxelev = zval
                    prevz = zval
                totalclimb = totalclimb + climb
                totaldescent = totaldescent + descent
                partnumber += 1
            # Set the attribute values
            attrs = feature.attributes()
            outattrs = []
            attrindex = 0
            for attr in attrs:
                # Skip attributes from the input layer that had names
                # that were equal to the generated ones
                if not (attrindex == climbindex or
                        attrindex == descentindex or
                        attrindex == minelevindex or
                        attrindex == maxelevindex):
                    outattrs.append(attr)
                attrindex = attrindex + 1
            # feature.setAttributes(outattrs + [climb, descent])
            feature.setAttributes(outattrs +
                                  [climb, descent, minelev, maxelev])
            # Add a feature to the sink
            sink.addFeature(feature, QgsFeatureSink.FastInsert)
            if minelevation > minelev:
                minelevation = minelev
            if maxelevation < maxelev:
                maxelevation = maxelev
            # Update the progress bar
            if fcount > 0:
                feedback.setProgress(int(100 * current / fcount))

        feedback.pushInfo(self.tr('Following nodes miss Z value ') + str(no_z_nodes))
        # Return the results
        return {self.OUTPUT: dest_id, self.TOTALCLIMB: totalclimb,
                self.TOTALDESCENT: totaldescent,
                self.MINELEVATION: minelevation,
                self.MAXELEVATION: maxelevation}
