# -*- coding: utf-8 -*-

"""
***************************************************************************
    utils.py
    ---------------------
    Date                 : November 2009
    Copyright            : (C) 2009 by Martin Dobias
    Email                : wonder dot sk at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Martin Dobias'
__date__ = 'November 2009'
__copyright__ = '(C) 2009, Martin Dobias'

"""
QGIS utilities module

"""
from typing import List, Dict, Optional

from qgis.PyQt.QtCore import QCoreApplication, QLocale, QThread, qDebug, QUrl
from qgis.PyQt.QtGui import QDesktopServices
from qgis.PyQt.QtWidgets import QPushButton, QApplication
from qgis.core import Qgis, QgsMessageLog, qgsfunction, QgsMessageOutput
from qgis.gui import QgsMessageBar

import os
import sys
import traceback
import glob
import os.path
import configparser
import warnings
import codecs
import time
import functools

import builtins
builtins.__dict__['unicode'] = str
builtins.__dict__['basestring'] = str
builtins.__dict__['long'] = int
builtins.__dict__['Set'] = set

# ######################
# ERROR HANDLING

warnings.simplefilter('default')
warnings.filterwarnings("ignore", "the sets module is deprecated")


def showWarning(message, category, filename, lineno, file=None, line=None):
    stk = ""
    for s in traceback.format_stack()[:-2]:
        if hasattr(s, 'decode'):
            stk += s.decode(sys.getfilesystemencoding())
        else:
            stk += s
    if hasattr(filename, 'decode'):
        decoded_filename = filename.decode(sys.getfilesystemencoding())
    else:
        decoded_filename = filename
    QgsMessageLog.logMessage(
        u"warning:{}\ntraceback:{}".format(warnings.formatwarning(message, category, decoded_filename, lineno), stk),
        QCoreApplication.translate("Python", "Python warning")
    )


def showException(type, value, tb, msg, messagebar=False, level=Qgis.Warning):
    if msg is None:
        msg = QCoreApplication.translate('Python', 'An error has occurred while executing Python code:')

    logmessage = ''
    for s in traceback.format_exception(type, value, tb):
        logmessage += s.decode('utf-8', 'replace') if hasattr(s, 'decode') else s

    title = QCoreApplication.translate('Python', 'Python error')
    QgsMessageLog.logMessage(logmessage, title, level)

    try:
        blockingdialog = QApplication.instance().activeModalWidget()
        window = QApplication.instance().activeWindow()
    except:
        blockingdialog = QApplication.activeModalWidget()
        window = QApplication.activeWindow()

    # Still show the normal blocking dialog in this case for now.
    if blockingdialog or not window or not messagebar or not iface:
        open_stack_dialog(type, value, tb, msg)
        return

    bar = iface.messageBar() if iface else None

    # If it's not the main window see if we can find a message bar to report the error in
    if not window.objectName() == "QgisApp":
        widgets = window.findChildren(QgsMessageBar)
        if widgets:
            # Grab the first message bar for now
            bar = widgets[0]

    item = bar.currentItem()
    if item and item.property("Error") == msg:
        # Return of we already have a message with the same error message
        return

    widget = bar.createMessage(title, msg + " " + QCoreApplication.translate("Python", "See message log (Python Error) for more details."))
    widget.setProperty("Error", msg)
    stackbutton = QPushButton(QCoreApplication.translate("Python", "Stack trace"), pressed=functools.partial(open_stack_dialog, type, value, tb, msg))
    button = QPushButton(QCoreApplication.translate("Python", "View message log"), pressed=show_message_log)
    widget.layout().addWidget(stackbutton)
    widget.layout().addWidget(button)
    bar.pushWidget(widget, Qgis.Warning)


def show_message_log(pop_error=True):
    if pop_error:
        iface.messageBar().popWidget()

    iface.openMessageLog()


def open_stack_dialog(type, value, tb, msg, pop_error=True):
    if pop_error and iface is not None:
        iface.messageBar().popWidget()

    if msg is None:
        msg = QCoreApplication.translate('Python', 'An error has occurred while executing Python code:')

    # TODO Move this to a template HTML file
    txt = u'''<font color="red"><b>{msg}</b></font>
<br>
<h3>{main_error}</h3>
<pre>
{error}
</pre>
<br>
<b>{version_label}</b> {num}
<br>
<b>{qgis_label}</b> {qversion} {qgisrelease}, {devversion}
<br>
<h4>{pypath_label}</h4>
<ul>
{pypath}
</ul>'''

    error = ''
    lst = traceback.format_exception(type, value, tb)
    for s in lst:
        error += s.decode('utf-8', 'replace') if hasattr(s, 'decode') else s
    error = error.replace('\n', '<br>')

    main_error = lst[-1].decode('utf-8', 'replace') if hasattr(lst[-1], 'decode') else lst[-1]

    version_label = QCoreApplication.translate('Python', 'Python version:')
    qgis_label = QCoreApplication.translate('Python', 'QGIS version:')
    pypath_label = QCoreApplication.translate('Python', 'Python Path:')
    txt = txt.format(msg=msg,
                     main_error=main_error,
                     error=error,
                     version_label=version_label,
                     num=sys.version,
                     qgis_label=qgis_label,
                     qversion=Qgis.QGIS_VERSION,
                     qgisrelease=Qgis.QGIS_RELEASE_NAME,
                     devversion=Qgis.QGIS_DEV_VERSION,
                     pypath_label=pypath_label,
                     pypath=u"".join(u"<li>{}</li>".format(path) for path in sys.path))

    txt = txt.replace('  ', '&nbsp; ')  # preserve whitespaces for nicer output

    dlg = QgsMessageOutput.createMessageOutput()
    dlg.setTitle(msg)
    dlg.setMessage(txt, QgsMessageOutput.MessageHtml)
    dlg.showMessage()


def qgis_excepthook(type, value, tb):
    # detect if running in the main thread
    in_main_thread = QCoreApplication.instance() is None or QThread.currentThread() == QCoreApplication.instance().thread()

    # only use messagebar if running in main thread - otherwise it will crash!
    showException(type, value, tb, None, messagebar=in_main_thread)


def installErrorHook():
    """
    Installs the QGIS application error/warning hook. This causes Python exceptions
    to be intercepted by the QGIS application and shown in the main window message bar
    and in custom dialogs.

    Generally you shouldn't call this method - it's automatically called by
    the QGIS app on startup, and has no use in standalone applications and scripts.
    """
    sys.excepthook = qgis_excepthook
    warnings.showwarning = showWarning


def uninstallErrorHook():
    sys.excepthook = sys.__excepthook__


# initialize 'iface' object
iface = None


def initInterface(pointer):
    from qgis.gui import QgisInterface
    from sip import wrapinstance

    global iface
    iface = wrapinstance(pointer, QgisInterface)


#######################
# PLUGINS

# list of plugin paths. it gets filled in by the QGIS python library
plugin_paths = []

# dictionary of plugins
plugins = {}

plugin_times = {}

# list of active (started) plugins
active_plugins = []

# list of plugins in plugin directory and home plugin directory
available_plugins = []

# dictionary of plugins providing metadata in a text file (metadata.txt)
# key = plugin package name, value = config parser instance
plugins_metadata_parser = {}


def findPlugins(path):
    """ for internal use: return list of plugins in given path """
    for plugin in glob.glob(path + "/*"):
        if not os.path.isdir(plugin):
            continue
        if not os.path.exists(os.path.join(plugin, '__init__.py')):
            continue

        metadataFile = os.path.join(plugin, 'metadata.txt')
        if not os.path.exists(metadataFile):
            continue

        cp = configparser.ConfigParser()

        try:
            with codecs.open(metadataFile, "r", "utf8") as f:
                cp.read_file(f)
        except:
            cp = None

        pluginName = os.path.basename(plugin)
        yield (pluginName, cp)


def metadataParser() -> dict:
    """Used by other modules to access the local parser object"""
    return plugins_metadata_parser


def updateAvailablePlugins(sort_by_dependencies=False):
    """ Go through the plugin_paths list and find out what plugins are available. """
    # merge the lists
    plugins = []
    metadata_parser = {}
    plugin_name_map = {}
    for pluginpath in plugin_paths:
        for plugin_id, parser in findPlugins(pluginpath):
            if parser is None:
                continue
            if plugin_id not in plugins:
                plugins.append(plugin_id)
                metadata_parser[plugin_id] = parser
                plugin_name_map[parser.get('general', 'name')] = plugin_id

    global plugins_metadata_parser
    plugins_metadata_parser = metadata_parser

    global available_plugins
    available_plugins = _sortAvailablePlugins(plugins, plugin_name_map) if sort_by_dependencies else plugins


def _sortAvailablePlugins(plugins: List[str], plugin_name_map: Dict[str, str]) -> List[str]:
    """Place dependent plugins after their dependencies

    1. Make a copy of plugins list to modify it.
    2. Get a plugin dependencies dict.
    3. Iterate plugins and leave the real work to _move_plugin()

    :param list plugins: List of available plugin ids
    :param dict plugin_name_map: Map of plugin_names and plugin_ids, because
                                 get_plugin_deps() only returns plugin names
    :return: List of plugins sorted by dependencies.
    """
    sorted_plugins = plugins.copy()
    visited_plugins = []

    deps = {}
    for plugin in plugins:
        deps[plugin] = [plugin_name_map.get(dep, '') for dep in get_plugin_deps(plugin)]

    for plugin in plugins:
        _move_plugin(plugin, deps, visited_plugins, sorted_plugins)

    return sorted_plugins


def _move_plugin(plugin: str, deps: Dict[str, List[str]], visited: List[str], sorted_plugins: List[str]):
    """Use recursion to move a plugin after its dependencies in a list of
    sorted plugins.

    Notes:
    This function modifies both visited and sorted_plugins lists.
    This function will not get trapped in circular dependencies. We avoid a
    maximum recursion error by calling return when revisiting a plugin.
    Therefore, if a plugin A depends on B and B depends on A, the order will
    work in one direction (e.g., A depends on B), but the other direction won't
    be satisfied. After all, a circular plugin dependency should not exist.

    :param str plugin: Id of the plugin that should be moved in sorted_plugins.
    :param dict deps: Dictionary of plugin dependencies.
    :param list visited: List of plugins already visited.
    :param list sorted_plugins: List of plugins to be modified and sorted.
    """
    if plugin in visited:
        return
    elif plugin not in deps or not deps[plugin]:
        visited.append(plugin)  # Plugin with no dependencies
    else:
        visited.append(plugin)

        # First move dependencies
        for dep in deps[plugin]:
            _move_plugin(dep, deps, visited, sorted_plugins)

        # Remove current plugin from sorted
        # list to get dependency indices
        max_index = sorted_plugins.index(plugin)
        sorted_plugins.pop(max_index)

        for dep in deps[plugin]:
            idx = sorted_plugins.index(dep) + 1 if dep in sorted_plugins else -1
            max_index = max(idx, max_index)

        # Finally, insert after dependencies
        sorted_plugins.insert(max_index, plugin)


def get_plugin_deps(plugin_id: str) -> Dict[str, Optional[str]]:
    result = {}
    try:
        parser = plugins_metadata_parser[plugin_id]
        plugin_deps = parser.get('general', 'plugin_dependencies')
    except (configparser.NoOptionError, configparser.NoSectionError, KeyError):
        return result

    for dep in plugin_deps.split(','):
        if dep.find('==') > 0:
            name, version_required = dep.split('==')
        else:
            name = dep
            version_required = None
        result[name] = version_required
    return result


def pluginMetadata(packageName: str, fct: str) -> str:
    """ fetch metadata from a plugin - use values from metadata.txt """
    try:
        return plugins_metadata_parser[packageName].get('general', fct)
    except Exception:
        return "__error__"


def loadPlugin(packageName: str) -> bool:
    """ load plugin's package """

    try:
        __import__(packageName)
        return True
    except:
        pass  # continue...

    # snake in the grass, we know it's there
    sys.path_importer_cache.clear()

    # retry
    try:
        __import__(packageName)
        return True
    except:
        msg = QCoreApplication.translate("Python", "Couldn't load plugin '{0}'").format(packageName)
        showException(sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2], msg, messagebar=True, level=Qgis.Critical)
        return False


