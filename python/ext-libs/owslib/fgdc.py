# -*- coding: ISO-8859-15 -*-
# =============================================================================
# Copyright (c) 2010 Tom Kralidis
#
# Authors : Tom Kralidis <tomkralidis@gmail.com>
#
# Contact email: tomkralidis@gmail.com
# =============================================================================

""" FGDC metadata parser """

from __future__ import (absolute_import, division, print_function)

from owslib.etree import etree
from owslib import util

class Metadata(object):
    """ Process metadata """
    def __init__(self, md):
        if hasattr(md, 'getroot'):  # standalone document
            self.xml = etree.tostring(md.getroot())
        else:  # part of a larger document
            self.xml = etree.tostring(md)

        self.idinfo = Idinfo(md)
        self.eainfo = Eainfo(md)
        self.distinfo = Distinfo(md)
        self.metainfo = Metainfo(md)

        if self.idinfo.datasetid:
            self.identifier = self.idinfo.datasetid

class Idinfo(object):
    """ Process idinfo """
    def __init__(self, md):
        val = md.find('idinfo/datasetid')
        self.datasetid = util.testXMLValue(val)

        val = md.find('idinfo/citation')
        self.citation = Citation(val)

        val = md.find('idinfo/descript')
        if val is not None:
            self.descript = Descript(val)

        val = md.find('idinfo/timeperd')
        self.timeperd = Timeperd(val)

        val = md.find('idinfo/status')
        if val is not None:
            self.status = Status(val)

        val = md.find('idinfo/spdom')
        if val is not None:
            self.spdom = Spdom(val)

        val = md.find('idinfo/keywords')
        if val is not None:
            self.keywords = Keywords(val)

        val = md.find('idinfo/accconst')
        self.accconst = util.testXMLValue(val)

        val = md.find('idinfo/useconst')
        self.useconst = util.testXMLValue(val)

        val = md.find('idinfo/ptcontac')
        if val is not None:
            self.ptcontac = Ptcontac(val)

        val = md.find('idinfo/datacred')
        self.datacred = util.testXMLValue(val)

        val = md.find('idinfo/crossref')
        self.crossref = Citation(val)

class Citation(object):
    """ Process citation """
    def __init__(self, md):
        if md is not None:
            self.citeinfo = {}
    
            val = md.find('citeinfo/origin')
            self.citeinfo['origin'] = util.testXMLValue(val)
    
            val = md.find('citeinfo/pubdate')
            self.citeinfo['pubdate'] = util.testXMLValue(val)
    
            val = md.find('citeinfo/title')
            self.citeinfo['title'] = util.testXMLValue(val)
    
            val = md.find('citeinfo/geoform')
            self.citeinfo['geoform'] = util.testXMLValue(val)
    
            val = md.find('citeinfo/pubinfo/pubplace')
            self.citeinfo['pubplace'] = util.testXMLValue(val)
    
            val = md.find('citeinfo/pubinfo/publish')
            self.citeinfo['publish'] = util.testXMLValue(val)

            self.citeinfo['onlink'] = []
            for link in md.findall('citeinfo/onlink'):
                self.citeinfo['onlink'].append(util.testXMLValue(link))

class Descript(object):
    """ Process descript """
    def __init__(self, md):
        val = md.find('abstract')
        self.abstract = util.testXMLValue(val)
        
        val = md.find('purpose')
        self.purpose = util.testXMLValue(val)

        val = md.find('supplinf')
        self.supplinf = util.testXMLValue(val)

class Timeperd(object):
    """ Process timeperd """
    def __init__(self, md):
        if md is not None:
            val = md.find('current')
            self.current = util.testXMLValue(val)

            val = md.find('timeinfo')
            if val is not None:
                self.timeinfo = Timeinfo(val)

class Timeinfo(object):
    """ Process timeinfo """
    def __init__(self, md):
        val = md.find('sngdate')
        if val is not None:
            self.sngdate = Sngdate(val)

        val = md.find('rngdates')
        if val is not None:
            self.rngdates = Rngdates(val)

class Sngdate(object):
    """ Process sngdate """
    def __init__(self, md):
        val = md.find('caldate')
        self.caldate = util.testXMLValue(val)
        val = md.find('time')
        self.time = util.testXMLValue(val)

class Rngdates(object):
    """ Process rngdates """
    def __init__(self, md):
        val = md.find('begdate')
        self.begdate = util.testXMLValue(val)
        val = md.find('begtime')
        self.begtime = util.testXMLValue(val)
        val = md.find('enddate')
        self.enddate = util.testXMLValue(val)
        val = md.find('endtime')
        self.endtime = util.testXMLValue(val)

class Status(object):
    """ Process status """
    def __init__(self, md):
        val = md.find('progress')
        self.progress = util.testXMLValue(val)

        val = md.find('update')
        self.update = util.testXMLValue(val)

