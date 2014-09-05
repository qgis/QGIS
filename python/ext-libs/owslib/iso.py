# -*- coding: ISO-8859-15 -*- 
# ============================================================================= 
# Copyright (c) 2009 Tom Kralidis
#
# Authors : Tom Kralidis <tomkralidis@gmail.com>
#           Angelos Tzotsos <tzotsos@gmail.com>
#
# Contact email: tomkralidis@gmail.com
# =============================================================================

""" ISO metadata parser """

from owslib.etree import etree
from owslib import util
from owslib.namespaces import Namespaces

# default variables
def get_namespaces():
    n = Namespaces()
    ns = n.get_namespaces(["gco","gmd","gml","gml32","gmx","gts","srv","xlink"])
    ns[None] = n.get_namespace("gmd")
    return ns
namespaces = get_namespaces()


class MD_Metadata(object):
    """ Process gmd:MD_Metadata """
    def __init__(self, md=None):

        if md is None:
            self.xml = None
            self.identifier = None
            self.parentidentifier = None
            self.language = None
            self.dataseturi = None
            self.languagecode = None
            self.datestamp = None
            self.charset = None
            self.hierarchy = None
            self.contact = []
            self.datetimestamp = None
            self.stdname = None
            self.stdver = None
            self.referencesystem = None
            self.identification = None
            self.serviceidentification = None
            self.identificationinfo = []
            self.distribution = None
            self.dataquality = None
        else:
            if hasattr(md, 'getroot'):  # standalone document
                self.xml = etree.tostring(md.getroot())
            else:  # part of a larger document
                self.xml = etree.tostring(md)

            val = md.find(util.nspath_eval('gmd:fileIdentifier/gco:CharacterString', namespaces))
            self.identifier = util.testXMLValue(val)

            val = md.find(util.nspath_eval('gmd:parentIdentifier/gco:CharacterString', namespaces))
            self.parentidentifier = util.testXMLValue(val)

            val = md.find(util.nspath_eval('gmd:language/gco:CharacterString', namespaces))
            self.language = util.testXMLValue(val)
            
            val = md.find(util.nspath_eval('gmd:dataSetURI/gco:CharacterString', namespaces))
            self.dataseturi = util.testXMLValue(val)

            val = md.find(util.nspath_eval('gmd:language/gmd:LanguageCode', namespaces))
            self.languagecode = util.testXMLValue(val)
            
            val = md.find(util.nspath_eval('gmd:dateStamp/gco:Date', namespaces))
            self.datestamp = util.testXMLValue(val)

            if not self.datestamp:
                val = md.find(util.nspath_eval('gmd:dateStamp/gco:DateTime', namespaces))
                self.datestamp = util.testXMLValue(val)

            self.charset = _testCodeListValue(md.find(util.nspath_eval('gmd:characterSet/gmd:MD_CharacterSetCode', namespaces)))
      
            self.hierarchy = _testCodeListValue(md.find(util.nspath_eval('gmd:hierarchyLevel/gmd:MD_ScopeCode', namespaces)))

            self.contact = []
            for i in md.findall(util.nspath_eval('gmd:contact/gmd:CI_ResponsibleParty', namespaces)):
                o = CI_ResponsibleParty(i)
                self.contact.append(o)
            
            val = md.find(util.nspath_eval('gmd:dateStamp/gco:DateTime', namespaces))
            self.datetimestamp = util.testXMLValue(val)
            
            val = md.find(util.nspath_eval('gmd:metadataStandardName/gco:CharacterString', namespaces))
            self.stdname = util.testXMLValue(val)

            val = md.find(util.nspath_eval('gmd:metadataStandardVersion/gco:CharacterString', namespaces))
            self.stdver = util.testXMLValue(val)

            val = md.find(util.nspath_eval('gmd:referenceSystemInfo/gmd:MD_ReferenceSystem', namespaces))
            if val is not None:
                self.referencesystem = MD_ReferenceSystem(val)
            else:
                self.referencesystem = None

            # TODO: merge .identificationinfo into .identification
            #warnings.warn(
            #    'the .identification and .serviceidentification properties will merge into '
            #    '.identification being a list of properties.  This is currently implemented '
            #    'in .identificationinfo.  '
            #    'Please see https://github.com/geopython/OWSLib/issues/38 for more information',
            #    FutureWarning)

            val = md.find(util.nspath_eval('gmd:identificationInfo/gmd:MD_DataIdentification', namespaces))
            val2 = md.find(util.nspath_eval('gmd:identificationInfo/srv:SV_ServiceIdentification', namespaces))

            if val is not None:
                self.identification = MD_DataIdentification(val, 'dataset')
                self.serviceidentification = None
            elif val2 is not None:
                self.identification = MD_DataIdentification(val2, 'service')
                self.serviceidentification = SV_ServiceIdentification(val2)
            else:
                self.identification = None
                self.serviceidentification = None

            self.identificationinfo = []
            for idinfo in md.findall(util.nspath_eval('gmd:identificationInfo', namespaces)):
                val = list(idinfo)[0]
                tagval = util.xmltag_split(val.tag)
                if tagval == 'MD_DataIdentification': 
                    self.identificationinfo.append(MD_DataIdentification(val, 'dataset'))
                elif tagval == 'MD_ServiceIdentification': 
                    self.identificationinfo.append(MD_DataIdentification(val, 'service'))
                elif tagval == 'SV_ServiceIdentification': 
                    self.identificationinfo.append(SV_ServiceIdentification(val))

            val = md.find(util.nspath_eval('gmd:distributionInfo/gmd:MD_Distribution', namespaces))

            if val is not None:
                self.distribution = MD_Distribution(val)
            else:
                self.distribution = None
            
            val = md.find(util.nspath_eval('gmd:dataQualityInfo/gmd:DQ_DataQuality', namespaces))
            if val is not None:
                self.dataquality = DQ_DataQuality(val)
            else:
                self.dataquality = None

