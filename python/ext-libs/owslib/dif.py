# -*- coding: ISO-8859-15 -*-
# =============================================================================
# Copyright (c) 2009 Tom Kralidis
#
# Authors : Tom Kralidis <tomkralidis@gmail.com>
#
# Contact email: tomkralidis@gmail.com
# =============================================================================

""" DIF metadata parser """

from owslib.etree import etree
from owslib import util
from owslib.namespaces import Namespaces

# default variables
def get_namespaces():
    n = Namespaces()
    ns = n.get_namespaces("dif")
    ns[None] = n.get_namespace("dif")
    return ns
namespaces = get_namespaces()

class DIF(object):
    """ Process DIF """
    def __init__(self, md):
        val = md.find(util.nspath_eval('dif:Entry_ID', namespaces))
        self.identifier = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Entry_Title', namespaces))
        self.title = util.testXMLValue(val)

        self.citation = []
        for el in md.findall(util.nspath_eval('dif:Data_Set_Citation', namespaces)):
            self.citation.append(Citation(el))

        self.personnel = []
        for el in md.findall(util.nspath_eval('dif:Personnel', namespaces)):
            self.personnel.append(util.testXMLValue(el))

        self.discipline = []
        for el in md.findall(util.nspath_eval('dif:Discipline', namespaces)):
            self.discipline.append(util.testXMLValue(el))

        self.parameters= []
        for el in md.findall(util.nspath_eval('dif:Parameters', namespaces)):
            self.parameters.append(util.testXMLValue(el))

        self.iso_topic_category  = []
        for el in md.findall(util.nspath_eval('dif:ISO_Topic_Category', namespaces)):
            self.iso_topic_category.append(util.testXMLValue(el))

        self.keyword = []
        for el in md.findall(util.nspath_eval('dif:Keyword', namespaces)):
            self.keyword.append(util.testXMLValue(el))

        self.sensor_name = []
        for el in md.findall(util.nspath_eval('dif:Sensor_Name', namespaces)):
            self.sensor_name.append(Name(el))

        self.source_name = []
        for el in md.findall(util.nspath_eval('dif:Source_Name', namespaces)):
            self.source_name.append(Name(el))

        self.temporal_coverage = []
        for el in md.findall(util.nspath_eval('dif:Temporal_Coverage', namespaces)):
            self.temporal_coverage.append(Temporal_Coverage(el))

        self.paleo_temporal_coverage = []
        for el in md.findall(util.nspath_eval('dif:Paleo_Temporal_Coverage', namespaces)):
            self.paleo_temporal_coverage.append(Paleo_Temporal_Coverage(el))

        self.data_set_progress = []
        for el in md.findall(util.nspath_eval('dif:Data_Set_Progress', namespaces)):
            self.data_set_progress.append(util.testXMLValue(el))

        self.spatial_coverage = []
        for el in md.findall(util.nspath_eval('dif:Spatial_Coverage', namespaces)):
            self.spatial_coverage.append(Spatial_Coverage(el))

        self.location = []
        for el in md.findall(util.nspath_eval('dif:location', namespaces)):
            self.location.append(util.testXMLValue(el))

        self.data_resolution = []
        for el in md.findall(util.nspath_eval('dif:Data_Resolution', namespaces)):
            self.data_resolution.append(Data_Resolution(el))

        self.project = []
        for el in md.findall(util.nspath_eval('dif:Project', namespaces)):
            self.project.append(Name(el))

        val = md.find(util.nspath_eval('dif:Quality', namespaces))
        self.quality = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Access_Constraints', namespaces))
        self.access_constraints = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Use_Constraints', namespaces))
        self.use_constraints = util.testXMLValue(val)

        self.language = []
        for el in md.findall(util.nspath_eval('dif:Data_Set_Language', namespaces)):
            self.language.append(util.testXMLValue(el))

        self.originating_center = []
        for el in md.findall(util.nspath_eval('dif:Originating_Center', namespaces)):
            self.originating_center.append(util.testXMLValue(el))

        self.data_center = []
        for el in md.findall(util.nspath_eval('dif:Data_Center', namespaces)):         
            self.data_center.append(Data_Center(el))

        self.distribution = []
        for el in md.findall(util.nspath_eval('dif:Distribution', namespaces)):     
            self.distribution.append(Distribution(el))

        self.multimedia_sample = []
        for el in md.findall(util.nspath_eval('dif:Multimedia_Sample', namespaces)):     
            self.multimedia_sample.append(Multimedia_Sample(el))

        val = md.find(util.nspath_eval('dif:Reference', namespaces))
        self.reference = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Summary', namespaces))
        self.summary = util.testXMLValue(val)

        self.related_url = []
        for el in md.findall(util.nspath_eval('dif:Related_URL', namespaces)):
            self.related_url.append(Related_URL(el))

        self.parent_dif = []
        for el in md.findall(util.nspath_eval('dif:Parent_DIF', namespaces)):
            self.parent_dif.append(util.testXMLValue(el))

        self.idn_node = []
        for el in md.findall(util.nspath_eval('dif:IDN_Node', namespaces)):
            self.idn_node.append(Name(el))

        val = md.find(util.nspath_eval('dif:Originating_Metadata_Node', namespaces))
        self.originating_metadata_node = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Metadata_Name', namespaces))
        self.metadata_name = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Metadata_Version', namespaces))
        self.metadata_version = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:DIF_Creation_Date', namespaces))
        self.dif_creation_date = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Last_DIF_Revision_Date', namespaces))
        self.last_dif_revision_date = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Future_DIF_Review_Date', namespaces))
        self.future_dif_review_date = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Private', namespaces))
        self.private = util.testXMLValue(val)