def _startPlugin(packageName: str) -> bool:
    """ initializes a plugin, but does not load GUI """
    global plugins, active_plugins, iface, plugin_times

    if packageName in active_plugins:
        return False

    if packageName not in sys.modules:
        return False

    package = sys.modules[packageName]

    # create an instance of the plugin
    try:
        plugins[packageName] = package.classFactory(iface)
    except:
        _unloadPluginModules(packageName)
        errMsg = QCoreApplication.translate("Python", "Couldn't load plugin '{0}'").format(packageName)
        msg = QCoreApplication.translate("Python", "{0} due to an error when calling its classFactory() method").format(errMsg)
        showException(sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2], msg, messagebar=True, level=Qgis.Critical)
        return False
    return True


def _addToActivePlugins(packageName: str, duration: int):
    """ Adds a plugin to the list of active plugins """
    active_plugins.append(packageName)
    plugin_times[packageName] = "{0:02f}s".format(duration)


def startPlugin(packageName: str) -> bool:
    """ initialize the plugin """
    global plugins, active_plugins, iface, plugin_times
    start = time.process_time()
    if not _startPlugin(packageName):
        return False

    # initGui
    try:
        plugins[packageName].initGui()
    except:
        del plugins[packageName]
        _unloadPluginModules(packageName)
        errMsg = QCoreApplication.translate("Python", "Couldn't load plugin '{0}'").format(packageName)
        msg = QCoreApplication.translate("Python", "{0} due to an error when calling its initGui() method").format(errMsg)
        showException(sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2], msg, messagebar=True, level=Qgis.Critical)
        return False

    end = time.process_time()
    _addToActivePlugins(packageName, end - start)
    return True


