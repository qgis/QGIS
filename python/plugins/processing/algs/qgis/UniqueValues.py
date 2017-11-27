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
from builtins import str

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import codecs

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsCoordinateReferenceSystem,
                       QgsWkbTypes,
                       QgsFeature,
                       QgsFeatureSink,
                       QgsFields,
                       QgsProcessingParameterField,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingOutputNumber,
                       QgsProcessingOutputString,
                       QgsProcessingParameterFileDestination,
                       QgsProcessingOutputHtml)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class UniqueValues(QgisAlgorithm):

    INPUT = 'INPUT'
    FIELD_NAME = 'FIELD_NAME'
    TOTAL_VALUES = 'TOTAL_VALUES'
    UNIQUE_VALUES = 'UNIQUE_VALUES'
    OUTPUT = 'OUTPUT'
    OUTPUT_HTML_FILE = 'OUTPUT_HTML_FILE'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'unique.png'))

    def group(self):
        return self.tr('Vector analysis')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterField(self.FIELD_NAME,
                                                      self.tr('Target field'),
                                                      parentLayerParameterName=self.INPUT, type=QgsProcessingParameterField.Any))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Unique values'), optional=True, defaultValue=''))

        self.addParameter(QgsProcessingParameterFileDestination(self.OUTPUT_HTML_FILE, self.tr('HTML report'), self.tr('HTML files (*.html)'), None, True))
        self.addOutput(QgsProcessingOutputHtml(self.OUTPUT_HTML_FILE, self.tr('HTML report')))
        self.addOutput(QgsProcessingOutputNumber(self.TOTAL_VALUES, self.tr('Total unique values')))
        self.addOutput(QgsProcessingOutputString(self.UNIQUE_VALUES, self.tr('Unique values')))

    def name(self):
        return 'listuniquevalues'

    def displayName(self):
        return self.tr('List unique values')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        field_name = self.parameterAsString(parameters, self.FIELD_NAME, context)
        values = source.uniqueValues(source.fields().lookupField(field_name))

        fields = QgsFields()
        field = source.fields()[source.fields().lookupField(field_name)]
        field.setName('VALUES')
        fields.append(field)
        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.NoGeometry, QgsCoordinateReferenceSystem())
        results = {}
        if sink:
            for value in values:
                if feedback.isCanceled():
                    break

                f = QgsFeature()
                f.setAttributes([value])
                sink.addFeature(f, QgsFeatureSink.FastInsert)
            results[self.OUTPUT] = dest_id

        output_file = self.parameterAsFileOutput(parameters, self.OUTPUT_HTML_FILE, context)
        if output_file:
            self.createHTML(output_file, values)
            results[self.OUTPUT_HTML_FILE] = output_file

        results[self.TOTAL_VALUES] = len(values)
        results[self.UNIQUE_VALUES] = ';'.join([str(v) for v in
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
                f.write('<li>' + str(s) + '</li>')
            f.write('</ul></body></html>')
