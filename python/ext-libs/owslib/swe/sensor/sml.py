# encoding: utf-8

from owslib.etree import etree
from owslib import crs, util
from owslib.util import testXMLValue, testXMLAttribute, nspath_eval, xmltag_split, dict_union, extract_xml_list
from owslib.namespaces import Namespaces

def get_namespaces():
    n = Namespaces()
    namespaces = n.get_namespaces(["sml","gml","xlink"])
    namespaces["ism"] = "urn:us:gov:ic:ism:v2"
    return namespaces
namespaces = get_namespaces()

def nsp(path):
    return nspath_eval(path, namespaces)

class SensorML(object):
    def __init__(self, element):
        if isinstance(element, str):
            self._root = etree.fromstring(element)
        else:
            self._root = element

        if hasattr(self._root, 'getroot'):
            self._root = self._root.getroot()

        self.members = [Member(x) for x in self._root.findall(nsp('sml:member'))]

class Member(object):
    def __new__(cls, element):
        t = element[-1].tag.split("}")[-1]
        if t == "System":
            return System(element.find(nsp("sml:System")))
        elif t == "ProcessChain":
            return ProcessChain(element.find(nsp("sml:ProcessChain")))
        elif t == "ProcessModel":
            return ProcessModel(element.find(nsp("sml:ProcessModel")))
        elif t == "Component":
            return Component(element.find(nsp("sml:Component")))

class PropertyGroup(object):
    def __init__(self, element):
        # Both capabilities and characteristics contain a single swe:DataRecord element
        self.capabilities = {}
        for cap in element.findall(nsp('sml:capabilities')):
            name = testXMLAttribute(cap, "name")
            if name is not None:
                self.capabilities[name] = cap[0]

        self.characteristics = {}
        for cha in element.findall(nsp('sml:characteristics')):
            name = testXMLAttribute(cha, "name")
            if name is not None:
                self.characteristics[name] = cha[0]

    def get_capabilities_by_name(self, name):
        """
            Return list of element by name, case insensitive
        """
        return [self.capabilities[capab] for capab in self.capabilities.keys() if capab.lower() == name.lower()]

    def get_characteristics_by_name(self, name):
        """
            Return list of element objects by name, case insensitive
        """
        return [self.characteristics[charac] for charac in self.characteristics.keys() if charac.lower() == name.lower()]

class ConstraintGroup(object):
    def __init__(self, element):
        # ism:SecurityAttributesOptionsGroup
        self.security            = element.findall(nsp("sml:securityConstraint/sml:Security/ism:SecurityAttributesOptionGroup"))
        # gml:TimeInstant or gml:TimePeriod element
        self.validTime           = element.find(nsp("sml:validTime"))
        self.rights              = [Right(x) for x in element.findall(nsp("sml:legalConstraint/sml:Rights"))]

class Documentation(object):
    def __init__(self, element):
        self.arcrole   = testXMLAttribute(element, nsp("xlink:arcrole"))
        self.url       = testXMLAttribute(element, nsp("xlink:href"))
        self.documents = [Document(d) for d in element.findall(nsp("sml:Document"))]

class Document(object):
    def __init__(self, element):
        self.id          = testXMLAttribute(element, nsp("gml:id"))
        self.version     = testXMLValue(element.find(nsp("sml:version")))
        self.description = testXMLValue(element.find(nsp("gml:description")))
        self.date        = testXMLValue(element.find(nsp("sml:date")))
        try:
            self.contact     = Contact(element.find(nsp("sml:contact")))
        except AttributeError:
            self.contact     = None
        self.format      = testXMLValue(element.find(nsp('sml:format')))
        self.url         = testXMLAttribute(element.find(nsp('sml:onlineResource')), nsp('xlink:href'))

class Right(object):
    def __init__(self, element):
        self.id                         = testXMLAttribute(element, nsp('gml:id'))
        self.privacyAct                 = testXMLAttribute(element, nsp('sml:privacyAct'))
        self.intellectualPropertyRights = testXMLAttribute(element, nsp('sml:intellectualPropertyRights'))
        self.copyRights                 = testXMLAttribute(element, nsp('sml:copyRights'))
        self.documentation              = [Documentation(x) for x in element.findall(nsp("sml:documentation"))]

class ReferenceGroup(object):
    def __init__(self, element):
        self.contacts = {}
        for contact in element.findall(nsp('sml:contact')):
            cont                     = Contact(contact)
            self.contacts[cont.role] = cont

        self.documentation = [Documentation(x) for x in element.findall(nsp("sml:documentation"))]

    def get_contacts_by_role(self, role):
        """
            Return a Contact by role, case insensitive
        """
        return [self.contacts[contact] for contact in self.contacts.keys() if contact.lower() == role.lower()]