class Citation(object):
    """ Parse Data_Set_Citation """
    def __init__(self, el):
        val = el.find(util.nspath_eval('dif:Dataset_Creator', namespaces))
        self.creator = util.testXMLValue(val)

        val = el.find(util.nspath_eval('dif:Dataset_Title', namespaces))
        self.title = util.testXMLValue(val)

        val = el.find(util.nspath_eval('dif:Dataset_Series_Name', namespaces))
        self.series_name = util.testXMLValue(val)

        val = el.find(util.nspath_eval('dif:Dataset_Release_Date', namespaces))
        self.release_date = util.testXMLValue(val)

        val = el.find(util.nspath_eval('dif:Dataset_Release_Place', namespaces))
        self.release_place = util.testXMLValue(val)

        val = el.find(util.nspath_eval('dif:Dataset_Publisher', namespaces))
        self.publisher = util.testXMLValue(val)

        val = el.find(util.nspath_eval('dif:Version', namespaces))
        self.version = util.testXMLValue(val)

        val = el.find(util.nspath_eval('dif:Issue_Identification', namespaces))
        self.issue_identification = util.testXMLValue(val)

        val = el.find(util.nspath_eval('dif:Data_Presentation_Form', namespaces))
        self.presentation_form = util.testXMLValue(val)

        val = el.find(util.nspath_eval('dif:Other_Citation_Details', namespaces))
        self.details = util.testXMLValue(val)

        val = el.find(util.nspath_eval('dif:Online_Resource', namespaces))
        self.onlineresource = util.testXMLValue(val)

