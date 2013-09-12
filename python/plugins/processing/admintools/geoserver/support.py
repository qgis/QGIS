# -*- coding: utf-8 -*-

"""
***************************************************************************
    support.py
    ---------------------
    Date                 : November 2012
    Copyright            : (C) 2012 by David Winslow
    Email                : dwins at opengeo dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'David Winslow'
__date__ = 'November 2012'
__copyright__ = '(C) 2012, David Winslow'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import logging
from xml.etree.ElementTree import TreeBuilder, tostring
import urllib
import urlparse
from zipfile import ZipFile
from processing.tools.system import *


logger = logging.getLogger("gsconfig.support")

FORCE_DECLARED = "FORCE_DECLARED"
## The projection handling policy for layers that should use coordinates
## directly while reporting the configured projection to clients.  This should be
## used when projection information is missing from the underlying datastore.


FORCE_NATIVE = "FORCE_NATIVE"
## The projection handling policy for layers that should use the projection
## information from the underlying storage mechanism directly, and ignore the
## projection setting.

REPROJECT = "REPROJECT"
## The projection handling policy for layers that should use the projection
## information from the underlying storage mechanism to reproject to the
## configured projection.

def url(base, seg, query=None):
    """
    Create a URL from a list of path segments and an optional dict of query
    parameters.
    """

    seg = (urllib.quote(s.strip('/')) for s in seg)
    if query is None or len(query) == 0:
        query_string = ''
    else:
        query_string = "?" + urllib.urlencode(query)
    path = '/'.join(seg) + query_string
    adjusted_base = base.rstrip('/') + '/'
    return urlparse.urljoin(adjusted_base, path)

def xml_property(path, converter = lambda x: x.text):
    def getter(self):
        if path in self.dirty:
            return self.dirty[path]
        else:
            if self.dom is None:
                self.fetch()
            node = self.dom.find(path)
            return converter(self.dom.find(path)) if node is not None else None

    def setter(self, value):
        self.dirty[path] = value

    def delete(self):
        self.dirty[path] = None

    return property(getter, setter, delete)

def bbox(node):
    if node is not None:
        minx = node.find("minx")
        maxx = node.find("maxx")
        miny = node.find("miny")
        maxy = node.find("maxy")
        crs  = node.find("crs")
        crs  = crs.text if crs is not None else None

        if (None not in [minx, maxx, miny, maxy]):
            return (minx.text, maxx.text, miny.text, maxy.text, crs)
        else:
            return None
    else:
        return None

def string_list(node):
    if node is not None:
        return [n.text for n in node.findall("string")]

def attribute_list(node):
    if node is not None:
        return [n.text for n in node.findall("attribute/name")]

def key_value_pairs(node):
    if node is not None:
        return dict((entry.attrib['key'], entry.text) for entry in node.findall("entry"))

def write_string(name):
    def write(builder, value):
        builder.start(name, dict())
        if (value is not None):
            builder.data(value)
        builder.end(name)
    return write

def write_bool(name):
    def write(builder, b):
        builder.start(name, dict())
        builder.data("true" if b else "false")
        builder.end(name)
    return write

def write_bbox(name):
    def write(builder, b):
        builder.start(name, dict())
        bbox_xml(builder, b)
        builder.end(name)
    return write

def write_string_list(name):
    def write(builder, words):
        builder.start(name, dict())
        for w in words:
            builder.start("string", dict())
            builder.data(w)
            builder.end("string")
        builder.end(name)
    return write

def write_dict(name):
    def write(builder, pairs):
        builder.start(name, dict())
        for k, v in pairs.iteritems():
            builder.start("entry", dict(key=k))
            builder.data(v)
            builder.end("entry")
        builder.end(name)
    return write

class ResourceInfo(object):
    def __init__(self):
        self.dom = None
        self.dirty = dict()

    def fetch(self):
        self.dom = self.catalog.get_xml(self.href)

    def clear(self):
        self.dirty = dict()

    def refresh(self):
        self.clear()
        self.fetch()

    def serialize(self, builder):
        # GeoServer will disable the resource if we omit the <enabled> tag,
        # so force it into the dirty dict before writing
        if hasattr(self, "enabled"):
            self.dirty['enabled'] = self.enabled

        for k, writer in self.writers.items():
            if k in self.dirty:
                writer(builder, self.dirty[k])

    def message(self):
        builder = TreeBuilder()
        builder.start(self.resource_type, dict())
        self.serialize(builder)
        builder.end(self.resource_type)
        msg = tostring(builder.close())
        return msg

def prepare_upload_bundle(name, data):
    """GeoServer's REST API uses ZIP archives as containers for file formats such
    as Shapefile and WorldImage which include several 'boxcar' files alongside
    the main data.  In such archives, GeoServer assumes that all of the relevant
    files will have the same base name and appropriate extensions, and live in
    the root of the ZIP archive.  This method produces a zip file that matches
    these expectations, based on a basename, and a dict of extensions to paths or
    file-like objects. The client code is responsible for deleting the zip
    archive when it's done."""
    #we ut the zip file in the processing temp dir, so it is deleted at the end.
    f = getTempFilename('zip')
    zip_file = ZipFile(f, 'w')
    for ext, stream in data.iteritems():
        fname = "%s.%s" % (name, ext)
        if (isinstance(stream, basestring)):
            zip_file.write(stream, fname)
        else:
            zip_file.writestr(fname, stream.read())
    zip_file.close()
    return f

def atom_link(node):
    if 'href' in node.attrib:
        return node.attrib['href']
    else:
        l = node.find("{http://www.w3.org/2005/Atom}link")
        return l.get('href')

def atom_link_xml(builder, href):
    builder.start("atom:link", {
        'rel': 'alternate',
        'href': href,
        'type': 'application/xml',
        'xmlns:atom': 'http://www.w3.org/2005/Atom'
    })
    builder.end("atom:link")

def bbox_xml(builder, box):
    minx, maxx, miny, maxy, crs = box
    builder.start("minx", dict())
    builder.data(minx)
    builder.end("minx")
    builder.start("maxx", dict())
    builder.data(maxx)
    builder.end("maxx")
    builder.start("miny", dict())
    builder.data(miny)
    builder.end("miny")
    builder.start("maxy", dict())
    builder.data(maxy)
    builder.end("maxy")
    if crs is not None:
        builder.start("crs", {"class": "projected"})
        builder.data(crs)
        builder.end("crs")

