# -*- coding: utf-8 -*-
"""@package OsmSaveDlg
This module is used to save OSM data into XML file.

Of course, user is asked where to save the current data first.
"""


from OsmSaveDlg_ui import Ui_OsmSaveDlg

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.QtXml import *
from PyQt4 import *
from sip import unwrapinstance
from qgis.core import QgsVectorLayer, QgsMapLayerRegistry
#from sip import *

import sqlite3



class OsmSaveDlg(QDialog, Ui_OsmSaveDlg):
    """This class provides all structures and methods neccessary for current OSM data saving.

    Saving is done to XML file.
    After XML file selection and confirming the dialog, process is started.
    """

    def __init__(self, plugin):
        """The constructor.

        @param plugin is pointer to instance of OSM Plugin
        """

        QDialog.__init__(self, None)
        self.setupUi(self)

        self.plugin=plugin
        self.ur=plugin.undoredo
        self.dbm=plugin.dbm

        self.progressDialog = QProgressDialog(self)
        self.progressDialog.setModal(True)
        self.progressDialog.setAutoClose(False)

        QObject.connect(self.browseOSMButton,SIGNAL("clicked()"),self.showSaveFileDialog)
        QObject.connect(self.buttonBox,SIGNAL("accepted()"),self.onOK)
        QObject.connect(self.progressDialog, SIGNAL("canceled()"), self.cancelSaving)


    def cancelSaving(self):
        """Function stops the whole OSM Saving process.

        Destination file is closed and removed.
        """

        # writing into output file was canceled, file must be enclosed
        self.outFile.close()

        # end removed
        self.outFile.remove()
        self.outFile=None