class CI_Date(object):
    """ process CI_Date """
    def __init__(self, md=None):
        if md is None:
            self.date = None
            self.type = None
        else:
            val = md.find(util.nspath_eval('gmd:date/gco:Date', namespaces))
            if val is not None:
                self.date = util.testXMLValue(val)
            else:
                val = md.find(util.nspath_eval('gmd:date/gco:DateTime', namespaces))
                if val is not None:
                    self.date = util.testXMLValue(val)
                else:
                    self.date = None

            val = md.find(util.nspath_eval('gmd:dateType/gmd:CI_DateTypeCode', namespaces))
            self.type = _testCodeListValue(val)

class CI_ResponsibleParty(object):
    """ process CI_ResponsibleParty """
    def __init__(self, md=None):

        if md is None:
            self.name = None
            self.organization = None
            self.position = None
            self.phone = None
            self.fax = None
            self.address = None
            self.city = None
            self.region = None
            self.postcode = None
            self.country = None
            self.email = None
            self.onlineresource = None
            self.role = None
        else:
            val = md.find(util.nspath_eval('gmd:individualName/gco:CharacterString', namespaces))
            self.name = util.testXMLValue(val)

            val = md.find(util.nspath_eval('gmd:organisationName/gco:CharacterString', namespaces))
            self.organization = util.testXMLValue(val)

            val = md.find(util.nspath_eval('gmd:positionName/gco:CharacterString', namespaces))
            self.position = util.testXMLValue(val)

            val = md.find(util.nspath_eval('gmd:contactInfo/gmd:CI_Contact/gmd:phone/gmd:CI_Telephone/gmd:voice/gco:CharacterString', namespaces))

            self.phone = util.testXMLValue(val)

            val = md.find(util.nspath_eval('gmd:contactInfo/gmd:CI_Contact/gmd:phone/gmd:CI_Telephone/gmd:facsimile/gco:CharacterString', namespaces))
            self.fax = util.testXMLValue(val)

            val = md.find(util.nspath_eval('gmd:contactInfo/gmd:CI_Contact/gmd:address/gmd:CI_Address/gmd:deliveryPoint/gco:CharacterString', namespaces))
            self.address = util.testXMLValue(val)

            val = md.find(util.nspath_eval('gmd:contactInfo/gmd:CI_Contact/gmd:address/gmd:CI_Address/gmd:city/gco:CharacterString', namespaces))
            self.city = util.testXMLValue(val)

            val = md.find(util.nspath_eval('gmd:contactInfo/gmd:CI_Contact/gmd:address/gmd:CI_Address/gmd:administrativeArea/gco:CharacterString', namespaces))
            self.region = util.testXMLValue(val)

            val = md.find(util.nspath_eval('gmd:contactInfo/gmd:CI_Contact/gmd:address/gmd:CI_Address/gmd:postalCode/gco:CharacterString', namespaces))
            self.postcode = util.testXMLValue(val)

            val = md.find(util.nspath_eval('gmd:contactInfo/gmd:CI_Contact/gmd:address/gmd:CI_Address/gmd:country/gco:CharacterString', namespaces))
            self.country = util.testXMLValue(val)

            val = md.find(util.nspath_eval('gmd:contactInfo/gmd:CI_Contact/gmd:address/gmd:CI_Address/gmd:electronicMailAddress/gco:CharacterString', namespaces))
            self.email = util.testXMLValue(val)

            val = md.find(util.nspath_eval('gmd:contactInfo/gmd:CI_Contact/gmd:onlineResource/gmd:CI_OnlineResource', namespaces))
            if val is not None:
              self.onlineresource = CI_OnlineResource(val)
            else:
              self.onlineresource = None
          
            self.role = _testCodeListValue(md.find(util.nspath_eval('gmd:role/gmd:CI_RoleCode', namespaces)))

