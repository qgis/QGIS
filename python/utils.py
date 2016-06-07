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
from __future__ import absolute_import
from future import standard_library
standard_library.install_aliases()
from builtins import str

__author__ = 'Martin Dobias'
__date__ = 'November 2009'
__copyright__ = '(C) 2009, Martin Dobias'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

"""
QGIS utilities module

"""

from qgis.PyQt.QtCore import QCoreApplication, QLocale
from qgis.PyQt.QtWidgets import QPushButton, QApplication
from qgis.core import QGis, QgsExpression, QgsMessageLog, qgsfunction, QgsMessageOutput
from qgis.gui import QgsMessageBar

import sys
import traceback
import glob
import os.path
try:
    import configparser
except ImportError:
    import ConfigParser as configparser
import warnings
import codecs
import time
import functools

if sys.version_info[0] >= 3:
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


warnings.showwarning = showWarning


def showException(type, value, tb, msg, messagebar=False):
    if msg is None:
        msg = QCoreApplication.translate('Python', 'An error has occurred while executing Python code:')

    logmessage = ''
    for s in traceback.format_exception(type, value, tb):
        logmessage += s.decode('utf-8', 'replace') if hasattr(s, 'decode') else s

    title = QCoreApplication.translate('Python', 'Python error')
    QgsMessageLog.logMessage(logmessage, title)

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

    bar = iface.messageBar()

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
    bar.pushWidget(widget, QgsMessageBar.WARNING)


def show_message_log(pop_error=True):
    if pop_error:
        iface.messageBar().popWidget()

    iface.openMessageLog()


def open_stack_dialog(type, value, tb, msg, pop_error=True):
    if pop_error:
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
                     qversion=QGis.QGIS_VERSION,
                     qgisrelease=QGis.QGIS_RELEASE_NAME,
                     devversion=QGis.QGIS_DEV_VERSION,
                     pypath_label=pypath_label,
                     pypath=u"".join(u"<li>{}</li>".format(path) for path in sys.path))

    txt = txt.replace('  ', '&nbsp; ')  # preserve whitespaces for nicer output

    dlg = QgsMessageOutput.createMessageOutput()
    dlg.setTitle(msg)
    dlg.setMessage(txt, QgsMessageOutput.MessageHtml)
    dlg.showMessage()


def qgis_excepthook(type, value, tb):
    showException(type, value, tb, None, messagebar=True)


def installErrorHook():
    sys.excepthook = qgis_excepthook


def uninstallErrorHook():
    sys.excepthook = sys.__excepthook__

# install error hook() on module load
installErrorHook()

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
            f = codecs.open(metadataFile, "r", "utf8")
            cp.readfp(f)
            f.close()
        except:
            cp = None

        pluginName = os.path.basename(plugin)
        yield (pluginName, cp)


def updateAvailablePlugins():
    """ Go through the plugin_paths list and find out what plugins are available. """
    # merge the lists
    plugins = []
    metadata_parser = {}
    for pluginpath in plugin_paths:
        for pluginName, parser in findPlugins(pluginpath):
            if parser is None:
                continue
            if pluginName not in plugins:
                plugins.append(pluginName)
                metadata_parser[pluginName] = parser

    global available_plugins
    available_plugins = plugins
    global plugins_metadata_parser
    plugins_metadata_parser = metadata_parser


def pluginMetadata(packageName, fct):
    """ fetch metadata from a plugin - use values from metadata.txt """
    try:
        return plugins_metadata_parser[packageName].get('general', fct)
    except Exception:
        return "__error__"


def loadPlugin(packageName):
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
        msgTemplate = QCoreApplication.translate("Python", "Couldn't load plugin '%s'")
        msg = msgTemplate % packageName
        showException(sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2], msg, messagebar=True)
        return False


def startPlugin(packageName):
    """ initialize the plugin """
    global plugins, active_plugins, iface, plugin_times

    if packageName in active_plugins:
        return False
    if packageName not in sys.modules:
        return False

    package = sys.modules[packageName]

    errMsg = QCoreApplication.translate("Python", "Couldn't load plugin %s") % packageName

    start = time.clock()
    # create an instance of the plugin
    try:
        plugins[packageName] = package.classFactory(iface)
    except:
        _unloadPluginModules(packageName)
        msg = QCoreApplication.translate("Python", "%s due to an error when calling its classFactory() method") % errMsg
        showException(sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2], msg, messagebar=True)
        return False

    # initGui
    try:
        plugins[packageName].initGui()
    except:
        del plugins[packageName]
        _unloadPluginModules(packageName)
        msg = QCoreApplication.translate("Python", "%s due to an error when calling its initGui() method") % errMsg
        showException(sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2], msg, messagebar=True)
        return False

    # add to active plugins
    active_plugins.append(packageName)
    end = time.clock()
    plugin_times[packageName] = "{0:02f}s".format(end - start)

    return True


def canUninstallPlugin(packageName):
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


def unloadPlugin(packageName):
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
        msg = QCoreApplication.translate("Python", "Error while unloading plugin %s") % packageName
        showException(sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2], msg, messagebar=True)
        return False


def _unloadPluginModules(packageName):
    """ unload plugin package with all its modules (files) """
    global _plugin_modules
    mods = _plugin_modules[packageName]

    for mod in mods:
        # if it looks like a Qt resource file, try to do a cleanup
        # otherwise we might experience a segfault next time the plugin is loaded
        # because Qt will try to access invalid plugin resource data
        try:
            if hasattr(sys.modules[mod], 'qCleanupResources'):
                sys.modules[mod].qCleanupResources()
        except:
            pass
        # try to remove the module from python
        try:
            del sys.modules[mod]
        except:
            pass
    # remove the plugin entry
    del _plugin_modules[packageName]


def isPluginLoaded(packageName):
    """ find out whether a plugin is active (i.e. has been started) """
    global plugins, active_plugins

    if packageName not in plugins:
        return False
    return (packageName in active_plugins)


def reloadPlugin(packageName):
    """ unload and start again a plugin """
    global active_plugins
    if packageName not in active_plugins:
        return  # it's not active

    unloadPlugin(packageName)
    loadPlugin(packageName)
    startPlugin(packageName)


def showPluginHelp(packageName=None, filename="index", section=""):
    """ show a help in the user's html browser. The help file should be named index-ll_CC.html or index-ll.html"""
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
        iface.openURL(url, False)


def pluginDirectory(packageName):
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
    import imp

    mod = imp.new_module("proj_macros_mod")

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


def startServerPlugin(packageName):
    """ initialize the plugin """
    global server_plugins, server_active_plugins, serverIface

    if packageName in server_active_plugins:
        return False
    if packageName not in sys.modules:
        return False

    package = sys.modules[packageName]

    errMsg = QCoreApplication.translate("Python", "Couldn't load server plugin %s") % packageName

    # create an instance of the plugin
    try:
        server_plugins[packageName] = package.serverClassFactory(serverIface)
    except:
        _unloadPluginModules(packageName)
        msg = QCoreApplication.translate("Python",
                                         "%s due to an error when calling its serverClassFactory() method") % errMsg
        showException(sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2], msg)
        return False

    # add to active plugins
    server_active_plugins.append(packageName)
    return True


#######################
# IMPORT wrapper

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
    """ wrapper around builtin import that keeps track of loaded plugin modules """
    if level is None:
        level = -1 if sys.version_info[0] < 3 else 0
    mod = _builtin_import(name, globals, locals, fromlist, level)

    if mod and '__file__' in mod.__dict__:
        module_name = mod.__name__
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


if _uses_builtins:
    builtins.__import__ = _import
else:
    __builtin__.__import__ = _import
