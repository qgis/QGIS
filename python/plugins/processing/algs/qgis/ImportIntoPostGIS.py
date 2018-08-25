# -*- coding: utf-8 -*-

"""
***************************************************************************
    ImportIntoPostGIS.py
    ---------------------
    Date                 : October 2012
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
__date__ = 'October 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsVectorLayerExporter,
                       QgsSettings,
                       QgsFeatureSink,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterString,
                       QgsProcessingParameterField,
                       QgsProcessingParameterBoolean,
                       QgsWkbTypes)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import postgis


class ImportIntoPostGIS(QgisAlgorithm):
    DATABASE = 'DATABASE'
    TABLENAME = 'TABLENAME'
    SCHEMA = 'SCHEMA'
    INPUT = 'INPUT'
    OVERWRITE = 'OVERWRITE'
    CREATEINDEX = 'CREATEINDEX'
    GEOMETRY_COLUMN = 'GEOMETRY_COLUMN'
    LOWERCASE_NAMES = 'LOWERCASE_NAMES'
    DROP_STRING_LENGTH = 'DROP_STRING_LENGTH'
    FORCE_SINGLEPART = 'FORCE_SINGLEPART'
    PRIMARY_KEY = 'PRIMARY_KEY'
    ENCODING = 'ENCODING'

    def group(self):
        return self.tr('Database')

    def groupId(self):
        return 'database'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Layer to import'),
                                                              types=[QgsProcessing.TypeVector]))

        db_param = QgsProcessingParameterString(
            self.DATABASE,
            self.tr('Database (connection name)'))
        db_param.setMetadata({
            'widget_wrapper': {
                'class': 'processing.gui.wrappers_postgis.ConnectionWidgetWrapper'}})
        self.addParameter(db_param)

        schema_param = QgsProcessingParameterString(
            self.SCHEMA,
            self.tr('Schema (schema name)'), 'public', False, True)
        schema_param.setMetadata({
            'widget_wrapper': {
                'class': 'processing.gui.wrappers_postgis.SchemaWidgetWrapper',
                'connection_param': self.DATABASE}})
        self.addParameter(schema_param)

        table_param = QgsProcessingParameterString(
            self.TABLENAME,
            self.tr('Table to import to (leave blank to use layer name)'), '', False, True)
        table_param.setMetadata({
            'widget_wrapper': {
                'class': 'processing.gui.wrappers_postgis.TableWidgetWrapper',
                'schema_param': self.SCHEMA}})
        self.addParameter(table_param)

        self.addParameter(QgsProcessingParameterField(self.PRIMARY_KEY,
                                                      self.tr('Primary key field'), None, self.INPUT,
                                                      QgsProcessingParameterField.Any, False, True))
        self.addParameter(QgsProcessingParameterString(self.GEOMETRY_COLUMN,
                                                       self.tr('Geometry column'), 'geom'))
        self.addParameter(QgsProcessingParameterString(self.ENCODING,
                                                       self.tr('Encoding'), 'UTF-8',
                                                       False, True))
        self.addParameter(QgsProcessingParameterBoolean(self.OVERWRITE,
                                                        self.tr('Overwrite'), True))
        self.addParameter(QgsProcessingParameterBoolean(self.CREATEINDEX,
                                                        self.tr('Create spatial index'), True))
        self.addParameter(QgsProcessingParameterBoolean(self.LOWERCASE_NAMES,
                                                        self.tr('Convert field names to lowercase'), True))
        self.addParameter(QgsProcessingParameterBoolean(self.DROP_STRING_LENGTH,
                                                        self.tr('Drop length constraints on character fields'), False))
        self.addParameter(QgsProcessingParameterBoolean(self.FORCE_SINGLEPART,
                                                        self.tr('Create single-part geometries instead of multi-part'),
                                                        False))

    def name(self):
        return 'importintopostgis'

    def displayName(self):
        return self.tr('Import into PostGIS')

    def processAlgorithm(self, parameters, context, feedback):
        connection = self.parameterAsString(parameters, self.DATABASE, context)
        db = postgis.GeoDB.from_name(connection)

        schema = self.parameterAsString(parameters, self.SCHEMA, context)
        overwrite = self.parameterAsBool(parameters, self.OVERWRITE, context)
        createIndex = self.parameterAsBool(parameters, self.CREATEINDEX, context)
        convertLowerCase = self.parameterAsBool(parameters, self.LOWERCASE_NAMES, context)
        dropStringLength = self.parameterAsBool(parameters, self.DROP_STRING_LENGTH, context)
        forceSinglePart = self.parameterAsBool(parameters, self.FORCE_SINGLEPART, context)
        primaryKeyField = self.parameterAsString(parameters, self.PRIMARY_KEY, context) or 'id'
        encoding = self.parameterAsString(parameters, self.ENCODING, context)

        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        table = self.parameterAsString(parameters, self.TABLENAME, context)
        if table:
            table.strip()
        if not table or table == '':
            table = source.sourceName()
            table = table.replace('.', '_')
        table = table.replace(' ', '').lower()[0:62]
        providerName = 'postgres'

        geomColumn = self.parameterAsString(parameters, self.GEOMETRY_COLUMN, context)
        if not geomColumn:
            geomColumn = 'geom'

        options = {}
        if overwrite:
            options['overwrite'] = True
        if convertLowerCase:
            options['lowercaseFieldNames'] = True
            geomColumn = geomColumn.lower()
        if dropStringLength:
            options['dropStringConstraints'] = True
        if forceSinglePart:
            options['forceSinglePartGeometryType'] = True

        # Clear geometry column for non-geometry tables
        if source.wkbType() == QgsWkbTypes.NoGeometry:
            geomColumn = None

        uri = db.uri
        uri.setDataSource(schema, table, geomColumn, '', primaryKeyField)

        if encoding:
            options['fileEncoding'] = encoding

        exporter = QgsVectorLayerExporter(uri.uri(), providerName, source.fields(),
                                          source.wkbType(), source.sourceCrs(), overwrite, options)

        if exporter.errorCode() != QgsVectorLayerExporter.NoError:
            raise QgsProcessingException(
                self.tr('Error importing to PostGIS\n{0}').format(exporter.errorMessage()))

        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0
        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            if not exporter.addFeature(f, QgsFeatureSink.FastInsert):
                feedback.reportError(exporter.errorMessage())

            feedback.setProgress(int(current * total))

        exporter.flushBuffer()
        if exporter.errorCode() != QgsVectorLayerExporter.NoError:
            raise QgsProcessingException(
                self.tr('Error importing to PostGIS\n{0}').format(exporter.errorMessage()))

        if geomColumn and createIndex:
            db.create_spatial_index(table, schema, geomColumn)

        db.vacuum_analyze(table, schema)

        return {}

    def dbConnectionNames(self):
        settings = QgsSettings()
        settings.beginGroup('/PostgreSQL/connections/')
        return settings.childGroups()
