# Adapted from unittest2/events.py from the unittest2 plugins branch.
# This module contains some code copied from unittest2/events.py and other
# code developed in reference to that module and others within unittest2.
# unittest2 is Copyright (c) 2001-2010 Python Software Foundation; All
# Rights Reserved. See: http://docs.python.org/license.html

import logging

import argparse
import six

from nose2 import config, util

log = logging.getLogger(__name__)
__unittest = True

# FIXME decide on a real rule for camelCase vs under_score and stick with it.

# XXX I'd rather move this stuff to Plugin.__init__ and
# have __init__ call self.configure() or something after the
# initial setup, but that would further break compatibilty
# with the unittest2 plugins branch Plugin class.


class PluginMeta(type):

    def __call__(cls, *args, **kwargs):
        session = kwargs.pop('session', None)
        instance = object.__new__(cls, *args, **kwargs)
        instance.session = session
        instance.config = config.Config([])

        config_section = getattr(instance, 'configSection', None)
        switch = getattr(instance, 'commandLineSwitch', None)

        if session is not None and config_section is not None:
            instance.config = session.get(config_section)

        always_on = instance.config.as_bool(
            'always-on', default=instance.alwaysOn)

        instance.__init__(*args, **kwargs)
        if always_on:
            instance.register()
        else:
            if switch is not None:
                short_opt, long_opt, help = switch
                instance.addOption(
                    instance._register_cb, short_opt, long_opt, help)
        return instance


class Plugin(six.with_metaclass(PluginMeta)):

    """Base class for nose2 plugins

    All nose2 plugins must subclass this class.

    .. attribute :: session

       The :class:`nose2.session.Session` under which the plugin
       has been loaded.

    .. attribute :: config

       The :class:`nose2.config.Config` representing the plugin's
       config section as loaded from the session's config files.

    .. attribute :: commandLineSwitch

       A tuple of (short opt, long opt, help text) that defines a command
       line flag that activates this plugin. The short opt may be None. If
       defined, it must be a single upper-case character. Both short and
       long opt must *not* start with dashes.

       Example::

         commandLineSwitch = ('B', 'buffer-output', 'Buffer output during tests')

    .. attribute :: configSection

       The name config file section to load into this plugin's config.

    .. attribute :: alwaysOn

       If this plugin should automatically register itself, set alwaysOn to
       True. Default is False.

    .. note ::

       Plugins that use config values from config files and want to
       use the nose2 sphinx extension to automatically generate
       documentation *must* extract all config values from
       ``self.config`` in ``__init__``. Otherwise the extension will
       not be able to detect the config keys that the plugin uses.

    """
    alwaysOn = False
    registered = False

    def register(self):
        """Register with appropriate hooks.

        This activates the plugin and enables it to receive events.

        """
        if self.session is None:
            log.warning("Unable to register %s, no session", self)
            return
        self.session.registerPlugin(self)
        self.registered = True

    def addMethods(self, *methods):
        """Add new plugin methods to hooks registry

        Any plugins that are already registered and implement
        a method added here will be registered for that
        method as well.

        """
        for method in methods:
            self.session.hooks.addMethod(method)
        for plugin in self.session.plugins:
            for method in methods:
                if plugin.registered and hasattr(plugin, method):
                    self.session.hooks.register(method, plugin)

    def _register_cb(self, *_):
        self.register()

    def addFlag(self, callback, short_opt, long_opt, help_text=None):
        """Add command-line flag that takes no arguments

        :param callback: Callback function to run when flag is seen.
                         The callback will receive one empty argument.
        :param short_opt: Short option. Must be uppercase, no dashes.
        :param long_opt: Long option. Must not start with dashes
        :param help_text: Help text for users so they know what this
                          flag does.

        """
        self.addOption(callback, short_opt, long_opt, help_text, nargs=0)

    def addArgument(self, callback, short_opt, long_opt, help_text=None):
        """Add command-line option that takes one argument.

        :param callback: Callback function to run when flag is seen.
                         The callback will receive one argument.
        :param short_opt: Short option. Must be uppercase, no dashes.
        :param long_opt: Long option. Must not start with dashes
        :param help_text: Help text for users so they know what this
                          flag does.

        """
        self.addOption(callback, short_opt, long_opt, help_text, nargs=1)

    def addOption(self, callback, short_opt, long_opt, help_text=None, nargs=0):
        """Add command-line option.

        :param callback: Callback function to run when flag is seen.
                         The callback will receive one argument.
                         The "callback" may also be a list, in which
                         case values submitted on the command line
                         will be appended to the list.
        :param short_opt: Short option. Must be uppercase, no dashes.
        :param long_opt: Long option. Must not start with dashes
        :param help_text: Help text for users so they know what this
                          flag does.
        :param nargs: Number of arguments to consume from command line.

        """
        if self.session is None:
            log.warning("Unable to add option %s/%s for %s, no session",
                        short_opt, long_opt, self)
            return

        class CB(argparse.Action):

            def __call__(self, parser, namespace, values, option_string=None):
                if six.callable(callback):
                    callback(values)
                elif isinstance(callback, list):
                    callback.extend(values)
                else:
                    raise ValueError(
                        "Invalid callback %s for plugin option %s",
                        callback, option_string)
        opts = []
        if short_opt:
            if short_opt.lower() == short_opt:
                raise ValueError(
                    'Lowercase short options are reserved: %s' % short_opt)
            opts.append('-' + short_opt)
        if long_opt:
            opts.append('--' + long_opt)
        self.session.pluginargs.add_argument(
            *opts, action=CB, help=help_text, const=True, nargs=nargs)