class MD_DataIdentification(object):
    """ process MD_DataIdentification """
    def __init__(self, md=None, identtype=None):
        if md is None:
            self.identtype = None
            self.title = None
            self.alternatetitle = None
            self.aggregationinfo = None
            self.uricode = []
            self.uricodespace = []
            self.date = []
            self.datetype = []
            self.uselimitation = []
            self.accessconstraints = []
            self.classification = []
            self.otherconstraints = []
            self.securityconstraints = []
            self.useconstraints = []
            self.denominators = []
            self.distance = []
            self.uom = []
            self.resourcelanguage = []
            self.creator = None
            self.publisher = None
            self.originator = None
            self.edition = None
            self.abstract = None
            self.purpose = None
            self.status = None
            self.contact = []
            self.keywords = []
            self.topiccategory = []
            self.supplementalinformation = None
            self.extent = None
            self.bbox = None
            self.temporalextent_start = None
            self.temporalextent_end = None
        else:
            self.identtype = identtype
            val = md.find(util.nspath_eval('gmd:citation/gmd:CI_Citation/gmd:title/gco:CharacterString', namespaces))
            self.title = util.testXMLValue(val)
            
            val = md.find(util.nspath_eval('gmd:citation/gmd:CI_Citation/gmd:alternateTitle/gco:CharacterString', namespaces))
            self.alternatetitle = util.testXMLValue(val)

            val = md.find(util.nspath_eval('gmd:aggregationInfo', namespaces))
            self.aggregationinfo = util.testXMLValue(val)

            self.uricode = []
            for i in md.findall(util.nspath_eval('gmd:citation/gmd:CI_Citation/gmd:identifier/gmd:RS_Identifier/gmd:code/gco:CharacterString', namespaces)):
                val = util.testXMLValue(i)
                if val is not None:
                    self.uricode.append(val)

            self.uricodespace = []
            for i in md.findall(util.nspath_eval('gmd:citation/gmd:CI_Citation/gmd:identifier/gmd:RS_Identifier/gmd:codeSpace/gco:CharacterString', namespaces)):
                val = util.testXMLValue(i)
                if val is not None:
                    self.uricodespace.append(val)

            self.date = []
            self.datetype = []
            
            for i in md.findall(util.nspath_eval('gmd:citation/gmd:CI_Citation/gmd:date/gmd:CI_Date', namespaces)):
                self.date.append(CI_Date(i))
            
            self.uselimitation = []
            for i in md.findall(util.nspath_eval('gmd:resourceConstraints/gmd:MD_Constraints/gmd:useLimitation/gco:CharacterString', namespaces)):
                val = util.testXMLValue(i)
                if val is not None:
                    self.uselimitation.append(val)
            
            self.accessconstraints = []
            for i in md.findall(util.nspath_eval('gmd:resourceConstraints/gmd:MD_LegalConstraints/gmd:accessConstraints/gmd:MD_RestrictionCode', namespaces)):
                val = _testCodeListValue(i)
                if val is not None:
                    self.accessconstraints.append(val)
            
            self.classification = []
            for i in md.findall(util.nspath_eval('gmd:resourceConstraints/gmd:MD_LegalConstraints/gmd:accessConstraints/gmd:MD_ClassificationCode', namespaces)):
                val = _testCodeListValue(i)
                if val is not None:
                    self.classification.append(val)
            
            self.otherconstraints = []
            for i in md.findall(util.nspath_eval('gmd:resourceConstraints/gmd:MD_LegalConstraints/gmd:otherConstraints/gco:CharacterString', namespaces)):
                val = util.testXMLValue(i)
                if val is not None:
                    self.otherconstraints.append(val)

            self.securityconstraints = []
            for i in md.findall(util.nspath_eval('gmd:resourceConstraints/gmd:MD_SecurityConstraints/gmd:useLimitation', namespaces)):
                val = util.testXMLValue(i)
                if val is not None:
                    self.securityconstraints.append(val)

            self.useconstraints = []
            for i in md.findall(util.nspath_eval('gmd:resourceConstraints/gmd:MD_LegalConstraints/gmd:useConstraints/gmd:MD_RestrictionCode', namespaces)):
                val = _testCodeListValue(i)
                if val is not None:
                    self.useconstraints.append(val)
            
            self.denominators = []
            for i in md.findall(util.nspath_eval('gmd:spatialResolution/gmd:MD_Resolution/gmd:equivalentScale/gmd:MD_RepresentativeFraction/gmd:denominator/gco:Integer', namespaces)):
                val = util.testXMLValue(i)
                if val is not None:
                    self.denominators.append(val)
            
            self.distance = []
            self.uom = []
            for i in md.findall(util.nspath_eval('gmd:spatialResolution/gmd:MD_Resolution/gmd:distance/gco:Distance', namespaces)):
                val = util.testXMLValue(i)
                if val is not None:
                    self.distance.append(val)
                self.uom.append(i.get("uom"))
            
            self.resourcelanguage = []
            for i in md.findall(util.nspath_eval('gmd:language/gmd:LanguageCode', namespaces)):
                val = _testCodeListValue(i)
                if val is not None:
                    self.resourcelanguage.append(val)

            val = md.find(util.nspath_eval('gmd:pointOfContact/gmd:CI_ResponsibleParty/gmd:organisationName', namespaces))
            if val is not None:
                val2 = val.find(util.nspath_eval('gmd:role/gmd:CI_RoleCode', namespaces)) 
                if val2 is not None:
                    clv = _testCodeListValue(val)
                    if clv == 'originator':
                        self.creator = util.testXMLValue(val)
                    elif clv == 'publisher':
                        self.publisher = util.testXMLValue(val)
                    elif clv == 'contributor':
                        self.originator = util.testXMLValue(val)

            val = md.find(util.nspath_eval('gmd:edition/gco:CharacterString', namespaces))
            self.edition = util.testXMLValue(val)

            val = md.find(util.nspath_eval('gmd:abstract/gco:CharacterString', namespaces))
            self.abstract = util.testXMLValue(val)

            val = md.find(util.nspath_eval('gmd:purpose/gco:CharacterString', namespaces))
            self.purpose = util.testXMLValue(val)

            self.status = _testCodeListValue(md.find(util.nspath_eval('gmd:status/gmd:MD_ProgressCode', namespaces)))

            self.contact = []
            for i in md.findall(util.nspath_eval('gmd:pointOfContact/gmd:CI_ResponsibleParty', namespaces)):
                o = CI_ResponsibleParty(i)
                self.contact.append(o)
            
            self.keywords = []

            for i in md.findall(util.nspath_eval('gmd:descriptiveKeywords', namespaces)):
                mdkw = {}
                mdkw['type'] = _testCodeListValue(i.find(util.nspath_eval('gmd:MD_Keywords/gmd:type/gmd:MD_KeywordTypeCode', namespaces)))

                mdkw['thesaurus'] = {}

                val = i.find(util.nspath_eval('gmd:MD_Keywords/gmd:thesaurusName/gmd:CI_Citation/gmd:title/gco:CharacterString', namespaces))
                mdkw['thesaurus']['title'] = util.testXMLValue(val)

                val = i.find(util.nspath_eval('gmd:MD_Keywords/gmd:thesaurusName/gmd:CI_Citation/gmd:date/gmd:CI_Date/gmd:date/gco:Date', namespaces))
                mdkw['thesaurus']['date'] = util.testXMLValue(val)

                val = i.find(util.nspath_eval('gmd:MD_Keywords/gmd:thesaurusName/gmd:CI_Citation/gmd:date/gmd:CI_Date/gmd:dateType/gmd:CI_DateTypeCode', namespaces))
                mdkw['thesaurus']['datetype'] = util.testXMLValue(val)

                mdkw['keywords'] = []

                for k in i.findall(util.nspath_eval('gmd:MD_Keywords/gmd:keyword', namespaces)):
                    val = k.find(util.nspath_eval('gco:CharacterString', namespaces))
                    if val is not None:
                        val2 = util.testXMLValue(val) 
                        if val2 is not None:
                            mdkw['keywords'].append(val2)

                self.keywords.append(mdkw)

            self.topiccategory = []
            for i in md.findall(util.nspath_eval('gmd:topicCategory/gmd:MD_TopicCategoryCode', namespaces)):
                val = util.testXMLValue(i)
                if val is not None:
                    self.topiccategory.append(val)
            
            val = md.find(util.nspath_eval('gmd:supplementalInformation/gco:CharacterString', namespaces))
            self.supplementalinformation = util.testXMLValue(val)
            
            # There may be multiple geographicElement, create an extent
            # from the one containing either an EX_GeographicBoundingBox or EX_BoundingPolygon.
            # The schema also specifies an EX_GeographicDescription. This is not implemented yet.
            val = None
            val2 = None
            val3 = None
            extents = md.findall(util.nspath_eval('gmd:extent', namespaces))
            extents.extend(md.findall(util.nspath_eval('srv:extent', namespaces)))
            for extent in extents:
                if val is None:
                    for e in extent.findall(util.nspath_eval('gmd:EX_Extent/gmd:geographicElement', namespaces)):
                        if e.find(util.nspath_eval('gmd:EX_GeographicBoundingBox', namespaces)) is not None or e.find(util.nspath_eval('gmd:EX_BoundingPolygon', namespaces)) is not None:
                            val = e
                            break
                    self.extent = EX_Extent(val)
                    self.bbox = self.extent.boundingBox  # for backwards compatibility

                if val2 is None:
                    val2 = extent.find(util.nspath_eval('gmd:EX_Extent/gmd:temporalElement/gmd:EX_TemporalExtent/gmd:extent/gml:TimePeriod/gml:beginPosition', namespaces))
                    if val2 is None:
                        val2 = extent.find(util.nspath_eval('gmd:EX_Extent/gmd:temporalElement/gmd:EX_TemporalExtent/gmd:extent/gml32:TimePeriod/gml32:beginPosition', namespaces))
                    self.temporalextent_start = util.testXMLValue(val2)

                if val3 is None:
                    val3 = extent.find(util.nspath_eval('gmd:EX_Extent/gmd:temporalElement/gmd:EX_TemporalExtent/gmd:extent/gml:TimePeriod/gml:endPosition', namespaces))
                    if val3 is None:
                        val3 = extent.find(util.nspath_eval('gmd:EX_Extent/gmd:temporalElement/gmd:EX_TemporalExtent/gmd:extent/gml32:TimePeriod/gml32:endPosition', namespaces))
                    self.temporalextent_end = util.testXMLValue(val3)

