# =============================================================================
# OWSLib. Copyright (C) 2005 Sean C. Gillies
#
# Contact email: sgillies@frii.com
# =============================================================================

from __future__ import (absolute_import, division, print_function)
import six
import inspect

def patch_well_known_namespaces(etree_module):

    import warnings
    from owslib.namespaces import Namespaces
    ns = Namespaces()

    """Monkey patches the etree module to add some well-known namespaces."""

    try:
        register_namespace = etree_module.register_namespace
    except AttributeError:
        try:
            etree_module._namespace_map

            def register_namespace(prefix, uri):
                etree_module._namespace_map[uri] = prefix
        except AttributeError:
            def register_namespace(prefix, uri):
                pass
            warnings.warn("Only 'lxml.etree' >= 2.3 and 'xml.etree.ElementTree' >= 1.3 are fully supported!")

    for k, v in six.iteritems(ns.get_namespaces()):
        register_namespace(k, v)

# try to find lxml or elementtree
try:
    from lxml import etree
    from lxml.etree import ParseError
    ElementType = etree._Element
except ImportError:
    try:
        # Python 2.x/3.x with ElementTree included
        import xml.etree.ElementTree as etree

        try:
            from xml.etree.ElementTree import ParseError
        except ImportError:
            from xml.parsers.expat import ExpatError as ParseError

        if hasattr(etree, 'Element') and inspect.isclass(etree.Element):
            # python 3.4, 3.3, 2.7
            ElementType = etree.Element
        else:
            # python 2.6
            ElementType = etree._ElementInterface

    except ImportError:
        try:
            # Python < 2.5 with ElementTree installed
            import elementtree.ElementTree as etree
            ParseError = StandardError      # i can't find a ParseError related item in elementtree docs!
            ElementType = etree.Element
        except ImportError:
            raise RuntimeError('You need either lxml or ElementTree to use OWSLib!')

patch_well_known_namespaces(etree)
