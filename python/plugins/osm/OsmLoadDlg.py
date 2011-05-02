# -*- coding: utf-8 -*-
"""@package OsmLoadDlg
This module provides all structures and methods necessary for OSM data loading.

Loading is done from XML file. After XML file selection and confirming the dialog three vector layers
are created in Quantum GIS. Layer for points, one for lines and one for polygons.

All these layers are created with OSM data provider.
Data provider is the one, who parses an input XML file.

OSM data loading can be canceled, in such case system returns to the same state as the one before loading.
"""


from ui_OsmLoadDlg import Ui_OsmLoadDlg

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sip import unwrapinstance
from qgis.core import QgsVectorLayer, QgsMapLayerRegistry, QgsRectangle
from sip import *



class OsmLoadDlg(QDialog, Ui_OsmLoadDlg):
    """This class provides all structures and methods necessary for OSM data loading.

    Loading is done from XML file. After XML file selection and confirming the dialog three vector layers
    are created in Quantum GIS. Layer for points, one for lines and one for polygons.

    All these layers are created with OSM data provider.
    Data provider is the one, who parses an input XML file.

    OSM data loading can be canceled, in such case system returns to the same state as the one before loading.
    """

    def __init__(self, plugin):
        """The constructor.

        @param plugin is pointer to instance of OSM Plugin
        """

        QDialog.__init__(self, None)
        self.setupUi(self)

        self.canvas=plugin.canvas
        self.dbm=plugin.dbm
        self.progress = None

        # we must connect action "click on browse button" with method for showing open file dialog
        QObject.connect(self.browseOSMButton,SIGNAL("clicked()"),self.showOpenFileDialog)
        QObject.connect(self.buttonBox,SIGNAL("accepted()"), self.onOK)

        for tag in ['name','place','highway','landuse','waterway','railway','amenity','tourism','learning']:
            item = QListWidgetItem(tag, self.lstTags)
            item.setFlags( Qt.ItemIsSelectable | Qt.ItemIsUserCheckable | Qt.ItemIsEnabled)
            item.setCheckState(Qt.Checked if tag == 'name' else Qt.Unchecked)

        # gui initialization
        self.initGui()


    def initGui(self):
        """Initializes GUI of OSM Load dialog.
        """

        # load default values to combobox determining style for custom renderer
        self.styles=["Small scale","Medium scale","Large scale"]
        directory=QFileInfo(__file__).canonicalPath()
        self.styleFiles=[directory+"/styles/small_scale.style", directory+"/styles/medium_scale.style", directory+"/styles/big_scale.style"]
        self.styleCombo.addItems(self.styles)

        if not self.dbm.currentKey:
            self.chkReplaceData.setEnabled(False)


    def showOpenFileDialog(self):
        """Function opens dialog for selecting XML file.

        Only files with extension .osm can be selected.
        Default directory for file selection is remembered/reload.
        """

        settings=QSettings()
        lastDir=settings.value("/OSM_Plugin/lastDir", QVariant(QString())).toString()

        # display file open dialog and get absolute path to selected file
        fileSelected=QFileDialog.getOpenFileName(self,"Choose an Open Street Map file",lastDir,"OSM Files (*.osm)");
        # insert OSM file path into line edit control
        if not fileSelected.isNull():
            self.OSMFileEdit.setText(fileSelected)

            # save directory
            fi=QFileInfo(fileSelected)
            settings.setValue("/OSM_Plugin/lastDir", QVariant(fi.path()) )


    def onOK(self):
        """Function is called after clicking on OK button of OSM Load dialog.

        It performs all actions necessary for OSM data loading.
        It creates three QGIS vector layers for loaded data (point,line,polygon layer).
        """

        self.buttonBox.setEnabled(False)

        # after closing a dialog, we want to add map layers using osm provider
        self.fname = self.OSMFileEdit.text()

        if self.fname=='':
            QMessageBox.information(self, "OSM Load", QString("Please enter path to OSM data file."))
            self.buttonBox.setEnabled(True)
            return

        osmfile = QFileInfo(self.fname)
        observer = "&observer="+str(unwrapinstance(self))
        basename = osmfile.baseName()

        if not osmfile.exists():
            QMessageBox.information(self, "OSM Load", QString("Path to OSM file is invalid: %1.").arg(self.fname))
            return

        fLoaded=self.filesLoaded()
        replacing=self.chkReplaceData.isChecked()
        newDB=self.fname.toAscii().data()+".db"
        curDB=self.dbm.currentKey

        if basename in fLoaded and newDB<>curDB:
            QMessageBox.information(self, "Error", QString("Layers of OSM file \"%1\" are loaded already.").arg(self.fname))
            return

        if replacing: 
            # remove layers of current data first 
            QgsMapLayerRegistry.instance().removeMapLayer(self.canvas.currentLayer().id(),True)

        if self.chkCustomRenderer.isChecked():
            styleFile=self.styleFiles[self.styleCombo.currentIndex()]
            style="&style="+styleFile
        else:
            style=""

        # some specific tags?
        tag = ""
        for row in xrange(self.lstTags.count()):
            item = self.lstTags.item(row)
            if item.checkState() == Qt.Checked:
                if len(tag) > 0: tag += "+"
                tag += item.text()
        if len(tag) > 0:
            tag = "&tag=" + tag

        # freeze map canvas until all vector layers are created
        self.canvas.freeze(True)

        self.loadingCanceled=False
        self.setProperty("osm_stop_parsing",QVariant(0))

        # add polygon layer
        polygonLayer = QgsVectorLayer(self.fname+"?type=polygon"+observer + tag + style, basename+" polygons", "osm")

        if self.loadingCanceled:
            polygonLayer=None
            return
        if not polygonLayer.isValid():
            QMessageBox.information(self,"Error",QString("Failed to load polygon layer."))
            return

        if self.chkCustomRenderer.isChecked():
            self.setCustomRenderer(polygonLayer)
        QgsMapLayerRegistry.instance().addMapLayer(polygonLayer)

        # add line layer
        lineLayer = QgsVectorLayer(self.fname+"?type=line"+observer + tag + style, basename+" lines", "osm")

        if self.loadingCanceled:
            lineLayer=None
            return
        if not lineLayer.isValid():
            QMessageBox.information(self,"Error",QString("Failed to load line layer."))
            return

        if self.chkCustomRenderer.isChecked():
            self.setCustomRenderer(lineLayer)
        QgsMapLayerRegistry.instance().addMapLayer(lineLayer)

        # add point layer
        pointLayer = QgsVectorLayer(self.fname+"?type=point"+observer + tag + style, basename+" points", "osm")

        if self.loadingCanceled:
            pointLayer=None
            return
        if not pointLayer.isValid():
            QMessageBox.information(self,"Error",QString("Failed to load point layer."))
            return

        if self.chkCustomRenderer.isChecked():
            self.setCustomRenderer(pointLayer)
        QgsMapLayerRegistry.instance().addMapLayer(pointLayer)

        # remember layers
        self.polygonLayer=polygonLayer
        self.lineLayer=lineLayer
        self.pointLayer=pointLayer

        self.canvas.freeze(False)
        rect=self.canvas.extent()

        if self.chkCustomRenderer.isChecked():
            midX=rect.xMinimum()+(rect.xMaximum()-rect.xMinimum())/2
            midY=rect.yMinimum()+(rect.yMaximum()-rect.yMinimum())/2
            rX=rect.xMaximum()-midX
            rY=rect.yMaximum()-midY

            st=self.styles[self.styleCombo.currentIndex()]
            if st=="Small scale":
                rect=QgsRectangle(midX-rX/5,midY-rY/5,midX+rX/5,midY+rY/5)
            elif st=="Medium scale":
                rect=QgsRectangle(midX-rX/2,midY-rY/2,midX+rX/2,midY+rY/2)

        self.canvas.setExtent(rect)
        self.canvas.refresh()
        self.accept()


    def setCustomRenderer(self, layer):
        """Function provides a way how to set custom renderer.
        @param layer pointer to QGIS vector layer
        """

        if QObject.connect( self, SIGNAL( "setRenderer(QgsVectorLayer *)" ), layer.dataProvider(), SLOT( "setRenderer( QgsVectorLayer * )" ) ):
          self.emit( SIGNAL( "setRenderer(QgsVectorLayer *)" ), layer )
          QObject.disconnect( self, SIGNAL( "setRenderer(QgsVectorLayer *)" ), layer.dataProvider(), SLOT( "setRenderer( QgsVectorLayer * )" ) )
        else:
          QMessageBox.information(self, "OSM Load", QString("Could not connect to setRenderer signal."))

    def filesLoaded(self):
        """Function returns list of keys of all currently loaded vector layers.
        Note that names are not absolute and not unique.

        @return list of keys of all currently loaded vector layers
        """

        mapLayers=QgsMapLayerRegistry.instance().mapLayers()
        fLoaded=[]
        for ix in mapLayers.keys():
            fileName=QString(ix)
            pos=ix.lastIndexOf("_")
            fileName=fileName.left(pos)
            if fileName not in fLoaded:
                fLoaded.append(fileName)
        return fLoaded


    def cancelLoading(self):
        """Function is called when progress dialog is canceled

        It's purpose is to tell OSM provider to stop XML file parsing.
        """

        self.setProperty("osm_stop_parsing",QVariant(1))
        self.loadingCanceled=True


    def event(self, e):
        """Function is used for OSM provider <-> OSM Plugin communication.
        """

        if e.type() == QEvent.DynamicPropertyChange:
            if e.propertyName() == "osm_status":
                # we're starting new part
                if not self.progress:
                    self.progress = QProgressDialog(self)
                    self.progress.setAutoClose(False)
                    self.progress.setModal(True)
                    QObject.connect(self.progress,SIGNAL("canceled()"),self.cancelLoading)
                    self.progress.show()
                status = self.property("osm_status").toString()
                self.progress.setLabelText(status)
                self.progress.setValue(0)

            if e.propertyName() == "osm_max":
                if not self.loadingCanceled:
                    # we've got new max. value
                    osm_max = self.property("osm_max").toInt()[0]
                    self.progress.setMaximum(osm_max)

            elif e.propertyName() == "osm_value":
                if not self.loadingCanceled:
                    # update in progressbar
                    osm_val = self.property("osm_value").toInt()[0]
                    self.progress.setValue(osm_val)

            elif e.propertyName() == "osm_done":
                if not self.loadingCanceled:
                    # we're done
                    QObject.disconnect(self.progress,SIGNAL("canceled()"),self.cancelLoading)
                    self.progress.close()
                    self.progress = None

            elif e.propertyName() == "osm_failure":
                if not self.loadingCanceled:
                    self.loadingCanceled=True
                    QObject.disconnect(self.progress,SIGNAL("canceled()"),self.cancelLoading)
                    self.progress.close()
                    self.progress = None
                    QMessageBox.information(self,"Error",QString("Failed to load layers: %1")
                            .arg(self.property("osm_failure").toString()))
                    self.buttonBox.setEnabled(True)

            qApp.processEvents()
        return QDialog.event(self,e)


