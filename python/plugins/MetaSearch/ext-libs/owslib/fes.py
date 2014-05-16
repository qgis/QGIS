# -*- coding: ISO-8859-15 -*-
# =============================================================================
# Copyright (c) 2009 Tom Kralidis
#
# Authors : Tom Kralidis <tomkralidis@gmail.com>
#
# Contact email: tomkralidis@gmail.com
# =============================================================================

"""
API for OGC Filter Encoding (FE) constructs and metadata.

Filter Encoding: http://www.opengeospatial.org/standards/filter

Currently supports version 1.1.0 (04-095).
"""

from owslib.etree import etree
from owslib import util
from owslib.namespaces import Namespaces

# default variables
def get_namespaces():
    n = Namespaces()
    ns = n.get_namespaces(["dif","fes","gml","ogc","xs","xsi"])
    ns[None] = n.get_namespace("ogc")
    return ns
namespaces = get_namespaces()
schema = 'http://schemas.opengis.net/filter/1.1.0/filter.xsd'
schema_location = '%s %s' % (namespaces['ogc'], schema)

class FilterRequest(object):
    """ filter class """
    def __init__(self, parent=None, version='1.1.0'):
        """

        filter Constructor

        Parameters 
        ----------

        - parent: parent etree.Element object (default is None)
        - version: version (default is '1.1.0')

        """

        self.version = version
        self._root = etree.Element(util.nspath_eval('ogc:Filter', namespaces))
        if parent is not None:
            self._root.set(util.nspath_eval('xsi:schemaLocation', namespaces), schema_location)

    def set(self, parent=False, qtype=None, keywords=[], typenames='csw:Record', propertyname='csw:AnyText', bbox=None, identifier=None):
        """

        Construct and process a  GetRecords request
    
        Parameters
        ----------

        - parent: the parent Element object.  If this is not, then generate a standalone request
        - qtype: type of resource to query (i.e. service, dataset)
        - keywords: list of keywords
        - propertyname: the PropertyName to Filter against 
        - bbox: the bounding box of the spatial query in the form [minx,miny,maxx,maxy]
        - identifier: the dc:identifier to query against with a PropertyIsEqualTo.  Ignores all other inputs.

        """

        # Set the identifier if passed.  Ignore other parameters
        dc_identifier_equals_filter = None
        if identifier is not None:
            dc_identifier_equals_filter = PropertyIsEqualTo('dc:identifier', identifier)
            self._root.append(dc_identifier_equals_filter.toXML())
            return self._root
   
        # Set the query type if passed
        dc_type_equals_filter = None
        if qtype is not None:
            dc_type_equals_filter = PropertyIsEqualTo('dc:type', qtype)

        # Set a bbox query if passed
        bbox_filter = None
        if bbox is not None:
            bbox_filter = BBox(bbox)
    
        # Set a keyword query if passed
        keyword_filter = None
        if len(keywords) > 0:
            if len(keywords) > 1: # loop multiple keywords into an Or
                ks = []    
                for i in keywords:
                    ks.append(PropertyIsLike(propertyname, "*%s*" % i, wildCard="*"))
                keyword_filter = Or(operations=ks)
            elif len(keywords) == 1: # one keyword
                keyword_filter = PropertyIsLike(propertyname, "*%s*" % keywords[0], wildCard="*")
    

        # And together filters if more than one exists
        filters = filter(None,[keyword_filter, bbox_filter, dc_type_equals_filter])
        if len(filters) == 1:
            self._root.append(filters[0].toXML())
        elif len(filters) > 1:
            self._root.append(And(operations=filters).toXML())

        return self._root
       
    def setConstraint(self, constraint):
        """
        Construct and process a  GetRecords request
    
        Parameters
        ----------

        - constraint: An OgcExpression object

        """
        self._root.append(constraint.toXML())
        return self._root

    def setConstraintList(self, constraints):
        """
        Construct and process a  GetRecords request
    
        Parameters
        ----------

        - constraints: A list of OgcExpression objects
                       The list is interpretted like so:

                       [a,b,c]
                       a || b || c

                       [[a,b,c]]
                       a && b && c

                       [[a,b],[c],[d],[e]] or [[a,b],c,d,e]
                       (a && b) || c || d || e

        """
        ors = []
        if len(constraints) == 1:
            if isinstance(constraints[0], OgcExpression):
                return self.setConstraint(constraints[0])
            else:
                self._root.append(And(operations=constraints[0]).toXML())
                return self._root

        for c in constraints:
            if isinstance(c, OgcExpression):
                ors.append(c)
            elif isinstance(c, list) or isinstance(c, tuple):
                if len(c) == 1:
                    ors.append(c[0])
                elif len(c) >= 2:
                    ands = []
                    for sub in c:
                        if isinstance(sub, OgcExpression):
                            ands.append(sub)
                    ors.append(And(operations=ands))

        self._root.append(Or(operations=ors).toXML())
        return self._root