class Hook(object):

    """A plugin hook

    Each plugin method in the :class:`nose2.events.PluginInterface` is
    represented at runtime by a Hook instance that lists the plugins
    that should be called by that hook.

    .. attribute :: method

       The name of the method that this Hook represents.

    .. attribute :: plugins

       The list of plugin instances bound to this hook.

    """

    def __init__(self, method):
        self.method = method
        self.plugins = []

    def __call__(self, event):
        for plugin in self.plugins[:]:
            result = getattr(plugin, self.method)(event)
            if event.handled:
                return result

    def append(self, plugin):
        if plugin not in self.plugins:
            self.plugins.append(plugin)


class PluginInterface(object):

    """Definition of plugin interface.

    Instances of this class contain the methods that may be called,
    and a dictionary of :class:`nose2.events.Hook` instances bound
    to each method.

    In a plugin, PluginInterface instance is typically available as
    self.session.hooks, and plugin hooks may be called on it
    directly::

      event = events.LoadFromModuleEvent(module=the_module)
      self.session.hooks.loadTestsFromModule(event)

    .. attribute :: preRegistrationMethods

       Tuple of methods that are called before registration.

    .. attribute :: methods

       Tuple of available plugin hook methods.

    .. attribute :: hookClass

       Class to instantiate for each hook. Default: :class:`nose2.events.Hook`.

    """
    preRegistrationMethods = ('pluginsLoaded', 'handleArgs')
    methods = (
        'loadTestsFromModule', 'loadTestsFromNames',
        'handleFile', 'startTestRun', 'startTest', 'stopTest',
        'loadTestsFromName', 'loadTestsFromTestCase',
        'stopTestRun', 'matchPath', 'matchDirPath', 'getTestCaseNames',
        'runnerCreated', 'resultCreated', 'testOutcome', 'wasSuccessful',
        'resultStop', 'setTestOutcome', 'describeTest',
        'reportStartTest', 'reportError', 'reportFailure', 'reportSkip',
        'reportSuccess', 'reportExpectedFailure', 'reportUnexpectedSuccess',
        'reportOtherOutcome', 'outcomeDetail', 'beforeErrorList',
        'beforeSummaryReport', 'afterSummaryReport', 'beforeInteraction',
        'afterInteraction', 'createTests', 'afterTestRun',
        'moduleLoadedSuite', 'handleDir',
        # ... etc?
    )
    hookClass = Hook

    def __init__(self):
        self.hooks = {}

    def addMethod(self, method):
        """Add a method to the available method.

        This allows plugins to register for this method.

        :param method: A method name

        """
        self.methods = self.methods + (method,)

    def register(self, method, plugin):
        """Register a plugin for a method.

        :param method: A method name
        :param plugin: A plugin instance

        """
        self.hooks.setdefault(method, self.hookClass(method)).append(plugin)

    def __getattr__(self, attr):
        return self.hooks.setdefault(attr, self.hookClass(attr))


