class Namespaces(object):
    """
        Class for holding and maniputlating a dictionary containing the various namespaces for
        each standard.
    """

    namespace_dict = {
        'atom'  :   'http://www.w3.org/2005/Atom',
        'csw'   :   'http://www.opengis.net/cat/csw/2.0.2',
        'dc'    :   'http://purl.org/dc/elements/1.1/',
        'dct'   :   'http://purl.org/dc/terms/',
        'dif'   :   'http://gcmd.gsfc.nasa.gov/Aboutus/xml/dif/',
        'draw'  :   'gov.usgs.cida.gdp.draw',
        'fes'   :   'http://www.opengis.net/fes/2.0',
        'fgdc'  :   'http://www.opengis.net/cat/csw/csdgm',
        'gco'   :   'http://www.isotc211.org/2005/gco',
        'gmd'   :   'http://www.isotc211.org/2005/gmd',
        'gml'   :   'http://www.opengis.net/gml',
        'gml311':   'http://www.opengis.net/gml',
        'gml32' :   'http://www.opengis.net/gml/3.2',
        'gmx'   :   'http://www.isotc211.org/2005/gmx',
        'gts'   :   'http://www.isotc211.org/2005/gts',
        'ogc'   :   'http://www.opengis.net/ogc',
        'om'    :   'http://www.opengis.net/om/1.0',
        'om10'  :   'http://www.opengis.net/om/1.0',
        'om100' :   'http://www.opengis.net/om/1.0',
        'om20'  :   'http://www.opengis.net/om/2.0',
        'ows'   :   'http://www.opengis.net/ows',
        'ows100':   'http://www.opengis.net/ows',
        'ows110':   'http://www.opengis.net/ows/1.1',
        'ows200':   'http://www.opengis.net/ows/2.0',
        'rim'   :   'urn:oasis:names:tc:ebxml-regrep:xsd:rim:3.0',
        'rdf'   :   'http://www.w3.org/1999/02/22-rdf-syntax-ns#',
        'sml'   :   'http://www.opengis.net/sensorML/1.0.1',
        'sml101':   'http://www.opengis.net/sensorML/1.0.1',
        'sos'   :   'http://www.opengis.net/sos/1.0',
        'sos20' :   'http://www.opengis.net/sos/2.0',
        'srv'   :   'http://www.isotc211.org/2005/srv',
        'swe'   :   'http://www.opengis.net/swe/1.0.1',
        'swe10' :   'http://www.opengis.net/swe/1.0',
        'swe101':   'http://www.opengis.net/swe/1.0.1',
        'swe20' :   'http://www.opengis.net/swe/2.0',
        'swes'  :   'http://www.opengis.net/swes/2.0',
        'tml'   :   'ttp://www.opengis.net/tml',
        'wfs'   :   'http://www.opengis.net/wfs',
        'wfs20' :   'http://www.opengis.net/wfs/2.0',
        'wcs'   :   'http://www.opengis.net/wcs',
        'wps'   :   'http://www.opengis.net/wps/1.0.0',
        'wps100':   'http://www.opengis.net/wps/1.0.0',
        'xlink' :   'http://www.w3.org/1999/xlink',
        'xs'    :   'http://www.w3.org/2001/XMLSchema',
        'xs2'   :   'http://www.w3.org/XML/Schema',
        'xsi'   :   'http://www.w3.org/2001/XMLSchema-instance'
    }    

    def get_namespace(self, key):
        """
            Retrieves a namespace from the dictionary

            Example:
            --------

            >>> from owslib.namespaces import Namespaces
            >>> ns = Namespaces()
            >>> ns.get_namespace('csw')
            'http://www.opengis.net/cat/csw/2.0.2'
            >>> ns.get_namespace('wfs20')
            'http://www.opengis.net/wfs/2.0'
        """
        retval = None
        if key in self.namespace_dict.keys():
            retval = self.namespace_dict[key]
        return retval
    
    def get_versioned_namespace(self, key, ver=None):
        """
            Retrieves a namespace from the dictionary with a specific version number

            Example:
            --------

            >>> from owslib.namespaces import Namespaces
            >>> ns = Namespaces()
            >>> ns.get_versioned_namespace('ows')
            'http://www.opengis.net/ows'
            >>> ns.get_versioned_namespace('ows','1.1.0')
            'http://www.opengis.net/ows/1.1'
        """
        
        if ver is None:
            return self.get_namespace(key)

        version = ''
        # Strip the decimals out of the passed in version
        for s in ver.split('.'):
            version += s
        
        key += version

        retval = None
        if key in self.namespace_dict.keys():
            retval = self.namespace_dict[key]
            
        return retval
    
    def get_namespaces(self, keys=None):
        """
            Retrieves a dict of namespaces from the namespace mapping

            Parameters
            ----------
            - keys: List of keys query and return

            Example:
            --------
            >>> ns = Namespaces()
            >>> ns.get_namespaces(['csw','gmd'])
            { 'csw' : http://www.opengis.net/cat/csw/2.0.2', 'gmd' : 'http://www.isotc211.org/2005/gmd' }
            >>> ns.get_namespaces('csw')
            { 'csw' : http://www.opengis.net/cat/csw/2.0.2' }
            >>> ns.get_namespaces()
            {...}
        """
        # If we aren't looking for any namespaces in particular return the whole dict
        if keys is None or len(keys) == 0:
            return self.namespace_dict

        if isinstance(keys, unicode) or isinstance(keys, str):
            return { keys: self.get_namespace(keys) }

        retval = {}
        for key in keys:
            if key in self.namespace_dict.keys():
                retval[key] = self.namespace_dict[key]

        return retval