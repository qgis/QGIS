import logging
from unittest import TestSuite

from nose2.events import Plugin

log = logging.getLogger(__name__)
undefined = object()


class AttributeSelector(Plugin):

    """Filter tests by attribute"""

    def __init__(self):
        self.attribs = []
        self.eval_attribs = []
        self.addArgument(
            self.attribs, "A", "attribute",
            "Select tests with matching attribute")
        self.addArgument(
            self.eval_attribs, "E", "eval-attribute",
            "Select tests for whose attributes the "
            "given Python expression evalures to True")

    def handleArgs(self, args):
        """Register if any attribs defined"""
        if self.attribs or self.eval_attribs:
            self.register()

    def moduleLoadedSuite(self, event):
        """Filter event.suite by specified attributes"""
        log.debug('Attribute selector attribs %s/%s',
                  self.attribs, self.eval_attribs)
        attribs = []
        for attr in self.eval_attribs:
            def eval_in_context(expr, obj):
                try:
                    return eval(expr, None, ContextHelper(obj))
                except Exception as e:
                    log.warning(
                        "%s raised exception %s with test %s", expr, e, obj)
                    return False
            attribs.append([(attr, eval_in_context)])
        for attr in self.attribs:
            # all attributes within an attribute group must match
            attr_group = []
            for attrib in attr.strip().split(","):
                # don't die on trailing comma
                if not attrib:
                    continue
                items = attrib.split("=", 1)
                if len(items) > 1:
                    # "name=value"
                    # -> 'str(obj.name) == value' must be True
                    key, value = items
                else:
                    key = items[0]
                    if key[0] == "!":
                        # "!name"
                        # 'bool(obj.name)' must be False
                        key = key[1:]
                        value = False
                    else:
                        # "name"
                        # -> 'bool(obj.name)' must be True
                        value = True
                attr_group.append((key, value))
            attribs.append(attr_group)
        if not attribs:
            return

        event.suite = self.filterSuite(event.suite, attribs)

    def filterSuite(self, suite, attribs):
        # FIXME probably need to copy or something to allow suites w/custom attrs to work or iter and remove instead of
        # recreating
        new_suite = suite.__class__()

        for test in suite:
            if isinstance(test, TestSuite):
                new_suite.addTest(self.filterSuite(test, attribs))
            elif self.validateAttrib(test, attribs):
                new_suite.addTest(test)
        return new_suite

    def validateAttrib(self, test, attribs):
        any_ = False
        for group in attribs:
            match = True
            for key, value in group:
                neg = False
                if key.startswith('!'):
                    neg, key = True, key[1:]
                obj_value = _get_attr(test, key)
                if callable(value):
                    if not value(key, test):
                        match = False
                        break
                elif value is True:
                    # value must exist and be True
                    if not bool(obj_value):
                        match = False
                        break
                elif value is False:
                    # value must not exist or be False
                    if bool(obj_value):
                        match = False
                        break
                elif type(obj_value) in (list, tuple):
                    # value must be found in the list attribute
                    found = str(value).lower() in [str(x).lower()
                                                   for x in obj_value]
                    if found and neg:
                        match = False
                        break
                    elif not found and not neg:
                        match = False
                        break
                else:
                    # value must match, convert to string and compare
                    if (value != obj_value
                        and str(value).lower() != str(obj_value).lower()):
                        match = False
                        break
            any_ = any_ or match
        return any_


# helpers

def _get_attr(test, key):
    # FIXME for vals that are lists (or just mutable?), combine all levels
    val = getattr(test, key, undefined)
    if val is not undefined:
        return val
    if hasattr(test, '_testFunc'):
        val = getattr(test._testFunc, key, undefined)
        if val is not undefined:
            return val
    elif hasattr(test, '_testMethodName'):
        meth = getattr(test, test._testMethodName, undefined)
        if meth is not undefined:
            val = getattr(meth, key, undefined)
            if val is not undefined:
                return val


class ContextHelper:

    def __init__(self, obj):
        self.obj = obj

    def __getitem__(self, name):
        return _get_attr(self.obj, name)