class Event(object):

    """Base class for all events.

    .. attribute :: metadata

       Storage for arbitrary information attached to an event.

    .. attribute :: handled

       Set to True to indicate that a plugin has handled the event,
       and no other plugins or core systems should process it further.

    .. attribute :: version

       Version of the event API. This will be incremented with each
       release of nose2 that changes the API.

    """
    _attrs = ('handled',)
    version = '0.4'

    def __init__(self, **metadata):
        self.handled = False
        self.metadata = {}
        self.metadata.update(metadata)

    def __str__(self):
        return '%s(%s)' % (self.__class__.__name__, self._format())

    def __repr__(self):
        return str(self)

    def _format(self):
        return ', '.join(['%s=%r' % (k, getattr(self, k, None))
                          for k in self._attrs])

    def __getstate__(self):
        state = self.__dict__
        # FIXME fails for loadTestsFailure
        if 'test' in state:
            state['test'] = util.test_name(state['test'])
        if 'executeTests' in state:
            state['executeTests'] = None
        if 'exc_info' in state and state['exc_info'] is not None:
            ec, ev, tb = state['exc_info']
            state['exc_info'] = (
                ec, ev, util.format_traceback(None, (ec, ev, tb)))
        clear = ('loader', 'result', 'runner')
        for attr in clear:
            if attr in state:
                state[attr] = None
        return state


class PluginsLoadedEvent(Event):

    """Event fired after all plugin classes are loaded.

    .. attribute :: pluginsLoaded

       List of all loaded plugin classes

    """
    _attrs = Event._attrs + ('pluginsLoaded',)

    def __init__(self, pluginsLoaded, **kw):
        self.pluginsLoaded = pluginsLoaded
        super(PluginsLoadedEvent, self).__init__(**kw)


class RunnerCreatedEvent(Event):

    """Event fired when test runner is created.

    .. attribute :: runner

       Test runner instance. Plugins may replace the test runner by
       setting this attribute to a new test runner instance.

    """
    _attrs = Event._attrs + ('runner',)

    def __init__(self, runner, **kw):
        self.runner = runner
        super(RunnerCreatedEvent, self).__init__(**kw)


class ResultCreatedEvent(Event):

    """Event fired when test result handler is created.

    .. attribute :: result

       Test result handler instance. Plugins may replace the test
       result by setting this attribute to a new test result instance.

    """
    _attrs = Event._attrs + ('result',)

    def __init__(self, result, **kw):
        self.result = result
        super(ResultCreatedEvent, self).__init__(**kw)


class StartTestRunEvent(Event):

    """Event fired when test run is about to start.

    Test collection is complete before this event fires, but
    no tests have yet been executed.

    .. attribute :: runner

       Test runner

    .. attribute :: suite

       Top-level test suite to execute. Plugins can filter this suite,
       or set event.suite to change which tests execute (or how they
       execute).

    .. attribute :: result

       Test result

    .. attribute :: startTime

       Timestamp of test run start

    .. attribute :: executeTests

       Callable that will be used to execute tests.  Plugins may set
       this attribute to wrap or otherwise change test execution. The
       callable must match the signature::

         def execute(suite, result):
             ...

    To prevent normal test execution, plugins may set ``handled`` on
    this event to True. When ``handled`` is true, the test executor
    does not run at all.

    """
    _attrs = Event._attrs + ('runner', 'suite', 'result', 'startTime',
                             'executeTests')

    def __init__(self, runner, suite, result, startTime, executeTests, **kw):
        self.suite = suite
        self.runner = runner
        self.result = result
        self.startTime = startTime
        self.executeTests = executeTests
        super(StartTestRunEvent, self).__init__(**kw)


