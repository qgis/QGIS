"""@package OsmAddRelationDlg
The main class of this module (OsmAddRelationDlg) is descendant of "Create OSM Relation" dialog.

The dialog either shows detail info on existing relation or is empty when no relation id is passed to constructor.
In brief this module (and its main class) just provides easy way to create or change OSM relation.
...
"""


from OsmAddRelationDlg_ui import Ui_OsmAddRelationDlg
from map_tools.OsmIdentifyMT import OsmIdentifyMT

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import *
from sip import unwrapinstance
from qgis.core import *
from qgis.gui import *

import sqlite3



class OsmAddRelationDlg(QDialog, Ui_OsmAddRelationDlg):
    """This class is direct descendant of "Create OSM Relation" dialog. It provides easy way to create
    or change OSM relation. Methods of OsmAddRelationDlg class catch signals emitted when changing relations
    type, tags or members, submitting or rejecting the whole dialog. After catching signal, methods must
    perform appropriate operation using methods of OsmDatabaseManager. The other methods serve to initialize
    dialog when displaying info on existing relation."""


    def __init__(self, plugin, newRelationFirstMember=None, relationToEdit=None):
        """The constructor.

        @param plugin pointer to OSM Plugin object; parent of this object
        @param newRelationFirstMember info on feature (in form: "idSPACEtype") which will be first member of new relation
        @param relationToEdit if relation is given, this dialog is for editing of existing relation, not for creation a new one
        """

        QDialog.__init__(self,None)
        self.setupUi(self)
        self.dockWidget=plugin.dockWidget
        self.plugin=plugin
        self.dbm=plugin.dbm
        self.ur=plugin.undoredo
        self.canvas=plugin.canvas

        # set icons for tool buttons (identify,move,createPoint,createLine,createPolygon)
        self.chooseMemberButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_identify.png"))
        self.removeMemberButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_remove.png"))
        self.removeTagButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_remove.png"))
        self.loadStandardTagsButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_star.png"))
        self.typeInfoButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_questionMark.png"))

        self.info = dict()
        self.newTagLabel = "<new tag here>"
        self.newMemberLabel = "<new member here>"
        self.tagInfoTextEdit.setText("<nothing>")

        if relationToEdit:
            # we are editing existing relation
            self.editing = True
            self.relId = relationToEdit
            self.createRelButton.setText("Save")
            self.setWindowTitle("Edit OSM relation")
        else:
            self.editing = False
            # we are adding new relation
            if newRelationFirstMember:
                ix = newRelationFirstMember.indexOf(" ")
                self.firstMemberType = QString(newRelationFirstMember).left(ix)
                self.firstMemberId = QString(newRelationFirstMember).right(len(newRelationFirstMember)-ix-1)

        self.relTags=[]
        self.relTagsEditIndex=-1
        self.relMembersRoleEditIndex=-1
        self.relMembersTypeEditIndex=-1

        self.connectDlgSignals()

        # clear all dialog items first
        self.clear()

        # load default values to combobox determining relation type
        self.relationTypes=["boundary","multipolygon","restriction","route","enforcement"]
        self.typeCombo.addItem("")
        self.typeCombo.addItems(self.relationTypes)

        if self.editing:
            # we are editing existing relation, we'll load relation data first
            self.loadRelationData(self.relId)
        else:
            if newRelationFirstMember:
                self.addRelationMember(self.firstMemberId,self.firstMemberType,None)

            # enable related tool buttons
            self.removeMemberButton.setEnabled(True)
            self.relMembersLoaded = True

        self.relMembersTable.setSelectionMode(QAbstractItemView.ExtendedSelection)
        self.relTagsTable.setSelectionMode(QAbstractItemView.ExtendedSelection)
        self.relMembersTable.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.relTagsTable.setSelectionBehavior(QAbstractItemView.SelectRows)


    def connectDlgSignals(self):
        """Function connects important dialog signals to appropriate slots.
        """

        # signals on working with tag and member tables
        QObject.connect(self.typeCombo, SIGNAL("currentIndexChanged(const QString &)"), self.__onTypeSelectionChanged)
        QObject.connect(self.relTagsTable, SIGNAL("itemDoubleClicked(QTableWidgetItem*)"), self.__onTagsItemDoubleClicked)
        QObject.connect(self.relMembersTable, SIGNAL("itemDoubleClicked(QTableWidgetItem*)"), self.__onMembersItemDoubleClicked)
        QObject.connect(self.relTagsTable,  SIGNAL("cellChanged(int,int)"), self.__onTagsCellChanged)
        QObject.connect(self.relMembersTable,  SIGNAL("cellChanged(int,int)"), self.__onMembersCellChanged)

        # signals on buttons clicking
        QObject.connect(self.createRelButton, SIGNAL("clicked()"), self.createOrUpdateRelation)
        QObject.connect(self.stornoButton, SIGNAL("clicked()"), self.stornoDialog)
        QObject.connect(self.typeInfoButton, SIGNAL("clicked()"), self.__showTypeInfo)
        QObject.connect(self.loadStandardTagsButton,  SIGNAL("clicked()"), self.loadStandardTags)
        QObject.connect(self.removeTagButton, SIGNAL("clicked()"), self.removeSelectedRelTags)
        QObject.connect(self.removeMemberButton, SIGNAL("clicked()"), self.removeSelectedRelMembers)
        QObject.connect(self.chooseMemberButton,  SIGNAL("clicked()"), self.__startIdentifyingMember)


    def addRelationMember(self,memberId,memberType,memberRole):
        """Function inserts one record into table representing list of all relations' members.
        New record is put to the first place of the list.

        @param memberId identifier of new relation member
        @param memberType type of new relation member
        @param memberRole role of new relation member
        """

        # insert row for new relation member
        self.relMembersTable.insertRow(0)

        self.relMembersTable.setItem(0,0,QTableWidgetItem(str(memberId)))
        self.relMembersTable.setItem(0,1,QTableWidgetItem(str(memberType)))
        self.relMembersTable.setItem(0,2,QTableWidgetItem(str(memberRole)))
        self.relMembersTable.item(0,0).setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled | Qt.ItemIsEditable)
        self.relMembersTable.item(0,1).setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled | Qt.ItemIsEditable)
        self.relMembersTable.item(0,2).setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled | Qt.ItemIsEditable)


    def addRelationTag(self,tagKey,tagValue):
        """Function inserts one record into table representing list of all relations' tags.
        New record is put to the first place of the list.

        @param tagKey key of inserted tag
        @param tagValue value of inserted tag
        """

        # insert row for new relation tag
        self.relTagsTable.insertRow(0)

        self.relTagsTable.setItem(0,0,QTableWidgetItem(tagKey))
        self.relTagsTable.setItem(0,1,QTableWidgetItem(tagValue))
        self.relTagsTable.item(0,0).setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled)
        self.relTagsTable.item(0,1).setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled | Qt.ItemIsEditable)


    def loadRelationData(self,relId):
        """Function fills dialog items with data of given relation.
        Data of relation means its type, tags and members.

        @param relId identifier of relation
        """

        # load tags of specified relation
        self.relTags=self.dbm.getFeatureTags(relId,'Relation')

        rowCount = len(self.relTags)
        for i in range(0,rowCount):
            key = self.relTags[i][0]
            value = self.relTags[i][1]

            if key=="type":
                # tag with key "type" is not shown in relation tags table, there is special combobox for it instead
                ix=self.typeCombo.findText(value)
                if ix<>-1:
                    self.typeCombo.setCurrentIndex(ix)
                else:
                    self.typeCombo.setEditText(value)
            else:
                # all other tags are displayed in tags table
                self.addRelationTag(key,value)


        # fill relation members table with relation members data
        mems=self.dbm.getRelationMembers(relId)

        # printing members
        for i in range(0,len(mems)):
            self.addRelationMember(mems[i][0],mems[i][1],mems[i][2])

        # enable related tool buttons
        self.removeMemberButton.setEnabled(True)

        # set new flags values
        self.relTagsLoaded = True
        self.relMembersLoaded = True


    def __startIdentifyingMember(self):
        """Function enables maptool for identifying relation members directly on map.
        """

        if self.chooseMemberButton.isChecked():
            self.mapTool=OsmIdentifyMT(self.canvas, self.dockWidget, self.dbm)
            self.canvas.setMapTool(self.mapTool)
            self.canvas.setCursor(QCursor(Qt.ArrowCursor))
            self.canvas.setFocus(Qt.OtherFocusReason)
        else:
            self.addRelationMember(self.dockWidget.feature.id(),self.dockWidget.featureType,"")
            self.canvas.unsetMapTool(self.mapTool)
            del self.mapTool
            self.mapTool=None


    def removeSelectedRelTags(self):
        """Function removes all selected tags from relation tags table.
        """

        # remove selected tags (rows)
        selectedItems=self.relTagsTable.selectedItems()
        selectedRowsIndexes=[]
        lastRowIndex=self.relTagsTable.rowCount()-1
        self.relTagsTable.setCurrentCell(lastRowIndex,0)

        for i in selectedItems:
            if i.column()==0 and not i.row()==lastRowIndex:
                selectedRowsIndexes.append(i.row())

        selectedRowsIndexes.sort()
        selectedRowsIndexes.reverse()

        for ix in selectedRowsIndexes:
            key=self.relTagsTable.item(ix,0).text()
            self.relTagsTable.removeRow(ix)


    def removeSelectedRelMembers(self):
        """Function removes all selected members from relation members table.
        """

        # remove selected members (rows)
        selectedItems=self.relMembersTable.selectedItems()
        selectedRowsIndexes=[]
        lastRowIndex=self.relMembersTable.rowCount()-1
        self.relMembersTable.setCurrentCell(lastRowIndex,0)

        for i in selectedItems:
            if i.column()==0 and not i.row()==lastRowIndex:
                selectedRowsIndexes.append(i.row())

        selectedRowsIndexes.sort()
        selectedRowsIndexes.reverse()

        for ix in selectedRowsIndexes:
            key=self.relMembersTable.item(ix,0).text()
            self.relMembersTable.removeRow(ix)


    def __onTagsCellChanged(self,row,column):
        """Function to handle user changes in cells of relations' tags table.

        It's called automatically whenever signal "cellChanged(...)" is emitted.

        @param row index of a row that has changed
        @param column index of a column that has changed
        """

        if not self.relTagsLoaded:
            return

        if row==self.relTagsTable.rowCount()-1:
            # changing value of the last row
            key = self.relTagsTable.item(row,0).text()
            if key=="" or key==self.newTagLabel:
                return

            # adding new tag to table
            if column==0:
                newLastRow = row+1
                self.relTagsTable.setRowCount(row+2)
                self.relTagsTable.setItem(newLastRow,0,QTableWidgetItem(self.newTagLabel))
                self.relTagsTable.setItem(newLastRow,1,QTableWidgetItem(""))
                self.relTagsTable.item(newLastRow,0).setFlags(Qt.ItemIsEnabled | Qt.ItemIsEditable)
                self.relTagsTable.item(newLastRow,1).setFlags(Qt.ItemIsEnabled)

                self.relTagsTable.item(row,0).setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled)
                self.relTagsTable.item(row,1).setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled | Qt.ItemIsEditable)


    def __onMembersCellChanged(self,row,column):
        """Function to handle user changes in cells of relations' members table.

        it's called automatically whenever signal "cellChanged(...)" is emitted.

        @param row index of a row that has changed
        @param column index of a column that has changed
        """

        if not self.relMembersLoaded:
            return

        if row==self.relMembersTable.rowCount()-1:
            # changing value of the last row
            memberId = self.relMembersTable.item(row,0).text()
            if memberId=="" or memberId==self.newMemberLabel:
                return

            # adding new member
            if column==0:
                newLastRow = row+1
                self.relMembersTable.setRowCount(row+2)
                self.relMembersTable.setItem(newLastRow,0,QTableWidgetItem(self.newMemberLabel))
                self.relMembersTable.setItem(newLastRow,1,QTableWidgetItem(""))
                self.relMembersTable.setItem(newLastRow,2,QTableWidgetItem(""))
                self.relMembersTable.item(newLastRow,0).setFlags(Qt.ItemIsEnabled | Qt.ItemIsEditable)
                self.relMembersTable.item(newLastRow,1).setFlags(Qt.ItemIsEnabled)
                self.relMembersTable.item(newLastRow,2).setFlags(Qt.ItemIsEnabled)

                self.relMembersTable.item(row,0).setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled | Qt.ItemIsEditable)
                self.relMembersTable.item(row,1).setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled | Qt.ItemIsEditable)
                self.relMembersTable.item(row,2).setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled | Qt.ItemIsEditable)


    def clear(self):
        """Function removes content of all dialog items (with one exception) and set them to default values.

        The only exception is combobox determining relation type.
        It stays untouched.
        """

        # clear table of relation properties(tags) and all related buttons
        self.relTagsTable.clear()
        self.removeTagButton.setEnabled(True)

        # clear table of relation members and all related buttons
        self.relMembersTable.clear()
        self.chooseMemberButton.setEnabled(True)
        self.removeMemberButton.setEnabled(False)

        # clear information panel
        self.tagInfoTextEdit.setText("<nothing>")
        self.tagInfoTextEdit.setEnabled(False)

        # set loading flags to false
        self.relTagsLoaded = True
        self.relMembersLoaded = True

        # load default data into relation members table
        self.relMembersTable.setColumnCount(3)
        self.relMembersTable.setHorizontalHeaderItem(0,QTableWidgetItem("Id"))
        self.relMembersTable.setHorizontalHeaderItem(1,QTableWidgetItem("Type"))
        self.relMembersTable.setHorizontalHeaderItem(2,QTableWidgetItem("Role"))

        self.relMembersTable.setRowCount(1)
        self.relMembersTable.setItem(0,0,QTableWidgetItem(self.newMemberLabel))
        self.relMembersTable.setItem(0,1,QTableWidgetItem(""))
        self.relMembersTable.setItem(0,2,QTableWidgetItem(""))
        self.relMembersTable.item(0,0).setFlags(Qt.ItemIsEnabled | Qt.ItemIsEditable)
        self.relMembersTable.item(0,1).setFlags(Qt.ItemIsEnabled)
        self.relMembersTable.item(0,2).setFlags(Qt.ItemIsEnabled)

        # load default data into relation tags table
        self.relTagsTable.setColumnCount(2)
        self.relTagsTable.setHorizontalHeaderItem(0,QTableWidgetItem("Key"))
        self.relTagsTable.setHorizontalHeaderItem(1,QTableWidgetItem("Value"))

        self.relTagsTable.setRowCount(1)
        self.relTagsTable.removeCellWidget(0,1)
        self.relTagsTable.setItem(0,0,QTableWidgetItem(self.newTagLabel))
        self.relTagsTable.setItem(0,1,QTableWidgetItem(""))
        self.relTagsTable.item(0,0).setFlags(Qt.ItemIsEnabled | Qt.ItemIsEditable)
        self.relTagsTable.item(0,1).setFlags(Qt.ItemIsEnabled)


    def loadStandardTags(self):
        """Function clears relations' tags table and then loads tags that are typical
        for chosen relation type.

        This provides good way how to help users with creating standard relations.
        User doesn't need to find out what tags are usually used for some relation.
        """

        # clear relation tags table first
        self.relTagsTable.clear()
        self.relTagsTable.setColumnCount(2)
        self.relTagsTable.setHorizontalHeaderItem(0,QTableWidgetItem("Key"))
        self.relTagsTable.setHorizontalHeaderItem(1,QTableWidgetItem("Value"))
        self.relTagsTable.setRowCount(1)
        self.relTagsTable.removeCellWidget(0,1)
        self.relTagsTable.setItem(0,0,QTableWidgetItem(self.newTagLabel))
        self.relTagsTable.setItem(0,1,QTableWidgetItem(""))
        self.relTagsTable.item(0,0).setFlags(Qt.ItemIsEnabled | Qt.ItemIsEditable)
        self.relTagsTable.item(0,1).setFlags(Qt.ItemIsEnabled)

        # find tags that are recommended for chosen relation type
        self.relTags = self.determineSuitableTags(self.typeCombo.currentText())

        # put found tags into relation tags table
        rowCount = len(self.relTags)
        for i in range(0,rowCount):
            self.addRelationTag(list(self.relTags)[i],"")

        # set variables and enable buttons for better manipulation with table
        self.removeTagButton.setEnabled(True)
        self.relTagsLoaded = True


    def __onTypeSelectionChanged(self,typeName):
        """Function is called after currentIndexChanged(...) signal is emitted on "RELATION TYPE" combobox.

        That means user select new relation type. Either one of predefined types or a new one.

        Function doesn't perform change of relation type this time. Everything will be done
        after submiting the whole dialog.

        @param typeName name of new selected type
        """

        # if non-standard typename was set up, loadStandardTagsButton is useless
        if typeName.toAscii().data() in self.relationTypes:
            self.loadStandardTagsButton.setEnabled(True)
        else:
            self.loadStandardTagsButton.setEnabled(False)


    def determineSuitableMemberRoles(self,relType):
        """Function is used to find typical member roles to given relation type.
        With help of this function plugin gives advice to user on relation creation.

        @param relType name of relation type
        @return list of typical roles to given type of relation
        """

        roles = []
        if relType=="boundary":
            roles = ["enclave","exclave"]
        elif relType=="multipolygon":
            roles = ["outer","inner"]
        elif relType=="restriction":
            roles = ["from","to","via","location_hint"]
        elif relType=="route":
            roles = ["forward","backward","stop_0","stop_1","stop_2","stop_3","stop_4","stop_5","stop_6","stop_7","stop_8","stop_9"
                    ,"forward_stop_0","forward_stop_1","forward_stop_2","forward_stop_3","forward_stop_4","forward_stop_5","forward_stop_6"
                    ,"forward_stop_7","forward_stop_8","forward_stop_9","backward_stop_0","backward_stop_1","backward_stop_2"
                    ,"backward_stop_3","backward_stop_4","backward_stop_5","backward_stop_6","backward_stop_7","backward_stop_8","backward_stop_9"]
        elif relType=="enforcement":
            roles = []
        return roles


    def determineSuitableTags(self,relType):
        """Function is used to find typical tags to given relation type.
        With of this function plugin gives advice to user on relation creation.

        @param relType name of relation type
        @return list of typical tags to given type of relation
        """

        tags = []
        if relType=="boundary":
            tags = dict(
                boundary='For a real boundary (sometimes in the middle of a river or 12 Miles away from coastline).'
                ,land_area='For coastline and real boundaries on land.'
                ,name=''
                ,admin_level=''
                )
        elif relType=="multipolygon":
            tags = dict()
        elif relType=="restriction":
            tags = {
                "restriction":"If the first word is \"no\", then no routing is possible from the \"from\" to the \"to\" member, and if it is \"only_\", then you know that the only routing originating from the \"from\" member leads to the \"to\" member. The \"from\" and \"to\" members must start/end at the \"via\" node (see 1)."
                ,"except":"The restriction does not apply to these vehicle types (possible more than one: except=bicycle;psv)"
                ,"day_on":"For example, no right turn in the morning peak on weekdays might be day_on=Monday;day_off=Friday;hour_on=07:30;hour_off=09:30."
                ,"day_off":""
                ,"hour_on":""
                ,"hour_off":""
                }
        elif relType=="route":
            tags = dict(
                route='A road (e.g. the ways making up the A14 trunk road), bicycle route, hiking route or whatever route.'
                ,name='The route is known by this name (e.g. "Jubilee Cycle Route", "Pembrokeshire Coastal Path").'
                ,ref='The route is known by this reference (e.g. "A14", "NCN 11", "Citi 4" (bus number); in germany there is always a space between character and number, e.g. "A 1", "L 130", "K 5"; in france too).'
                ,network='A wider network of routes of which this is one example. For example, the UKs national cycle network; the Cambridge Citi bus network; the UKs long distance footpath network. (The "uk_" bit isnt particularly to identify it as belonging to the uk, merely to be a conventional way to separate the namespace.)'
                ,operator='The route is operated by this authority/company etc. e.g. "Stagecoach Cambridge", "Eurostar".'
                ,state='Sometimes routes may not be permanent (ie: diversions), or may be in a proposed state (ie: UK NCN routes are sometimes not official routes pending some negotiation or development). Connection is used for routes linking two different routes or linking a route with for example a village centre.'
                ,symbol='Describes the symbol that is used to mark the way along the route, e.g., "Red cross on white ground" for the "Frankenweg" in Franconia, Germany.'
                ,color='(optional) Color code noted in hex triplet format. Especially useful for public transport routes. Example: "#008080" for teal color.'
                ,description='Description tells what is special about this route.'
                ,distance='(optional) The distance covered by this route, if known. For information of users and automatic evaluation e.g. of completeness. Given including a unit and with a dot for decimals. (e.g. "12.5km")'
                )
        elif relType=="enforcement":
            tags = dict()
        return tags


    def determineSuitableTagValues(self,relType,tagKey):
        """Function is used to find typical tag values for given relation type and given key.
        With help of this function plugin gives advice to user on relation creation.

        @param relType name of relation type
        @param tagKey key of tag
        @return list of typical tag values to given relation type
        """

        vals = []
        if relType=="boundary":
            if tagKey=="boundary":
                vals = ["administrative","national_park","political","civil"]
            elif tagKey=="land_area":
                vals = ["administrative"]
            elif tagKey=="admin_level":
                vals = ["1","2","3","4","5","6","7","8","9","10","11"]
        elif relType=="restriction":
            if tagKey=="restriction":
                vals = ["no_right_turn","no_left_turn","no_u_turn","no_straight_on","only_right_turn","only_left_turn","only_straight_on"]
            elif tagKey=="except":
                vals = ["psv","bicycle","hgv","motorcar"]
        elif relType=="route":
            if tagKey=="route":
                vals = ["road","bicycle","foot","hiking","bus","pilgrimage","detour","railway","tram","mtb","roller_skate","running","horse"]
            elif tagKey=="network":
                vals = ["ncn","rcn","lcn","uk_ldp","lwn","rwn","nwn","e-road"]
            elif tagKey=="state":
                vals = ["proposed","alternate","temporary","connection"]
        return vals


    def createRelation(self):
        """Function creates new OSM relation from dialog data.
        It performs commit.
        """

        # collect relation members data into a list
        relMems=[]
        for i in range(0,self.relMembersTable.rowCount()-1):    # except from the last row
            memId = self.relMembersTable.item(i,0).text().toUtf8().data()
            memType = self.relMembersTable.item(i,1).text().toUtf8().data()
            memRole = self.relMembersTable.item(i,2).text().toUtf8().data()

            relMems.append((memId,memType,memRole))

        # find out relation type
        relType=self.typeCombo.currentText().toUtf8().data()

        # call relation creation
        self.ur.startAction("Create relation.")
        relId=self.dbm.createRelation(relType,relMems)

        relTags=[]
        for i in range(0,self.relTagsTable.rowCount()-1):    # except from the last row
            key=self.relTagsTable.item(i,0).text().toUtf8().data()
            val=self.relTagsTable.item(i,1).text().toUtf8().data()

            relTags.append((key,val))

        # relation type has to be stored as tag too
        relTags.append(('type',relType))

        # insert relation tags
        self.dbm.insertTags(relId,'Relation',relTags)

        # make actions persistent
        self.dbm.commit()
        self.ur.stopAction()


    def updateRelation(self):
        """Function updates existing OSM relation.
        It performs commit.

        Relation is not given in parameter; it's in member variable of this class.
        """

        self.ur.startAction("Update relation.")

        # find out relation type and change it
        relType=self.typeCombo.currentText().toUtf8().data()
        self.dbm.changeRelationType(self.relId,relType)

        # remove all relation tags and members
        self.dbm.removeFeaturesTags(self.relId,'Relation')

        # collect relation members into a list
        relMems=[]
        for i in range(0,self.relMembersTable.rowCount()-1):    # except from the last row
            memId = self.relMembersTable.item(i,0).text().toUtf8().data()
            memType = self.relMembersTable.item(i,1).text().toUtf8().data()
            memRole = self.relMembersTable.item(i,2).text().toUtf8().data()

            relMems.append((memId,memType,memRole))

        # remove old members and insert new ones
        self.dbm.changeAllRelationMembers(self.relId,relMems)

        # collect relation tags into a list
        relTags=[]
        for i in range(0,self.relTagsTable.rowCount()-1):    # except from the last row
            key=self.relTagsTable.item(i,0).text().toUtf8().data()
            val=self.relTagsTable.item(i,1).text().toUtf8().data()

            relTags.append((key,val))

        # relation type has to be stored as tag too
        relTags.append(('type',relType))

        # insert relation tags
        self.dbm.insertTags(self.relId,'Relation',relTags)

        # make actions persistent
        self.ur.stopAction()
        self.dbm.commit()


    def createOrUpdateRelation(self):
        """Function starts process of creation of new OSM relation from dialog data.
        When in editing mode function updates opened relation instead.
        """

        if not self.editing:
            # lets create new relation from predefined information
            self.createRelation()
        else:
            # lets update existing relation
            self.updateRelation()

        # close addRelation dialog
        self.close()

        #load features' relations info into dockWidget again
        if self.dockWidget.feature:
            self.dockWidget.reloadFeatureRelations()


    def stornoDialog(self):
        """Function just cancels the whole dialog.
        It is called after clicked() signal is emitted on "Storno" button.
        """

        # close addRelation dialog
        self.close()


    def __onTagsItemDoubleClicked(self,item):
        """Function is called after itemDoubleClicked(...) signal is emitted on table of relation tags.

        It shows combobox with possible values for given item of table.

        @param item item of table of relation tags
        """

        if item.column()==0:
            return

        if self.relTagsEditIndex<>None:
            row=self.relTagsEditIndex
            if row<>-1:
                value=self.relTagsTable.cellWidget(row,1).currentText()
                self.relTagsTable.item(row,1).setText(value)
                self.relTagsTable.removeCellWidget(row,1)

        tagValues = self.determineSuitableTagValues(self.typeCombo.currentText(),self.relTagsTable.item(item.row(),0).text())
        if len(tagValues)>0:
            valCombo=QComboBox()
            valCombo.setEditable(True)
            valCombo.addItem("")
            valCombo.addItems(tagValues)
            ix=valCombo.findText(self.relTagsTable.item(item.row(),1).text())
            valCombo.setCurrentIndex(ix)

            self.relTagsTable.setCellWidget(item.row(),1,valCombo)
            self.relTagsEditIndex=item.row()
            QObject.connect(valCombo, SIGNAL("currentIndexChanged(const QString &)"), self.__onValueSelectionChanged)


    def __onMembersItemDoubleClicked(self,item):
        """Function is called after itemDoubleClicked(...) signal is emitted on table of relation members.

        It shows combobox with possible values for given item of table.

        @param item item of table of relation members
        """

        if (item.column()==0) or (item.row()==self.relMembersTable.rowCount()-1):
            return

        if item.column()==2:

            if self.relMembersRoleEditIndex<>None:
                row=self.relMembersRoleEditIndex
                if row<>-1:
                    role=self.relMembersTable.cellWidget(row,2).currentText()
                    self.relMembersTable.item(row,2).setText(role)
                    self.relMembersTable.removeCellWidget(row,2)

            memberRoles = self.determineSuitableMemberRoles(self.typeCombo.currentText())
            if len(memberRoles)>0:
                rolesCombo=QComboBox()
                rolesCombo.setEditable(True)
                rolesCombo.addItem("")
                rolesCombo.addItems(memberRoles)
                ix=rolesCombo.findText(self.relMembersTable.item(item.row(),1).text())
                rolesCombo.setCurrentIndex(ix)

                self.relMembersTable.setCellWidget(item.row(),2,rolesCombo)
                self.relMembersRoleEditIndex=item.row()
                QObject.connect(rolesCombo, SIGNAL("currentIndexChanged(const QString &)"), self.__onRoleSelectionChanged)

        elif item.column()==1:

            if self.relMembersTypeEditIndex<>None:
                row=self.relMembersTypeEditIndex
                if row<>-1:
                    memType=self.relMembersTable.cellWidget(row,1).currentText()
                    self.relMembersTable.item(row,1).setText(memType)
                    self.relMembersTable.removeCellWidget(row,1)

            memberTypes=["Point","Line","Polygon","Relation"]
            memTypesCombo=QComboBox()
            memTypesCombo.setEditable(True)
            memTypesCombo.addItem("")
            memTypesCombo.addItems(memberTypes)
            ix=memTypesCombo.findText(self.relMembersTable.item(item.row(),1).text())
            memTypesCombo.setCurrentIndex(ix)

            self.relMembersTable.setCellWidget(item.row(),1,memTypesCombo)
            self.relMembersTypeEditIndex=item.row()
            QObject.connect(memTypesCombo, SIGNAL("currentIndexChanged(const QString &)"), self.__onMemTypeSelectionChanged)


    def __onValueSelectionChanged(self,value):
        """Function is called after currentIndexChanged(...) signal is emitted on combobox of table item.
        This combobox is related to table of relation tags (column Value).

        @param value new current value in combobox
        """

        row=self.relTagsEditIndex
        self.relTagsTable.item(row,1).setText(value)
        self.relTagsTable.removeCellWidget(row,1)
        self.relTagsEditIndex=-1


    def __onRoleSelectionChanged(self,role):
        """Function is called after currentIndexChanged(...) signal is emitted on combobox of table item.
        This combobox is related to table of relation members (column Role).

        @param role new current value in combobox
        """

        row=self.relMembersRoleEditIndex
        self.relMembersTable.item(row,2).setText(role)
        self.relMembersTable.removeCellWidget(row,2)
        self.relMembersRoleEditIndex=-1


    def __onMemTypeSelectionChanged(self,memType):
        """Function is called after currentIndexChanged(...) signal is emitted on combobox of table item.
        This combobox is related to table of relation members (column Type).

        @param memType new current value in combobox
        """

        row=self.relMembersTypeEditIndex
        self.relMembersTable.item(row,1).setText(memType)
        self.relMembersTable.removeCellWidget(row,1)
        self.relMembersTypeEditIndex=-1


    def __showTypeInfo(self):
        """Function shows messagebox with brief information on currently selected relation type.
        """

        typeName = self.typeCombo.currentText()
        info = ""

        if typeName=="boundary":
            info = "for grouping boundaries and marking enclaves / exclaves"
        elif typeName=="multipolygon":
            info = "to put holes into areas (might have to be renamed, see article)"
        elif typeName=="restriction":
            info = "any kind of turn restriction"
        elif typeName=="route":
            info = "like bus routes, cycle routes and numbered highways"
        elif typeName=="enforcement":
            info = "traffic enforcement devices; speed cameras, redlight cameras, weight checks, ..."

        QMessageBox.information(self, self.tr("OSM Information")
                        ,self.tr(info))