def startProcessingPlugin(packageName: str) -> bool:
    """ initialize only the Processing components of a plugin """
    global plugins, active_plugins, iface, plugin_times
    start = time.process_time()
    if not _startPlugin(packageName):
        return False

    errMsg = QCoreApplication.translate("Python", "Couldn't load plugin '{0}'").format(packageName)
    if not hasattr(plugins[packageName], 'initProcessing'):
        del plugins[packageName]
        _unloadPluginModules(packageName)
        msg = QCoreApplication.translate("Python", "{0} - plugin has no initProcessing() method").format(errMsg)
        showException(sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2], msg, messagebar=True, level=Qgis.Critical)
        return False

    # initProcessing
    try:
        plugins[packageName].initProcessing()
    except:
        del plugins[packageName]
        _unloadPluginModules(packageName)
        msg = QCoreApplication.translate("Python", "{0} due to an error when calling its initProcessing() method").format(errMsg)
        showException(sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2], msg, messagebar=True)
        return False

    end = time.process_time()
    _addToActivePlugins(packageName, end - start)

    return True


def canUninstallPlugin(packageName: str) -> bool:
    """ confirm that the plugin can be uninstalled """
    global plugins, active_plugins

    if packageName not in plugins:
        return False
    if packageName not in active_plugins:
        return False

    try:
        metadata = plugins[packageName]
        if "canBeUninstalled" not in dir(metadata):
            return True
        return bool(metadata.canBeUninstalled())
    except:
        msg = "Error calling " + packageName + ".canBeUninstalled"
        showException(sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2], msg, messagebar=True)
        return True