class StopTestRunEvent(Event):

    """Event fired when test run has stopped.

    .. attribute :: runner

       Test runner

    .. attribute :: result

       Test result

    .. attribute :: stopTime

       Timestamp of test run stop

    .. attribute :: timeTaken

       Number of seconds test run took to execute

    """
    _attrs = Event._attrs + ('runner', 'result', 'stopTime', 'timeTaken')

    def __init__(self, runner, result, stopTime, timeTaken, **kw):
        self.runner = runner
        self.result = result
        self.stopTime = stopTime
        self.timeTaken = timeTaken
        super(StopTestRunEvent, self).__init__(**kw)


class StartTestEvent(Event):

    """Event fired before a test is executed.

    .. attribute :: test

       The test case

    .. attribute :: result

       Test result

    .. attribute :: startTime

       Timestamp of test start

    """
    _attrs = Event._attrs + ('test', 'result', 'startTime')

    def __init__(self, test, result, startTime, **kw):
        self.test = test
        self.result = result
        self.startTime = startTime
        super(StartTestEvent, self).__init__(**kw)


class StopTestEvent(Event):

    """Event fired after a test is executed.

    .. attribute :: test

       The test case

    .. attribute :: result

       Test result

    .. attribute :: stopTime

       Timestamp of test stop

    """
    _attrs = Event._attrs + ('test', 'result', 'stopTime')

    def __init__(self, test, result, stopTime, **kw):
        self.test = test
        self.result = result
        self.stopTime = stopTime
        super(StopTestEvent, self).__init__(**kw)


class TestOutcomeEvent(Event):

    """Event fired when a test completes.

    .. attribute :: test

       The test case

    .. attribute :: result

       Test result

    .. attribute :: outcome

       Description of test outcome. Typically will be one of 'error',
       'failed', 'skipped', or 'passed'.

    .. attribute :: exc_info

       If the test resulted in an exception, the tuple of (exception
       class, exception value, traceback) as returned by
       sys.exc_info(). If the test did not result in an exception,
       None.

    .. attribute :: reason

       For test outcomes that include a reason (Skips, for example),
       the reason.

    .. attribute :: expected

       Boolean indicating whether the test outcome was expected. In
       general, all tests are expected to pass, and any other outcome
       will have expected as False. The exceptions to that rule are
       unexpected successes and expected failures.

    .. attribute :: shortLabel

       A short label describing the test outcome. (For example, 'E'
       for errors).

    .. attribute :: longLabel

       A long label describing the test outcome (for example, 'ERROR'
       for errors).

    Plugins may influence how the rest of the system sees the test
    outcome by setting ``outcome`` or ``exc_info`` or ``expected``. They
    may influence how the test outcome is reported to the user by
    setting ``shortLabel`` or ``longLabel``.

    """
    _attrs = Event._attrs + ('test', 'result', 'outcome', 'exc_info', 'reason',
                             'expected', 'shortLabel', 'longLabel')

    def __init__(self, test, result, outcome, exc_info=None, reason=None,
                 expected=False, shortLabel=None, longLabel=None, **kw):
        self.test = test
        self.result = result
        self.outcome = outcome
        self.exc_info = exc_info
        self.reason = reason
        self.expected = expected
        self.shortLabel = shortLabel
        self.longLabel = longLabel
        super(TestOutcomeEvent, self).__init__(**kw)


class LoadFromModuleEvent(Event):

    """Event fired when a test module is loaded.

    .. attribute :: loader

       Test loader instance

    .. attribute :: module

       The module whose tests are to be loaded

    .. attribute :: extraTests

       A list of extra tests loaded from the module. To load tests
       from a module without interfering with other plugins' loading
       activities, append tests to extraTests.

    Plugins may set ``handled`` on this event and return a test suite
    to prevent other plugins from loading tests from the module. If
    any plugin sets ``handled`` to True, ``extraTests`` will be
    ignored.

    """
    _attrs = Event._attrs + ('loader', 'module', 'extraTests')

    def __init__(self, loader, module, **kw):
        self.loader = loader
        self.module = module
        self.extraTests = []
        super(LoadFromModuleEvent, self).__init__(**kw)


