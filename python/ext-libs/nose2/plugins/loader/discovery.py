"""
Discovery-based test loader.

This plugin implements nose2's automatic test module discovery. It
looks for test modules in packages and directories whose names start
with 'test', then fires the :func:`loadTestsFromModule` hook for each
one to allow other plugins to load the actual tests.

It also fires :func:`handleFile` for every file that it sees, and
:func:`matchPath` for every python module, to allow other plugins to
load tests from other kinds of files and to influence which modules
are examined for tests.

"""


# Adapted from unittest2/loader.py from the unittest2 plugins branch.
# This module contains some code copied from unittest2/loader.py and other
# code developed in reference to that module and others within unittest2.
# unittest2 is Copyright (c) 2001-2010 Python Software Foundation; All
# Rights Reserved. See: http://docs.python.org/license.html

from fnmatch import fnmatch
import logging
import os
import sys

from nose2 import events, util

__unittest = True
log = logging.getLogger(__name__)


class DirectoryHandler(object):
    def __init__(self, session):
        self.session = session
        self.event_handled = False

    def handle_dir(self, event, full_path, top_level):
        dirname = os.path.basename(full_path)
        pattern = self.session.testFilePattern

        evt = events.HandleFileEvent(
            event.loader, dirname, full_path, pattern, top_level)
        result = self.session.hooks.handleDir(evt)
        if evt.extraTests:
            for test in evt.extraTests:
                yield test
        if evt.handled:
            if result:
                yield result
            self.event_handled = True
            return
        
        evt = events.MatchPathEvent(dirname, full_path, pattern)
        result = self.session.hooks.matchDirPath(evt)
        if evt.handled and not result:
            self.event_handled = True


