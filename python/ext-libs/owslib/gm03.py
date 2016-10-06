# -*- coding: ISO-8859-15 -*-
# =============================================================================
# Copyright (c) 2015 Tom Kralidis
#
# Authors : Tom Kralidis <tomkralidis@gmail.com>
#
# Contact email: tomkralidis@gmail.com
# =============================================================================

"""GM03 Core metadata parser http://www.geocat.ch/internet/geocat/en/home/documentation/gm03.html"""

from __future__ import (absolute_import, division, print_function)

from owslib import util
from owslib.etree import etree
from owslib.namespaces import Namespaces


# default variables
def get_namespaces():
    n = Namespaces()
    ns = n.get_namespaces(["gm03"])
    ns[None] = n.get_namespace("gm03")
    return ns

namespaces = get_namespaces()


class _GenericObject(object):
    """GM03 generic object type"""
    def __init__(self, md):
        """constructor"""

        self.tid = None

        if md is not None:
            self.tid = md.attrib.get('TID')


class _GenericObjectProperty(object):
    """GM03 generic object type"""
    def __init__(self, md):
        """constructor"""

        self.ref = None
        self.bid = None
        self.order_post = None

        if md is not None:
            self.ref = md.attrib.get('REF')
            self.bid = md.attrib.get('BID')
            self.order_pos = md.attrib.get('ORDER_POS')


class PT_Group(object):
    """PT_Group parser"""
    def __init__(self, md):
        """constructor"""

        self.language = util.testXMLValue(md.find(util.nspath_eval('gm03:language', namespaces)))
        self.country = util.testXMLValue(md.find(util.nspath_eval('gm03:country', namespaces)))
        self.character_set_code = util.testXMLValue(md.find(util.nspath_eval('gm03:characterSetCode', namespaces)))
        self.plain_text = util.testXMLValue(md.find(util.nspath_eval('gm03:plainText', namespaces)))
        self.plain_url = util.testXMLValue(md.find(util.nspath_eval('gm03:plainURL', namespaces)))


class PT_FreeText(object):
    """PT_FreeText parser"""
    def __init__(self, md):
        """constructor"""

        pt_groups = []
        for pt_group in md.findall(util.nspath_eval('gm03:GM03_2_1Core.Core.PT_FreeText/gm03:textGroup/gm03:GM03_2_1Core.Core.PT_Group', namespaces)):
            pt_groups.append(PT_Group(pt_group))

        self.pt_group = pt_groups


class PT_FreeURL(object):
    """PT_FreeURL parser"""
    def __init__(self, md):
        """constructor"""

        pt_groups = []
        for pt_group in md.findall(util.nspath_eval('gm03:GM03_2_1Core.Core.PT_FreeURL/gm03:URLGroup/gm03:GM03_2_1Core.Core.PT_URLGroup', namespaces)):
            pt_groups.append(PT_Group(pt_group))

        self.pt_group = pt_groups


class GM03(object):
    """TRANSFER parser"""
    def __init__(self, md):
        """constructor"""

        if hasattr(md, 'getroot'):  # standalone document
            self.xml = etree.tostring(md.getroot())
        else:  # part of a larger document
            self.xml = etree.tostring(md)

        self.header = HeaderSection(md)
        self.data = DataSection(md)


class HeaderSection(object):
    """HEADERSECTION parser"""
    def __init__(self, md):
        """constructor"""

        header = None
        header = md.find(util.nspath_eval('gm03:HEADERSECTION', namespaces))

        if header is None:
            return None

        self.version = header.attrib.get('VERSION')
        self.sender = header.attrib.get('SENDER')
        self.models = []

        for model in header.findall(util.nspath_eval('gm03:MODELS/gm03:MODEL', namespaces)):
            name = util.testXMLValue(model.find(util.nspath_eval('gm03:NAME', namespaces)))
            version = util.testXMLValue(model.find(util.nspath_eval('gm03:VERSION', namespaces)))
            uri = util.testXMLValue(model.find(util.nspath_eval('gm03:URI', namespaces)))
            model_dict = {
                'name': name,
                'version': version,
                'uri': uri
            }
            self.models.append(model_dict)

        self.comment = util.testXMLValue(header.find(util.nspath_eval('gm03:COMMENT', namespaces)))


