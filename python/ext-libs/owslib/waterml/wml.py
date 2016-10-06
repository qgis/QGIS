from __future__ import (absolute_import, division, print_function)

from owslib.etree import etree
from owslib.util import nspath, testXMLValue, openURL
from owslib.util import xml_to_dict as _xml_to_dict
from datetime import datetime
from dateutil import parser

namespaces = {
    'wml1.1':'{http://www.cuahsi.org/waterML/1.1/}',
    'wml1.0':'{http://www.cuahsi.org/waterML/1.0/}',
    'xsi':'{http://www.w3.org/2001/XMLSchema-instance',
    'xsd':'{http://www.w3.org/2001/XMLSchema'
}

def ns(namespace):
    return namespaces.get(namespace)

class XMLParser(object):
    """
        Convienence class; provides some useful shortcut methods to make retrieving xml elements from etree
        a little easier.
    """
    def __init__(self,xml_root,namespace):
        try:
            self._root = etree.parse(xml_root)
        except:
            self._root = xml_root

        if not namespace in namespaces:
            raise ValueError('Unsupported namespace passed in to parser!')

        self._ns = namespace

    def _find(self,tofind):
        try:
            return self._root.find(namespaces.get(self._ns) + tofind)
        except:
            return None

    def _findall(self,tofind):
        try:
            return self._root.findall(namespaces.get(self._ns) + tofind)
        except:
            return None

class SitesResponse(XMLParser):
    """
        Parses the response from a 'GetSites' request

        Parameters
        ===========
        :xmlio - A file-like object that holds the xml response from the request.

        Return 
        =======
        An object constructed from a dictionary parse of the response. The object has get access and can iterate
        over the sites returned.
    """
    def __init__(self,xml,version='wml1.1'):
        super(SitesResponse,self).__init__(xml,version)
        self.parse_sites_response()

    def __iter__(self):
        for s in self.sites:
            yield s

    def __getitem__(self,key):
        if isinstance(key,int) and key < len(self.sites):
            return self.sites[key]

        if isinstance(key,str):
            site = [site for site in self.sites for code in site.site_info.site_codes if code == key]
            if len(site) > 0:
                return site[0]

        raise KeyError('Unknown key ' + str(key))

    def parse_sites_response(self,xml=None):
        """
        """
        if xml is not None:
            try:
                self._root = etree.parse(xml)
            except:
                self._root = xml

        # try:
        self.query_info = QueryInfo(self._find('queryInfo'), self._ns)
        self.sites = [Site(site, self._ns) for site in self._findall('site')]
        # except:
        #   raise ValueError('Cannot parse sitesResponse element correctly')

    """Accesability properties/methods"""
    @property
    def site_codes(self):
        return [site.site_info.site_codes for site in self.sites]

    @property
    def site_names(self):
        return [site.site_info.site_name for site in self.sites]

class QueryInfo(XMLParser):
    """
    """
    def __init__(self,xml_root,version='wml1.1'):
        super(QueryInfo, self).__init__(xml_root,version)
        self.parse_query_info()

    def parse_query_info(self, xml=None):
        if xml is not None:
            try:
                self._root = etree.parse(xml)
            except:
                self._root = xml

        # try:
            # create queryinfo object from dict
        xml_dict = _xml_to_dict(self._root)
        self.creation_time = parser.parse(xml_dict.get('creation_time')) if xml_dict.get('creation_time') is not None else None
        self.notes = [testXMLValue(note) for note in self._findall('note')]
        self.criteria = Criteria(self._find('criteria'), self._ns)
        # except:
        #   raise ValueError('Unable to parse queryInfo element correctly')

class Criteria(XMLParser):
    """
    """
    def __init__(self,xml_root,version='wml1.1'):
        super(Criteria, self).__init__(xml_root,version)
        self.parse_criteria()

    def parse_criteria(self, xml=None):
        if xml is not None:
            try:
                self._root = etree.parse(xml)
            except:
                self._root = xml

        # try:
        xml_dict = _xml_to_dict(self._root,depth=4)
        self.method_called = self._root.attrib.get('MethodCalled')
        self.location_param = xml_dict.get('location_param')
        self.variable_param = xml_dict.get('variable_param')
        try:
            self.begin_date_time = parser.parse(xml_dict['begin_date_time'])
        except:
            self.begin_date_time = None

        try:
            self.end_date_time = parser.parse(xml_dict['end_date_time'])
        except:
            self.end_date_time = None

        self.parameters = [(param.attrib.get('name'),param.attrib.get('value')) for param in self._findall('parameter')]
        # except:
        #   raise ValueError('Unable to parse xml for criteria element')