class FilterCapabilities(object):
    """ Abstraction for Filter_Capabilities """
    def __init__(self, elem):
        # Spatial_Capabilities
        self.spatial_operands = [f.text for f in elem.findall(util.nspath_eval('ogc:Spatial_Capabilities/ogc:GeometryOperands/ogc:GeometryOperand', namespaces))]
        self.spatial_operators = []
        for f in elem.findall(util.nspath_eval('ogc:Spatial_Capabilities/ogc:SpatialOperators/ogc:SpatialOperator', namespaces)):
            self.spatial_operators.append(f.attrib['name'])

        # Temporal_Capabilities
        self.temporal_operands = [f.text for f in elem.findall(util.nspath_eval('ogc:Temporal_Capabilities/ogc:TemporalOperands/ogc:TemporalOperand', namespaces))]
        self.temporal_operators = []
        for f in elem.findall(util.nspath_eval('ogc:Temporal_Capabilities/ogc:TemporalOperators/ogc:TemporalOperator', namespaces)):
            self.temporal_operators.append(f.attrib['name'])

        # Scalar_Capabilities
        self.scalar_comparison_operators = [f.text for f in elem.findall(util.nspath_eval('ogc:Scalar_Capabilities/ogc:ComparisonOperators/ogc:ComparisonOperator', namespaces))]

class FilterCapabilities200(object):
    """Abstraction for Filter_Capabilities 2.0"""
    def __init__(self, elem):
        # Spatial_Capabilities
        self.spatial_operands = [f.attrib.get('name') for f in elem.findall(util.nspath_eval('fes:Spatial_Capabilities/fes:GeometryOperands/fes:GeometryOperand', namespaces))]
        self.spatial_operators = []
        for f in elem.findall(util.nspath_eval('fes:Spatial_Capabilities/fes:SpatialOperators/fes:SpatialOperator', namespaces)):
            self.spatial_operators.append(f.attrib['name'])

        # Temporal_Capabilities
        self.temporal_operands = [f.attrib.get('name') for f in elem.findall(util.nspath_eval('fes:Temporal_Capabilities/fes:TemporalOperands/fes:TemporalOperand', namespaces))]
        self.temporal_operators = []
        for f in elem.findall(util.nspath_eval('fes:Temporal_Capabilities/fes:TemporalOperators/fes:TemporalOperator', namespaces)):
            self.temporal_operators.append(f.attrib['name'])

        # Scalar_Capabilities
        self.scalar_comparison_operators = [f.text for f in elem.findall(util.nspath_eval('fes:Scalar_Capabilities/fes:ComparisonOperators/fes:ComparisonOperator', namespaces))]

        # Conformance
        self.conformance = []
        for f in elem.findall(util.nspath_eval('fes:Conformance/fes:Constraint', namespaces)):
           self.conformance[f.attrib.get('name')] = f.find(util.nspath_eval('fes:DefaultValue', namespaces)).text



def setsortby(parent, propertyname, order='ASC'):
    """

    constructs a SortBy element

    Parameters
    ----------

    - parent: parent etree.Element object
    - propertyname: the PropertyName
    - order: the SortOrder (default is 'ASC')

    """

    tmp = etree.SubElement(parent, util.nspath_eval('ogc:SortBy', namespaces))
    tmp2 = etree.SubElement(tmp, util.nspath_eval('ogc:SortProperty', namespaces))
    etree.SubElement(tmp2, util.nspath_eval('ogc:PropertyName', namespaces)).text = propertyname
    etree.SubElement(tmp2, util.nspath_eval('ogc:SortOrder', namespaces)).text = order
    