class MD_Distributor(object):        
    """ process MD_Distributor """
    def __init__(self, md=None):
        if md is None:
            self.contact = None
            self.online = []
        else:
            self.contact = None
            val = md.find(util.nspath_eval('gmd:MD_Distributor/gmd:distributorContact/gmd:CI_ResponsibleParty', namespaces))
            if val is not None:
                self.contact = CI_ResponsibleParty(val)

            self.online = []

            for ol in md.findall(util.nspath_eval('gmd:MD_Distributor/gmd:distributorTransferOptions/gmd:MD_DigitalTransferOptions/gmd:onLine/gmd:CI_OnlineResource', namespaces)):
                self.online.append(CI_OnlineResource(ol))

class MD_Distribution(object):
    """ process MD_Distribution """
    def __init__(self, md=None):
        if md is None:
            self.format = None
            self.version = None
            self.distributor = []
            self.online = []
            pass
        else:
            val = md.find(util.nspath_eval('gmd:distributionFormat/gmd:MD_Format/gmd:name/gco:CharacterString', namespaces))
            self.format = util.testXMLValue(val)

            val = md.find(util.nspath_eval('gmd:distributionFormat/gmd:MD_Format/gmd:version/gco:CharacterString', namespaces))
            self.version = util.testXMLValue(val)

            self.distributor = []
            for dist in md.findall(util.nspath_eval('gmd:distributor', namespaces)):
                self.distributor.append(MD_Distributor(dist))

            self.online = []

            for ol in md.findall(util.nspath_eval('gmd:transferOptions/gmd:MD_DigitalTransferOptions/gmd:onLine/gmd:CI_OnlineResource', namespaces)):
                self.online.append(CI_OnlineResource(ol))

        