class GeneralInfoGroup(object):
    def __init__(self, element):
        self.keywords    = extract_xml_list(element.findall(nsp('sml:keywords/sml:KeywordList/sml:keyword')))

        self.identifiers = {}
        for identifier in element.findall(nsp('sml:identification/sml:IdentifierList/sml:identifier')):
            ident = Identifier(identifier)
            self.identifiers[ident.name] = ident

        self.classifiers = {}
        for classifier in element.findall(nsp('sml:classification/sml:ClassifierList/sml:classifier')):
            classi = Classifier(classifier)
            self.classifiers[classi.name] = classi

    def get_identifiers_by_name(self, name):
        """
            Return list of Identifier objects by name, case insensitive
        """
        return [self.identifiers[identifier] for identifier in self.identifiers.keys() if identifier.lower() == name.lower()]

    def get_classifiers_by_name(self, name):
        """
            Return list of Classifier objects by name, case insensitive
        """
        return [self.classifiers[classi] for classi in self.classifiers.keys() if classi.lower() == name.lower()]

class Contact(object):
    def __init__(self, element):
        # TODO: This only supports the sml:contact/sml:ResponsibleParty elements, but there are numerous ways to store
        # contact information here.
        self.role         = testXMLAttribute(element, nsp("xlink:role"))
        self.href         = testXMLAttribute(element, nsp("xlink:href"))
        self.organization = testXMLValue(element.find(nsp('sml:ResponsibleParty/sml:organizationName')))
        self.phone        = testXMLValue(element.find(nsp('sml:ResponsibleParty/sml:contactInfo/sml:phone/sml:voice')))
        self.address      = testXMLValue(element.find(nsp('sml:ResponsibleParty/sml:contactInfo/sml:address/sml:deliveryPoint')))
        self.city         = testXMLValue(element.find(nsp('sml:ResponsibleParty/sml:contactInfo/sml:address/sml:city')))
        self.region       = testXMLValue(element.find(nsp('sml:ResponsibleParty/sml:contactInfo/sml:address/sml:administrativeArea')))
        self.postcode     = testXMLValue(element.find(nsp('sml:ResponsibleParty/sml:contactInfo/sml:address/sml:postalCode')))
        self.country      = testXMLValue(element.find(nsp('sml:ResponsibleParty/sml:contactInfo/sml:address/sml:country')))
        self.email        = testXMLValue(element.find(nsp('sml:ResponsibleParty/sml:contactInfo/sml:address/sml:electronicMailAddress')))
        self.url          = testXMLAttribute(element.find(nsp('sml:ResponsibleParty/sml:contactInfo/sml:onlineResource')), nsp("xlink:href"))

class HistoryGroup(object):
    def __init__(self, element):
        self.history = {}
        for event_member in element.findall(nsp('sml:history/sml:EventList/sml:member')):
            name = testXMLAttribute(event_member, "name")
            if self.history.get(name) is None:
                self.history[name] = []
            for e in event_member.findall(nsp("sml:Event")):
                self.history[name].append(Event(e))

    def get_history_by_name(self, name):
        """
            Return Events list by members name
        """
        return self.history.get(name.lower(), [])

class Event(ReferenceGroup, GeneralInfoGroup):
    def __init__(self, element):
        ReferenceGroup.__init__(self, element)
        GeneralInfoGroup.__init__(self, element)
        self.id            = testXMLAttribute(element, nsp("gml:id"))
        self.date          = testXMLValue(element.find(nsp('sml:date')))
        self.description   = testXMLValue(element.find(nsp('gml:description')))

class MetadataGroup(GeneralInfoGroup, PropertyGroup, ConstraintGroup, ReferenceGroup, HistoryGroup):
    def __init__(self, element):
        GeneralInfoGroup.__init__(self, element)
        PropertyGroup.__init__(self, element)
        ConstraintGroup.__init__(self, element)
        ReferenceGroup.__init__(self, element)
        HistoryGroup.__init__(self, element)

class AbstractFeature(object):
    def __init__(self, element):
        self.name         = testXMLValue(element.find(nsp("gml:name")))
        self.description  = testXMLValue(element.find(nsp("gml:description")))
        self.gmlBoundedBy = testXMLValue(element.find(nsp("gml:boundedBy")))

class AbstractProcess(AbstractFeature, MetadataGroup):
    def __init__(self, element):
        AbstractFeature.__init__(self, element)
        MetadataGroup.__init__(self, element)
        # sml:IoComponentPropertyType
        self.inputs     = element.findall(nsp("sml:input"))
        # sml:IoComponentPropertyType
        self.outputs    = element.findall(nsp("sml:output"))
        # swe:DataComponentPropertyType
        self.parameters = element.findall(nsp("sml:parameter"))