class ModuleSuiteEvent(Event):
    _attrs = Event._attrs + ('loader', 'module', 'suite')

    def __init__(self, loader, module, suite, **kw):
        self.loader = loader
        self.module = module
        self.suite = suite
        super(ModuleSuiteEvent, self).__init__(**kw)


class LoadFromTestCaseEvent(Event):

    """Event fired when tests are loaded from a test case.

    .. attribute :: loader

       Test loader instance

    .. attribute :: testCase

       The :class:`unittest.TestCase` instance being loaded.

    .. attribute :: extraTests

        A list of extra tests loaded from the module. To load tests
        from a test case without interfering with other plugins'
        loading activities, append tests to extraTests.

    Plugins may set ``handled`` on this event and return a test suite
    to prevent other plugins from loading tests from the test case. If
    any plugin sets ``handled`` to True, ``extraTests`` will be
    ignored.

    """
    _attrs = Event._attrs + ('loader', 'testCase', 'extraTests')

    def __init__(self, loader, testCase, **kw):
        self.loader = loader
        self.testCase = testCase
        self.extraTests = []
        super(LoadFromTestCaseEvent, self).__init__(**kw)


class LoadFromNamesEvent(Event):

    """Event fired to load tests from test names.

    .. attribute :: loader

       Test loader instance

    .. attribute :: names

       List of test names. May be empty or None.

    .. attribute :: module

       Module to load from. May be None. If not None, names should be
       considered relative to this module.

    .. attribute :: extraTests

        A list of extra tests loaded from the tests named. To load
        tests from test names without interfering with other plugins'
        loading activities, append tests to extraTests.

    Plugins may set ``handled`` on this event and return a test suite
    to prevent other plugins from loading tests from the test names. If
    any plugin sets ``handled`` to True, ``extraTests`` will be
    ignored.

    """
    _attrs = Event._attrs + ('loader', 'names', 'module', 'extraTests')

    def __init__(self, loader, names, module, **kw):
        self.loader = loader
        self.names = names
        self.module = module
        self.extraTests = []
        super(LoadFromNamesEvent, self).__init__(**kw)

    def __str__(self):
        return "LoadFromNames(names=%r, module=%r)" % (self.names, self.module)


class LoadFromNameEvent(Event):

    """Event fired to load tests from test names.

    .. attribute :: loader

       Test loader instance

    .. attribute :: name

       Test name to load

    .. attribute :: module

       Module to load from. May be None. If not None, names should be
       considered relative to this module.

    .. attribute :: extraTests

        A list of extra tests loaded from the name. To load tests
        from a test name without interfering with other plugins'
        loading activities, append tests to extraTests.

    Plugins may set ``handled`` on this event and return a test suite
    to prevent other plugins from loading tests from the test name. If
    any plugin sets ``handled`` to True, ``extraTests`` will be
    ignored.

    """
    _attrs = Event._attrs + ('loader', 'name', 'module', 'extraTests')

    def __init__(self, loader, name, module, **kw):
        self.loader = loader
        self.name = name
        self.module = module
        self.extraTests = []
        super(LoadFromNameEvent, self).__init__(**kw)


