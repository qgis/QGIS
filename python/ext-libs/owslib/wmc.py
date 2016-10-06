# -*- coding: ISO-8859-15 -*-
# =============================================================================
# Copyright (c) 2004 Sean C. Gillies
# Copyright (c) 2005 Nuxeo SARL <http://nuxeo.com>
#
# Authors : Sean Gillies <sgillies@frii.com>
#           Julien Anguenot <ja@nuxeo.com>
#
# Contact email: sgillies@frii.com
# =============================================================================

"""Web Map Context (WMC)

Specification can be found over there :
https://portal.opengeospatial.org/files/?artifact_id=8618

"""

from __future__ import (absolute_import, division, print_function)

from .etree import etree


context_ns_uri = 'http://www.opengis.net/context'
context_schemas_uri = 'http://schemas.opengis.net/context/1.0.0/context.xsd'

def WMCElement(tag):
    """WMC based element
    """
    return etree.Element("{%s}"%context_ns_uri + tag)

class MapContext:
    """ Map Context abstraction

    It uses a Map representation as input and export it as as map
    context
    """

    def __init__(self, map_):
        self._map = map_

    def _getRootElement(self):
        root = WMCElement('ViewContext')
        attrs = {
            '{http://www.w3.org/2001/XMLSchema-instance}schemaLocation':
            context_ns_uri + ' ' + context_schemas_uri,
            'id' : self._map.id,
            'version' : '1.0.0',
            }
        for k, v in attrs.items():
            root.attrib[k] = v
        return root

    def _getGeneralElement(self):
        general = WMCElement('General')
        general.append(self._getWindowElement())
        general.append(self._getBoundingBoxElement())
        return general

    def _getWindowElement(self):
        window = WMCElement('Window')
        window.attrib['width'] = str(self._map.size[0])
        window.attrib['height'] = str(self._map.size[1])
        return window

    def _getBoundingBoxElement(self):
        bbox = WMCElement('BoundingBox')
        bbox.attrib['SRS'] = str(self._map.srs.split()[0])
        bbox.attrib['minx'] = str(self._map.bounds[0])
        bbox.attrib['miny'] = str(self._map.bounds[1])
        bbox.attrib['maxx'] = str(self._map.bounds[2])
        bbox.attrib['maxy'] = str(self._map.bounds[3])
        return bbox

    def _getLayerListElement(self):
        layerlist = WMCElement('LayerList')
        layering = zip(self._map.layernames, self._map.layertitles)
        layer_infos = self._map.getLayerInfos()

        # mapbuilder draws layers in bottom-top order
        for name, title in layering:

            # Layer
            layer = WMCElement('Layer')
            layer.attrib['queryable'] = '0'
            layer.attrib['hidden'] = str(
                int(name not in self._map.visible_layers))

            # Layer styles
            if layer_infos and layer_infos.get(title):
                stylelist = WMCElement('StyleList')
                # Get wms `Style` nodes for a given layer
                for e_style in layer_infos.get(title):
                    e_style.attrib['current'] = '1'
                    # Change namespace to wmc
                    for node in e_style.getiterator():
                        tag_name = node.tag[node.tag.rfind('}')+1:]
                        node.tag = "{%s}"%context_ns_uri + tag_name
                    stylelist.append(e_style)
                layer.append(stylelist)

            # Server
            server = WMCElement('Server')
            server.attrib['service'] = 'OGC:WMS'
            server.attrib['version'] = '1.1.1'
            server.attrib['title'] = 'OGC:WMS'

            # OnlineRessource
            oressource = WMCElement('OnlineResource')
            oressource.attrib[
                '{http://www.w3.org/1999/xlink}type'] = 'simple'
            oressource.attrib[
                '{http://www.w3.org/1999/xlink}href'] = self._map.url
            server.append(oressource)
            layer.append(server)

            # Name
            e_name = WMCElement('Name')
            e_name.text = name
            layer.append(e_name)

            # Title
            e_title = WMCElement('Title')
            e_title.text = title
            layer.append(e_title)

            # Format
            formatlist = WMCElement('FormatList')
            format = WMCElement('Format')
            format.attrib['current'] = '1'
            format.text = self._map.format
            formatlist.append(format)
            layer.append(formatlist)
            layerlist.append(layer)

        return layerlist

    def __call__(self):
        """Export self._map to WMC
        """
        wmc_doc_tree = self._getRootElement()
        wmc_doc_tree.append(self._getGeneralElement())
        wmc_doc_tree.append(self._getLayerListElement())
        return etree.tostring(wmc_doc_tree)


class AggregateMapContext(MapContext):
    """ Map Context abstraction

    It uses a Map representation as input and export it as as map
    context -- with aggregation of all layers accomplished through
    overload of the Layer/Name property
    """

    def _getLayerListElement(self):
        layerlist = WMCElement('LayerList')
        #layering = zip(self._map.layernames, self._map.layertitles)
        layer_infos = self._map.getLayerInfos()

        # Layer
        layer = WMCElement('Layer')
        layer.attrib['queryable'] = '0'
        layer.attrib['hidden'] = '0'

        # Server
        server = WMCElement('Server')
        server.attrib['service'] = 'OGC:WMS'
        server.attrib['version'] = '1.1.1'
        server.attrib['title'] = 'OGC:WMS'

        # OnlineRessource
        oressource = WMCElement('OnlineResource')
        oressource.attrib['{http://www.w3.org/1999/xlink}type'] = 'simple'
        oressource.attrib['{http://www.w3.org/1999/xlink}href'] = self._map.url
        server.append(oressource)
        layer.append(server)

        # Name
        e_name = WMCElement('Name')
        e_name.text = ','.join(self._map.layernames)
        layer.append(e_name)

        # Title
        e_title = WMCElement('Title')
        e_title.text = 'Aggregate Layers'
        layer.append(e_title)

        # Format
        formatlist = WMCElement('FormatList')
        format = WMCElement('Format')
        format.attrib['current'] = '1'
        format.text = self._map.format
        formatlist.append(format)
        layer.append(formatlist)
        layerlist.append(layer)
        
        return layerlist


def mapToWebMapContext(map, aggregate_layers=False):
    """Helper

    if the second argument evaluates to True, then all map layers are
    aggregated into a single map context layer.
    """
    if aggregate_layers:
        return AggregateMapContext(map)()
    else:
        return MapContext(map)()

