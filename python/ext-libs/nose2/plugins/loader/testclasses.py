"""
Load tests from classes that are *not* :class:`unittest.TestCase` subclasses.

This plugin responds to :func:`loadTestsFromModule` by adding test
cases for test methods found in classes in the module that are *not*
sublcasses of :class:`unittest.TestCase`, but whose names (lowercased)
match the configured test method prefix.

Test class methods that are generators or have param lists are not
loaded here, but by the :class:`nose2.plugins.loader.generators.Generators` and
:class:`nose2.plugins.loader.parameters.Parameters` plugins.

This plugin also implements :func:`loadTestsFromName` to enable
loading tests from dotted class and method names passed on the command
line.

This plugin makes two additional plugin hooks available for other
test loaders to use:

.. function :: loadTestsFromTestClass(self, event)

   :param event: A :class:`LoadFromTestClassEvent` instance

   Plugins can use this hook to load tests from a class that is not a
   :class:`unittest.TestCase` subclass. To prevent other plugins from
   loading tests from the test class, set ``event.handled`` to True and
   return a test suite. Plugins can also append tests to
   ``event.extraTests`` -- ususally that's what you want to do, since
   that will allow other plugins to load their tests from the test
   case as well.

.. function :: getTestMethodNames(self, event)

   :param event: A :class:`GetTestMethodNamesEvent` instance

   Plugins can use this hook to limit or extend the list of test case
   names that will be loaded from a class that is not a
   :class:`unittest.TestCase` subclass by the standard nose2 test
   loader plugins (and other plugins that respect the results of the
   hook). To force a specific list of names, set ``event.handled`` to
   True and return a list: this exact list will be the only test case
   names loaded from the test case. Plugins can also extend the list
   of names by appending test names to ``event.extraNames``, and
   exclude names by appending test names to ``event.excludedNames``.

About Test Classes
------------------

Test classes are classes that look test-like but are not subclasses of
:class:`unittest.TestCase`. Test classes support all of the same test
types and fixtures as test cases.

To "look test-like" a class must have a name that, lowercased, matches
the configured test method prefix -- "test" by default. Test classes
must also be able to be instantiated without arguments.

What are they useful for? Mostly the case where a test class can't for
some reason subclass :class:`unittest.TestCase`. Otherwise, test class
tests and test cases are functionally equivalent in nose2, and test
cases have broader support and all of those helpful *assert\** methods
-- so when in doubt, you should use a :class:`unittest.TestCase`.

Here's an example of a test class::

  class TestSomething(object):

      def test(self):
          assert self.something(), "Something failed!"

"""

import unittest
import sys

from nose2 import events, util
from nose2.compat import unittest as ut2

__unittest = True


class TestClassLoader(events.Plugin):

    """Loader plugin that loads test functions"""
    alwaysOn = True
    configSection = 'test-classes'

    def registerInSubprocess(self, event):
        event.pluginClasses.append(self.__class__)

    def register(self):
        """Install extra hooks

        Adds the new plugin hooks:

        - loadTestsFromTestClass
        - getTestMethodNames

        """
        super(TestClassLoader, self).register()
        self.addMethods('loadTestsFromTestClass', 'getTestMethodNames')

    def loadTestsFromModule(self, event):
        """Load test classes from event.module"""
        module = event.module
        for name in dir(module):
            obj = getattr(module, name)
            if (isinstance(obj, type) and
                not issubclass(obj, unittest.TestCase) and
                not issubclass(obj, unittest.TestSuite) and
                name.lower().startswith(self.session.testMethodPrefix)):
                event.extraTests.append(
                    self._loadTestsFromTestClass(event, obj))

    def loadTestsFromName(self, event):
        """Load tests from event.name if it names a test class/method"""
        name = event.name
        module = event.module
        try:
            result = util.test_from_name(name, module)
        except (AttributeError, ImportError) as e:
            event.handled = True
            return event.loader.failedLoadTests(name, e)
        if result is None:
            return
        parent, obj, name, index = result
        if isinstance(obj, type) and not issubclass(obj, unittest.TestCase):
            # name is a test case class
            event.extraTests.append(self._loadTestsFromTestClass(event, obj))
        elif (isinstance(parent, type) and
              not issubclass(parent, unittest.TestCase) and
              not util.isgenerator(obj) and
              not hasattr(obj, 'paramList')):
            # name is a single test method
            event.extraTests.append(
                util.transplant_class(
                    MethodTestCase(parent), parent.__module__)(obj.__name__))

    def _loadTestsFromTestClass(self, event, cls):
        # ... fire event for others to load from
        evt = LoadFromTestClassEvent(event.loader, cls)
        result = self.session.hooks.loadTestsFromTestClass(evt)
        if evt.handled:
            loaded_suite = result or event.loader.suiteClass()
        else:
            names = self._getTestMethodNames(event, cls)
            try:
                loaded_suite = event.loader.suiteClass(
                    [util.transplant_class(
                     MethodTestCase(cls), cls.__module__)(name)
                        for name in names])
            except:
                _, ev, _ = sys.exc_info()
                return event.loader.suiteClass(
                    event.loader.failedLoadTests(cls.__name__, ev))
        if evt.extraTests:
            loaded_suite.addTests(evt.extraTests)
        # ... add extra tests
        return loaded_suite

    def _getTestMethodNames(self, event, cls):
        # ... give others a chance to modify list
        excluded = set()

        def isTestMethod(attrname, cls=cls, excluded=excluded):
            # FIXME allow plugs to change prefix
            prefix = self.session.testMethodPrefix
            return (
                attrname.startswith(prefix) and
                hasattr(getattr(cls, attrname), '__call__') and
                attrname not in excluded
            )
        evt = GetTestMethodNamesEvent(event.loader, cls, isTestMethod)
        result = self.session.hooks.getTestMethodNames(evt)
        if evt.handled:
            test_names = result or []
        else:
            excluded.update(evt.excludedNames)

            test_names = [entry for entry in dir(cls)
                          if isTestMethod(entry)]

        if event.loader.sortTestMethodsUsing:
            test_names.sort(key=event.loader.sortTestMethodsUsing)
        return test_names


# to prevent unit2 discover from running this as a test, need to
# hide it inside of a factory func. ugly!
def MethodTestCase(cls):
    class _MethodTestCase(ut2.TestCase):

        def __init__(self, method):
            self.method = method
            self._name = "%s.%s.%s" % (cls.__module__, cls.__name__, method)
            self.obj = cls()
            ut2.TestCase.__init__(self, 'runTest')

        @classmethod
        def setUpClass(klass):
            if hasattr(cls, 'setUpClass'):
                cls.setUpClass()

        @classmethod
        def tearDownClass(klass):
            if hasattr(cls, 'tearDownClass'):
                cls.tearDownClass()

        def setUp(self):
            if hasattr(self.obj, 'setUp'):
                self.obj.setUp()

        def tearDown(self):
            if hasattr(self.obj, 'tearDown'):
                self.obj.tearDown()

        def __repr__(self):
            return self._name
        id = __str__ = __repr__

        def runTest(self):
            getattr(self.obj, self.method)()

    return _MethodTestCase

#
# Event classes
#


class LoadFromTestClassEvent(events.LoadFromTestCaseEvent):

    """Bare subclass of :class:`nose2.events.LoadFromTestCaseEvent`"""


class GetTestMethodNamesEvent(events.GetTestCaseNamesEvent):

    """Bare subclass of :class:`nose2.events.GetTestCaseNamesEvent`"""
