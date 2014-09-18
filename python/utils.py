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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

"""
QGIS utilities module

"""

from PyQt4.QtCore import QCoreApplication, QLocale
from qgis.core import QGis, QgsExpression, QgsMessageLog
from string import Template
import sys
import traceback
import glob
import os.path
import re
import ConfigParser
import warnings
import codecs
import time

#######################
# ERROR HANDLING

warnings.simplefilter('default')
warnings.filterwarnings("ignore", "the sets module is deprecated")

def showWarning(message, category, filename, lineno, file=None, line=None):
  stk = ""
  for s in traceback.format_stack()[:-2]:
    stk += s.decode('utf-8', 'replace')
  QgsMessageLog.logMessage(
    "warning:%s\ntraceback:%s" % ( warnings.formatwarning(message, category, filename, lineno), stk),
    QCoreApplication.translate( "Python", "Python warning" )
  )
warnings.showwarning = showWarning

def showException(type, value, tb, msg):
  lst = traceback.format_exception(type, value, tb)
  if msg == None:
    msg = QCoreApplication.translate('Python', 'An error has occured while executing Python code:')
  txt = '<font color="red">%s</font><br><br><pre>' % msg
  for s in lst:
    txt += s.decode('utf-8', 'replace')
  txt += '</pre><br>%s<br>%s<br><br>' % (QCoreApplication.translate('Python','Python version:'), sys.version)
  txt += '<br>%s<br>%s %s, %s<br><br>' % (QCoreApplication.translate('Python','QGIS version:'), QGis.QGIS_VERSION, QGis.QGIS_RELEASE_NAME, QGis.QGIS_DEV_VERSION)
  txt += '%s %s' % (QCoreApplication.translate('Python','Python path:'), str(sys.path))
  txt = txt.replace('\n', '<br>')
  txt = txt.replace('  ', '&nbsp; ') # preserve whitespaces for nicer output

  from qgis.core import QgsMessageOutput
  msg = QgsMessageOutput.createMessageOutput()
  msg.setTitle(QCoreApplication.translate('Python', 'Python error'))
  msg.setMessage(txt, QgsMessageOutput.MessageHtml)
  msg.showMessage()

def qgis_excepthook(type, value, tb):
  showException(type, value, tb, None)

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

    cp = ConfigParser.ConfigParser()

    try:
      cp.readfp(codecs.open(metadataFile, "r", "utf8"))
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
      if parser is None: continue
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
    pass # continue...

  # snake in the grass, we know it's there
  sys.path_importer_cache.clear()

  # retry
  try:
    __import__(packageName)
    return True
  except:
    msgTemplate = QCoreApplication.translate("Python", "Couldn't load plugin '%s' from ['%s']")
    msg = msgTemplate % (packageName, "', '".join(sys.path))
    showException(sys.exc_type, sys.exc_value, sys.exc_traceback, msg)
    return False


def startPlugin(packageName):
  """ initialize the plugin """
  global plugins, active_plugins, iface, plugin_times

  if packageName in active_plugins: return False
  if packageName not in sys.modules: return False

  package = sys.modules[packageName]

  errMsg = QCoreApplication.translate("Python", "Couldn't load plugin %s" ) % packageName

  start = time.clock()
  # create an instance of the plugin
  try:
    plugins[packageName] = package.classFactory(iface)
  except:
    _unloadPluginModules(packageName)
    msg = QCoreApplication.translate("Python", "%s due an error when calling its classFactory() method") % errMsg
    showException(sys.exc_type, sys.exc_value, sys.exc_traceback, msg)
    return False

  # initGui
  try:
    plugins[packageName].initGui()
  except:
    del plugins[packageName]
    _unloadPluginModules(packageName)
    msg = QCoreApplication.translate("Python", "%s due an error when calling its initGui() method" ) % errMsg
    showException(sys.exc_type, sys.exc_value, sys.exc_traceback, msg)
    return False

  # add to active plugins
  active_plugins.append(packageName)
  end = time.clock()
  plugin_times[packageName] = "{0:02f}s".format(end - start)

  return True


