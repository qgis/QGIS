from owslib.util import nspath_eval
from owslib.namespaces import Namespaces
from owslib.util import testXMLAttribute, testXMLValue, InfiniteDateTime, NegativeInfiniteDateTime

from dateutil import parser
from datetime import timedelta

from owslib.etree import etree

def get_namespaces():
    ns = Namespaces()
    return ns.get_namespaces(["swe20", "xlink"])
namespaces = get_namespaces()

def nspv(path):
    return nspath_eval(path, namespaces)

def make_pair(string, cast=None):
    if string is None:
        return None

    string = string.split(" ")
    if cast is not None:
        try:
            string = map(lambda x: cast(x), string)
        except:
            print "Could not cast pair to correct type.  Setting to an empty tuple!"
            string = ""

    return tuple(string)

def get_uom(element):

    uom = testXMLAttribute(element, "code")
    if uom is None:
        uom = testXMLAttribute(element, nspv("xlink:href"))
    return uom

def get_boolean(value):
    if value is None:
        return None
    if value is True or value.lower() in ["yes","true"]:
        return True
    elif value is False or value.lower() in ["no","false"]:
        return False
    else:
        return None

def get_int(value):
    try:
        return int(value)
    except:
        return None

def get_float(value):
    try:
        return float(value)
    except:
        return None

AnyScalar    = map(lambda x: nspv(x), ["swe20:Boolean", "swe20:Count", "swe20:Quantity", "swe20:Time", "swe20:Category", "swe20:Text"])
AnyNumerical = map(lambda x: nspv(x), ["swe20:Count", "swe20:Quantity", "swe20:Time"])
AnyRange     = map(lambda x: nspv(x), ["swe20:QuantityRange", "swe20:TimeRange", "swe20:CountRange", "swe20:CategoryRange"])

class NamedObject(object):
    def __init__(self, element):
        # No call to super(), the type object will process that.
        self.name           = testXMLAttribute(element, "name")
        try:
            self.content    = eval(element[-1].tag.split("}")[-1])(element[-1])
        except IndexError:
            self.content    = None
        except BaseException:
            raise

    # Revert to the content if attribute does not exists
    def __getattr__(self, name):
        return getattr(self.content, name)

class AbstractSWE(object):
    def __init__(self, element):
        # Attributes
        self.id             = testXMLAttribute(element,"id")   # string, optional

        # Elements
        self.extension      = []                            # anyType, min=0, max=X

class AbstractSWEIdentifiable(AbstractSWE):
    def __init__(self, element):
        super(AbstractSWEIdentifiable, self).__init__(element)
        # Elements
        self.identifier     = testXMLValue(element.find(nspv("swe20:identifier")))    # anyURI, min=0
        self.label          = testXMLValue(element.find(nspv("swe20:label")))         # string, min=0
        self.description    = testXMLValue(element.find(nspv("swe20:description")))   # string, min=0

class AbstractDataComponent(AbstractSWEIdentifiable):
    def __init__(self, element):
        super(AbstractDataComponent, self).__init__(element)
        # Attributes
        self.definition     = testXMLAttribute(element,"definition")                        # anyURI, required
        self.updatable      = get_boolean(testXMLAttribute(element,"updatable"))            # boolean, optional
        self.optional       = get_boolean(testXMLAttribute(element,"optional")) or False    # boolean, default=False

class AbstractSimpleComponent(AbstractDataComponent):
    def __init__(self, element):
        super(AbstractSimpleComponent, self).__init__(element)
        # Attributes
        self.referenceFrame = testXMLAttribute(element,"referenceFrame")    # anyURI, optional
        self.axisID         = testXMLAttribute(element,"axisID")            # string, optional

        # Elements
        self.quality        = filter(None, [Quality(q) for q in [e.find('*') for e in element.findall(nspv("swe20:quality"))] if q is not None])
        try:
            self.nilValues  = NilValues(element.find(nspv("swe20:nilValues")))
        except:
            self.nilValues  = None

