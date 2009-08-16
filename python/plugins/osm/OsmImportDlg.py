# -*- coding: utf-8 -*-
"""@package OsmImportDlg
This module is used to import OSM data from standard QGIS vector layer.
"""


from ui_OsmImportDlg import Ui_OsmImportDlg

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import *


class dummyPoint:
    """A wrapper around QgsPoint which adds hash calculation.
    """

    def __init__(self, pt):
        """The constructor."""
        self.pt = pt

    def __hash__(self):
        return int(self.pt.x()*230783 + self.pt.y()*680091)

    def __eq__(self, other):
        return self.pt.x() == other.pt.x() and self.pt.y() == other.pt.y()



class dummyFeat:
    """A dummy QgsFeature that just returns its feature id.
    """

    def __init__(self, fid):
        """The constructor."""
        self.fid = fid

    def id(self):
        return self.fid


class OsmImportDlg(QDialog, Ui_OsmImportDlg):
    """This class provides structures and methods neccessary for import OSM data.
    Class is direct descendant of OSM Import dialog.

    After confirming OSM Import dialog process is immediately started.
    """

    def __init__(self, plugin):
        """The constructor.

        @param plugin is pointer to instance of OSM Plugin
        """

        QDialog.__init__(self, None)
        self.setupUi(self)

        self.plugin=plugin
        self.dbm = plugin.dbm
        self.affected=set()

        self.populateLayers()

        QObject.connect(self.buttonBox,SIGNAL("accepted()"),self.onOK)

        QObject.connect(self.cboLayer,SIGNAL("currentIndexChanged(int)"), self.updateUi)

        if self.cboLayer.count() > 0:
          self.updateUi(0)


    def updateUi(self, index):
        """Function checks whether there is any selection in the layer.
        """

        layerId = self.cboLayer.itemData(index).toString()
        layer = QgsMapLayerRegistry.instance().mapLayer(layerId)
        if layer is None or len(layer.selectedFeaturesIds()) == 0:
          self.chkOnlySelection.setChecked(False)
          self.chkOnlySelection.setEnabled(False)


    def populateLayers(self):
        """Funtion populates layers.
        """

        self.cboLayer.clear()
        layers = QgsMapLayerRegistry.instance().mapLayers()
        for lyrId,lyr in layers.iteritems():
          if lyr.type() == QgsMapLayer.VectorLayer and lyr.dataProvider().name() != "osm":
            self.cboLayer.addItem(lyr.name(), QVariant(lyrId) )


    def onOK(self):
        """Function does OSM data importing.
        """

        layerId = self.cboLayer.itemData(self.cboLayer.currentIndex()).toString()
        onlySel = self.chkOnlySelection.isChecked()

        layer = QgsMapLayerRegistry.instance().mapLayer(layerId)
        if layer is None:
          QMessageBox.warning(self, "Layer doesn't exist", "The selected layer doesn't exist anymore!")
          return

        self.progress = QProgressDialog("Importing features...", "Cancel", 0, 100, self)
        self.progress.setWindowModality(Qt.WindowModal)

        self.nodes = { }

        if onlySel:
          # only selected features
          features = layer.selectedFeatures()
          count = len(features)
          self.progress.setMaximum(count)
          for f in features:
            if not self.updateProgress():
              break
            self.addFeature(f)
        else:
          # all features from layer
          count = layer.featureCount()
          self.progress.setMaximum(count)
          layer.select()
          f = QgsFeature()
          while layer.nextFeature(f):
            if not self.updateProgress():
              break
            self.addFeature(f)

        self.dbm.commit()
        self.progress.setValue(count)

        self.dbm.recacheAffectedNow(self.affected)
        self.plugin.canvas.refresh()

        QMessageBox.information(self, "Import", "Import has been completed.")
        self.accept()


    def updateProgress(self):
        """Function updates progress dialog.
        """

        self.progress.setValue( self.progress.value()+1 )
        return not self.progress.wasCanceled()


    def addFeature(self,f):
        """Function adds given feature.

        @param f feature to be added
        """

        g = f.geometry()
        if g is None:
          return
        wkbType = g.wkbType()

        if wkbType == QGis.WKBPoint:
          self.extractPoint(g.asPoint())

        elif wkbType == QGis.WKBLineString:
          self.extractLineString(g.asPolyline())

        elif wkbType == QGis.WKBPolygon:
          self.extractPolygon(g.asPolygon())

        elif wkbType == QGis.WKBMultiPoint:
          for pnt in g.asMultiPoint():
            self.extractPoint(pnt)

        elif wkbType == QGis.WKBMultiLineString:
          for line in g.asMultiPolyline():
            self.extractLineString(line)

        elif wkbType == QGis.WKBMultiPolygon:
          for polygon in g.asMultiPolygon():
            self.extractPolygon(polygon)


    def extractPoint(self, pnt):
        """Function extracts a point.

        @param pnt point to extract
        """

        # TODO: check that another point isn't already at the same position
        (node,affected)=self.dbm.createPoint(pnt, None, None, False)
        self.affected.update(affected)


    def snapPoint(self, pnt):
        """ Function checks whether there's already other point with the same position.
        If so, it pretends it got snapped to that point.

        @param pnt point to snap
        """

        dp = dummyPoint(pnt)
        if dp in self.nodes:
          return (pnt, self.nodes[dp], 'Point')
        else:
          return (pnt,None,None)


    def extractLineString(self, line):
        """Function extracts a line.

        @param line line to extract
        """

        points = map(self.snapPoint, line)

        (feat,affected) = self.dbm.createLine(points, False)
        self.affected.update(affected)

        # this is a hack that uses the knowledge of inner working of createLine.
        # that sucks.
        nodeId = feat.id()-1
        for p in points:
          if p[1] is None:
            dp = dummyPoint(p[0])
            self.nodes[dp] = dummyFeat(nodeId)
          nodeId -= 1


    def extractPolygon(self, polygon):
        """Function extracts a polygon.

        @param polygon polygon to extract
        """

        # TODO: do something with holes?

        points=map(self.snapPoint, polygon[0])
        if len(points)>0:
            points.pop()

        (feat,affected)=self.dbm.createPolygon(points, False)
        self.affected.update(affected)

        # this is a hack that uses the knowledge of inner working of createPolygon.
        # that sucks.
        nodeId = feat.id()-1
        for p in points:
          if p[1] is None:
            dp = dummyPoint(p[0])
            self.nodes[dp] = dummyFeat(nodeId)
          nodeId -= 1

