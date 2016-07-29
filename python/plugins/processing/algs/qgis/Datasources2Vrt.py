# -*- coding: utf-8 -*-

"""
***************************************************************************
    __init__.py
    ---------------------
    Date                 : May 2015
    Copyright            : (C) 2015 by Luigi Pirelli
    Email                : luipir at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Luigi Pirelli'
__date__ = 'May 2015'
__copyright__ = '(C) 2015, Luigi Pirelli'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import codecs
import xml.sax.saxutils

from osgeo import ogr

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterMultipleInput
from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputFile
from processing.core.outputs import OutputString


class Datasources2Vrt(GeoAlgorithm):
    DATASOURCES = 'DATASOURCES'
    UNIONED = 'UNIONED'

    VRT_FILE = 'VRT_FILE'
    VRT_STRING = 'VRT_STRING'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Build virtual vector')
        self.group, self.i18n_group = self.trAlgorithm('Vector general tools')

        self.addParameter(ParameterMultipleInput(self.DATASOURCES,
                                                 self.tr('Input datasources'),
                                                 ParameterMultipleInput.TYPE_VECTOR_ANY))
        self.addParameter(ParameterBoolean(self.UNIONED,
                                           self.tr('Create "unioned" VRT'),
                                           default=False))

        self.addOutput(OutputFile(self.VRT_FILE,
                                  self.tr('Virtual vector'), ext='vrt'))
        self.addOutput(OutputString(self.VRT_STRING,
                                    self.tr('Virtual string')))

    def processAlgorithm(self, progress):
        input_layers = self.getParameterValue(self.DATASOURCES)
        unioned = self.getParameterValue(self.UNIONED)
        vrtPath = self.getOutputValue(self.VRT_FILE)
        vrtString = self.getOutputValue(self.VRT_STRING)

        layers = input_layers.split(';')

        vrtString = self.mergeDataSources2Vrt(layers,
                                              vrtPath,
                                              union=unioned,
                                              relative=False,
                                              schema=False,
                                              progress=progress)

        self.setOutputValue(self.VRT_STRING, vrtString)

    def mergeDataSources2Vrt(self, dataSources, outFile, union=False, relative=False,
                             schema=False, progress=None):
        '''Function to do the work of merging datasources in a single vrt format

        @param data_sources: Array of path strings
        @param outfile: the output vrt file to generate
        @param relative: Write relative flag. DOES NOT relativise paths. They have to be already relative
        @param schema: Schema flag
        @return: vrt in string format
        '''
        vrt = '<OGRVRTDataSource>'
        if union:
            vrt += '<OGRVRTUnionLayer name="UnionedLayer">'

        total = 100.0 / len(dataSources)
        for current, inFile in enumerate(dataSources):
            progress.setPercentage(int(current * total))

            srcDS = ogr.Open(inFile, 0)
            if srcDS is None:
                raise GeoAlgorithmExecutionException(
                    self.tr('Invalid datasource: {}'.format(inFile)))

            if schema:
                inFile = '@dummy@'

            for layer in srcDS:
                layerDef = layer.GetLayerDefn()
                layerName = layerDef.GetName()

                vrt += '<OGRVRTLayer name="{}">'.format(self.XmlEsc(layerName))
                vrt += '<SrcDataSource relativeToVRT="{}" shared="{}">{}</SrcDataSource>'.format(1 if relative else 0, not schema, self.XmlEsc(inFile))
                if schema:
                    vrt += '<SrcLayer>@dummy@</SrcLayer>'
                else:
                    vrt += '<SrcLayer>{}</SrcLayer>'.format(self.XmlEsc(layerName))

                vrt += '<GeometryType>{}</GeometryType>'.format(self.GeomType2Name(layerDef.GetGeomType()))

                crs = layer.GetSpatialRef()
                if crs is not None:
                    vrt += '<LayerSRS>{}</LayerSRS>'.format(self.XmlEsc(crs.ExportToWkt()))

                # Process all the fields.
                for fieldIdx in xrange(layerDef.GetFieldCount()):
                    fieldDef = layerDef.GetFieldDefn(fieldIdx)
                    vrt += '<Field name="{}" type="{}"'.format(self.XmlEsc(fieldDef.GetName()), self.fieldType2Name(fieldDef.GetType()))
                    if not schema:
                        vrt += ' src="{}"'.format(self.XmlEsc(fieldDef.GetName()))
                    if fieldDef.GetWidth() > 0:
                        vrt += ' width="{}"'.format(fieldDef.GetWidth())
                    if fieldDef.GetPrecision() > 0:
                        vrt += ' precision="{}"'.format(fieldDef.GetPrecision())
                    vrt += '/>'

                vrt += '</OGRVRTLayer>'

            srcDS.Destroy()

        if union:
            vrt += '</OGRVRTUnionLayer>'

        vrt += '</OGRVRTDataSource>'

        #TODO: pretty-print XML

        if outFile is not None:
            with codecs.open(outFile, 'w') as f:
                f.write(vrt)

        return vrt

    def GeomType2Name(self, geomType):
        if geomType == ogr.wkbUnknown:
            return 'wkbUnknown'
        elif geomType == ogr.wkbPoint:
            return 'wkbPoint'
        elif geomType == ogr.wkbLineString:
            return 'wkbLineString'
        elif geomType == ogr.wkbPolygon:
            return 'wkbPolygon'
        elif geomType == ogr.wkbMultiPoint:
            return 'wkbMultiPoint'
        elif geomType == ogr.wkbMultiLineString:
            return 'wkbMultiLineString'
        elif geomType == ogr.wkbMultiPolygon:
            return 'wkbMultiPolygon'
        elif geomType == ogr.wkbGeometryCollection:
            return 'wkbGeometryCollection'
        elif geomType == ogr.wkbNone:
            return 'wkbNone'
        elif geomType == ogr.wkbLinearRing:
            return 'wkbLinearRing'
        else:
            return 'wkbUnknown'

    def fieldType2Name(self, fieldType):
        if fieldType == ogr.OFTInteger:
            return 'Integer'
        elif fieldType == ogr.OFTString:
            return 'String'
        elif fieldType == ogr.OFTReal:
            return 'Real'
        elif fieldType == ogr.OFTStringList:
            return 'StringList'
        elif fieldType == ogr.OFTIntegerList:
            return 'IntegerList'
        elif fieldType == ogr.OFTRealList:
            return 'RealList'
        elif fieldType == ogr.OFTBinary:
            return 'Binary'
        elif fieldType == ogr.OFTDate:
            return 'Date'
        elif fieldType == ogr.OFTTime:
            return 'Time'
        elif fieldType == ogr.OFTDateTime:
            return 'DateTime'
        else:
            return 'String'

    def XmlEsc(self, text):
        return xml.sax.saxutils.escape(text)
