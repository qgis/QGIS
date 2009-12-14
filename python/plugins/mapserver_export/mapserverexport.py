"""
/***************************************************************************
        MapServerExport  - A QGIS plugin to export a saved project file
                            to a MapServer map file
                             -------------------
    begin                : 2008-01-07
    copyright            : (C) 2008 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* Adapted by Erik van de Pol */
"""
# Import the PyQt and QGIS libraries
from PyQt4.QtCore import * 
from PyQt4.QtGui import *
from xml.dom import minidom
from qgis.core import *
# Initialize Qt resources from file resources.py
import resources_rc
# Import the code for the dialog
from mapserverexportdialog import MapServerExportDialog
# Import the ms_export script that does the real work
from ms_export import *

class MapServerExport: 

  def __init__(self, iface):
    # Save reference to the QGIS interface
    self.iface = iface

  def setCurrentTheme(self, theThemeName):
    # Set icons to the current theme
    self.action.setIcon(self.getThemeIcon("mapserver_export.png"))

  def getThemeIcon(self, theName):
    # get the icon from the best available theme
    myCurThemePath = QgsApplication.activeThemePath() + "/plugins/" + theName;
    myDefThemePath = QgsApplication.defaultThemePath() + "/plugins/" + theName;
    myQrcPath = ":/plugins/mapserver_export/" + theName;
    if QFile.exists(myCurThemePath):
      return QIcon(myCurThemePath)
    elif QFile.exists(myDefThemePath):
      return QIcon(myDefThemePath)
    elif QFile.exists(myQrcPath):
      return QIcon(myQrcPath)
    else:
      return QIcon()

  def initGui(self):  
    # Create action that will start plugin configuration
    self.action = QAction(self.getThemeIcon("mapserver_export.png"), \
        "MapServer Export", self.iface.mainWindow())
    # connect the action to the run method
    QObject.connect(self.action, SIGNAL("activated()"), self.run) 
    QObject.connect(self.iface, SIGNAL("currentThemeChanged ( QString )"), self.setCurrentTheme)
    # Add toolbar button and menu item
    self.iface.addToolBarIcon(self.action)
    self.iface.addPluginToMenu("&MapServer Export...", self.action)

  def unload(self):
    # Remove the plugin menu item and icon
    self.iface.removePluginMenu("&MapServer Export...",self.action)
    self.iface.removeToolBarIcon(self.action)

  # run method that performs all the real work
  def run(self): 
    # create and show the MapServerExport dialog 
    self.dlg = MapServerExportDialog()
    # attach events to inputs and buttons
    QObject.connect(self.dlg.ui.btnChooseFile, SIGNAL("clicked()"), self.setMapFile)
    QObject.connect(self.dlg.ui.txtMapFilePath, SIGNAL("textChanged(QString)"), self.mapfileChanged)
    QObject.connect(self.dlg.ui.btnChooseProjectFile, SIGNAL("clicked()"), self.setProjectFile)
    QObject.connect(self.dlg.ui.chkExpLayersOnly, SIGNAL("clicked(bool)"), self.toggleLayersOnly)
    QObject.connect(self.dlg.ui.checkBoxCurrentProject, SIGNAL("clicked(bool)"), self.toggleUseCurrentProject)
    QObject.connect(self.dlg.ui.btnChooseFooterFile, SIGNAL("clicked()"), self.setFooterFile)
    QObject.connect(self.dlg.ui.btnChooseHeaderFile, SIGNAL("clicked()"), self.setHeaderFile)
    QObject.connect(self.dlg.ui.btnChooseTemplateFile, SIGNAL("clicked()"), self.setTemplateFile)
    QObject.connect(self.dlg.ui.buttonBox, SIGNAL("accepted()"), self.ok_clicked)
    QObject.connect(self.dlg.ui.buttonBox, SIGNAL("helpRequested()"), self.helpRequested)
    # qgs-project defaults to current instance
    project = QgsProject.instance()
    self.dlg.ui.txtQgisFilePath.setText(project.fileName())
    # get some settings from former successfull exports
    # defaults are defined in ms_export.py and set in mapserverexportdialog.py
    settings = QSettings()
    # map-file name and force mapfileChanged to enable/disable ok button
    self.dlg.ui.txtMapFilePath.setText(settings.value("/MapserverExport/mapfileName", QVariant("")).toString ()) 
    self.mapfileChanged(self.dlg.ui.txtMapFilePath.text())
    # map width and height
    if settings.contains("/MapserverExport/mapWidth"):
      self.dlg.ui.txtMapWidth.setText(settings.value("/MapserverExport/mapWidth").toString ())
    if settings.contains("/MapserverExport/mapHeight"):
      self.dlg.ui.txtMapHeight.setText(settings.value("/MapserverExport/mapHeight").toString ())
    # MapServer IMAGETYPE's [gif|png|jpeg|wbmp|gtiff|swf|userdefined]
    self.dlg.ui.cmbMapImageType.addItems(QStringList(["agg","png","gif","jpeg","wbmp","gtiff","swf","userdefined"]))
    if settings.contains("/MapserverExport/imageType"):
      idx = self.dlg.ui.cmbMapImageType.findText(settings.value("/MapserverExport/imageType").toString ())
      self.dlg.ui.cmbMapImageType.setCurrentIndex(idx)
    # MapServer URL (default value already set by dialog defaults)
    if settings.contains("/MapserverExport/mapserverUrl"):
      self.dlg.ui.txtMapServerUrl.setText(settings.value("/MapserverExport/mapserverUrl").toString())
    # set title or default to one if none available
    title = project.title()
    if title == "":
      title = "QGIS-MAP"
    self.dlg.ui.txtMapName.setText(title)
    # TODO: fetch units used from current project
    # QGIS: Meters, Feet, Degrees, UnknownUnit since 1.4 also: DecimalDegrees, DegreesMinutesSeconds, DegreesDecimalMinutes 	
    # Mapserver: UNITS [feet|inches|kilometers|meters|miles|dd]
    self.dlg.show()

  def helpRequested(self):
    QgsContextHelp.run( "MapServerExport" )

  def ok_clicked(self):
    # check if current project is saved or dirty
    if not self.checkCurrentProject():
      # abort because user apparently did not wat to save or Cancelled
      return
    if not self.checkMapFile():
      print "Failed for Map file check, try again..."
      return
    else:
      self.saveMapFile()

  def toggleUseCurrentProject(self, boolUseCurrent):
    self.dlg.ui.txtQgisFilePath.setEnabled(not boolUseCurrent)
    self.dlg.ui.btnChooseProjectFile.setEnabled(not boolUseCurrent)
    if boolUseCurrent:
      #if self.dlg.ui.txtQgisFilePath.text().size() == 0:
      # reload path of current project
      self.dlg.ui.txtQgisFilePath.setText(QgsProject.instance().fileName())
    else:  
      # open dialog to choose project file
      self.setProjectFile()
    

  def saveMapFile(self):   
    # get the settings from the dialog and export the map file
    print "Creating exporter using '%s' and '%s'" % (self.dlg.ui.txtQgisFilePath.text(), self.dlg.ui.txtMapFilePath.text())
    exporter = Qgis2Map(unicode(self.dlg.ui.txtMapFilePath.text()))
    # Parse qgis project file and check success
    if not(exporter.setQgsProject(self.dlg.ui.txtQgisFilePath.text())):
      QMessageBox.warning(self.dlg, "No Map file export!", "Map file not exported because no valid qgis project file was given.")
      return
    self.dlg.hide()
    print "Setting options"
    exporter.setOptions(
        unicode(self.dlg.ui.txtMapServerUrl.text()),
        unicode(self.dlg.ui.cmbMapUnits.itemData( self.dlg.ui.cmbMapUnits.currentIndex() ).toString()),
        unicode(self.dlg.ui.cmbMapImageType.currentText()),
        unicode(self.dlg.ui.txtMapName.text()),
        unicode(self.dlg.ui.txtMapWidth.text()),
        unicode(self.dlg.ui.txtMapHeight.text()),
        unicode(self.dlg.ui.txtWebTemplate.text()),
        unicode(self.dlg.ui.txtWebHeader.text()),
        unicode(self.dlg.ui.txtWebFooter.text()),
        self.dlg.ui.checkBoxDump.isChecked(),
        self.dlg.ui.checkBoxForce.isChecked(),
        self.dlg.ui.checkBoxAntiAlias.isChecked(),
        self.dlg.ui.checkBoxPartials.isChecked(),
        self.dlg.ui.chkExpLayersOnly.isChecked(),
        unicode(self.dlg.ui.txtFontsetPath.text()),
        unicode(self.dlg.ui.txtSymbolsetPath.text())
    )
    print "Calling writeMapFile"
    try:
      result = exporter.writeMapFile()
    except Exception, err:
      QMessageBox.information(self.dlg, "MapServer Export Error", str(err))
      return
    # ok succesfull: write some setting for a next session
    settings = QSettings()
    # mapfile name 
    settings.setValue("/MapserverExport/mapfileName", QVariant(self.dlg.ui.txtMapFilePath.text()))
    # map width and heigth
    settings.setValue("/MapserverExport/mapWidth", QVariant(self.dlg.ui.txtMapWidth.text()))
    settings.setValue("/MapserverExport/mapHeight", QVariant(self.dlg.ui.txtMapHeight.text()))
    # mapserver url
    settings.setValue("/MapserverExport/mapserverUrl", QVariant(self.dlg.ui.txtMapServerUrl.text()))
    # map ImageType
    settings.setValue("/MapserverExport/imageType", QVariant(self.dlg.ui.cmbMapImageType.currentText()))
    # show results
    QMessageBox.information(self.dlg, "MapServer Export Results", result)

  def mapfileChanged(self, text):
    # Enable OK button
    btnOk = self.dlg.ui.buttonBox.button(QDialogButtonBox.Ok)
    if text.size() > 0:
      btnOk.setEnabled(True)
    else:
      btnOk.setEnabled(False)      

  def checkMapFile(self):
    # Check if map file name is provided
    mapFileName = self.dlg.ui.txtMapFilePath.text()
    if mapFileName.size() == 0:
      QMessageBox.warning(self.dlg, "Not saved!", "Map file not saved because no file name was given")
      return False
    # Check/fix for .map extension (mapserver fails to read otherwise)
    if not mapFileName.trimmed().endsWith('.map'):
      mapFileName += '.map'
    self.dlg.ui.txtMapFilePath.setText(mapFileName)
    # Check if map file exists and we should overwrite it
    if QFile(mapFileName).exists():
      if QMessageBox.Cancel == QMessageBox.question(self.dlg, "Overwrite excisting Map file?",
                    "Map file \"" + mapFileName + "\" already exists. \nShould we overwrite it?",
                    QMessageBox.Yes, QMessageBox.Cancel):
        return False
    # mapfile ok, extension ok, overwrite  ok
    return True
    
  # check if current project is saved and or dirty (has modifications)
  def checkCurrentProject(self, forUnload=False):
    project = QgsProject.instance()
    # question: save project on loading export dialog?
    if project.isDirty():
      msg = "Save project to \"" + project.fileName() + "\" before exporting?\nOnly the last saved version of your project will be exported."
      if project.fileName()=="":
        msg = "Please save project before exporting.\nOnly saved projects can be exported."
      if forUnload:
        msg = "Save project first?\nAfter saving, this project will be unloaded."
      shouldSave = QMessageBox.question(self.dlg, "Save?", msg,
                    QMessageBox.Yes, QMessageBox.No, QMessageBox.Cancel)
      if shouldSave == QMessageBox.Yes:
        if project.fileName().size() == 0:
          # project has not yet been saved:
          saveAsFileName = QFileDialog.getSaveFileName(self.dlg,
                      "Save QGIS Project file as...", ".",
                      "QGIS Project Files (*.qgs)", "Filter list for selecting files from a dialog box")
          # Check that a file was selected
          if saveAsFileName.size() == 0:
            QMessageBox.warning(self.dlg, "Not saved!", "QGis project file not saved because no file name was given.")
            # fall back to using current project if available
            self.dlg.ui.txtQgisFilePath.setText(project.fileName())
          else:
            if not saveAsFileName.trimmed().endsWith('.qgs'):
              saveAsFileName += '.qgs'
            self.dlg.ui.txtQgisFilePath.setText(saveAsFileName)
            project.setFileName(saveAsFileName)
            project.write()
        else:
          project.write()
        #project ok now
        return True
      elif shouldSave == QMessageBox.No and forUnload:
        # unloading a non saved project: just leave ...
        return True
      elif shouldSave == QMessageBox.No:
        # users does not want to save project, but has to because only saved projects can be exported
        return False
      elif shouldSave == QMessageBox.Cancel:
        # user cancelled
        return False
    else: 
      # project saved and not dirty
      return True
      
  def setMapFile(self):
    mapFileName = QFileDialog.getSaveFileName(self.dlg, "Name for the map file", \
      self.dlg.ui.txtMapFilePath.text(), "MapServer map files (*.map);;All files (*.*)","Filter list for selecting files from a dialog box")
    self.dlg.ui.txtMapFilePath.setText(mapFileName)

  def setProjectFile(self):
    # check if it's needed to save current project, will return False if user cancelled
    if not self.checkCurrentProject(True):
      return
    qgisProjectFile = QFileDialog.getOpenFileName(self.dlg, "Choose a QGIS Project", \
      ".", "QGIS Project Files (*.qgs);;", "Filter list for selecting files from a dialog box")
    if not qgisProjectFile:
      # cancelled: check checkBoxCurrentProject again 
      self.dlg.ui.checkBoxCurrentProject.setChecked(True)
      self.dlg.ui.txtQgisFilePath.setEnabled(False)
      return
    try:
      # reading a nog qgs or not existing file results in qgis crash
      # QgsProject.instance().read(QFileInfo(qgisProjectFile))
      # we try to open the file first to see if it can be parsed...
      exporter = Qgis2Map(unicode(self.dlg.ui.txtMapFilePath.text()))
      if exporter.setQgsProject(qgisProjectFile):
        # project file OK !!
        pass
      if exporter.projectHasPostgisLayers():
        loadProject = QMessageBox.question(self.dlg, "Load project?", 
            "The project you selected holds one or more postgis layers. \nTo be able to export a valid DATA string in the map file,\nthe project should be loaded into QGIS. \nNot loading can result in non valid DATA strings in map file.\nSo, should we load it into qgis?",
            QMessageBox.Yes, QMessageBox.No)
        if loadProject == QMessageBox.Yes:
          QgsProject.instance().read(QFileInfo(qgisProjectFile))
        else:
          # postgis, but user refuses to load it, just go and see what happens
          pass
      else:
        # NO postgis, go
        pass
    except QgsException, err:
      QMessageBox.information(self.dlg, "Error reading or loading the selected project file", str(err))
      self.dlg.ui.checkBoxCurrentProject.setChecked(True)
      self.dlg.ui.txtQgisFilePath.setEnabled(False)
      return
    self.dlg.ui.txtQgisFilePath.setText(qgisProjectFile)

  def setTemplateFile(self):
    templateFile = QFileDialog.getOpenFileName(self.dlg, "Choose the MapServer template file", \
      ".", "All files (*.*)", "Filter list for selecting files from a dialog box")
    self.dlg.ui.txtWebTemplate.setText(templateFile)

  def setHeaderFile(self):
    headerFile = QFileDialog.getOpenFileName(self.dlg, "Choose the MapServer header file", \
      ".", "All files (*.*)",  "Filter list for selecting files from a dialog box")
    self.dlg.ui.txtWebHeader.setText(headerFile)

  def setFooterFile(self):
    footerFile = QFileDialog.getOpenFileName(self.dlg, "Choose the MapServer footer file", \
        ".", "All files (*.*)", "Filter list for selecting files from a dialog box")
    self.dlg.ui.txtWebFooter.setText(footerFile)
  
  def toggleLayersOnly(self, isChecked):
    # disable other sections if only layer export is desired
    self.dlg.ui.grpPaths.setEnabled(not isChecked)
    self.dlg.ui.grpMap.setEnabled(not isChecked)