class Site(XMLParser):
    def __init__(self, xml, version='wml1.1'):
        super(Site,self).__init__(xml,version)
        self.parse_site()

    def __iter__(self):
        for c in self.series_catalogs:
            yield c

    def __getitem__(self,key):
        if isinstance(key,int) and key < len(self.series_catalogs):
            return self.series_catalogs[key]

        if isinstance(key,str):
            var = [series.variable for catalog in self.series_catalogs for series in catalog if series.code == key]
            if len(var) > 0:
                return var[0]

        raise KeyError('Unknown key ' + str(key))

    """Accessor propeties/methods"""
    @property
    def name(self):
        return self.site_info.site_name

    @property
    def codes(self):
        return self.site_info.site_codes

    @property
    def variable_names(self):
        return list(set([series.variable.variable_name for catalog in self.series_catalogs for series in catalog]))

    @property
    def variable_codes(self):
        return list(set([series.variable.variable_code for catalog in self.series_catalogs for series in catalog]))

    @property
    def geo_coords(self):
        return self.site_info.location.geo_coords

    @property
    def latitudes(self):
        return [g[1] for g in self.site_info.location.geo_coords]

    @property
    def longitudes(self):
        return [g[0] for g in self.site_info.location.geo_coords]

    def parse_site(self,xml=None):
        if xml is not None:
            try:
                self._root = etree.parse(xml)
            except:
                self._root = xml

        # try:
        self.site_info = SiteInfo(self._find('siteInfo'), self._ns)
        self.series_catalogs = [SeriesCatalog(elm, self._ns) for elm in self._findall('seriesCatalog')]
            # self.extension = Extension(self._find('extension'), self._ns)
        # except:
        #   raise ValueError('Unable to parse site element correctly')


class SiteInfo(XMLParser):
    def __init__(self,xml,version='wml1.1'):
        super(SiteInfo,self).__init__(xml,version)
        self.parse_siteinfo()

    def parse_siteinfo(self,xml=None):
        if xml is not None:
            try:
                self._root = etree.parse(xml)
            except:
                self._root = xml

        # try:
        xml_dict = _xml_to_dict(self._root)
        self.site_name = xml_dict.get('site_name')
        self.site_codes = [testXMLValue(code) for code in self._findall('siteCode')]
        self.elevation = xml_dict.get('elevation_m')
        self.vertical_datum = xml_dict.get('vertical_datum')
        self.site_types = [testXMLValue(typ) for typ in self._findall('siteType')]
        self.site_properties = dict([(prop.attrib.get('name'),testXMLValue(prop)) for prop in self._findall('siteProperty')])
        self.altname = xml_dict.get('altname')
        self.notes = [testXMLValue(note) for note in self._findall('note')]
        # sub-objects
        tzi = self._find('timeZoneInfo')
        if tzi is not None:
            self.time_zone_info = TimeZoneInfo(tzi, self._ns)

        self.location = Location(self._find('geoLocation'), self._ns)

        # except:
        #   raise ValueError('Unable to parse siteInfo element')

class Location(XMLParser):
    def __init__(self,xml,version='wml1.1'):
        super(Location,self).__init__(xml,version)
        self.parse_location()

    def parse_location(self,xml=None):
        if xml is not None:
            try:
                self._root = etree.parse(xml)
            except:
                self._root = xml

        # try:
        xml_dict = _xml_to_dict(self._root)
        geogs = self._findall('geogLocation')
        self.geo_coords = list()
        self.srs = list()
        for g in geogs:
            self.geo_coords.append((testXMLValue(g.find(ns(self._ns) + 'longitude')),testXMLValue(g.find(ns(self._ns) + 'latitude'))))
            self.srs.append(g.attrib.get('srs'))

        locsite = self._findall('localSiteXY')
        self.local_sites = list()
        self.notes = list()
        self.projections = list()
        for ls in locsite:
            z = testXMLValue(ls.find(ns(self._ns) + 'Z'))
            if z is not None:
                self.local_sites.append((testXMLValue(ls.find(ns(self._ns) + 'X')),testXMLValue(ls.find(ns(self._ns) + 'Y')),z))
            else:
                self.local_sites.append((testXMLValue(ls.find(ns(self._ns) + 'X')),testXMLValue(ls.find(ns(self._ns) + 'Y')),'0'))

            self.notes.append([testXMLValue(note) for note in ls.findall(ns(self._ns) + 'note')])
            self.projections.append(ls.attrib.get('projectionInformation'))

        # except:
        #   raise ValueError('Unable to parse geoLocation element')


