"""@package DockWidget
This module is descendant of "OSM Feature" dockable widget and makes user able
to view and edit information on selected OSM feature.

DockWidget module shows details of selected feature - its basic info, tags and relations.
It provides methods for editing features' tags so that user can edit them directly on the widget.

There are also some identify and edit buttons on "DockWidget" widget - this modul implements all the methods that are called
after clicking on these buttons. Such methods creates (and set) map tool that coresponds to specified button.
"""


from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from DockWidget_ui import Ui_OsmDockWidget
from DlgAddRelation import DlgAddRelation

# include all osm map tools
from map_tools.CreatePointMapTool import CreatePointMapTool
from map_tools.CreateLineMapTool import CreateLineMapTool
from map_tools.CreatePolygonMapTool import CreatePolygonMapTool
from map_tools.MoveMapTool import MoveMapTool
from map_tools.IdentifyMapTool import IdentifyMapTool


class DockWidget(QDockWidget, Ui_OsmDockWidget,  object):
    """This class shows details of selected feature - its basic info, tags and relations.

    It provides methods for editing features' tags so that user can edit them directly on the widget.

    There are also some identify and edit buttons on "DockWidget" widget - this modul implements all the methods that are called
    after clicking them. Such methods creates (and set) map tool that coresponds to specified button.
    """


    def __init__(self, plugin):
        """The constructor."""

        QDockWidget.__init__(self, None)
        self.setupUi(self)
        self.setAllowedAreas(Qt.LeftDockWidgetArea | Qt.RightDockWidgetArea)

        self.plugin=plugin
        self.__mapTool=None
        self.__dlgAddRel=None

        # set icons for tool buttons (identify,move,createPoint,createLine,createPolygon)
        self.identifyButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_identify.png"))
        self.moveButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_move.png"))
        self.createPointButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_createPoint.png"))
        self.createLineButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_createLine.png"))
        self.createPolygonButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_createPolygon.png"))
        self.createRelationButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_createRelation.png"))
        self.removeButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_remove.png"))
        self.deleteTagsButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_remove.png"))
        self.undoButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_undo.png"))
        self.redoButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_redo.png"))

        # initializing group of edit buttons
        self.toolButtons=QButtonGroup(self)
        self.dummyButton.setVisible(False)
        self.toolButtons.addButton(self.dummyButton)
        self.toolButtons.addButton(self.identifyButton)
        self.toolButtons.addButton(self.moveButton)
        self.toolButtons.addButton(self.createPointButton)
        self.toolButtons.addButton(self.createLineButton)
        self.toolButtons.addButton(self.createPolygonButton)
        self.toolButtons.setExclusive(True)

        # initializing table of feature tags
        self.tagTable.setColumnCount(2)
        self.tagTable.setHorizontalHeaderItem(0,QTableWidgetItem("Key"))
        self.tagTable.setHorizontalHeaderItem(1,QTableWidgetItem("Value"))
        self.tagTable.setSelectionMode(QAbstractItemView.ExtendedSelection)
        self.tagTable.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.newTagLabel="<new tag here>"
        self.relTagsTree.setSelectionMode(QAbstractItemView.NoSelection)

        # initializing rubberbands/vertexmarkers; getting qgis settings of line width and color for rubberbands
        settings=QSettings()
        qgsLineWidth=settings.value( "/qgis/digitizing/line_width", QVariant(10) ).toInt()
        qgsLineRed=settings.value( "/qgis/digitizing/line_color_red", QVariant(255) ).toInt()
        qgsLineGreen=settings.value( "/qgis/digitizing/line_color_green", QVariant(0) ).toInt()
        qgsLineBlue=settings.value( "/qgis/digitizing/line_color_blue", QVariant(0) ).toInt()

        self.rubBandPol=QgsRubberBand(plugin.canvas,True)
        self.rubBandPol.setColor(QColor(qgsLineRed[0],qgsLineGreen[0],qgsLineBlue[0]))
        self.rubBandPol.setWidth(qgsLineWidth[0])

        self.rubBand=QgsRubberBand(plugin.canvas,False)
        self.rubBand.setColor(QColor(qgsLineRed[0],qgsLineGreen[0],qgsLineBlue[0]))
        self.rubBand.setWidth(qgsLineWidth[0])

        self.verMarker=QgsVertexMarker(plugin.canvas)
        self.verMarker.setIconType(2)
        self.verMarker.setIconSize(13)
        self.verMarker.setColor(QColor(qgsLineRed[0],qgsLineGreen[0],qgsLineBlue[0]))
        self.verMarker.setPenWidth(qgsLineWidth[0])
        self.verMarkers=[]

        self.relRubBandPol=QgsRubberBand(plugin.canvas,True)
        self.relRubBandPol.setColor(QColor(qgsLineRed[0],50,50))
        self.relRubBandPol.setWidth(qgsLineWidth[0]+4)

        self.relRubBand=QgsRubberBand(plugin.canvas,False)
        self.relRubBand.setColor(QColor(qgsLineRed[0],50,50))
        self.relRubBand.setWidth(qgsLineWidth[0]+4)

        self.relVerMarker=QgsVertexMarker(plugin.canvas)
        self.relVerMarker.setIconType(2)
        self.relVerMarker.setIconSize(13)
        self.relVerMarker.setColor(QColor(qgsLineRed[0],50,50))
        self.relVerMarker.setPenWidth(qgsLineWidth[0])

        # initializing inner variables
        self.activeEditButton=self.dummyButton
        self.__tagsLoaded=False
        self.__relTagsLoaded=False
        self.feature=None
        self.featureType=None
        self.featRels=[]
        self.featRelTags=[]
        self.featRelMembers=[]

        # clear all widget items
        self.clear()

        self.__connectWidgetSignals()

        self.removeButton.setEnabled(False)
        self.createRelationButton.setCheckable(False)

        # set current tab to "Properties"
        self.propRelBox.setCurrentIndex(0)
        self.plugin.canvas.setFocus(Qt.OtherFocusReason)

        # init coordinate transform
        self.projectionChanged()

        renderer=self.plugin.canvas.mapRenderer()
        self.connect(renderer, SIGNAL("hasCrsTransformEnabled(bool)"), self.projectionChanged)
        self.connect(renderer, SIGNAL("destinationSrsChanged()"), self.projectionChanged)


    def setContentEnabled(self,flag):

        self.featInfoBox.setEnabled(flag)
        self.propRelBox.setEnabled(flag)
        self.identifyButton.setEnabled(flag)
        self.moveButton.setEnabled(flag)
        self.createPointButton.setEnabled(flag)
        self.createLineButton.setEnabled(flag)
        self.createPolygonButton.setEnabled(flag)
        self.createRelationButton.setEnabled(flag)

        if flag:
            if self.plugin.undoredo.undoCounter>0:
                self.undoButton.setEnabled(True)
            if self.plugin.undoredo.redoCounter>0:
                self.redoButton.setEnabled(True)
        else:
            self.undoButton.setEnabled(False)
            self.redoButton.setEnabled(False)

        self.urDetailsButton.setEnabled(flag)


    def projectionChanged(self):
        """Function is connected to signals from QgsMapRenderer.
        It updates coordinate transforms.
        """

        renderer = self.plugin.canvas.mapRenderer()
        if renderer.hasCrsTransformEnabled():
          self.coordXform = QgsCoordinateTransform(renderer.destinationSrs(), QgsCoordinateReferenceSystem(4326))
        else:
          self.coordXform = None


    def canvasToOsmCoords(self, point):
        """Performs conversion from canvas to map coordinates.

        @param point canvas coordinates to convert
        """
        point = self.plugin.canvas.getCoordinateTransform().toMapCoordinates( point )

        # optional conversion from map to layer coordinates
        if self.coordXform is not None:
            point = self.coordXform.transform( point )

        return point


    def __connectWidgetSignals(self):
        """Function connects all neccessary signals to appropriate slots.
        """

        # signals emitted on clicking with tag and member tables
        QObject.connect(self.identifyButton,  SIGNAL("clicked()"), self.__startIdentifyingFeature)
        QObject.connect(self.moveButton,  SIGNAL("clicked()"), self.__startMovingFeature)
        QObject.connect(self.createPointButton,  SIGNAL("clicked()"), self.__startPointCreation)
        QObject.connect(self.createLineButton,  SIGNAL("clicked()"), self.__startLineCreation)
        QObject.connect(self.createPolygonButton,  SIGNAL("clicked()"), self.__startPolygonCreation)
        QObject.connect(self.createRelationButton,  SIGNAL("clicked()"), self.__createRelation)
        QObject.connect(self.removeButton, SIGNAL("clicked()"), self.removeFeature)
        QObject.connect(self.relListWidget, SIGNAL("currentRowChanged(int)"), self.loadRelationStuff)
        QObject.connect(self.relMembersList, SIGNAL("currentRowChanged(int)"), self.__showRelMemberOnMap)
        QObject.connect(self.addRelationButton,  SIGNAL("clicked()"), self.createRelationWithMember)
        QObject.connect(self.editRelationButton,  SIGNAL("clicked()"), self.editSelectedRelation)
        QObject.connect(self.removeRelationButton,  SIGNAL("clicked()"), self.removeSelectedRelation)
        QObject.connect(self.deleteTagsButton, SIGNAL("clicked()"), self.removeSelectedTags)
        QObject.connect(self.tagTable, SIGNAL("cellChanged(int,int)"), self.__onTagsCellChanged)
        QObject.connect(self.tagTable, SIGNAL("currentCellChanged(int,int,int,int)"), self.__onCurrentCellChanged)
        QObject.connect(self.tagTable, SIGNAL("itemDoubleClicked(QTableWidgetItem*)"), self.__onTagsItemDoubleClicked)
        QObject.connect(self.undoButton,  SIGNAL("clicked()"), self.__undo)
        QObject.connect(self.redoButton, SIGNAL("clicked()"), self.__redo)
        QObject.connect(self.urDetailsButton, SIGNAL("clicked()"), self.__urDetailsChecked)


    def databaseChanged(self,dbKey):
        """This function is called when current OSM database of plugin changes.
        The DockWidget performs neccessary actions and tells current map tool about the change.

        @param dbKey key (name) of new current database
        """

        if self.__dlgAddRel and dbKey:

            self.__dlgAddRel.close()
            self.__dlgAddRel=None
            self.setContentEnabled(True)
            self.plugin.undoredo.setContentEnabled(True)

            QMessageBox.information(self, self.tr("OSM Plugin")
                ,self.tr("The 'Create OSM Relation' dialog was closed automatically because current OSM database was changed."))

        # clear the whole OSM Feature dockwidget as well as all related rubberbands and vertex markers
        self.clear()

        # if some mapTool is currently set tell it about database changing
        if self.__mapTool:
            self.__mapTool.databaseChanged(dbKey)
            self.activeEditButton=self.dummyButton

        # and if new database is None, disable the whole dockwidget
        if not dbKey:
            self.setContentEnabled(False)
            if self.__mapTool:
                self.plugin.canvas.unsetMapTool(self.__mapTool)
                self.__mapTool=None
                self.plugin.canvas.setCursor(QCursor(Qt.ArrowCursor))
            return

        self.setContentEnabled(True)


    def clear(self):
        """Function clears all widget items.
        It resets rubberbands, vertexmarkers, re-initializes DockWidget inner structures.
        """

        # clear common feature infos
        self.typeIdLabel.setText("")
        self.userLabel.setText("")
        self.createdLabel.setText("")

        # clear table with information about feature's tags
        self.tagTable.clear()
        self.tagTable.setEnabled(False)
        self.tagTable.setRowCount(0)
        self.tagTable.setColumnCount(0)

        # clear table with info about feature's relations
        self.relListWidget.clear()
        self.relTagsTree.clear()
        self.relMembersList.clear()
        self.relTagsTree.setColumnCount(0)

        self.relListWidget.setEnabled(False)
        self.relTagsTree.setEnabled(False)
        self.relMembersList.setEnabled(False)

        # disable widget buttons
        self.deleteTagsButton.setEnabled(False)
        self.editRelationButton.setEnabled(False)
        self.removeRelationButton.setEnabled(False)
        self.addRelationButton.setEnabled(False)
        self.removeButton.setEnabled(False)

        # remove previous rubber bands
        self.rubBand.reset(False)
        self.rubBandPol.reset(True)
        self.verMarker.setCenter(QgsPoint(-1000,-1000))
        self.verMarker.hide()
        self.__removeMemberMarkers()
        self.verMarkers=[]

        self.relRubBand.reset(False)
        self.relRubBandPol.reset(True)
        self.relVerMarker.setCenter(QgsPoint(-1000,-1000))
        self.relVerMarker.hide()

        # clear member variables
        self.__tagsLoaded=False
        self.__relTagsLoaded=False
        self.feature=None
        self.featureType=None
        self.featRels=[]
        self.featRelTags=[]
        self.featRelMembers=[]


    def __createRelation(self):
        """Function calls relation creating process.
        Creation is started by displaying appropriate dialog.
        """

        # clear dockwidget
        self.clear()
        self.plugin.iface.mainWindow().statusBar().showMessage("")

        self.plugin.canvas.unsetMapTool(self.__mapTool)
        del self.__mapTool
        self.__mapTool=None

        self.plugin.canvas.setCursor(QCursor(Qt.ArrowCursor))
        self.activeEditButton=self.dummyButton

        self.setContentEnabled(False)
        self.plugin.undoredo.setContentEnabled(False)
        self.plugin.toolBar.setEnabled(False)

        # DlgAddRelation parameters: plugin, newRelationFirstMember, relationToEdit
        self.__dlgAddRel=DlgAddRelation(self.plugin, None, None)
        self.__dlgAddRel.setWindowModality(Qt.WindowModal)
        self.__dlgAddRel.exec_()
        self.__dlgAddRel=None

        self.setContentEnabled(True)
        self.plugin.undoredo.setContentEnabled(True)
        self.plugin.toolBar.setEnabled(True)


    def createRelationWithMember(self):
        """Function calls relation creating process. Creation is started by displaying appropriate dialog.
        Function pre-fills dialog with information on currently loaded feature.
        """

        self.__dlgAddRel=DlgAddRelation(self.plugin, QString(self.featureType+" %1").arg(self.feature.id()), None)

        # clear dockwidget
        self.clear()
        self.plugin.iface.mainWindow().statusBar().showMessage("")

        self.plugin.canvas.unsetMapTool(self.__mapTool)
        del self.__mapTool
        self.__mapTool=None

        self.plugin.canvas.setCursor(QCursor(Qt.ArrowCursor))
        self.activeEditButton=self.dummyButton

        self.setContentEnabled(False)
        self.plugin.undoredo.setContentEnabled(False)
        self.plugin.toolBar.setEnabled(False)

        self.__dlgAddRel.setWindowModality(Qt.WindowModal)
        self.__dlgAddRel.exec_()
        self.__dlgAddRel=None

        self.setContentEnabled(True)
        self.plugin.undoredo.setContentEnabled(True)
        self.plugin.toolBar.setEnabled(True)


    def editSelectedRelation(self):
        """Function calls editing of a relation. Editing is started by displaying appropriate dialog.
        Relation identifier is not passed to this function. Function has to find it out from current row
        of appropriate list widget.
        """

        # show modal dialog "Edit relation"
        if not self.feature:
            QMessageBox.information(self, self.tr("OSM Feature Dock Widget"), self.tr("Choose OSM feature first."))
            return

        item=self.relListWidget.item(self.relListWidget.currentRow())
        if not item:
            QMessageBox.information(self, self.tr("OSM Feature Dock Widget"), self.tr("Choose relation for editing first."))
            return

        relId=self.featRels[self.relListWidget.currentRow()]

        self.setContentEnabled(False)
        self.plugin.undoredo.setContentEnabled(False)
        self.plugin.toolBar.setEnabled(False)

        self.__dlgAddRel=DlgAddRelation(self.plugin, None, relId)
        self.__dlgAddRel.setWindowModality(Qt.WindowModal)
        self.__dlgAddRel.exec_()
        self.__dlgAddRel=None

        self.setContentEnabled(True)
        self.plugin.undoredo.setContentEnabled(True)
        self.plugin.toolBar.setEnabled(True)


    def __onTagsCellChanged(self,row,column):
        """Function is called after cellChanged(int,int) signal is emitted on table of all features' relations.
        It means that user is changed key or value of some existing tag.

        @param row index of row in table of tags
        @param column index of column in table of tags
        """

        if not self.__tagsLoaded:
            # this signal was emitted during table initialization,
            # but we are interested in user actions only
            return

        if row<self.tagTable.rowCount()-1:

            # changing value of tag that already exists
            key=self.tagTable.item(row,0).text()
            value=self.tagTable.item(row,1).text()

            # store tag's change into database
            self.plugin.undoredo.startAction("Change tag value.")
            self.plugin.dbm.changeTagValue(self.feature.id(),self.featureType,key.toUtf8().data(),value.toUtf8().data())
        else:
            key = self.tagTable.item(row,0).text()
            if key=="" or key==self.newTagLabel:
                return

            # adding new tag and setting its key
            if column==0:

                # store it into database
                isAlreadyDef=self.plugin.dbm.isTagDefined(self.feature.id(),self.featureType,key.toUtf8().data())
                if isAlreadyDef:
                    # such a key already exists for this relation
                    self.tagTable.setItem(row,0,QTableWidgetItem(self.newTagLabel))
                    QMessageBox.information(self, self.tr("OSM Feature Dock Widget")
                        ,self.tr("Property '%1' cannot be added twice.").arg(key.toUtf8().data()))
                    return

                # well, insert new tag into database
                self.plugin.undoredo.startAction("Insert new tag.")
                self.plugin.dbm.insertTag(self.feature.id(),self.featureType,key.toUtf8().data(),'')

                self.__tagsLoaded=False

                self.tagTable.item(row,0).setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled)
                self.tagTable.item(row,1).setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled | Qt.ItemIsEditable)

                newLastRow = row+1
                self.tagTable.setRowCount(row+2)
                self.tagTable.setItem(newLastRow,0,QTableWidgetItem(self.newTagLabel))
                self.tagTable.setItem(newLastRow,1,QTableWidgetItem(""))
                self.tagTable.item(newLastRow,0).setFlags(Qt.ItemIsEnabled | Qt.ItemIsEditable)
                self.tagTable.item(newLastRow,1).setFlags(Qt.ItemIsEnabled)

                self.__tagsLoaded=True

        # updating feature status (from Normal to Updated)
        if self.featureType=='Point':
            self.plugin.dbm.changePointStatus(self.feature.id(),'N','U')

        elif self.featureType=='Line':
            self.plugin.dbm.changeLineStatus(self.feature.id(),'N','U')

        elif self.featureType=='Polygon':
            self.plugin.dbm.changePolygonStatus(self.feature.id(),'N','U')

        elif self.featureType=='Relation':
            self.plugin.dbm.changeRelationStatus(self.feature.id(),'N','U')

        self.plugin.dbm.commit()

        affected=[(self.feature.id(),self.featureType)]
        self.plugin.undoredo.stopAction(affected)
        self.plugin.dbm.recacheAffectedNow(affected)

        # refresh map canvas so that changes take effect
        self.plugin.canvas.refresh()


    def __onTagsItemDoubleClicked(self,item):
        """Function is called after itemDoubleClicked(...) signal is emitted on table of feature tags.

        It shows combobox with possible values for given item of table.

        @param item item of table of feature tags
        """

        if item.column()==0:

            if item.row()<self.tagTable.rowCount()-1:
                return

            tagValues=self.determineSuitableTagKeys(self.featureType)
            tagValues.sort()
            if len(tagValues)>0:
                valCombo=QComboBox()
                valCombo.setEditable(True)
                valCombo.addItems(tagValues)
                currentComboText=self.tagTable.item(item.row(),0).text()
                ix=valCombo.findText(currentComboText)
                valCombo.setCurrentIndex(ix)
                if ix==-1:
                    valCombo.setEditText(currentComboText)

                self.tagTable.setCellWidget(item.row(),0,valCombo)
                QObject.connect(valCombo, SIGNAL("currentIndexChanged(const QString &)"), self.__onTagKeySelectionChanged)
            return

        key=self.tagTable.item(item.row(),0).text()
        tagValues=self.determineSuitableTagValues(self.featureType,key)
        tagValues.sort()

        if len(tagValues)>0:
            valCombo=QComboBox()
            valCombo.setEditable(True)
            valCombo.addItems(tagValues)
            currentComboText=self.tagTable.item(item.row(),1).text()
            ix=valCombo.findText(currentComboText)
            valCombo.setCurrentIndex(ix)
            if ix==-1:
                valCombo.setEditText(currentComboText)

            self.tagTable.setCellWidget(item.row(),1,valCombo)
            QObject.connect(valCombo, SIGNAL("currentIndexChanged(const QString &)"), self.__onTagValueSelectionChanged)


    def __onCurrentCellChanged(self,curRow,curCol,prevRow,prevCol):
        """Function is called after currentCellChanged(...) signal is emitted on table of feature tags.

        @param curRow current row index to tags table
        @param curCol current column index to tags table
        @param prevRow previous row index to tags table
        @param prevCol previous column index to tags table
        """

        cellWidget=self.tagTable.cellWidget(prevRow,prevCol)
        if cellWidget==None:
            return

        self.tagTable.removeCellWidget(prevRow,prevCol)


    def __onTagKeySelectionChanged(self,key):
        """Function is called after currentIndexChanged(...) signal is emitted on combobox in 1st column of tags table.

        @param key key selected in combobox
        """

        row=self.tagTable.currentRow()
        col=self.tagTable.currentColumn()

        self.tagTable.item(row,col).setText(key)
        self.tagTable.removeCellWidget(row,col)


    def __onTagValueSelectionChanged(self,value):
        """Function is called after currentIndexChanged(...) signal is emitted on combobox in 2nd column of tags table.

        @param value value selected in combobox
        """

        row=self.tagTable.currentRow()
        col=self.tagTable.currentColumn()

        self.tagTable.item(row,col).setText(value)
        self.tagTable.removeCellWidget(row,col)


    def determineSuitableTagValues(self,featType,tagKey):
        """Function is used to find out typical tag values to given feature type and key.
        With help of this function plugin gives advice to user on feature tags editing.
        Information on typical/recommended tag values was taken from wiki.openstreetmap.org.

        @param featType name of feature type; one of 'Point','Line','Polygon'
        @param tagKey key of tag
        @return list of typical values to given feature type and key
        """

        vals=[]
        # POINT TAGS
        if featType=='Point':

            if tagKey=="highway":
                vals=["services","mini_roundabout","stop","traffic_signals","crossing","incline","incline_steep","ford","bus_stop","turning_circle"
                     ,"emergency_access_point","speed_camera","motorway_junction","passing_place"]

            elif tagKey=="traffic_calming":
                vals=["yes","bump","chicane","cushion","hump","rumble_strip","table","choker"]

            elif tagKey=="barrier":
                vals=["bollard","cycle_barrier","cattle_grid","toll_booth","entrance","gate","stile","sally_port"]

            elif tagKey=="waterway":
                vals=["dock","lock_gate","turning_point","boatyard","weir"]

            elif tagKey=="lock":
                vals=["yes"]

            elif tagKey=="railway":
                vals=["station","halt","tram_stop","crossing","level_crossing","subway_entrance","turntable","buffer_stop"]

            elif tagKey=="aeroway":
                vals=["aerodrome","terminal","helipad","gate","windsock"]

            elif tagKey=="aerialway":
                vals=["station"]

            elif tagKey=="power":
                vals=["tower","station","sub_station","generator"]

            elif tagKey=="man_made":
                vals=["beacon","crane","gasometer","lighthouse","reservoir_covered","surveillance","survey_point","tower","wastewater_plant","watermill"
                     ,"water_tower","water_works","windmill","works"]

            elif tagKey=="leisure":
                vals=["sports_centre ","sports_centre ","stadium","track","pitch","water_park","marina","slipway","fishing","nature_reserve"
                     ,"park","playground","garden","common","ice_rink","miniature_golf"]

            elif tagKey=="amenity":
                vals=["restaurant","pub","food_court","fast_food","drinking_water","bbq","biergarten","cafe","kindergarten","school","college"
                     ,"library","university","ferry_terminal","bicycle_parking","bicycle_rental","bus_station","car_rental","car_sharing","fuel"
                     ,"grit_bin","parking","signpost","taxi","atm","bank","bureau_de_change","pharmacy","hospital","baby_hatch","dentist","doctors","veterinary"
                     ,"arts_centre","cinema","fountain","nightclub","studio","theatre","bench","brothel","courthouse","crematorium","embassy","emergency_phone"
                     ,"fire_station","grave_yard","hunting_stand","place_of_worship","police","post_box","post_office","prison","public_building","recycling"
                     ,"shelter","telephone","toilets","townhall","vending_machine","waste_basket","waste_disposal"]

            elif tagKey=="shop":
                vals=["alcohol","bakery","beverages","bicycle","books","butcher","car","car_repair","chemist","clothes","computer","confectionery","convenience"
                     ,"department_store","dry_cleaning","doityourself","electronics","florist","furniture","garden_centre","greengrocer","hairdresser"
                     ,"hardware","hifi","kiosk","laundry","mall","motorcycle","newsagent","optician","organic","outdoor","sports","stationery","supermarket"
                     ,"shoes","toys","travel_agency","video"]

            elif tagKey=="tourism":
                vals=["alpine_hut","attraction","artwork","camp_site","caravan_site","chalet","guest_house","hostel","hotel","information","motel","museum"
                     ,"picnic_site","theme_park","viewpoint","zoo","yes"]

            elif tagKey=="historic":
                vals=["castle","monument","memorial","archaeological_site","ruins","battlefield","wreck","yes"]

            elif tagKey=="landuse":
                vals=["quarry","landfill","basin","reservoir","forest","allotments","vineyard","residential","retail","commercial","industrial","brownfield"
                     ,"greenfield","construction","military","meadow","village_green","wood","recreation_ground"]

            elif tagKey=="military":
                vals=["airfield","bunker","barracks","danger_area","range","naval_base"]

            elif tagKey=="natural":
                vals=["bay","beach","cave_entrance","cliff","coastline","fell","glacier","heath","land","marsh","mud","peak","scree","scrub","spring","tree"
                     ,"volcano","water","wetland","wood"]

            elif tagKey=="sport":
                vals=["9pin","10pin","archery","athletics","australian_football","baseball","basketball","beachvolleyball","boules","bowls","canoe","chess"
                      "climbing","cricket","cricket_nets","croquet","cycling","diving","dog_racing","equestrian","football","golf","gymnastics","hockey"
                      "horse_racing","korfball","motor","multi","orienteering","paddle_tennis","pelota","racquet","rowing","rugby","shooting","skating"
                      "skateboard","skiing","soccer","swimming","table_tennis","team_handball","tennis","volleyball"]

            elif tagKey=="internet_access":
                vals=["public","service","terminal","wired","wlan"]

            elif tagKey=="motorroad":
                vals=["yes","no"]

            elif tagKey=="bridge":
                vals=["yes","aqueduct","viaduct","swing"]

            elif tagKey=="crossing":
                vals=["no","traffic_signals","uncontrolled"]

            elif tagKey=="mountain_pass":
                vals=["yes"]

            elif tagKey=="disused":
                vals=["yes"]

            elif tagKey=="wheelchair":
                vals=["yes","no","limited"]

            elif tagKey=="wood":
                vals=["coniferous","deciduous","mixed"]

            elif tagKey=="place":
                vals=["continent","country","state","region","country","city","town","village","hamlet","suburb","locality","island"]

            elif tagKey=="source":
                vals=["extrapolation","knowledge","historical","image","survey","voice"]


        # LINE TAGS
        elif featType=='Line':

            if tagKey=="highway":
                vals=["motorway","motorway_link","trunk","trunk_link","primary","primary_link","secondary","secondary_link","tertiary","unclassified"
                     ,"road","residential","living_street","service","track","pedestrian","bus_guideway","path","cycleway","footway","bridleway"
                     ,"byway","steps","ford","construction"]

            elif tagKey=="traffic_calming":
                vals=["yes","bump","chicane","cushion","hump","rumble_strip","table","choker"]

            elif tagKey=="service":
                vals=["parking_aisle","driveway","alley","yard","siding","spur"]

            elif tagKey=="smoothness":
                vals=["excellent","good","intermediate","bad","very_bad","horrible","very_horrible","impassable"]

            elif tagKey=="passing_places":
                vals=["yes"]

            elif tagKey=="barrier":
                vals=["hedge","fence","wall","ditch","retaining_wall","city_wall","bollard"]

            elif tagKey=="cycleway":
                vals=["lane","track","opposite_lane","opposite_track","opposite"]

            elif tagKey=="tracktype":
                vals=["grade1","grade2","grade3","grade4","grade5"]

            elif tagKey=="waterway":
                vals=["stream","river","canal","drain","weir","dam"]

            elif tagKey=="lock":
                vals=["yes"]

            elif tagKey=="mooring":
                vals=["yes","private","no"]

            elif tagKey=="railway":
                vals=["rail","tram","light_rail","abandoned","disused","subway","preserved","narrow_gauge","construction","monorail","funicular","platform"]

            elif tagKey=="usage":
                vals=["main","branch","industrial","military","tourism"]

            elif tagKey=="electrified":
                vals=["contact_line","rail","yes","no"]

            elif tagKey=="bridge":
                vals=["yes"]

            elif tagKey=="tunnel":
                vals=["yes","no"]

            elif tagKey=="aeroway":
                vals=["runway","taxiway"]

            elif tagKey=="aerialway":
                vals=["cable_car","gondola","chair_lift","drag_lift"]

            elif tagKey=="power":
                vals=["line"]

            elif tagKey=="cables":
                vals=["3","4","6","8","9","12","15","18"]

            elif tagKey=="wires":
                vals=["single","double","triple","quad"]

            elif tagKey=="voltage":
                vals=["110000","220000","380000","400000"]

            elif tagKey=="man_made":
                vals=["pier","pipeline"]

            elif tagKey=="leisure":
                vals=["track"]

            elif tagKey=="amenity":
                vals=["marketplace"]

            elif tagKey=="tourism":
                vals=["artwork"]

            elif tagKey=="natural":
                vals=["cliff","coastline"]

            elif tagKey=="route":
                vals=["bus","detour","ferry","flight","subsea","hiking","bicycle","mtb","road","ski","tour","tram","pub_crawl"]

            elif tagKey=="abutters":
                vals=["residential","retail","commercial","industrial","mixed"]

            elif tagKey=="fenced":
                vals=["yes","no"]

            elif tagKey=="lit":
                vals=["yes","no"]

            elif tagKey=="motorroad":
                vals=["yes","no"]

            elif tagKey=="bridge":
                vals=["yes","aqueduct","viaduct","swing"]

            elif tagKey=="tunnel":
                vals=["yes"]

            elif tagKey=="cutting":
                vals=["yes"]

            elif tagKey=="embankment":
                vals=["yes"]

            elif tagKey=="layer":
                vals=["-5","-4","-3","-2","-1","0","1","2","3","4","5"]

            elif tagKey=="surface":
                vals=["paved","unpaved","asphalt","concrete","paving_stones","cobblestone","metal","wood","grass_paver","gravel","pebblestone"
                     ,"grass","ground","earth","dirt","mud","sand","ice_road"]

            elif tagKey=="disused":
                vals=["yes"]

            elif tagKey=="wheelchair":
                vals=["yes","no","limited"]

            elif tagKey=="narrow":
                vals=["yes"]

            elif tagKey=="sac_scale":
                vals=["hiking","mountain_hiking","demanding_mountain_hiking","alpine_hiking","demanding_alpine_hiking","difficult_alpine_hiking"]

            elif tagKey=="trail_visibility":
                vals=["excellent","good","intermediate","bad","horrible","no"]

            elif tagKey=="mtb:scale":
                vals=["0","1","2","3","4","5"]

            elif tagKey=="mtb:scale:uphill":
                vals=["0","1","2","3","4","5"]

            elif tagKey=="mtb:scale:imba":
                vals=["0","1","2","3","4"]

            elif tagKey=="access":
                vals=["yes","designated","official","private","permissive","destination","delivery","agricultural","forestry","unknown","no"]

            elif tagKey=="vehicle":
                vals=["yes","designated","private","permissive","destination","delivery","agricultural","forestry","unknown","no"]

            elif tagKey=="bicycle":
                vals=["yes","designated","official","private","permissive","dismount","destination","delivery","agricultural","forestry","unknown","no"]

            elif tagKey=="foot":
                vals=["yes","designated","official","private","permissive","destination","delivery","agricultural","forestry","unknown","no"]

            elif tagKey=="goods":
                vals=["yes","designated","private","permissive","destination","delivery","agricultural","forestry","unknown","no"]

            elif tagKey=="hgv":
                vals=["yes","designated","private","permissive","destination","delivery","agricultural","forestry","unknown","no"]

            elif tagKey=="hazmat":
                vals=["yes","designated","private","permissive","destination","delivery","agricultural","forestry","unknown","no"]

            elif tagKey=="agricultural":
                vals=["yes","designated","private","permissive","destination","delivery","agricultural","forestry","unknown","no"]

            elif tagKey=="horse":
                vals=["yes","designated","official","private","permissive","destination","delivery","agricultural","forestry","unknown","no"]

            elif tagKey=="motorcycle":
                vals=["yes","designated","private","permissive","destination","delivery","agricultural","forestry","unknown","no"]

            elif tagKey=="motorcar":
                vals=["yes","designated","private","permissive","destination","delivery","agricultural","forestry","unknown","no"]

            elif tagKey=="motor_vehicle":
                vals=["yes","designated","private","permissive","destination","delivery","agricultural","forestry","unknown","no"]

            elif tagKey=="psv":
                vals=["yes","designated","private","permissive","destination","delivery","agricultural","forestry","unknown","no"]

            elif tagKey=="motorboat":
                vals=["yes","designated","private","permissive","destination","delivery","agricultural","forestry","unknown","no"]

            elif tagKey=="boat":
                vals=["yes","designated","private","permissive","destination","delivery","agricultural","forestry","unknown","no"]

            elif tagKey=="oneway":
                vals=["yes","no","-1"]

            elif tagKey=="noexit":
                vals=["yes"]

            elif tagKey=="toll":
                vals=["yes"]

            elif tagKey=="addr:interpolation":
                vals=["all","even","odd","alphabetic"]

            elif tagKey=="source":
                vals=["extrapolation","knowledge","historical","image","survey","voice"]


        # POLYGON TAGS
        elif featType=='Polygon':

            if tagKey=="highway":
                vals=["pedestrian","services"]

            elif tagKey=="junction":
                vals=["roundabout"]

            elif tagKey=="barrier":
                vals=["hedge","fence","wall","ditch","retaining_wall","city_wall"]

            elif tagKey=="waterway":
                vals=["riverbank","dock","dam"]

            elif tagKey=="railway":
                vals=["station","turntable","platform"]

            elif tagKey=="aeroway":
                vals=["aerodrome","terminal","helipad","apron"]

            elif tagKey=="aerialway":
                vals=["station"]

            elif tagKey=="power":
                vals=["station","sub_station","generator"]

            elif tagKey=="man_made":
                vals=["crane","gasometer","pier","reservoir_covered","surveillance","wastewater_plant","watermill","water_tower","water_works","windmill","works"]

            elif tagKey=="building":
                vals=["yes"]

            elif tagKey=="leisure":
                vals=["sports_centre ","sports_centre ","stadium","track","pitch","water_park","marina","fishing","nature_reserve"
                     ,"park","playground","garden","common","ice_rink","miniature_golf"]

            elif tagKey=="amenity":
                vals=["restaurant","pub","food_court","fast_food","biergarten","cafe","kindergarten","school","college"
                     ,"library","university","ferry_terminal","bicycle_parking","bicycle_rental","bus_station","car_rental","car_sharing","fuel"
                     ,"parking","taxi","bank","pharmacy","hospital","baby_hatch","dentist","doctors","veterinary"
                     ,"arts_centre","cinema","fountain","nightclub","studio","theatre","brothel","courthouse","crematorium","embassy"
                     ,"fire_station","grave_yard","hunting_stand","marketplace","place_of_worship","police","post_office","prison","public_building","recycling"
                     ,"shelter","townhall"]

            elif tagKey=="shop":
                vals=["alcohol","bakery","beverages","bicycle","books","butcher","car","car_repair","chemist","clothes","computer","confectionery","convenience"
                     ,"department_store","dry_cleaning","doityourself","electronics","florist","furniture","garden_centre","greengrocer","hairdresser"
                     ,"hardware","hifi","kiosk","laundry","mall","motorcycle","newsagent","optician","organic","outdoor","sports","stationery","supermarket"
                     ,"shoes","toys","travel_agency","video"]

            elif tagKey=="tourism":
                vals=["alpine_hut","attraction","artwork","camp_site","caravan_site","chalet","museum","picnic_site","theme_park","zoo","yes"]

            elif tagKey=="historic":
                vals=["castle","monument","memorial","archaeological_site","ruins","battlefield","wreck","yes"]

            elif tagKey=="landuse":
                vals=["farm","farmyard","quarry","landfill","basin","reservoir","forest","allotments","vineyard","residential","retail","commercial","industrial"
                     ,"brownfield","greenfield","construction","railway","military","cemetery","meadow","village_green","wood","recreation_ground","salt_pond"]

            elif tagKey=="military":
                vals=["airfield","bunker","barracks","danger_area","range","naval_base"]

            elif tagKey=="natural":
                vals=["bay","beach","cave_entrance","cliff","coastline","fell","glacier","heath","land","marsh","mud","scree","scrub"
                     ,"water","wetland","wood"]

            elif tagKey=="boundary":
                vals=["administrative","civil","political","national_park"]

            elif tagKey=="sport":
                vals=["9pin","10pin","archery","athletics","australian_football","baseball","basketball","beachvolleyball","boules","bowls","canoe","chess"
                      "climbing","cricket","cricket_nets","croquet","cycling","diving","dog_racing","equestrian","football","golf","gymnastics","hockey"
                      "horse_racing","korfball","motor","multi","paddle_tennis","pelota","racquet","rowing","rugby","shooting","skating"
                      "skateboard","skiing","soccer","swimming","table_tennis","team_handball","tennis","volleyball"]

            elif tagKey=="area":
                vals=["yes"]

            elif tagKey=="disused":
                vals=["yes"]

            elif tagKey=="wheelchair":
                vals=["yes","no","limited"]

            elif tagKey=="wood":
                vals=["coniferous","deciduous","mixed"]

            elif tagKey=="place":
                vals=["continent","state","region","country","city","town","village","hamlet","suburb","locality","island"]

            elif tagKey=="source":
                vals=["extrapolation","knowledge","historical","image","survey","voice"]


        return vals


    def determineSuitableTagKeys(self,featType):
        """Function is used to find out typical tag keys to given feature type.
        With help of this function plugin gives advice to user on feature tags editing.
        Information on typical/recommended tag keys was taken from wiki.openstreetmap.org.

        @param featType name of feature type; one of 'Point','Line','Polygon'
        @return list of typical keys to given feature type
        """

        vals = []
        if featType=='Point':

            vals=["highway","traffic_calming","barrier","waterway","lock","railway","aeroway","aerialway","power","man_made","leisure"
                 ,"amenity","shop","tourism","historic","landuse","military","natural","route","sport","internet_access","motorroad","bridge","crossing"
                 ,"mountain_pass","ele","incline","operator","opening_hours","disused","wheelchair","TMC:LocationCode","wood","traffic_sign","disused"
                 ,"name","alt_name"
                 ,"alt_name","int_name","nat_name","reg_name","loc_name","old_name","name:lg","ref","int_ref","nat_ref","reg_ref","loc_ref","old_ref"
                 ,"source_ref","icao","iata","place","place_numbers","postal_code","is_in","population","addr:housenumber","addr:housename","addr:street"
                 ,"addr:postcode","addr:city","addr:country","note","description","image","source","source_ref","source_name","source:ref"
                 ,"attribution","url","website","wikipedia","created_by","history"]

        elif featType=='Line':

            vals=["highway","construction","junction","traffic_calming","service","smoothness","passing_places","barrier","cycleway"
                 ,"tracktype","waterway","lock","mooring","railway","usage","electrified","frequency","voltage","bridge","tunnel","service"
                 ,"aeroway","aerialway","power","cables","wires","voltage","man_made","leisure","amenity","natural","route","abutters","fenced"
                 ,"lit","motorroad","bridge","tunnel","cutting","embankment","lanes","layer","surface","width","est_width","depth","est_depth"
                 ,"incline","start_date","end_date","operator","opening_hours","disused","wheelchair","narrow","sac_scale","trail_visibility"
                 ,"mtb:scale","mtb:scale:uphill","mtb:scale:imba","mtb:description","TMC:LocationCode","access","vehicle","bicycle","foot","goods"
                 ,"hgv","hazmat","agricultural","horse","motorcycle","motorcar","motor_vehicle","psv","motorboat","boat","oneway","noexit"
                 ,"date_on","date_off","hour_on","hour_off","maxweight","maxheight","maxwidth","maxlength","maxspeed","minspeed","maxstay"
                 ,"disused","toll","charge","name"
                 ,"alt_name","int_name","nat_name","reg_name","loc_name","old_name","name:lg","ref","int_ref","nat_ref","reg_ref","loc_ref","old_ref"
                 ,"ncn_ref","rcn_ref","lcn_ref"
                 ,"source_ref","icao","iata","place_numbers","postal_code","is_in","addr:interpolation","note","description","image","source"
                 ,"source_ref","source_name","source:ref","attribution","url","website","wikipedia","created_by","history"]


        elif featType=='Polygon':

            vals=["highway","junction","barrier","waterway","railway","landuse","aeroway","aerialway","power","man_made","building","leisure"
                 ,"amenity","shop","tourism","historic","landuse","military","natural","route","boundary","sport","area","ele","depth","est_depth"
                 ,"operator","opening_hours","disused","wheelchair","wood","admin_level","disused","name"
                 ,"alt_name","int_name","nat_name","reg_name","loc_name","old_name","name:lg","ref","int_ref","nat_ref","reg_ref","loc_ref","old_ref"
                 ,"source_ref","icao","iata","place","place_name","place_numbers","postal_code","is_in","population"
                 ,"addr:housenumber","addr:housename","addr:street","addr:postcode","addr:city","addr:country"
                 ,"note","description","image","source","source_ref","source_name","source:ref","attribution","url","website","wikipedia"
                 ,"created_by","history"]

        return vals


    def __startIdentifyingFeature(self):
        """Function prepares feature identification.
        The appropriate map tool (IdentifyMapTool) is set to map canvas.
        """

        if self.activeEditButton==self.identifyButton:
            return

        self.plugin.canvas.unsetMapTool(self.plugin.canvas.mapTool())

        # clear dockwidget
        self.clear()

        self.plugin.iface.mainWindow().statusBar().showMessage("")

        self.__mapTool=IdentifyMapTool(self.plugin.canvas, self, self.plugin.dbm)
        self.plugin.canvas.setMapTool(self.__mapTool)
        self.plugin.canvas.setCursor(QCursor(Qt.ArrowCursor))
        self.activeEditButton=self.identifyButton
        self.plugin.canvas.setFocus(Qt.OtherFocusReason)


    def __startMovingFeature(self):
        """Function prepares feature moving.
        The appropriate map tool (MoveMapTool) is set to map canvas.
        """

        if self.activeEditButton==self.moveButton:
            return

        # clear dockwidget
        self.clear()
        self.plugin.iface.mainWindow().statusBar().showMessage("Snapping ON. Hold Ctrl to disable it.")

        self.__mapTool=MoveMapTool(self.plugin)
        self.plugin.canvas.setMapTool(self.__mapTool)
        self.plugin.canvas.setCursor(QCursor(Qt.CrossCursor))
        self.activeEditButton=self.moveButton
        self.plugin.canvas.setFocus(Qt.OtherFocusReason)


    def __startPointCreation(self):
        """Function prepares point creating operation.
        The appropriate map tool (CreatePointMapTool) is set to map canvas.
        """

        if self.activeEditButton==self.createPointButton:
            return

        self.plugin.iface.mainWindow().statusBar().showMessage("Snapping ON. Hold Ctrl to disable it.")

        self.__mapTool=CreatePointMapTool(self.plugin)
        self.plugin.canvas.setMapTool(self.__mapTool)
        self.plugin.canvas.setCursor(QCursor(Qt.ArrowCursor))
        self.activeEditButton=self.createPointButton
        self.plugin.canvas.setFocus(Qt.OtherFocusReason)


    def __startLineCreation(self):
        """Function prepares line creating operation.
        The appropriate map tool (CreateLineMapTool) is set to map canvas.
        """

        if self.activeEditButton==self.createLineButton:
            return

        self.plugin.iface.mainWindow().statusBar().showMessage("Snapping ON. Hold Ctrl to disable it.")

        self.__mapTool=CreateLineMapTool(self.plugin)
        self.plugin.canvas.setMapTool(self.__mapTool)
        self.plugin.canvas.setCursor(QCursor(Qt.ArrowCursor))
        self.activeEditButton=self.createLineButton
        self.plugin.canvas.setFocus(Qt.OtherFocusReason)


    def __startPolygonCreation(self):
        """Function prepares polygon creating operation.
        The appropriate map tool (CreatePolygonMapTool) is set to map canvas.
        """

        if self.activeEditButton==self.createPolygonButton:
            return

        self.plugin.iface.mainWindow().statusBar().showMessage("Snapping ON. Hold Ctrl to disable it.")

        self.__mapTool=CreatePolygonMapTool(self.plugin)
        self.plugin.canvas.setMapTool(self.__mapTool)
        self.plugin.canvas.setCursor(QCursor(Qt.ArrowCursor))
        self.activeEditButton=self.createPolygonButton
        self.plugin.canvas.setFocus(Qt.OtherFocusReason)


    def removeFeature(self):
        """Function completely removes feature that is currently loaded on "OSM Feature" widget.
        """

        self.removeButton.setDown(False)
        self.removeButton.setChecked(False)
        self.plugin.iface.mainWindow().statusBar().showMessage("")

        # remove object that was identified by "identify tool"
        featId=self.feature.id()
        featType=self.featureType
        affected=[]

        if featType=='Point':
            self.plugin.undoredo.startAction("Remove point.")
            affected=self.plugin.dbm.removePoint(featId)

        elif featType=='Line':
            self.plugin.undoredo.startAction("Remove line.")
            affected=self.plugin.dbm.removeLine(featId,True)    # todo: False when Ctrl pressed

        elif featType=='Polygon':
            self.plugin.undoredo.startAction("Remove polygon.")
            affected=self.plugin.dbm.removePolygon(featId,True)    # todo: False when Ctrl pressed

        elif featType=='Relation':
            self.plugin.undoredo.startAction("Remove relation.")
            self.plugin.dbm.removeRelation(featId)
        else:
            return    # strange situation

        self.plugin.undoredo.stopAction(affected)
        self.plugin.dbm.recacheAffectedNow(affected)

        # refresh map canvas so that changes take effect
        self.plugin.canvas.refresh()

        # clear dockwidget
        self.clear()

        self.plugin.iface.mainWindow().statusBar().showMessage("")

        self.plugin.canvas.unsetMapTool(self.__mapTool)
        del self.__mapTool
        self.__mapTool=IdentifyMapTool(self.plugin.canvas, self, self.plugin.dbm)
        self.plugin.canvas.setMapTool(self.__mapTool)
        self.plugin.canvas.setCursor(QCursor(Qt.ArrowCursor))
        self.plugin.canvas.setFocus(Qt.OtherFocusReason)
        self.activeEditButton=self.identifyButton
        self.activeEditButton.setChecked(True)


    def removeSelectedTags(self):
        """Function completely removes all tags that are currently selected in the appropriate
        list of the "OSM Feature" widget. More than one tag can be selected using Ctrl and clicking.
        """

        # remove selected tags (rows)
        selectedItems=self.tagTable.selectedItems()
        selectedRowsIndexes=[]
        lastRowIndex=self.tagTable.rowCount()-1
        self.tagTable.setCurrentCell(lastRowIndex,0)

        for i in selectedItems:
            if i.column()==0 and not i.row()==lastRowIndex:
                selectedRowsIndexes.append(i.row())

        self.plugin.undoredo.startAction("Remove tags.")

        selectedRowsIndexes.sort()
        selectedRowsIndexes.reverse()

        for ix in selectedRowsIndexes:

            key=self.tagTable.item(ix,0).text()
            # updating feature status (from Normal to Updated)
            if self.featureType=='Point':
                self.plugin.dbm.changePointStatus(self.feature.id(),'N','U')

            elif self.featureType=='Line':
                self.plugin.dbm.changeLineStatus(self.feature.id(),'N','U')

            elif self.featureType=='Polygon':
                self.plugin.dbm.changePolygonStatus(self.feature.id(),'N','U')

            elif self.featureType=='Relation':
                self.plugin.dbm.changeRelationStatus(self.feature.id(),'N','U')

            # perform tag removing 
            self.plugin.dbm.removeTag(self.feature.id(),self.featureType,key.toUtf8().data())

            self.tagTable.removeRow(ix)

        # make this action permanent
        self.plugin.dbm.commit()
        affected=[(self.feature.id(),self.featureType)]
        self.plugin.undoredo.stopAction(affected)
        self.plugin.dbm.recacheAffectedNow(affected)

        # refresh map canvas so that changes take effect
        self.plugin.canvas.refresh()


    def removeSelectedRelation(self):
        """Function completely removes all relations that are selected in the appropriate list of the "OSM Feature" widget.
        More than one relation can be selected using Ctrl and clicking.
        """

        # find out id of relation which is selected
        item=self.relListWidget.item(self.relListWidget.currentRow())
        if not item or not self.plugin.dbm.currentKey:
            return

        relId=self.featRels[self.relListWidget.currentRow()]

        self.plugin.undoredo.startAction("Remove relation.")
        self.plugin.dbm.removeRelation(relId)
        self.plugin.dbm.commit()
        self.plugin.undoredo.stopAction()

        # reload list of feature's relations
        self.__loadFeatureRelations(self.feature.id(),self.featureType)


    def __showRelMemberOnMap(self,ixRow):
        """Functions marks relation member on the map.

        Marking is realized with simple rubberBand (in case of line/polygon) or vertexMarker (for point).
        Relation member is given by its index to the list of all members of currently loaded relation.

        @param ixRow index to the list of all relations on mentioned widget
        """

        # move rubberband to selected feature
        self.relRubBand.reset(False)
        self.relRubBandPol.reset(True)
        self.relVerMarker.hide()

        if ixRow==-1 or not self.plugin.dbm.currentKey:
            return
        mem=self.featRelMembers[ixRow]
        self.showFeatureOnMap(mem[0],mem[1])    # id & type


    def loadRelationStuff(self,ixRow):
        """Functions loads information on specified relation into "OSM Feature" widget.

        Relation is given by its index to the list of all relations on mentioned widget.
        Relation "stuff" means its basic info, tags and members.

        @param ixRow index to the list of all relations on mentioned widget
        """

        if ixRow==-1 or not self.plugin.dbm.currentKey:
            return
        relId=self.featRels[ixRow]

        # show all tags connected to selected relation
        self.__loadRelationTags(relId)

        # show all relation members on osm dock widget
        self.__loadRelationMembers(relId)

        # enable list of members and buttons for relation removing and editing
        self.relMembersList.setEnabled(True)
        self.editRelationButton.setEnabled(True)
        self.removeRelationButton.setEnabled(True)


    def __loadRelationMembers(self,relId):
        """Functions loads the list of members of specified relation.
        Relation is given by its identifier.

        Loading is realized into the appropriate QListWidget of "OSM Feature" dockable widget.
        Resulting list contains info on member's identifier, type and its role in specified relation.

        @param relId identifier of relation
        """

        self.relMembersList.clear()
        # ask database manager for all relation members
        self.featRelMembers=self.plugin.dbm.getRelationMembers(relId)

        # printing members
        for i in range(0,len(self.featRelMembers)):
            memId=self.featRelMembers[i][0]
            memType=self.featRelMembers[i][1]
            memRole=self.featRelMembers[i][2]

            listRow=QString("(%1) - %2").arg(memId).arg(memType)
            if memRole and memRole<>"":
                listRow=listRow.append(QString(", role:%3").arg(memRole))

            self.relMembersList.addItem(listRow)


    def __loadRelationTags(self,relId):
        """Functions loads the list of tags of specified relation.
        Relation is given by its identifier.

        Loading is realized into the appropriate QTreeWidget of "OSM Feature" dockable widget.
        Resulting tags table has two columns "Key","Value" for easy representing of tag pairs.

        @param relId identifier of relation
        """

        self.relTagsTree.clear()
        self.__relTagsLoaded=False
        # ask database manager for all relation tags
        self.featRelTags=self.plugin.dbm.getFeatureTags(relId,"Relation")

        self.relTagsTree.setColumnCount(2)
        self.relTagsTree.setHeaderLabels(["Key","Value"])

        for i in range(0,len(self.featRelTags)):
            self.relTagsTree.addTopLevelItem(QTreeWidgetItem([self.featRelTags[i][0],self.featRelTags[i][1]]))

        self.__relTagsLoaded=True


    def __loadFeatureInformation(self,featId,featType):
        """Functions shows up the basic information on feature.
        Feature is given by its identifier and its type.

        Info is loaded to appropriate place of the "OSM Feature" widget.
        Basic info consists (mainly) of feature's identifier, type, owner and timestamp.

        @param featId identifier of feature to load
        @param featType type of feature to load - one of 'Point','Line','Polygon'
        """

        # asking DatabaseManager for missing information
        featUser=self.plugin.dbm.getFeatureOwner(featId,featType)
        featCreated=self.plugin.dbm.getFeatureCreated(featId,featType)    # returned as string
            # timestamp example: "2008-02-18T15:34:14Z"

        self.typeIdLabel.setText("")
        self.userLabel.setText("")
        self.createdLabel.setText("")

        # put non-tags feature information on dock widget
        if featType:
            self.typeIdLabel.setText(QString("%1  %2").arg(featType).arg(str(featId)))
        if featUser:
            self.userLabel.setText(QString("%1").arg(featUser))
        if featCreated:
            # format timestamp
            DT_format_osm=Qt.ISODate
                # "yyyy-MM-ddThh:mm:ssZ"
            DT_format_plugin="yy/MM/dd - h:mm"
            self.createdLabel.setText(QDateTime.fromString(featCreated,DT_format_osm).toString(DT_format_plugin))


    def __loadFeatureTags(self,featId,featType):
        """Functions loads the list of tags of specified feature.
        Feature is given by its identifier and its type.

        Loading is realized into the appropriate QTableWidget of "OSM Feature" dockable widget.
        Resulting tags table has two columns "Key","Value" for easy representing of tag pairs.

        @param featId identifier of feature to load
        @param featType type of feature to load - one of 'Point','Line','Polygon'
        """

        # clear table with information about feature's tags
        self.tagTable.clear()

        # fill tableWidget with tags of selected feature
        tableData=self.plugin.dbm.getFeatureTags(featId,featType)
        rowCount=len(tableData)
        self.__tagsLoaded=False

        self.tagTable.setRowCount(rowCount+1)
        self.tagTable.setColumnCount(2)
        self.tagTable.setHorizontalHeaderItem(0,QTableWidgetItem("Key"))
        self.tagTable.setHorizontalHeaderItem(1,QTableWidgetItem("Value"))

        for i in range(0,rowCount):
            self.tagTable.setItem(i,0,QTableWidgetItem(tableData[i][0]))
            self.tagTable.setItem(i,1,QTableWidgetItem(tableData[i][1]))
            self.tagTable.item(i,0).setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled)
            self.tagTable.item(i,1).setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled | Qt.ItemIsEditable)

        self.tagTable.setItem(rowCount,0,QTableWidgetItem(self.newTagLabel))
        self.tagTable.setItem(rowCount,1,QTableWidgetItem(""))
        self.tagTable.item(rowCount,0).setFlags(Qt.ItemIsEnabled | Qt.ItemIsEditable)
        self.tagTable.item(rowCount,1).setFlags(Qt.ItemIsEnabled)
        self.__tagsLoaded=True

        # enable tags table for editing
        self.tagTable.setEnabled(True)
        self.deleteTagsButton.setEnabled(True)


    def __loadFeatureRelations(self,featId,featType):
        """Functions loads the list of relations of specified feature.
        Feature is given by its identifier and its type.

        Loading is realized into the appropriate QListWidget of "OSM Feature" dockable widget.
        If no relation exists for specified feature, listWidget is filled with the only row with text: "<no relation>".

        @param featId identifier of feature to load
        @param featType type of feature to load - one of 'Point','Line','Polygon'
        """

        self.relTagsTree.setColumnCount(0)
        self.relMembersList.setEnabled(False)

        # disable widget buttons
        self.editRelationButton.setEnabled(False)
        self.removeRelationButton.setEnabled(False)

        # clear all tables connected to relations
        self.relListWidget.clear()
        self.relTagsTree.clear()
        self.relMembersList.clear()

        # load relations for selected feature
        self.featRels=self.plugin.dbm.getFeatureRelations(featId,featType)

        for i in range(0,len(self.featRels)):
            self.relListWidget.addItem(self.__getRelationInfo(self.featRels[i]))

        if len(self.featRels)==0:
            self.relListWidget.addItem("<no relation>")
            self.addRelationButton.setEnabled(True)
            return

        # enable relation tables and button for relation addition
        self.addRelationButton.setEnabled(True)
        self.relListWidget.setEnabled(True)
        self.relTagsTree.setEnabled(True)


    def reloadFeatureRelations(self):
        """Functions reloads the list of relations for currently loaded feature.

        Loading is realized into the appropriate QListWidget of "OSM Feature" dockable widget.
        If no relation exists for specified feature, listWidget is filled with the only row with text: "<no relation>".
        """

        self.relTagsTree.setColumnCount(0)
        self.relMembersList.setEnabled(False)

        # disable widget buttons
        self.editRelationButton.setEnabled(False)
        self.removeRelationButton.setEnabled(False)

        # clear all tables connected to relations
        self.relListWidget.clear()
        self.relTagsTree.clear()
        self.relMembersList.clear()

        # load relations for selected feature
        self.featRels=self.plugin.dbm.getFeatureRelations(self.feature.id(),self.featureType)

        for i in range(0,len(self.featRels)):
            self.relListWidget.addItem(self.__getRelationInfo(self.featRels[i]))

        if len(self.featRels)==0:
            self.relListWidget.addItem("<no relation>")
            self.addRelationButton.setEnabled(True)
            return

        # enable relation tables and button for relation addition
        self.addRelationButton.setEnabled(True)
        self.relListWidget.setEnabled(True)
        self.relTagsTree.setEnabled(True)


    def putMarkersOnMembers(self,feat,featType):
        """Function adds additional vertexMarkers to the map.

        Additional vertexMarker are used to provide better marking of line/polygon.
        In that case line/polygon geometry is marked with rubberband first, and second one vertexMarker is put
        on each its vertex. Such extended marking of line/polygon is used (for example) when calling loadFeature() method with
        "markingMode" parameter set to 2.

        @param featId identifier of feature to load
        @param featType type of feature to load - one of 'Point','Line','Polygon'
        """

        if featType=='Point':
            return

        pline=None
        if featType=='Line':
            pline=feat.geometry().asPolyline()

        elif featType=='Polygon':
            pline=feat.geometry().asPolygon()[0]

        # get qgis settings of line width and color for rubberband
        settings=QSettings()
        qgsLineWidth=settings.value("/qgis/digitizing/line_width",QVariant(10)).toInt()
        qgsLineRed=settings.value("/qgis/digitizing/line_color_red",QVariant(255)).toInt()
        qgsLineGreen=settings.value("/qgis/digitizing/line_color_green",QVariant(0)).toInt()
        qgsLineBlue=settings.value("/qgis/digitizing/line_color_blue",QVariant(0)).toInt()

        for i in range(0,len(pline)):
            verMarker=QgsVertexMarker(self.plugin.canvas)
            verMarker.setIconType(3)
            verMarker.setIconSize(6)
            verMarker.setColor(QColor(qgsLineRed[0],qgsLineGreen[0],qgsLineBlue[0]))
            verMarker.setPenWidth(qgsLineWidth[0])
            verMarker.setCenter(pline[i])
            verMarker.show()
            self.verMarkers.append(verMarker)


    def __removeMemberMarkers(self):
        """Function removes additional vertexMarkers from the map.

        Additional vertexMarker are used to provide better marking of line/polygon.
        In that case line/polygon geometry is marked with rubberband first, and second one vertexMarker is put
        on each its vertex. Such extended marking of line/polygon is used (for example) when calling loadFeature() method with
        "markingMode" parameter set to 2.
        """

        for verMarker in self.verMarkers:
            self.plugin.canvas.scene().removeItem(verMarker)
            del verMarker
        self.verMarkers=[]


    def showFeatureOnMap(self,featId,featType):
        """Function just shows up specified feature on the map canvas.

        Showing feature up is realized by putting rubberBand (vertexMarker) on its whole geometry.
        This rubberBand (vM) is the same as when using loadFeature() method with "markingMode" parameter set to 1.

        Feature is given by its identifier and its type. Feature data are not loaded into "OSM Feature" widget.
        Rubberbands (vertexMarkers) of other features are not removed from the map canvas before this action.

        @param featId identifier of feature to load
        @param featType type of feature to load - one of 'Point','Line','Polygon'
        """

        # we have to know feature's geometry to be able to display it on map
        featGeom=self.plugin.dbm.getFeatureGeometry(featId,featType)
        if not featGeom:
           return    # nothing to show :-/

        if featType=='Polygon':
            self.relRubBandPol.setToGeometry(featGeom,self.plugin.canvas.currentLayer())
        elif featType=='Point':
            self.relVerMarker.setCenter(featGeom.asPoint())
            self.relVerMarker.show()
        elif featType=='Line':
            self.relRubBand.setToGeometry(featGeom,self.plugin.canvas.currentLayer())


    def loadFeature(self,feat,featType,markingMode=1):
        """Function loads information on specified feature into "OSM Feature" widget elements.
        It shows up features identifier, type, osm user, timestamp, all its tags and related OSM relations.

        According to the value of parameter "markingMode" is marks feature on the map canvas. If "markingMode" equals to zero
        feature is not marked on the map. If it equals to 1, simple rubberband (or vertexMarker) is put on the feature (this is
        default behavior). If "markingMode" equals to 2, extended rubberband is shown on the map, especially for non-point features.

        @param feat QgsFeature object of feature to load
        @param featType type of feature to load - one of 'Point','Line','Polygon'
        @param markingMode not compulsory; defines quality of feature's rubberband on map
        """

        if not feat or not featType:
            self.clear()
            return

        # remember which feature is loaded
        self.featureType=featType
        self.feature=feat

        # move rubberband to selected feature
        self.rubBandPol.reset(True)
        self.rubBand.reset(False)
        self.verMarker.hide()
        self.__removeMemberMarkers()

        if markingMode>0:
            if self.featureType=='Polygon':
                self.rubBandPol.setToGeometry(feat.geometry(),self.plugin.canvas.currentLayer())
                if markingMode>1:
                    self.putMarkersOnMembers(feat,featType)
            elif self.featureType=='Point':
                self.verMarker.setCenter(feat.geometry().asPoint())
                self.verMarker.show()
            elif self.featureType=='Line':
                self.rubBand.setToGeometry(feat.geometry(),self.plugin.canvas.currentLayer())
                if markingMode>1:
                    self.putMarkersOnMembers(feat,featType)

        # show common feature information (id,feature originator,created,feature type,...)
        self.__loadFeatureInformation(feat.id(),featType)

        # show all tags connected to feature onto osm widget dialog
        self.__loadFeatureTags(feat.id(),featType)

        # show all relations connected to feature onto osm widget dialog
        self.__loadFeatureRelations(feat.id(),featType)

        # feature has been loaded; enable "remove feature" button
        self.removeButton.setEnabled(True)


    def __getRelationInfo(self,relId):
        """Function returns brief info on specified relation. Information consists of relation identifier,
        its type and other important relation properties and it is returned simply in QString() object.

        Information are good to be shown in some list (of relations).

        @param relId identifier of relation
        @return brief info on relation concatenated in string
        """

        relInfo=relType=relRoute=relBoundary=relRef=relRestr=""
        tags=self.plugin.dbm.getFeatureTags(relId,"Relation")

        for i in range(0,len(tags)):
            key=tags[i][0]
            value=tags[i][1]

            if key=="type":
                relType=value        # type: route / boundary, ...
            elif key=="route":
                relRoute=value       # route: road / bicycle / foot / hiking / bus / railway / tram, ...
            elif key=="boundary":
                relBoundary=value    # boundary: administrative, ...
            elif key=="ref":
                relRef=value
            elif key=="restriction":
                relRestr=value

        if relType=="route":
            relInfo = QString("(%1) "+relRoute+" "+relType+" "+relRef).arg(relId)

        elif relType=="boundary":
            relInfo = QString("(%1) "+relBoundary+" "+relType).arg(relId)

        elif relType=="restriction":
            relInfo = QString("(%1) "+relRestr+" "+relType).arg(relId)

        elif relType=="":
            relInfo = QString("(%1) relation of unknown type").arg(relId)

        else:
            relInfo = QString("(%1) "+relType).arg(relId)

        return relInfo


    def __undo(self):
        """Function performs exactly one undo operation.
        """

        self.plugin.undoredo.undo()


    def __redo(self):
        """Function performs exactly one redo operation.
        """

        self.plugin.undoredo.redo()


    def __urDetailsChecked(self):
        """Function is called after clicking on urDetailsButton checkbox.
        It shows or hides OSM Edit History widget.
        """

        if self.urDetailsButton.isChecked():
            self.plugin.undoredo.show()
        else:
            self.plugin.undoredo.hide()