class DQ_DataQuality(object):
    ''' process DQ_DataQuality'''
    def __init__(self, md=None):
        if md is None:
            self.conformancetitle = []
            self.conformancedate = []
            self.conformancedatetype = []
            self.conformancedegree = []
            self.lineage = None
            self.specificationtitle = None
            self.specificationdate = []
        else:
            self.conformancetitle = []
            for i in md.findall(util.nspath_eval('gmd:report/gmd:DQ_DomainConsistency/gmd:result/gmd:DQ_ConformanceResult/gmd:specification/gmd:CI_Citation/gmd:title/gco:CharacterString', namespaces)):
                val = util.testXMLValue(i)
                if val is not None:
                    self.conformancetitle.append(val)
            
            self.conformancedate = []
            for i in md.findall(util.nspath_eval('gmd:report/gmd:DQ_DomainConsistency/gmd:result/gmd:DQ_ConformanceResult/gmd:specification/gmd:CI_Citation/gmd:date/gmd:CI_Date/gmd:date/gco:Date', namespaces)):
                val = util.testXMLValue(i)
                if val is not None:
                    self.conformancedate.append(val)
            
            self.conformancedatetype = []
            for i in md.findall(util.nspath_eval('gmd:report/gmd:DQ_DomainConsistency/gmd:result/gmd:DQ_ConformanceResult/gmd:specification/gmd:CI_Citation/gmd:date/gmd:CI_Date/gmd:dateType/gmd:CI_DateTypeCode', namespaces)):
                val = _testCodeListValue(i)
                if val is not None:
                    self.conformancedatetype.append(val)
            
            self.conformancedegree = []
            for i in md.findall(util.nspath_eval('gmd:report/gmd:DQ_DomainConsistency/gmd:result/gmd:DQ_ConformanceResult/gmd:pass/gco:Boolean', namespaces)):
                val = util.testXMLValue(i)
                if val is not None:
                    self.conformancedegree.append(val)
            
            val = md.find(util.nspath_eval('gmd:lineage/gmd:LI_Lineage/gmd:statement/gco:CharacterString', namespaces))
            self.lineage = util.testXMLValue(val)

            val = md.find(util.nspath_eval('gmd:report/gmd:DQ_DomainConsistency/gmd:result/gmd:DQ_ConformanceResult/gmd:specification/gmd:CI_Citation/gmd:title/gco:CharacterString', namespaces))
            self.specificationtitle = util.testXMLValue(val)

            self.specificationdate = []
            for i in md.findall(util.nspath_eval('gmd:report/gmd:DQ_DomainConsistency/gmd:result/gmd:DQ_ConformanceResult/gmd:specification/gmd:CI_Citation/gmd:date/gmd:CI_Date', namespaces)):
                val = util.testXMLValue(i)
                if val is not None:
                    self.specificationdate.append(val)

