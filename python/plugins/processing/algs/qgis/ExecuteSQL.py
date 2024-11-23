"""
***************************************************************************
    ExecuteSQL.py -- use virtual layers to execute SQL on any sources
    ---------------------
    Date                 : Jan 2016
    Copyright            : (C) 2016 by Hugo Mercier
    Email                : hugo dot mercier at oslandia dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Hugo Mercier'
__date__ = 'January 2016'
__copyright__ = '(C) 2016, Hugo Mercier'

from qgis.core import (Qgis,
                       QgsVirtualLayerDefinition,
                       QgsVectorLayer,
                       QgsWkbTypes,
                       QgsProcessingAlgorithm,
                       QgsProcessingParameterMultipleLayers,
                       QgsProcessingParameterDefinition,
                       QgsExpression,
                       QgsProcessingUtils,
                       QgsProcessingParameterString,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterFeatureSink,
                       QgsFeatureSink,
                       QgsProcessingException,
                       QgsVectorFileWriter,
                       QgsProject)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class ParameterExecuteSql(QgsProcessingParameterDefinition):

    def __init__(self, name='', description=''):
        super().__init__(name, description)
        self.setMetadata({
            'widget_wrapper': 'processing.algs.qgis.ui.ExecuteSQLWidget.ExecuteSQLWidgetWrapper'
        })

    def type(self):
        return 'execute_sql'

    def clone(self):
        return ParameterExecuteSql(self.name(), self.description())


class ExecuteSQL(QgisAlgorithm):
    """ This algorithm allows executing an SQL query on a set of input
    vector layers thanks to the virtual layer provider
    """

    INPUT_DATASOURCES = 'INPUT_DATASOURCES'
    INPUT_QUERY = 'INPUT_QUERY'
    INPUT_UID_FIELD = 'INPUT_UID_FIELD'
    INPUT_GEOMETRY_FIELD = 'INPUT_GEOMETRY_FIELD'
    INPUT_GEOMETRY_TYPE = 'INPUT_GEOMETRY_TYPE'
    INPUT_GEOMETRY_CRS = 'INPUT_GEOMETRY_CRS'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector general')

    def groupId(self):
        return 'vectorgeneral'

    def __init__(self):
        super().__init__()

        self.geometry_types = [
            (None, self.tr('Autodetect')),
            (Qgis.WkbType.NoGeometry, self.tr('No geometry')),
            (Qgis.WkbType.Point, self.tr('Point')),
            (Qgis.WkbType.LineString, self.tr('LineString')),
            (Qgis.WkbType.Polygon, self.tr('Polygon')),
            (Qgis.WkbType.MultiPoint, self.tr('MultiPoint')),
            (Qgis.WkbType.MultiLineString, self.tr('MultiLineString')),
            (Qgis.WkbType.MultiPolygon, self.tr('MultiPolygon'))]

    def flags(self):
        return super().flags() | QgsProcessingAlgorithm.Flag.FlagNoThreading

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterMultipleLayers(name=self.INPUT_DATASOURCES,
                                                               description=self.tr('Input data sources (called input1, .., inputN in the query)'),
                                                               optional=True))

        self.addParameter(ParameterExecuteSql(name=self.INPUT_QUERY, description=self.tr('SQL query')))

        self.addParameter(QgsProcessingParameterString(name=self.INPUT_UID_FIELD,
                                                       description=self.tr('Unique identifier field'), optional=True))

        self.addParameter(QgsProcessingParameterString(name=self.INPUT_GEOMETRY_FIELD,
                                                       description=self.tr('Geometry field'), optional=True))

        self.addParameter(QgsProcessingParameterEnum(self.INPUT_GEOMETRY_TYPE,
                                                     self.tr('Geometry type'),
                                                     options=[t[1] for t in self.geometry_types],
                                                     defaultValue=0))

        self.addParameter(QgsProcessingParameterCrs(self.INPUT_GEOMETRY_CRS,
                                                    self.tr('CRS'), optional=True))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('SQL Output')))

    def name(self):
        return 'executesql'

    def displayName(self):
        return self.tr('Execute SQL')

    def processAlgorithm(self, parameters, context, feedback):
        layers = self.parameterAsLayerList(parameters, self.INPUT_DATASOURCES, context)
        query = self.parameterAsString(parameters, self.INPUT_QUERY, context)
        uid_field = self.parameterAsString(parameters, self.INPUT_UID_FIELD, context)
        geometry_field = self.parameterAsString(parameters, self.INPUT_GEOMETRY_FIELD, context)
        geometry_type = self.geometry_types[
            self.parameterAsEnum(parameters, self.INPUT_GEOMETRY_TYPE, context)
        ][0]
        geometry_crs = self.parameterAsCrs(parameters, self.INPUT_GEOMETRY_CRS, context)

        df = QgsVirtualLayerDefinition()
        for layerIdx, layer in enumerate(layers):

            # Issue https://github.com/qgis/QGIS/issues/24041
            # When using this algorithm from the graphic modeler, it may try to
            # access (thanks the QgsVirtualLayerProvider) to memory layer that
            # belongs to temporary QgsMapLayerStore, not project.
            # So, we write them to disk is this is the case.
            if context.project() and not context.project().mapLayer(layer.id()):
                basename = "memorylayer." + QgsVectorFileWriter.supportedFormatExtensions()[0]
                tmp_path = QgsProcessingUtils.generateTempFilename(basename, context)
                QgsVectorFileWriter.writeAsVectorFormat(
                    layer, tmp_path, layer.dataProvider().encoding())
                df.addSource(f'input{layerIdx + 1}', tmp_path, "ogr")
            else:
                df.addSource(f'input{layerIdx + 1}', layer.id())

        if query == '':
            raise QgsProcessingException(
                self.tr('Empty SQL. Please enter valid SQL expression and try again.'))
        localContext = self.createExpressionContext(parameters, context)
        expandedQuery = QgsExpression.replaceExpressionText(query, localContext)
        df.setQuery(expandedQuery)

        if uid_field:
            df.setUid(uid_field)

        if geometry_type == Qgis.WkbType.NoGeometry:
            df.setGeometryWkbType(Qgis.WkbType.NoGeometry)
        else:
            if geometry_field:
                df.setGeometryField(geometry_field)
            if geometry_type is not None:
                df.setGeometryWkbType(geometry_type)
            if geometry_crs.isValid():
                df.setGeometrySrid(geometry_crs.postgisSrid())

        vLayer = QgsVectorLayer(df.toString(), "temp_vlayer", "virtual")
        if not vLayer.isValid():
            raise QgsProcessingException(vLayer.dataProvider().error().message())

        if vLayer.wkbType() == QgsWkbTypes.Type.Unknown:
            raise QgsProcessingException(self.tr("Cannot find geometry field"))

        (sink, dest_id) = self.parameterAsSink(parameters,
                                               self.OUTPUT,
                                               context,
                                               vLayer.fields(),
                                               vLayer.wkbType(),
                                               vLayer.crs())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        features = vLayer.getFeatures()
        total = 100.0 / vLayer.featureCount() if vLayer.featureCount() else 0
        for current, inFeat in enumerate(features):
            if feedback.isCanceled():
                break

            sink.addFeature(inFeat, QgsFeatureSink.Flag.FastInsert)
            feedback.setProgress(int(current * total))
        sink.finalize()
        return {self.OUTPUT: dest_id}
