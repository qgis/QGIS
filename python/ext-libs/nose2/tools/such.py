from contextlib import contextmanager
import inspect
import logging
import sys

import six

from nose2.compat import unittest
from nose2 import util
from nose2.main import PluggableTestProgram

log = logging.getLogger(__name__)

__unittest = True

LAYERS_PLUGIN_NOT_LOADED_MESSAGE = 'Warning: Such will not function properly if the "nose2.plugins.layers" plugin not loaded!\n'

@contextmanager
def A(description):
    """Test scenario context manager.

    Returns a :class:`nose2.tools.such.Scenario` instance,
    which by convention is bound to ``it``:

    .. code-block :: python

      with such.A('test scenario') as it:
          # tests and fixtures

    """
    yield Scenario(description)


class Helper(unittest.TestCase):

    def runTest(self):
        pass


helper = Helper()


class Scenario(object):

    """A test scenario.

    A test scenario defines a set of fixtures and tests
    that depend on those fixtures.
    """
    _helper = helper

    def __init__(self, description):
        self._group = Group('A %s' % description, 0)

    @contextmanager
    def having(self, description):
        """Define a new group under the current group.

        Fixtures and tests defined within the block will
        belong to the new group.

        .. code-block :: python

           with it.having('a description of this group'):
               # ...

        """
        last = self._group
        self._group = self._group.child(
            "having %s" % description)
        log.debug("starting new group from %s", description)
        yield self
        log.debug("leaving group %s", description)
        self._group = last

    def uses(self, layer):
        log.debug("Adding %s as mixin to %s", layer, self._group)
        self._group.mixins.append(layer)

    def has_setup(self, func):
        """Add a setup method to this group.

        The setup method will run once, before any of the
        tests in the containing group.

        A group may define any number of setup functions. They
        will execute in the order in which they are defined.

        .. code-block :: python

           @it.has_setup
           def setup():
               # ...

        """
        self._group.addSetup(func)
        return func

    def has_teardown(self, func):
        """Add a teardown method to this group.

        The teardown method will run once, after all of the
        tests in the containing group.

        A group may define any number of teardown functions. They
        will execute in the order in which they are defined.

        .. code-block :: python

           @it.has_teardown
           def teardown():
               # ...

        """
        self._group.addTeardown(func)
        return func

    def has_test_setup(self, func):
        """Add a test case setup method to this group.

        The setup method will run before each of the
        tests in the containing group.

        A group may define any number of test case setup
        functions. They will execute in the order in which they are
        defined.

        Test setup functions may optionally take one argument. If
        they do, they will be passed the :class:`unittest.TestCase`
        instance generated for the test.

        .. code-block :: python

           @it.has_test_setup
           def setup(case):
               # ...

        """
        self._group.addTestSetUp(func)

    def has_test_teardown(self, func):
        """Add a test case teardown method to this group.

        The teardown method will run before each of the
        tests in the containing group.

        A group may define any number of test case teardown
        functions. They will execute in the order in which they are
        defined.

        Test teardown functions may optionally take one argument. If
        they do, they will be passed the :class:`unittest.TestCase`
        instance generated for the test.

        .. code-block :: python

           @it.has_test_teardown
           def teardown(case):
               # ...

        """
        self._group.addTestTearDown(func)

    def should(self, desc):
        """Define a test case.

        Each function marked with this decorator becomes a test
        case in the current group.

        The decorator takes one optional argument, the description
        of the test case: what it **should** do. If this argument
        is not provided, the docstring of the decorated function
        will be used as the test case description.

        Test functions may optionally take one argument. If they do,
        they will be passed the :class:`unittest.TestCase` instance generated
        for the test. They can use this TestCase instance to execute assert
        methods, among other things.

        .. code-block :: python

           @it.should('do this')
           def dothis(case):
               # ....

           @it.should
           def dothat():
               "do that also"
               # ....

        """
        def decorator(f):
            _desc = desc if isinstance(desc, six.string_types) else f.__doc__
            case = Case(self._group, f, "should %s" % _desc)
            self._group.addCase(case)
            return case
        if isinstance(desc, type(decorator)):
            return decorator(desc)
        return decorator

    def __getattr__(self, attr):
        return getattr(self._helper, attr)

    def createTests(self, mod):
        """Generate test cases for this scenario.

        .. warning ::

           You must call this, passing in ``globals()``, to
           generate tests from the scenario. If you don't
           call ``createTests``, **no tests will be
           created**.

        .. code-block :: python

           it.createTests(globals())

        """
        self._checkForLayersPlugin()
        self._makeGroupTest(mod, self._group)

    def _checkForLayersPlugin(self):
        currentSession = PluggableTestProgram.getCurrentSession()
        if not currentSession:
            return
        if not currentSession.isPluginLoaded('nose2.plugins.layers'):
            sys.stderr.write(LAYERS_PLUGIN_NOT_LOADED_MESSAGE)

    def _makeGroupTest(self, mod, group, parent_layer=None, position=0):
        layer = self._makeLayer(group, parent_layer, position)
        case = self._makeTestCase(group, layer, parent_layer)
        log.debug(
            "Made test case %s with layer %s from %s", case, layer, group)
        mod[layer.__name__] = layer
        layer.__module__ = mod['__name__']
        name = case.__name__
        long_name = ' '.join(
            [n[0].description for n in util.ancestry(layer)] + [name])
        mod[long_name] = case
        if name not in mod:
            mod[name] = case
        case.__module__ = mod['__name__']
        for index, child in enumerate(group._children):
            self._makeGroupTest(mod, child, layer, index)

    def _makeTestCase(self, group, layer, parent_layer):
        attr = {
            'layer': layer,
            'group': group,
            'description': group.description,
        }

        def _make_test_func(case):
            '''
            Needs to be outside of the for-loop scope so that "case" is properly registered as a closure
            '''
            def _test(s, *args):
                case(s, *args)
            return _test
            
        for index, case in enumerate(group._cases):
            name = 'test %04d: %s' % (index, case.description)
            _test = _make_test_func(case)
            _test.__name__ = name
            _test.description = case.description
            _test.case = case
            _test.index = index
            if hasattr(case.func, 'paramList'):
                _test.paramList = case.func.paramList
            attr[name] = _test  # for collection and sorting
            attr[case.description] = _test  # for random access by name

        setups = getattr(parent_layer, 'testSetups', []) + group._test_setups
        if setups:
            def setUp(self):
                for func in setups:
                    args, _, _, _ = inspect.getargspec(func)
                    if args:
                        func(self)
                    else:
                        func()
            attr['setUp'] = setUp
        teardowns = getattr(parent_layer, 'testTeardowns', []) + group._test_teardowns[:]
        if teardowns:
            def tearDown(self):
                for func in teardowns:
                    args, _, _, _ = inspect.getargspec(func)
                    if args:
                        func(self)
                    else:
                        func()
            attr['tearDown'] = tearDown

        def methodDescription(self):
            return getattr(self, self._testMethodName).description
        attr['methodDescription'] = methodDescription

        return type(group.description, (unittest.TestCase,), attr)

    def _makeLayer(self, group, parent_layer=None, position=0):
        if parent_layer is None:
            parent_layer = object

        def setUp(cls):
            for func in cls.setups:
                args, _, _, _ = inspect.getargspec(func)
                if args:
                    func(self)
                else:
                    func()

        def tearDown(cls):
            for func in cls.teardowns:
                args, _, _, _ = inspect.getargspec(func)
                if args:
                    func(self)
                else:
                    func()

        attr = {
            'description': group.description,
            'setUp': classmethod(setUp),
            'tearDown': classmethod(tearDown),
            'setups': group._setups[:],
            'testSetups': getattr(parent_layer, 'testSetups', []) + group._test_setups,
            'teardowns': group._teardowns[:],
            'testTeardowns': getattr(parent_layer, 'testTeardowns', []) + group._test_teardowns[:],
            'position': position,
            'mixins': ()
        }

        if group.base_layer:
            # inject this layer into the group class list
            # by making it a subclass of parent_layer
            layer = group.base_layer
            if parent_layer not in layer.__bases__:
                layer.mixins = (parent_layer,)
        else:
            layer = type("%s:layer" % group.description, (parent_layer,), attr)
        if group.mixins:
            layer.mixins = getattr(layer, 'mixins', ()) + tuple(group.mixins)
        log.debug("made layer %s with bases %s and mixins %s",
                  layer, layer.__bases__, layer.mixins)
        return layer


