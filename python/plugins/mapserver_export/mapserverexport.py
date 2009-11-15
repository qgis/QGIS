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

  # ----------------------------------------- #
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
    #self.action.setWhatsThis("Configuration for Zoom To Point plugin")
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
    
    # TODO: should come from settings
    self.dlg.ui.txtMapFilePath.setText('/usr/lib/cgi-bin/kaas.map')

    # project defaults to current instance
    project = QgsProject.instance()
    
    # question: save project on loading export dialog?
    if project.isDirty():
      shouldSave = QMessageBox.question(self.dlg,
                      "Save?",
                      "Save project to \"" + project.fileName() + "\" before exporting? Only the last saved version of your project will be exported.",
                      QMessageBox.Yes,
                      QMessageBox.No,
                      QMessageBox.Cancel
                      )
      if shouldSave == QMessageBox.Yes:
        if project.fileName().size() == 0:
          # project has not yet been saved:
          saveAsFileName = QFileDialog.getSaveFileName(self.dlg,
                      "Save QGIS Project file as...",
                      ".",
                      "QGIS Project Files (*.qgs)",
                      "Filter list for selecting files from a dialog box")
          # Check that a file was selected
          if saveAsFileName.size() == 0:
            QMessageBox.warning(self.dlg, "Not saved!", "QGis project file not saved because no file name was given.")
            # fall back to using current project if available
            self.dlg.ui.txtQgisFilePath.setText(project.fileName())
          else:
            project.setFileName(saveAsFileName)
            project.write()
        else:
          project.write()
      elif shouldSave == QMessageBox.Cancel:
        return # do not show the export dialog

    self.dlg.ui.txtQgisFilePath.setText(project.fileName())
    
    # set title, or set default one if none available
    title = project.title()
    if title == "":
      title = "QGisGeneratedMapfile"
    self.dlg.ui.txtMapName.setText(project.title())
    
    if self.dlg.ui.txtMapFilePath.text().size() == 0:
      btnOk = self.dlg.ui.buttonBox.button(QDialogButtonBox.Ok)
      btnOk.setEnabled(False)
      
    # TODO: fetch unit used from QSettings

    # TODO: fetch width/height guess from QSettings:
    # fetch the last used values from settings and intialize the
    # dialog with them
    #settings = QSettings("MicroResources", "ZoomToPoint")
    #xValue = settings.value("coordinate/x")
    #self.dlg.ui.xCoord.setText(str(xValue.toString()))
    #yValue = settings.value("coordinate/y")
    #self.dlg.ui.yCoord.setText(str(yValue.toString()))
    #scale = settings.value("zoom/scale", QVariant(4))
    #self.dlg.ui.spinBoxScale.setValue(scale.toInt()[0])

    QObject.connect(self.dlg.ui.btnChooseFile, SIGNAL("clicked()"), self.setSaveFile)
    QObject.connect(self.dlg.ui.txtMapFilePath, SIGNAL("textChanged(QString)"), self.mapfileChanged)
    QObject.connect(self.dlg.ui.btnChooseProjectFile, SIGNAL("clicked()"), self.setProjectFile)
    QObject.connect(self.dlg.ui.chkExpLayersOnly, SIGNAL("clicked(bool)"), self.toggleLayersOnly)
    QObject.connect(self.dlg.ui.checkBoxCurrentProject, SIGNAL("clicked(bool)"), self.toggleUseCurrentProject)
    QObject.connect(self.dlg.ui.btnChooseFooterFile, SIGNAL("clicked()"), self.setFooterFile)
    QObject.connect(self.dlg.ui.btnChooseHeaderFile, SIGNAL("clicked()"), self.setHeaderFile)
    QObject.connect(self.dlg.ui.btnChooseTemplateFile, SIGNAL("clicked()"), self.setTemplateFile)
    QObject.connect(self.dlg.ui.buttonBox, SIGNAL("accepted()"), self.ok_clicked)
	
    self.dlg.show()

  def ok_clicked(self):
    # Check if map file name is provided
    if self.dlg.ui.txtMapFilePath.text().size() == 0:
      saveAsFileName = QFileDialog.getSaveFileName(self.dlg,
                    "Please choose to save map file as...",
                    ".",
                    "Map files (*.map)",
                    "Filter list for selecting files from a dialog box")
      # Check that a file was selected
      if saveAsFileName.size() == 0:
        QMessageBox.warning(self.dlg, "Not saved!", "Map file not saved because no file name was given")
        return
      else:
        self.dlg.ui.txtMapFilePath.setText(saveAsFileName)
        self.saveMapFile()

    # Check if map file exists and we should overwrite it
    elif QFile(self.dlg.ui.txtMapFilePath.text()).exists():
      shouldOverwrite = QMessageBox.question(self.dlg,
                    "Overwrite?",
                    "Map file \"" + self.dlg.ui.txtMapFilePath.text() + "\" already exists. \nShould we overwrite it?",
                    QMessageBox.Yes,
                    QMessageBox.Cancel
                    )
      if shouldOverwrite == QMessageBox.Yes:
        self.saveMapFile()
      elif shouldOverwrite == QMessageBox.Cancel:
        return
        
    else:
      self.saveMapFile()

  def toggleUseCurrentProject(self, boolUseCurrent):
    self.dlg.ui.txtQgisFilePath.setEnabled(not boolUseCurrent)
    self.dlg.ui.btnChooseProjectFile.setEnabled(not boolUseCurrent)
    if boolUseCurrent:
      if self.dlg.ui.txtQgisFilePath.text().size() == 0:
        # reload path of current project
        self.dlg.ui.txtQgisFilePath.setText(QgsProject.instance().fileName())
        # check if current project is saved and/or dirty? Nope: will be done when Ok clicked
    else:  
      # open dialog to choose project file
      self.setProjectFile()
    
    

  def saveMapFile(self):
    # get the settings from the dialog and export the map file  
    print "Creating exporter using %s and %s" % (self.dlg.ui.txtQgisFilePath.text(), self.dlg.ui.txtMapFilePath.text())
    if self.dlg.ui.txtQgisFilePath.text().size() == 0:
      saveAsFileName = QFileDialog.getSaveFileName(self.dlg,
                    "Please choose to save QGis project file as...",
                    ".",
                    "QGis files (*.qgs)",
                    "Filter list for selecting files from a dialog box")
      # Check that a file was selected
      if saveAsFileName.size() == 0:
        QMessageBox.warning(self.dlg, "Not saved!", "QGis project file not saved because no file name was given")
        return
      else:
        self.dlg.ui.txtQgisFilePath.setText(saveAsFileName)
        
    exporter = Qgis2Map(unicode(self.dlg.ui.txtMapFilePath.text()))
    
    # Parse qgis project file and check success
    if not(exporter.setQgsProject(self.dlg.ui.txtQgisFilePath.text())):
      QMessageBox.warning(self.dlg, "Not saved!", "File not saved because no valid qgis project file was given.")
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
      
    QMessageBox.information(self.dlg, "MapServer Export Results", result)

  def mapfileChanged(self, text):
    # Enable OK button
    btnOk = self.dlg.ui.buttonBox.button(QDialogButtonBox.Ok)
    if text.size() > 0:
      btnOk.setEnabled(True)
    else:
      btnOk.setEnabled(False)      

  def setSaveFile(self):
    mapFile = QFileDialog.getSaveFileName(self.dlg, "Name for the map file", \
      ".", "MapServer map files (*.map);;All files (*.*)","Filter list for selecting files from a dialog box")
    self.dlg.ui.txtMapFilePath.setText(mapFile)
      
  def setProjectFile(self):
    qgisProjectFile = QFileDialog.getOpenFileName(self.dlg, "Choose a QGIS Project file", \
      ".", "QGIS Project Files (*.qgs);;All files (*.*)", "Filter list for selecting files from a dialog box")
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