class TimeZoneInfo(XMLParser):
    def __init__(self,xml,version='wml1.1'):
        super(TimeZoneInfo,self).__init__(xml,version)
        self.parse_timezoneinfo()

    def parse_timezoneinfo(self,xml=None):
        if xml is not None:
            try:
                self._root = etree.parse(xml)
            except:
                self._root = xml

        # try:
        xml_dict = _xml_to_dict(self._root)
        default = self._find('defaultTimeZone')
        if default is not None:
          self.zone_offset = default.attrib.get('zoneOffset')
          self.zone_abbreviation = default.attrib.get('zoneAbbreviation')

        daylight = self._find('daylightSavingsTimeZone')
        if daylight is not None:
          self.daylight_zone_offset = daylight.attrib.get('zoneOffset')
          self.daylight_zone_abbreviation = daylight.attrib.get('zoneAbbreviation')

        # except:
        #   raise ValueError('Unable to properly parset the timeZoneInfo element')
            

class SeriesCatalog(XMLParser):
    def __init__(self,xml,version='wml1.1'):
        super(SeriesCatalog,self).__init__(xml,version)
        self.parse_seriescatalog()

    def __iter__(self):
        for s in self.series:
            yield s

    def __getitem__(self,key):
        if isinstance(key,int) and key < len(self.series):
            return self.series[key]

        if isinstance(key,str):
            srs = [series for series in self.series if series.code == key]
            if len(srs) > 0:
                return srs[0]

        raise KeyError('Unknown key ' + str(key))

    def parse_seriescatalog(self,xml=None):
        if xml is not None:
            try:
                self._root = etree.parse(xml)
            except:
                self._root = xml

        # try:
        self.series = [Series(elm,self._ns) for elm in self._findall('series')]
        # except:
        #   raise ValueError('Unable to properly parse the seriesCatalog element')


class Series(XMLParser):
    def __init__(self,xml,version='wml1.1'):
        super(Series,self).__init__(xml,version)
        self.parse_series()

    """Accessor proeprties/methods"""
    @property
    def name(self):
        return self.variable.variable_name

    @property
    def code(self):
        return self.variable.variable_code

    def parse_series(self,xml=None):
        if xml is not None:
            try:
                self._root = etree.parse(xml)
            except:
                self._root = xml

        # try:
        xml_dict = _xml_to_dict(self._root,depth=3)
        self.value_count = xml_dict.get('value_count')
        self.value_type = xml_dict.get('value_type')
        self.general_category = xml_dict.get('general_category')
        self.sample_medium = xml_dict.get('sample_medium')
        self.data_type = xml_dict.get('data_type')
        # date-time
        self.begin_date_time = parser.parse(xml_dict.get('begin_date_time'))
        self.begin_date_time_utc = parser.parse(xml_dict.get('begin_date_time_utc')) if xml_dict.get('begin_date_time_utc') is not None else None
        self.end_date_time = parser.parse(xml_dict.get('end_date_time'))
        self.end_date_time_utc = parser.parse(xml_dict.get('end_date_time_utc')) if xml_dict.get('end_date_time_utc') is not None else None
        # method info
        self.method_description = xml_dict.get('method_description')
        self.method_code = xml_dict.get('method_code')
        self.method_link = xml_dict.get('method_link')
        method = self._find('method')
        if method is not None:
            self.method_id = method.attrib.get('methodID')
        else:
            self.method_id = None

        # source info
        self.organization = xml_dict.get('organization')
        self.source_description = xml_dict.get('source_description')
        self.citation = xml_dict.get('citation')
        source = self._find('source')
        if source is not None:
            self.source_id = source.attrib.get('sourceID')
        else:
            self.source_id = None

        # quality control info
        self.quality_control_level_code = xml_dict.get('quality_control_level_code')
        self.definition = xml_dict.get('definition')
        qa = self._find('qualityControlLevel')
        if qa is not None:
            self.quality_control_level_id = qa.attrib.get('qualityControlLevelID')
        else:
            self.quality_control_level_id = None

        # properties
        self.properties = dict([(prop.attrib.get('name'),testXMLValue(prop)) for prop in self._findall('seriesProperty')])
        # sub-objects
        self.variable = Variable(self._find('variable'),self._ns)
        # except:
        #   raise ValueError('Unable to correctly parse Series element')