class DataSection(object):
    """DATASECTION parser"""
    def __init__(self, md):
        """constructor"""

        section = None
        section = md.find(util.nspath_eval('gm03:DATASECTION', namespaces))

        if section is None:
            return None

        mdata = section.find(util.nspath_eval('gm03:GM03_2_1Core.Core', namespaces))
        if mdata is not None:
            self.core = Core(mdata)
        else:
            mdata = section.find(util.nspath_eval('gm03:GM03_2_1Comprehensive.Comprehensive', namespaces))
            if mdata is not None:
                self.comprehensive = Comprehensive(mdata)


class Core(object):
    """Core parser"""
    def __init__(self, md):
        """constructor"""

        self.bid = md.attrib.get('BID')

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.DQ_DataQuality', namespaces))
        if val is not None:
            self.data_quality = DQ_DataQuality(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.MD_Authority', namespaces))
        if val is not None:
            self.authority = _GenericObject(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.MD_DigitalTransferOptions', namespaces))
        if val is not None:
            self.digital_transfer_options = MD_DigitalTransferOptions(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.MD_Distribution', namespaces))
        if val is not None:
            self.distribution = _GenericObject(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.MD_ReferenceSystem', namespaces))
        if val is not None:
            self.reference_system_identifier = MD_ReferenceSystem(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.MD_Thesaurus', namespaces))
        if val is not None:
            self.thesaurus = MD_Thesaurus(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.SC_VerticalDatum', namespaces))
        if val is not None:
            self.vertical_datum = SC_VerticalDatum(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.CI_Address', namespaces))
        if val is not None:
            self.address = CI_Address(val)

        self.date = []
        for cid in md.findall(util.nspath_eval('gm03:GM03_2_1Core.Core.CI_Date', namespaces)):
            self.date.append(CI_Date(cid))

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.CI_Telephone', namespaces))
        if val is not None:
            self.telephone = CI_Telephone(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.DQ_Scope', namespaces))
        if val is not None:
            self.scope = DQ_Scope(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.EX_VerticalExtent', namespaces))
        if val is not None:
            self.vertical_extent = EX_VerticalExtent(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.MD_Format', namespaces))
        if val is not None:
            self.format = MD_Format(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.MD_Metadata', namespaces))
        if val is not None:
            self.metadata = MD_Metadata(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.MD_RepresentativeFraction', namespaces))
        if val is not None:
            self.representative_fraction = MD_RepresentativeFraction(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.MD_Resolution', namespaces))
        if val is not None:
            self.resolution = MD_Resolution(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.EX_BoundingPolygon', namespaces))
        if val is not None:
            self.bounding_polygon = EX_BoundingPolygon(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.EX_GeographicBoundingBox', namespaces))
        if val is not None:
            self.geographic_bounding_box = EX_GeographicBoundingBox(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.EX_GeographicDescription', namespaces))
        if val is not None:
            self.geographic_description = EX_GeographicDescription(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.EX_TemporalExtent', namespaces))
        if val is not None:
            self.temporal_extent = EX_TemporalExtent(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.MD_DistributiondistributionFormat', namespaces))
        if val is not None:
            self.distribution_distribution_format = MD_DistributiondistributionFormat(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.referenceSystemInfoMD_Metadata', namespaces))
        if val is not None:
            self.reference_system_metadata = referenceSystemInfoMD_Metadata(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.CI_Citation', namespaces))
        if val is None:
            val = md.find(util.nspath_eval('gm03:GM03_2_1Comprehensive.Comprehensive.CI_Citation', namespaces))
            if val is not None:
                self.citation = CI_Citation(val)
        else:
            self.citation = CI_Citation(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.CI_Contact', namespaces))
        if val is not None:
            self.contact = CI_Contact(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.CI_OnlineResource', namespaces))
        if val is not None:
            self.online_resource = CI_OnlineResource(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.CI_ResponsibleParty', namespaces))
        if val is not None:
            self.responsible_party = CI_ResponsibleParty(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.EX_Extent', namespaces))
        if val is not None:
            self.extent = EX_Extent(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.EX_SpatialTemporalExtent', namespaces))
        if val is not None:
            self.spatial_temporal_extent = EX_Extent(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.LI_Lineage', namespaces))
        if val is not None:
            self.lineage = LI_Lineage(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.MD_Identifier', namespaces))
        if val is not None:
            self.identifier = MD_Identifier(val)

        self.keywords = []
        for kw in md.findall(util.nspath_eval('gm03:GM03_2_1Core.Core.MD_Keywords', namespaces)):
            self.keywords.append(MD_Keywords(kw))

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.MD_DataIdentification', namespaces))
        if val is None:
            val = md.find(util.nspath_eval('gm03:GM03_2_1Comprehensive.Comprehensive.MD_DataIdentification', namespaces))
            if val is not None:
                self.data_identification = MD_DataIdentification(val)
        else:
            self.data_identification = MD_DataIdentification(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.RS_Identifier', namespaces))
        if val is not None:
            self.rs_identifier = RS_Identifier(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.CI_ResponsiblePartyparentinfo', namespaces))
        if val is not None:
            self.responsible_party_parent_info = CI_ResponsiblePartyparentinfo(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.descriptiveKeywordsMD_Identification', namespaces))
        if val is not None:
            self.descriptive_keywords_identification = descriptiveKeywordsMD_Identification(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.EX_ExtentgeographicElement', namespaces))
        if val is not None:
            self.extent_geographic_element = EX_ExtentgeographicElement(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.EX_ExtenttemporalElement', namespaces))
        if val is not None:
            self.extent_temporal_element = EX_ExtenttemporalElement(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.EX_ExtentverticalElement', namespaces))
        if val is not None:
            self.extent_vertical_element = EX_ExtenttemporalElement(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.MD_IdentificationpointOfContact', namespaces))
        if val is not None:
            self.identification_point_of_contact = MD_IdentificationpointOfContact(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.MD_Metadatacontact', namespaces))
        if val is not None:
            self.metadata_point_of_contact = MD_Metadatacontact(val)

        val = md.find(util.nspath_eval('gm03:GM03_2_1Core.Core.spatialExtentEX_SpatialTemporalExtent', namespaces))
        if val is not None:
            self.self.spatial_temporal_extent = spatialExtentEX_SpatialTemporalExtent(val)

    @property
    def elements(self):
        """helper function to return all properties"""

        dict_elements = vars(self)
        if 'bid' in dict_elements:
            del dict_elements['bid']
        return dict_elements

    def get_element_by_tid(self, tid):
        """helper function to find values by reference"""

        for key, value in self.elements.items():
            if hasattr(value, 'tid') and value.tid == tid:
                return self.elements[key]
        return None


class DQ_DataQuality(_GenericObject):
    """DQ_DataQuality parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        val = md.find(util.nspath_eval('gm03:MD_Metadata', namespaces))
        self.metadata = _GenericObjectProperty(val)


class MD_Authority(_GenericObject):
    """MD_Authority parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)


class MD_ReferenceSystem(_GenericObject):
    """MD_ReferenceSystem parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        val = md.find(util.nspath_eval('gm03:referenceSystemIdentifier', namespaces))
        self.reference_system_identifier = _GenericObjectProperty(val)


class MD_DigitalTransferOptions(_GenericObject):
    """MD_DigitalTransferOptions parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        val = md.find(util.nspath_eval('gm03:MD_Distribution', namespaces))
        self.distribution = _GenericObjectProperty(val)


class MD_Thesaurus(_GenericObject):
    """MD_Thesaurus parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        val = md.find(util.nspath_eval('gm03:citation', namespaces))
        self.citation = _GenericObjectProperty(val)


class SC_VerticalDatum(_GenericObject):
    """SC_VerticalDatum parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        val = md.find(util.nspath_eval('gm03:datumID', namespaces))
        self.datum_id = _GenericObjectProperty(val)


class CI_Address(_GenericObject):
    """CI_Address parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        self.street_name = util.testXMLValue(md.find(util.nspath_eval('gm03:streetName', namespaces)))
        self.street_number = util.testXMLValue(md.find(util.nspath_eval('gm03:streetNumber', namespaces)))
        self.address_line = util.testXMLValue(md.find(util.nspath_eval('gm03:addressLine', namespaces)))
        self.post_box = util.testXMLValue(md.find(util.nspath_eval('gm03:postBox', namespaces)))
        self.postal_code = util.testXMLValue(md.find(util.nspath_eval('gm03:postalCode', namespaces)))
        self.city = util.testXMLValue(md.find(util.nspath_eval('gm03:city', namespaces)))
        self.administrative_area = util.testXMLValue(md.find(util.nspath_eval('gm03:administrativeArea', namespaces)))
        self.country = util.testXMLValue(md.find(util.nspath_eval('gm03:country', namespaces)))


class CI_Date(_GenericObject):
    """CI_Date parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        self.date = util.testXMLValue(md.find(util.nspath_eval('gm03:date', namespaces)))
        self.date_type = util.testXMLValue(md.find(util.nspath_eval('gm03:dateType', namespaces)))

        val = md.find(util.nspath_eval('gm03:CI_Citation', namespaces))
        self.citation = _GenericObjectProperty(val)


class CI_Telephone(_GenericObject):
    """CI_Telephone parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        self.number = util.testXMLValue(md.find(util.nspath_eval('gm03:number', namespaces)))
        self.number_type = util.testXMLValue(md.find(util.nspath_eval('gm03:numberType', namespaces)))

        val = md.find(util.nspath_eval('gm03:CI_ResponsibleParty', namespaces))
        self.responsible_party = _GenericObjectProperty(val)


class DQ_Scope(_GenericObject):
    """DQ_Scope parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        self.level = util.testXMLValue(md.find(util.nspath_eval('gm03:level', namespaces)))

        val = md.find(util.nspath_eval('gm03:DQ_DataQuality', namespaces))
        self.data_quality = _GenericObjectProperty(val)


class EX_VerticalExtent(_GenericObject):
    """EX_VerticalExtent parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        self.minimum_value = util.testXMLValue(md.find(util.nspath_eval('gm03:minimumValue', namespaces)))
        self.maximum_value = util.testXMLValue(md.find(util.nspath_eval('gm03:maximumValue', namespaces)))
        self.unit_of_measure = util.testXMLValue(md.find(util.nspath_eval('gm03:unitOfMeasure', namespaces)))

        val = md.find(util.nspath_eval('gm03:verticalDatum', namespaces))
        self.vertical_datum = _GenericObjectProperty(val)


class MD_Format(_GenericObject):
    """MD_Format parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        self.name = util.testXMLValue(md.find(util.nspath_eval('gm03:name', namespaces)))
        self.version = util.testXMLValue(md.find(util.nspath_eval('gm03:version', versionspaces)))


class MD_Metadata(_GenericObject):
    """MD_Metadata parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        self.file_identifier = util.testXMLValue(md.find(util.nspath_eval('gm03:fileIdentifier', namespaces)))
        self.language = util.testXMLValue(md.find(util.nspath_eval('gm03:language', namespaces)))
        self.character_set = util.testXMLValue(md.find(util.nspath_eval('gm03:characterSet', namespaces)))
        self.date_stamp = util.testXMLValue(md.find(util.nspath_eval('gm03:dateStamp', namespaces)))
        self.metadata_standard_name = util.testXMLValue(md.find(util.nspath_eval('gm03:metadataStandardName', namespaces)))
        self.metadata_standard_version = util.testXMLValue(md.find(util.nspath_eval('gm03:metadataStandardVersion', namespaces)))
        self.dataset_uri = util.testXMLValue(md.find(util.nspath_eval('gm03:dataSetURI', namespaces)))

        val = md.find(util.nspath_eval('gm03:hierarchyLevel', namespaces))
        if val is not None:
            values = []
            for value in val.findall(util.nspath_eval('gm03:GM03_2_1Core.Core.MD_ScopeCode_/gm03:value', namespaces)):
                values.append(util.testXMLValue(value))
            self.hierarchy_level = values

        val = md.find(util.nspath_eval('gm03:hierarchyLevelName', namespaces))
        if val is not None:
            values = []
            for value in val.findall(util.nspath_eval('gm03:GM03_2_1Core.Core.CharacterString_/gm03:value', namespaces)):
                values.append(util.testXMLValue(value))
            self.hierarchy_level_name = values

        val = md.find(util.nspath_eval('gm03:distributionInfo', namespaces))
        self.distribution_info = _GenericObjectProperty(val)

        val = md.find(util.nspath_eval('gm03:parentIdentifier', namespaces))
        self.parent_identifier = _GenericObjectProperty(val)


class MD_RepresentativeFraction(_GenericObject):
    """MD_RepresentativeFraction parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        self.denominator = util.testXMLValue(md.find(util.nspath_eval('gm03:denominator', namespaces)))


class MD_Resolution(_GenericObject):
    """MD_Resolution parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        self.distance = util.testXMLValue(md.find(util.nspath_eval('gm03:distance', namespaces)))

        val = md.find(util.nspath_eval('gm03:MD_DataIdentification', namespaces))
        self.data_identification = _GenericObjectProperty(val)

        val = md.find(util.nspath_eval('gm03:equivalentScale', namespaces))
        self.equivalent_scale = _GenericObjectProperty(val)


class MD_ScopeDescription(_GenericObject):
    """MD_ScopeDescription parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        self.attributes = util.testXMLValue(md.find(util.nspath_eval('gm03:attributes', namespaces)))
        self.features = util.testXMLValue(md.find(util.nspath_eval('gm03:features', namespaces)))
        self.feature_instances = util.testXMLValue(md.find(util.nspath_eval('gm03:featureInstances', namespaces)))
        self.attribute_instances = util.testXMLValue(md.find(util.nspath_eval('gm03:attributeInstances', namespaces)))
        self.dataset = util.testXMLValue(md.find(util.nspath_eval('gm03:dataset', namespaces)))
        self.other = util.testXMLValue(md.find(util.nspath_eval('gm03:other', namespaces)))

        val = md.find(util.nspath_eval('gm03:DQ_Scope', namespaces))
        self.scope = _GenericObjectProperty(val)


class EX_BoundingPolygon(_GenericObject):
    """EX_BoundingPolygon parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        self.extent_type_code = util.testXMLValue(md.find(util.nspath_eval('gm03:extentTypeCode', namespaces)))

        boundaries = []

        for boundary in md.findall(util.nspath_eval('gm03:SURFACE/gm03:BOUNDARY', namespaces)):
            polylines = []
            for polyline in boundary.findall(util.nspath_eval('gm03:POLYLINE', namespaces)):
                coords = []
                arcs = []
                for coord in polyline.findall(util.nspath_eval('gm03:COORD', namespaces)):
                    c1 = util.testXMLValue(coord.find(util.nspath_eval('gm03:C1', namespaces)))
                    c2 = util.testXMLValue(coord.find(util.nspath_eval('gm03:C2', namespaces)))
                    c3 = util.testXMLValue(coord.find(util.nspath_eval('gm03:C3', namespaces)))
                    coordvalue = {'c1': c1, 'c2': c2, 'c3': c3}
                    coords.append(coordvalue)
                for arc in polyline.findall(util.nspath_eval('gm03:ARC', namespaces)):
                    c1 = util.testXMLValue(coord.find(util.nspath_eval('gm03:C1', namespaces)))
                    c2 = util.testXMLValue(coord.find(util.nspath_eval('gm03:C2', namespaces)))
                    c3 = util.testXMLValue(coord.find(util.nspath_eval('gm03:C3', namespaces)))
                    a1 = util.testXMLValue(coord.find(util.nspath_eval('gm03:A1', namespaces)))
                    a2 = util.testXMLValue(coord.find(util.nspath_eval('gm03:A2', namespaces)))
                    r = util.testXMLValue(coord.find(util.nspath_eval('gm03:R', namespaces)))
                    arcpoint = {'c1': c1, 'c2': c2, 'c3': c3, 'a1': a1, 'a2': a2, 'r': r}
                    arcs.append(arcpoint)
                polylines.append(coords)
                polylines.append(arcs)
            boundaries.append(polylines)
        self.boundary = boundaries


class EX_GeographicBoundingBox(_GenericObject):
    """EX_GeographicBoundingBox parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        self.extent_type_code = util.testXMLValue(md.find(util.nspath_eval('gm03:extentTypeCode', namespaces)))
        self.north_bound_latitude = util.testXMLValue(md.find(util.nspath_eval('gm03:northBoundLatitude', namespaces)))
        self.south_bound_latitude = util.testXMLValue(md.find(util.nspath_eval('gm03:southBoundLatitude', namespaces)))
        self.east_bound_longitude = util.testXMLValue(md.find(util.nspath_eval('gm03:eastBoundLongitude', namespaces)))
        self.west_bound_longitude = util.testXMLValue(md.find(util.nspath_eval('gm03:westBoundLongitude', namespaces)))


class EX_GeographicDescription(_GenericObject):
    """EX_GeographicDescription parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        self.extent_type_code = util.testXMLValue(md.find(util.nspath_eval('gm03:extentTypeCode', namespaces)))

        val = md.find(util.nspath_eval('gm03:geographicIdentifier', namespaces))
        self.geographic_identifier = _GenericObjectProperty(val)


class EX_TemporalExtent(_GenericObject):
    """EX_TemporalExtent parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        begin = util.testXMLValue(md.find(util.nspath_eval('gm03:extent/gm03:GM03_2_1Core.Core.TM_Primitive/begin', namespaces)))
        end = util.testXMLValue(md.find(util.nspath_eval('gm03:extent/gm03:GM03_2_1Core.Core.TM_Primitive/end', namespaces)))

        self.extent = {'begin': begin, 'end': end}


class MD_DistributiondistributionFormat(object):
    """MD_DistributiondistributionFormat parser"""
    def __init__(self, md):
        """constructor"""

        val = md.find(util.nspath_eval('gm03:MD_Distribution', namespaces))
        self.distribution = _GenericObjectProperty(val)

        val = md.find(util.nspath_eval('gm03:distributionFormat', namespaces))
        self.distribution_format = _GenericObjectProperty(val)


class referenceSystemInfoMD_Metadata(object):
    """referenceSystemInfoMD_Metadata parser"""
    def __init__(self, md):
        """constructor"""

        val = md.find(util.nspath_eval('gm03:referenceSystemInfo', namespaces))
        self.reference_system_info = _GenericObjectProperty(val)

        val = md.find(util.nspath_eval('gm03:MD_Metadata', namespaces))
        self.metadata = _GenericObjectProperty(val)


class CI_Citation(_GenericObject):
    """CI_Citation parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        val = md.find(util.nspath_eval('gm03:title', namespaces))
        if val is not None:
            self.title = PT_FreeText(val)

        val = md.find(util.nspath_eval('gm03:MD_Authority', namespaces))
        self.authority = _GenericObjectProperty(val)


class CI_Contact(_GenericObject):
    """CI_Contact parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        self.hours_of_service = util.testXMLValue(md.find(util.nspath_eval('gm03:hoursOfService', namespaces)))

        val = md.find(util.nspath_eval('gm03:contactInstructions', namespaces))
        if val is not None:
            self.contact_instructions = PT_FreeText(val)


class CI_OnlineResource(_GenericObject):
    """CI_OnlineResource parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        self.protocol = util.testXMLValue(md.find(util.nspath_eval('gm03:protocol', namespaces)))
        self.application_profile = util.testXMLValue(md.find(util.nspath_eval('gm03:applicationProfile', namespaces)))
        self.function = util.testXMLValue(md.find(util.nspath_eval('gm03:function', namespaces)))

        val = md.find(util.nspath_eval('gm03:description', namespaces))
        if val is not None:
            self.description = PT_FreeText(val)

        val = md.find(util.nspath_eval('gm03:name', namespaces))
        if val is not None:
            self.name = PT_FreeText(val)

        val = md.find(util.nspath_eval('gm03:linkage', namespaces))
        if val is not None:
            self.linkage = PT_FreeURL(val)

        val = md.find(util.nspath_eval('gm03:MD_DigitalTransferOptions', namespaces))
        self.digital_transfer_options = _GenericObjectProperty(val)


class CI_ResponsibleParty(_GenericObject):
    """CI_ResponsibleParty parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        self.individual_first_name = util.testXMLValue(md.find(util.nspath_eval('gm03:individualFirstName', namespaces)))
        self.individual_last_name = util.testXMLValue(md.find(util.nspath_eval('gm03:individualLastName', namespaces)))

        val = md.find(util.nspath_eval('gm03:electronicalMailAddress', namespaces))
        if val is not None:
            self.electronical_mail_address = util.testXMLValue(val.find(util.nspath_eval('gm03:GM03_2_1Core.Core.URL_/gm03:value', namespaces)))

        val = md.find(util.nspath_eval('gm03:organisationName', namespaces))
        if val is not None:
            self.organisation_name = PT_FreeText(val)

        val = md.find(util.nspath_eval('gm03:positionName', namespaces))
        if val is not None:
            self.position_name = PT_FreeText(val)

        val = md.find(util.nspath_eval('gm03:organisationAcronym', namespaces))
        if val is not None:
            self.organisation_acronym = PT_FreeText(val)

        val = md.find(util.nspath_eval('gm03:linkage', namespaces))
        if val is not None:
            self.linkage = PT_FreeURL(val)

        val = md.find(util.nspath_eval('gm03:address', namespaces))
        self.address = _GenericObjectProperty(val)

        val = md.find(util.nspath_eval('gm03:contactInfo', namespaces))
        self.contact_info = _GenericObjectProperty(val)


class EX_Extent(_GenericObject):
    """EX_Extent parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        val = md.find(util.nspath_eval('gm03:description', namespaces))
        if val is not None:
            self.description = PT_FreeText(val)

        val = md.find(util.nspath_eval('gm03:MD_DataIdentification', namespaces))
        self.data_identification = _GenericObjectProperty(val)


class EX_SpatialTemporalExtent(EX_TemporalExtent):
    """EX_SpatialTemporalExtent parser"""
    def __init__(self, md):
        """constructor"""

        EX_TemporalExtent.__init__(self, md)


class LI_Lineage(_GenericObject):
    """LI_Lineage parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        val = md.find(util.nspath_eval('gm03:statement', namespaces))
        if val is not None:
            self.statement = PT_FreeText(val)

        val = md.find(util.nspath_eval('gm03:DQ_DataQuality', namespaces))
        self.data_quality = _GenericObjectProperty(val)


class MD_Identifier(_GenericObject):
    """MD_Identifier parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        val = md.find(util.nspath_eval('gm03:code', namespaces))
        if val is not None:
            self.code = PT_FreeText(val)

        val = md.find(util.nspath_eval('gm03:MD_Authority', namespaces))
        self.authority = _GenericObjectProperty(val)


class MD_Keywords(_GenericObject):
    """MD_Keywords parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        self.type = util.testXMLValue(md.find(util.nspath_eval('gm03:type', namespaces)))

        val = md.find(util.nspath_eval('gm03:keyword', namespaces))
        if val is not None:
            self.keyword = PT_FreeText(val)

        val = md.find(util.nspath_eval('gm03:thesaurus', namespaces))
        self.thesaurus = _GenericObjectProperty(val)


class MD_DataIdentification(_GenericObject):
    """MD_DataIdentification parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        val = md.find(util.nspath_eval('gm03:status', namespaces))
        if val is not None:
            self.status = util.testXMLValue(val.find(util.nspath_eval('gm03:GM03_2_1Core.Core.MD_ProgressCode_/gm03:value', namespaces)))

        val = md.find(util.nspath_eval('gm03:abstract', namespaces))
        if val is not None:
            self.abstract = PT_FreeText(val)

        val = md.find(util.nspath_eval('gm03:purpose', namespaces))
        if val is not None:
            self.purpose = PT_FreeText(val)

        val = md.find(util.nspath_eval('gm03:MD_Metadata', namespaces))
        self.metadata = _GenericObjectProperty(val)

        val = md.find(util.nspath_eval('gm03:citation', namespaces))
        self.citation = _GenericObjectProperty(val)

        val = md.find(util.nspath_eval('gm03:spatialRepresentationType', namespaces))
        if val is not None:
            self.spatial_representation_type = util.testXMLValue(val.find(util.nspath_eval('gm03:GM03_2_1Core.Core.MD_SpatialRepresentationTypeCode_/gm03:value', namespaces)))

        val = md.find(util.nspath_eval('gm03:language', namespaces))
        if val is not None:
            self.language = util.testXMLValue(val.find(util.nspath_eval('gm03:CodeISO.LanguageCodeISO_/gm03:value', namespaces)))

        val = md.find(util.nspath_eval('gm03:characterSet', namespaces))
        if val is not None:
            self.character_set = util.testXMLValue(val.find(util.nspath_eval('gm03:GM03_2_1Core.Core.MD_CharacterSetCode_/gm03:value', namespaces)))

        val = md.find(util.nspath_eval('gm03:topicCategory', namespaces))
        if val is not None:
            self.topic_category = util.testXMLValue(val.find(util.nspath_eval('gm03:GM03_2_1Core.Core.MD_TopicCategoryCode_/gm03:value', namespaces)))


class RS_Identifier(_GenericObject):
    """RS_Identifier parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        val = md.find(util.nspath_eval('gm03:code', namespaces))
        if val is not None:
            self.code = PT_FreeText(val)

        val = md.find(util.nspath_eval('gm03:MD_Authority', namespaces))
        self.authority = _GenericObjectProperty(val)


class CI_ResponsiblePartyparentinfo(_GenericObject):
    """CI_ResponsiblePartyparentinfo parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        val = md.find(util.nspath_eval('gm03:parentResponsibleParty', namespaces))
        self.parent_responsible_party = _GenericObjectProperty(val)

        val = md.find(util.nspath_eval('gm03:CI_ResponsibleParty', namespaces))
        self.responsible_party = _GenericObjectProperty(val)


class descriptiveKeywordsMD_Identification(_GenericObject):
    """descriptiveKeywordsMD_Identification parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        val = md.find(util.nspath_eval('gm03:descriptiveKeywords', namespaces))
        self.descriptive_keywords = _GenericObjectProperty(val)

        val = md.find(util.nspath_eval('gm03:MD_Identification', namespaces))
        self.identification = _GenericObjectProperty(val)


class EX_ExtentgeographicElement(_GenericObject):
    """EX_ExtentgeographicElement parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        val = md.find(util.nspath_eval('gm03:EX_Extent', namespaces))
        self.extent = _GenericObjectProperty(val)

        val = md.find(util.nspath_eval('gm03:temporalElement', namespaces))
        self.temporal_element = _GenericObjectProperty(val)


class EX_ExtentverticalElement(_GenericObject):
    """EX_ExtentverticalElement parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        val = md.find(util.nspath_eval('gm03:EX_Extent', namespaces))
        self.extent = _GenericObjectProperty(val)

        val = md.find(util.nspath_eval('gm03:verticalElement', namespaces))
        self.vertical_element = _GenericObjectProperty(val)


class EX_ExtentgeographicElement(_GenericObject):
    """EX_ExtentgeographicElement parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        val = md.find(util.nspath_eval('gm03:EX_Extent', namespaces))
        self.extent = _GenericObjectProperty(val)

        val = md.find(util.nspath_eval('gm03:geographicElement', namespaces))
        self.geographic_element = _GenericObjectProperty(val)


class MD_IdentificationpointOfContact(_GenericObject):
    """MD_IdentificationpointOfContact parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        val = md.find(util.nspath_eval('gm03:pointOfContact', namespaces))
        self.point_of_contact = _GenericObjectProperty(val)

        val = md.find(util.nspath_eval('gm03:MD_Identification', namespaces))
        self.identification = _GenericObjectProperty(val)

        val = md.find(util.nspath_eval('gm03:role', namespaces))
        if val is not None:
            self.role = util.testXMLValue(val.find(util.nspath_eval('gm03:GM03_2_1Core.Core.CI_RoleCode_/gm03:value', namespaces)))


class MD_Metadatacontact(_GenericObject):
    """MD_Metadatacontact parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        val = md.find(util.nspath_eval('gm03:contact', namespaces))
        self.contact = _GenericObjectProperty(val)

        val = md.find(util.nspath_eval('gm03:MD_Metadata', namespaces))
        self.metadata = _GenericObjectProperty(val)

        val = md.find(util.nspath_eval('gm03:role', namespaces))
        if val is not None:
            self.role = util.testXMLValue(val.find(util.nspath_eval('gm03:GM03_2_1Core.Core.CI_RoleCode_/gm03:value', namespaces)))


class spatialExtentEX_SpatialTemporalExtent(_GenericObject):
    """spatialExtentEX_SpatialTemporalExtent parser"""
    def __init__(self, md):
        """constructor"""

        _GenericObject.__init__(self, md)

        val = md.find(util.nspath_eval('gm03:spatialExtent', namespaces))
        self.spatial_extent = _GenericObjectProperty(val)

        val = md.find(util.nspath_eval('gm03:EX_SpatialTemporalExtent', namespaces))
        self.spatial_temporal_extent = _GenericObjectProperty(val)


class Comprehensive(Core):
    """Comprehensive parser"""
    def __init__(self, md):
        """constructor"""

        Core.__init__(self, md)
