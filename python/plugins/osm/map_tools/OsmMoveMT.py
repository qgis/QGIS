"""@package OsmMoveMT
This module holds all structures and methods required to perform move operation on OSM data.

Snapping to existing features is supported when moving a feature.
Moving process generates some rubberBands and vertexMarkers so that user can watch
the whole action on the map in a nice way.

There is also an interaction with plugin's "OSM Feature" dialog. Affected features are dynamically
loaded to it; thanks to that user is not confused about what (s)he is moving.
"""


from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *
from math import *


class OsmMoveMT(QgsMapTool):
    """This class represents map tool for feature moving (see QgsMapTool from Quantum GIS API).

    It enables to move any OSM feature. User is expected to left click, select and move... The second phase (selecting)
    is necessary, because there can be more than one feature at the same place.

    At the beginning of action left-click is used to choose the position on the map.
    Repeatable right-clicking is then used to select required feature from specified position.
    After that mouse moving moves selected feature.
    The last action is left-clicking again (that confirms the moving operation) or right-clicking (canceling operation).

    Snapping to existing features is supported when moving a feature. When moving a line/polygon, only three closest
    vertexes to the mouse position can be snapped. If snapping is enabled for all vertexes, operation will be very slow
    on features with many vertexes.
    When moving a point (also vertex of line/polygon) snapping to both vertexes and segments is done.
    When moving a line/polygon snapping to vertexes is supported only.

    Moving process generates some rubberBands and vertexMarkers so that user can watch
    the whole action on the map in a nice way.

    There is also an interaction with plugin's "OSM Feature" dialog. Affected features are dynamically
    loaded to it; thanks to that user is not confused about what (s)he is moving.

    Map tool catches the signal of changing OSM database. It such case not-ended operation is canceled.
    """


    def __init__(self, plugin):
        """The constructor.
        Initializes the map tool, creates necessary snappers.

        @param plugin pointer to OSM Plugin instance
        """

        QgsMapTool.__init__(self,plugin.canvas)
        self.canvas=plugin.canvas
        self.dockWidget=plugin.dockWidget
        self.dbm=plugin.dbm
        self.ur=plugin.undoredo

        # init info about feature that is being moved!
        self.mapPointFrom=None
        self.mapPointTo=None
        self.movFeatType=None
        self.movFeat=None
        self.movIsPolygon=False
        self.movIndexes=[]

        self.snapDeltas=None
        self.snapVertexIx=None
        self.snapFeat=None
        self.snapFeatType=None

        # init variables that keeps system state
        self.snappingEnabled=True
        self.doubleclick=False
        self.movingMode="INTRO"    # -> "SELECTION" -> "MOVING"
        self.moves=0
        self.featuresFound=[]
        self.ixFeature=0

        # init objects to display on canvas
        self.rubBands=[]            # rubBands of neighbour (non-point) feats that are also affected by moving operation
        self.isPolygonFlags=[]      # isPolygon flags for self.rubBands[]
        self.memberIndexes=[]
        self.snapRubBand=self.__createSnapRubberband()    # creating rubberband for snapped objects
        self.verMarker=self.__createVertexMarker()        # creating vertex marker for point moving

        # creating Vertex & Segment snapper + creating Vertex (only!) snapper
        self.snapperVS=self.__createVSSnapper(self.canvas.mapRenderer())
        self.snapperV=self.__createVSnapper(self.canvas.mapRenderer())

        # create dialog with feature selection
        self.dlgSelect=QDialog(self.dockWidget)
        self.dlgSelect.setWindowTitle("Feature identification")
        butBox=QDialogButtonBox(QDialogButtonBox.Ok|QDialogButtonBox.Cancel,Qt.Horizontal,self.dlgSelect)

        self.lw=QListWidget(self.dlgSelect)

        layout=QVBoxLayout(self.dlgSelect)
        layout.addWidget(self.lw)
        layout.addWidget(butBox)
        self.dlgSelect.setLayout(layout)

        # set dialog signals
        QObject.connect(butBox,SIGNAL("accepted()"),self.__onSelectDlgOK)
        QObject.connect(butBox,SIGNAL("rejected()"),self.__onSelectDlgCancel)

        self.dockWidget.plugin.iface.mainWindow().statusBar().showMessage("")


    def reinit(self):
        """Function re-initializes the map tool, prepare necessary rubberbands again.

        After calling this function, move map tool is in the same state as after its creation.
        """

        # reinit info about feature that is being moved!
        self.mapPointFrom=None
        self.mapPointTo=None
        self.movFeatType=None
        self.movFeat=None
        self.movIsPolygon=False
        self.movIndexes=[]

        self.snapDeltas=None
        self.snapVertexIx=None
        self.snapFeat=None
        self.snapFeatType=None

        # reinit variables that keeps system state
        self.snappingEnabled=True
        self.doubleclick=False
        self.movingMode="INTRO"    # -> "SELECTION" -> "MOVING"
        self.moves=0
        self.featuresFound=[]
        self.ixFeature=0

        # reinit objects to display on canvas
        for ix in range(0,len(self.rubBands)):
            self.rubBands[ix].reset(self.isPolygonFlags[ix])

        del self.rubBands
        self.rubBands=[]            # rubBands of neighbour feats that are also affected by moving operation

        del self.isPolygonFlags
        del self.memberIndexes
        self.isPolygonFlags=[]      # isPolygon flags for self.rubBands[]
        self.memberIndexes=[]

        self.snapRubBand.reset()    # todo: ??? polygon ???
        del self.snapRubBand
        self.snapRubBand=self.__createSnapRubberband()    # recreating rubberband for snapped objects

        self.verMarker.setCenter(QgsPoint(-1000,-1000))
        del self.verMarker
        self.verMarker=self.__createVertexMarker()        # recreating vertex marker for point moving

        self.dockWidget.plugin.iface.mainWindow().statusBar().showMessage("")


    def databaseChanged(self,dbKey):
        """This function is called automatically when current OSM database has changed.

        Function calls re-initialization of maptool and create new snappers again.

        @param dbKey key of database with new current OSM data
        """

        # re-initialization
        self.reinit()

        # plus creation of new snappers
        if dbKey:
            del self.snapperVS
            self.snapperVS=self.__createVSSnapper(self.canvas.mapRenderer())
            del self.snapperV
            self.snapperV=self.__createVSnapper(self.canvas.mapRenderer())


    def __createFeatRubberband(self,isPolygon):
        """Function creates rubberband that is used for marking moved feature on the map.

        @param isPolygon is hint for this function; it says if feature is of polygon type or not
        @return rubberband for marking moved feature on the map
        """

        # get qgis settings of line width and color for rubberband
        settings=QSettings()
        qgsLineWidth=2 # use fixed width
        qgsLineRed=settings.value( "/qgis/digitizing/line_color_red", QVariant(255) ).toInt()
        qgsLineGreen=settings.value( "/qgis/digitizing/line_color_green", QVariant(0) ).toInt()
        qgsLineBlue=settings.value( "/qgis/digitizing/line_color_blue", QVariant(0) ).toInt()

        rband=QgsRubberBand(self.canvas,isPolygon)
        rband.setColor(QColor(qgsLineRed[0],qgsLineGreen[0],qgsLineBlue[0]))
        rband.setWidth( qgsLineWidth )

        return rband


    def __createSnapRubberband(self):
        """Function creates rubberband that is used for marking map features
        to which snapping will be performed.

        @return rubberband for marking map features
        """

        # get qgis settings of line width and color for rubberband
        settings=QSettings()
        qgsLineWidth=2 # use fixed width

        rband=QgsRubberBand(self.canvas,False)
        rband.setColor(QColor(255,0,0))
        rband.setWidth(qgsLineWidth)

        return rband


    def __createVertexMarker(self):
        """Function creates vertexMarker that is used for marking moved feature (point) on the map.

        @return vertex marker for marking moved feature on map
        """

        # get qgis settings
        settings=QSettings()
        qgsLineWidth=2 # use fixed width
        qgsLineRed=settings.value("/qgis/digitizing/line_color_red",QVariant(255)).toInt()
        qgsLineGreen=settings.value("/qgis/digitizing/line_color_green",QVariant(0)).toInt()
        qgsLineBlue=settings.value("/qgis/digitizing/line_color_blue",QVariant(0)).toInt()

        verMarker=QgsVertexMarker(self.canvas)
        verMarker.setIconType(2)
        verMarker.setIconSize(13)
        verMarker.setColor(QColor(qgsLineRed[0],qgsLineGreen[0],qgsLineBlue[0]))
        verMarker.setPenWidth(qgsLineWidth)
        verMarker.setCenter(QgsPoint(-1000,-1000))

        return verMarker


    def __createVSSnapper(self,canvasRenderer):
        """Function creates snapper that snaps within standard qgis tolerance.

        Snapping of this snapper is done to all segments and vertexes
        of all three layers of current OSM database.

        @param canvasRenderer renderer of current map canvas
        @return instance of vertex+segment QgsSnapper
        """

        if not self.dbm.currentKey:
            # there is no current database -> no layer for snapping
            return QgsSnapper(canvasRenderer)

        snapper=QgsSnapper(canvasRenderer)
        snapLayers=[]

        # snap to osm layers from current database only
        sLayer=QgsSnapper.SnapLayer()
        sLayer.mLayer=self.dbm.pointLayers[self.dbm.currentKey]
        sLayer.mTolerance=QgsTolerance.vertexSearchRadius(sLayer.mLayer,canvasRenderer)
        sLayer.mSnapTo=QgsSnapper.SnapToVertex
        snapLayers.append(sLayer)

        sLayer=QgsSnapper.SnapLayer()
        sLayer.mLayer=self.dbm.lineLayers[self.dbm.currentKey]
        sLayer.mTolerance=QgsTolerance.vertexSearchRadius(sLayer.mLayer,canvasRenderer)
        sLayer.mSnapTo=QgsSnapper.SnapToVertexAndSegment
        snapLayers.append(sLayer)

        sLayer=QgsSnapper.SnapLayer()
        sLayer.mLayer=self.dbm.polygonLayers[self.dbm.currentKey]
        sLayer.mTolerance=QgsTolerance.vertexSearchRadius(sLayer.mLayer,canvasRenderer)
        sLayer.mSnapTo=QgsSnapper.SnapToVertexAndSegment
        snapLayers.append(sLayer)

        snapper.setSnapLayers(snapLayers)
        return snapper


    def __createVSnapper(self,canvasRenderer):
        """Function creates snapper that snaps within standard qgis tolerance.

        Snapping of this snapper is done to all vertexes (but not segments)
        of all three layers of current OSM database.

        @param canvasRenderer renderer of current map canvas
        @return instance of vertex QgsSnapper
        """

        if not self.dbm.currentKey:
            # there is no current database -> no layer for snapping
            return QgsSnapper(canvasRenderer)

        snapper=QgsSnapper(canvasRenderer)
        snapLayers=[]

        # snap to osm layers from current database only
        sLayer=QgsSnapper.SnapLayer()
        sLayer.mLayer=self.dbm.pointLayers[self.dbm.currentKey]
        sLayer.mTolerance=QgsTolerance.vertexSearchRadius(sLayer.mLayer,canvasRenderer)
        sLayer.mSnapTo=QgsSnapper.SnapToVertex
        snapLayers.append(sLayer)

        sLayer=QgsSnapper.SnapLayer()
        sLayer.mLayer=self.dbm.lineLayers[self.dbm.currentKey]
        sLayer.mTolerance=QgsTolerance.vertexSearchRadius(sLayer.mLayer,self.canvas.mapRenderer())
        sLayer.mSnapTo=QgsSnapper.SnapToVertex
        snapLayers.append(sLayer)

        sLayer=QgsSnapper.SnapLayer()
        sLayer.mLayer=self.dbm.polygonLayers[self.dbm.currentKey]
        sLayer.mTolerance=QgsTolerance.vertexSearchRadius(sLayer.mLayer,canvasRenderer)
        sLayer.mSnapTo=QgsSnapper.SnapToVertex
        snapLayers.append(sLayer)

        snapper.setSnapLayers(snapLayers)
        return snapper


    def deactivate(self):
        """Functions is called when move-map-tool is being deactivated.

        Function performs standard cleaning; re-initialization etc.
        """

        self.reinit()

        self.dockWidget.toolButtons.setExclusive(False)
        self.dockWidget.moveButton.setChecked(False)
        self.dockWidget.toolButtons.setExclusive(True)
        self.dockWidget.activeEditButton=self.dockWidget.dummyButton


    def keyPressEvent(self, event):
        """This function is called after keyPressEvent(QKeyEvent *) signal is emmited when using move map tool.
        If Control key was pressed, function disables snapping til key is released.

        @param event event that occured when key pressing
        """

        if (event.key() == Qt.Key_Control):
            self.snappingEnabled=False
            self.snapRubBand.reset()
            self.dockWidget.plugin.iface.mainWindow().statusBar().showMessage("Snapping OFF.")


    def keyReleaseEvent(self, event):
        """This function is called after keyReleaseEvent(QKeyEvent *) signal is emmited when using move map tool.
        If Control key was released, function enables snapping again.

        @param event event that occured when key releasing
        """

        if (event.key() == Qt.Key_Control):
            self.snappingEnabled = True
            self.dockWidget.plugin.iface.mainWindow().statusBar().showMessage("Snapping ON - hold Ctrl to disable it.")


    def canvasDoubleClickEvent(self,event):
        """This function is called after doubleclick on map when using move map tool.
        Function is interested into left-doubleclicking only.

        It finds out all features that are currently at the place where doubleclick was done.
        Then it shows simple dialog with the list of all these features. User can select the required one,
        close dialog and continue moving.

        @param event event that occured when double clicking
        """

        if event.button()<>Qt.LeftButton:
            return

        self.dockWidget.clear()
        #self.removeMarkers()

        # find out map coordinates from mouse click
        mapPoint=self.dockWidget.canvasToOsmCoords(event.pos())

        # display modal dialog with features selection
        self.featuresFound=self.dbm.findAllFeatures(mapPoint)
        self.ixFeature=0

        lwItems=[]
        for f in self.featuresFound:
            feat=f[0]
            featType=f[1]
            name=self.dbm.getTagValue(feat.id(),featType,"name")
            lwItems.append(QString("[%1] ").arg(feat.id()).append(featType).append(QString(" ")).append(name))

        self.lw.clear()
        self.lw.addItems(lwItems)

        self.dockWidget.plugin.iface.mainWindow().statusBar().showMessage("")
        self.doubleclick=True

        if self.dlgSelect:
            # continue only if OK button was clicked
            if self.dlgSelect.exec_()==0:
                return


    def canvasReleaseEvent(self, event):
        """This function is called after mouse button releasing on map when using move map tool.

        Such button releasing can have a lot of meanings.
        It depends on the current phase of moving. See documentation of the whole move map tool
        to know how moving works and how many times user has to release the mouse button to perform the whole moving.

        @param event event that occured when button releasing
        """

        if self.doubleclick:
            self.doubleclick=False
            return

        if self.movingMode=="INTRO":

            # we are interested only in left button clicking
            if event.button()<>Qt.LeftButton:
                return

            # find out map coordinates from mouse click
            self.mapPointFrom = self.dockWidget.canvasToOsmCoords(event.pos())

            # what to move?
            self.featuresFound=self.dbm.findAllFeatures(self.mapPointFrom)
            self.ixFeature=0

            if len(self.featuresFound)>0:
                self.movingMode="SELECTION"

                (feat,featType)=self.featuresFound[self.ixFeature]
                self.dockWidget.loadFeature(feat,featType,2)

            elif self.dockWidget.feature:
                self.dockWidget.clear()
                self.mapPointFrom=None
            return

        elif self.movingMode=="SELECTION":

            # we are interested only in left/right button clicking
            if event.button() not in (Qt.LeftButton,Qt.RightButton):
                return

            if event.button()==Qt.RightButton:
                if len(self.featuresFound)<1:
                    return

                self.ixFeature=self.ixFeature+1
                if self.ixFeature>=len(self.featuresFound):
                    self.ixFeature=0

                (feat,featType)=self.featuresFound[self.ixFeature]
                self.dockWidget.loadFeature(feat,featType,2)

            else: # LeftButton

                self.reinit()
                if self.dockWidget.feature:
                    self.dockWidget.clear()

        elif self.movingMode=="MOVING":
            # finish feature moving

            # we are interested only in left button clicking; other buttons just cancel moving operation
            if event.button()<>Qt.LeftButton:
                self.reinit()
                return

            whereIAm = self.dockWidget.canvasToOsmCoords(event.pos())

            if self.snapDeltas:
                self.mapPointTo=QgsPoint(whereIAm.x()+self.snapDeltas[0],whereIAm.y()+self.snapDeltas[1])
            else:
                # no snapping for this moving
                self.snapVertexIx=-1
                self.mapPointTo=whereIAm

            deltaX=self.mapPointTo.x()-self.mapPointFrom.x()
            deltaY=self.mapPointTo.y()-self.mapPointFrom.y()

            self.__finishFeatureMoving(deltaX,deltaY)


    def __tryIdentifyFeature(self,event):
        """Function just finds first feature at the place when event occured.
        Feature is marked on map with rubberBand (or vertexMarker) and is loaded to OSM Feature widget.

        @param event event that occured
        """

        # find out map coordinates from mouse click
        mapPoint=self.dockWidget.canvasToOsmCoords(event.pos())
        feature=self.dbm.findFeature(mapPoint)

        if feature:
            (feat,featType)=feature
            if not self.dockWidget.feature or feat.id()<>self.dockWidget.feature.id():
                self.dockWidget.loadFeature(feat,featType,1)

        elif self.dockWidget.feature:
            self.dockWidget.clear()


    def canvasMoveEvent(self,event):
        """This function is called after mouse moving.

        Mouse moving is ignored before feature (to move) is selected.

        @param event event that occured when mouse moving.
        """

        # ignore one move from each two moves
        if self.moves<>1:
            self.moves=self.moves+1
            return
        self.moves=0


        if self.movingMode=="INTRO":
            self.__tryIdentifyFeature(event)
            return

        if self.movingMode=="SELECTION":

            # remember what to move
            self.movFeat=self.featuresFound[self.ixFeature][0]
            self.movFeatType=self.featuresFound[self.ixFeature][1]
            self.movIsPolygon=False
            self.featuresFound=[]
            self.ixFeature=0

            # initializing rubberbands
            if self.movFeatType in ('Polygon','Line'):

                layer=self.dbm.lineLayers[self.dbm.currentKey]
                if self.movFeatType=='Polygon':
                    self.movIsPolygon=True
                    layer=self.dbm.polygonLayers[self.dbm.currentKey]

                # finding out three closest vertexes (for these snapping will be enabled)
                self.movIndexes=[]
                (p,ix,ixB,ixA,dis)=self.movFeat.geometry().closestVertex(self.mapPointFrom)
                self.movIndexes.append(ix)
                self.movIndexes.append(ixB)
                self.movIndexes.append(ixA)

                rubBand=self.__createFeatRubberband(self.movIsPolygon)
                rubBand.setToGeometry(self.movFeat.geometry(),layer)
                self.rubBands.append(rubBand)
                self.isPolygonFlags.append(self.movIsPolygon)

            elif self.movFeatType in ('Point'):

                # find out parent features
                (parentFeats,self.memberIndexes,self.isPolygonFlags)=self.dbm.getNodeParents(self.movFeat)
                self.movParentVertices=[]

                if len(parentFeats)==0:
                    self.verMarker.setCenter(self.movFeat.geometry().asPoint())

                for ix in range(0,len(parentFeats)):
                    layer=None
                    if self.isPolygonFlags[ix]:
                        layer=self.dbm.polygonLayers[self.dbm.currentKey]
                        self.movParentVertices=self.movParentVertices+(parentFeats[ix].geometry().asPolygon())[0]
                    else:
                        layer=self.dbm.lineLayers[self.dbm.currentKey]
                        self.movParentVertices=self.movParentVertices+parentFeats[ix].geometry().asPolyline()

                    parentRubBand=self.__createFeatRubberband(self.isPolygonFlags[ix])
                    parentRubBand.setToGeometry(parentFeats[ix].geometry(),layer)
                    self.rubBands.append(parentRubBand)

            if self.dockWidget.feature:
                self.dockWidget.clear()

            # change moving mode to the last one!
            self.movingMode="MOVING"

        # movingMode ~ "MOVING"
        if self.movFeatType=='Point':

            (deltaX,deltaY)=self.__getDeltaForPoint(event)    # snapping is done in this function
            targetPoint=QgsPoint(self.mapPointFrom.x()+deltaX,self.mapPointFrom.y()+deltaY)

            if len(self.rubBands)==0:
                point=self.movFeat.geometry().asPoint()
                self.verMarker.setCenter(QgsPoint(point.x()+deltaX,point.y()+deltaY))

            # move rubberbands
            for ix in range(0,len(self.rubBands)):
                for j in range(0,len(self.memberIndexes[ix])):
                    vertexIx=self.memberIndexes[ix][j]
                    lastVertexIx=self.rubBands[ix].numberOfVertices()-1
                    self.rubBands[ix].movePoint(vertexIx,targetPoint)

                    if self.isPolygonFlags[ix]:
                        if vertexIx==1:
                            self.rubBands[ix].movePoint(lastVertexIx,targetPoint)
                        elif vertexIx==lastVertexIx:
                            self.rubBands[ix].movePoint(vertexIx,targetPoint)

                    if vertexIx==1:
                        self.rubBands[ix].movePoint(0,targetPoint)

        elif self.movFeatType in ('Line','Polygon'):

            (deltaX,deltaY)=self.__getDeltaForLinePolygon(event)
            # move feature rubberband
            self.rubBands[0].setTranslationOffset(deltaX,deltaY)


    def __getDeltaForPoint(self,event):
        """Function gets an event object, performs snapping from place where event occured and then counts distance
        of found position from the place where the whole moving operation has started.

        Special version for points.

        @param event event that occured
        """

        # find out where and how far (from the place where moving was started) we are now
        mapPoint = self.dockWidget.canvasToOsmCoords(event.pos())

        if not self.snappingEnabled:
            self.snapDeltas=self.snapFeat=self.snapFeatType=None
            # returns how far from the place where moving was started we are now
            return (mapPoint.x()-self.mapPointFrom.x(),mapPoint.y()-self.mapPointFrom.y())

        # perform snapping
        self.movParentVertices.append(self.movFeat.geometry().asPoint())
        (retval,snappingResults)=self.snapperVS.snapPoint(event.pos(),self.movParentVertices)

        if len(snappingResults)==0:
            self.snapDeltas=self.snapFeat=self.snapFeatType=None
            # returns how far from the place where moving was started we are now
            return (mapPoint.x()-self.mapPointFrom.x(),mapPoint.y()-self.mapPointFrom.y())

        # we snapped successfully to something
        snappedPoint=QgsPoint(snappingResults[0].snappedVertex)

        self.snapDeltas=(snappedPoint.x()-mapPoint.x(),snappedPoint.y()-mapPoint.y())
        # use snappingResults[0].layer in findFeature() ??? findFeatureInLayer() ???
        (self.snapFeat,self.snapFeatType)=self.dbm.findFeature(snappedPoint)

        # returns how far from the place where moving was started we are now
        return (snappedPoint.x()-self.mapPointFrom.x(),snappedPoint.y()-self.mapPointFrom.y())


    def __getDeltaForLinePolygon(self,event):
        """Function gets an event object, performs snapping from place where event occured and then counts distance
        of found position from the place where the whole moving operation has started.

        Special version for lines and polygons.

        @param event event that occured
        """

        # find out where and how far (from the place where moving was started) we are now
        mapPoint = self.dockWidget.canvasToOsmCoords(event.pos())
        deltaX=mapPoint.x()-self.mapPointFrom.x()
        deltaY=mapPoint.y()-self.mapPointFrom.y()

        if not self.snappingEnabled:
            self.snapDeltas=self.snapFeat=self.snapFeatType=None
            return (deltaX,deltaY)

        lineMembers=[]
        for ix in self.movIndexes:
            lineMembers.append(self.movFeat.geometry().vertexAt(ix))

        allMembers=[]
        if self.movFeatType=='Line':
            allMembers=self.movFeat.geometry().asPolyline()
        else:
            polygon=self.movFeat.geometry().asPolygon()
            allMembers=polygon[0]

        minDistance=99999
        bestSnappedPoint=None
        bestActualLineMember=None

        for i in range(0,len(lineMembers)):

            actualLineMember=QgsPoint(lineMembers[i].x()+deltaX,lineMembers[i].y()+deltaY)
            point=self.canvas.getCoordinateTransform().transform(actualLineMember)
            (retval,snappingResults)=self.snapperV.snapPoint(QPoint(point.x(),point.y()),allMembers)

            if len(snappingResults)>0:
                snappedPoint=QgsPoint(snappingResults[0].snappedVertex)
                dX=snappedPoint.x()-(actualLineMember.x()+deltaX)
                dY=snappedPoint.y()-(actualLineMember.y()+deltaY)
                dist=sqrt(pow(dX,2)+pow(dY,2))    # pythagoras ;)

                if dist<minDistance:
                    minDistance=dist
                    bestSnappedPoint=snappedPoint
                    bestActualLineMember=actualLineMember
                    self.snapVertexIx=self.movIndexes[i]

        if not bestSnappedPoint:
            self.snapDeltas=self.snapFeat=self.snapFeatType=None
            return (deltaX,deltaY)

        self.snapDeltas=(bestSnappedPoint.x()-bestActualLineMember.x(),bestSnappedPoint.y()-bestActualLineMember.y())
        # use snappingResults[0].layer in findFeature() ??? findFeatureInLayer() ???
        (self.snapFeat,self.snapFeatType)=self.dbm.findFeature(bestSnappedPoint)
        return (deltaX+self.snapDeltas[0],deltaY+self.snapDeltas[1])


    def __finishFeatureMoving(self,deltaX,deltaY):
        """It finishes the whole moving process.
        Function also refreshes map canvas.

        @param deltaX distance from target position (of moving) to start position on X axis
        @param deltaY distance from target position (of moving) to start position on Y axis
        """

        affected=set()
        if self.movFeatType=="Point":
            self.ur.startAction("Move a point.")
            affected=self.dbm.movePoint(self.movFeat,deltaX,deltaY,self.snapFeat,self.snapFeatType)

        elif self.movFeatType=="Line":
            self.ur.startAction("Move a line.")
            affected=self.dbm.moveLine(self.movFeat,deltaX,deltaY,self.snapFeat,self.snapFeatType,self.snapVertexIx)

        elif self.movFeatType=="Polygon":
            self.ur.startAction("Move a polygon.")
            affected=self.dbm.movePolygon(self.movFeat,deltaX,deltaY,self.snapFeat,self.snapFeatType,self.snapVertexIx)

        self.ur.stopAction(affected)
        self.dbm.recacheAffectedNow(affected)
        self.dockWidget.loadFeature(self.movFeat,self.movFeatType,0)
        self.reinit()

        # reload map canvas so that changes take effect
        self.canvas.refresh()


    def __onSelectDlgOK(self):
        """This function handles clicking on OK button of selection dialog.
        """

        self.dlgSelect.close()

        if not self.lw.currentItem():
            return

        self.ixFeature=self.lw.currentRow()
        (feat,featType)=self.featuresFound[self.ixFeature]

        if feat:
            self.dockWidget.loadFeature(feat,featType,2)


    def __onSelectDlgCancel(self):
        """This function handles clicking on Cancel button of selection dialog.
        """

        self.dlgSelect.close()



