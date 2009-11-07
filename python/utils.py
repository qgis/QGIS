"""
QGIS utilities module

"""

from PyQt4.QtCore import QCoreApplication
import sys
import traceback


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

def pluginMetadata(packageName, fct):
  """ fetch metadata from a plugin """
  try:
    package = sys.modules[packageName]
    return getattr(package, fct)()
  except:
    return "__error__"

def loadPlugin(packageName):
  """ load plugin's package and ensure that plugin is reloaded when changed """

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
    reload(packageName)
    return True
  except:
    msgTemplate = QCoreApplication.translate("Python", "Couldn't load plugin '%1' from ['%2']")
    msg = msgTemplate.arg(packageName).arg("', '".join(sys.path))
    showException(sys.exc_type, sys.exc_value, sys.exc_traceback, msg)
    return False


def startPlugin(packageName):
  """ initialize the plugin """
  global plugins, iface

  package = sys.modules[packageName]

  errMsg = QCoreApplication.translate("Python", "Couldn't load plugin %1" ).arg(packageName)

  # create an instance of the plugin
  try:
    plugins[packageName] = package.classFactory(iface)
  except:
    msg = QCoreApplication.translate("Python", "%1 due an error when calling its classFactory() method").arg(errMsg)
    showException(sys.exc_type, sys.exc_value, sys.exc_traceback, msg)
    return False

  # initGui
  try:
    plugins[packageName].initGui()
  except:
    msg = QCoreApplication.translate("Python", "%1 due an error when calling its initGui() method" ).arg( errMsg )
    showException(sys.exc_type, sys.exc_value, sys.exc_traceback, msg)
    return False

  return True


def unloadPlugin(packageName):
  """ unload and delete plugin! """
  global plugins

  try:
    plugins[packageName].unload()
    del plugins[packageName]
    return True
  except Exception, e:
    errMsg = QCoreApplication.translate("Python", "Error while unloading plugin %1").arg(packageName)
    showException(sys.exc_type, sys.exc_value, sys.exc_traceback, msg)
    return False