class Quality(object):
    def __new__(cls, element):
        t = element.tag.split("}")[-1]
        if t == "Quantity":
            return Quantity(element)
        elif t == "QuantityRange":
            return QuantityRange(element)
        elif t == "Category":
            return Category(element)
        elif t == "Text":
            return Text(element)
        else:
            return None

class NilValues(AbstractSWE):
    def __init__(self, element):
        super(NilValues, self).__init__(element)
        self.nilValue           = filter(None, [nilValue(x) for x in element.findall(nspv("swe20:nilValue"))]) # string, min=0, max=X

class nilValue(object):
    def __init__(self, element):
        self.reason             = testXMLAttribute(element, "reason")
        self.value              = testXMLValue(element)

class AllowedTokens(AbstractSWE):
    def __init__(self, element):
        super(AllowedTokens, self).__init__(element)
        self.value              = filter(None, [testXMLValue(x) for x in element.findall(nspv("swe20:value"))]) # string, min=0, max=X
        self.pattern            = testXMLValue(element.find(nspv("swe20:pattern")))                             # string (Unicode Technical Standard #18, Version 13), min=0

class AllowedValues(AbstractSWE):
    def __init__(self, element):
        super(AllowedValues, self).__init__(element)
        self.value              = filter(None, map(lambda x: get_float(x), [testXMLValue(x) for x in element.findall(nspv("swe20:value"))]))
        self.interval           = filter(None, [make_pair(testXMLValue(x)) for x in element.findall(nspv("swe20:interval"))])
        self.significantFigures = get_int(testXMLValue(element.find(nspv("swe20:significantFigures"))))                                         # integer, min=0

class AllowedTimes(AbstractSWE):
    def __init__(self, element):
        super(AllowedTimes, self).__init__(element)
        self.value              = filter(None, [testXMLValue(x) for x in element.findall(nspv("swe20:value"))])
        self.interval           = filter(None, [make_pair(testXMLValue(x)) for x in element.findall(nspv("swe20:interval"))])
        self.significantFigures = get_int(testXMLValue(element.find(nspv("swe20:significantFigures"))))                         # integer, min=0

class Boolean(AbstractSimpleComponent):
    def __init__(self, element):
        super(Boolean, self).__init__(element)
        # Elements
        """
        6.2.1 Boolean
                A Boolean representation of a proptery can take only two values that should be "true/false" or "yes/no".
        """
        value               = get_boolean(testXMLValue(element.find(nspv("swe20:value"))))   # boolean, min=0, max=1

class Text(AbstractSimpleComponent):
    def __init__(self, element):
        super(Text, self).__init__(element)
        # Elements
        """
        Req 6. A textual representation shall at least consist of a character string.
        """
        self.value          = testXMLValue(element.find(nspv("swe20:value")))                               # string, min=0, max=1

        try:
            self.constraint     = AllowedTokens(element.find(nspv("swe20:constraint/swe20:AllowedTokens"))) # AllowedTokens, min=0, max=1
        except:
            self.constraint     = None


class Category(AbstractSimpleComponent):
    def __init__(self, element):
        super(Category, self).__init__(element)
        # Elements
        self.codeSpace      = testXMLAttribute(element.find(nspv("swe20:codeSpace")), nspv("xlink:href"))   # Reference, min=0, max=1
        self.value          = testXMLValue(element.find(nspv("swe20:value")))                               # string, min=0, max=1

        try:
            self.constraint     = AllowedTokens(element.find(nspv("swe20:constraint/swe20:AllowedTokens"))) # AllowedTokens, min=0, max=1
        except:
            self.constraint     = None


class CategoryRange(Category):
    def __init__(self, element):
        super(CategoryRange, self).__init__(element)
        # Elements
        value               = testXMLValue(element.find(nspv("swe20:value")))
        self.values         = make_pair(value) if value is not None else None

class Count(AbstractSimpleComponent):
    def __init__(self, element):
        super(Count, self).__init__(element)
        # Elements
        self.value          = get_int(testXMLValue(element.find(nspv("swe20:value"))))                  # integer, min=0, max=1

        try:
            self.constraint = AllowedValues(element.find(nspv("swe20:constraint/swe20:AllowedValues"))) # AllowedValues, min=0, max=1
        except:
            self.constraint = None


