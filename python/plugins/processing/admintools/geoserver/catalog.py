# -*- coding: utf-8 -*-

"""
***************************************************************************
    catalog.py
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

from datetime import datetime, timedelta
import logging
from processing.admintools.geoserver.layer import Layer
from processing.admintools.geoserver.store import coveragestore_from_index, datastore_from_index, \
    UnsavedDataStore, UnsavedCoverageStore
from processing.admintools.geoserver.style import Style
from processing.admintools.geoserver.support import prepare_upload_bundle, url
from processing.admintools.geoserver.layergroup import LayerGroup, UnsavedLayerGroup
from processing.admintools.geoserver.workspace import workspace_from_index, Workspace
from os import unlink
from xml.etree.ElementTree import XML
from xml.parsers.expat import ExpatError
from urlparse import urlparse
from processing.admintools import httplib2

logger = logging.getLogger("gsconfig.catalog")

class UploadError(Exception):
    pass

class ConflictingDataError(Exception):
    pass

class AmbiguousRequestError(Exception):
    pass

class FailedRequestError(Exception):
    pass

def _name(named):
    """Get the name out of an object.  This varies based on the type of the input:
       * the "name" of a string is itself
       * the "name" of None is itself
       * the "name" of an object with a property named name is that property -
         as long as it's a string
       * otherwise, we raise a ValueError
    """
    if isinstance(named, basestring) or named is None:
        return named
    elif hasattr(named, 'name') and isinstance(named.name, basestring):
        return named.name
    else:
        raise ValueError("Can't interpret %s as a name or a configuration object" % named)

class Catalog(object):
    """
    The GeoServer catalog represents all of the information in the GeoServer
    configuration.    This includes:
    - Stores of geospatial data
    - Resources, or individual coherent datasets within stores
    - Styles for resources
    - Layers, which combine styles with resources to create a visible map layer
    - LayerGroups, which alias one or more layers for convenience
    - Workspaces, which provide logical grouping of Stores
    - Maps, which provide a set of OWS services with a subset of the server's
        Layers
    - Namespaces, which provide unique identifiers for resources
    """

    def __init__(self, service_url, username="admin", password="geoserver", disable_ssl_certificate_validation=False):
        self.service_url = service_url
        if self.service_url.endswith("/"):
            self.service_url = self.service_url.strip("/")
        self.http = httplib2.Http(
            disable_ssl_certificate_validation=disable_ssl_certificate_validation)
        self.username = username
        self.password = password
        self.http.add_credentials(self.username, self.password)
        netloc = urlparse(service_url).netloc
        self.http.authorizations.append(
                httplib2.BasicAuthentication(
                        (username, password),
                        netloc,
                        service_url,
                        {},
                        None,
                        None,
                        self.http
                        ))
        self._cache = dict()

    def delete(self, config_object, purge=False, recurse=False):
        """
        send a delete request
        XXX [more here]
        """
        rest_url = config_object.href

        #params aren't supported fully in httplib2 yet, so:
        params = []

        # purge deletes the SLD from disk when a style is deleted
        if purge:
            params.append("purge=true")

        # recurse deletes the resource when a layer is deleted.
        if recurse:
            params.append("recurse=true")

        if params:
            rest_url = rest_url + "?" + "&".join(params)

        headers = {
            "Content-type": "application/xml",
            "Accept": "application/xml"
        }
        response, content = self.http.request(rest_url, "DELETE", headers=headers)
        self._cache.clear()

        if response.status == 200:
            return (response, content)
        else:
            raise FailedRequestError("Tried to make a DELETE request to %s but got a %d status code: \n%s" % (rest_url, response.status, content))

    def get_xml(self, rest_url):
        logger.debug("GET %s", rest_url)

        cached_response = self._cache.get(rest_url)

        def is_valid(cached_response):
            return cached_response is not None and datetime.now() - cached_response[0] < timedelta(seconds=5)

        def parse_or_raise(xml):
            try:
                return XML(xml)
            except (ExpatError, SyntaxError), e:
                msg = "GeoServer gave non-XML response for [GET %s]: %s"
                msg = msg % (rest_url, xml)
                raise Exception(msg, e)

        if is_valid(cached_response):
            raw_text = cached_response[1]
            return parse_or_raise(raw_text)
        else:
            response, content = self.http.request(rest_url)
            if response.status == 200:
                self._cache[rest_url] = (datetime.now(), content)
                return parse_or_raise(content)
            else:
                raise FailedRequestError("Tried to make a GET request to %s but got a %d status code: \n%s" % (url, response.status, content))

    def reload(self):
        reload_url = url(self.service_url, ['reload'])
        response = self.http.request(reload_url, "POST")
        self._cache.clear()
        return response

    def save(self, obj):
        """
        saves an object to the REST service

        gets the object's REST location and the XML from the object,
        then POSTS the request.
        """
        rest_url = obj.href
        message = obj.message()

        headers = {
            "Content-type": "application/xml",
            "Accept": "application/xml"
        }
        logger.debug("%s %s", obj.save_method, obj.href)
        response = self.http.request(rest_url, obj.save_method, message, headers)
        headers, body = response
        self._cache.clear()
        if 400 <= int(headers['status']) < 600:
            raise FailedRequestError("Error code (%s) from GeoServer: %s" %
                (headers['status'], body))
        return response

    def get_store(self, name, workspace=None):
        #stores = [s for s in self.get_stores(workspace) if s.name == name]
        if workspace is None:
            store = None
            for ws in self.get_workspaces():
                found = None
                try:
                    found = self.get_store(name, ws)
                except:
                    # don't expect every workspace to contain the named store
                    pass
                if found:
                    if store:
                        raise AmbiguousRequestError("Multiple stores found named: " + name)
                    else:
                        store = found
            if not store:
                raise FailedRequestError("No store found named: " + name)
            return store
        else: # workspace is not None
            if isinstance(workspace, basestring):
                workspace = self.get_workspace(workspace)
                if workspace is None:
                    return None
            logger.debug("datastore url is [%s]", workspace.datastore_url )
            ds_list = self.get_xml(workspace.datastore_url)
            cs_list = self.get_xml(workspace.coveragestore_url)
            datastores = [n for n in ds_list.findall("dataStore") if n.find("name").text == name]
            coveragestores = [n for n in cs_list.findall("coverageStore") if n.find("name").text == name]
            ds_len, cs_len = len(datastores), len(coveragestores)

            if ds_len == 1 and cs_len == 0:
                return datastore_from_index(self, workspace, datastores[0])
            elif ds_len == 0 and cs_len == 1:
                return coveragestore_from_index(self, workspace, coveragestores[0])
            elif ds_len == 0 and cs_len == 0:
                raise FailedRequestError("No store found in " + str(workspace) + " named: " + name)
            else:
                raise AmbiguousRequestError(str(workspace) + " and name: " + name + " do not uniquely identify a layer")

    def get_stores(self, workspace=None):
        if workspace is not None:
            if isinstance(workspace, basestring):
                workspace = self.get_workspace(workspace)
            ds_list = self.get_xml(workspace.datastore_url)
            cs_list = self.get_xml(workspace.coveragestore_url)
            datastores = [datastore_from_index(self, workspace, n) for n in ds_list.findall("dataStore")]
            coveragestores = [coveragestore_from_index(self, workspace, n) for n in cs_list.findall("coverageStore")]
            return datastores + coveragestores
        else:
            stores = []
            for ws in self.get_workspaces():
                a = self.get_stores(ws)
                stores.extend(a)
            return stores

    def create_datastore(self, name, workspace=None):
        if isinstance(workspace, basestring):
            workspace = self.get_workspace(workspace)
        elif workspace is None:
            workspace = self.get_default_workspace()
        return UnsavedDataStore(self, name, workspace)

    def create_coveragestore2(self, name, workspace = None):
        """
        Hm we already named the method that creates a coverage *resource*
        create_coveragestore... time for an API break?
        """
        if isinstance(workspace, basestring):
            workspace = self.get_workspace(workspace)
        elif workspace is None:
            workspace = self.get_default_workspace()
        return UnsavedCoverageStore(self, name, workspace)

    def add_data_to_store(self, store, name, data, workspace=None, overwrite = False, charset = None):
        if isinstance(store, basestring):
            store = self.get_store(store, workspace=workspace)
        if workspace is not None:
            workspace = _name(workspace)
            assert store.workspace.name == workspace, "Specified store (%s) is not in specified workspace (%s)!" % (store, workspace)
        else:
            workspace = store.workspace.name
        store = store.name

        if isinstance(data, dict):
            bundle = prepare_upload_bundle(name, data)
        else:
            bundle = data

        params = dict()
        if overwrite:
            params["update"] = "overwrite"
        if charset is not None:
            params["charset"] = charset

        message = open(bundle)
        headers = { 'Content-Type': 'application/zip', 'Accept': 'application/xml' }
        upload_url = url(self.service_url,
            ["workspaces", workspace, "datastores", store, "file.shp"], params)

        try:
            headers, response = self.http.request(upload_url, "PUT", message, headers)
            self._cache.clear()
            if headers.status != 201:
                raise UploadError(response)
        finally:
            unlink(bundle)

    def create_featurestore(self, name, data, workspace=None, overwrite=False, charset=None):
        if not overwrite:
            try:
                store = self.get_store(name, workspace)
                msg = "There is already a store named " + name
                if workspace:
                    msg += " in " + str(workspace)
                raise ConflictingDataError(msg)
            except FailedRequestError:
                # we don't really expect that every layer name will be taken
                pass

        if workspace is None:
            workspace = self.get_default_workspace()
        workspace = _name(workspace)
        params = dict()
        if charset is not None:
            params['charset'] = charset
        ds_url = url(self.service_url,
            ["workspaces", workspace, "datastores", name, "file.shp"], params)

        # PUT /workspaces/<ws>/datastores/<ds>/file.shp
        headers = {
            "Content-type": "application/zip",
            "Accept": "application/xml"
        }
        if isinstance(data,dict):
            logger.debug('Data is NOT a zipfile')
            archive = prepare_upload_bundle(name, data)
        else:
            logger.debug('Data is a zipfile')
            archive = data
        message = open(archive)
        try:
            headers, response = self.http.request(ds_url, "PUT", message, headers)
            self._cache.clear()
            if headers.status != 201:
                raise UploadError(response)
        finally:
            unlink(archive)

    def create_coveragestore(self, name, data, workspace=None, overwrite=False):
        if not overwrite:
            try:
                store = self.get_store(name, workspace)
                msg = "There is already a store named " + name
                if workspace:
                    msg += " in " + str(workspace)
                raise ConflictingDataError(msg)
            except FailedRequestError:
                # we don't really expect that every layer name will be taken
                pass

        if workspace is None:
            workspace = self.get_default_workspace()
        headers = {
            "Content-type": "image/tiff",
            "Accept": "application/xml"
        }

        archive = None
        ext = "geotiff"

        if isinstance(data, dict):
            archive = prepare_upload_bundle(name, data)
            message = open(archive)
            if "tfw" in data:
                headers['Content-type'] = 'application/archive'
                ext = "worldimage"
        elif isinstance(data, basestring):
            message = open(data)
        else:
            message = data

        cs_url = url(self.service_url,
            ["workspaces", workspace.name, "coveragestores", name, "file." + ext])

        try:
            headers, response = self.http.request(cs_url, "PUT", message, headers)
            self._cache.clear()
            if headers.status != 201:
                raise UploadError(response)
        finally:
            if archive is not None:
                unlink(archive)

    def get_resource(self, name, store=None, workspace=None):
        if store is not None:
            candidates = [s for s in self.get_resources(store) if s.name == name]
            if len(candidates) == 0:
                return None
            elif len(candidates) > 1:
                raise AmbiguousRequestError
            else:
                return candidates[0]

        if workspace is not None:
            for store in self.get_stores(workspace):
                resource = self.get_resource(name, store)
                if resource is not None:
                    return resource
            return None

        for ws in self.get_workspaces():
            resource = self.get_resource(name, workspace=ws)
            if resource is not None:
                return resource
        return None

    def get_resources(self, store=None, workspace=None):
        if isinstance(workspace, basestring):
            workspace = self.get_workspace(workspace)
        if isinstance(store, basestring):
            store = self.get_store(store, workspace)
        if store is not None:
            return store.get_resources()
        if workspace is not None:
            resources = []
            for store in self.get_stores(workspace):
                resources.extend(self.get_resources(store))
            return resources
        resources = []
        for ws in self.get_workspaces():
            resources.extend(self.get_resources(workspace=ws))
        return resources

    def get_layer(self, name):
        try:
            lyr = Layer(self, name)
            lyr.fetch()
            return lyr
        except FailedRequestError:
            return None

    def get_layers(self, resource=None):
        if isinstance(resource, basestring):
            resource = self.get_resource(resource)
        layers_url = url(self.service_url, ["layers.xml"])
        description = self.get_xml(layers_url)
        lyrs = [Layer(self, l.find("name").text) for l in description.findall("layer")]
        if resource is not None:
            lyrs = [l for l in lyrs if l.resource.href == resource.href]
        # TODO: Filter by style
        return lyrs

    def get_layergroup(self, name=None):
        try:
            group_url = url(self.service_url, ["layergroups", name + ".xml"])
            group = self.get_xml(group_url)
            return LayerGroup(self, group.find("name").text)
        except FailedRequestError:
            return None

    def get_layergroups(self):
        groups = self.get_xml("%s/layergroups.xml" % self.service_url)
        return [LayerGroup(self, g.find("name").text) for g in groups.findall("layerGroup")]

    def create_layergroup(self, name, layers = (), styles = (), bounds = None):
        if any(g.name == name for g in self.get_layergroups()):
            raise ConflictingDataError("LayerGroup named %s already exists!" % name)
        else:
            return UnsavedLayerGroup(self, name, layers, styles, bounds)

    def get_style(self, name):
        try:
            style_url = url(self.service_url, ["styles", name + ".xml"])
            dom = self.get_xml(style_url)
            return Style(self, dom.find("name").text)
        except FailedRequestError:
            return None

    def get_styles(self):
        styles_url = url(self.service_url, ["styles.xml"])
        description = self.get_xml(styles_url)
        return [Style(self, s.find('name').text) for s in description.findall("style")]

    def create_style(self, name, data, overwrite = False):
        if overwrite == False and self.get_style(name) is not None:
            raise ConflictingDataError("There is already a style named %s" % name)

        headers = {
            "Content-type": "application/vnd.ogc.sld+xml",
            "Accept": "application/xml"
        }

        if overwrite:
            style_url = url(self.service_url, ["styles", name + ".sld"])
            headers, response = self.http.request(style_url, "PUT", data, headers)
        else:
            style_url = url(self.service_url, ["styles"], dict(name=name))
            headers, response = self.http.request(style_url, "POST", data, headers)

        self._cache.clear()
        if headers.status < 200 or headers.status > 299: raise UploadError(response)

    def create_workspace(self, name, uri):
        xml = ("<namespace>"
            "<prefix>{name}</prefix>"
            "<uri>{uri}</uri>"
            "</namespace>").format(name=name, uri=uri)
        headers = { "Content-Type": "application/xml" }
        workspace_url = self.service_url + "/namespaces/"

        headers, response = self.http.request(workspace_url, "POST", xml, headers)
        assert 200 <= headers.status < 300, "Tried to create workspace but got " + str(headers.status) + ": " + response
        self._cache.clear()
        return self.get_workspace(name)

    def get_workspaces(self):
        description = self.get_xml("%s/workspaces.xml" % self.service_url)
        return [workspace_from_index(self, node) for node in description.findall("workspace")]

    def get_workspace(self, name):
        candidates = [w for w in self.get_workspaces() if w.name == name]
        if len(candidates) == 0:
            return None
        elif len(candidates) > 1:
            raise AmbiguousRequestError()
        else:
            return candidates[0]

    def get_default_workspace(self):
        return Workspace(self, "default")

    def set_default_workspace(self):
        raise NotImplementedError()