def canUninstallPlugin(packageName):
  """ confirm that the plugin can be uninstalled """
  global plugins, active_plugins

  if not plugins.has_key(packageName): return False
  if packageName not in active_plugins: return False

  try:
    metadata = plugins[packageName]
    if "canBeUninstalled" not in dir(metadata):
      return True
    return bool(metadata.canBeUninstalled())
  except:
    msg = "Error calling "+packageName+".canBeUninstalled"
    showException(sys.exc_type, sys.exc_value, sys.exc_traceback, msg)
    return True


def unloadPlugin(packageName):
  """ unload and delete plugin! """
  global plugins, active_plugins

  if not plugins.has_key(packageName): return False
  if packageName not in active_plugins: return False

  try:
    plugins[packageName].unload()
    del plugins[packageName]
    active_plugins.remove(packageName)
    _unloadPluginModules(packageName)
    return True
  except Exception, e:
    msg = QCoreApplication.translate("Python", "Error while unloading plugin %s") % packageName
    showException(sys.exc_type, sys.exc_value, sys.exc_traceback, msg)
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

  if not plugins.has_key(packageName): return False
  return (packageName in active_plugins)


def reloadPlugin(packageName):
  """ unload and start again a plugin """
  global active_plugins
  if packageName not in active_plugins:
    return # it's not active

  unloadPlugin(packageName)
  loadPlugin(packageName)
  startPlugin(packageName)


def showPluginHelp(packageName=None,filename="index",section=""):
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
  helpfile = os.path.join(path,filename+"-"+locale+".html")
  if not os.path.exists(helpfile):
    helpfile = os.path.join(path,filename+"-"+locale.split("_")[0]+".html")
  if not os.path.exists(helpfile):
    helpfile = os.path.join(path,filename+"-en.html")
  if not os.path.exists(helpfile):
    helpfile = os.path.join(path,filename+"-en_US.html")
  if not os.path.exists(helpfile):
    helpfile = os.path.join(path,filename+".html")
  if os.path.exists(helpfile):
    url = "file://"+helpfile
    if section != "":
        url = url + "#" + section
    iface.openURL(url,False)


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
  exec unicode(code) in mod.__dict__
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


def qgsfunction(args, group, **kwargs):
  """
  Decorator function used to define a user expression function.

  Custom functions should take (values, feature, parent) as args,
  they can also shortcut naming feature and parent args by using *args
  if they are not needed in the function.

  Functions should return a value compatible with QVariant

  Eval errors can be raised using parent.setEvalErrorString()

  Functions must be unregistered when no longer needed using
  QgsExpression.unregisterFunction

  Example:
    @qgsfunction(2, 'test'):
    def add(values, feature, parent):
      pass

    Will create and register a function in QgsExpression called 'add' in the
    'test' group that takes two arguments.

    or not using feature and parent:

    @qgsfunction(2, 'test'):
    def add(values, *args):
      pass
  """
  helptemplate = Template("""<h3>$name function</h3><br>$doc""")
  class QgsExpressionFunction(QgsExpression.Function):
    def __init__(self, name, args, group, helptext='', usesgeometry=False):
      QgsExpression.Function.__init__(self, name, args, group, helptext, usesgeometry)

    def func(self, values, feature, parent):
      pass

  def wrapper(func):
    name = kwargs.get('name', func.__name__)
    usesgeometry = kwargs.get('usesgeometry', False)
    help = func.__doc__ or ''
    help = help.strip()
    if args == 0 and not name[0] == '$':
      name = '${0}'.format(name)
    func.__name__ = name
    help = helptemplate.safe_substitute(name=name, doc=help)
    f = QgsExpressionFunction(name, args, group, help, usesgeometry)
    f.func = func
    register = kwargs.get('register', True)
    if register:
      QgsExpression.registerFunction(f)
    return f
  return wrapper

#######################
# IMPORT wrapper

import __builtin__

_builtin_import = __builtin__.__import__
_plugin_modules = { }

def _import(name, globals={}, locals={}, fromlist=[], level=-1):
  """ wrapper around builtin import that keeps track of loaded plugin modules """
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

__builtin__.__import__ = _import
