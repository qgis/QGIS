"""@package DockWidget
This module is descendant of "OSM Feature" dockable widget (in Quantum GIS) and makes user able
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

        self.toolButtons=QButtonGroup(self)
        self.dummyButton.setVisible(False)
        self.toolButtons.addButton(self.dummyButton)
        self.toolButtons.addButton(self.identifyButton)
        self.toolButtons.addButton(self.moveButton)
        self.toolButtons.addButton(self.createPointButton)
        self.toolButtons.addButton(self.createLineButton)
        self.toolButtons.addButton(self.createPolygonButton)
        self.toolButtons.setExclusive(True)

        self.tagsTableWidget.setColumnCount(2)
        self.tagsTableWidget.setHorizontalHeaderItem(0,QTableWidgetItem("Key"))
        self.tagsTableWidget.setHorizontalHeaderItem(1,QTableWidgetItem("Value"))
        self.newTagLabel = "<new tag here>"

        self.plugin=plugin
        self.__mapTool=None
        self.__dlgAddRel=None

        # get qgis settings of line width and color for rubberband
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

        self.__activeEditButton=self.dummyButton
        self.__tagsLoaded=False
        self.__relTagsLoaded=False
        self.feature=None
        self.featureType=None
        self.featRels=[]
        self.featRelTags=[]
        self.featRelMembers=[]

        self.tagsEditIndex=-1

        # clear all widget items
        self.clear()

        self.__connectWidgetSignals()

        self.removeButton.setEnabled(False)
        self.createRelationButton.setCheckable(False)

        self.relTagsTreeWidget.setSelectionMode(QAbstractItemView.NoSelection)
        self.tagsTableWidget.setSelectionMode(QAbstractItemView.ExtendedSelection)
        self.tagsTableWidget.setSelectionBehavior(QAbstractItemView.SelectRows)

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
        QObject.connect(self.tagsTableWidget, SIGNAL("cellChanged(int,int)"), self.__onTagsCellChanged)
        QObject.connect(self.tagsTableWidget, SIGNAL("itemDoubleClicked(QTableWidgetItem*)"), self.__onTagsItemDoubleClicked)
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
            self.__activeEditButton=self.dummyButton

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
        self.tagsTableWidget.clear()
        self.tagsTableWidget.setEnabled(False)
        self.tagsTableWidget.setRowCount(0)
        self.tagsTableWidget.setColumnCount(0)
        self.tagsEditIndex=-1

        # clear table with info about feature's relations
        self.relListWidget.clear()
        self.relTagsTreeWidget.clear()
        self.relMembersList.clear()
        self.relTagsTreeWidget.setColumnCount(0)

        self.relListWidget.setEnabled(False)
        self.relTagsTreeWidget.setEnabled(False)
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
        self.__activeEditButton=self.dummyButton

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
        self.__activeEditButton=self.dummyButton

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

        if row<self.tagsTableWidget.rowCount()-1:

            # changing value of tag that already exists
            key = self.tagsTableWidget.item(row,0).text()
            value = self.tagsTableWidget.item(row,1).text()

            # store tag's change into database
            self.plugin.undoredo.startAction("Change tag value.")
            self.plugin.dbm.changeTagValue(self.feature.id(),self.featureType,key.toUtf8().data(),value.toUtf8().data())
        else:
            key = self.tagsTableWidget.item(row,0).text()
            if key=="" or key==self.newTagLabel:
                return

            # adding new tag and setting its key
            if column==0:

                # store it into database
                isAlreadyDef=self.plugin.dbm.isTagDefined(self.feature.id(),self.featureType,key.toUtf8().data())
                if isAlreadyDef:
                    # such a key already exists for this relation
                    self.tagsTableWidget.setItem(row,0,QTableWidgetItem(self.newTagLabel))
                    QMessageBox.information(self, self.tr("OSM Feature Dock Widget")
                        ,self.tr("Property with key '%1' already exists for this feature.").arg(key.toUtf8().data()))
                    return

                # well, insert new tag into database
                self.plugin.undoredo.startAction("Insert new tag.")
                self.plugin.dbm.insertTag(self.feature.id(),self.featureType,key.toUtf8().data(),'')

                self.__tagsLoaded=False

                self.tagsTableWidget.item(row,0).setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled)
                self.tagsTableWidget.item(row,1).setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled | Qt.ItemIsEditable)

                newLastRow = row+1
                self.tagsTableWidget.setRowCount(row+2)
                self.tagsTableWidget.setItem(newLastRow,0,QTableWidgetItem(self.newTagLabel))
                self.tagsTableWidget.setItem(newLastRow,1,QTableWidgetItem(""))
                self.tagsTableWidget.item(newLastRow,0).setFlags(Qt.ItemIsEnabled | Qt.ItemIsEditable)
                self.tagsTableWidget.item(newLastRow,1).setFlags(Qt.ItemIsEnabled)

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
            return

        if self.tagsEditIndex<>None:
            row=self.tagsEditIndex
            if row<>-1:
                value=self.tagsTableWidget.cellWidget(row,1).currentText()
                self.tagsTableWidget.item(row,1).setText(value)
                self.tagsTableWidget.removeCellWidget(row,1)

        key=self.tagsTableWidget.item(item.row(),0).text()
        tagValues=self.determineSuitableTagValues(self.featureType,key)

        if len(tagValues)>0:
            valCombo=QComboBox()
            valCombo.setEditable(True)
            valCombo.addItems(tagValues)
            currentComboText=self.tagsTableWidget.item(item.row(),1).text()
            ix=valCombo.findText(currentComboText)
            if ix==-1:
                valCombo.setEditText(currentComboText)
            else:
                valCombo.setCurrentIndex(ix)

            self.tagsTableWidget.setCellWidget(item.row(),1,valCombo)
            self.tagsEditIndex=item.row()
            QObject.connect(valCombo, SIGNAL("currentIndexChanged(const QString &)"), self.__onTagValueSelectionChanged)


    def __onTagValueSelectionChanged(self,value):
        """TODO: Function is called after currentIndexChanged(...) signal is emitted on combobox of table item.
        This combobox is related to table of relation tags (column Value).

        @param value new current value in combobox
        """

        row=self.tagsEditIndex
        self.tagsTableWidget.item(row,1).setText(value)
        self.tagsTableWidget.removeCellWidget(row,1)
        self.tagsEditIndex=-1


    def determineSuitableTagValues(self,featType,tagKey):
        """TODO: Function is used to find typical tag values for given relation type and given key.
        With help of this function plugin gives advice to user on relation creation.

        @param relType name of relation type
        @param tagKey key of tag
        @return list of typical tag values to given relation type
        """

        vals = []
        if featType in ('Point','Line','Polygon','Relation'):

            if tagKey=="highway":
                vals = ["trunk","motorway","primary","secondary","tertiary","residential"]

            elif tagKey=="boundary":
                vals = ["administrative","national_park","political","civil"]
            elif tagKey=="land_area":
                vals = ["administrative"]
            elif tagKey=="admin_level":
                vals = ["1","2","3","4","5","6","7","8","9","10","11"]
            elif tagKey=="restriction":
                vals = ["no_right_turn","no_left_turn","no_u_turn","no_straight_on","only_right_turn","only_left_turn","only_straight_on"]
            elif tagKey=="except":
                vals = ["psv","bicycle","hgv","motorcar"]
            elif tagKey=="route":
                vals = ["road","bicycle","foot","hiking","bus","pilgrimage","detour","railway","tram","mtb","roller_skate","running","horse"]
            elif tagKey=="network":
                vals = ["ncn","rcn","lcn","uk_ldp","lwn","rwn","nwn","e-road"]
            elif tagKey=="state":
                vals = ["proposed","alternate","temporary","connection"]

        return vals


    def __startIdentifyingFeature(self):
        """Function prepares feature identification.
        The appropriate map tool (IdentifyMapTool) is set to map canvas.
        """

        if self.__activeEditButton==self.identifyButton:
            return

        # clear dockwidget
        self.clear()

        self.plugin.iface.mainWindow().statusBar().showMessage("")

        self.__mapTool=IdentifyMapTool(self.plugin.canvas, self, self.plugin.dbm)
        self.plugin.canvas.unsetMapTool(self.plugin.canvas.mapTool())
        self.plugin.canvas.setMapTool(self.__mapTool)
        self.plugin.canvas.setCursor(QCursor(Qt.ArrowCursor))
        self.__activeEditButton=self.identifyButton
        self.plugin.canvas.setFocus(Qt.OtherFocusReason)


    def __startMovingFeature(self):
        """Function prepares feature moving.
        The appropriate map tool (MoveMapTool) is set to map canvas.
        """

        if self.__activeEditButton==self.moveButton:
            return

        # clear dockwidget
        self.clear()
        self.plugin.iface.mainWindow().statusBar().showMessage("Snapping ON. Hold Ctrl to disable it.")

        self.__mapTool=MoveMapTool(self.plugin)
        self.plugin.canvas.setMapTool(self.__mapTool)
        self.plugin.canvas.setCursor(QCursor(Qt.CrossCursor))
        self.__activeEditButton=self.moveButton
        self.plugin.canvas.setFocus(Qt.OtherFocusReason)


    def __startPointCreation(self):
        """Function prepares point creating operation.
        The appropriate map tool (CreatePointMapTool) is set to map canvas.
        """

        if self.__activeEditButton==self.createPointButton:
            return

        self.plugin.iface.mainWindow().statusBar().showMessage("Snapping ON. Hold Ctrl to disable it.")

        self.__mapTool=CreatePointMapTool(self.plugin)
        self.plugin.canvas.setMapTool(self.__mapTool)
        self.plugin.canvas.setCursor(QCursor(Qt.ArrowCursor))
        self.__activeEditButton=self.createPointButton
        self.plugin.canvas.setFocus(Qt.OtherFocusReason)


    def __startLineCreation(self):
        """Function prepares line creating operation.
        The appropriate map tool (CreateLineMapTool) is set to map canvas.
        """

        if self.__activeEditButton==self.createLineButton:
            return

        self.plugin.iface.mainWindow().statusBar().showMessage("Snapping ON. Hold Ctrl to disable it.")

        self.__mapTool=CreateLineMapTool(self.plugin)
        self.plugin.canvas.setMapTool(self.__mapTool)
        self.plugin.canvas.setCursor(QCursor(Qt.ArrowCursor))
        self.__activeEditButton=self.createLineButton
        self.plugin.canvas.setFocus(Qt.OtherFocusReason)


    def __startPolygonCreation(self):
        """Function prepares polygon creating operation.
        The appropriate map tool (CreatePolygonMapTool) is set to map canvas.
        """

        if self.__activeEditButton==self.createPolygonButton:
            return

        self.plugin.iface.mainWindow().statusBar().showMessage("Snapping ON. Hold Ctrl to disable it.")

        self.__mapTool=CreatePolygonMapTool(self.plugin)
        self.plugin.canvas.setMapTool(self.__mapTool)
        self.plugin.canvas.setCursor(QCursor(Qt.ArrowCursor))
        self.__activeEditButton=self.createPolygonButton
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
        self.__activeEditButton=self.identifyButton
        self.__activeEditButton.setChecked(True)


    def removeSelectedTags(self):
        """Function completely removes all tags that are currently selected in the appropriate
        list of the "OSM Feature" widget. More than one tag can be selected using Ctrl and clicking.
        """

        # remove selected tags (rows)
        selectedItems=self.tagsTableWidget.selectedItems()
        selectedRowsIndexes=[]
        lastRowIndex=self.tagsTableWidget.rowCount()-1
        self.tagsTableWidget.setCurrentCell(lastRowIndex,0)

        for i in selectedItems:
            if i.column()==0 and not i.row()==lastRowIndex:
                selectedRowsIndexes.append(i.row())

        self.plugin.undoredo.startAction("Remove tags.")

        selectedRowsIndexes.sort()
        selectedRowsIndexes.reverse()

        for ix in selectedRowsIndexes:

            key=self.tagsTableWidget.item(ix,0).text()
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

            self.tagsTableWidget.removeRow(ix)

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

        self.relTagsTreeWidget.clear()
        self.__relTagsLoaded=False
        # ask database manager for all relation tags
        self.featRelTags=self.plugin.dbm.getFeatureTags(relId,"Relation")

        self.relTagsTreeWidget.setColumnCount(2)
        self.relTagsTreeWidget.setHeaderLabels(["Key","Value"])

        for i in range(0,len(self.featRelTags)):
            self.relTagsTreeWidget.addTopLevelItem(QTreeWidgetItem([self.featRelTags[i][0],self.featRelTags[i][1]]))

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
        self.tagsTableWidget.clear()

        # fill tableWidget with tags of selected feature
        tableData=self.plugin.dbm.getFeatureTags(featId,featType)
        rowCount=len(tableData)
        self.__tagsLoaded=False

        self.tagsTableWidget.setRowCount(rowCount+1)
        self.tagsTableWidget.setColumnCount(2)
        self.tagsTableWidget.setHorizontalHeaderItem(0,QTableWidgetItem("Key"))
        self.tagsTableWidget.setHorizontalHeaderItem(1,QTableWidgetItem("Value"))

        for i in range(0,rowCount):
            self.tagsTableWidget.setItem(i,0,QTableWidgetItem(tableData[i][0]))
            self.tagsTableWidget.setItem(i,1,QTableWidgetItem(tableData[i][1]))
            self.tagsTableWidget.item(i,0).setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled)
            self.tagsTableWidget.item(i,1).setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled | Qt.ItemIsEditable)

        self.tagsTableWidget.setItem(rowCount,0,QTableWidgetItem(self.newTagLabel))
        self.tagsTableWidget.setItem(rowCount,1,QTableWidgetItem(""))
        self.tagsTableWidget.item(rowCount,0).setFlags(Qt.ItemIsEnabled | Qt.ItemIsEditable)
        self.tagsTableWidget.item(rowCount,1).setFlags(Qt.ItemIsEnabled)
        self.__tagsLoaded=True

        # enable tags table for editing
        self.tagsTableWidget.setEnabled(True)
        self.deleteTagsButton.setEnabled(True)


    def __loadFeatureRelations(self,featId,featType):
        """Functions loads the list of relations of specified feature.
        Feature is given by its identifier and its type.

        Loading is realized into the appropriate QListWidget of "OSM Feature" dockable widget.
        If no relation exists for specified feature, listWidget is filled with the only row with text: "<no relation>".

        @param featId identifier of feature to load
        @param featType type of feature to load - one of 'Point','Line','Polygon'
        """

        self.relTagsTreeWidget.setColumnCount(0)
        self.relMembersList.setEnabled(False)

        # disable widget buttons
        self.editRelationButton.setEnabled(False)
        self.removeRelationButton.setEnabled(False)

        # clear all tables connected to relations
        self.relListWidget.clear()
        self.relTagsTreeWidget.clear()
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
        self.relTagsTreeWidget.setEnabled(True)


    def reloadFeatureRelations(self):
        """Functions reloads the list of relations for currently loaded feature.

        Loading is realized into the appropriate QListWidget of "OSM Feature" dockable widget.
        If no relation exists for specified feature, listWidget is filled with the only row with text: "<no relation>".
        """

        self.relTagsTreeWidget.setColumnCount(0)
        self.relMembersList.setEnabled(False)

        # disable widget buttons
        self.editRelationButton.setEnabled(False)
        self.removeRelationButton.setEnabled(False)

        # clear all tables connected to relations
        self.relListWidget.clear()
        self.relTagsTreeWidget.clear()
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
        self.relTagsTreeWidget.setEnabled(True)


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