class Personnel(object):
    """ Process Personnel """
    def __init__(self, md):
        self.role = []
        for el in md.findall(util.nspath_eval('dif:Role', namespaces)):
            self.role.append(util.testXMLValue(el))

        val = md.find(util.nspath_eval('dif:First_Name', namespaces))
        self.first_name = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Middle_Name', namespaces))
        self.middle_name = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Last_Name', namespaces))
        self.last_name = util.testXMLValue(val)

        self.email = []
        for el in md.findall(util.nspath_eval('dif:Email', namespaces)):
            self.email.append(util.testXMLValue(el))

        self.phone = []
        for el in md.findall(util.nspath_eval('dif:Phone', namespaces)):
            self.phone.append(util.testXMLValue(el))

        self.fax = []
        for el in md.findall(util.nspath_eval('dif:Fax', namespaces)):
            self.fax.append(util.testXMLValue(el))

        val = md.find(util.nspath_eval('dif:Contact_Address', namespaces))
        self.contact_address = Contact_Address(val)

class Contact_Address(object):
    """ Process Contact_Address """
    def __init__(self, md):
        self.address = []
        for el in md.findall(util.nspath_eval('dif:Address', namespaces)):
            self.address.append(util.testXMLValue(el))

        val = md.find(util.nspath_eval('dif:City', namespaces))
        self.city = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Province_or_State', namespaces))
        self.province_or_state = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Postal_Code', namespaces))
        self.postal_code = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Country', namespaces))
        self.country = util.testXMLValue(val)

class Discipline(object):
    """ Process Discipline """
    def __init__(self, md):
        val = md.find(util.nspath_eval('dif:Discipline_Name', namespaces))
        self.name = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Subdiscipline', namespaces))
        self.subdiscipline = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Detailed_Subdiscipline', namespaces))
        self.detailed_subdiscipline = util.testXMLValue(val)

class Parameters(object):
    """ Process Parameters """
    def __init__(self, md):
        val = md.find(util.nspath_eval('dif:Category', namespaces))
        self.category = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Topic', namespaces))
        self.topic = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Term', namespaces))
        self.term = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Variable_Level_1', namespaces))
        self.variable_l1 = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Variable_Level_2', namespaces))
        self.variable_l2 = util.testXMLValue(val)
    
        val = md.find(util.nspath_eval('dif:Variable_Level_3', namespaces))
        self.variable_l3 = util.testXMLValue(val)
 
        val = md.find(util.nspath_eval('dif:Detailed_Variable', namespaces))
        self.detailed_variable = util.testXMLValue(val)

class Name(object):
    """ Process Sensor_Name, Source_Name, Project, IDN_Node """
    def __init__(self, md):
        val = md.find(util.nspath_eval('dif:Short_Name', namespaces))
        self.short_name = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Long_Name', namespaces))
        self.long_name = util.testXMLValue(val)

class Temporal_Coverage(object):
    """ Process Temporal_Coverage """
    def __init__(self, md):
        val = md.find(util.nspath_eval('dif:Start_Date', namespaces))
        self.start_date = util.testXMLValue(val)
        
        val = md.find(util.nspath_eval('dif:End_Date', namespaces))
        self.end_date = util.testXMLValue(val)

class Paleo_Temporal_Coverage(object):
    """ Process Paleo_Temporal_Coverage """
    def __init__(self, md):
        val = md.find(util.nspath_eval('dif:Paleo_Start_Date', namespaces))
        self.paleo_start_date = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Paleo_End_Date', namespaces))
        self.paleo_end_date = util.testXMLValue(val)

        self.chronostratigraphic_unit = []
        for el in md.findall(util.nspath_eval('dif:Chronostratigraphic_Unit', namespaces)):
            self.chronostratigraphic_unit.append(Chronostratigraphic_Unit(el))

class Chronostratigraphic_Unit(object):
    """ Process Chronostratigraphic_Unit """
    def __init__(self, md):
        val = md.find(util.nspath_eval('dif:Eon', namespaces))
        self.eon = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Era', namespaces))
        self.era = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Period', namespaces))
        self.period = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Epoch', namespaces))
        self.epoch = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Stage', namespaces))
        self.stage = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Detailed_Classification', namespaces))
        self.detailed_classification = util.testXMLValue(val)

