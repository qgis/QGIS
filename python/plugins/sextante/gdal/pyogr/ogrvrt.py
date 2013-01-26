# -*- coding: utf-8 -*-

"""
***************************************************************************
    ogrvrt.py
    ---------------------
    Date                 : November 2012
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
__date__ = 'November 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

# Based on ogr2vrt.py implementation by Frank Warmerdam included in GDAL/OGR
import string
import re
import cgi
import sys

from osgeo import gdal, ogr

from string import Template
import os
import tempfile

def GeomType2Name( type ):
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

#############################################################################
def Esc(x):
    return gdal.EscapeString( x, gdal.CPLES_XML )

def transformed_template(template, substitutions):
    vrt_templ = Template(open(template).read())
    vrt_xml = vrt_templ.substitute(substitutions)
    vrt = tempfile.mktemp( '.vrt',  'ogr_',  '/vsimem')
    # Create in-memory file
    gdal.FileFromMemBuffer(vrt, vrt_xml)
    return vrt

def free_template(vrt):
    # Free memory associated with the in-memory file
    gdal.Unlink(vrt)

def transformed_datasource(template, substitutions):
    vrt = transformed_template(template, substitutions)
    ds = ogr.Open(vrt)
    return ds

def close_datasource(ds):
   if ds is not None:
       ds.Destroy()

def ogr2vrt(infile,
            outfile = None,
            layer_list = [],
            relative = "0",
            schema=0):

    #############################################################################
    # Open the datasource to read.

    src_ds = ogr.Open( infile, update = 0 )

    if src_ds is None:
        return None

    if schema:
        infile = '@dummy@'

    if len(layer_list) == 0:
        for layer in src_ds:
            layer_list.append( layer.GetLayerDefn().GetName() )

    #############################################################################
    # Start the VRT file.

    vrt = '<OGRVRTDataSource>\n'

    #############################################################################
    #	Process each source layer.

    for name in layer_list:
        layer = src_ds.GetLayerByName(name)
        layerdef = layer.GetLayerDefn()

        vrt += '  <OGRVRTLayer name="%s">\n' % Esc(name)
        vrt += '    <SrcDataSource relativeToVRT="%s" shared="%d">%s</SrcDataSource>\n' \
               % (relative,not schema,Esc(infile))
        if schema:
            vrt += '    <SrcLayer>@dummy@</SrcLayer>\n'
        else:
            vrt += '    <SrcLayer>%s</SrcLayer>\n' % Esc(name)
        vrt += '    <GeometryType>%s</GeometryType>\n' \
               % GeomType2Name(layerdef.GetGeomType())
        srs = layer.GetSpatialRef()
        if srs is not None:
            vrt += '    <LayerSRS>%s</LayerSRS>\n' \
                   % (Esc(srs.ExportToWkt()))

        # Process all the fields.
        for fld_index in range(layerdef.GetFieldCount()):
            src_fd = layerdef.GetFieldDefn( fld_index )
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
                   % (Esc(src_fd.GetName()), type)
            if not schema:
                vrt += ' src="%s"' % Esc(src_fd.GetName())
            if src_fd.GetWidth() > 0:
                vrt += ' width="%d"' % src_fd.GetWidth()
            if src_fd.GetPrecision() > 0:
                vrt += ' precision="%d"' % src_fd.GetPrecision()
            vrt += '/>\n'

        vrt += '  </OGRVRTLayer>\n'

    vrt += '</OGRVRTDataSource>\n'

    if outfile is not None:
        f = open(outfile, "w")
        f.write(vrt)
        f.close()

    return vrt
