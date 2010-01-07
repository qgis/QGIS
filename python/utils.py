# -*- coding: utf-8 -*-
"""
QGIS utilities module

"""

from PyQt4.QtCore import QCoreApplication
import sys
import traceback
import glob
import os.path
import re


#######################
# ERROR HANDLING

def showException(type, value, tb, msg):
  lst = traceback.format_exception(type, value, tb)
  if msg == None:
    msg = QCoreApplication.translate('Python', 'An error has occured while executing Python code:')
  txt = '<font color="red">%s</font><br><br>' % msg
  for s in lst:
    txt += s
  txt += '<br>%s<br>%s<br><br>' % (QCoreApplication.translate('Python','Python version:'), sys.version)
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
# CONSOLE OUTPUT

old_stdout = sys.stdout
console_output = None

# hook for python console so all output will be redirected
# and then shown in console
def console_displayhook(obj):
  global console_output
  console_output = obj

class QgisOutputCatcher:
  def __init__(self):
    self.data = ''
  def write(self, stuff):
    self.data += stuff
  def get_and_clean_data(self):
    tmp = self.data
    self.data = ''
    return tmp
  def flush(self):
    pass
  

def installConsoleHooks():
  sys.displayhook = console_displayhook
  sys.stdout = QgisOutputCatcher()

def uninstallConsoleHooks():
  sys.displayhook = sys.__displayhook__
  sys.stdout = old_stdout


#######################
# PLUGINS

# dictionary of plugins
plugins = {}

# list of active (started) plugins
active_plugins = []

# list of plugins in plugin directory and home plugin directory
available_plugins = []


def updateAvailablePlugins():
  from qgis.core import QgsApplication
  pythonPath = unicode(QgsApplication.pkgDataPath()) + "/python"
  homePythonPath = unicode(QgsApplication.qgisSettingsDirPath()) + "/python"

  plugins = map(os.path.basename, glob.glob(pythonPath + "/plugins/*"))
  homePlugins = map(os.path.basename, glob.glob(homePythonPath + "/plugins/*"))

  # merge the lists
  for p in homePlugins:
    if p not in plugins:
      plugins.append(p)

  global available_plugins
  available_plugins = plugins

# update list on start
updateAvailablePlugins()


def pluginMetadata(packageName, fct):
  """ fetch metadata from a plugin """
  try:
    package = sys.modules[packageName]
    return getattr(package, fct)()
  except:
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
    msgTemplate = QCoreApplication.translate("Python", "Couldn't load plugin '%1' from ['%2']")
    msg = msgTemplate.arg(packageName).arg("', '".join(sys.path))
    showException(sys.exc_type, sys.exc_value, sys.exc_traceback, msg)
    return False


def startPlugin(packageName):
  """ initialize the plugin """
  global plugins, active_plugins, iface

  if packageName in active_plugins: return False

  package = sys.modules[packageName]

  errMsg = QCoreApplication.translate("Python", "Couldn't load plugin %1" ).arg(packageName)

  # create an instance of the plugin
  try:
    plugins[packageName] = package.classFactory(iface)
  except:
    _unloadPluginModules(packageName)
    msg = QCoreApplication.translate("Python", "%1 due an error when calling its classFactory() method").arg(errMsg)
    showException(sys.exc_type, sys.exc_value, sys.exc_traceback, msg)
    return False

  # initGui
  try:
    plugins[packageName].initGui()
  except:
    del plugins[packageName]
    _unloadPluginModules(packageName)
    msg = QCoreApplication.translate("Python", "%1 due an error when calling its initGui() method" ).arg( errMsg )
    showException(sys.exc_type, sys.exc_value, sys.exc_traceback, msg)
    return False

  # add to active plugins
  active_plugins.append(packageName)

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
    msg = QCoreApplication.translate("Python", "Error while unloading plugin %1").arg(packageName)
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
    if "resources" in mod:
      try:
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
    
  return mod

__builtin__.__import__ = _import