class SV_ServiceIdentification(object):
    """ process SV_ServiceIdentification """
    def __init__(self, md=None):
        if md is None:
            self.identtype = 'service'
            self.type = None
            self.version = None
            self.fees = None
            self.bbox = None
            self.couplingtype
            self.operations = []
            self.operateson = []
        else:
            self.identtype = 'service'
            val = md.find(util.nspath_eval('srv:serviceType/gco:LocalName', namespaces))
            self.type = util.testXMLValue(val)
          
            val = md.find(util.nspath_eval('srv:serviceTypeVersion/gco:CharacterString', namespaces))
            self.version = util.testXMLValue(val)

            val = md.find(util.nspath_eval('srv:accessProperties/gmd:MD_StandardOrderProcess/gmd:fees/gco:CharacterString', namespaces))
            self.fees = util.testXMLValue(val)

            val = md.find(util.nspath_eval('srv:extent/gmd:EX_Extent', namespaces))

            if val is not None:
                self.bbox = EX_Extent(val)
            else:
                self.bbox = None

            self.couplingtype = _testCodeListValue(md.find(util.nspath_eval('gmd:couplingType/gmd:SV_CouplingType', namespaces)))

            self.operations = []

            for i in md.findall(util.nspath_eval('srv:containsOperations', namespaces)):
                tmp = {}
                val = i.find(util.nspath_eval('srv:SV_OperationMetadata/srv:operationName/gco:CharacterString', namespaces))
                tmp['name'] = util.testXMLValue(val)
                tmp['dcplist'] = []
                for d in i.findall(util.nspath_eval('srv:SV_OperationMetadata/srv:DCP', namespaces)):
                    tmp2 = _testCodeListValue(d.find(util.nspath_eval('srv:DCPList', namespaces)))
                    tmp['dcplist'].append(tmp2)
             
                tmp['connectpoint'] = []
     
                for d in i.findall(util.nspath_eval('srv:SV_OperationMetadata/srv:connectPoint', namespaces)):
                    tmp3 = d.find(util.nspath_eval('gmd:CI_OnlineResource', namespaces))
                    tmp['connectpoint'].append(CI_OnlineResource(tmp3))
                self.operations.append(tmp)

            self.operateson = []
             
            for i in md.findall(util.nspath_eval('srv:operatesOn', namespaces)):
                tmp = {}
                tmp['uuidref'] = i.attrib.get('uuidref')
                tmp['href'] = i.attrib.get(util.nspath_eval('xlink:href', namespaces))
                tmp['title'] = i.attrib.get(util.nspath_eval('xlink:title', namespaces))
                self.operateson.append(tmp)

