# -*- coding: utf-8 -*-

"""
***************************************************************************
    ImportIntoSpatialite.py
    ---------------------
    Date                 : October 2016
    Copyright            : (C) 2016 by Mathieu Pellerin
    Email                : nirvn dot asia at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Mathieu Pellerin'
__date__ = 'October 2016'
__copyright__ = '(C) 2012, Mathieu Pellerin'

from qgis.core import (QgsDataSourceUri,
                       QgsFeatureSink,
                       QgsProcessingAlgorithm,
                       QgsVectorLayerExporter,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterVectorLayer,
                       QgsProcessingParameterField,
                       QgsProcessingParameterString,
                       QgsProcessingParameterBoolean,
                       QgsWkbTypes,
                       QgsProviderRegistry,
                       QgsProviderConnectionException,
                       QgsAbstractDatabaseProviderConnection)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class ImportIntoSpatialite(QgisAlgorithm):
    DATABASE = 'DATABASE'
    TABLENAME = 'TABLENAME'
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
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT, self.tr('Layer to import'),
                                                              types=[QgsProcessing.TypeVector]))
        self.addParameter(QgsProcessingParameterVectorLayer(self.DATABASE, self.tr('File database'), optional=False))
        self.addParameter(
            QgsProcessingParameterString(self.TABLENAME, self.tr('Table to import to (leave blank to use layer name)'),
                                         optional=True))
        self.addParameter(QgsProcessingParameterField(self.PRIMARY_KEY, self.tr('Primary key field'), None, self.INPUT,
                                                      QgsProcessingParameterField.Any, False, True))
        self.addParameter(QgsProcessingParameterString(self.GEOMETRY_COLUMN, self.tr('Geometry column'), 'geom'))
        self.addParameter(QgsProcessingParameterString(self.ENCODING, self.tr('Encoding'), 'UTF-8', optional=True))
        self.addParameter(QgsProcessingParameterBoolean(self.OVERWRITE, self.tr('Overwrite'), True))
        self.addParameter(QgsProcessingParameterBoolean(self.CREATEINDEX, self.tr('Create spatial index'), True))
        self.addParameter(
            QgsProcessingParameterBoolean(self.LOWERCASE_NAMES, self.tr('Convert field names to lowercase'), True))
        self.addParameter(QgsProcessingParameterBoolean(self.DROP_STRING_LENGTH,
                                                        self.tr('Drop length constraints on character fields'), False))
        self.addParameter(QgsProcessingParameterBoolean(self.FORCE_SINGLEPART,
                                                        self.tr('Create single-part geometries instead of multi-part'),
                                                        False))

    def flags(self):
        return super().flags() | QgsProcessingAlgorithm.FlagNoThreading

    def name(self):
        return 'importintospatialite'

    def displayName(self):
        return self.tr('Export to SpatiaLite')

    def shortDescription(self):
        return self.tr('Exports a vector layer to a SpatiaLite database')

    def tags(self):
        return self.tr('import,table,layer,into,copy').split(',')

    def processAlgorithm(self, parameters, context, feedback):
        database = self.parameterAsVectorLayer(parameters, self.DATABASE, context)

        databaseuri = database.dataProvider().dataSourceUri()
        uri = QgsDataSourceUri(databaseuri)
        if uri.database() == '':
            if '|layername' in databaseuri:
                databaseuri = databaseuri[:databaseuri.find('|layername')]
            elif '|layerid' in databaseuri:
                databaseuri = databaseuri[:databaseuri.find('|layerid')]
            uri = QgsDataSourceUri('dbname=\'%s\'' % (databaseuri))

        try:
            md = QgsProviderRegistry.instance().providerMetadata('spatialite')
            conn = md.createConnection(uri.uri(), {})
        except QgsProviderConnectionException:
            raise QgsProcessingException(self.tr('Could not connect to {}').format(uri.uri()))

        overwrite = self.parameterAsBoolean(parameters, self.OVERWRITE, context)
        createIndex = self.parameterAsBoolean(parameters, self.CREATEINDEX, context)
        convertLowerCase = self.parameterAsBoolean(parameters, self.LOWERCASE_NAMES, context)
        dropStringLength = self.parameterAsBoolean(parameters, self.DROP_STRING_LENGTH, context)
        forceSinglePart = self.parameterAsBoolean(parameters, self.FORCE_SINGLEPART, context)
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
        table = table.replace(' ', '').lower()
        providerName = 'spatialite'

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

        uri.setDataSource('', table, geomColumn, '', primaryKeyField)

        if encoding:
            options['fileEncoding'] = encoding

        exporter = QgsVectorLayerExporter(uri.uri(), providerName, source.fields(),
                                          source.wkbType(), source.sourceCrs(), overwrite, options)

        if exporter.errorCode() != QgsVectorLayerExporter.NoError:
            raise QgsProcessingException(
                self.tr('Error importing to Spatialite\n{0}').format(exporter.errorMessage()))

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
                self.tr('Error importing to Spatialite\n{0}').format(exporter.errorMessage()))

        if geomColumn and createIndex:
            try:
                options = QgsAbstractDatabaseProviderConnection.SpatialIndexOptions()
                options.geometryColumnName = geomColumn
                conn.createSpatialIndex('', table, options)
            except QgsProviderConnectionException as e:
                raise QgsProcessingException(self.tr('Error creating spatial index:\n{0}').format(e))

        return {}
