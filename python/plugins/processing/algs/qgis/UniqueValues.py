# -*- coding: utf-8 -*-

"""
***************************************************************************
    UniqueValues.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import codecs

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsApplication,
                       QgsCoordinateReferenceSystem,
                       QgsWkbTypes,
                       QgsFeature,
                       QgsFeatureSink,
                       QgsFeatureRequest,
                       QgsFields,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterField,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingOutputNumber,
                       QgsProcessingOutputString,
                       QgsProcessingFeatureSource,
                       QgsProcessingParameterFileDestination)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class UniqueValues(QgisAlgorithm):

    INPUT = 'INPUT'
    FIELDS = 'FIELDS'
    TOTAL_VALUES = 'TOTAL_VALUES'
    UNIQUE_VALUES = 'UNIQUE_VALUES'
    OUTPUT = 'OUTPUT'
    OUTPUT_HTML_FILE = 'OUTPUT_HTML_FILE'

    def icon(self):
        return QgsApplication.getThemeIcon("/algorithms/mAlgorithmUniqueValues.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("/algorithms/mAlgorithmUniqueValues.svg")

    def group(self):
        return self.tr('Vector analysis')

    def groupId(self):
        return 'vectoranalysis'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'), types=[QgsProcessing.TypeVector]))
        self.addParameter(QgsProcessingParameterField(self.FIELDS,
                                                      self.tr('Target field(s)'),
                                                      parentLayerParameterName=self.INPUT, type=QgsProcessingParameterField.Any, allowMultiple=True))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Unique values'), optional=True, defaultValue=''))

        self.addParameter(QgsProcessingParameterFileDestination(self.OUTPUT_HTML_FILE, self.tr('HTML report'), self.tr('HTML files (*.html)'), None, True))
        self.addOutput(QgsProcessingOutputNumber(self.TOTAL_VALUES, self.tr('Total unique values')))
        self.addOutput(QgsProcessingOutputString(self.UNIQUE_VALUES, self.tr('Unique values')))

    def name(self):
        return 'listuniquevalues'

    def displayName(self):
        return self.tr('List unique values')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        field_names = self.parameterAsFields(parameters, self.FIELDS, context)

        fields = QgsFields()
        field_indices = []
        for field_name in field_names:
            field_index = source.fields().lookupField(field_name)
            if field_index < 0:
                feedback.reportError(self.tr('Invalid field name {}').format(field_name))
                continue
            field = source.fields()[field_index]
            fields.append(field)
            field_indices.append(field_index)
        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.NoGeometry, QgsCoordinateReferenceSystem())

        results = {}
        values = set()
        if len(field_indices) == 1:
            # one field, can use provider optimised method
            values = tuple([v] for v in source.uniqueValues(field_indices[0]))
        else:
            # have to scan whole table
            # TODO - add this support to QgsVectorDataProvider so we can run it on
            # the backend
            request = QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry)
            request.setSubsetOfAttributes(field_indices)
            total = 100.0 / source.featureCount() if source.featureCount() else 0
            for current, f in enumerate(source.getFeatures(request, QgsProcessingFeatureSource.FlagSkipGeometryValidityChecks)):
                if feedback.isCanceled():
                    break

                value = tuple(f.attribute(i) for i in field_indices)
                values.add(value)
                feedback.setProgress(int(current * total))

        if sink:
            for value in values:
                if feedback.isCanceled():
                    break

                f = QgsFeature()
                f.setAttributes([attr for attr in value])
                sink.addFeature(f, QgsFeatureSink.FastInsert)
            results[self.OUTPUT] = dest_id

        output_file = self.parameterAsFileOutput(parameters, self.OUTPUT_HTML_FILE, context)
        if output_file:
            self.createHTML(output_file, values)
            results[self.OUTPUT_HTML_FILE] = output_file

        results[self.TOTAL_VALUES] = len(values)
        results[self.UNIQUE_VALUES] = ';'.join([','.join([str(attr) for attr in v]) for v in
                                                values])
        return results

    def createHTML(self, outputFile, algData):
        with codecs.open(outputFile, 'w', encoding='utf-8') as f:
            f.write('<html><head>')
            f.write('<meta http-equiv="Content-Type" content="text/html; \
                     charset=utf-8" /></head><body>')
            f.write(self.tr('<p>Total unique values: ') + str(len(algData)) + '</p>')
            f.write(self.tr('<p>Unique values:</p>'))
            f.write('<ul>')
            for s in algData:
                f.write('<li>' + ','.join([str(attr) for attr in s]) + '</li>')
            f.write('</ul></body></html>')