class CI_OnlineResource(object):
    """ process CI_OnlineResource """
    def __init__(self,md=None):
        if md is None:
            self.url = None
            self.protocol = None
            self.name = None
            self.description = None
            self.function = None
        else:
            val = md.find(util.nspath_eval('gmd:linkage/gmd:URL', namespaces))
            self.url = util.testXMLValue(val)

            val = md.find(util.nspath_eval('gmd:protocol/gco:CharacterString', namespaces))
            self.protocol = util.testXMLValue(val)

            val = md.find(util.nspath_eval('gmd:name/gco:CharacterString', namespaces))
            self.name = util.testXMLValue(val)

            val = md.find(util.nspath_eval('gmd:description/gco:CharacterString', namespaces))
            self.description = util.testXMLValue(val)

            self.function = _testCodeListValue(md.find(util.nspath_eval('gmd:function/gmd:CI_OnLineFunctionCode', namespaces)))


class EX_GeographicBoundingBox(object):
    def __init__(self, md=None):
        if md is None:
            self.minx = None
            self.maxx = None
            self.miny = None
            self.maxy = None
        else:
            val = md.find(util.nspath_eval('gmd:westBoundLongitude/gco:Decimal', namespaces))
            self.minx = util.testXMLValue(val)
            val = md.find(util.nspath_eval('gmd:eastBoundLongitude/gco:Decimal', namespaces))
            self.maxx = util.testXMLValue(val)
            val = md.find(util.nspath_eval('gmd:southBoundLatitude/gco:Decimal', namespaces))
            self.miny = util.testXMLValue(val)
            val = md.find(util.nspath_eval('gmd:northBoundLatitude/gco:Decimal', namespaces))
            self.maxy = util.testXMLValue(val)
    
class EX_Polygon(object):
    def __init__(self, md=None):
        if md is None:
            self.exterior_ring = None
            self.interior_rings = []
        else:
            linear_ring = md.find(util.nspath_eval('gml32:Polygon/gml32:exterior/gml32:LinearRing', namespaces))
            if linear_ring is not None:
                self.exterior_ring = self._coordinates_for_ring(linear_ring)
                        
            interior_ring_elements = md.findall(util.nspath_eval('gml32:Polygon/gml32:interior', namespaces))
            self.interior_rings = []
            for iring_element in interior_ring_elements:
                linear_ring = iring_element.find(util.nspath_eval('gml32:LinearRing', namespaces))
                self.interior_rings.append(self._coordinates_for_ring(linear_ring))
            
    def _coordinates_for_ring(self, linear_ring):
        coordinates = []
        positions = linear_ring.findall(util.nspath_eval('gml32:pos', namespaces))
        for pos in positions:
            tokens = pos.text.split()
            coords = tuple([float(t) for t in tokens])
            coordinates.append(coords)
        return coordinates
        
class EX_GeographicBoundingPolygon(object):
    def __init__(self, md=None):
        if md is None:
            self.is_extent
            self.polygons = []
        else:
            val = md.find(util.nspath_eval('gmd:extentTypeCode', namespaces))
            self.is_extent = util.testXMLValue(val)
            
            md_polygons = md.findall(util.nspath_eval('gmd:polygon', namespaces))
            
            self.polygons = []
            for val in md_polygons:
                self.polygons.append(EX_Polygon(val))
                