class Discoverer(object):
    
    def loadTestsFromName(self, event):
        """Load tests from module named by event.name"""
        # turn name into path or module name
        # fire appropriate hooks (handle file or load from module)
        if event.module:
            return
        name = event.name
        module = None
        _, top_level_dir = self._getStartDirs()
        try:
            # try name as a dotted module name first
            __import__(name)
            module = sys.modules[name]
        except ImportError:
            # if that fails, try it as a file or directory
            event.extraTests.extend(
                self._find_tests(event, name, top_level_dir))
        else:
            event.extraTests.extend(
                self._find_tests_in_module(event, module, top_level_dir))

    def loadTestsFromNames(self, event):
        """Discover tests if no test names specified"""
        log.debug("Received event %s", event)
        if event.names or event.module:
            return
        event.handled = True  # I will handle discovery
        return self._discover(event)

    def _checkIfPathIsOK(self, start_dir):
        if not os.path.isdir(os.path.abspath(start_dir)):
            raise OSError("%s is not a directory" % os.path.abspath(start_dir))

    def _getStartDirs(self):
        start_dir = self.session.startDir
        top_level_dir = self.session.topLevelDir
        if start_dir is None:
            start_dir = '.'
        if top_level_dir is None:
            top_level_dir = start_dir

        self._checkIfPathIsOK(start_dir)

        is_not_importable = False
        start_dir = os.path.abspath(start_dir)
        top_level_dir = os.path.abspath(top_level_dir)
        if start_dir != top_level_dir:
            is_not_importable = not os.path.isfile(
                os.path.join(start_dir, '__init__.py'))
        if is_not_importable:
            raise ImportError(
                'Start directory is not importable: %r' % start_dir)
        # this is redundant in some cases, but that's ok
        self.session.prepareSysPath()
        return start_dir, top_level_dir

    def _discover(self, event):
        loader = event.loader
        try:
            start_dir, top_level_dir = self._getStartDirs()
        except (OSError, ImportError):
            _, ev, _ = sys.exc_info()
            return loader.suiteClass(
                loader.failedLoadTests(self.session.startDir, ev))
        log.debug("_discover in %s (%s)", start_dir, top_level_dir)
        tests = list(self._find_tests(event, start_dir, top_level_dir))
        return loader.suiteClass(tests)

    def _find_tests(self, event, start, top_level):
        """Used by discovery. Yields test suites it loads."""
        log.debug('_find_tests(%r, %r)', start, top_level)
        if start == top_level:
            full_path = start
        else:
            full_path = os.path.join(top_level, start)
        if os.path.isdir(start):
            for test in self._find_tests_in_dir(
                event, full_path, top_level):
                yield test
        elif os.path.isfile(start):
            for test in self._find_tests_in_file(
                event, start, full_path, top_level):
                yield test
        
    def _find_tests_in_dir(self, event, full_path, top_level):
        if not os.path.isdir(full_path):
            return
        log.debug("find in dir %s (%s)", full_path, top_level)
        dir_handler = DirectoryHandler(self.session)
        for test in dir_handler.handle_dir(event, full_path, top_level):
            yield test
        if dir_handler.event_handled:
            return
        for path in os.listdir(full_path):
            entry_path = os.path.join(full_path, path)
            if os.path.isfile(entry_path):
                for test in self._find_tests_in_file(
                    event, path, entry_path, top_level):
                    yield test
            elif os.path.isdir(entry_path):
                if ('test' in path.lower()
                    or util.ispackage(entry_path)
                    or path in self.session.libDirs):
                    for test in self._find_tests(event, entry_path, top_level):
                        yield test

    def _find_tests_in_file(self, event, filename, full_path, top_level, module_name=None):
        log.debug("find in file %s (%s)", full_path, top_level)
        pattern = self.session.testFilePattern
        loader = event.loader
        evt = events.HandleFileEvent(
            loader, filename, full_path, pattern, top_level)
        result = self.session.hooks.handleFile(evt)
        if evt.extraTests:
            yield loader.suiteClass(evt.extraTests)

        if evt.handled:
            if result:
                yield result
            return

        if not util.valid_module_name(filename):
            # valid Python identifiers only
            return

        evt = events.MatchPathEvent(filename, full_path, pattern)
        result = self.session.hooks.matchPath(evt)
        if evt.handled:
            if not result:
                return
        elif not self._match_path(filename, full_path, pattern):
            return

        if module_name is None:
            module_name = util.name_from_path(full_path)
        
        try:
            module = util.module_from_name(module_name)
        except:
            yield loader.failedImport(module_name)
        else:
            mod_file = os.path.abspath(
                getattr(module, '__file__', full_path))
            realpath = os.path.splitext(mod_file)[0]
            fullpath_noext = os.path.splitext(full_path)[0]
            if realpath.lower() != fullpath_noext.lower():
                module_dir = os.path.dirname(realpath)
                mod_name = os.path.splitext(os.path.basename(full_path))[0]
                expected_dir = os.path.dirname(full_path)
                msg = ("%r module incorrectly imported from %r. "
                       "Expected %r. Is this module globally installed?"
                       )
                raise ImportError(
                    msg % (mod_name, module_dir, expected_dir))
            yield loader.loadTestsFromModule(module)

    def _find_tests_in_module(self, event, module, top_level_dir):
        # only called from loadTestsFromName
        yield event.loader.loadTestsFromModule(module)
        # may be a package; recurse into __path__ if so
        pkgpath = getattr(module, '__path__', None)
        if pkgpath:
            for entry in pkgpath:
                full_path = os.path.abspath(os.path.join(top_level_dir, entry))
                for test in self._find_tests_in_dir(
                    event, full_path, top_level_dir):
                    yield test

    def _match_path(self, path, full_path, pattern):
        # override this method to use alternative matching strategy
        return fnmatch(path, pattern)


class DiscoveryLoader(events.Plugin, Discoverer):
    """Loader plugin that can discover tests"""
    alwaysOn = True
    configSection = 'discovery'

    def registerInSubprocess(self, event):
        event.pluginClasses.append(self.__class__)

    def loadTestsFromName(self, event):
        """Load tests from module named by event.name"""
        return Discoverer.loadTestsFromName(self, event)

    def loadTestsFromNames(self, event):
        """Discover tests if no test names specified"""
        return Discoverer.loadTestsFromNames(self, event)
