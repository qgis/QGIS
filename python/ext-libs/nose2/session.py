import logging
import os

import argparse
from six.moves import configparser

from nose2 import config, events, util


log = logging.getLogger(__name__)
__unittest = True


class Session(object):

    """Configuration session.

    Encapsulates all configuration for a given test run.

    .. attribute :: argparse

       An instance of :class:`argparse.ArgumentParser`. Plugins can
       use this directly to add arguments and argument groups, but
       *must* do so in their ``__init__`` methods.

    .. attribute :: pluginargs

       The argparse argument group in which plugins (by default) place
       their command-line arguments. Plugins can use this directly to
       add arguments, but *must* do so in their ``__init__`` methods.

    .. attribute :: hooks

       The :class:`nose2.events.PluginInterface` instance contains
       all available plugin methods and hooks.

    .. attribute :: plugins

       The list of loaded -- but not necessarily *active* -- plugins.

    .. attribute :: verbosity

       Current verbosity level. Default: 1.

    .. attribute :: startDir

       Start directory of test run. Test discovery starts
       here. Default: current working directory.

    .. attribute :: topLevelDir

       Top-level directory of test run. This directory is added to
       sys.path. Default: starting directory.

    .. attribute :: libDirs

       Names of code directories, relative to starting
       directory. Default: ['lib', 'src']. These directories are added
       to sys.path and discovery if the exist.

    .. attribute :: testFilePattern

       Pattern used to discover test module files. Default: test*.py

    .. attribute :: testMethodPrefix

       Prefix used to discover test methods and functions: Default: 'test'.

    .. attribute :: unittest

       The config section for nose2 itself.

    """
    configClass = config.Config
    
    def __init__(self):
        self.argparse = argparse.ArgumentParser(prog='nose2', add_help=False)
        self.pluginargs = self.argparse.add_argument_group(
            'plugin arguments',
            'Command-line arguments added by plugins:')
        self.config = configparser.ConfigParser()
        self.hooks = events.PluginInterface()
        self.plugins = []
        self.verbosity = 1
        self.startDir = None
        self.topLevelDir = None
        self.testResult = None
        self.testLoader = None
        self.logLevel = logging.WARN

    def get(self, section):
        """Get a config section.

        :param section: The section name to retreive.
        :returns: instance of self.configClass.

        """
        # FIXME cache these
        items = []
        if self.config.has_section(section):
            items = self.config.items(section)
        return self.configClass(items)

    def loadConfigFiles(self, *filenames):
        """Load config files.

        :param filenames: Names of config files to load.

        Loads all names files that exist into ``self.config``.

        """
        self.config.read(filenames)

    def loadPlugins(self, modules=None, exclude=None):
        """Load plugins.

        :param modules: List of module names from which to load plugins.

        """
        # plugins set directly
        if modules is None:
            modules = []
        if exclude is None:
            exclude = []
        # plugins mentioned in config file(s)
        cfg = self.unittest
        more_plugins = cfg.as_list('plugins', [])
        cfg_exclude = cfg.as_list('exclude-plugins', [])
        exclude.extend(cfg_exclude)
        exclude = set(exclude)
        all_ = (set(modules) | set(more_plugins)) - exclude
        log.debug("Loading plugin modules: %s", all_)
        for module in all_:
            self.loadPluginsFromModule(util.module_from_name(module))
        self.hooks.pluginsLoaded(events.PluginsLoadedEvent(self.plugins))

    def loadPluginsFromModule(self, module):
        """Load plugins from a module.

        :param module: A python module containing zero or more plugin
                       classes.

        """
        avail = []
        for entry in dir(module):
            try:
                item = getattr(module, entry)
            except AttributeError:
                pass
            try:
                if issubclass(item, events.Plugin):
                    avail.append(item)
            except TypeError:
                pass
        for cls in avail:
            log.debug("Plugin is available: %s", cls)
            plugin = cls(session=self)
            self.plugins.append(plugin)
            for method in self.hooks.preRegistrationMethods:
                if hasattr(plugin, method):
                    self.hooks.register(method, plugin)

    def registerPlugin(self, plugin):
        """Register a plugin.

        :param plugin: A `nose2.events.Plugin` instance.

        Register the plugin with all methods it implements.

        """
        log.debug("Register active plugin %s", plugin)
        if plugin not in self.plugins:
            self.plugins.append(plugin)
        for method in self.hooks.methods:
            if hasattr(plugin, method):
                log.debug("Register method %s for plugin %s", method, plugin)
                self.hooks.register(method, plugin)

    def setStartDir(self):
        if self.startDir is None:
            self.startDir = self.unittest.as_str('start-dir', '.')

    def prepareSysPath(self):
        """Add code directories to sys.path"""
        tld = self.topLevelDir
        sd = self.startDir
        if tld is None:
            tld = sd
        tld = os.path.abspath(tld)
        util.ensure_importable(tld)
        for libdir in self.libDirs:
            libdir = os.path.abspath(os.path.join(tld, libdir))
            if os.path.exists(libdir):
                util.ensure_importable(libdir)

    # convenience properties
    @property
    def libDirs(self):
        return self.unittest.as_list('code-directories', ['lib', 'src'])

    @property
    def testFilePattern(self):
        return self.unittest.as_str('test-file-pattern', 'test*.py')

    @property
    def testMethodPrefix(self):
        return self.unittest.as_str('test-method-prefix', 'test')

    @property
    def unittest(self):
        return self.get('unittest')

    def isPluginLoaded(self, pluginName):
        """Returns True if a given plugin is loaded.

        :param pluginName: the name of the plugin module: e.g. "nose2.plugins.layers".

        """
        for plugin in self.plugins:
            if pluginName == plugin.__class__.__module__:
                return True
        return False