class CountRange(Count):
    def __init__(self, element):
        super(CountRange, self).__init__(element)
        # Elements
        value               = testXMLValue(element.find(nspv("swe20:value")))
        self.value          = make_pair(value,int) if value is not None else None

class Quantity(AbstractSimpleComponent):
    def __init__(self, element):
        super(Quantity, self).__init__(element)
        # Elements
        self.uom            = get_uom(element.find(nspv("swe20:uom")))
        self.value          = get_float(testXMLValue(element.find(nspv("swe20:value"))))                  # double, min=0, max=1

        try:
            self.constraint = AllowedValues(element.find(nspv("swe20:constraint/swe20:AllowedValues")))   # AllowedValues, min=0, max=1
        except:
            self.constraint = None

class QuantityRange(Quantity):
    def __init__(self, element):
        super(QuantityRange, self).__init__(element)
        # Elements
        value               = testXMLValue(element.find(nspv("swe20:value")))
        self.value          = make_pair(value,float) if value is not None else None

def get_time(value, referenceTime, uom):
    try:
        value = parser.parse(value)

    except (AttributeError, ValueError): # Most likely an integer/float using a referenceTime
        try:
            if uom.lower() == "s":
                value  = referenceTime + timedelta(seconds=float(value))
            elif uom.lower() == "min":
                value  = referenceTime + timedelta(minutes=float(value))
            elif uom.lower() == "h":
                value  = referenceTime + timedelta(hours=float(value))
            elif uom.lower() == "d":
                value  = referenceTime + timedelta(days=float(value))

        except (AttributeError, ValueError):
            pass

    except OverflowError: # Too many numbers (> 10) or INF/-INF
        if value.lower() == "inf":
            value  = InfiniteDateTime()
        elif value.lower() == "-inf":
            value  = NegativeInfiniteDateTime()

    return value

class Time(AbstractSimpleComponent):
    def __init__(self, element):
        super(Time, self).__init__(element)
        # Elements
        self.uom                = get_uom(element.find(nspv("swe20:uom")))

        try:
            self.constraint     = AllowedTimes(element.find(nspv("swe20:constraint/swe20:AllowedTimes"))) # AllowedTimes, min=0, max=1
        except:
            self.constraint     = None

        # Attributes
        self.localFrame         = testXMLAttribute(element,"localFrame")                                    # anyURI, optional
        try:
            self.referenceTime  = parser.parse(testXMLAttribute(element,"referenceTime"))                   # dateTime, optional
        except (AttributeError, ValueError):
            self.referenceTime  = None

        value                   = testXMLValue(element.find(nspv("swe20:value")))                           # TimePosition, min=0, max=1
        self.value              = get_time(value, self.referenceTime, self.uom)

class TimeRange(AbstractSimpleComponent):
    def __init__(self, element):
        super(TimeRange, self).__init__(element)
        # Elements
        self.uom                = get_uom(element.find(nspv("swe20:uom")))

        try:
            self.constraint     = AllowedTimes(element.find(nspv("swe20:constraint/swe20:AllowedTimes"))) # AllowedTimes, min=0, max=1
        except:
            self.constraint     = None

        # Attributes
        self.localFrame         = testXMLAttribute(element,"localFrame")                                # anyURI, optional
        try:
            self.referenceTime  = parser.parse(testXMLAttribute(element,"referenceTime"))               # dateTime, optional
        except (AttributeError, ValueError):
            self.referenceTime  = None

        values                  = make_pair(testXMLValue(element.find(nspv("swe20:value"))))            # TimePosition, min=0, max=1
        self.value              = [get_time(t, self.referenceTime, self.uom) for t in values]

class DataRecord(AbstractDataComponent):
    def __init__(self, element):
        super(DataRecord, self).__init__(element)
        # Elements
        self.field          = [Field(x) for x in element.findall(nspv("swe20:field"))]
    def get_by_name(self, name):
        return next((x for x in self.field if x.name == name), None)

class Field(NamedObject):
    def __init__(self, element):
        super(Field, self).__init__(element)