class Spatial_Coverage(object):
    """ Process Spatial_Coverage """
    def __init__(self, md):
        val = md.find(util.nspath_eval('dif:Southernmost_Latitude', namespaces))
        self.miny = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Northernmost_Latitude', namespaces))
        self.maxy = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Westernmost_Latitude', namespaces))
        self.minx = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Easternmost_Latitude', namespaces))
        self.maxx = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Minimum_Altitude', namespaces))
        self.minz = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Maximum_Altitude', namespaces))
        self.maxz = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Minimum_Depth', namespaces))
        self.mindepth = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Maximum_Depth', namespaces))
        self.maxdepth = util.testXMLValue(val)

class Location(object):
    """ Process Location """
    def __init__(self, md):
        val = md.find(util.nspath_eval('dif:Location_Category', namespaces))
        self.category = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Location_Category', namespaces))
        self.type = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Location_Subregion1', namespaces))
        self.subregion1 = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Location_Subregion2', namespaces))
        self.subregion2 = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Location_Subregion3', namespaces))
        self.subregion3 = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Detailed_Location', namespaces))
        self.detailed_location = util.testXMLValue(val)

class Data_Resolution(object):
    """ Process Data_Resolution"""
    def __init__(self, md):
        val = md.find(util.nspath_eval('dif:Latitude_Resolution', namespaces))
        self.y = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Longitude_Resolution', namespaces))
        self.x = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Horizontal_Resolution_Range', namespaces))
        self.horizontal_res_range = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Vertical_Resolution', namespaces))
        self.vertical_res = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Vertical_Resolution_Range', namespaces))
        self.vertical_res_range = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Temporal_Resolution', namespaces))
        self.temporal_res = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Temporal_Resolution_Range', namespaces))
        self.temporal_res_range = util.testXMLValue(val)

class Data_Center(object):
    """ Process Data_Center """
    def __init__(self, md):
        val = md.find(util.nspath_eval('dif:Data_Center_Name', namespaces))
        self.name = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Data_Center_URL', namespaces))
        self.url = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Data_Set_ID', namespaces))
        self.data_set_id = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Personnel', namespaces))
        self.personnel = util.testXMLValue(val)

class Distribution(object):
    """ Process Distribution """
    def __init__(self, md):
        val = md.find(util.nspath_eval('dif:Distribution_Media', namespaces))
        self.media = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Distribution_Size', namespaces))
        self.size = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Distribution_Format', namespaces))
        self.format = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Fees', namespaces))
        self.fees = util.testXMLValue(val)

class Multimedia_Sample(object):
    """ Process Multimedia_Sample """
    def __init__(self, md):
        val = md.find(util.nspath_eval('dif:File', namespaces))
        self.file = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:URL', namespaces))
        self.url = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Format', namespaces))
        self.format = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Caption', namespaces))
        self.caption = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Description', namespaces))
        self.description = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Visualization_URL', namespaces))
        self.vis_url = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Visualization_Type', namespaces))
        self.vis_type = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Visualization_Subtype', namespaces))
        self.vis_subtype = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Visualization_Duration', namespaces))
        self.vis_duration = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Visualization_File_Size', namespaces))
        self.file_size = util.testXMLValue(val)

class Related_URL(object):
    """ Process Related_URL """
    def __init__(self, md):
        self.content_type = []
        for el in md.findall(util.nspath_eval('dif:URL_Content_Type', namespaces)):
            self.content_type.append(URL_Content_Type(el))

        val = md.find(util.nspath_eval('dif:URL', namespaces))
        self.url = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:Description', namespaces))
        self.description = util.testXMLValue(val)

class URL_Content_Type(object):
    """ Process URL_Content_Type """
    def __init__(self, md):
        val = md.find(util.nspath_eval('dif:Type', namespaces))
        self.type = util.testXMLValue(val)

        val = md.find(util.nspath_eval('dif:SubType', namespaces))
        self.subtype = util.testXMLValue(val)