def unloadPlugin(packageName: str) -> bool:
    """ unload and delete plugin! """
    global plugins, active_plugins

    if packageName not in plugins:
        return False
    if packageName not in active_plugins:
        return False

    try:
        plugins[packageName].unload()
        del plugins[packageName]
        active_plugins.remove(packageName)
        _unloadPluginModules(packageName)
        return True
    except Exception as e:
        msg = QCoreApplication.translate("Python", "Error while unloading plugin {0}").format(packageName)
        showException(sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2], msg, messagebar=True)
        return False


def _unloadPluginModules(packageName: str):
    """ unload plugin package with all its modules (files) """
    global _plugin_modules
    mods = _plugin_modules[packageName]

    for mod in mods:
        if mod not in sys.modules:
            continue

        # if it looks like a Qt resource file, try to do a cleanup
        # otherwise we might experience a segfault next time the plugin is loaded
        # because Qt will try to access invalid plugin resource data
        try:
            if hasattr(sys.modules[mod], 'qCleanupResources'):
                sys.modules[mod].qCleanupResources()
        except:
            # Print stack trace for debug
            qDebug("qCleanupResources error:\n%s" % traceback.format_exc())

        # try removing path
        if hasattr(sys.modules[mod], '__path__'):
            for path in sys.modules[mod].__path__:
                try:
                    sys.path.remove(path)
                except ValueError:
                    # Discard if path is not there
                    pass

        # try to remove the module from python
        try:
            del sys.modules[mod]
        except:
            qDebug("Error when removing module:\n%s" % traceback.format_exc())
    # remove the plugin entry
    del _plugin_modules[packageName]


