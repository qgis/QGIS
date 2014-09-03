# -*- coding: utf-8 -*-
###############################################################################
#
# MetaSearch Catalogue Client
#
# Copyright (C) 2014 Tom Kralidis (tomkralidis@gmail.com)
#
# This source is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This code is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
###############################################################################

WMSWMST_LINK_TYPES = [
    'OGC:WMS',
    'OGC:WMTS',
    'OGC:WMS-1.1.1-http-get-map',
    'OGC:WMS-1.1.1-http-get-capabilities',
    'OGC:WMS-1.3.0-http-get-map',
    'OGC:WMS-1.3.0-http-get-capabilities',
    'urn:x-esri:specification:ServiceType:wms:url',
    'urn:x-esri:specification:ServiceType:Gmd:URL.wms'
]

WFS_LINK_TYPES = [
    'OGC:WFS',
    'OGC:WFS-1.0.0-http-get-capabilities',
    'OGC:WFS-1.1.0-http-get-capabilities',
    'urn:x-esri:specification:ServiceType:wfs:url',
    'urn:x-esri:specification:ServiceType:Gmd:URL.wfs'
]

WCS_LINK_TYPES = [
    'OGC:WCS',
    'OGC:WCS-1.1.0-http-get-capabilities',
    'urn:x-esri:specification:ServiceType:wcs:url',
    'urn:x-esri:specification:ServiceType:Gmd:URL.wcs'
]
