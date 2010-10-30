# -*- coding: utf-8 -*-
"""@package OsmSaveDlg
This module is used to save OSM data into XML file.

Of course, user is asked where to save the current data first.
"""


from ui_OsmSaveDlg import Ui_OsmSaveDlg

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.QtXml import *
from sip import unwrapinstance
from qgis.core import QgsVectorLayer, QgsMapLayerRegistry

import sqlite3



class OsmSaveDlg(QDialog, Ui_OsmSaveDlg):
    """This class provides all structures and methods necessary for current OSM data saving.

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

        # variables for identifiers of all objects that will be saved to output file
        self.nodeIds=set()

        # connecting dialog and progressbar signals
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

        # close the whole Save OSM dialog
        self.close()


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

        It performs all actions necessary for OSM data saving.
        """

        # prepare data

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

        c=self.dbm.getConnection().cursor()

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

        # <bounds> element
        dataExtent=self.plugin.canvas.extent()
        self.xml.writeEmptyElement("bounds")
        self.xml.writeAttribute("minlat",str(dataExtent.yMinimum()))
        self.xml.writeAttribute("minlon",str(dataExtent.xMinimum()))
        self.xml.writeAttribute("maxlat",str(dataExtent.yMaximum()))
        self.xml.writeAttribute("maxlon",str(dataExtent.xMaximum()))

        # todo: uid and changeset attributes are not compulsory! support for them in future!

        if points:
            self.progressDialog.setLabelText(self.tr("Saving nodes..."))
            self.progressDialog.setMaximum(cntPoints)
            self.progressDialog.setValue(0)
            i=0

            c.execute("select n.id,n.lat,n.lon,v.version_id,n.user,n.timestamp from node n, version v \
                       where n.status<>'R' and n.u=1 and v.object_id=n.id and v.object_type='node' \
                       and n.lat>=:minLat AND n.lat<=:maxLat AND n.lon>=:minLon AND n.lon<=:maxLon"
                       ,{"minLat":dataExtent.yMinimum(),"maxLat":dataExtent.yMaximum()
                        ,"minLon":dataExtent.xMinimum(),"maxLon":dataExtent.xMaximum()})

            for (nid,lat,lon,ver,usr,tms) in c:
                anyTags=False
                tagList=[]

                self.nodeIds.add(nid)

                if tags:
                    tagList=self.dbm.getFeatureTags(nid,'Point')
                    if len(tagList)>0:
                        anyTags=True

                if anyTags:
                    self.xml.writeStartElement("node")
                else:
                    self.xml.writeEmptyElement("node")

                self.xml.writeAttribute("id",str(nid))
                self.xml.writeAttribute("lat",str(lat))
                self.xml.writeAttribute("lon",str(lon))
                self.xml.writeAttribute("version",str(ver))
                if usr<>"":
                    self.xml.writeAttribute("user",usr)
                self.xml.writeAttribute("visible","true")
                self.xml.writeAttribute("timestamp",tms)

                if anyTags:
                    for r in tagList:
                        self.xml.writeEmptyElement("tag")
                        self.xml.writeAttribute("k",r[0])
                        self.xml.writeAttribute("v",r[1])
                    self.xml.writeEndElement()

                i=i+1
                self.progressDialog.setValue(i)

        if lines:
            self.progressDialog.setLabelText(self.tr("Saving lines..."))
            self.progressDialog.setMaximum(cntLines)
            self.progressDialog.setValue(0)
            i=0

            c.execute("select w.id,v.version_id,w.user,w.timestamp from way w,version v \
                       where w.closed=0 and w.status<>'R' and w.u=1 and v.object_id=w.id and v.object_type='way' \
                       and (((w.max_lat between :minLat and :maxLat) or (w.min_lat between :minLat and :maxLat) or (w.min_lat<:minLat and w.max_lat>:maxLat)) \
                         and ((w.max_lon between :minLon and :maxLon) or (w.min_lon between :minLon and :maxLon) or (w.min_lon<:minLon and w.max_lon>:maxLon)))"
                       ,{"minLat":dataExtent.yMinimum(),"maxLat":dataExtent.yMaximum()
                        ,"minLon":dataExtent.xMinimum(),"maxLon":dataExtent.xMaximum()})

            for (lid,ver,usr,tms) in c:

                geom=self.dbm.getFeatureGeometry(lid,'Line')
                if not geom.intersects(dataExtent):
                    continue

                self.xml.writeStartElement("way")
                self.xml.writeAttribute("id",str(lid))
                self.xml.writeAttribute("visible","true")
                self.xml.writeAttribute("timestamp",tms)
                self.xml.writeAttribute("version",str(ver))
                if usr<>"":
                    self.xml.writeAttribute("user",usr)

                d=self.dbm.getConnection().cursor()
                d.execute("select node_id from way_member where way_id=:wayId",{"wayId":lid})
                for r in d:
                    self.xml.writeEmptyElement("nd")
                    self.xml.writeAttribute("ref",str(r[0]))
                d.close()

                if tags:
                    tagList=self.dbm.getFeatureTags(lid,'Line')
                    for r in tagList:
                        self.xml.writeEmptyElement("tag")
                        self.xml.writeAttribute("k",r[0])
                        self.xml.writeAttribute("v",r[1])

                self.xml.writeEndElement()

                d=self.dbm.getConnection().cursor()
                d.execute("select node_id from way_member where way_id=:wayId",{"wayId":lid})
                for r in d:
                    if r[0] not in self.nodeIds:

                        e=self.dbm.getConnection().cursor()
                        e.execute("select n.id,n.lat,n.lon,v.version_id,n.user,n.timestamp from node n, version v \
                                   where n.id=:nid",{"nid":r[0]})
                        for nodeRec in e:
                            nid=nodeRec[0]
                            lat=nodeRec[1]
                            lon=nodeRec[2]
                            ver=nodeRec[3]
                            usr=nodeRec[4]
                            tms=nodeRec[5]
                        e.close()

                        anyTags=False
                        tagList=[]
                        self.nodeIds.add(nid)

                        if tags:
                            tagList=self.dbm.getFeatureTags(nid,'Point')
                            if len(tagList)>0:
                                anyTags=True

                        if anyTags:
                            self.xml.writeStartElement("node")
                        else:
                            self.xml.writeEmptyElement("node")

                        self.xml.writeAttribute("id",str(nid))
                        self.xml.writeAttribute("lat",str(lat))
                        self.xml.writeAttribute("lon",str(lon))
                        self.xml.writeAttribute("version",str(ver))
                        if usr<>"":
                            self.xml.writeAttribute("user",usr)
                        self.xml.writeAttribute("visible","true")
                        self.xml.writeAttribute("timestamp",tms)

                        if anyTags:
                            for r in tagList:
                                self.xml.writeEmptyElement("tag")
                                self.xml.writeAttribute("k",r[0])
                                self.xml.writeAttribute("v",r[1])
                            self.xml.writeEndElement()

                d.close()
                i=i+1
                self.progressDialog.setValue(i)

        if polys:
            self.progressDialog.setLabelText(self.tr("Saving polygons..."))
            self.progressDialog.setMaximum(cntPolys)
            self.progressDialog.setValue(0)
            i=0

            c.execute("select w.id,v.version_id,w.user,w.timestamp from way w,version v \
                       where w.closed=1 and w.status<>'R' and w.u=1 and v.object_id=w.id and v.object_type='way' \
                       and (((w.max_lat between :minLat and :maxLat) or (w.min_lat between :minLat and :maxLat) or (w.min_lat<:minLat and w.max_lat>:maxLat)) \
                         and ((w.max_lon between :minLon and :maxLon) or (w.min_lon between :minLon and :maxLon) or (w.min_lon<:minLon and w.max_lon>:maxLon)))"
                       ,{"minLat":dataExtent.yMinimum(),"maxLat":dataExtent.yMaximum()
                        ,"minLon":dataExtent.xMinimum(),"maxLon":dataExtent.xMaximum()})

            for (pid,ver,usr,tms) in c:

                geom=self.dbm.getFeatureGeometry(pid,'Polygon')
                if not geom.intersects(dataExtent):
                    continue

                self.xml.writeStartElement("way")
                self.xml.writeAttribute("id",str(pid))
                self.xml.writeAttribute("visible","true")
                self.xml.writeAttribute("timestamp",tms)
                self.xml.writeAttribute("version",str(ver))
                if usr<>"":
                    self.xml.writeAttribute("user",usr)

                d=self.dbm.getConnection().cursor()
                d.execute("select node_id from way_member where way_id=:wayId",{"wayId":pid})
                first=None
                for r in d:
                    self.xml.writeEmptyElement("nd")
                    self.xml.writeAttribute("ref",str(r[0]))
                    if first==None:
                        first=r[0]
                d.close()
                self.xml.writeEmptyElement("nd")
                self.xml.writeAttribute("ref",str(first))

                if tags:
                    tagList=self.dbm.getFeatureTags(pid,'Polygon')
                    for r in tagList:
                        self.xml.writeEmptyElement("tag")
                        self.xml.writeAttribute("k",r[0])
                        self.xml.writeAttribute("v",r[1])

                self.xml.writeEndElement()

                d=self.dbm.getConnection().cursor()
                d.execute("select node_id from way_member where way_id=:wayId",{"wayId":pid})
                for r in d:
                    if r[0] not in self.nodeIds:

                        e=self.dbm.getConnection().cursor()
                        e.execute("select n.id,n.lat,n.lon,v.version_id,n.user,n.timestamp from node n, version v \
                                   where n.id=:nid",{"nid":r[0]})
                        for nodeRec in e:
                            nid=nodeRec[0]
                            lat=nodeRec[1]
                            lon=nodeRec[2]
                            ver=nodeRec[3]
                            usr=nodeRec[4]
                            tms=nodeRec[5]
                        e.close()

                        anyTags=False
                        tagList=[]
                        self.nodeIds.add(nid)

                        if tags:
                            tagList=self.dbm.getFeatureTags(nid,'Point')
                            if len(tagList)>0:
                                anyTags=True

                        if anyTags:
                            self.xml.writeStartElement("node")
                        else:
                            self.xml.writeEmptyElement("node")

                        self.xml.writeAttribute("id",str(nid))
                        self.xml.writeAttribute("lat",str(lat))
                        self.xml.writeAttribute("lon",str(lon))
                        self.xml.writeAttribute("version",str(ver))
                        if usr<>"":
                            self.xml.writeAttribute("user",usr)
                        self.xml.writeAttribute("visible","true")
                        self.xml.writeAttribute("timestamp",tms)

                        if anyTags:
                            for r in tagList:
                                self.xml.writeEmptyElement("tag")
                                self.xml.writeAttribute("k",r[0])
                                self.xml.writeAttribute("v",r[1])
                            self.xml.writeEndElement()

                d.close()
                i=i+1
                self.progressDialog.setValue(i)

        if rels:
            self.progressDialog.setLabelText(self.tr("Saving relations..."))
            self.progressDialog.setMaximum(cntRels)
            self.progressDialog.setValue(0)
            i=0

            c.execute("select r.id,v.version_id,r.user,r.timestamp from relation r,version v \
                       where r.status<>'R' and r.u=1 and v.object_id=r.id and v.object_type='relation' \
                       and ( \
                           exists ( \
                               select 1 from node n, relation_member rm \
                               where rm.relation_id=r.id and n.status<>'R' and n.u=1 and rm.member_id=n.id and rm.member_type='node' \
                               and n.lat>=:minLat and n.lat<=:maxLat and n.lon>=:minLon and n.lon<=:maxLon ) \
                           or exists ( \
                               select 1 from way w, relation_member rm \
                               where rm.relation_id=r.id and w.status<>'R' and w.u=1 and rm.member_id=w.id and rm.member_type='way' \
                               and (((w.max_lat between :minLat and :maxLat) or (w.min_lat between :minLat and :maxLat) or (w.min_lat<:minLat and w.max_lat>:maxLat)) \
                                 and ((w.max_lon between :minLon and :maxLon) or (w.min_lon between :minLon and :maxLon) or (w.min_lon<:minLon and w.max_lon>:maxLon))) \
                                  ))"
                       ,{"minLat":dataExtent.yMinimum(),"maxLat":dataExtent.yMaximum()
                        ,"minLon":dataExtent.xMinimum(),"maxLon":dataExtent.xMaximum()})

            for (rid,ver,usr,tms) in c:

                self.xml.writeStartElement("relation")
                self.xml.writeAttribute("id",str(rid))
                self.xml.writeAttribute("visible","true")
                self.xml.writeAttribute("timestamp",tms)
                self.xml.writeAttribute("version",str(ver))
                if usr<>"":
                    self.xml.writeAttribute("user",usr)

                d=self.dbm.getConnection().cursor()
                d.execute("select member_id,member_type,role from relation_member where relation_id=:relId",{"relId":rid})
                for r in d:
                    self.xml.writeEmptyElement("member")
                    self.xml.writeAttribute("type",r[1])
                    self.xml.writeAttribute("ref",str(r[0]))
                    self.xml.writeAttribute("role",r[2])
                d.close()

                if tags:
                    tagList=self.dbm.getFeatureTags(rid,'Relation')
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