class SortProperty(object):
    def __init__(self, propertyname, order='ASC'):
        self.propertyname   = propertyname
        self.order          = order.upper()
        if self.order not in ['DESC','ASC']:
            raise ValueError("SortOrder can only be 'ASC' or 'DESC'")
    def toXML(self):
        node0 = etree.Element(util.nspath_eval("ogc:SortProperty", namespaces))
        etree.SubElement(node0, util.nspath_eval('ogc:PropertyName', namespaces)).text = self.propertyname
        etree.SubElement(node0, util.nspath_eval('ogc:SortOrder', namespaces)).text = self.order
        return node0

class SortBy(object):
    def __init__(self, properties):
        self.properties = properties
    def toXML(self):
        node0 = etree.Element(util.nspath_eval("ogc:SortBy", namespaces))
        for prop in self.properties:
            node0.append(prop.toXML())
        return node0

class OgcExpression(object):
    def __init__(self):
        pass

class BinaryComparisonOpType(OgcExpression):
    """ Super class of all the property operation classes"""
    def __init__(self, propertyoperator, propertyname, literal, matchcase=True):
        self.propertyoperator = propertyoperator
        self.propertyname = propertyname
        self.literal = literal
        self.matchcase = matchcase
    def toXML(self):
        node0 = etree.Element(util.nspath_eval(self.propertyoperator, namespaces))
        if self.matchcase is False:
            node0.set('matchCase', 'false')
        etree.SubElement(node0, util.nspath_eval('ogc:PropertyName', namespaces)).text = self.propertyname
        etree.SubElement(node0, util.nspath_eval('ogc:Literal', namespaces)).text = self.literal
        return node0
    
class PropertyIsEqualTo(BinaryComparisonOpType):
    """ PropertyIsEqualTo class"""
    def __init__(self, propertyname, literal, matchcase=True):
        BinaryComparisonOpType.__init__(self, 'ogc:PropertyIsEqualTo',  propertyname, literal, matchcase)

class PropertyIsNotEqualTo(BinaryComparisonOpType):
    """ PropertyIsNotEqualTo class """
    def __init__(self, propertyname, literal, matchcase=True):
        BinaryComparisonOpType.__init__(self, 'ogc:PropertyIsNotEqualTo',  propertyname, literal, matchcase)
        
class PropertyIsLessThan(BinaryComparisonOpType):
    """PropertyIsLessThan class"""
    def __init__(self, propertyname, literal, matchcase=True):
        BinaryComparisonOpType.__init__(self, 'ogc:PropertyIsLessThan',  propertyname, literal, matchcase)

class PropertyIsGreaterThan(BinaryComparisonOpType):
    """PropertyIsGreaterThan class"""
    def __init__(self, propertyname, literal, matchcase=True):
        BinaryComparisonOpType.__init__(self, 'ogc:PropertyIsGreaterThan',  propertyname, literal, matchcase)

class PropertyIsLessThanOrEqualTo(BinaryComparisonOpType):
    """PropertyIsLessThanOrEqualTo class"""
    def __init__(self, propertyname, literal, matchcase=True):
        BinaryComparisonOpType.__init__(self, 'ogc:PropertyIsLessThanOrEqualTo',  propertyname, literal, matchcase)

class PropertyIsGreaterThanOrEqualTo(BinaryComparisonOpType):
    """PropertyIsGreaterThanOrEqualTo class"""
    def __init__(self, propertyname, literal, matchcase=True):
        BinaryComparisonOpType.__init__(self, 'ogc:PropertyIsGreaterThanOrEqualTo',  propertyname, literal, matchcase)