def isPluginLoaded(packageName: str) -> bool:
    """ find out whether a plugin is active (i.e. has been started) """
    global plugins, active_plugins

    if packageName not in plugins:
        return False
    return (packageName in active_plugins)


def reloadPlugin(packageName: str) -> bool:
    """ unload and start again a plugin """
    global active_plugins
    if packageName not in active_plugins:
        return False  # it's not active

    unloadPlugin(packageName)
    loadPlugin(packageName)
    started = startPlugin(packageName)
    return started


def showPluginHelp(packageName: str = None, filename: str = "index", section: str = ""):
    """Open help in the user's html browser. The help file should be named index-ll_CC.html or index-ll.html or index.html.

    :param str packageName: name of package folder, if None it's using the current file package. Defaults to None. Optional.
    :param str filename: name of file to open. It can be a path like 'doc/index' for example. Defaults to 'index'.
    :param str section: URL path to open. Defaults to empty string.
    """
    try:
        source = ""
        if packageName is None:
            import inspect

            source = inspect.currentframe().f_back.f_code.co_filename
        else:
            source = sys.modules[packageName].__file__
    except:
        return
    path = os.path.dirname(source)
    locale = str(QLocale().name())
    helpfile = os.path.join(path, filename + "-" + locale + ".html")
    if not os.path.exists(helpfile):
        helpfile = os.path.join(path, filename + "-" + locale.split("_")[0] + ".html")
    if not os.path.exists(helpfile):
        helpfile = os.path.join(path, filename + "-en.html")
    if not os.path.exists(helpfile):
        helpfile = os.path.join(path, filename + "-en_US.html")
    if not os.path.exists(helpfile):
        helpfile = os.path.join(path, filename + ".html")
    if os.path.exists(helpfile):
        url = "file://" + helpfile
        if section != "":
            url = url + "#" + section
        QDesktopServices.openUrl(QUrl.fromLocalFile(url))


def pluginDirectory(packageName: str) -> str:
    """ return directory where the plugin resides. Plugin must be loaded already """
    return os.path.dirname(sys.modules[packageName].__file__)


def reloadProjectMacros():
    # unload old macros
    unloadProjectMacros()

    from qgis.core import QgsProject

    code, ok = QgsProject.instance().readEntry("Macros", "/pythonCode")
    if not ok or not code or code == '':
        return

    # create a new empty python module
    import importlib
    mod = importlib.util.module_from_spec(importlib.machinery.ModuleSpec("proj_macros_mod", None))

    # set the module code and store it sys.modules
    exec(str(code), mod.__dict__)
    sys.modules["proj_macros_mod"] = mod

    # load new macros
    openProjectMacro()


def unloadProjectMacros():
    if "proj_macros_mod" not in sys.modules:
        return
    # unload old macros
    closeProjectMacro()
    # destroy the reference to the module
    del sys.modules["proj_macros_mod"]


def openProjectMacro():
    if "proj_macros_mod" not in sys.modules:
        return
    mod = sys.modules["proj_macros_mod"]
    if hasattr(mod, 'openProject'):
        mod.openProject()


def saveProjectMacro():
    if "proj_macros_mod" not in sys.modules:
        return
    mod = sys.modules["proj_macros_mod"]
    if hasattr(mod, 'saveProject'):
        mod.saveProject()


def closeProjectMacro():
    if "proj_macros_mod" not in sys.modules:
        return
    mod = sys.modules["proj_macros_mod"]
    if hasattr(mod, 'closeProject'):
        mod.closeProject()


#######################
# SERVER PLUGINS
#
# TODO: move into server_utils.py ?

# list of plugin paths. it gets filled in by the QGIS python library
server_plugin_paths = []

# dictionary of plugins
server_plugins = {}

# list of active (started) plugins
server_active_plugins = []


# initialize 'serverIface' object
serverIface = None


def initServerInterface(pointer):
    from qgis.server import QgsServerInterface
    from sip import wrapinstance
    sys.excepthook = sys.__excepthook__
    global serverIface
    serverIface = wrapinstance(pointer, QgsServerInterface)