class Variable(XMLParser):
    def __init__(self,xml,version='wml1.1'):
        super(Variable,self).__init__(xml,version)
        self.parse_variable()

    def parse_variable(self,xml=None):
        if xml is not None:
            try:
                self._root = etree.parse(xml)
            except:
                self._root = xml

        # try:
        xml_dict = _xml_to_dict(self._root)
        self.value_type = xml_dict.get('value_type')
        self.data_type = xml_dict.get('data_type')
        self.general_category = xml_dict.get('general_category')
        self.sample_medium = xml_dict.get('sample_medium')
        self.no_data_value = xml_dict.get('no_data_value')
        self.variable_name = xml_dict.get('variable_name')
        self.variable_code = xml_dict.get('variable_code')
        self.variable_description = xml_dict.get('variable_description')
        self.speciation = xml_dict.get('speciation')
        # notes and properties
        notes = [(note.attrib.get('title'),testXMLValue(note)) for note in self._findall('note')]
        none_notes = [note[1] for note in notes if note[0] is None]
        self.notes = dict([note for note in notes if note[0] is not None])
        if len(none_notes) > 0:
            self.notes['none'] = none_notes

        self.properties = dict([(prop.attrib.get('name'),testXMLValue(prop)) for prop in self._findall('variableProperty')])
        # related
        related = self._find('related')
        if related is not None:
            self.parent_codes = [dict([('network',code.attrib.get('network')),('vocabulary',code.attrib.get('vocabulary')),('default',code.attrib.get('default'))])
                             for code in related.findall(ns(self._ns) + 'parentCode')]
            self.related_codes = [dict([('network',d.get('network')),('vocabulary',d.get('vocabulary')),('default',d.get('default'))])
                             for code in related.findall(ns(self._ns) + 'relatedCode')]
        else:
            self.parent_codes = None
            self.related_codes = None

        # sub-objects
        if self._ns == 'wml1.0':
            unit = self._find('units')
            self.unit = Unit1_0(unit, self._ns) if unit is not None else None

            timesupport = self._find('timeSupport')
            self.time_support = TimeScale(timesupport, self._ns) if timesupport is not None else None
        else:
            unit = self._find('unit')
            self.unit = Unit(unit, self._ns) if unit is not None else None

            timescale = self._find('timeScale')
            self.time_scale = TimeScale(timescale, self._ns) if timescale is not None else None

        categories = self._find('categories')
        if categories is not None:
            self.categories = [Category(cat,self._ns) for cat in categories.findall(ns(self._ns) + 'category')]
        else:
            self.categories = None
        # except:
        #   raise ValueError('Unable to correctly parse variable element')


class TimeScale(XMLParser):
    def __init__(self,xml,version='wml1.1'):
        super(TimeScale,self).__init__(xml,version)
        self.parse_timescale()

    def parse_timescale(self):
        try:
            xml_dict = _xml_to_dict(self._root)
            self.time_spacing = xml_dict.get('time_spacing')
            self.time_support = xml_dict.get('time_support')
            self.time_interval = xml_dict.get('time_interval')
            unit = self._find('unit')
            self.unit = Unit(unit, self._ns) if unit is not None else None
        except:
            raise


class Unit(XMLParser):
    def __init__(self,xml,version='wml1.1'):
        super(Unit,self).__init__(xml,version)
        self.parse_unit()

    def parse_unit(self):
        try:
            xml_dict = _xml_to_dict(self._root)
            self.name = xml_dict.get('unit_name')
            self.unit_type = xml_dict.get('unit_type')
            self.description = xml_dict.get('unit_description')
            self.abbreviation = xml_dict.get('unit_abbreviation')
            self.code = xml_dict.get('unit_code')
            self.id = self._root.attrib.get('UnitID')
        except:
            raise