class HandleFileEvent(Event):

    """Event fired when a non-test file is examined.

    .. note ::

       This event is fired for all processed python files and modules
       including but not limited to the ones that
       match the test file pattern.

    .. attribute :: loader

       Test loader instance

    .. attribute :: name

       File basename

    .. attribute :: path

       Full path to file

    .. attribute :: pattern

       Current test file match pattern

    .. attribute :: topLevelDirectory

       Top-level directory of the test run

    .. attribute :: extraTests

        A list of extra tests loaded from the file. To load tests
        from a file without interfering with other plugins'
        loading activities, append tests to extraTests.

    Plugins may set ``handled`` on this event and return a test suite
    to prevent other plugins from loading tests from the file. If
    any plugin sets ``handled`` to True, ``extraTests`` will be
    ignored.

    """
    _attrs = Event._attrs + ('loader', 'name', 'path', 'pattern',
                             'topLevelDirectory')

    def __init__(self, loader, name, path, pattern, topLevelDirectory, **kw):
        self.extraTests = []
        self.path = path
        self.loader = loader
        self.name = name
        # note: pattern may be None if not called during test discovery
        self.pattern = pattern
        self.topLevelDirectory = topLevelDirectory
        super(HandleFileEvent, self).__init__(**kw)


class MatchPathEvent(Event):

    """Event fired during file matching.

    Plugins may return False and set ``handled`` on this event to prevent
    a file from being matched as a test file, regardless of other system
    settings.

    .. attribute :: path

       Full path to the file

    .. attribute :: name

       File basename

    .. attribute :: pattern

       Current test file match pattern

    """
    _attrs = Event._attrs + ('name', 'path', 'pattern')

    def __init__(self, name, path, pattern, **kw):
        self.path = path
        self.name = name
        self.pattern = pattern
        super(MatchPathEvent, self).__init__(**kw)


class GetTestCaseNamesEvent(Event):

    """Event fired to find test case names in a test case.

    Plugins may return a list of names and set ``handled`` on this
    event to force test case name selection.


    .. attribute :: loader

       Test loader instance

    .. attribute :: testCase

       The :class:`unittest.TestCase` instance being loaded.

    .. attribute :: testMethodPrefix

       Set this to change the test method prefix. Unless set by a plugin,
       it is None.

    .. attribute :: extraNames

       A list of extra test names to load from the test case. To cause
       extra tests to be loaded from the test case, append the names
       to this list. Note that the names here must be attributes of
       the test case.

    .. attribute :: excludedNames

       A list of names to exclude from test loading. Add names to this
       list to prevent other plugins from loading the named tests.

    .. attribute :: isTestMethod

       Callable that plugins can use to examine test case attributes to
       determine whether nose2 thinks they are test methods.

    """
    _attrs = Event._attrs + ('loader', 'testCase', 'testMethodPrefix',
                             'extraNames', 'excludedNames', 'isTestMethod')

    def __init__(self, loader, testCase, isTestMethod, **kw):
        self.loader = loader
        self.testCase = testCase
        self.testMethodPrefix = None
        self.extraNames = []
        self.excludedNames = []
        self.isTestMethod = isTestMethod
        super(GetTestCaseNamesEvent, self).__init__(**kw)


class ResultSuccessEvent(Event):

    """Event fired at end of test run to determine success.

    This event fires at the end of the test run and allows
    plugins to determine whether the test run was successful.

    .. attribute :: result

       Test result

    .. attribute :: success

       Set this to True to indicate that the test run was
       successful. If no plugin sets the ``success`` to
       True, the test run fails.

    """
    _attrs = Event._attrs + ('result', 'success')

    def __init__(self, result, success, **kw):
        self.result = result
        self.success = success
        super(ResultSuccessEvent, self).__init__(**kw)


class ResultStopEvent(Event):

    """Event fired when a test run is told to stop.

    Plugins can use this event to prevent other plugins from stopping
    a test run.

    .. attribute :: result

       Test result

    .. attribute :: shouldStop

       Set to True to indicate that the test run should stop.

    """
    _attrs = Event._attrs + ('result', 'shouldStop')

    def __init__(self, result, shouldStop, **kw):
        self.result = result
        self.shouldStop = shouldStop
        super(ResultStopEvent, self).__init__(**kw)


class DescribeTestEvent(Event):

    """Event fired to get test description.


    .. attribute :: test

       The test case

    .. attribute :: description

       Description of the test case. Plugins can set this to change
       how tests are described in output to users.

    .. attribute :: errorList

       Is the event fired as part of error list output?

    """
    _attrs = Event._attrs + ('test', 'description')

    def __init__(self, test, description=None, errorList=False, **kw):
        self.test = test
        self.description = description
        self.errorList = errorList
        super(DescribeTestEvent, self).__init__(**kw)