class Spdom(object):
    """ Process spdom """
    def __init__(self, md):
        val = md.find('bounding/westbc')
        self.westbc = util.testXMLValue(val)

        val = md.find('bounding/eastbc')
        self.eastbc = util.testXMLValue(val)
       
        val = md.find('bounding/northbc')
        self.northbc = util.testXMLValue(val)

        val = md.find('bounding/southbc')
        self.southbc = util.testXMLValue(val)

        if (self.southbc is not None and self.northbc is not None and
        self.eastbc is not None and self.westbc is not None):
            self.bbox = Bbox(self)

class Bbox(object):
    """ Generate bbox for spdom (convenience function) """
    def __init__(self, spdom):
        self.minx = spdom.westbc
        self.miny = spdom.southbc
        self.maxx = spdom.eastbc
        self.maxy = spdom.northbc

class Keywords(object):
    """ Process keywords """
    def __init__(self, md):
        self.theme = []
        self.place = []
        self.temporal = []

        for i in md.findall('theme'):
            theme = {}
            val = i.find('themekt')
            theme['themekt'] = util.testXMLValue(val)
            theme['themekey'] = []
            for j in i.findall('themekey'):
                themekey = util.testXMLValue(j)
                if themekey is not None:
                    theme['themekey'].append(themekey)
            self.theme.append(theme)

        for i in md.findall('place'):
            theme = {}
            place = {}
            val = i.find('placekt')
            theme['placekt'] = util.testXMLValue(val)
            theme['placekey'] = []
            for j in i.findall('placekey'):
                theme['placekey'].append(util.testXMLValue(j))
            self.place.append(place)

        for i in md.findall('temporal'):
            theme = {}
            temporal = {}
            val = i.find('tempkt')
            theme['tempkt'] = util.testXMLValue(val)
            theme['tempkey'] = []
            for j in i.findall('tempkey'):
                theme['tempkey'].append(util.testXMLValue(j))
            self.temporal.append(temporal)

class Ptcontac(object):
    """ Process ptcontac """
    def __init__(self, md):
        val = md.find('cntinfo/cntorgp/cntorg')
        self.cntorg = util.testXMLValue(val)    

        val = md.find('cntinfo/cntorgp/cntper')
        self.cntper = util.testXMLValue(val)    

        val = md.find('cntinfo/cntpos')
        self.cntpos = util.testXMLValue(val)    

        val = md.find('cntinfo/cntaddr/addrtype')
        self.addrtype = util.testXMLValue(val)

        val = md.find('cntinfo/cntaddr/address')
        self.address = util.testXMLValue(val)

        val = md.find('cntinfo/cntaddr/city')
        self.city = util.testXMLValue(val)

        val = md.find('cntinfo/cntaddr/state')
        self.state = util.testXMLValue(val)

        val = md.find('cntinfo/cntaddr/postal')
        self.postal = util.testXMLValue(val)

        val = md.find('cntinfo/cntaddr/country')
        self.country = util.testXMLValue(val)

        val = md.find('cntinfo/cntvoice')
        self.voice = util.testXMLValue(val)

        val = md.find('cntinfo/cntemail')
        self.email = util.testXMLValue(val)

class Eainfo(object):
    """ Process eainfo """
    def __init__(self, md):
        val = md.find('eainfo/detailed/enttyp/enttypl')
        self.enttypl = util.testXMLValue(val)

        val = md.find('eainfo/detailed/enttyp/enttypd')
        self.enttypd = util.testXMLValue(val)

        val = md.find('eainfo/detailed/enttyp/enttypds')
        self.enttypds = util.testXMLValue(val)

        self.attr = []
        for i in md.findall('eainfo/detailed/attr'):
            attr = {}
            val = i.find('attrlabl')
            attr['attrlabl'] = util.testXMLValue(val)

            val = i.find('attrdef')
            attr['attrdef'] = util.testXMLValue(val)

            val = i.find('attrdefs')
            attr['attrdefs'] = util.testXMLValue(val)

            val = i.find('attrdomv/udom')
            attr['udom'] = util.testXMLValue(val)

            self.attr.append(attr)

class Distinfo(object):
    """ Process distinfo """
    def __init__(self, md):
        val = md.find('distinfo')
        if val is not None:
            val2 = val.find('stdorder')
            if val2 is not None:
                self.stdorder = {'digform': []}
                for link in val2.findall('digform'):
                    digform = {}
                    digform['name'] = util.testXMLValue(link.find('digtinfo/formname'))
                    digform['url'] = util.testXMLValue(link.find('digtopt/onlinopt/computer/networka/networkr/'))
                    self.stdorder['digform'].append(digform)

class Metainfo(object):
    """ Process metainfo """
    def __init__(self, md):
        val = md.find('metainfo/metd')
        self.metd = util.testXMLValue(val)

        val = md.find('metainfo/metrd')
        self.metrd = util.testXMLValue(val)

        val = md.find('metainfo/metc')        
        if val is not None:
            self.metc = Ptcontac(val)

        val = md.find('metainfo/metstdn')
        self.metstdn = util.testXMLValue(val)

        val = md.find('metainfo/metstdv')
        self.metstdv = util.testXMLValue(val)

        val = md.find('metainfo/metac')
        self.metac = util.testXMLValue(val)

        val = md.find('metainfo/metuc')
        self.metuc = util.testXMLValue(val)