class Unit1_0(XMLParser):
    def __init__(self,xml,version='wml1.0'):
        super(Unit1_0,self).__init__(xml,version)
        self.parse_unit()

    def parse_unit(self):
        try:
            self.name = testXMLValue(self._root)
            self.code = self._root.attrib.get('unitsCode')
            self.abbreviation = self._root.attrib.get('unitsAbbreviation')
            self.type = self._root.attrib.get('unitsType')
            self.id = self._root.attrib.get('unitID')
        except:
            raise


class Category(XMLParser):
    def __init__(self,xml,version='wml1.1'):
        super(Category,self).__init__(xml,version)
        self.parse_category()

    def parse_category(self):
        try:
            xml_dict = _xml_to_dict(self._root)
            self.data_value = xml_dict.get('data_value')
            self.description = xml_dict.get('description')
            self.id = self._root.attrib.get('categoryID')
        except:
            raise


class TimeSeriesResponse(XMLParser):
    """
        Parses the response from a 'GetValues' request

        Parameters
        ===========
        :xmlio - A file-like object that holds the xml response from the request.

        Return 
        =======
        An object constructed from a dictionary parse of the response. The object has get access and can
        also iterate over each timeSeries element returned.
    """
    def __init__(self,xml,version='wml1.1'):
        super(TimeSeriesResponse,self).__init__(xml,version)
        self.parse_timeseriesresponse()

    """Accessor properties/methods"""
    @property
    def series_names(self):
        return [series.name for series in self.time_series]

    @property
    def variable_names(self):
        return list(set([series.variable.variable_name for series in self.time_series]))

    @property
    def variable_codes(self):
        return list(set([s.variable.variable_code for s in self.time_series]))

    def get_series_by_variable(self,var_name=None,var_code=None):
        if var_code is not None:
            return [s for s in self.time_series if s.variable.variable_code == var_code]

        elif var_name is not None:
            return [series for series in self.time_series if series.variable.variable_name == var_name]

        return None

    def parse_timeseriesresponse(self):
        try:
            qi = self._find('queryInfo')
            self.query_info = QueryInfo(qi,self._ns)
            self.time_series = [TimeSeries(series,self._ns) for series in self._findall('timeSeries')]
        except:
            raise


class TimeSeries(XMLParser):
    def __init__(self,xml,version='wml1.1'):
        super(TimeSeries,self).__init__(xml,version)
        self.parse_timeseries()

    def parse_timeseries(self):
        try:
            self.variable = Variable(self._find('variable'), self._ns)
            self.values = [Values(val,self._ns) for val in self._findall('values')]
            self.source_info = SiteInfo(self._find('sourceInfo'), self._ns)
            self.name = self._root.attrib.get('name')
        except:
            raise

class Values(XMLParser):
    def __init__(self,xml,version='wml1.1'):
        super(Values,self).__init__(xml,version)
        self.parse_values()

    def __iter__(self):
        for v in self.values:
            yield v

    """Accessor properties/methods"""
    def get_date_values(self,method_id=None,source_id=None,sample_id=None,quality_level=None,utc=False):
        varl = [v for v in self.values]
        if method_id is not None:
            varl = [v for v in varl if v.method_id == method_id]

        if source_id is not None:
            varl = [v for v in varl if v.source_id == source_id]

        if sample_id is not None:
            varl = [v for v in varl if v.sample_id == sample_id]

        if quality_level is not None:
            varl = [v for v in varl if v.quality_control_level == quality_level]

        if not utc:
            return [(v.date_time,v.value) for v in varl]
        else:
            return [(v.date_time_utc,v.value) for v in varl]

    def parse_values(self):
        xml_dict = _xml_to_dict(self._root)
        # method info
        self.methods = [Method(method,self._ns) for method in self._findall('method')]

        # source info
        self.sources = [Source(source,self._ns) for source in self._findall('source')]

        # quality control info
        self.qualit_control_levels = [QualityControlLevel(qal, self._ns) for qal in self._findall('qualityControlLevel')]

        # offset info
        self.offsets = [Offset(os,self._ns) for os in self._findall('offset')]

        # sample info
        self.samples = [Sample(sample,self._ns) for sample in self._findall('sample')]

        # censor codes
        self.censor_codes = [CensorCode(code, self._ns) for code in self._findall('censorCode')]

        # unit
        if self._ns == 'wml1.0':
            self.unit_abbreviation = self._root.attrib.get('unitsAbbreviation')
            self.unit_code = self._root.attrib.get('unitsCode')
            self.count = self._root.attrib.get('count')
        else:
            unit = self._find('unit')
            self.unit = Unit(unit, self._ns) if unit is not None else None

        # values
        self.values = [Value(val, self._ns) for val in self._findall('value')]