def startServerPlugin(packageName: str):
    """ initialize the plugin """
    global server_plugins, server_active_plugins, serverIface

    if packageName in server_active_plugins:
        return False
    if packageName not in sys.modules:
        return False

    package = sys.modules[packageName]

    errMsg = QCoreApplication.translate("Python", "Couldn't load server plugin {0}").format(packageName)

    # create an instance of the plugin
    try:
        server_plugins[packageName] = package.serverClassFactory(serverIface)
    except:
        _unloadPluginModules(packageName)
        msg = QCoreApplication.translate("Python",
                                         "{0} due to an error when calling its serverClassFactory() method").format(errMsg)
        showException(sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2], msg)
        return False

    # add to active plugins
    server_active_plugins.append(packageName)
    return True


def spatialite_connect(*args, **kwargs):
    """returns a dbapi2.Connection to a SpatiaLite db
using the "mod_spatialite" extension (python3)"""
    import sqlite3
    import re

    def fcnRegexp(pattern, string):
        result = re.search(pattern, string)
        return True if result else False

    con = sqlite3.dbapi2.connect(*args, **kwargs)
    con.enable_load_extension(True)
    cur = con.cursor()
    libs = [
        # SpatiaLite >= 4.2 and Sqlite >= 3.7.17, should work on all platforms
        ("mod_spatialite", "sqlite3_modspatialite_init"),
        # SpatiaLite >= 4.2 and Sqlite < 3.7.17 (Travis)
        ("mod_spatialite.so", "sqlite3_modspatialite_init"),
        # SpatiaLite < 4.2 (linux)
        ("libspatialite.so", "sqlite3_extension_init")
    ]
    found = False
    for lib, entry_point in libs:
        try:
            cur.execute("select load_extension('{}', '{}')".format(lib, entry_point))
        except sqlite3.OperationalError:
            continue
        else:
            found = True
            break
    if not found:
        raise RuntimeError("Cannot find any suitable spatialite module")
    if any(['.gpkg' in arg for arg in args]):
        try:
            cur.execute("SELECT EnableGpkgAmphibiousMode()")
        except (sqlite3.Error, sqlite3.DatabaseError, sqlite3.NotSupportedError):
            QgsMessageLog.logMessage(u"warning:{}".format("Could not enable geopackage amphibious mode"),
                                     QCoreApplication.translate("Python", "Python warning"))

    cur.close()
    con.enable_load_extension(False)
    con.create_function("regexp", 2, fcnRegexp)
    return con


class OverrideCursor():
    """
    Executes a code block with a different cursor set and makes sure the cursor
    is restored even if exceptions are raised or an intermediate ``return``
    statement is hit.

    Example:
    ```
    with OverrideCursor(Qt.WaitCursor):
        do_a_slow(operation)
    ```
    """

    def __init__(self, cursor):
        self.cursor = cursor

    def __enter__(self):
        QApplication.setOverrideCursor(self.cursor)

    def __exit__(self, exc_type, exc_val, exc_tb):
        QApplication.restoreOverrideCursor()
        return exc_type is None


#######################
# IMPORT wrapper

if os.name == 'nt' and sys.version_info < (3, 8):
    import ctypes
    from ctypes import windll, wintypes

    kernel32 = ctypes.WinDLL('kernel32', use_last_error=True)

    _hasAddDllDirectory = hasattr(kernel32, 'AddDllDirectory')
    if _hasAddDllDirectory:
        _import_path = os.environ['PATH']
        _import_paths = {}

        def _errcheck_zero(result, func, args):
            if not result:
                raise ctypes.WinError(ctypes.get_last_error())
            return args

        DLL_DIRECTORY_COOKIE = wintypes.LPVOID

        _AddDllDirectory = kernel32.AddDllDirectory
        _AddDllDirectory.errcheck = _errcheck_zero
        _AddDllDirectory.restype = DLL_DIRECTORY_COOKIE
        _AddDllDirectory.argtypes = (wintypes.LPCWSTR,)

        _RemoveDllDirectory = kernel32.RemoveDllDirectory
        _RemoveDllDirectory.errcheck = _errcheck_zero
        _RemoveDllDirectory.argtypes = (DLL_DIRECTORY_COOKIE,)

_uses_builtins = True
try:
    import builtins
    _builtin_import = builtins.__import__
