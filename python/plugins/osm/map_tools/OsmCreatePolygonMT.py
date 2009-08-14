"""@package OsmCreatePolygonMT
This module holds all structures and methods required to perform
"create polygon" operation on current OSM data.

Snapping to existing points is supported when creating new polygon.
Process generates some rubberBands and vertexMarkers so that user can watch
the whole operation on the map in a nice way.

There is also an interaction with plugin's "OSM Feature" dockwidget.
Points to which snapping is performed are loaded to it dynamically.
"""


from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *


class OsmCreatePolygonMT(QgsMapTool):
    """This class holds all structures and methods required to perform
    "create polygon" operation on current OSM data.

    Snapping to existing points is supported when creating new polygon.
    Process generates some rubberBands and vertexMarkers so that user can watch
    the whole operation on the map in a nice way.

    There is also an interaction with plugin's "OSM Feature" dockwidget.
    Points to which snapping is performed are loaded to it dynamically.
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

        # initialization
        self.snappingEnabled=True
        self.lastPointIsStable=True
        self.polygonPoints=[]
        self.snappedPoint=None
        self.snapFeat=None
        self.snapFeatType=None

        # creating rubberband which will be on new polygon
        self.polygonRubBand=self.createPolygonRubberband()

        # creating rubberband for snapped objects
        self.snapVerMarker=self.createSnapVertexMarker()

        # creating snapper to this map tool
        self.snapper=self.createSnapper(self.canvas.mapRenderer())


    def databaseChanged(self,dbKey):
        """This function is called automatically when current OSM database has changed.

        Function does re-initialization of maptool and create new snappers again (if necessary).

        @param dbKey key of database with new current OSM data
        """

        # re-initialization
        self.snappingEnabled=True
        self.snapFeat=None
        self.snapFeatType=None
        self.snapVerMarker.setCenter(QgsPoint(-1000,-1000))
        del self.snapVerMarker
        self.snapVerMarker=self.createSnapVertexMarker()
        self.polygonRubBand.reset(True)

        self.lastPointIsStable=True
        self.polygonPoints=[]
        self.snappedPoint=None

        if dbKey:
            del self.snapper
            self.snapper=self.createSnapper(self.canvas.mapRenderer())

        self.dockWidget.plugin.iface.mainWindow().statusBar().showMessage("")


    def createPolygonRubberband(self):
        """Function creates rubberband that is used for marking new polygon on the map.

        @return rubberband that marks new polygon
        """

        # get qgis settings of line width and color for rubberband
        settings = QSettings()
        qgsLineWidth = settings.value( "/qgis/digitizing/line_width", QVariant(10) ).toInt()
        qgsLineRed = settings.value( "/qgis/digitizing/line_color_red", QVariant(255) ).toInt()
        qgsLineGreen = settings.value( "/qgis/digitizing/line_color_green", QVariant(0) ).toInt()
        qgsLineBlue = settings.value( "/qgis/digitizing/line_color_blue", QVariant(0) ).toInt()

        rband=QgsRubberBand(self.canvas,True)
        rband.setColor( QColor(qgsLineRed[0],qgsLineGreen[0],qgsLineBlue[0]) )
        rband.setWidth( qgsLineWidth[0] )

        return rband


    def createSnapVertexMarker(self):
        """Function creates vertexMarker that is used for marking feature
        to which snapping was done.

        @return vertex marker - QgsVertexMarker object
        """

        # get qgis settings
        settings=QSettings()
        qgsLineWidth=settings.value("/qgis/digitizing/line_width",QVariant(10)).toInt()
        qgsLineRed=settings.value("/qgis/digitizing/line_color_red",QVariant(255)).toInt()
        qgsLineGreen=settings.value("/qgis/digitizing/line_color_green",QVariant(0)).toInt()
        qgsLineBlue=settings.value("/qgis/digitizing/line_color_blue",QVariant(0)).toInt()

        verMarker=QgsVertexMarker(self.canvas)
        verMarker.setIconType(2)
        verMarker.setIconSize(13)
        verMarker.setColor(QColor(qgsLineRed[0],qgsLineGreen[0],qgsLineBlue[0]))
        verMarker.setPenWidth(qgsLineWidth[0])
        verMarker.setCenter(QgsPoint(-1000,-1000))

        return verMarker


    def createSnapper(self,canvasRenderer):
        """Function creates snapper that snaps within standard qgis tolerance.

        Snapping of this snapper is done to all segments and vertexes
        of all three layers of current OSM database.

        @param canvasRenderer renderer of current map canvas
        @return instance of vertex+segment QgsSnapper
        """

        if not self.dbm.currentKey:
            # there is no current database -> no layer for snapping
            return QgsSnapper(self.canvas.mapRenderer())

        snapper=QgsSnapper(self.canvas.mapRenderer())
        snapLayers=[]

        # snap to osm layers from current database only
        sLayer=QgsSnapper.SnapLayer()
        sLayer.mLayer=self.dbm.pointLayers[self.dbm.currentKey]
        sLayer.mTolerance=QgsTolerance.vertexSearchRadius(sLayer.mLayer,self.canvas.mapRenderer())
        sLayer.mSnapTo=QgsSnapper.SnapToVertex
        snapLayers.append(sLayer)

        sLayer=QgsSnapper.SnapLayer()
        sLayer.mLayer=self.dbm.lineLayers[self.dbm.currentKey]
        sLayer.mTolerance=QgsTolerance.vertexSearchRadius(sLayer.mLayer,self.canvas.mapRenderer())
        sLayer.mSnapTo=QgsSnapper.SnapToVertex
        snapLayers.append(sLayer)

        sLayer=QgsSnapper.SnapLayer()
        sLayer.mLayer=self.dbm.polygonLayers[self.dbm.currentKey]
        sLayer.mTolerance=QgsTolerance.vertexSearchRadius(sLayer.mLayer,self.canvas.mapRenderer())
        sLayer.mSnapTo=QgsSnapper.SnapToVertex
        snapLayers.append(sLayer)

        snapper.setSnapLayers(snapLayers)
        return snapper


    def deactivate(self):
        """Functions is called when create polygon map-tool is being deactivated.

        Function performs standard cleaning; re-initialization etc.
        """

        self.polygonRubBand.reset(True)
        self.snapVerMarker.setCenter(QgsPoint(-1000,-1000))
        self.snappingEnabled=True
        self.lastPointIsStable=True
        self.polygonPoints=[]

        self.dockWidget.toolButtons.setExclusive(False)
        self.dockWidget.createPolygonButton.setChecked(False)
        self.dockWidget.toolButtons.setExclusive(True)
        self.dockWidget.activeEditButton=self.dockWidget.dummyButton


    def keyPressEvent(self, event):
        """This function is called after keyPressEvent(QKeyEvent *) signal
        is emmited when using this map tool.

        If Control key was pressed, function disables snapping til key is released again.

        @param event event that occured when key pressing
        """

        if (event.key() == Qt.Key_Control):
            self.snappingEnabled = False
            self.snapVerMarker.setCenter(QgsPoint(-1000,-1000))
            self.snappedPoint=None
            self.dockWidget.plugin.iface.mainWindow().statusBar().showMessage("Snapping OFF.")


    def keyReleaseEvent(self, event):
        """This function is called after keyReleaseEvent(QKeyEvent *) signal
        is emmited when using this map tool.

        If Control key was released, function enables snapping again.

        @param event event that occured when key releasing
        """

        if (event.key() == Qt.Key_Control):
            self.snappingEnabled = True
            self.dockWidget.plugin.iface.mainWindow().statusBar().showMessage("Snapping ON - hold Ctrl to disable it.")


    def canvasMoveEvent(self, event):
        """This function is called when mouse moving.

        @param event event that occured when mouse moving.
        """

        self.mapPoint=self.dockWidget.canvasToOsmCoords(event.pos())

        if len(self.polygonPoints)>0:
            if not self.lastPointIsStable:
                self.polygonRubBand.removeLastPoint()
            self.polygonRubBand.addPoint(QgsPoint(self.mapPoint.x(),self.mapPoint.y()))
            self.lastPointIsStable=False

        if not self.snappingEnabled:
            self.snapVerMarker.setCenter(self.mapPoint)
            return

        # snapping! first reset old snapping vertexMarker
        self.snapVerMarker.setCenter(QgsPoint(-1000,-1000))

        # try snapping to the closest vertex/segment
        (retval,snappingResults)=self.snapper.snapPoint(event.pos(),[])

        if len(snappingResults)==0:
            self.snapVerMarker.setCenter(self.mapPoint)
            self.snappedPoint=None
            self.snapFeat=None
            self.snapFeatType=None

            if self.dockWidget.feature:
                self.dockWidget.clear()
            return

        # process snapping result (get point, set rubberband)
        self.snappedPoint=QgsPoint(snappingResults[0].snappedVertex)
        self.snapVerMarker.setCenter(self.snappedPoint)

        if len(self.polygonPoints)>0:
            self.polygonRubBand.removeLastPoint()
            self.polygonRubBand.addPoint(QgsPoint(self.snappedPoint.x(),self.snappedPoint.y()))

        # start identification
        feature=self.dbm.findFeature(self.snappedPoint)
        if feature:
            (self.snapFeat,self.snapFeatType)=feature
            if not self.dockWidget.feature or self.snapFeat.id()<>self.dockWidget.feature.id():
                self.dockWidget.loadFeature(self.snapFeat,self.snapFeatType)


    def canvasReleaseEvent(self, event):
        """This function is called after mouse button releasing when using this map tool.

        If left button is released new vertex of polygon is created (pre-created).
        If right button is released the whole process of polygon creation is finished.

        @param event event that occured when button releasing
        """

        # we are interested only in left/right button clicking
        if event.button() not in (Qt.LeftButton,Qt.RightButton):
            return

        if event.button()==Qt.LeftButton:

            # where we are exactly?
            actualMapPoint = self.dockWidget.canvasToOsmCoords(event.pos())

            # what point will be the next polygon member?
            newPolygonPoint=actualMapPoint
            if self.snappedPoint:
                newPolygonPoint=self.snappedPoint

            # add new point into rubberband (and removing last one if neccessary) and into new polygon members list
            if not self.lastPointIsStable:
                self.polygonRubBand.removeLastPoint()
                self.lastPointIsStable=True

            self.polygonRubBand.addPoint(newPolygonPoint)
            self.polygonPoints.append((newPolygonPoint,self.snapFeat,self.snapFeatType))

        # right button clicking signalizes the last line member!
        elif event.button()==Qt.RightButton:

            # polygon must have at least three member points (triangle)
            if len(self.polygonPoints)<3:

                self.polygonRubBand.reset(True)
                self.snapVerMarker.setCenter(QgsPoint(-1000,-1000))
                self.lastPointIsStable=True
                self.polygonPoints=[]
                return

            self.ur.startAction("Create a polygon.")
            # call function of database manager that will create new polygon
            (polyg,affected)=self.dbm.createPolygon(self.polygonPoints)
            self.ur.stopAction(affected)
            self.dbm.recacheAffectedNow(affected)

            if polyg:
                self.dockWidget.loadFeature(polyg,'Polygon',2)

            # cleaning..
            self.polygonRubBand.reset(True)
            self.polygonPoints=[]

            # after polygon creation canvas must be refresh so that changes take effect on map
            self.canvas.refresh()