class Value(XMLParser):
    def __init__(self,xml,version='wml1.1'):
        super(Value,self).__init__(xml,version)
        self.parse_value()

    def parse_value(self):
        try:
            self.value = testXMLValue(self._root)
            d = self._root.attrib
            self.qualifiers = d.get('qualifiers')
            self.censor_code = d.get('censorCode')
            self.date_time = parser.parse(d.get('dateTime')) if d.get('dateTime') is not None else None
            self.time_offset = d.get('timeOffset')
            self.date_time_utc = parser.parse(d.get('dateTimeUTC')) if d.get('dateTimeUTC') is not None else None
            self.method_id = d.get('methodID')
            self.source_id = d.get('sourceID')
            self.accuracy_std_dev = d.get('accuracyStdDev')
            self.sample_id = d.get('sampleID')
            self.method_code = d.get('methodCode')
            self.source_code = d.get('sourceCode')
            self.lab_sample_code = d.get('lab_sample_code')
            self.offset_value = d.get('offsetValue')
            self.offset_type_id = d.get('offsetTypeID')
            self.offset_type_code = d.get('offsetTypeCode')
            self.coded_vocabulary = d.get('codedVocabulary')
            self.coded_vocabulary_term = d.get('codedVocabularyTerm')
            self.quality_control_level = d.get('qualityControlLevel')
            self.metadata_time = d.get('metadataTime')
            self.oid = d.get('oid')
        except:
            raise


class Sample(XMLParser):
    def __init__(self,xml,version='wml1.1'):
        super(Sample,self).__init__(xml,version)
        self.parse_sample()

    def parse_sample(self):
        try:
            xml_dict = _xml_to_dict(self._root)
            self.code = xml_dict.get('lab_sample_code')
            self.type = xml_dict.get('sample_type')
            lm = self._find('labMethod')
            self.method = LabMethod(lm, self._ns) if lm is not None else None
        except:
            raise


class LabMethod(XMLParser):
    def __init__(self,xml,version='wml1.1'):
        super(LabMethod,self).__init__(xml,version)
        self.parse_labmethod()

    def parse_labmethod(self):
        try:
            xml_dict = _xml_to_dict(self._root)
            self.code = xml_dict.get('lab_code')
            self.name = xml_dict.get('lab_name')
            self.organization = xml_dict.get('lab_organization')
            self.method_name = xml_dict.get('lab_method_name')
            self.method_description = xml_dict.get('lab_method_description')
            self.method_link = xml_dict.get('lab_method_link')
            # sub-objects
            source = self._find('labSourceDetails')
            self.source_details = Source(source,self._ns) if source is not None else None
        except:
            raise


class Source(XMLParser):
    def __init__(self,xml,version='wml1.1'):
        super(Source,self).__init__(xml,version)
        self.parse_source()

    def __str__(self):
        return str(self.__dict__)

    def get_contact(self,name):
        ci = [ci for ci in self.contact_info if ci.name == name]
        if len(ci) < 0:
            return ci[0]
        return None

    def parse_source(self):
        try:
            xml_dict = _xml_to_dict(self._root)
            self.code = xml_dict.get('source_code')
            self.organization = xml_dict.get('organization')
            self.description = xml_dict.get('source_description')
            self.links = [testXMLValue(link) for link in self._findall('sourceLink')]
            self.citation = xml_dict.get('citation')
            # metadata
            self.topic_category = xml_dict.get('topic_category')
            self.title = xml_dict.get('title')
            self.abstract = xml_dict.get('abstract')
            self.profile_version = xml_dict.get('profile_version')
            self.metadata_link = xml_dict.get('metadata_link')
            # contact info
            self.contact_info = [ContactInformation(ci,self._ns) for ci in self._findall('contactInformation')]
        except:
            raise