except AttributeError:
    _uses_builtins = False
    import __builtin__
    _builtin_import = __builtin__.__import__

_plugin_modules = {}


def _import(name, globals={}, locals={}, fromlist=[], level=None):
    """
    Wrapper around builtin import that keeps track of loaded plugin modules and blocks
    certain unsafe imports
    """
    if level is None:
        level = 0

    if 'PyQt4' in name:
        msg = 'PyQt4 classes cannot be imported in QGIS 3.x.\n' \
              'Use {} or the version independent {} import instead.'.format(name.replace('PyQt4', 'PyQt5'), name.replace('PyQt4', 'qgis.PyQt'))
        raise ImportError(msg)

    if os.name == 'nt' and sys.version_info < (3, 8):
        global _hasAddDllDirectory
        if _hasAddDllDirectory:
            global _import_path
            global _import_paths

            old_path = _import_path
            new_path = os.environ['PATH']
            if old_path != new_path:
                global _AddDllDirectory
                global _RemoveDllDirectory

                for p in set(new_path.split(';')) - set(old_path.split(';')):
                    if p is not None and p not in _import_path and os.path.isdir(p):
                        _import_paths[p] = _AddDllDirectory(p)

                for p in set(old_path.split(';')) - set(new_path.split(';')):
                    if p in _import_paths:
                        _RemoveDllDirectory(_import_paths.pop(p))

                _import_path = new_path

    mod = _builtin_import(name, globals, locals, fromlist, level)

    if mod and getattr(mod, '__file__', None):
        module_name = mod.__name__ if fromlist else name
        package_name = module_name.split('.')[0]
        # check whether the module belongs to one of our plugins
        if package_name in available_plugins:
            if package_name not in _plugin_modules:
                _plugin_modules[package_name] = set()
            _plugin_modules[package_name].add(module_name)
            # check the fromlist for additional modules (from X import Y,Z)
            if fromlist:
                for fromitem in fromlist:
                    frmod = module_name + "." + fromitem
                    if frmod in sys.modules:
                        _plugin_modules[package_name].add(frmod)

    return mod


if not os.environ.get('QGIS_NO_OVERRIDE_IMPORT'):
    if _uses_builtins:
        builtins.__import__ = _import
    else:
        __builtin__.__import__ = _import


def processing_algorithm_from_script(filepath: str):
    """
    Tries to import a Python processing algorithm from given file, and returns an instance
    of the algorithm
    """
    import sys
    import inspect
    from qgis.processing import alg
    try:
        from qgis.core import QgsApplication, QgsProcessingAlgorithm, QgsProcessingFeatureBasedAlgorithm
        _locals = {}
        with open(filepath.replace("\\\\", "/").encode(sys.getfilesystemencoding())) as input_file:
            exec(input_file.read(), _locals)
        alg_instance = None
        try:
            alg_instance = alg.instances.pop().createInstance()
        except IndexError:
            for name, attr in _locals.items():
                if inspect.isclass(attr) and issubclass(attr, (QgsProcessingAlgorithm, QgsProcessingFeatureBasedAlgorithm)) and attr.__name__ not in ("QgsProcessingAlgorithm", "QgsProcessingFeatureBasedAlgorithm"):
                    alg_instance = attr()
                    break
        if alg_instance:
            script_provider = QgsApplication.processingRegistry().providerById("script")
            alg_instance.setProvider(script_provider)
            alg_instance.initAlgorithm()
            return alg_instance
    except ImportError:
        pass

    return None


def import_script_algorithm(filepath: str) -> Optional[str]:
    """
    Imports a script algorithm from given file to the processing script provider, and returns the
    ID of the imported algorithm
    """
    alg_instance = processing_algorithm_from_script(filepath)
    if alg_instance:
        from qgis.core import QgsApplication
        script_provider = QgsApplication.processingRegistry().providerById("script")
        script_provider.add_algorithm_class(type(alg_instance))
        return alg_instance.id()

    return None


def run_script_from_file(filepath: str):
    """
    Runs a Python script from a given file. Supports loading processing scripts.
    :param filepath: The .py file to load.
    """
    try:
        from qgis.processing import execAlgorithmDialog
    except ImportError:
        return

    alg_instance = processing_algorithm_from_script(filepath)
    if alg_instance:
        execAlgorithmDialog(alg_instance)
