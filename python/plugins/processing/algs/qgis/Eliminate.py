# -*- coding: utf-8 -*-

"""
***************************************************************************
    Eliminate.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2013 by Bernhard Str�bl
    Email                : bernhard.stroebl@jena.de
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Bernhard Ströbl'
__date__ = 'September 2013'
__copyright__ = '(C) 2013, Bernhard Ströbl'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QLocale, QDate

from qgis.core import QgsFeatureRequest, QgsFeature, QgsGeometry

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.ProcessingLog import ProcessingLog
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterTableField
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class Eliminate(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    MODE = 'MODE'
    KEEPSELECTION = 'KEEPSELECTION'
    ATTRIBUTE = 'ATTRIBUTE'
    COMPARISONVALUE = 'COMPARISONVALUE'
    COMPARISON = 'COMPARISON'

    MODE_LARGEST_AREA = 0
    MODE_SMALLEST_AREA = 1
    MODE_BOUNDARY = 2

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'eliminate.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Eliminate sliver polygons')
        self.group, self.i18n_group = self.trAlgorithm('Vector geometry tools')

        self.modes = [self.tr('Largest area'),
                      self.tr('Smallest Area'),
                      self.tr('Largest common boundary')]

        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_POLYGON]))
        self.addParameter(ParameterBoolean(self.KEEPSELECTION,
                                           self.tr('Use current selection in input layer (works only if called from toolbox)'), False))
        self.addParameter(ParameterTableField(self.ATTRIBUTE,
                                              self.tr('Selection attribute'), self.INPUT))
        self.comparisons = [
            '==',
            '!=',
            '>',
            '>=',
            '<',
            '<=',
            'begins with',
            'contains',
        ]
        self.addParameter(ParameterSelection(self.COMPARISON,
                                             self.tr('Comparison'), self.comparisons, default=0))
        self.addParameter(ParameterString(self.COMPARISONVALUE,
                                          self.tr('Value'), default='0'))
        self.addParameter(ParameterSelection(self.MODE,
                                             self.tr('Merge selection with the neighbouring polygon with the'),
                                             self.modes))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Cleaned')))

    def processAlgorithm(self, progress):
        inLayer = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT))
        boundary = self.getParameterValue(self.MODE) == self.MODE_BOUNDARY
        smallestArea = self.getParameterValue(self.MODE) == self.MODE_SMALLEST_AREA
        keepSelection = self.getParameterValue(self.KEEPSELECTION)
        processLayer = vector.duplicateInMemory(inLayer)

        if not keepSelection:
            # Make a selection with the values provided
            attribute = self.getParameterValue(self.ATTRIBUTE)
            comparison = self.comparisons[self.getParameterValue(self.COMPARISON)]
            comparisonvalue = self.getParameterValue(self.COMPARISONVALUE)

            selectindex = vector.resolveFieldIndex(processLayer, attribute)
            selectType = processLayer.fields()[selectindex].type()
            selectionError = False

            if selectType == 2:
                try:
                    y = int(comparisonvalue)
                except ValueError:
                    selectionError = True
                    msg = self.tr('Cannot convert "%s" to integer' % unicode(comparisonvalue))
            elif selectType == 6:
                try:
                    y = float(comparisonvalue)
                except ValueError:
                    selectionError = True
                    msg = self.tr('Cannot convert "%s" to float' % unicode(comparisonvalue))
            elif selectType == 10:
                # 10: string, boolean
                try:
                    y = unicode(comparisonvalue)
                except ValueError:
                    selectionError = True
                    msg = self.tr('Cannot convert "%s" to unicode' % unicode(comparisonvalue))
            elif selectType == 14:
                # date
                dateAndFormat = comparisonvalue.split(' ')

                if len(dateAndFormat) == 1:
                    # QDate object
                    y = QLocale.system().toDate(dateAndFormat[0])

                    if y.isNull():
                        msg = self.tr('Cannot convert "%s" to date with system date format %s' % (unicode(dateAndFormat), QLocale.system().dateFormat()))
                elif len(dateAndFormat) == 2:
                    y = QDate.fromString(dateAndFormat[0], dateAndFormat[1])

                    if y.isNull():
                        msg = self.tr('Cannot convert "%s" to date with format string "%s"' % (unicode(dateAndFormat[0]), dateAndFormat[1]))
                else:
                    y = QDate()
                    msg = ''

                if y.isNull():
                    # Conversion was unsuccessfull
                    selectionError = True
                    msg += self.tr('Enter the date and the date format, e.g. "07.26.2011" "MM.dd.yyyy".')

            if (comparison == 'begins with' or comparison == 'contains') \
               and selectType != 10:
                selectionError = True
                msg = self.tr('"%s" can only be used with string fields' % comparison)

            selected = []

            if selectionError:
                raise GeoAlgorithmExecutionException(
                    self.tr('Error in selection input: %s' % msg))
            else:
                for feature in processLayer.getFeatures():
                    aValue = feature.attributes()[selectindex]

                    if aValue is None:
                        continue

                    if selectType == 2:
                        x = int(aValue)
                    elif selectType == 6:
                        x = float(aValue)
                    elif selectType == 10:
                        # 10: string, boolean
                        x = unicode(aValue)
                    elif selectType == 14:
                        # date
                        x = aValue  # should be date

                    match = False

                    if comparison == '==':
                        match = x == y
                    elif comparison == '!=':
                        match = x != y
                    elif comparison == '>':
                        match = x > y
                    elif comparison == '>=':
                        match = x >= y
                    elif comparison == '<':
                        match = x < y
                    elif comparison == '<=':
                        match = x <= y
                    elif comparison == 'begins with':
                        match = x.startswith(y)
                    elif comparison == 'contains':
                        match = x.find(y) >= 0

                    if match:
                        selected.append(feature.id())

            processLayer.setSelectedFeatures(selected)

        if processLayer.selectedFeatureCount() == 0:
            ProcessingLog.addToLog(ProcessingLog.LOG_WARNING,
                                   self.tr('%s: (No selection in input layer "%s")' % (self.commandLineName(), self.getParameterValue(self.INPUT))))

        # Keep references to the features to eliminate
        featToEliminate = []
        for aFeat in processLayer.selectedFeatures():
            featToEliminate.append(aFeat)

        # Delete all features to eliminate in processLayer (we won't save this)
        processLayer.startEditing()
        processLayer.deleteSelectedFeatures()

        # ANALYZE
        if len(featToEliminate) > 0:  # Prevent zero division
            start = 20.00
            add = 80.00 / len(featToEliminate)
        else:
            start = 100

        progress.setPercentage(start)
        madeProgress = True

        # We go through the list and see if we find any polygons we can
        # merge the selected with. If we have no success with some we
        # merge and then restart the whole story.
        while madeProgress:  # Check if we made any progress
            madeProgress = False
            featNotEliminated = []

            # Iterate over the polygons to eliminate
            for i in range(len(featToEliminate)):
                feat = featToEliminate.pop()
                geom2Eliminate = QgsGeometry(feat.geometry())
                bbox = geom2Eliminate.boundingBox()
                fit = processLayer.getFeatures(
                    QgsFeatureRequest().setFilterRect(bbox))
                mergeWithFid = None
                mergeWithGeom = None
                max = 0
                min = -1
                selFeat = QgsFeature()

                while fit.nextFeature(selFeat):
                    selGeom = QgsGeometry(selFeat.geometry())

                    if geom2Eliminate.intersects(selGeom):
                        # We have a candidate
                        iGeom = geom2Eliminate.intersection(selGeom)

                        if iGeom is None:
                            continue

                        if boundary:
                            selValue = iGeom.length()
                        else:
                            # area. We need a common boundary in
                            # order to merge
                            if 0 < iGeom.length():
                                selValue = selGeom.area()
                            else:
                                selValue = -1

                        if -1 != selValue:
                            useThis = True

                            if smallestArea:
                                if -1 == min:
                                    min = selValue
                                else:
                                    if selValue < min:
                                        min = selValue
                                    else:
                                        useThis = False
                            else:
                                if selValue > max:
                                    max = selValue
                                else:
                                    useThis = False

                            if useThis:
                                mergeWithFid = selFeat.id()
                                mergeWithGeom = QgsGeometry(selGeom)
                # End while fit

                if mergeWithFid is not None:
                    # A successful candidate
                    newGeom = mergeWithGeom.combine(geom2Eliminate)

                    if processLayer.changeGeometry(mergeWithFid, newGeom):
                        madeProgress = True
                    else:
                        raise GeoAlgorithmExecutionException(
                            self.tr('Could not replace geometry of feature with id %s' % mergeWithFid))

                    start = start + add
                    progress.setPercentage(start)
                else:
                    featNotEliminated.append(feat)

            # End for featToEliminate

            featToEliminate = featNotEliminated

        # End while

        # Create output
        provider = processLayer.dataProvider()
        output = self.getOutputFromName(self.OUTPUT)
        writer = output.getVectorWriter(provider.fields(),
                                        provider.geometryType(), processLayer.crs())

        # Write all features that are left over to output layer
        iterator = processLayer.getFeatures()
        for feature in iterator:
            writer.addFeature(feature)

        # Leave processLayer untouched
        processLayer.rollBack()

        for feature in featNotEliminated:
            writer.addFeature(feature)