class ContactInformation(XMLParser):
    def __init__(self,xml,version='wml1.1'):
        super(ContactInformation,self).__init__(xml,version)
        self.parse_contactinformation()

    def parse_contactinformation(self):
        try:
            xml_dict = _xml_to_dict(self._root)
            self.name = xml_dict.get('contact_name')
            self.type = xml_dict.get('type_of_contact')
            self.email = [testXMLValue(email) for email in self._findall('email')]
            self.phone = [testXMLValue(phone) for phone in self._findall('phone')]
            self.address = [testXMLValue(address) for address in self._findall('address')]
        except:
            raise


class Offset(XMLParser):
    def __init__(self,xml,version='wml1.1'):
        super(Offset,self).__init__(xml,version)
        self.parse_offset()

    def parse_offset(self):
        try:
            xml_dict = _xml_to_dict(self._root)
            self.type_code = xml_dict.get('offset_type_code')
            self.value = xml_dict.get('offset_value')
            self.description = xml_dict.get('offset_description')
            self.is_vertical = xml_dict.get('offset_is_vertical')
            self.azimuth_degrees = xml_dict.get('offset_azimuth_degrees')
            unit = self._root.find('unit')
            if self._ns == 'wml1.0':
                self.unit = Unit1_0(unit, self._ns) if unit is not None else None
            else:
                self.unit = Unit(unit,self._ns) if unit is not None else None
        except:
            raise


class Method(XMLParser):
    def __init__(self,xml,version='wml1.1'):
        super(Method,self).__init__(xml,version)
        self.parse_method()

    def parse_method(self):
        try:
            xml_dict = _xml_to_dict(self._root)
            self.code = xml_dict.get('method_code')
            self.description = xml_dict.get('method_description')
            self.link = xml_dict.get('method_link')
            self.id = self._root.attrib.get('methodID')
        except:
            raise


class QualityControlLevel(XMLParser):
    def __init__(self,xml,version='wml1.1'):
        super(QualityControlLevel,self).__init__(xml,version)
        self.parse_qcl()

    def parse_qcl(self):
        try:
            xml_dict = _xml_to_dict(self._root)
            self.code = xml_dict.get('quality_control_level_code')
            self.definition = xml_dict.get('definition')
            self.explanation = xml_dict.get('explanation')
            self.id = self._root.attrib.get('qualityControlLevelID')
        except:
            raise


class CensorCode(XMLParser):
    def __init__(self,xml,version='wml1.1'):
        super(CensorCode,self).__init__(xml,version)
        self.parse_censorcode()

    def parse_censorcode(self):
        try:
            xml_dict = _xml_to_dict(self._root)
            self.code = xml_dict.get('censor_code')
            self.description = xml_dict.get('censor_code_description')
            self.id = self._root.attrib.get('censorCodeID')
        except:
            raise

class VariablesResponse(XMLParser):
    """
        Parses the response from a 'GetVariableInfo' request

        Parameters
        ===========
        :xmlio - A file-like object that holds the xml response from the request.

        Return
        =======
        An object constructed from a dictionary parse of the response. The object has get access to its variables and
        can also be used as an iterator.
    """
    def __init__(self,xml,version='wml1.1'):
        super(VariablesResponse,self).__init__(xml,version)
        self.parse_variablesresponse()

    def __iter__(self):
        for v in self.variables:
            yield v

    def __getitem__(self,key):
        if isinstance(key,int) and key < len(self.variables):
            return self.variables[key]

        if isinstance(key,str):
            v = [var for var in self.variables if var.variable_code == key]
            if len(v) > 0:
                return v[0]

            v = [var for var in self.variables if var.variable_name == key]
            if len(v) > 0:
                return v[0]

        raise KeyError('Unknown key ' + str(key))

    """Accessor properties/methods"""
    @property
    def variable_names(self):
        return list(set([var.variable_name for var in self.variables]))

    @property
    def variable_codes(self):
        return  [var.variable_code for var in self.variables]

    def parse_variablesresponse(self):
        try:
            qi = self._find('queryInfo')
            self.query_info = QueryInfo(qi, self._ns) if qi is not None else None
            varis = self._find('variables')
            self.variables = [Variable(var,self._ns) for var in varis.findall(ns(self._ns) + 'variable')]
        except:
            raise

