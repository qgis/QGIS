"""@package DlgUploadOSM
Module provides simple way of uploading current OSM data.

When performing edit operations all changed features are marked as 'U'=Updated.
All new features are marked as 'A'=Added and all removed features are marked as 'R'=Removed.
Uploader checks features statuses first.

Then it creates HTTP connection. Upload is done in correct order
so that data on OSM server always stay consistent.

Upload phases and their exact order:
- phases: 0.changeset creation, 1.nodes creation, 2.ways deletion, 3.ways update,
          4.ways addition, 5.nodes deletion, 6.nodes update, 7.relation creation,
          8.relation deletion, 9.relation update, 10.changeset closing
"""


from DlgUploadOSM_ui import Ui_DlgUploadOSM

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.QtNetwork import *
from PyQt4 import *
from sys import *



class DlgUploadOSM(QDialog, Ui_DlgUploadOSM):
    """Class provides simple way of uploading current OSM data.

    When performing edit operations all changed features are marked as 'U'=Updated.
    All new features are marked as 'A'=Added and all removed features are marked as 'R'=Removed.

    Uploader checks features statuses first.
    Then it creates HTTP connection. Upload is done in correct order
    so that data on OSM server always stay consistent.

    Upload phases and their exact order:
    - phases: 0.changeset creation, 1.nodes creation, 2.ways deletion, 3.ways update,
              4.ways addition, 5.nodes deletion, 6.nodes update, 7.relation creation,
              8.relation deletion, 9.relation update, 10.changeset closing
    """

    def __init__(self,plugin):
        """The constructor.

        @param plugin is pointer to instance of OSM Plugin.
        """

        QDialog.__init__(self,None)
        self.setupUi(self)

        self.dockWidget=plugin.dockWidget
        self.plugin=plugin
        self.dbm=plugin.dbm
        self.ur=plugin.undoredo

        self.urlHost = "api.openstreetmap.org"
        self.uploadButton = self.buttonBox.addButton("Upload", QDialogButtonBox.ActionRole)

        self.uploadChangesTable.setColumnCount(5)
        self.uploadChangesTable.setColumnWidth(0,80)
        self.uploadChangesTable.setColumnWidth(1,60)
        self.uploadChangesTable.setColumnWidth(2,60)
        self.uploadChangesTable.setColumnWidth(3,60)
        self.uploadChangesTable.setColumnWidth(4,60)

        self.uploadChangesTable.setHeaderLabels(["","Points","Lines","Polygons","Relations"])
        key=QString(self.dbm.currentKey)
        p1=key.lastIndexOf(".")
        p2=key.lastIndexOf("/")
        key=key.mid(p2+1,p1-p2-1)
        self.groupBox.setTitle(QString("Changes in ").append(key))

        item = QTreeWidgetItem(["Added","0","0","0","0"])
        for i in range(1,5):
            item.setTextAlignment(i,Qt.AlignHCenter)
        self.uploadChangesTable.addTopLevelItem(item)

        item = QTreeWidgetItem(["Removed","0","0","0","0"])
        for i in range(1,5):
            item.setTextAlignment(i,Qt.AlignHCenter)
        self.uploadChangesTable.addTopLevelItem(item)

        item = QTreeWidgetItem(["Changed","0","0","0","0"])
        for i in range(1,5):
            item.setTextAlignment(i,Qt.AlignHCenter)
        self.uploadChangesTable.addTopLevelItem(item)

        self.userLineEdit.setFocus()
        self.passwdLineEdit.setEchoMode(QLineEdit.Password)
        self.uploadButton.setEnabled(False)
        self.pseudoId_map={}
        self.featureId_map={}
        self.qhttp_map={}
        self.progressDialog=QProgressDialog(self)
        self.progressDialog.setModal(True)
        self.finished=False
        self.httpRequestAborted=False
        self.savePasswd=False
        self.changesetId=None

        settings=QSettings()
        if settings.contains("/OSM_Plugin/uploadUser"):
            uplUser=settings.value("/OSM_Plugin/uploadUser",QVariant(QString())).toString()
            self.userLineEdit.setText(uplUser)
        if settings.contains("/OSM_Plugin/uploadPasswd"):
            uplPasswd=settings.value("/OSM_Plugin/uploadPasswd",QVariant(QString())).toString()
            self.passwdLineEdit.setText(uplPasswd)
            self.chkSavePasswd.setChecked(True)

        self.commentTextEdit.setFocus()

        # phases: 0.changeset creation, 1.nodes creation, 2.ways deletion, 3.ways update,
        # 4.ways addition, 5.nodes deletion, 6.nodes update, 7.relation creation,
        # 8.relation deletion, 9.relation update, 10.changeset closing
        self.phase=-1
        self.cntPhases=11
        self.cntActionsInPhase=[0]*self.cntPhases
        self.cntActionsInPhaseDone=0
        self.cntActionsDone=0

        self.createStatistics()
        if self.cntActionsAll<=2:
            # no action to upload (except for changeset opening and closing)
            self.commentTextEdit.setEnabled(False)
            self.accountGroupBox.setEnabled(False)

        self.showStatistics()
        self.__prepareDatabaseQueries()

        # connect signals
        self.connect(self.uploadButton, SIGNAL("clicked()"), self.uploadChanges)
        self.connect(self.userLineEdit, SIGNAL("textChanged(const QString &)"), self.enableUploadButton)
        self.connect(self.passwdLineEdit, SIGNAL("textChanged(const QString &)"), self.enableUploadButton)
        self.connect(self.chkShowPasswd, SIGNAL("clicked()"), self.__showPassword)
        self.connect(self.chkSavePasswd, SIGNAL("clicked()"), self.__savePassword)
        self.connect(self.progressDialog, SIGNAL("canceled()"), self.__cancelProgressDlg)

        self.enableUploadButton()

        # http connection
        self.http=QHttp(self)

        self.connect(self.http,SIGNAL("responseHeaderReceived(QHttpResponseHeader)"), self.__readResponseHeader)
        self.connect(self.http,SIGNAL("authenticationRequired(const QString &, quint16, QAuthenticator *)"), self.__authRequired)

        self.__setProxy()
        self.reqSetHost=self.http.setHost(self.urlHost, 80)
        self.httpRequestAborted = False
        self.httpRequestCanceled = False
        self.httpRequestZombie = False
        self.responseHeader=""

        # increase maximum recursion depth in python (__uploadStep is recursive function)
        setrecursionlimit(1000000)


    def uploadChanges(self):
        """Main function; starts the whole upload process.
        If checkbox for password saving was checked, password is stored to Quantum GIS settings.
        """

        self.progressDialog.setWindowTitle(self.tr("OSM Upload"))
        self.progressDialog.setLabelText(self.tr("Uploading data..."))
        self.uploadButton.setEnabled(False)

        self.cursor=self.dbm.getConnection().cursor()

        settings=QSettings()
        settings.setValue("/OSM_Plugin/uploadUser",QVariant(self.userLineEdit.text()))
        if self.savePasswd:
            settings.setValue("/OSM_Plugin/uploadPasswd",QVariant(self.passwdLineEdit.text()))
        else:
            settings.remove("/OSM_Plugin/uploadPasswd")

        self.reqSetUser=self.http.setUser(self.userLineEdit.text(),self.passwdLineEdit.text())

        # start upload with its first step (the next steps will follow automatically)
        self.__uploadStep()


    def __uploadStep(self):
        """Function calls the next step of uploading.
        """

        # first update progressbar value
        self.progressDialog.setMaximum(self.cntActionsAll)
        self.progressDialog.setValue(self.cntActionsDone)

        # check if (in actual upload phase) number of done actions reach all actions that should be done;
        # if it does, switch actual upload phase to the next one in the order
        if (self.phase==-1) or (self.cntActionsInPhaseDone==self.cntActionsInPhase[self.phase]):
            self.dbm.commit()             # commit actions performed in finished phase

            # search for first next phase in which some upload actions should be done
            self.phase+=1
            while self.phase<self.cntPhases and self.cntActionsInPhase[self.phase]==0:
                self.phase+=1

            # is there another phase to run?
            if self.phase>=self.cntPhases:
                self.cursor.close()            # all upload phases were finished!
                self.__finishUpload()
                return

            # yes, it is ;) tell its number
            # print QString("Starting upload phase no.%1.").arg(self.phase)

            # if necessary set up database cursor
            if not self.phase in (0,10):
                self.cursor.execute(self.selectQuery[self.phase])

            # no action has been done yet in running phase
            self.cntActionsInPhaseDone = 0

        # just print common info
        # print QString("Running step %1 of upload phase %2.").arg(self.cntActionsInPhaseDone+1).arg(self.phase)

        # perform next action in running phase
        if self.phase not in (0,10):
            record = self.cursor.fetchone()
            if record == None:
                # strange situation, number of features ready to upload in actual phase was actually lower than it was signalized
                # by self.cntActionsAll; well, ignore this step and run the next one, that will start the next upload phase!
                self.cntActionsInPhaseDone = self.cntActionsInPhase[self.phase]
                self.__uploadStep()

        if self.phase == 0:        # compulsory changeset creation
            self.__createChangeset()
        elif self.phase == 1:      # nodes creation
            self.__uploadNodeAddition(record)
        elif self.phase == 2:      # ways deletion
            self.__uploadWayDeletion(record)
        elif self.phase == 3:      # ways update
            self.__uploadWayUpdate(record)
        elif self.phase == 4:      # ways creation
            self.__uploadWayAddition(record)
        elif self.phase == 5:      # nodes deletion
            self.__uploadNodeDeletion(record)
        elif self.phase == 6:      # nodes update
            self.__uploadNodeUpdate(record)
        elif self.phase == 7:      # relations creation
            self.__uploadRelationAddition(record)
        elif self.phase == 8:      # relations deletion
            self.__uploadRelationDeletion(record)
        elif self.phase == 9:      # relations update
            self.__uploadRelationUpdate(record)
        elif self.phase == 10:     # compulsory changeset closing
            self.__closeChangeset()
        else:
            print "Program shouldn't reach this place! (2)"
            return


    def __showPassword(self):
        """Function to show hidden password on dialog box,
        so that user can verify if he has written it right.
        """

        if self.chkShowPasswd.isChecked():
            self.passwdLineEdit.setEchoMode(QLineEdit.Normal)
        else:
            self.passwdLineEdit.setEchoMode(QLineEdit.Password)


    def __savePassword(self):
        """Function to show hidden password on dialog box,
        so that user can verify if he has written it right.
        """

        if self.chkSavePasswd.isChecked():
            self.savePasswd=True
        else:
            self.savePasswd=False


    def createStatistics(self):
        """Function finds out number of steps for all upload phases.
        These statistics are found out with examining features' statuses.
        Results are stored in uploaders' member variables.
        """

        c=self.dbm.getConnection().cursor()
        self.cntActionsAll = 0

        # phases: 0.changeset creation, 1.nodes creation, 2.ways deletion, 3.ways update,
        # 4.ways addition, 5.nodes deletion, 6.nodes update, 7.relation creation,
        # 8.relation deletion, 9.relation update, 10.changeset closing

        c.execute("select count(*) from node where status='A' and u=1")
        for row in c:
            self.cntNodesCreated = row[0]
            self.cntActionsAll += row[0]

        c.execute("select count(*) from way where status='A' and closed=0 and u=1")
        for row in c:
            self.cntLinesCreated = row[0]
            self.cntActionsAll += row[0]

        c.execute("select count(*) from way where status='A' and closed=1 and u=1")
        for row in c:
            self.cntPolygonsCreated = row[0]
            self.cntActionsAll += row[0]

        c.execute("select count(*) from node where status='R' and u=1")
        for row in c:
            self.cntNodesRemoved = row[0]
            self.cntActionsAll += row[0]

        c.execute("select count(*) from way where status='R' and closed=0 and u=1")
        for row in c:
            self.cntLinesRemoved = row[0]
            self.cntActionsAll += row[0]

        c.execute("select count(*) from way where status='R' and closed=1 and u=1")
        for row in c:
            self.cntPolygonsRemoved = row[0]
            self.cntActionsAll += row[0]

        c.execute("select count(*) from node where status='U' and u=1")
        for row in c:
            self.cntNodesUpdated = row[0]
            self.cntActionsAll += row[0]

        c.execute("select count(*) from way where status='U' and closed=0 and u=1")
        for row in c:
            self.cntLinesUpdated = row[0]
            self.cntActionsAll += row[0]

        c.execute("select count(*) from way where status='U' and closed=1 and u=1")
        for row in c:
            self.cntPolygonsUpdated = row[0]
            self.cntActionsAll += row[0]

        c.execute("select count(*) from relation where status='A' and u=1")
        for row in c:
            self.cntRelationsCreated = row[0]
            self.cntActionsAll += row[0]

        c.execute("select count(*) from relation where status='R' and u=1")
        for row in c:
            self.cntRelationsRemoved = row[0]
            self.cntActionsAll += row[0]

        c.execute("select count(*) from relation where status='U' and u=1")
        for row in c:
            self.cntRelationsUpdated = row[0]
            self.cntActionsAll += row[0]

        self.cntActionsInPhase[0] = 1
        self.cntActionsInPhase[1] = self.cntNodesCreated
        self.cntActionsInPhase[2] = self.cntLinesRemoved+self.cntPolygonsRemoved
        self.cntActionsInPhase[3] = self.cntLinesUpdated+self.cntPolygonsUpdated
        self.cntActionsInPhase[4] = self.cntLinesCreated+self.cntPolygonsCreated
        self.cntActionsInPhase[5] = self.cntNodesRemoved
        self.cntActionsInPhase[6] = self.cntNodesUpdated
        self.cntActionsInPhase[7] = self.cntRelationsCreated
        self.cntActionsInPhase[8] = self.cntRelationsRemoved
        self.cntActionsInPhase[9] = self.cntRelationsUpdated
        self.cntActionsInPhase[10] = 1
        self.cntActionsAll += 2
        c.close()


    def zeroStatistics(self):
        """Function cancels all statistics of uploader.
        """

        for i in range(1,self.cntPhases-1):
            self.cntActionsInPhase[i] = 0
        self.cntActionsAll = 2

        self.cntNodesCreated = 0
        self.cntLinesRemoved = 0
        self.cntPolygonsRemoved = 0
        self.cntLinesCreated = 0
        self.cntPolygonsCreated = 0
        self.cntNodesRemoved = 0
        self.cntNodesUpdated = 0
        self.cntLinesUpdated = 0
        self.cntPolygonsUpdated = 0
        self.cntRelationsCreated = 0
        self.cntRelationsRemoved = 0
        self.cntRelationsUpdated = 0


    def showStatistics(self):
        """Function shows (statistics) counts of typical actions that are ready for upload.

        Showing them is realized on OSM upload dialog with appropriate table of statistics.
        """

        # phases: 0.changeset creation, 1.nodes creation, 2.ways deletion, 3.ways addition,
        # 4.nodes deletion, 5.nodes update, 6.ways update, 7.relations addition, 8.relations deletion,
        # 9.relations update, 10.changeset closing

        self.uploadChangesTable.topLevelItem(0).setText(1,str(self.cntNodesCreated))
        self.uploadChangesTable.topLevelItem(1).setText(1,str(self.cntNodesRemoved))
        self.uploadChangesTable.topLevelItem(2).setText(1,str(self.cntNodesUpdated))

        self.uploadChangesTable.topLevelItem(0).setText(2,str(self.cntLinesCreated))
        self.uploadChangesTable.topLevelItem(1).setText(2,str(self.cntLinesRemoved))
        self.uploadChangesTable.topLevelItem(2).setText(2,str(self.cntLinesUpdated))

        self.uploadChangesTable.topLevelItem(0).setText(3,str(self.cntPolygonsCreated))
        self.uploadChangesTable.topLevelItem(1).setText(3,str(self.cntPolygonsRemoved))
        self.uploadChangesTable.topLevelItem(2).setText(3,str(self.cntPolygonsUpdated))

        self.uploadChangesTable.topLevelItem(0).setText(4,str(self.cntRelationsCreated))
        self.uploadChangesTable.topLevelItem(1).setText(4,str(self.cntRelationsRemoved))
        self.uploadChangesTable.topLevelItem(2).setText(4,str(self.cntRelationsUpdated))


    def __prepareDatabaseQueries(self):
        """Function prepares SQL queries for selecting objects to upload.
        Resulting array of queries is stored in member variable.
        """

        self.selectQuery = [
            # changeset creation
            ""
            # nodes creation
           ,"select n.id, n.lat, n.lon, n.user, n.timestamp, (select version_id \
             from version where object_id=n.id and object_type='node') version_id from node n where n.status='A' and n.u=1"
            # ways deletion
           ,"select w.id, w.user, w.timestamp, (select version_id from version where \
             object_id=w.id and object_type='way') version_id, w.closed from way w where w.status='R' and w.u=1"
            # ways update
           ,"select w.id, w.user, w.timestamp, (select version_id from version \
             where object_id=w.id and object_type='way') version_id, w.closed from way w where w.status='U' and w.u=1"
            # ways creation
           ,"select w.id, w.user, w.timestamp, (select version_id from version \
             where object_id=w.id and object_type='way') version_id, w.closed from way w where w.status='A' and w.u=1"
            # nodes deletion
           ,"select n.id, n.lat, n.lon, n.user, n.timestamp, (select version_id \
             from version where object_id=n.id and object_type='node') version_id from node n where n.status='R' and n.u=1"
            # nodes update
           ,"select n.id, n.lat, n.lon, n.user, n.timestamp, (select version_id \
             from version where object_id=n.id and object_type='node') version_id from node n where n.status='U' and n.u=1"

            # relations creation
           ,"select r.id, r.user, r.timestamp, (select version_id \
             from version where object_id=r.id and object_type='relation') version_id from relation r where r.status='A' and r.u=1"
            # relations deletion
           ,"select r.id, r.user, r.timestamp, (select version_id \
             from version where object_id=r.id and object_type='relation') version_id from relation r where r.status='R' and r.u=1"
            # relations update
           ,"select r.id, r.user, r.timestamp, (select version_id \
             from version where object_id=r.id and object_type='relation') version_id from relation r where r.status='U' and r.u=1"
            # changeset closing
           ,""
           ]


    def __uploadNodeAddition(self, nodeRecord):
        """Sends upload request for addition of new node.

        @param nodeRecord tuple object with information on node
        """

        # create http connection with neccessary authentification
        urlPath = "/api/0.6/node/create"

        # set http request's header
        header = QHttpRequestHeader("PUT", urlPath,1,1)
        header.setValue("Host", self.urlHost)
        header.setValue("Connection", "keep-alive")
        header.setContentType("text/xml; charset=utf-8")

        # create http request's body (create XML with info about uploaded node)
        requestXml = self.createNodeXml(nodeRecord)

        # connect http response signals to appropriate functions
        self.connect(self.http, SIGNAL("requestFinished(int, bool)"), self.__httpNodeAdditionReqFinished)

        # send prepared request
        requestBytes = requestXml.toAscii()
        httpSessionId = self.http.request(header, requestBytes)

        # remember http connection object and pseudoId, that was used
        # to node's identification in sqlite3 database
        self.qhttp_map[httpSessionId]=1
        self.pseudoId_map[httpSessionId]=nodeRecord[0]


    def __uploadNodeUpdate(self, nodeRecord):
        """Sends upload request for updating one node.

        @param nodeRecord tuple object with information on node
        """

        urlPath = QString("/api/0.6/node/%1").arg(nodeRecord[0])

        # set http request's header
        header = QHttpRequestHeader("PUT", urlPath,1,1)
        header.setValue("Host", self.urlHost)
        header.setValue("Connection", "keep-alive")
        header.setContentType("text/xml; charset=utf-8")

        # create http request's body (create XML with info about uploaded node)
        requestXml = self.createNodeXml(nodeRecord)

        # connect http response signals to appropriate functions
        self.connect(self.http, SIGNAL("requestFinished(int, bool)"), self.__httpNodeUpdateReqFinished)

        # send prepared request
        requestBytes=requestXml.toAscii()
        httpSessionId=self.http.request(header, requestBytes)

        # remember http connection object
        self.qhttp_map[httpSessionId]=1
        self.featureId_map[httpSessionId]=nodeRecord[0]


    def __uploadNodeDeletion(self, nodeRecord):
        """Send upload request for deletion of one node.

        @param nodeRecord tuple object with information on node
        """

        # create http connection with neccessary authentification
        urlPath = QString("/api/0.6/node/%1").arg(nodeRecord[0])

        # set http request's header
        header = QHttpRequestHeader("DELETE", urlPath,1,1)
        header.setValue("Host", self.urlHost)
        header.setValue("Connection", "keep-alive")
        header.setContentType("text/xml; charset=utf-8")

        # connect http response signals to appropriate functions
        self.connect(self.http, SIGNAL("requestFinished(int, bool)"), self.__httpNodeDeletionReqFinished)

        requestXml=self.createNodeDelXml(nodeRecord)

        # send prepared request
        requestBytes = requestXml.toAscii()
        httpSessionId = self.http.request(header, requestBytes)

        # remember http connection object
        self.qhttp_map[httpSessionId]=1
        self.featureId_map[httpSessionId]=nodeRecord[0]


    def createNodeXml(self, nodeRecord):
        """Function creates XML that will be added as http body to node addition request.

        @param nodeRecord tuple object with information on node
        """

        requestXml = QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>")
        version = nodeRecord[5]                     # version_id
        requestXml.append(QString("<osm version=\"0.6\" generator=\"OpenStreetMap server\">"))
        requestXml.append(QString("<node id=\"%1\" lat=\"%2\" lon=\"%3\" visible=\"true\" user=\"%4\" timestamp=\"%5\"")
            .arg(nodeRecord[0])                     # node_id
            .arg(nodeRecord[1],0,'f',50)            # lat
            .arg(nodeRecord[2],0,'f',50)            # lon
            .arg(str(nodeRecord[3]))                # user
            .arg(str(nodeRecord[4]))                # timestamp
            )
        if version<>None:
            requestXml.append(QString(" version=\"%1\"").arg(str(version)))
        requestXml.append(QString(" changeset=\"%1\">")
            .arg(str(self.changesetId))
            )

        # selecting tags to construct correct XML
        ct=self.dbm.getConnection().cursor()
        ct.execute("select key, val from tag where object_id=:nodeId and object_type=\"node\"",{"nodeId":nodeRecord[0]})
        for tagRecord in ct:
            requestXml.append(QString("<tag k=\"%1\" v=\"%2\"/>").arg(tagRecord[0]).arg(tagRecord[1]))
        ct.close()

        requestXml.append(QString("</node></osm>"))
        return requestXml


    def createNodeDelXml(self, nodeRecord):
        """Function creates XML that will be added as http body to node deletion request.

        @param nodeRecord tuple object with information on node
        """

        requestXml = QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>")
        requestXml.append(QString("<osm version=\"0.6\" generator=\"OpenStreetMap server\">"))
        requestXml.append(QString("<node id=\"%1\" lat=\"%2\" lon=\"%3\" visible=\"true\" user=\"%4\" timestamp=\"%5\"")
                .arg(nodeRecord[0])
                .arg(nodeRecord[1],0,'f',50)
                .arg(nodeRecord[2],0,'f',50)
                .arg(str(nodeRecord[3]))
                .arg(str(nodeRecord[4]))
                )
        requestXml.append(QString(" version=\"%1\"").arg(nodeRecord[5]))
        requestXml.append(QString(" changeset=\"%1\"></node>").arg(str(self.changesetId)))
        requestXml.append(QString("</osm>"))
        return requestXml


    def createWayXml(self, wayRecord):
        """Creates XML that have to be added as http body to way addition request.

        @param wayRecord tuple object with information on way
        """

        pseudoWayId = wayRecord[0]
        requestXml = QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>")

        version = wayRecord[3]        # version_id
        closed = wayRecord[4]
        requestXml.append(QString("<osm version=\"0.6\" generator=\"OpenStreetMap server\">"))
        requestXml.append(QString("<way id=\"%1\" visible=\"true\" timestamp=\"%3\" user=\"%2\"")
            .arg(wayRecord[0])        # way_id
            .arg(str(wayRecord[1]))   # user
            .arg(str(wayRecord[2]))   # timestamp
            )

        if version<>None:
            requestXml.append(QString(" version=\"%1\"").arg(str(version)))
        requestXml.append(QString(" changeset=\"%1\">")
            .arg(str(self.changesetId))
            )

        # selecting way members to construct correct XML
        firstMember = None
        cwm=self.dbm.getConnection().cursor()
        cwm.execute("select node_id from way_member where way_id=:pseudoWayId and u=1",{"pseudoWayId": pseudoWayId})
        for wayMemberRecord in cwm:
            if firstMember==None:
                firstMember=wayMemberRecord[0]
            requestXml.append(QString("<nd ref=\"%1\"/>").arg(wayMemberRecord[0]))
        cwm.close()

        if closed==1:
            tmp=None

            if firstMember:
                requestXml.append(QString("<nd ref=\"%1\"/>").arg(firstMember))

        # selecting tags to construct correct XML
        ct=self.dbm.getConnection().cursor()
        ct.execute("select key, val from tag where object_id=:pseudoWayId and u=1",{"pseudoWayId": pseudoWayId})
        for tagRecord in ct:
            tmp=None
            requestXml.append(QString("<tag k=\"%1\" v=\"%2\"/>").arg(tagRecord[0]).arg(tagRecord[1]))
        ct.close()

        # finish XML construction
        requestXml.append(QString("</way></osm>"))
        return requestXml


    def __uploadWayAddition(self, wayRecord):
        """Sends upload request for addition of new way.

        @param wayRecord tuple object with information on way
        """

        pseudoWayId = wayRecord[0]

        # create http connection with neccessary authentification
        urlPath = "/api/0.6/way/create"

        # set http request's header
        header = QHttpRequestHeader("PUT", urlPath,1,1)
        header.setValue("Host", self.urlHost)
        header.setValue("Connection", "keep-alive")
        header.setContentType("text/xml; charset=utf-8")

        # connect http response signals to appropriate functions
        self.connect(self.http, SIGNAL("requestFinished(int, bool)"), self.__httpWayAdditionReqFinished)

        # create http request's body (create XML with info about uploaded way)
        requestXml = self.createWayXml(wayRecord)

        # send prepared request
        requestBytes = requestXml.toAscii()
        httpSessionId = self.http.request(header, requestBytes)

        # remember pseudoId, that was used to nodes identification in sqlite3 database
        self.qhttp_map[httpSessionId]=1
        self.pseudoId_map[httpSessionId]=pseudoWayId


    def __uploadWayUpdate(self, wayRecord):
        """Send upload request for updating given way.

        @param wayRecord tuple object with information on way
        """

        # create http connection with neccessary authentification
        urlPath = QString("/api/0.6/way/%1").arg(wayRecord[0])

        # set http request's header
        header=QHttpRequestHeader("PUT", urlPath,1,1)
        header.setValue("Host", self.urlHost)
        header.setValue("Connection", "keep-alive")
        header.setContentType("text/xml; charset=utf-8")

        # connect http response signals to appropriate functions
        self.connect(self.http, SIGNAL("requestFinished(int, bool)"), self.__httpWayUpdateReqFinished)

        # create http request's body (create XML with info about uploaded way)
        requestXml = self.createWayXml(wayRecord)

        # send prepared request
        requestBytes = requestXml.toAscii()
        httpSessionId = self.http.request(header, requestBytes)

        self.qhttp_map[httpSessionId]=1
        self.featureId_map[httpSessionId]=wayRecord[0]


    def __uploadWayDeletion(self, wayRecord):
        """Send upload request for removing given way.

        @param wayRecord tuple object with information on way
        """

        # create http connection with neccessary authentification
        urlPath = QString("/api/0.6/way/%1").arg(wayRecord[0])

        # set http request's header
        header = QHttpRequestHeader("DELETE", urlPath,1,1)
        header.setValue("Host", self.urlHost)
        header.setValue("Connection", "keep-alive")
        header.setContentType("text/xml; charset=utf-8")

        # connect http response signals to appropriate functions
        self.connect(self.http, SIGNAL("requestFinished(int, bool)"), self.__httpWayDeletionReqFinished)

         # create http request's body (create XML with info about uploaded way deletion)
        requestXml = self.createWayXml(wayRecord)

        # send prepared request
        requestBytes = requestXml.toAscii()
        httpSessionId = self.http.request(header, requestBytes)

        # remember http connection object
        self.qhttp_map[httpSessionId]=1
        self.featureId_map[httpSessionId]=wayRecord[0]


    def createRelationXml(self, relRecord):
        """Creates XML that will be added to relation addition/deletion/update http request.

        @param relRecord tuple object with information on relation
        """

        pseudoRelId = relRecord[0]
        requestXml = QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>")

        version = relRecord[3]        # version_id
        requestXml.append(QString("<osm version=\"0.6\" generator=\"OpenStreetMap server\">"))
        requestXml.append(QString("<relation id=\"%1\" visible=\"true\" timestamp=\"%3\" user=\"%2\"")
            .arg(relRecord[0])        # way_id
            .arg(str(relRecord[1]))   # user
            .arg(str(relRecord[2]))   # timestamp
            )

        if version<>None:
            requestXml.append(QString(" version=\"%1\"").arg(str(version)))
        requestXml.append(QString(" changeset=\"%1\">").arg(str(self.changesetId)))

        # selecting way members to construct correct XML
        firstMember = None
        c=self.dbm.getConnection().cursor()
        c.execute("select member_type, role, member_id from relation_member where relation_id=:pseudoRelId and u=1",{"pseudoRelId": pseudoRelId})
        for relMemberRecord in c:
            requestXml.append(QString("<member type=\"%1\"").arg(relMemberRecord[0]))
            if relMemberRecord[1]<>None:
                requestXml.append(QString(" role=\"%1\"").arg(relMemberRecord[1]))
            requestXml.append(QString(" ref=\"%1\"/>").arg(relMemberRecord[2]))
        c.close()

        # selecting tags to construct correct XML
        ct=self.dbm.getConnection().cursor()
        ct.execute("select key, val from tag where object_id=:pseudoRelId and object_type='relation' and u=1",{"pseudoRelId": pseudoRelId})
        for tagRecord in ct:
            requestXml.append(QString("<tag k=\"%1\" v=\"%2\"/>").arg(tagRecord[0]).arg(tagRecord[1]))
        ct.close()

        # finish XML construction
        requestXml.append(QString("</relation></osm>"))
        return requestXml


    def __uploadRelationAddition(self, relRecord):
        """Sends upload request for addition of new relation.

        @param relRecord tuple object with information on relation
        """

        pseudoRelId = relRecord[0]

        # create http connection with neccessary authentification
        urlPath = "/api/0.6/relation/create"

        # set http request's header
        header = QHttpRequestHeader("PUT", urlPath,1,1)
        header.setValue("Host", self.urlHost)
        header.setValue("Connection", "keep-alive")
        header.setContentType("text/xml; charset=utf-8")

        # connect http response signals to appropriate functions
        self.connect(self.http, SIGNAL("requestFinished(int, bool)"), self.__httpRelationAdditionReqFinished)

        # create http request's body (create XML with info about uploaded way)
        requestXml = self.createRelationXml(relRecord)

        # send prepared request
        requestBytes = requestXml.toAscii()
        httpSessionId = self.http.request(header, requestBytes)

        # remember pseudoId, that was used to nodes identification in sqlite3 database
        self.qhttp_map[httpSessionId]=1
        self.pseudoId_map[httpSessionId]=pseudoRelId


    def __uploadRelationUpdate(self,relRecord):
        """Sends upload request for updating given relation.

        @param relRecord tuple object with information on relation
        """

        # create http connection with neccessary authentification
        urlPath = QString("/api/0.6/relation/%1").arg(relRecord[0])

        # set http request's header
        header=QHttpRequestHeader("PUT", urlPath,1,1)
        header.setValue("Host", self.urlHost)
        header.setValue("Connection", "keep-alive")
        header.setContentType("text/xml; charset=utf-8")

        # connect http response signals to appropriate functions
        self.connect(self.http, SIGNAL("requestFinished(int, bool)"), self.__httpRelationUpdateReqFinished)

        # create http request's body (create XML with info about uploaded way)
        requestXml = self.createRelationXml(relRecord)

        # send prepared request
        requestBytes = requestXml.toAscii()
        httpSessionId = self.http.request(header, requestBytes)

        self.qhttp_map[httpSessionId]=1
        self.featureId_map[httpSessionId]=relRecord[0]


    def __uploadRelationDeletion(self, relRecord):
        """Function sends upload request for removing given relation.

        @param relRecord tuple object with information on relation
        """


        # create http connection with neccessary authentification
        urlPath = QString("/api/0.6/relation/%1").arg(relRecord[0])

        # set http request's header
        header=QHttpRequestHeader("DELETE", urlPath,1,1)
        header.setValue("Host", self.urlHost)
        header.setValue("Connection", "keep-alive")
        header.setContentType("text/xml; charset=utf-8")

        # connect http response signals to appropriate functions
        self.connect(self.http, SIGNAL("requestFinished(int, bool)"), self.__httpRelationDeletionReqFinished)

         # create http request's body (create XML with info about uploaded way deletion)
        requestXml = self.createRelationXml(relRecord)

        # send prepared request
        requestBytes=requestXml.toAscii()
        httpSessionId=self.http.request(header, requestBytes)

        # remember http connection object
        self.qhttp_map[httpSessionId]=1
        self.featureId_map[httpSessionId]=relRecord[0]


    def escape_html(self,text):
        """Function replaces chars that are problematic for html,
        such as < > & " ' with &lt; &gt; &amp; &quot; &apos;

        @param text text to replace problematic characters in
        @return transformed text
        """

        text=text.replace(QString("&"), QString("&amp;"));    # must be first
        text=text.replace(QString("<"), QString("&lt;"));
        text=text.replace(QString(">"), QString("&gt;"));
        text=text.replace(QString("\""), QString("&quot;"));
        text=text.replace(QString("'"), QString("&apos;"));
        return text


    def __createChangeset(self):
        """Function sends request for creating new OSM changeset.

        Changeset creation has to be first upload action.
        Changeset closing has to be the last one.
        """

        # create http connection with neccessary authentification
        urlPath="/api/0.6/changeset/create"

        # set http request's header
        header=QHttpRequestHeader("PUT",urlPath,1,1)
        header.setValue("Host", self.urlHost)
        header.setValue("Connection", "keep-alive")
        header.setContentType("text/xml; charset=utf-8")

        # connect http response signals to appropriate functions
        self.connect(self.http, SIGNAL("requestFinished(int, bool)"), self.__httpChangesetCreationReqFinished)

        # get user comment on upload actions
        userComment=self.commentTextEdit.toPlainText()
        userComment=self.escape_html(userComment)
        userCommentBytes=userComment.toUtf8()

        # create http request's body (create XML with info about uploaded way)
        requestXml=QString("<osm>\n<changeset>\n<tag k=\"created_by\" v=\"QGIS OSM v0.4\"/>\n<tag k=\"comment\" v=\""+userCommentBytes.data()+"\"/>\n</changeset>\n</osm>")

        # send prepared request
        requestBytes=requestXml.toAscii()
        httpSessionId=self.http.request(header, requestBytes)

        # remember http connection object
        self.qhttp_map[httpSessionId]=1


    def __closeChangeset(self):
        """Function sends request for closing opened OSM changeset.

        Changeset creation has to be first upload action.
        Changeset closing has to be the last one.
        """

        # create http connection with neccessary authentification
        urlPath = QString("/api/0.6/changeset/%1/close").arg(self.changesetId)

        # set http request's header
        header = QHttpRequestHeader("PUT",urlPath,1,1)
        header.setValue("Host", self.urlHost)
        header.setValue("Connection", "keep-alive")
        header.setContentLength(0)

        # connect http response signals to appropriate functions
        self.connect(self.http, SIGNAL("requestFinished(int, bool)"), self.__httpChangesetClosingReqFinished)
        self.changesetId=None

        # send prepared request
        httpSessionId=self.http.request(header)

        # remember http connection object
        self.qhttp_map[httpSessionId]=1


    def enableUploadButton(self):
        """Function finds out if it is possible to enable upload button.
        If yes, function just enables it, else it disables it.

        Following condition has to be satisfied to enable upload button:
        -There has to be something to upload.
        -Both user login and password has to be filled.
        """

        user=self.userLineEdit.text()
        passwd=self.passwdLineEdit.text()

        if not self.finished and user<>"" and passwd<>"" and self.cntActionsAll>2:
            self.uploadButton.setEnabled(True)
        else:
            self.uploadButton.setEnabled(False)


    def __finishUpload(self):
        """This is function that runs after all edit actions changes are uploaded to OSM server.

        It clears edit changes tables, hide progress bar, clear statistics,
        clear undo/redo data, etc.
        """

        self.finished = True
        self.progressDialog.hide()

        # delete database records with status='R' (Remove)
        self.dbm.removeUselessRecords()

        # show info that there are no more actions to upload
        self.zeroStatistics()
        self.showStatistics()

        # disable dialog items
        self.userLineEdit.setEnabled(False)
        self.passwdLineEdit.setEnabled(False)
        self.chkShowPasswd.setEnabled(False)

        self.accept()

        # clear undo/redo
        self.ur.clear()


    def cancelUpload(self,errMessage=None):
        """Function aborts the whole upload operation.
        In errMessage function gets the reason of upload canceling (error message).

        If OSM changeset was already opened while uploading, HTTP request to close it is send immediately.
        The HTTP connection of uploader is aborted, progress dialog is closed and the whole upload dialog is rejected.

        If there is some errMessage parameter, the message is shown to user.

        @param errMessage message to tell user after canceling upload
        """

        if self.httpRequestAborted:
            return

        if self.changesetId:

            urlPath=QString("/api/0.6/changeset/%1/close").arg(self.changesetId)
            header=QHttpRequestHeader("PUT",urlPath,1,1)
            header.setValue("Host", self.urlHost)
            header.setValue("Connection", "keep-alive")
            httpSessionId=self.http.request(header)

        self.httpRequestAborted=True
        self.http.abort()

        self.uploadButton.setEnabled(True)
        self.disconnect(self.progressDialog, SIGNAL("canceled()"), self.__cancelProgressDlg)
        self.progressDialog.close()
        self.reject()

        if errMessage and errMessage<>"":
            QMessageBox.information(self,self.tr("OSM Upload"),errMessage)


    def __cancelProgressDlg(self):
        """This functions catches the signal emitted when clicking on Cancel button of progress dialog.
        It aborts opened HTTP connection and closes upload dialog.
        """

        if self.httpRequestAborted:
            return

        self.httpRequestAborted=True
        self.http.abort()

        self.uploadButton.setEnabled(True)
        self.reject()


    ##########################################################
    #  !!!!! HTTP RESPONSES FROM OPENSTREETMAP SERVER !!!!!  #
    ##########################################################


    def __httpNodeAdditionReqFinished(self, requestId, error):
        """Function is called when requestFinished(...) signal is emitted on global HTTP connection object
        and "node adition" is current uploader's phase.

        OSM server returns id that was assigned to uploaded node.

        @param requestId identifier of http request
        @param error True if error occured on given request; False otherwise
        """

        if self.httpRequestAborted:
            return

        if not requestId in self.qhttp_map:
            self.__examineResponse(requestId,error)
            return

        self.disconnect(self.http, SIGNAL("requestFinished(int, bool)"), self.__httpNodeAdditionReqFinished)

        nodePseudoId=self.pseudoId_map[requestId]
        del self.qhttp_map[requestId]
        del self.pseudoId_map[requestId]

        if error:
            self.cancelUpload("Node addition failed.")
            return

        newNodeIdStr=QString(self.http.readAll().data())
        newNodeId=(newNodeIdStr.toInt())[0]

        # update node's identification in sqlite3 database
        self.dbm.updateNodeId(nodePseudoId,newNodeId,self.userLineEdit.text())
        self.dbm.recacheAffectedNow([(nodePseudoId,'Point'),(newNodeId,'Point')])

        # call the next upload step
        self.cntActionsInPhaseDone+=1
        self.cntActionsDone+=1
        self.__uploadStep()


    def __httpNodeUpdateReqFinished(self, requestId, error):
        """Function is called when requestFinished(...) signal is emitted on global HTTP connection object
        and "node updating" is current uploader's phase.

        OSM server returns id of new node's version.

        @param requestId identifier of http request
        @param error True if error occured on given request; False otherwise
        """

        if self.httpRequestAborted:
            return

        if not requestId in self.qhttp_map:
            self.__examineResponse(requestId,error)
            return

        self.disconnect(self.http, SIGNAL("requestFinished(int, bool)"), self.__httpNodeUpdateReqFinished)

        pointId=self.featureId_map[requestId]
        del self.qhttp_map[requestId]
        del self.featureId_map[requestId]

        if error:
            self.cancelUpload("Node update failed.")
            return

        newVersionIdStr=QString(self.http.readAll().data())
        newVersionId=(newVersionIdStr.toInt())[0]

        self.dbm.updateVersionId(pointId,'node',newVersionId)
        self.dbm.updateUser(pointId,'node',self.userLineEdit.text())
        self.dbm.changePointStatus(pointId,'U','N')
        self.dbm.recacheAffectedNow([(pointId,'Point')])

        # call the next upload step
        self.cntActionsInPhaseDone+=1
        self.cntActionsDone+=1
        self.__uploadStep()


    def __httpNodeDeletionReqFinished(self, requestId, error):
        """Function is called when requestFinished(...) signal is emitted on global HTTP connection object
        and "node deletion" is current uploader's phase.

        @param requestId identifier of http request
        @param error True if error occured on given request; False otherwise
        """

        if self.httpRequestAborted:
            return

        if not requestId in self.qhttp_map:
            self.__examineResponse(requestId,error)
            return

        self.disconnect(self.http, SIGNAL("requestFinished(int, bool)"), self.__httpNodeDeletionReqFinished)

        pointId=self.featureId_map[requestId]
        del self.qhttp_map[requestId]
        del self.featureId_map[requestId]

        if error:
            self.cancelUpload("Node deletion failed.")
            return

        self.dbm.removePointRecord(pointId)
        self.dbm.recacheAffectedNow([(pointId,'Point')])

        # call the next upload step
        self.cntActionsInPhaseDone+=1
        self.cntActionsDone+=1
        self.__uploadStep()


    def __httpWayAdditionReqFinished(self, requestId, error):
        """Function is called when requestFinished(...) signal is emitted on global HTTP connection object
        and "way addition" is current uploader's phase.

        OSM server returns id that was assigned to uploaded way.

        @param requestId identifier of http request
        @param error True if error occured on given request; False otherwise
        """

        if self.httpRequestAborted:
            return

        if not requestId in self.qhttp_map:
            self.__examineResponse(requestId,error)
            return

        self.disconnect(self.http, SIGNAL("requestFinished(int, bool)"), self.__httpWayAdditionReqFinished)

        wayPseudoId = self.pseudoId_map[requestId]
        del self.qhttp_map[requestId]
        del self.pseudoId_map[requestId]

        if error:
            self.cancelUpload("Way addition failed.")
            return

        newWayIdStr = QString(self.http.readAll().data())
        newWayId = (newWayIdStr.toInt())[0]

        # update way identification in sqlite3 database
        self.dbm.updateWayId(wayPseudoId,newWayId,self.userLineEdit.text())
        self.dbm.recacheAffectedNow([(wayPseudoId,'Line'),(newWayId,'Line'),(wayPseudoId,'Polygon'),(newWayId,'Polygon')])

        # call the next upload step
        self.cntActionsInPhaseDone+=1
        self.cntActionsDone+=1
        self.__uploadStep()


    def __httpWayUpdateReqFinished(self, requestId, error):
        """Function is called when requestFinished(...) signal is emitted on global HTTP connection object
        and "way updating" is current uploader's phase.

        OSM server returns id of new way's version.

        @param requestId identifier of http request
        @param error True if error occured on given request; False otherwise
        """

        if self.httpRequestAborted:
            return

        if not requestId in self.qhttp_map:
            self.__examineResponse(requestId,error)
            return

        self.disconnect(self.http, SIGNAL("requestFinished(int, bool)"), self.__httpWayUpdateReqFinished)

        wayId=self.featureId_map[requestId]
        del self.qhttp_map[requestId]
        del self.featureId_map[requestId]

        if error:
            self.cancelUpload("Way update failed.")
            return

        newVersionIdStr=QString(self.http.readAll().data())
        newVersionId=(newVersionIdStr.toInt())[0]

        self.dbm.updateVersionId(wayId,'way',newVersionId)
        self.dbm.updateUser(wayId,'way',self.userLineEdit.text())
        self.dbm.changeWayStatus(wayId,'U','N')
        self.dbm.recacheAffectedNow([(wayId,'Line'),(wayId,'Polygon')])

        # call the next upload step
        self.cntActionsInPhaseDone+=1
        self.cntActionsDone+=1
        self.__uploadStep()


    def __httpWayDeletionReqFinished(self, requestId, error):
        """Function is called when requestFinished(...) signal is emitted on global HTTP connection object
        and "way deletion" is current uploader's phase.

        @param requestId identifier of http request
        @param error True if error occured on given request; False otherwise
        """

        if self.httpRequestAborted:
            return

        if not requestId in self.qhttp_map:
            self.__examineResponse(requestId,error)
            return

        self.disconnect(self.http, SIGNAL("requestFinished(int, bool)"), self.__httpWayDeletionReqFinished)

        wayId=self.featureId_map[requestId]
        del self.qhttp_map[requestId]
        del self.featureId_map[requestId]

        if error:
            self.cancelUpload("Way deletion failed.")
            return

        self.dbm.removeWayRecord(wayId)
        self.dbm.recacheAffectedNow([(wayId,'Line'),(wayId,'Polygon')])

        # call the next upload step
        self.cntActionsInPhaseDone+=1
        self.cntActionsDone+=1
        self.__uploadStep()


    def __httpRelationAdditionReqFinished(self, requestId, error):
        """Function is called when requestFinished(...) signal is emitted on global HTTP connection object
        and "relation addition" is current uploader's phase.

        OSM server returns id that was assigned to uploaded relation.

        @param requestId identifier of http request
        @param error True if error occured on given request; False otherwise
        """

        if self.httpRequestAborted:
            return

        if not requestId in self.qhttp_map:
            self.__examineResponse(requestId,error)
            return

        self.disconnect(self.http, SIGNAL("requestFinished(int, bool)"), self.__httpRelationAdditionReqFinished)

        relPseudoId = self.pseudoId_map[requestId]
        del self.qhttp_map[requestId]
        del self.pseudoId_map[requestId]

        if error:
            self.cancelUpload("Relation addition failed.")
            return

        newRelIdStr=QString(self.http.readAll().data())
        newRelId=(newRelIdStr.toInt())[0]

        # update way identification in sqlite3 database
        self.dbm.updateRelationId(relPseudoId,newRelId,self.userLineEdit.text())

        # call the next upload step
        self.cntActionsInPhaseDone+=1
        self.cntActionsDone+=1
        self.__uploadStep()


    def __httpRelationUpdateReqFinished(self, requestId, error):
        """Function is called when requestFinished(...) signal is emitted on global HTTP connection object
        and "relation updating" is current uploader's phase.

        OSM server returns id of new relation's version.

        @param requestId identifier of http request
        @param error True if error occured on given request; False otherwise
        """

        if self.httpRequestAborted:
            return

        if not requestId in self.qhttp_map:
            self.__examineResponse(requestId,error)
            return

        self.disconnect(self.http, SIGNAL("requestFinished(int, bool)"), self.__httpRelationUpdateReqFinished)

        relId=self.featureId_map[requestId]
        del self.qhttp_map[requestId]
        del self.featureId_map[requestId]

        if error:
            self.cancelUpload("Relation update failed.")
            return

        newVersionIdStr=QString(self.http.readAll().data())
        newVersionId=(newVersionIdStr.toInt())[0]

        self.dbm.updateVersionId(relId,'relation',newVersionId)
        self.dbm.updateUser(relId,'relation',self.userLineEdit.text())
        self.dbm.changeRelationStatus(relId,'U','N')

        # call the next upload step
        self.cntActionsInPhaseDone+=1
        self.cntActionsDone+=1
        self.__uploadStep()


    def __httpRelationDeletionReqFinished(self, requestId, error):
        """Function is called when requestFinished(...) signal is emitted on global HTTP connection object
        and "relation deletion" is current uploader's phase.

        @param requestId identifier of http request
        @param error True if error occured on given request; False otherwise
        """

        if self.httpRequestAborted:
            return

        if not requestId in self.qhttp_map:
            self.__examineResponse(requestId,error)
            return

        self.disconnect(self.http, SIGNAL("requestFinished(int, bool)"), self.__httpRelationDeletionReqFinished)

        relId=self.featureId_map[requestId]
        del self.qhttp_map[requestId]
        del self.featureId_map[requestId]

        if error:
            self.cancelUpload("Relation deletion failed.")
            return

        self.dbm.removeRelationRecord(relId)

        # call the next upload step
        self.cntActionsInPhaseDone+=1
        self.cntActionsDone+=1
        self.__uploadStep()


    def __httpChangesetCreationReqFinished(self, requestId, error):
        """Function is called when requestFinished(...) signal is emitted on global HTTP connection object
        and "changeset creation" is current uploader's phase.

        OSM server returns id that was assigned to created changeset.

        @param requestId identifier of http request
        @param error True if error occured on given request; False otherwise
        """

        if self.httpRequestAborted:
            return

        if not requestId in self.qhttp_map:
            self.__examineResponse(requestId,error)
            return

        self.disconnect(self.http, SIGNAL("requestFinished(int, bool)"), self.__httpChangesetCreationReqFinished)

        if error:
            self.cancelUpload("Connection to OpenStreetMap server cannot be established. Please check your proxy settings, firewall settings and try again.")
            return

        del self.qhttp_map[requestId]
        changesetIdStr=QString(self.http.readAll().data())
        self.changesetId=(changesetIdStr.toInt())[0]

        # call the next upload step
        self.cntActionsInPhaseDone+=1
        self.cntActionsDone+=1
        self.__uploadStep()


    def __httpChangesetClosingReqFinished(self, requestId, error):
        """Function is called when requestFinished(...) signal is emitted on global HTTP connection object
        and "changeset closing" is current uploader's phase.

        @param requestId identifier of http request
        @param error True if error occured on given request; False otherwise
        """

        if self.httpRequestAborted:
            return

        if not requestId in self.qhttp_map:
            self.__examineResponse(requestId,error)
            return

        self.disconnect(self.http, SIGNAL("requestFinished(int, bool)"), self.__httpChangesetClosingReqFinished)

        if error:
            self.cancelUpload("Changeset closing failed.")
            return

        # call the next upload step
        self.cntActionsInPhaseDone+=1
        self.cntActionsDone+=1
        self.__uploadStep()


    def __readResponseHeader(self, responseHeader):
        """Function is called when responseHeaderReceived(...) signal is emitted
        on global HTTP connection object.

        If statusCode of responseHeader doesn't equal to 200, function cancels the whole connection.

        @param responseHeader header of HTTP response from the OSM server
        """

        if responseHeader.statusCode() != 200:

            self.cancelUpload(QString("Upload process failed. OpenStreetMap server response: %1 - %2.")
                .arg(responseHeader.reasonPhrase())
                .arg(responseHeader.value("Error")))


    def __authRequired(self,s,a,b):
        """Function is called when authenticationRequired(...) signal is emitted
        on global HTTP connection object.

        We are really not interested in function parameters - we just cancel the connection.
        """

        self.cancelUpload("Authentification failed. Please try again with correct login and password.")


    def __setProxy(self):
        """Function sets proxy to HTTP connection of uploader.

        HTTP connection object is not given in function parameter,
        because it's global - accessible for the whole uploader.
        """

        # getting and setting proxy information
        settings=QSettings()
        proxyHost=QString()
        proxyUser=QString()
        proxyPassword=QString()
        proxyPort=0
        proxyType=QNetworkProxy.NoProxy
        proxyEnabled=settings.value("proxy/proxyEnabled",QVariant(0)).toBool()

        if proxyEnabled:

            proxyHost=settings.value("proxy/proxyHost",QVariant("")).toString()
            proxyPort=settings.value("proxy/proxyPort",QVariant(8080)).toInt()[0]
            proxyUser=settings.value("proxy/proxyUser",QVariant("")).toString()
            proxyPassword=settings.value("proxy/proxyPassword",QVariant("")).toString()
            proxyTypeString=settings.value("proxy/proxyType",QVariant("")).toString()

            if proxyTypeString=="DefaultProxy":
                proxyType=QNetworkProxy.DefaultProxy
            elif proxyTypeString=="Socks5Proxy":
                proxyType=QNetworkProxy.Socks5Proxy
            elif proxyTypeString=="HttpProxy":
                proxyType=QNetworkProxy.HttpProxy
            elif proxyTypeString=="HttpCachingProxy":
                proxyType=QNetworkProxy.HttpCachingProxy
            elif proxyTypeString=="FtpCachingProxy":
                proxyType=QNetworkProxy.FtpCachingProxy

            self.proxy=QNetworkProxy()
            self.proxy.setType(proxyType)
            self.proxy.setHostName(proxyHost)
            self.proxy.setPort(proxyPort)
            self.reqSetProxy=self.http.__setProxy(self.proxy)


    def __examineResponse(self,requestId,error):
        """If error occured function examines requestId and cancel upload for appropriate reason.

        @param requestId identifier of http request
        @param error True if error occured on given request; False otherwise
        """

        if self.httpRequestAborted or not error:
            return

        if requestId==self.reqSetHost:
            self.cancelUpload("Setting host failed.")

        elif requestId==self.reqSetUser:
            self.cancelUpload("Setting user and password failed.")

        elif requestId==self.reqSetProxy:
            self.cancelUpload("Setting proxy failed.")