#        if self.xml.device().isOpen():
#            self.xml.device().close()

        # close the whole Save OSM dialog
        self.close()

        # todo: segfault... why????


    def showSaveFileDialog(self):
        """Function opens dialog for selecting XML file.

        Only files with extension .osm can be selected.
        Default directory for file selection is remembered/reload.
        """

        settings = QSettings()
        lastDir = settings.value("/OSM_Plugin/lastDir", QVariant(QString())).toString()

        # display file open dialog and get absolute path to selected file
        fileSelected = QFileDialog.getSaveFileName(self,"Choose an Open Street Map file",lastDir,"OSM Files (*.osm)");
        # insert OSM file path into line edit control
        if not fileSelected.isNull():
            self.OSMFileEdit.setText(fileSelected)

            # remember directory
            fi = QFileInfo(fileSelected)
            settings.setValue("/OSM_Plugin/lastDir", QVariant(fi.path()) )


    def onOK(self):
        """Function is called after clicking on OK button of OSM Save dialog.

        It performs all actions neccessary for OSM data saving.
        """

        # after closing a dialog, we want to save data into osm
        self.fname=self.OSMFileEdit.text()
        self.outFile=QFile(self.fname)

        if not self.outFile.open(QIODevice.WriteOnly):
            QMessageBox.information(self,self.tr("Save OSM to file"),self.tr("Unable to save the file %1: %2.")
                               .arg(self.fname).arg(self.outFile.errorString()))
            self.outFile=None
            return

        points=self.chkPoints.isChecked()
        lines=self.chkLines.isChecked()
        polys=self.chkPolygons.isChecked()
        rels=self.chkRelations.isChecked()
        tags=self.chkTags.isChecked()

        self.xml=QXmlStreamWriter(self.outFile)
        self.xml.setCodec(QTextCodec.codecForName("utf-8"))
        self.xml.setAutoFormatting(True)

        self.dbConnection=sqlite3.connect(self.plugin.dbFileName.toLatin1().data())
        c=self.dbConnection.cursor()

        cntPoints=cntLines=cntPolys=cntRels=0
        c.execute("select count(*) from node")
        for rec in c:
            cntPoints=rec[0]
        c.execute("select count(*) from way where closed=0")
        for rec in c:
            cntLines=rec[0]
        c.execute("select count(*) from way where closed=1")
        for rec in c:
            cntPolys=rec[0]
        c.execute("select count(*) from relation")
        for rec in c:
            cntRels=rec[0]

        self.xml.writeStartDocument()
        self.xml.writeStartElement("osm")
        self.xml.writeAttribute("version","0.6")
        self.xml.writeAttribute("generator","OpenStreetMap server")

        self.progressDialog.setWindowTitle(self.tr("Save OSM to file"))
        self.progressDialog.setLabelText(self.tr("Initializing..."))
        self.progressDialog.setMaximum(1)
        self.progressDialog.setValue(0)
        self.progressDialog.show()

        # todo: <bounds> element?
        # todo: and what about uid? changeset? are they compulsory?

        if points:
            self.progressDialog.setLabelText(self.tr("Saving nodes..."))
            self.progressDialog.setMaximum(cntPoints)
            self.progressDialog.setValue(0)
            i=0

            c.execute("select n.id,n.lat,n.lon,v.version_id,n.user,n.timestamp from \
                       node n,version v where v.object_id=n.id and v.object_type='node'")
            for rec in c:
                anyTags=False
                tagList=[]

                if tags:
                    tagList=self.dbm.getFeatureTags(rec[0],'Point')
                    if len(tagList)>0:
                        anyTags=True

                if anyTags:
                    self.xml.writeStartElement("node")
                else:
                    self.xml.writeEmptyElement("node")

                self.xml.writeAttribute("id",str(rec[0]))
                self.xml.writeAttribute("lat",str(rec[1]))
                self.xml.writeAttribute("lon",str(rec[2]))
                self.xml.writeAttribute("version",str(rec[3]))
                self.xml.writeAttribute("user",rec[4])
                self.xml.writeAttribute("visible","true")
                self.xml.writeAttribute("timestamp",rec[5])

                if anyTags:
                    for r in tagList:
                        self.xml.writeEmptyElement("tag")
                        self.xml.writeAttribute("k",r[0])
                        self.xml.writeAttribute("v",r[1])

                if anyTags:
                    self.xml.writeEndElement()
                i=i+1
                self.progressDialog.setValue(i)

        if lines:
            self.progressDialog.setLabelText(self.tr("Saving lines..."))
            self.progressDialog.setMaximum(cntLines)
            self.progressDialog.setValue(0)
            i=0

            c.execute("select w.id,v.version_id,w.user,w.timestamp from way w,version v \
                       where w.closed=0 and v.object_id=w.id and v.object_type='way'")
            for rec in c:
                self.xml.writeStartElement("way")
                self.xml.writeAttribute("id",str(rec[0]))
                self.xml.writeAttribute("visible","true")
                self.xml.writeAttribute("timestamp",rec[3])
                self.xml.writeAttribute("version",str(rec[1]))
                self.xml.writeAttribute("user",rec[2])

                d=self.dbConnection.cursor()
                d.execute("select node_id from way_member where way_id=:wayId",{"wayId":rec[0]})
                for r in d:
                    self.xml.writeStartElement("nd")
                    self.xml.writeAttribute("ref",str(r[0]))
                    self.xml.writeEndElement()
                d.close()

                if tags:
                    tagList=self.dbm.getFeatureTags(rec[0],'Line')
                    for r in tagList:
                        self.xml.writeEmptyElement("tag")
                        self.xml.writeAttribute("k",r[0])
                        self.xml.writeAttribute("v",r[1])

                self.xml.writeEndElement()
                i=i+1
                self.progressDialog.setValue(i)

        if polys:
            self.progressDialog.setLabelText(self.tr("Saving polygons..."))
            self.progressDialog.setMaximum(cntPolys)
            self.progressDialog.setValue(0)
            i=0

            c.execute("select w.id,v.version_id,w.user,w.timestamp from way w,version v \
                       where w.closed=1 and v.object_id=w.id and v.object_type='way'")
            for rec in c:
                self.xml.writeStartElement("way")
                self.xml.writeAttribute("id",str(rec[0]))
                self.xml.writeAttribute("visible","true")
                self.xml.writeAttribute("timestamp",rec[3])
                self.xml.writeAttribute("version",str(rec[1]))
                self.xml.writeAttribute("user",rec[2])

                d=self.dbConnection.cursor()
                d.execute("select node_id from way_member where way_id=:wayId",{"wayId":rec[0]})
                for r in d:
                    self.xml.writeStartElement("nd")
                    self.xml.writeAttribute("ref",str(r[0]))
                    self.xml.writeEndElement()

                d.close()

                if tags:
                    tagList=self.dbm.getFeatureTags(rec[0],'Polygon')
                    for r in tagList:
                        self.xml.writeEmptyElement("tag")
                        self.xml.writeAttribute("k",r[0])
                        self.xml.writeAttribute("v",r[1])

                self.xml.writeEndElement()
                i=i+1
                self.progressDialog.setValue(i)

        if rels:
            self.progressDialog.setLabelText(self.tr("Saving relations..."))
            self.progressDialog.setMaximum(cntRels)
            self.progressDialog.setValue(0)
            i=0

            c.execute("select r.id,v.version_id,r.user,r.timestamp from relation r,version v \
                       where v.object_id=r.id and v.object_type='relation'")
            for rec in c:
                self.xml.writeStartElement("relation")
                self.xml.writeAttribute("id",str(rec[0]))
                self.xml.writeAttribute("visible","true")
                self.xml.writeAttribute("timestamp",rec[3])
                self.xml.writeAttribute("version",str(rec[1]))
                self.xml.writeAttribute("user",rec[2])

                d=self.dbConnection.cursor()
                d.execute("select member_id,member_type,role from relation_member where relation_id=:relId",{"relId":rec[0]})
                for r in d:
                    self.xml.writeStartElement("member")
                    self.xml.writeAttribute("type",r[1])
                    self.xml.writeAttribute("ref",str(r[0]))
                    self.xml.writeAttribute("role",r[2])
                    self.xml.writeEndElement()
                d.close()

                if tags:
                    tagList=self.dbm.getFeatureTags(rec[0],'Relation')
                    for r in tagList:
                        self.xml.writeEmptyElement("tag")
                        self.xml.writeAttribute("k",r[0])
                        self.xml.writeAttribute("v",r[1])

                self.xml.writeEndElement()
                i=i+1
                self.progressDialog.setValue(i)

        self.xml.writeEndElement()    # osm
        self.xml.writeEndDocument()

        c.close()
        self.disconnect(self.progressDialog, SIGNAL("canceled()"), self.cancelSaving)
        self.progressDialog.close()

        # writing into output file was finished, file can be enclosed
        if self.outFile and self.outFile.exists():
            self.outFile.close()
        self.close()