class Vector(AbstractDataComponent):
    def __init__(self, element):
        super(Vector, self).__init__(element)
        # Elements
        self.coordinate     = [Coordinate(x) for x in element.findall(nspv("swe20:coordinate"))]

        # Attributes
        self.referenceFrame = testXMLAttribute(element,"referenceFrame")        # anyURI, required
        self.localFrame     = testXMLAttribute(element,"localFrame")            # anyURI, optional
    def get_by_name(self, name):
        return next((x for x in self.coordinate if x.name == name), None)

class Coordinate(NamedObject):
    def __init__(self, element):
        super(Coordinate, self).__init__(element)
        #if element[-1].tag not in AnyNumerical:
        #    print "Coordinate does not appear to be an AnyNumerical member"

class DataChoice(AbstractDataComponent):
    def __init__(self, element):
        super(DataChoice, self).__init__(element)
        self.item           = [Item(x) for x in element.findall(nspv("swe20:item"))]
    def get_by_name(self, name):
        return next((x for x in self.item if x.name == name), None)

class Item(NamedObject):
    def __init__(self, element):
        super(Item, self).__init__(element)

class DataArray(AbstractDataComponent):
    def __init__(self, element):
        super(DataArray, self).__init__(element)
        self.elementCount   = element.find(nspv("swe20:elementCount/swe20:Count"))      # required
        self.elementType    = ElementType(element.find(nspv("swe20:elementType")))      # required
        self.values         = testXMLValue(element.find(nspv("swe20:values")))
        try:
            self.encoding   = AbstractEncoding(element.find(nspv("swe20:encoding")))
        except:
            self.encoding   = None

class Matrix(AbstractDataComponent):
    def __init__(self, element):
        super(Matrix, self).__init__(element)
        self.elementCount   = element.find(nspv("swe20:elementCount/swe20:Count"))      # required
        self.elementType    = ElementType(element.find(nspv("swe20:elementType")))      # required
        self.encoding       = AbstractEncoding(element.find(nspv("swe20:encoding")))
        self.values         = testXMLValue(element.find(nspv("swe20:values")))
        self.referenceFrame = testXMLAttribute(element, "referenceFrame")               # anyURI, required
        self.localFrame     = testXMLAttribute(element, "localFrame")                   # anyURI, optional

class DataStream(AbstractSWEIdentifiable):
    def __init__(self, element):
        super(DataStream, self).__init__(element)
        self.elementCount   = element.find(nspv("swe20:elementCount/swe20:Count"))      # optional
        self.elementType    = ElementType(element.find(nspv("swe20:elementType")))      # optional
        self.encoding       = AbstractEncoding(element.find(nspv("swe20:encoding")))
        self.values         = testXMLValue(element.find(nspv("swe20:values")))

class ElementType(NamedObject):
    def __init__(self, element):
        super(ElementType, self).__init__(element)

class AbstractEncoding(object):
    def __new__(cls, element):
        t = element[-1].tag.split("}")[-1]
        if t == "TextEncoding":
            return super(AbstractEncoding, cls).__new__(TextEncoding, element)
        elif t == "XMLEncoding":
            return super(AbstractEncoding, cls).__new__(XMLEncoding, element)
        elif t == "BinaryEncoding":
            return super(AbstractEncoding, cls).__new__(BinaryEncoding, element)

class TextEncoding(AbstractEncoding):
    def __init__(self, element):
        self.tokenSeparator         = testXMLAttribute(element[-1], "tokenSeparator")                           # string,  required
        self.blockSeparator         = testXMLAttribute(element[-1], "blockSeparator")                           # string,  required
        self.decimalSeparator       = testXMLAttribute(element[-1], "decimalSeparator") or "."                  # string,  optional, default="."
        self.collapseWhiteSpaces    = get_boolean(testXMLAttribute(element[-1], "collapseWhiteSpaces")) or True # boolean, optional, default=True

class XMLEncoding(AbstractEncoding):
    def __init__(self, element):
        raise NotImplementedError

class BinaryEncoding(AbstractEncoding):
    def __init__(self, element):
        raise NotImplementedError