class EX_Extent(object):
    """ process EX_Extent """
    def __init__(self, md=None):
        if md is None:
            self.boundingBox = None
            self.boundingPolygon = None
            self.description_code = None
        else:
            self.boundingBox = None
            self.boundingPolygon = None

            if md is not None:
                bboxElement = md.find(util.nspath_eval('gmd:EX_GeographicBoundingBox', namespaces))
                if bboxElement is not None:
                    self.boundingBox = EX_GeographicBoundingBox(bboxElement)
            
                polygonElement = md.find(util.nspath_eval('gmd:EX_BoundingPolygon', namespaces))
                if polygonElement is not None:
                    self.boundingPolygon = EX_GeographicBoundingPolygon(polygonElement)
     
                val = md.find(util.nspath_eval('gmd:EX_GeographicDescription/gmd:geographicIdentifier/gmd:MD_Identifier/gmd:code/gco:CharacterString', namespaces))
                self.description_code = util.testXMLValue(val)

class MD_ReferenceSystem(object):
    """ process MD_ReferenceSystem """
    def __init__(self, md):
        if md is None:
            pass
        else:
            val = md.find(util.nspath_eval('gmd:referenceSystemIdentifier/gmd:RS_Identifier/gmd:code/gco:CharacterString', namespaces))
            self.code = util.testXMLValue(val)

def _testCodeListValue(elpath):
    """ get gco:CodeListValue_Type attribute, else get text content """
    if elpath is not None:  # try to get @codeListValue
        val = util.testXMLValue(elpath.attrib.get('codeListValue'), True)
        if val is not None:
            return val
        else:  # see if there is element text
            return util.testXMLValue(elpath)
    else:
        return None

class CodelistCatalogue(object):
    """ process CT_CodelistCatalogue """
    def __init__(self, ct):
        val = ct.find(util.nspath_eval('gmx:name/gco:CharacterString', namespaces))
        self.name = util.testXMLValue(val)
        val = ct.find(util.nspath_eval('gmx:scope/gco:CharacterString', namespaces))
        self.scope = util.testXMLValue(val)
        val = ct.find(util.nspath_eval('gmx:fieldOfApplication/gco:CharacterString', namespaces))
        self.fieldapp = util.testXMLValue(val)
        val = ct.find(util.nspath_eval('gmx:versionNumber/gco:CharacterString', namespaces))
        self.version = util.testXMLValue(val)
        val = ct.find(util.nspath_eval('gmx:versionDate/gco:Date', namespaces))
        self.date = util.testXMLValue(val)

        self.dictionaries = {}

        for i in ct.findall(util.nspath_eval('gmx:codelistItem/gmx:CodeListDictionary', namespaces)):
            id = i.attrib.get(util.nspath_eval('gml32:id', namespaces))
            self.dictionaries[id] = {}
            val = i.find(util.nspath_eval('gml32:description', namespaces))
            self.dictionaries[id]['description'] = util.testXMLValue(val)
            val = i.find(util.nspath_eval('gml32:identifier', namespaces))
            self.dictionaries[id]['identifier'] = util.testXMLValue(val)
            self.dictionaries[id]['entries'] = {}

            for j in i.findall(util.nspath_eval('gmx:codeEntry', namespaces)):
                id2 = j.find(util.nspath_eval('gmx:CodeDefinition', namespaces)).attrib.get(util.nspath_eval('gml32:id', namespaces))
                self.dictionaries[id]['entries'][id2] = {}
                val = j.find(util.nspath_eval('gmx:CodeDefinition/gml32:description', namespaces))
                self.dictionaries[id]['entries'][id2]['description'] = util.testXMLValue(val)

                val = j.find(util.nspath_eval('gmx:CodeDefinition/gml32:identifier', namespaces))
                self.dictionaries[id]['entries'][id2]['identifier'] = util.testXMLValue(val)

                val = j.find(util.nspath_eval('gmx:CodeDefinition', namespaces)).attrib.get('codeSpace')
                self.dictionaries[id]['entries'][id2]['codespace'] = util.testXMLValue(val, True)

    def getcodelistdictionaries(self):
        return self.dictionaries.keys()

    def getcodedefinitionidentifiers(self, cdl):
        if self.dictionaries.has_key(cdl):
            ids = []
            for i in self.dictionaries[cdl]['entries']:
                ids.append(self.dictionaries[cdl]['entries'][i]['identifier'])
            return ids
        else:
            return None

