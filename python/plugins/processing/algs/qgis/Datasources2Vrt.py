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

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterMultipleInput, ParameterBoolean
from processing.core.outputs import OutputFile, OutputString

# algorithm specific imports
from osgeo import ogr, gdal
from string import Template
import tempfile
import logging
import os
import xml.sax.saxutils


class Datasources2Vrt(GeoAlgorithm):

    """ This algorithm merge the layers of different data sources in
    a single vrt file

    This algo is especially useful in case an algo need multiple layers
    but accept only one vrt in which the layers are specified
    """

    OUTPUT_VRT_FILE = 'OUTPUT_VRT_FILE'
    OUTPUT_VRT_STRING = 'OUTPUT_VRT_STRING'
    INPUT_DATASOURCES = 'INPUT_DATASOURCES'
    INPUT_OVERWRITE_FLAG = 'INPUT_OVERWRITE_FLAG'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Build virtual vector')
        self.group, self.i18n_group = self.trAlgorithm('Vector general tools')

        self.addParameter(ParameterMultipleInput(name=self.INPUT_DATASOURCES,
                                                 description=self.tr('Input datasources'),
                                                 datatype=ParameterMultipleInput.TYPE_VECTOR_ANY,
                                                 optional=False))
        self.addParameter(ParameterBoolean(name=self.INPUT_OVERWRITE_FLAG,
                                           description=self.tr('Overwrite output vrt'),
                                           default=False))

        self.addOutput(OutputFile(self.OUTPUT_VRT_FILE,
                                  self.tr('Virtual vector'), ext='vrt'))
        self.addOutput(OutputString(self.OUTPUT_VRT_STRING,
                                    self.tr('Virtual string')))

    def processAlgorithm(self, progress):
        input_layers = self.getParameterValue(self.INPUT_DATASOURCES)
        overwrite = self.getParameterValue(self.INPUT_OVERWRITE_FLAG)
        outVrtPath = self.getOutputValue(self.OUTPUT_VRT_FILE)
        outVrtString = self.getOutputValue(self.OUTPUT_VRT_STRING)

        ds = input_layers.split(";")
        if not isinstance(ds, list):
            msg = "Input datasources would be a ';' separated list of path strings"
            raise GeoAlgorithmExecutionException(msg)

        if not ds:
            msg = "Input data sources is empty"
            raise GeoAlgorithmExecutionException(msg)

        if not overwrite and os.path.exists(outVrtPath):
            msg = "Output vrt: %s already exist and choosed to avoid overwrite it" % outVrtPath
            raise GeoAlgorithmExecutionException(msg)

        outVrtString = self.mergeDataSources2Vrt(data_sources=ds,
                                                 outfile=outVrtPath,
                                                 relative=False,
                                                 schema=False,
                                                 progress=progress)

        self.setOutputValue(self.OUTPUT_VRT_STRING, outVrtString)

    def mergeDataSources2Vrt(self,
                             data_sources=[],
                             outfile=None,
                             relative=False,
                             schema=False,
                             progress=None):
        '''Function to do the work of merging datasources in a single vrt format

        @param data_sources: Array of path strings
        @param outfile: the output vrt file to generate
        @param relative: Write relative flag. DOES NOT relativise paths. They have to be already relative
        @param schema: Schema flag
        @return: vrt in string format
        '''
        if not data_sources or len(data_sources) == 0:
            return None

        # Start the VRT file.
        vrt = '<OGRVRTDataSource>\n'

        # For each file open the datasource to read.
        for i, infile in enumerate(data_sources):
            progress.setPercentage(int(100 * i / len(data_sources)))

            src_ds = ogr.Open(infile, update=0)

            if src_ds is None:
                msg = "Invalid datasource: %s" % infile
                raise GeoAlgorithmExecutionException(msg)

            if schema:
                infile = '@dummy@'

            layer_list = []
            for layer in src_ds:
                layer_list.append(layer.GetLayerDefn().GetName())

            # Process each source layer.
            for name in layer_list:
                layer = src_ds.GetLayerByName(name)
                layerdef = layer.GetLayerDefn()

                vrt += '  <OGRVRTLayer name="%s">\n' % self.XmlEsc(name)
                vrt += '    <SrcDataSource relativeToVRT="%s" shared="%d">%s</SrcDataSource>\n' \
                       % ('1' if relative else '0', not schema, self.XmlEsc(infile))
                if schema:
                    vrt += '    <SrcLayer>@dummy@</SrcLayer>\n'
                else:
                    vrt += '    <SrcLayer>%s</SrcLayer>\n' % self.XmlEsc(name)
                vrt += '    <GeometryType>%s</GeometryType>\n' \
                       % self.GeomType2Name(layerdef.GetGeomType())
                srs = layer.GetSpatialRef()
                if srs is not None:
                    vrt += '    <LayerSRS>%s</LayerSRS>\n' \
                           % (self.XmlEsc(srs.ExportToWkt()))

                # Process all the fields.
                for fld_index in range(layerdef.GetFieldCount()):
                    src_fd = layerdef.GetFieldDefn(fld_index)
                    if src_fd.GetType() == ogr.OFTInteger:
                        type = 'Integer'
                    elif src_fd.GetType() == ogr.OFTString:
                        type = 'String'
                    elif src_fd.GetType() == ogr.OFTReal:
                        type = 'Real'
                    elif src_fd.GetType() == ogr.OFTStringList:
                        type = 'StringList'
                    elif src_fd.GetType() == ogr.OFTIntegerList:
                        type = 'IntegerList'
                    elif src_fd.GetType() == ogr.OFTRealList:
                        type = 'RealList'
                    elif src_fd.GetType() == ogr.OFTBinary:
                        type = 'Binary'
                    elif src_fd.GetType() == ogr.OFTDate:
                        type = 'Date'
                    elif src_fd.GetType() == ogr.OFTTime:
                        type = 'Time'
                    elif src_fd.GetType() == ogr.OFTDateTime:
                        type = 'DateTime'
                    else:
                        type = 'String'

                    vrt += '    <Field name="%s" type="%s"' \
                           % (self.XmlEsc(src_fd.GetName()), type)
                    if not schema:
                        vrt += ' src="%s"' % self.XmlEsc(src_fd.GetName())
                    if src_fd.GetWidth() > 0:
                        vrt += ' width="%d"' % src_fd.GetWidth()
                    if src_fd.GetPrecision() > 0:
                        vrt += ' precision="%d"' % src_fd.GetPrecision()
                    vrt += '/>\n'

                vrt += '  </OGRVRTLayer>\n'

            # Сlose data source
            src_ds.Destroy()

        vrt += '</OGRVRTDataSource>\n'

        if outfile is not None:
            f = open(outfile, "w")
            f.write(vrt)
            f.close()

        return vrt

    def GeomType2Name(self, type):
        if type == ogr.wkbUnknown:
            return 'wkbUnknown'
        elif type == ogr.wkbPoint:
            return 'wkbPoint'
        elif type == ogr.wkbLineString:
            return 'wkbLineString'
        elif type == ogr.wkbPolygon:
            return 'wkbPolygon'
        elif type == ogr.wkbMultiPoint:
            return 'wkbMultiPoint'
        elif type == ogr.wkbMultiLineString:
            return 'wkbMultiLineString'
        elif type == ogr.wkbMultiPolygon:
            return 'wkbMultiPolygon'
        elif type == ogr.wkbGeometryCollection:
            return 'wkbGeometryCollection'
        elif type == ogr.wkbNone:
            return 'wkbNone'
        elif type == ogr.wkbLinearRing:
            return 'wkbLinearRing'
        else:
            return 'wkbUnknown'

    def XmlEsc(self, x):
        return xml.sax.saxutils.escape(x)