class PropertyIsLike(OgcExpression):
    """PropertyIsLike class"""
    def __init__(self, propertyname, literal, escapeChar='\\', singleChar='_', wildCard='%'):
        self.propertyname = propertyname
        self.literal = literal
        self.escapeChar = escapeChar
        self.singleChar = singleChar
        self.wildCard = wildCard
    def toXML(self):
        node0 = etree.Element(util.nspath_eval('ogc:PropertyIsLike', namespaces))
        node0.set('wildCard', self.wildCard)
        node0.set('singleChar', self.singleChar)
        node0.set('escapeChar', self.escapeChar)
        etree.SubElement(node0, util.nspath_eval('ogc:PropertyName', namespaces)).text = self.propertyname
        etree.SubElement(node0, util.nspath_eval('ogc:Literal', namespaces)).text = self.literal
        return node0

class PropertyIsNull(OgcExpression):
    """PropertyIsNull class"""
    def __init__(self, propertyname):
        self.propertyname = propertyname
    def toXML(self):
        node0 = etree.Element(util.nspath_eval('ogc:PropertyIsNull', namespaces))
        etree.SubElement(node0, util.nspath_eval('ogc:PropertyName', namespaces)).text = self.propertyname
        return node0
        
class PropertyIsBetween(OgcExpression):
    """PropertyIsBetween class"""
    def __init__(self, propertyname, lower, upper):
        self.propertyname = propertyname
        self.lower = lower
        self.upper = upper
    def toXML(self):
        node0 = etree.Element(util.nspath_eval('ogc:PropertyIsBetween', namespaces))
        etree.SubElement(node0, util.nspath_eval('ogc:PropertyName', namespaces)).text = self.propertyname
        node1 = etree.SubElement(node0, util.nspath_eval('ogc:LowerBoundary', namespaces))
        etree.SubElement(node1, util.nspath_eval('ogc:Literal', namespaces)).text = '%s' % self.lower
        node2 = etree.SubElement(node0, util.nspath_eval('ogc:UpperBoundary', namespaces))
        etree.SubElement(node2, util.nspath_eval('ogc:Literal', namespaces)).text = '%s' % self.upper
        return node0
        
class BBox(OgcExpression):
    """Construct a BBox, two pairs of coordinates (west-south and east-north)"""
    def __init__(self, bbox):
        self.bbox = bbox
    def toXML(self):
        tmp = etree.Element(util.nspath_eval('ogc:BBOX', namespaces))
        etree.SubElement(tmp, util.nspath_eval('ogc:PropertyName', namespaces)).text = 'ows:BoundingBox'
        tmp2 = etree.SubElement(tmp, util.nspath_eval('gml:Envelope', namespaces))
        etree.SubElement(tmp2, util.nspath_eval('gml:lowerCorner', namespaces)).text = '%s %s' % (self.bbox[0], self.bbox[1])
        etree.SubElement(tmp2, util.nspath_eval('gml:upperCorner', namespaces)).text = '%s %s' % (self.bbox[2], self.bbox[3])
        return tmp

# BINARY
class BinaryLogicOpType(OgcExpression):
    """ Binary Operators: And / Or """
    def __init__(self, binary_operator, operations):
        self.binary_operator = binary_operator
        try:
            assert len(operations) >= 2
            self.operations = operations
        except:
            raise ValueError("Binary operations (And / Or) require a minimum of two operations to operate against")
    def toXML(self):
        node0 = etree.Element(util.nspath_eval(self.binary_operator, namespaces))
        for op in self.operations:
            node0.append(op.toXML())
        return node0

class And(BinaryLogicOpType):
    def __init__(self, operations):
        super(And,self).__init__('ogc:And', operations)

class Or(BinaryLogicOpType):
    def __init__(self, operations):
        super(Or,self).__init__('ogc:Or', operations)

# UNARY
class UnaryLogicOpType(OgcExpression):
    """ Unary Operator: Not """
    def __init__(self, urary_operator, operations):
        self.urary_operator = urary_operator
        self.operations = operations
    def toXML(self):
        node0 = etree.Element(util.nspath_eval(self.urary_operator, namespaces))
        for op in self.operations:
            node0.append(op.toXML())
        return node0

class Not(UnaryLogicOpType):
    def __init__(self, operations):
        super(Not,self).__init__('ogc:Not', operations)

