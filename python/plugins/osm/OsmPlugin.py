"""@package OsmPlugin
This is the main module of the OSM Plugin.

It shows/hides all tool buttons, widgets and dialogs.

After closing dialogs it does all actions related with their return codes.

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""


from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.QtNetwork import *
from qgis.core import *

from OsmLoadDlg import OsmLoadDlg
from OsmSaveDlg import OsmSaveDlg
from OsmDownloadDlg import OsmDownloadDlg
from OsmUploadDlg import OsmUploadDlg
from OsmImportDlg import OsmImportDlg
from OsmFeatureDW import *
from OsmUndoRedoDW import *

# initialize Qt resources from file resouces.py
import resources_rc



def versionNumber():
    """Returns current version number of OpenStreetMap plugin.

    @return current version number of the plugin
    """
    return "0.5"



class OsmPlugin:
    """OsmPlugin is the main class OSM Plugin module.

    It shows/hides all tool buttons, widgets and dialogs and after closing dialogs
    it does all actions related with their return codes.
    """

    def __init__(self, iface):
        """The constructor.

        @param iface QgisInterface object
        """

        self.iface=iface
        self.canvas=self.iface.mapCanvas()
        self.http=QHttp()
        self.outFile=None
        self.httpGetId=0
        self.httpRequestAborted=False
        self.fname=""


    def initGui(self):
        """Function initalizes GUI of the OSM Plugin.
        """

        self.dockWidgetVisible = False

        # create action for loading OSM file
        self.actionLoad=QAction(QIcon(":/plugins/osm_plugin/images/osm_load.png")
            ,QCoreApplication.translate( "OsmPlugin", "Load OSM from file"), self.iface.mainWindow())
        self.actionLoad.setWhatsThis( QCoreApplication.translate( "OsmPlugin", "Load OpenStreetMap from file") )
        # create action for import of a layer into OSM
        self.actionImport=QAction(QIcon(":/plugins/osm_plugin/images/osm_import.png")
            ,QCoreApplication.translate( "OsmPlugin", "Import data from a layer"), self.iface.mainWindow())
        self.actionImport.setWhatsThis(QCoreApplication.translate( "OsmPlugin", "Import data from a layer to OpenStreetMap") )
        # create action for saving OSM file
        self.actionSave=QAction(QIcon(":/plugins/osm_plugin/images/osm_save.png")
            ,QCoreApplication.translate( "OsmPlugin", "Save OSM to file"), self.iface.mainWindow())
        self.actionSave.setWhatsThis(QCoreApplication.translate( "OsmPlugin", "Save OpenStreetMap to file") )
        # create action for OSM data downloading
        self.actionDownload=QAction(QIcon(":/plugins/osm_plugin/images/osm_download.png")
            ,QCoreApplication.translate( "OsmPlugin", "Download OSM data"), self.iface.mainWindow())
        self.actionDownload.setWhatsThis(QCoreApplication.translate( "OsmPlugin", "Download OpenStreetMap data") )
        # create action for OSM data downloading
        self.actionUpload=QAction(QIcon(":/plugins/osm_plugin/images/osm_upload.png")
            ,QCoreApplication.translate( "OsmPlugin", "Upload OSM data"), self.iface.mainWindow())
        self.actionUpload.setWhatsThis(QCoreApplication.translate( "OsmPlugin", "Upload OpenStreetMap data") )
        # create action for OSM dockable window
        self.actionDockWidget=QAction(QIcon(":/plugins/osm_plugin/images/osm_featureManager.png")
            ,QCoreApplication.translate( "OsmPlugin", "Show/Hide OSM Feature Manager"),self.iface.mainWindow())
        self.actionDockWidget.setWhatsThis(QCoreApplication.translate( "OsmPlugin", "Show/Hide OpenStreetMap Feature Manager") )
        self.actionDockWidget.setCheckable(True)

        # connect new action to plugin function - when action is triggered
        QObject.connect(self.actionLoad, SIGNAL("triggered()"), self.loadOsmFromFile)
        QObject.connect(self.actionSave, SIGNAL("triggered()"), self.saveOsmToFile)
        QObject.connect(self.actionDownload, SIGNAL("triggered()"), self.downloadOsmData)
        QObject.connect(self.actionUpload, SIGNAL("triggered()"), self.uploadOsmData)
        QObject.connect(self.actionDockWidget, SIGNAL("triggered()"), self.showHideDockWidget)
        QObject.connect(self.actionImport, SIGNAL("triggered()"), self.importData)

        # create a toolbar
        self.toolBar=self.iface.addToolBar("OpenStreetMap")
        self.toolBar.setObjectName("OpenStreetMap")
        self.toolBar.addAction(self.actionLoad)
        self.toolBar.addAction(self.actionDockWidget)
        self.toolBar.addAction(self.actionDownload)
        self.toolBar.addAction(self.actionUpload)
        self.toolBar.addAction(self.actionImport)
        self.toolBar.addAction(self.actionSave)

        # populate plugins menu
        self.iface.addPluginToMenu("&OpenStreetMap", self.actionLoad)
        self.iface.addPluginToMenu("&OpenStreetMap", self.actionDockWidget)
        self.iface.addPluginToMenu("&OpenStreetMap", self.actionDownload)
        self.iface.addPluginToMenu("&OpenStreetMap", self.actionUpload)
        self.iface.addPluginToMenu("&OpenStreetMap", self.actionImport)
        self.iface.addPluginToMenu("&OpenStreetMap", self.actionSave)

        # create manager of sqlite database(-s)
        self.dbm=OsmDatabaseManager(self)

        self.undoredo=None
        self.dockWidget=None

        # create widget for undo/redo actions
        self.undoredo=OsmUndoRedoDW(self)
        self.iface.addDockWidget(Qt.LeftDockWidgetArea,self.undoredo)
        self.undoredo.hide()
        QObject.connect(self.undoredo,SIGNAL("visibilityChanged(bool)"),self.__urVisibilityChanged)
        self.undoredo.setContentEnabled(False)

        # create widget for osm feature info
        self.dockWidget=OsmFeatureDW(self)
        self.iface.addDockWidget(Qt.RightDockWidgetArea, self.dockWidget)
        QObject.connect(self.dockWidget,SIGNAL("visibilityChanged(bool)"),self.__ofVisibilityChanged)
        self.dockWidget.setContentEnabled(False)


    def unload(self):
        """Function unloads the OSM Plugin.
        """

        self.dbm.disconnectSignals()
        self.canvas.unsetMapTool(self.dockWidget.mapTool)
        del self.dockWidget.mapTool
        self.dockWidget.mapTool=None

        # remove the plugin menu items
        self.iface.removePluginMenu("&OpenStreetMap",self.actionLoad)
        self.iface.removePluginMenu("&OpenStreetMap",self.actionSave)
        self.iface.removePluginMenu("&OpenStreetMap",self.actionDownload)
        self.iface.removePluginMenu("&OpenStreetMap",self.actionUpload)
        self.iface.removePluginMenu("&OpenStreetMap",self.actionImport)
        self.iface.removePluginMenu("&OpenStreetMap",self.actionDockWidget)

        self.dockWidget.close()
        if self.dockWidget.rubBand:
            self.dockWidget.rubBand.reset(False)
        if self.dockWidget.rubBandPol:
            self.dockWidget.rubBandPol.reset(True)

        self.undoredo.clear()
        self.undoredo.close()

        self.iface.removeDockWidget(self.dockWidget)
        self.iface.removeDockWidget(self.undoredo)

        del self.dockWidget
        del self.undoredo
        self.dockWidget=None
        self.undoredo=None

        # remove toolbar
        del self.toolBar

        # w/o osm plugin we don't need osm layers
        self.dbm.removeAllOsmLayers()


    def loadOsmFromFile(self):
        """Function shows up the "Load OSM from file" dialog.

        After closing it, function calls the appropriate actions
        according to dialog's return code.
        """

        # sanity check whether we're able to load osm data
        if 'osm' not in QgsProviderRegistry.instance().providerList():
            QMessageBox.critical(None, QCoreApplication.translate( "OsmPlugin", "Sorry" ),
              QCoreApplication.translate( "OsmPlugin", "You don't have OSM provider installed!") )
            return

        # show modal dialog with OSM file selection
        self.dlgLoad=OsmLoadDlg(self)

        # continue only if OK button was clicked
        if self.dlgLoad.exec_()==0:
            return

        self.fname=self.dlgLoad.OSMFileEdit.text()
        self.dbFileName=self.fname+".db"
        self.dbm.addDatabase(self.dbFileName,self.dlgLoad.pointLayer,self.dlgLoad.lineLayer,self.dlgLoad.polygonLayer)
        self.undoredo.clear()

        self.dockWidget.setContentEnabled(True)
        self.undoredo.setContentEnabled(True)

        self.dataLoaded=True


    def saveOsmToFile(self):
        """Function shows up the "Save OSM to file" dialog.

        After closing it, function calls the appropriate actions
        according to dialog's return code.
        """

        if 'osm' not in QgsProviderRegistry.instance().providerList():
            QMessageBox.critical(None, QCoreApplication.translate( "OsmPlugin", "Sorry" ),
              QCoreApplication.translate( "OsmPlugin", "You don't have OSM provider installed!") )
            return

        if not self.dbm.currentKey:
            QMessageBox.information(QWidget(), QCoreApplication.translate( "OsmPlugin", "OSM Save to file"),
                QCoreApplication.translate( "OsmPlugin", "No OSM data are loaded/downloaded or no OSM layer is selected in Layers panel. \
Please change this situation first, because OSM Plugin doesn't know what to save.") )
            return

        # show modal dialog with OSM file selection
        self.dlgSave=OsmSaveDlg(self)

        # continue only if OK button was clicked
        if self.dlgSave.exec_()==0:
            return


    def downloadOsmData(self):
        """Function shows up the "Download OSM data" dialog.

        After closing it, function calls the appropriate actions
        according to dialog's return code.
        """

        if 'osm' not in QgsProviderRegistry.instance().providerList():
            QMessageBox.critical(None, QCoreApplication.translate( "OsmPlugin", "Sorry" ),
              QCoreApplication.translate( "OsmPlugin", "You don't have OSM provider installed!") )
            return

        self.dlgDownload=OsmDownloadDlg(self)
        self.dlgDownload.exec_()
        if not self.dlgDownload.httpSuccess:
            return

        if not self.dlgDownload.autoLoadCheckBox.isChecked():
            return

        # create loading dialog, submit it
        self.dlgLoad=OsmLoadDlg(self)
        self.dlgLoad.setModal(True)
        self.dlgLoad.show()
        self.dlgLoad.close()
        self.dlgLoad.OSMFileEdit.setText(self.dlgDownload.destdirLineEdit.text())
        self.dlgLoad.styleCombo.setCurrentIndex(self.dlgDownload.styleCombo.currentIndex())

        if self.dlgDownload.chkCustomRenderer.isChecked():
            self.dlgLoad.chkCustomRenderer.setChecked(True)
        else:
            self.dlgLoad.chkCustomRenderer.setChecked(False)

        for row in xrange(self.dlgLoad.lstTags.count()):
            self.dlgLoad.lstTags.item(row).setCheckState(Qt.Checked)

        if self.dlgDownload.chkReplaceData.isChecked():
            self.dlgLoad.chkReplaceData.setChecked(True)
        else:
            self.dlgLoad.chkReplaceData.setChecked(False)

        self.dlgLoad.onOK()

        self.fname=self.dlgLoad.OSMFileEdit.text()
        self.dbFileName=self.fname+".db"
        self.dbm.addDatabase(self.dbFileName,self.dlgLoad.pointLayer,self.dlgLoad.lineLayer,self.dlgLoad.polygonLayer)


    def uploadOsmData(self):
        """Function shows up the "Upload OSM data" dialog.

        After closing it, function calls the appropriate actions
        according to dialog's return code.
        """

        if 'osm' not in QgsProviderRegistry.instance().providerList():
            QMessageBox.critical(None, "Sorry", "You don't have OSM provider installed!")
            return

        # first check if there are some data; if not upload doesn't have sense
        if not self.dbm.currentKey:
            QMessageBox.information(QWidget(), QCoreApplication.translate( "OsmPlugin", "OSM Upload"),
                QCoreApplication.translate( "OsmPlugin", "No OSM data are loaded/downloaded or no OSM layer is selected in Layers panel. \
Please change this situation first, because OSM Plugin doesn't know what to upload.") )
            return

        self.dlgUpload=OsmUploadDlg(self)
        self.dlgUpload.exec_()


    def importData(self):
        """Function shows up the "Import OSM data" dialog.

        After closing it, function calls the appropriate actions
        according to dialog's return code.
        """

        if 'osm' not in QgsProviderRegistry.instance().providerList():
            QMessageBox.critical(None, QCoreApplication.translate( "OsmPlugin", "Sorry"),
              QCoreApplication.translate( "OsmPlugin", "You don't have OSM provider installed!") )
            return

        if self.dbm.currentKey is None:
            QMessageBox.information(self.iface.mainWindow(), QCoreApplication.translate( "OsmPlugin", "OSM Import"),
                QCoreApplication.translate( "OsmPlugin", "No OSM data are loaded/downloaded or no OSM layer is selected in Layers panel. \
Please change this situation first, because OSM Plugin doesn't know what layer will be destination of the import.") )
            return

        dlg=OsmImportDlg(self)
        if dlg.cboLayer.count()==0:
            QMessageBox.information(self.iface.mainWindow(), QCoreApplication.translate( "OsmPlugin", "OSM Import"),
              QCoreApplication.translate( "OsmPlugin", "There are currently no available vector layers.") )
            return

        dlg.exec_()


    def showHideDockWidget(self):
        """Function shows/hides main dockable widget of the plugin ("OSM Feature" widget)
        """

        if self.dockWidget.isVisible():
            self.dockWidget.hide()
        else:
            self.dockWidget.show()


    def __urVisibilityChanged(self):
        """Function is called after visibilityChanged(...) signal is emitted on OSM Edit History widget.

        Function changes state of related checkbox according to the fact
        if widget is currently visible of not.
        """

        if self.undoredo.isVisible():
            self.dockWidget.urDetailsButton.setChecked(True)
        else:
            self.dockWidget.urDetailsButton.setChecked(False)


    def __ofVisibilityChanged(self):
        """Function is called after visibilityChanged(...) signal is emitted on OSM Feature widget.

        Function changes state of appropriate tool button according to the fact
        if widget is currently visible of not.
        """

        if self.dockWidget.isVisible():
            self.actionDockWidget.setChecked(True)
        else:
            self.actionDockWidget.setChecked(False)