class Group(object):

    """Group of tests w/common fixtures & description"""

    def __init__(self, description, indent=0, parent=None, base_layer=None):
        self.description = description
        self.indent = indent
        self.parent = parent
        self.base_layer = base_layer
        self.mixins = []
        self._cases = []
        self._setups = []
        self._teardowns = []
        self._test_setups = []
        self._test_teardowns = []
        self._children = []

    def addCase(self, case):
        if not self._cases:
            case.first = True
        case.indent = self.indent
        self._cases.append(case)

    def addSetup(self, func):
        self._setups.append(func)

    def addTeardown(self, func):
        self._teardowns.append(func)

    def addTestSetUp(self, func):
        self._test_setups.append(func)

    def addTestTearDown(self, func):
        self._test_teardowns.append(func)

    def fullDescription(self):
        d = []
        p = self.parent
        while p:
            d.insert(0, p.description)
            p = p.parent
        d.append(self.description)
        return ' '.join(d)

    def child(self, description, base_layer=None):
        child = Group(description, self.indent + 1, self, base_layer)
        self._children.append(child)
        return child


class Case(object):

    """Information about a test case"""
    _helper = helper

    def __init__(self, group, func, description):
        self.group = group
        self.func = func
        self.description = description
        self._setups = []
        self._teardowns = []
        self.first = False
        self.full = False

    def __call__(self, testcase, *args):
        # ... only if it takes an arg
        self._helper = testcase
        funcargs, _, _, _ = inspect.getargspec(self.func)
        if funcargs:
            self.func(testcase, *args)
        else:
            self.func()

    def __getattr__(self, attr):
        return getattr(self._helper, attr)
