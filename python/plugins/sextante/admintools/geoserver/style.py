# -*- coding: utf-8 -*-

"""
***************************************************************************
    style.py
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

from sextante.admintools.geoserver.support import ResourceInfo, url, xml_property

class Style(ResourceInfo):
    def __init__(self, catalog, name):
        super(Style, self).__init__()
        assert isinstance(name, basestring)

        self.catalog = catalog
        self.name = name
        self._sld_dom = None

    @property
    def href(self):
        return url(self.catalog.service_url, ["styles", self.name + ".xml"])

    def body_href(self):
        return url(self.catalog.service_url, ["styles", self.name + ".sld"])

    filename = xml_property("filename")

    def _get_sld_dom(self):
        if self._sld_dom is None:
            self._sld_dom = self.catalog.get_xml(self.body_href())
        return self._sld_dom

    @property
    def sld_title(self):
        user_style = self._get_sld_dom().find("{http://www.opengis.net/sld}NamedLayer/{http://www.opengis.net/sld}UserStyle")
        title_node = user_style.find("{http://www.opengis.net/sld}Title")
        return title_node.text if title_node is not None else None

    @property
    def sld_name(self):
        user_style = self._get_sld_dom().find("{http://www.opengis.net/sld}NamedLayer/{http://www.opengis.net/sld}UserStyle")
        name_node = user_style.find("{http://www.opengis.net/sld}Name")
        return name_node.text if name_node is not None else None

    @property
    def sld_body(self):
        content = self.catalog.http.request(self.body_href())[1]
        return content

    def update_body(self, body):
        headers = { "Content-Type": "application/vnd.ogc.sld+xml" }
        self.catalog.http.request(
            self.body_href(), "PUT", body, headers)
