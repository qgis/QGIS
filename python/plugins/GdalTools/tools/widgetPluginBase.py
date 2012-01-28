# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from dialogBase import GdalToolsBaseDialog as BaseDialog
import GdalTools_utils as Utils

class GdalToolsBasePluginWidget:

  def __init__(self, iface, commandName):
      self.iface = iface
      self.initialized = False
      self.base = BaseDialog( iface.mainWindow(), iface, self, self.windowTitle(), commandName)

      self.connect(self.base, SIGNAL("processError(QProcess::ProcessError)"), self.onError)
      self.connect(self.base, SIGNAL("processFinished(int, QProcess::ExitStatus)"), self.onFinished)

      self.connect(self.base, SIGNAL("okClicked()"), self.onRun)
      self.connect(self.base, SIGNAL("closeClicked()"), self.onClosing)
      self.connect(self.base, SIGNAL("helpClicked()"), self.onHelp)

      self.connect(self.base, SIGNAL("finished(bool)"), self.finished)
      self.connect(self.base, SIGNAL("refreshArgs()"), self.someValueChanged)

  def someValueChanged(self):
      self.emit(SIGNAL("valuesChanged(const QStringList &)"), self.getArguments())

  def onLayersChanged(self):
      pass

  def exec_(self):
      if not self.initialized:
        self.connect(Utils.LayerRegistry.instance(), SIGNAL("layersChanged"), self.onLayersChanged)
        self.onLayersChanged()
        self.someValueChanged()
      self.initialized = True
      return self.base.exec_()

  def show_(self):
      if not self.initialized:
        self.connect(Utils.LayerRegistry.instance(), SIGNAL("layersChanged"), self.onLayersChanged)
        self.onLayersChanged()
        self.someValueChanged()
      self.initialized = True
      return self.base.show()

  def setCommandViewerEnabled(self, enable):
      self.base.setCommandViewerEnabled(enable)
      self.someValueChanged()

  def onRun(self):
      self.base.onRun()

  def onClosing(self):
      self.disconnect(Utils.LayerRegistry.instance(), SIGNAL("layersChanged"), self.onLayersChanged)
      self.base.onClosing()

  def onHelp(self):
      self.base.onHelp()

  def onFinished(self, exitCode, status):
      self.base.onFinished(exitCode, status)

  def onError(self, error):
      self.base.onError(error)

  def getArguments(self):
      pass

  def getInputFileName(self):
      pass

  def getOutputFileName(self):
      pass

  def addLayerIntoCanvas(self, fileInfo):
      pass

  def finished(self, load):
      outFn = self.getOutputFileName()
      if outFn == None:
        return

      outFn = QString(outFn)
      if outFn.isEmpty():
        QMessageBox.warning(self, self.tr( "Warning" ), self.tr( "No output file created." ) ) 
        return

      fileInfo = QFileInfo(outFn)
      if fileInfo.exists():
        if load:
          self.addLayerIntoCanvas(fileInfo)
        QMessageBox.information(self, self.tr( "Finished" ), self.tr( "Processing completed." ) )
      else:
        QMessageBox.warning(self, self.tr( "Warning" ), self.tr( "%1 not created." ).arg( outFn ) )

  # This method is useful to set up options for the command. It sets for each passed widget:
  # 1. its passed signals to connect to the BasePluginWidget.someValueChanged() slot, 
  # 2. its enabler checkbox or enabled status, 
  # 3. its status as visible (hide) if the installed gdal version is greater or equal (lesser) then the passed version
  #
  # wdgts_sgnls_chk_ver_list: list of wdgts_sgnls_chk_ver
  #     wdgts_sgnls_chk_ver: tuple containing widgets, signals, enabler checkbox or enabled status, required version
  def setParamsStatus(self, wdgts_sgnls_chk_ver_list):
      if isinstance(wdgts_sgnls_chk_ver_list, list):
        for wdgts_sgnls_chk_ver in wdgts_sgnls_chk_ver_list:
          self.setParamsStatus(wdgts_sgnls_chk_ver)
        return

      wdgts_sgnls_chk_ver = wdgts_sgnls_chk_ver_list
      if not isinstance(wdgts_sgnls_chk_ver, tuple):
        return

      if len(wdgts_sgnls_chk_ver) > 0:
        wdgts = wdgts_sgnls_chk_ver[0]
      else:
        wdgts = None

      if len(wdgts_sgnls_chk_ver) > 1:
        sgnls = wdgts_sgnls_chk_ver[1]
      else:
        sgnls = None

      if len(wdgts_sgnls_chk_ver) > 2:
        chk = wdgts_sgnls_chk_ver[2]
      else:
        chk = None

      if len(wdgts_sgnls_chk_ver) > 3:
        ver = wdgts_sgnls_chk_ver[3]
      else:
        ver = None

      if isinstance(wdgts, list):
        for wdgt in wdgts:
          self.setParamsStatus((wdgt, sgnls, chk, ver))
        return

      wdgt = wdgts
      if not isinstance(wdgt, QWidget):
        return

      # if check version fails, disable the widget then hide both it and its enabler checkbox
      if ver != None:
        if not isinstance(ver, Utils.Version):
          ver = Utils.Version(ver)
        gdalVer = Utils.GdalConfig.version()
        if ver < "0" or ( gdalVer != None and ver > gdalVer ):
          wdgt.setVisible(False)
          if isinstance(chk, QWidget):
            chk.setVisible(False)
            chk.setChecked(False)
          sgnls = None
          chk = False

      # connects the passed signals to the BasePluginWidget.someValueChanged slot
      if isinstance(sgnls, list):
        for sgnl in sgnls:
          self.setParamsStatus((wdgt, sgnl, chk))
        return

      sgnl = sgnls
      if sgnl != None:
        self.connect(wdgt, sgnl, self.someValueChanged)

      # set the passed checkbox as widget enabler
      if isinstance(chk, bool):
        wdgt.setEnabled(chk)
      if ( isinstance(chk, QAbstractButton) or isinstance(chk, QGroupBox) ) and \
           chk.isCheckable():
        wdgt.setEnabled(chk.isChecked())
        self.connect(chk, SIGNAL("toggled(bool)"), wdgt.setEnabled)
        self.connect(chk, SIGNAL("toggled(bool)"), self.someValueChanged)
