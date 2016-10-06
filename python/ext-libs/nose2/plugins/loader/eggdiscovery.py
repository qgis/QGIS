"""
Egg-based discovery test loader.

This plugin implements nose2's automatic test module discovery inside Egg Files.
It looks for test modules in packages whose names start
with 'test', then fires the :func:`loadTestsFromModule` hook for each
one to allow other plugins to load the actual tests.

It also fires :func:`handleFile` for every file that it sees, and
:func:`matchPath` for every python module, to allow other plugins to
load tests from other kinds of files and to influence which modules
are examined for tests.

"""
import logging
import os

from nose2 import events

from nose2.plugins.loader import discovery

__unittest = True
log = logging.getLogger(__name__)

try:
    import pkg_resources
except ImportError:
    pkg_resources = None


class EggDiscoveryLoader(events.Plugin, discovery.Discoverer):
    """Loader plugin that can discover tests inside Egg Files"""
    alwaysOn = True
    configSection = 'discovery'

    def registerInSubprocess(self, event):
        event.pluginClasses.append(self.__class__)

    def loadTestsFromName(self, event):
        """Load tests from module named by event.name"""
        return discovery.Discoverer.loadTestsFromName(self, event)

    def loadTestsFromNames(self, event):
        """Discover tests if no test names specified"""
        return discovery.Discoverer.loadTestsFromNames(self, event)

    def _checkIfPathIsOK(self, start_dir):
        if not os.path.exists(os.path.abspath(start_dir)):
            raise OSError("%s does not exist" % os.path.abspath(start_dir))

    def _find_tests_in_egg_dir(self, event, rel_path, dist):
        log.debug("find in egg dir %s %s (%s)", dist.location, rel_path, dist.project_name)
        full_path = os.path.join(dist.location, rel_path)
        dir_handler = discovery.DirectoryHandler(self.session)
        for test in dir_handler.handle_dir(event, full_path, dist.location):
            yield test
        if dir_handler.event_handled:
            return
        for path in dist.resource_listdir(rel_path):
            entry_path = os.path.join(rel_path, path)
            if dist.resource_isdir(entry_path):
                for test in self._find_tests_in_egg_dir(event, entry_path, dist):
                    yield test
            else:
                modname = os.path.splitext(entry_path)[0].replace(os.sep, '.')
                for test in self._find_tests_in_file(
                    event, path, os.path.join(dist.location, entry_path), dist.location, modname):
                    yield test

    def _find_tests_in_dir(self, event, full_path, top_level):
        if os.path.exists(full_path):
            return
        elif pkg_resources and full_path.find('.egg') != -1:
            egg_path = full_path.split('.egg')[0] + '.egg'
            for dist in pkg_resources.find_distributions(egg_path):
                for modname in dist._get_metadata('top_level.txt'):
                    for test in self._find_tests_in_egg_dir(
                        event, modname, dist):
                        yield test