class OutcomeDetailEvent(Event):

    """Event fired to acquire additional details about test outcome.

    .. attribute :: outcomeEvent

       A :class:`nose2.events.TestOutcomeEvent` instance holding
       the test outcome to be described.

    .. attribute :: extraDetail

       Extra detail lines to be appended to test outcome
       output. Plugins can append lines (of strings) to this list to
       include their extra information in the error list report.

    """
    _attrs = Event._attrs + ('outcomeEvent', 'extraDetail')

    def __init__(self, outcomeEvent, **kw):
        self.outcomeEvent = outcomeEvent
        self.extraDetail = []
        super(OutcomeDetailEvent, self).__init__(**kw)


class ReportSummaryEvent(Event):

    """Event fired before and after summary report.

    .. attribute :: stopTestEvent

       A :class:`nose2.events.StopTestEvent` instance.

    .. attribute :: stream

       The output stream. Plugins can set this to change or capture
       output.

    .. attribute :: reportCategories

       Dictionary of report category and test events captured in that
       category. Default categories include 'errors', 'failures',
       'skipped', 'expectedFails', and 'unexpectedSuccesses'. Plugins
       may add their own categories.

    """
    _attrs = Event._attrs + ('stopTestEvent', 'stream', 'reportCategories')

    def __init__(self, stopTestEvent, stream, reportCategories, **kw):
        self.stopTestEvent = stopTestEvent
        self.stream = stream
        self.reportCategories = reportCategories
        super(ReportSummaryEvent, self).__init__(**kw)


class ReportTestEvent(Event):

    """Event fired to report a test event.

    Plugins can respond to this event by producing output for the user.

    .. attribute :: testEvent

       A test event. In most cases, a
       :class:`nose2.events.TestOutcomeEvent` instance. For startTest,
       a :class:`nose2.events.StartTestEvent` instance.

    .. attribute :: stream

       The output stream. Plugins can set this to change or capture
       output.

    """
    _attrs = Event._attrs + ('testEvent', 'stream')

    def __init__(self, testEvent, stream, **kw):
        self.testEvent = testEvent
        self.stream = stream
        super(ReportTestEvent, self).__init__(**kw)


class UserInteractionEvent(Event):

    """Event fired before and after user interaction.

    Plugins that capture stdout or otherwise prevent user interaction
    should respond to this event.

    To prevent the user interaction from occurring, return False and
    set ``handled``. Otherwise, turn off whatever you are doing that
    prevents users from typing/clicking/touching/psionics/whatever.

    """

    def __init__(self, **kw):
        super(UserInteractionEvent, self).__init__(**kw)


class CommandLineArgsEvent(Event):

    """Event fired after parsing of command line arguments.

    Plugins can respond to this event by configuring themselves or other plugins
    or modifying the parsed arguments.

    .. note ::

       Many plugins register options with callbacks. By the time this event
       fires, those callbacks have already fired. So you can't use this
       event to reliably influence all plugins.

    .. attribute :: args

       Args object returned by argparse.

    """
    _attrs = Event._attrs + ('args',)

    def __init__(self, args, **kw):
        self.args = args
        super(CommandLineArgsEvent, self).__init__(**kw)


class CreateTestsEvent(Event):

    """Event fired before test loading.

    Plugins can take over test loading by returning a test suite and setting
    ``handled`` on this event.

    .. attribute :: loader

       Test loader instance

    .. attribute :: names

       List of test names. May be empty or None.

    .. attribute :: module

       Module to load from. May be None. If not None, names should be
       considered relative to this module.

    """
    _attrs = Event._attrs + ('loader', 'testNames', 'module')

    def __init__(self, loader, testNames, module, **kw):
        self.loader = loader
        self.testNames = testNames
        self.module = module
        super(CreateTestsEvent, self).__init__(**kw)
