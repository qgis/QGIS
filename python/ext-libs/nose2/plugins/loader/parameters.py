"""
Load tests from parameterized functions and methods.

This plugin implements :func:`getTestCaseNames`,
:func:`loadTestsFromModule`, and :func:`loadTestsFromName` to support
loading tests from parameterized test functions and methods.

To parameterize a function or test case method, use :func:`nose2.tools.params`.

To address a particular parameterized test via a command-line test name,
append a colon (':') followed by the index, *starting from 1*, of the
case you want to execute.

Such And The Parameters Plugin
------------------------------

The parameters plugin can work with the Such DSL, as long as the first argument
of the test function is the "case" argument, followed by the other parameters::

    from nose2.tools import such
    from nose2.tools.params import params
    
    with such.A('foo') as it:
        @it.should('do bar')
        @params(1,2,3)
        def test(case, bar):
            case.assert_(isinstance(bar, int))
    
        @it.should('do bar and extra')
        @params((1, 2), (3, 4) ,(5, 6))
        def testExtraArg(case, bar, foo):
            case.assert_(isinstance(bar, int))
            case.assert_(isinstance(foo, int))
    
    it.createTests(globals())

"""
# This module contains some code copied from unittest2 and other code
# developed in reference to unittest2.
# unittest2 is Copyright (c) 2001-2010 Python Software Foundation; All
# Rights Reserved. See: http://docs.python.org/license.html

import functools
import logging
import types
import unittest

from nose2 import exceptions, util
from nose2.events import Plugin
from nose2.compat import unittest as ut2
from nose2.plugins.loader.testclasses import MethodTestCase


log = logging.getLogger(__name__)
__unittest = True


class ParamsFunctionCase(ut2.FunctionTestCase):

    def __init__(self, name, func, **args):
        self._funcName = name
        ut2.FunctionTestCase.__init__(self, func, **args)

    def __repr__(self):
        return self._funcName

    id = __str__ = __repr__


class Parameters(Plugin):

    """Loader plugin that loads parameterized tests"""
    alwaysOn = True
    configSection = 'parameters'

    def registerInSubprocess(self, event):
        event.pluginClasses.append(self.__class__)

    def getTestCaseNames(self, event):
        """Generate test case names for all parameterized methods"""
        log.debug('getTestCaseNames %s', event)
        names = filter(event.isTestMethod, dir(event.testCase))
        testCaseClass = event.testCase
        for name in names:
            method = getattr(testCaseClass, name)
            paramList = getattr(method, 'paramList', None)
            if paramList is None:
                continue
            # exclude this method from normal collection
            event.excludedNames.append(name)
            # generate the methods to be loaded by the testcase loader
            self._generate(event, name, method, testCaseClass)

    def getTestMethodNames(self, event):
        return self.getTestCaseNames(event)

    def loadTestsFromModule(self, event):
        """Load tests from parameterized test functions in the module"""
        module = event.module

        def is_test(obj):
            return (obj.__name__.startswith(self.session.testMethodPrefix) and
                    hasattr(obj, 'paramList'))
        tests = []
        for name in dir(module):
            obj = getattr(module, name)
            if isinstance(obj, types.FunctionType) and is_test(obj):
                tests.extend(
                    self._generateFuncTests(obj)
                )
        event.extraTests.extend(tests)

    def loadTestsFromName(self, event):
        """Load parameterized test named on command line"""
        original_name = name = event.name
        module = event.module
        try:
            result = util.test_from_name(name, module)
        except (AttributeError, ImportError) as e:
            event.handled = True
            return event.loader.failedLoadTests(name, e)
        if result is None:
            # we can't find it - let the default case handle it
            return

        parent, obj, fqname, index = result
        if not hasattr(obj, 'paramList'):
            return

        if (index is None and not
            isinstance(parent, type) and not
            isinstance(obj, types.FunctionType)):
            log.debug(
                "Don't know how to load parameterized tests from %s", obj)
            return

        if (parent and
            isinstance(parent, type) and
            issubclass(parent, unittest.TestCase)):
            # generator method
            names = self._generate(event, obj.__name__, obj, parent)
            tests = [parent(n) for n in names]
        elif (parent and
              isinstance(parent, type)):
            names = self._generate(event, name, obj, parent)
            tests = [MethodTestCase(parent)(name) for name in names]
        else:
            # generator func
            tests = list(self._generateFuncTests(obj))

        if index is not None:
            try:
                tests = [tests[index - 1]]
            except IndexError:
                raise exceptions.TestNotFoundError(original_name)

        suite = event.loader.suiteClass()
        suite.addTests(tests)
        event.handled = True
        return suite

    def _generate(self, event, name, method, testCaseClass):
        names = []
        for index, argSet in enumerate_params(method.paramList):
            method_name = util.name_from_args(name, index, argSet)
            if not hasattr(testCaseClass, method_name):
                # not already generated
                def _method(self, method=method, argSet=argSet):
                    return method(self, *argSet)
                _method = functools.update_wrapper(_method, method)
                delattr(_method, 'paramList')
                setattr(testCaseClass, method_name, _method)
            names.append(method_name)
        return names

    def _generateFuncTests(self, obj):
        args = {}
        setUp = getattr(obj, 'setUp', None)
        tearDown = getattr(obj, 'tearDown', None)
        if setUp is not None:
            args['setUp'] = setUp
        if tearDown is not None:
            args['tearDown'] = tearDown
        for index, argSet in enumerate_params(obj.paramList):
            def func(argSet=argSet, obj=obj):
                return obj(*argSet)
            func = functools.update_wrapper(func, obj)
            delattr(func, 'paramList')
            name = '%s.%s' % (obj.__module__, obj.__name__)
            func_name = util.name_from_args(name, index, argSet)
            yield util.transplant_class(
                ParamsFunctionCase, obj.__module__)(func_name, func, **args)


def enumerate_params(paramList):
    for index, argSet in enumerate(paramList):
        if not isinstance(argSet, tuple):
            argSet = (argSet,)
        yield index, argSet
