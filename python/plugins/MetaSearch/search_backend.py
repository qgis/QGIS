# -*- coding: utf-8 -*-
###############################################################################
#
# CSW Client
# ---------------------------------------------------------
# QGIS Catalog Service client.
#
# Copyright (C) 2021 Tom Kralidis (tomkralidis@gmail.com)
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

import warnings

import owslib
from owslib.fes import BBox, PropertyIsLike

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=ResourceWarning)
    warnings.filterwarnings("ignore", category=ImportWarning)
    from owslib.csw import CatalogueServiceWeb  # spellok

if owslib.__version__ < '0.25':
    OWSLIB_OAREC_SUPPORTED = False
else:
    OWSLIB_OAREC_SUPPORTED = True

CATALOG_TYPES = [
    'OGC CSW 2.0.2',
    'OGC API - Records'
]


class SearchBase:
    def __init__(self, url, timeout, username=None, password=None, auth=None):
        self.url = url
        self.timeout = timeout
        self.username = username
        self.password = password
        self.auth = auth
        self.service_info_template = None
        self.record_info_template = None
        self.request = None
        self.response = None
        self.matches = 0
        self.returned = 0
        self.format = None

    def get_service_info(self):
        pass

    def query_records(self):
        pass

    def records(self):
        pass

    def get_record(self, identifier):
        pass

    def parse_link(self, link):
        return link


class CSW202Search(SearchBase):
    def __init__(self, url, timeout, username, password, auth):
        super().__init__(url, timeout, username, password, auth)

        self.type = CATALOG_TYPES[0]
        self.format = 'xml'
        self.service_info_template = 'csw_service_metadata.html'
        self.record_info_template = 'record_metadata_dc.html'
        self.constraints = []

        self.conn = CatalogueServiceWeb(self.url,  # spellok
                                        timeout=self.timeout,
                                        username=self.username,
                                        password=self.password,
                                        auth=self.auth)

        self.request = self.conn.request
        self.response = self.conn.response

    def query_records(self, bbox=[], keywords=None, limit=10, offset=1):

        # only apply spatial filter if bbox is not global
        # even for a global bbox, if a spatial filter is applied, then
        # the CSW server will skip records without a bbox
        if bbox and bbox != ['-180', '-90', '180', '90']:
            minx, miny, maxx, maxy = bbox
            self.constraints.append(BBox([miny, minx, maxy, maxx],
                                         crs='urn:ogc:def:crs:EPSG::4326'))

        # keywords
        if keywords:
            # TODO: handle multiple word searches
            self.constraints.append(PropertyIsLike('csw:AnyText', keywords))

        if len(self.constraints) > 1:  # exclusive search (a && b)
            self.constraints = [self.constraints]

        self.conn.getrecords2(constraints=self.constraints, maxrecords=limit,
                              startposition=offset, esn='full')

        self.matches = self.conn.results['matches']
        self.returned = self.conn.results['returned']

        self.request = self.conn.request
        self.response = self.conn.response

    def records(self):
        recs = []

        for record in self.conn.records:
            rec = {
                'identifier': None,
                'type': None,
                'title': None,
                'bbox': None
            }

            if self.conn.records[record].identifier:
                rec['identifier'] = self.conn.records[record].identifier
            if self.conn.records[record].type:
                rec['type'] = self.conn.records[record].type
            if self.conn.records[record].title:
                rec['title'] = self.conn.records[record].title
            if self.conn.records[record].bbox:
                rec['bbox'] = bbox_list_to_dict(
                    self.conn.records[record].bbox)

            rec['links'] = (self.conn.records[record].uris +
                            self.conn.records[record].references)

            recs.append(rec)

        return recs

    def get_record(self, identifier):
        self.conn.getrecordbyid([identifier])

        return self.conn.records[identifier]


class OARecSearch(SearchBase):
    def __init__(self, url, timeout, auth):
        try:
            from owslib.ogcapi.records import Records
        except ModuleNotFoundError:
            # OWSLIB_OAREC_SUPPORTED already set to False
            pass

        super().__init__(url, timeout, auth)

        self.type = CATALOG_TYPES[1]
        self.format = 'json'
        self.service_info_template = 'oarec_service_metadata.html'
        self.record_info_template = 'record_metadata_oarec.html'
        self.base_url = None
        self.record_collection = None

        if '/collections/' in self.url:  # catalog is a collection
            self.base_url, self.record_collection = self.url.split('/collections/')  # noqa
            self.conn = Records(
                self.base_url, timeout=self.timeout, auth=self.auth)
            c = self.conn.collection(self.record_collection)
            try:
                self.conn.links = c['links']
                self.conn.title = c['title']
                self.conn.description = c['description']
            except KeyError:
                pass
            self.request = self.conn.request
        else:
            self.conn = Records(self.url, timeout=self.timeout, auth=self.auth)
            self.request = None

        self.response = self.conn.response

    def query_records(self, bbox=[], keywords=None, limit=10, offset=1):

        params = {
            'collection_id': self.record_collection,
            'limit': limit,
            'startindex': offset,
        }

        if keywords:
            params['q'] = keywords
        if bbox and bbox != ['-180', '-90', '180', '90']:
            params['bbox'] = bbox

        self.response = self.conn.collection_items(**params)

        self.matches = self.response.get('numberMatched', 0)
        self.returned = self.response.get('numberReturned', 0)
        self.request = self.conn.request

    def records(self):
        recs = []

        for rec in self.response['features']:
            rec1 = {
                'identifier': rec['id'],
                'type': rec['properties']['type'],



                'bbox': None,
                'title': rec['properties']['title'],
                'links': rec['properties'].get('associations', [])
            }
            try:
                bbox2 = rec['properties']['extent']['spatial']['bbox'][0]
                if bbox2:
                    rec1['bbox'] = bbox_list_to_dict(bbox2)
            except KeyError:
                pass

            recs.append(rec1)

        return recs

    def get_record(self, identifier):
        return self.conn.collection_item(self.record_collection, identifier)

    def parse_link(self, link):
        link2 = {}
        if 'href' in link:
            link2['url'] = link['href']
        if 'type' in link:
            link2['protocol'] = link['type']
        if 'title' in link:
            link2['title'] = link['title']
        if 'id' in link:
            link2['name'] = link['id']
        return link2


def get_catalog_service(url, catalog_type, timeout, username, password,
                        auth=None):
    if catalog_type in [None, CATALOG_TYPES[0]]:
        return CSW202Search(url, timeout, username, password, auth)
    elif catalog_type == CATALOG_TYPES[1]:
        if not OWSLIB_OAREC_SUPPORTED:
            raise ValueError("OGC API - Records requires OWSLib 0.25 or above")
        return OARecSearch(url, timeout, auth)


def bbox_list_to_dict(bbox):
    if isinstance(bbox, list):
        dict_ = {
            'minx': bbox[0],
            'maxx': bbox[2],
            'miny': bbox[1],
            'maxy': bbox[3]
        }
    else:
        dict_ = {
            'minx': bbox.minx,
            'maxx': bbox.maxx,
            'miny': bbox.miny,
            'maxy': bbox.maxy
        }
    return dict_