class AbstractRestrictedProcess(AbstractFeature):
    """ Removes ('restricts' in xml schema language) gml:name, gml:description, and sml:metadataGroup from an AbstractProcess """
    def __init__(self, element):
        AbstractFeature.__init__(self, element)
        self.name        = None
        self.description = None

class AbstractPureProcess(AbstractRestrictedProcess):
    def __init__(self, element):
        AbstractRestrictedProcess.__init__(self, element)

        # sml:IoComponentPropertyType
        self.inputs      = element.findall(nsp("sml:input"))
        # sml:IoComponentPropertyType
        self.outputs     = element.findall(nsp("sml:output"))
        # swe:DataComponentPropertyType
        self.parameters  = element.findall(nsp("sml:parameter"))

class ProcessModel(AbstractPureProcess):
    def __init__(self, element):
        AbstractPureProcess.__init__(self, element)
        self.method = ProcessMethod(element.find("method"))

class CompositePropertiesGroup(object):
    def __init__(self, element):
        # All components should be of instance AbstractProcess (sml:_Process)
        self.components  = element.findall(nsp("sml:components/sml:ComponentList/sml:component"))
        # sml:Link or sml:ArrayLink element
        self.connections = element.findall(nsp("sml:connections/sml:ConnectionList/sml:connection"))

class PhysicalPropertiesGroup(object):
    def __init__(self, element):
        # gml:EngieeringCRS element
        self.spatialReferenceFrame  = element.find(nsp("sml:spatialReferenceFrame/gml:EngineeringCRS"))
        # gml:TemporalCRS element
        self.temporalReferenceFrame = element.find(nsp("sml:temporalReferenceFrame/gml:TemporalCRS"))
        # gml:Envelope element
        self.smlBoundedBy           = element.find(nsp("sml:boundedBy"))
        # swe:Time or sml:_Process element
        self.timePosition           = element.find(nsp("sml:timePosition"))

        # It is either a sml:position OR and sml:location element here.  Process both.
        # swe:Position, swe:Vector, or sml:_Process element
        self.positions              = element.findall(nsp("sml:position"))
        # gml:Point of gml:_Curve
        self.location               = element.find(nsp("sml:location"))

        try:
            self.interface = Interface(element.find(nsp("sml:interface")))
        except AttributeError:
            self.interface = None

class ProcessChain(AbstractPureProcess, CompositePropertiesGroup):
    def __init__(self, element):
        AbstractPureProcess.__init__(self, element)
        CompositePropertiesGroup.__init__(self, element)

class System(AbstractProcess, PhysicalPropertiesGroup, CompositePropertiesGroup):
    def __init__(self, element):
        AbstractProcess.__init__(self, element)
        PhysicalPropertiesGroup.__init__(self, element)
        CompositePropertiesGroup.__init__(self, element)

class Component(AbstractProcess, PhysicalPropertiesGroup):
    def __init__(self, element):
        AbstractProcess.__init__(self, element)
        PhysicalPropertiesGroup.__init__(self, element)
        self.method = ProcessMethod(element.find("method"))

class Term(object):
    def __init__(self, element):
        self.codeSpace  = testXMLAttribute(element.find(nsp('sml:Term/sml:codeSpace')), nsp("xlink:href"))
        self.definition = testXMLAttribute(element.find(nsp('sml:Term')), "definition")
        self.value      = testXMLValue(element.find(nsp('sml:Term/sml:value')))

class Classifier(Term):
    def __init__(self, element):
        Term.__init__(self, element)
        self.name      = testXMLAttribute(element, "name")

class Identifier(Term):
    def __init__(self, element):
        Term.__init__(self, element)
        self.name      = testXMLAttribute(element, "name")

class ProcessMethod(MetadataGroup):
    """ Inherits from gml:AbstractGMLType """
    def __init__(self, element):
        MetadataGroup.__init__(self, element)
        self.rules           = element.find(nsp("sml:rules"))
        self.ioStructure     = element.find(nsp("sml:IOStructureDefinition"))
        self.algorithm       = element.find(nsp("sml:algorithm"))
        self.implementations = element.findall(nsp("sml:implementation"))

class Interface(object):
    def __init__(self, element):
        self.name                 = testXMLAttribute(element, "name")
        self.interface_definition = InterfaceDefinition(element.find(nsp("sml:InterfaceDefinition")))

class InterfaceDefinition(object):
    def __init__(self, element):
        raise NotImplementedError("InterfaceDefinition is not implemented in OWSLib (yet)")

class Link(object):
    def __init__(self, element):
        raise NotImplementedError("Link is not implemented in OWSLib (yet)")

class ArrayLink(object):
    def __init__(self, element):
        raise NotImplementedError("ArrayLink is not implemented in OWSLib (yet)")
