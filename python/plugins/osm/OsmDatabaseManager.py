"""@package OsmDatabaseManager
This module provides methods to manipulate with database where OSM data are stored.

OsmDatabaseManager is the only part of OSM Plugin that has the right to access OSM (sqlite) database.
If any other part of plugin wants to manipulate with OSM data, it is expected to use constructs of this module.

This module can manage more than one database at a time.
But only one of such databases is considered to be the "current database" and all operations are done on it.

Module provides pretty good way to change "current database".
"""


from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.QtNetwork import *
from qgis.core import *

import sqlite3
import datetime



class OsmDatabaseManager:
    """This is the only class of OsmDatabaseManager module. Its purpose is to manage work with OSM databases.

    OsmDatabaseManager class provides method to add new OSM database.

    It holds all connections to all databases and holds the information
    on which database is currently used with all operations.

    If anyone wants to read data from (or write data to) other database then the "current one",
    method for changing current database must be called first.

    Class also provides method for performing both commit and rollback on current database.
    """

    def __init__(self,plugin):
        """The constructor.

        Initializes inner structures of OsmDatabaseManager and connect signals to appropriate slots.
        """

        self.plugin=plugin
        self.canvas=plugin.canvas

        self.dbConns={}    # map dbFileName->sqlite3ConnectionObject
        self.pointLayers={}
        self.lineLayers={}
        self.polygonLayers={}
        self.currentKey=None
        self.removing=False

        QObject.connect(self.plugin.iface,SIGNAL("currentLayerChanged(QgsMapLayer*)"),self.currLayerChanged)
        QObject.connect(QgsMapLayerRegistry.instance(),SIGNAL("layerWillBeRemoved(QString)"),self.layerRemoved)


    def currLayerChanged(self,layer):
        """Function is called after currentLayerChanged(QgsMapLayer*) signal is emitted.

        Information that we work with another OSM database from now is very important and almost all plugins' modules
        should know about this change.

        Identifying, editing, uploading, downloading,... - these all actions are performed on current database.
        If current database changes, operations must be terminated violently.

        Function set new current database and tells other modules about this change...

        @param layer pointer to QgsVectorLayer object that was set as new current layer
        """

        # set new currentKey and tell all other plugin components
        if not layer:
            self.currentKey=None
            self.plugin.undoredo.databaseChanged(None)
            self.plugin.dockWidget.databaseChanged(None)
            return

        if layer.type() != QgsMapLayer.VectorLayer or layer.dataProvider().name()<>"osm":
            self.currentKey=None
            self.plugin.undoredo.databaseChanged(None)
            self.plugin.dockWidget.databaseChanged(None)
            return

        # find out filename of new current database
        layerSource=layer.source()
        pos=layerSource.lastIndexOf("?")
        dbFileName=layerSource.left(pos)+".db"

        if dbFileName not in self.dbConns.keys():
            self.currentKey=None
            return

        if dbFileName.toLatin1().data()<>self.currentKey:
            self.currentKey=dbFileName.toLatin1().data()
            self.plugin.undoredo.databaseChanged(self.currentKey)
            self.plugin.dockWidget.databaseChanged(self.currentKey)


    def layerRemoved(self,layerID):
        """Function is called after layerWillBeRemoved(QString) signal is emitted.

        Through this signal Quantum GIS gives our plugin chance to prepare itself on removing of a vector layer.

        Plugin must find out if this layer is one of OSM layers.
        If it is, not only this layer but also the two with the same data source (OSM database) has to be removed.

        @param layerID unique Quantum GIS identifier of layer
        """

        if self.removing:
            return

        # get appropriate qgsvectorlayer object
        layer=QgsMapLayerRegistry.instance().mapLayer(layerID)

        if not layer:
            return            # strange situation

        if layer.type() != QgsMapLayer.VectorLayer or layer.dataProvider().name()<>"osm":
            return            # it's not OSM layer -> just ignore it

        # yes, it's osm layer; find out database file it's getting OSM data from
        layerSource=layer.source()
        pos=layerSource.lastIndexOf("?")
        dbFileName=layerSource.left(pos)+".db"
        key=dbFileName.toLatin1().data()

        # remove all map layers that belong to dbFileName database
        self.removing=True
        if layer.getLayerID()<>self.pointLayers[key].getLayerID():
            QgsMapLayerRegistry.instance().removeMapLayer(self.pointLayers[key].getLayerID(),True)
        if layer.getLayerID()<>self.lineLayers[key].getLayerID():
            QgsMapLayerRegistry.instance().removeMapLayer(self.lineLayers[key].getLayerID(),True)
        if layer.getLayerID()<>self.polygonLayers[key].getLayerID():
            QgsMapLayerRegistry.instance().removeMapLayer(self.polygonLayers[key].getLayerID(),True)
        self.removing=False

        # removed map items of key <dbFileName>
        del self.dbConns[key]
        del self.pointLayers[key]
        del self.lineLayers[key]
        del self.polygonLayers[key]


    def addDatabase(self,dbFileName,pointLayer,lineLayer,polygonLayer):
        """Function provides possibility to add new OSM data.

        It's called (mainly) from OSM data loader.
        New data (OSM database) is added into inner structures of OsmDatabaseManager.
        New database is automatically considered new current (!) OSM database.

        @param dbFileName filename of new OSM database
        @param pointLayer pointer to QGIS vector layer that represents OSM points (of new data)
        @param lineLayer pointer to QGIS vector layer that represents OSM lines (of new data)
        @param polygonLayer pointer to QGIS vector layer that represents OSM polygons (of new data)
        """

        # put new sqlite3 db connection into map (rewrite if its already there)
        key=dbFileName.toLatin1().data()
        if key in self.dbConns.keys():
            self.dbConns[key].close()
            del self.dbConns[key]
            del self.pointLayers[key]
            del self.lineLayers[key]
            del self.polygonLayers[key]

        self.dbConns[key]=sqlite3.connect(key)
        self.pointLayers[key]=pointLayer
        self.lineLayers[key]=lineLayer
        self.polygonLayers[key]=polygonLayer
        self.currentKey=key

        # tell everyone that database changes
        self.plugin.undoredo.databaseChanged(self.currentKey)
        self.plugin.dockWidget.databaseChanged(self.currentKey)


    def getConnection(self):
        """Function finds out current OSM data and returns pointer to related sqlite3 database.

        @return pointer to sqlite3 database connection object of current OSM data
        """

        if not self.currentKey in self.dbConns.keys():
            return None

        dbConn=self.dbConns[self.currentKey]
        if not dbConn:
            return None    # not OSM layer

        return dbConn


    def __getFreeFeatureId(self):
        """Finds out the highest feature id (less than zero),
        that is still not used and can be assigned to new feature.

        @return free identifier to be assigned to new feature
        """

        c=self.getConnection().cursor()
        c.execute("SELECT min(id) FROM (SELECT min(id) id FROM node UNION SELECT min(id) id FROM way UNION SELECT min(id) id FROM relation)")
        for rec in c:
            freeId = rec[0]
        c.close()

        if freeId<0:
            return freeId-1
        return -1


    def getTolerance(self):
        """Functions finds out default tolerance from qgis settings.

        Required value (of qgis settings) is returned by QgsTolerance.defaultTolerance(...) calling.
        If returned value equals 0.0, we ignore it and calculate our own tolerance from current extent width.

        @return default tolerance
        """

        qgisTol=QgsTolerance.defaultTolerance(self.pointLayers[self.currentKey],self.canvas.mapRenderer())

        if not self.currentKey or qgisTol==0.0:
            extent=self.canvas.extent()
            if self.plugin.dockWidget.coordXform is not None:
                extent=self.plugin.dockWidget.coordXform.transform(extent)

            w=extent.xMaximum()-extent.xMinimum()
            return w/220

        return qgisTol


    def findFeature(self,mapPoint):
        """Function finds exactly one feature at specified map point (within QGIS tolerance).

        It ignores the fact there can be more features at the same place.
        Typical usage is simple marking features when cursor goes over them.
        (use it anytime when you prefer speed to completeness)

        @param mapPoint point of the map where to search for feature
        @return found feature - pair (QgsFeature object,featureType)
        """

        if not self.currentKey:    # no layer loaded
            return None

        # finds out tolerance for snapping
        tolerance=self.getTolerance()
        area=QgsRectangle(mapPoint.x()-tolerance,mapPoint.y()-tolerance,mapPoint.x()+tolerance,mapPoint.y()+tolerance)

        feat=QgsFeature()
        lay=self.pointLayers[self.currentKey]
        lay.select([],area,True,True)
        result=lay.nextFeature(feat)
        lay.dataProvider().rewind()

        if result:
            return (feat,'Point')

        feat=QgsFeature()
        lay=self.lineLayers[self.currentKey]
        lay.select([],area,True,True)
        result=lay.nextFeature(feat)
        lay.dataProvider().rewind()

        if result:
            # line vertices
            c=self.getConnection().cursor()
            c.execute("select n.id,n.lat,n.lon from node n,way_member wm where n.u=1 and wm.u=1 and wm.way_id=:lineId and wm.node_id=n.id and n.status<>'R' and n.lat>=:minLat and n.lat<=:maxLat and n.lon>=:minLon and n.lon<=:maxLon"
                    ,{"minLat":area.yMinimum(),"maxLat":area.yMaximum(),"minLon":area.xMinimum(),"maxLon":area.xMaximum(),"lineId":str(feat.id())})

            for rec in c:
                feat2=QgsFeature(rec[0],"Point")
                feat2.setGeometry(QgsGeometry.fromPoint(QgsPoint(rec[2],rec[1])))
                # without features' attributes here! we don't need them...
                c.close()
                return (feat2,'Point')

            c.close()
            return (feat,'Line')

        feat=QgsFeature()
        lay=self.polygonLayers[self.currentKey]
        lay.select([],area,True,True)
        result=lay.nextFeature(feat)
        lay.dataProvider().rewind()

        if result:
            # polygon vertices
            c=self.getConnection().cursor()
            c.execute("select n.id,n.lat,n.lon from node n,way_member wm where n.u=1 and wm.u=1 and wm.way_id=:polygonId and wm.node_id=n.id and n.status<>'R' and n.lat>=:minLat and n.lat<=:maxLat and n.lon>=:minLon and n.lon<=:maxLon"
                    ,{"minLat":area.yMinimum(),"maxLat":area.yMaximum(),"minLon":area.xMinimum(),"maxLon":area.xMaximum(),"polygonId":str(feat.id())})

            for rec in c:
                feat2=QgsFeature(rec[0],"Point")
                feat2.setGeometry(QgsGeometry.fromPoint(QgsPoint(rec[2],rec[1])))
                # without features' attributes here! we don't need them...
                c.close()
                return (feat2,'Point')

            c.close()
            return (feat,'Polygon')
        return None


    def findAllFeatures(self,mapPoint):
        """Function finds all features at specified map point (within QGIS tolerance).
        (use it anytime when you prefer completeness to speed)

        @param mapPoint point of the map where to search for features
        @return list of found features - pairs of (QgsFeature object,featureType)
        """

        if not self.currentKey:    # no layer loaded
            return []

        foundPoints=[]
        foundLines=[]
        foundPolygons=[]

        # finds out tolerance for snapping
        tolerance=self.getTolerance()
        area=QgsRectangle(mapPoint.x()-tolerance,mapPoint.y()-tolerance,mapPoint.x()+tolerance,mapPoint.y()+tolerance)

        lay=self.pointLayers[self.currentKey]
        lay.select([],area,True,True)
        feat=QgsFeature()
        result=lay.nextFeature(feat)
        featMap={}

        while result:
            foundPoints.append((feat,'Point'))
            feat=QgsFeature()
            result=lay.nextFeature(feat)

        lay=self.lineLayers[self.currentKey]
        lay.select([],area,True,True)
        feat=QgsFeature()
        result=lay.nextFeature(feat)

        while result:
            # line vertices
            c=self.getConnection().cursor()
            c.execute("select n.id,n.lat,n.lon from node n,way_member wm where n.u=1 and wm.u=1 and wm.way_id=:lineId and wm.node_id=n.id and n.status<>'R' and n.lat>=:minLat and n.lat<=:maxLat and n.lon>=:minLon and n.lon<=:maxLon"
                    ,{"minLat":area.yMinimum(),"maxLat":area.yMaximum(),"minLon":area.xMinimum(),"maxLon":area.xMaximum(),"lineId":str(feat.id())})

            for rec in c:
                feat2=QgsFeature(rec[0],"Point")
                feat2.setGeometry(QgsGeometry.fromPoint(QgsPoint(rec[2],rec[1])))
                # without features' attributes here! we don't need them...
                featMap[feat2.id()]=feat2

            c.close()

            foundLines.append((feat,'Line'))
            feat=QgsFeature()
            result=lay.nextFeature(feat)

        lay=self.polygonLayers[self.currentKey]
        lay.select([],area,True,True)
        feat=QgsFeature()
        result=lay.nextFeature(feat)

        while result:
            # polygon vertices
            c=self.getConnection().cursor()
            c.execute("select n.id,n.lat,n.lon from node n,way_member wm where n.u=1 and wm.u=1 and wm.way_id=:polygonId and wm.node_id=n.id and n.status<>'R' and n.lat>=:minLat and n.lat<=:maxLat and n.lon>=:minLon and n.lon<=:maxLon"
                    ,{"minLat":area.yMinimum(),"maxLat":area.yMaximum(),"minLon":area.xMinimum(),"maxLon":area.xMaximum(),"polygonId":str(feat.id())})

            for rec in c:
                feat2=QgsFeature(rec[0],"Point")
                feat2.setGeometry(QgsGeometry.fromPoint(QgsPoint(rec[2],rec[1])))
                # without features' attributes here! we don't need them...
                featMap[feat2.id()]=feat2

            c.close()

            foundPolygons.append((feat,'Polygon'))
            feat=QgsFeature()
            result=lay.nextFeature(feat)

        res=foundPoints
        for key in featMap.keys():
            res.append((featMap[key],'Point'))
        res=res+foundLines+foundPolygons
        return res


    def createPoint(self,mapPoint,snapFeat,snapFeatType,doCommit=True):
        """Function creates new point.

        @param mapPoint is QgsPoint; it says where to create new point
        @param snapFeat is QgsFeature object to which snapping is performed
        @param snapFeatType is type of object to which snapping is performed
        @param doCommit flag; tells if this function should call commit before it finishes
        @return identifier of new node
        """

        # we need to get new id which will represents this new point in osm database
        nodeId=self.__getFreeFeatureId()

        affected=set()
        feat=QgsFeature(nodeId,"Point")
        feat.setGeometry(QgsGeometry.fromPoint(QgsPoint(mapPoint.x(),mapPoint.y())))

        # should snapping be done? if not, everything's easy
        if not snapFeat:
            c=self.getConnection().cursor()
            c.execute("insert into node (id,lat,lon,usage,status) values (:nodeId,:lat,:lon,0,'A')"
                    ,{"nodeId":str(nodeId),"lat":str(mapPoint.y()),"lon":str(mapPoint.x())})
            c.close()

            if doCommit:
                self.commit()

            affected.add((nodeId,'Point'))
            return (feat,affected)

        if snapFeatType=='Point':
            # what to do? we are trying to snap point to existing point
            QMessageBox.information(self.plugin.dockWidget,"OSM point creation"
                ,"Point creation failed. Two points cannot be at the same place.")
            return (None,[])

        # well, we are snapping to 'Line' or 'Polygon', actions are the same in both cases...
        c=self.getConnection().cursor()
        c.execute("insert into node (id,lat,lon,usage,status) values (:nodeId,:lat,:lon,1,'A')"
                ,{"nodeId":str(nodeId),"lat":str(mapPoint.y()),"lon":str(mapPoint.x())})
        c.close()

        # finding out exact position index of closest vertex in line (or polygon) geometry
        (a,b,vertexIx)=snapFeat.geometry().closestSegmentWithContext(mapPoint)
        newMemberIx=vertexIx+1

        # we need to shift indexes of all vertexes that will be after new vertex
        d=self.getConnection().cursor()
        d.execute("select way_id,pos_id from way_member where way_id=:wayId and pos_id>:posId and u=1 order by pos_id desc"
                 ,{"wayId":str(snapFeat.id()),"posId":str(vertexIx)})

        # for all such vertexes
        for rec in d:
            posId=rec[1]    # original position index
            c.execute("update way_member set pos_id=:newPosId where way_id=:wayId and pos_id=:posId and u=1"
                    ,{"wayId":str(snapFeat.id()),"posId":str(posId),"newPosId":str(posId+1)})
        d.close()

        # putting new node into set of lines/polygons members
        c.execute("insert into way_member (way_id,node_id,pos_id) values (:wayId,:nodeId,:posId)"
                ,{"wayId":str(snapFeat.id()),"nodeId":str(nodeId),"posId":str(newMemberIx)})

        # original line/polygon geometry must be forgotten (new one will be created by provider on canvas refresh)
        c.execute("update way set wkb=:wkb where id=:wayId and u=1"
                 ,{"wkb":sqlite3.Binary(""),"wayId":str(snapFeat.id())})

        if snapFeatType=='Line':
            self.changeLineStatus(snapFeat.id(),"N","U")
        elif snapFeatType=='Polygon':
            self.changePolygonStatus(snapFeat.id(),"N","U")

        # finishing
        c.close()

        if doCommit:
            self.commit()

        affected.add((nodeId,'Point'))
        if snapFeatType=='Line':
            affected.add((snapFeat.id(),'Line'))
        elif snapFeatType=='Polygon':
            affected.add((snapFeat.id(),'Polygon'))

        return (feat,affected)


    def createLine(self,mapPoints, doCommit=True):
        """Function creates new line.

        @param mapPoints is list of line points
        @param doCommit flag; tells if this function should call commit before it finishes
        @return identifier of new line
        """

        # getting id which will represent this new line in osm database
        lineId=self.__getFreeFeatureId()

        affected=set()
        pline=[]

        # creating database cursor and inits...
        c=self.getConnection().cursor()
        cnt=len(mapPoints)
        i,minLat,minLon,maxLat,maxLon=0,9999999,9999999,-9999999,-9999999

        # go through the line points
        for (mapPoint,snapFeat,snapFeatType) in mapPoints:

            lat=mapPoint.y()
            lon=mapPoint.x()
            pline.append(QgsPoint(mapPoint.x(),mapPoint.y()))

            # we need to know max and min latitude/longitude of the whole line (its boundary)
            if lat<minLat:
                minLat=lat
            if lon<minLon:
                minLon=lon
            if lat>maxLat:
                maxLat=lat
            if lon>maxLon:
                maxLon=lon

            nodeId=None
            if snapFeat:
                if snapFeatType=='Point':
                    nodeId=snapFeat.id()
                    # update record of existing node to which we has snapped
                    c.execute("update node set usage=usage+1 where id=:nodeId and u=1"
                            ,{"nodeId":str(nodeId)})
                else:
                    # snapping to non-point features is forbidden!
                    return (None,[])

            else:
                # this vertex is a new one (no snapping to existing one)
                nodeId=lineId-i-1
                c.execute("insert into node (id,lat,lon,usage,status) values (:nodeId,:lat,:lon,1,'A')"
                        ,{ "nodeId":str(nodeId),"lat":str(lat),"lon":str(lon) })

                affected.add((nodeId,'Point'))

            # insert record into table of way members
            c.execute("insert into way_member (way_id,pos_id,node_id) values (:wayId,:posId,:nodeId)",{
                         "wayId":str(lineId)
                        ,"posId":str(i+1)
                        ,"nodeId":str(nodeId)
                    })
            i=i+1

        # create half-empty database record for new line
        c.execute("insert into way (id,wkb,membercnt,closed,min_lat,min_lon,max_lat,max_lon,status) values (?,?,?,0,?,?,?,?,'A')"
                ,(str(lineId),sqlite3.Binary(""),str(cnt),str(minLat),str(minLon),str(maxLat),str(maxLon)))

        # finishing...
        c.close()
        feat=QgsFeature(lineId,"Line")
        feat.setGeometry(QgsGeometry.fromPolyline(pline))

        if doCommit:
          self.commit()

        affected.add((lineId,'Line'))
        return (feat,affected)


    def createPolygon(self,mapPoints, doCommit=True):
        """Function creates new polygon.

        @param mapPoints is list of polygon points
        @param doCommit flag; tells if this function should call commit before it finishes
        @return identifier of new polygon
        """

        # getting id which will represents this new polygon in osm database
        polygonId=self.__getFreeFeatureId()

        affected=set()
        pline=[]

        # creating database cursor and inits...
        c=self.getConnection().cursor()
        cnt=len(mapPoints)
        i,minLat,minLon,maxLat,maxLon=0,9999999,9999999,-9999999,-9999999

        # go through the polygon points
        for (mapPoint,snapFeat,snapFeatType) in mapPoints:

            lat=mapPoint.y()
            lon=mapPoint.x()
            pline.append(QgsPoint(mapPoint.x(),mapPoint.y()))

            # we need to know max and min latitude/longitude of the whole polygon (its boundary)
            if lat<minLat:
                minLat=lat
            if lon<minLon:
                minLon=lon
            if lat>maxLat:
                maxLat=lat
            if lon>maxLon:
                maxLon=lon

            nodeId=None
            if snapFeat:
                if snapFeatType=='Point':
                    nodeId=snapFeat.id()
                    # update record of existing node to which snapping was done
                    c.execute("update node set usage=usage+1 where id=:nodeId and u=1"
                            ,{"nodeId":str(nodeId)})
                else:
                    # snapping to non-point feature is not allowed
                    nodeId=None

            else:
                # this vertex is a new one (no snapping to existing one)
                nodeId=polygonId-i-1
                c.execute("insert into node (id,lat,lon,usage,status) values (:nodeId,:lat,:lon,1,'A')"
                        ,{ "nodeId":str(nodeId),"lat":str(lat),"lon":str(lon) })

                affected.add((nodeId,'Point'))

            # insert record into table of way members
            c.execute("insert into way_member (way_id,pos_id,node_id) values (:wayId,:posId,:nodeId)",{
                         "wayId":str(polygonId)
                        ,"posId":str(i+1)
                        ,"nodeId":str(nodeId)
                    })
            i=i+1

        # create half-empty database record for new polygon
        c.execute("insert into way (id,wkb,membercnt,closed,min_lat,min_lon,max_lat,max_lon,status) values (?,?,?,1,?,?,?,?,'A')"
                ,(str(polygonId),sqlite3.Binary(""),str(cnt),str(minLat),str(minLon),str(maxLat),str(maxLon)))

        # finish
        c.close()
        feat=QgsFeature(polygonId,"Polygon")
        polygon=[]
        polygon.append(pline)
        feat.setGeometry(QgsGeometry.fromPolygon(polygon))

        if doCommit:
          self.commit()

        affected.add((polygonId,'Polygon'))
        return (feat,affected)


    def createRelation(self,relType,relMems):
        """Function creates new relation.

        @param relType is name of relation type
        @param relMems is list of relation members
        @return unique identifier of new relation
        """

        # we need to get new id which will represents new relation in osm database
        relId=self.__getFreeFeatureId()

        c=self.getConnection().cursor()

        # insert relation record
        c.execute("insert into relation (id, type, timestamp, user, status) values (:id, :type, :timestamp, :user, :status)"
                 ,{"id":relId,"type":relType,"timestamp":'',"user":'',"status":"A"})

        # insert relation members records into "relation_member" table
        posId=1
        for relMem in relMems:

            memId=relMem[0]
            memType=relMem[1]
            memRole=relMem[2]

            osmType=self.convertToOsmType(memType)
            c.execute("insert into relation_member (relation_id,pos_id,member_id,member_type,role) values (?,?,?,?,?)"
                    ,(str(relId),posId,memId,osmType,memRole))

            # increase position number
            posId=posId+1

        c.close()
        return relId


    def removePoint(self,pointId):
        """Function removes an existing point.
        Point is given by its identifier.

        @param pointId identifier of point to remove
        @return list of all features affected with this operation
        """

        affected=set()

        # first change status of point to 'R' ~ Removed
        self.changePointStatus(pointId,None,"R")

        # remove all points' tags
        self.removeFeaturesTags(pointId,"Point")

        # remove point from all lines
        affeA=self.__removePointFromAllLines(pointId)

        # remove point from all polygons
        affeB=self.__removePointFromAllPolygons(pointId)

        # remove point from all its relations
        self.__removeFeatureFromAllRelations(pointId,"Point")

        self.commit()

        affected.add((pointId,'Point'))
        affected.update(affeA)
        affected.update(affeB)

        return affected


    def __removeIsolatedPoint(self,pointId):
        """Removes an existing point which is not part of any other feature.
        Point is given by its identifier.
        Function must be private, coz' it doesn't verify if point is really isolated.

        @param pointId is a point identifier
        """

        # first change status of point to 'R' ~ Removed
        self.changePointStatus(pointId,None,"R")

        # remove all points' tags
        self.removeFeaturesTags(pointId,"Point")

        # remove point from all its relations
        self.__removeFeatureFromAllRelations(pointId,"Point")

        self.commit()


    def __removePointFromAllPolygons(self,pointId):
        """Function removes given point from all polygons.
        It's possible that point has multiple occurance in the same polygon,
        function removes all the occurances in all polygons.

        @param pointId identifier of point to remove
        @return list of all features affected with this operation
        """

        affected=set()
        c=self.getConnection().cursor()
        c.execute("select w.id from way w where w.closed=1 and w.u=1 and exists(select 1 from way_member wm \
                   where wm.node_id=:pointId and wm.way_id=w.id and wm.u=1)"
                ,{"pointId":str(pointId)})

        for rec in c:
            polId=rec[0]
            aff=self.__removePointFromPolygon(polId,pointId)
            affected.update(aff)

        # and that's all
        c.close()
        self.commit()

        affected.add((pointId,'Point'))
        return affected


    def __removePointFromPolygon(self,polygonId,pointId):
        """Function removes given point from given polygon.
        It's possible that point has multiple occurance in the polygon,
        function removes all such occurances.

        @param polygonId identifier of polygon to remove point from
        @param pointId identifier of point to remove
        @return list of all features affected with this operation
        """

        affected=set()
        c=self.getConnection().cursor()

        # find all occurences of point in polygon (get list of position identifiers)
        c.execute("select pos_id from way_member where way_id=:polygonId and node_id=:pointId and u=1"
                ,{"polygonId":str(polygonId),"pointId":str(pointId)})

        for rec in c:
            aff=self.__removePolygonMember(polygonId,pointId,rec[0])
            affected.update(aff)

        c.execute("update node set usage=usage-1 where id=:pointId and u=1 and usage>0"
                ,{"pointId":str(pointId)})

        # mark polygon as 'Updated' and clear its geometry to make provider able to generate a new one
        c.execute("update way set wkb=:wkb where id=:polygonId and u=1"
                ,{"wkb":sqlite3.Binary(""),"polygonId":str(polygonId)})
        self.changePolygonStatus(polygonId,"N","U")

        # verify that polygon has still sufficient number of members (>2) required by polygon definition
        c.execute("select count(pos_id) from way_member where way_id=:polygonId and u=1"
                ,{"polygonId":str(polygonId)})

        polMembersCnt=0
        for rec in c:
            polMembersCnt=rec[0]

        if polMembersCnt<1:
            # the feature can no longer be called "polygon", it has no members
            aff=self.removePolygon(polygonId,True)
            affected.update(aff)

        elif polMembersCnt<3:
            # the feature can no longer be called "polygon", is it a line now?
            aff=self.__convertPseudoPolygonToLine(polygonId)
            affected.update(aff)

        # a-a-and that's all folks!
        c.close()
        self.commit()

        affected.add((polygonId,'Polygon'))
        affected.add((pointId,'Point'))

        return affected


    def __removePolygonMember(self,polygonId,pointId,posId):
        """Function removes exactly one member given by its position in polygon.

        "Polygon member" is interpreted as exactly one occurance of a point in a polygon.
        So... two members of the same polygon can still be the same point.

        All polygon members that were at higher positions will get new position numbers (their oldPosition-1).

        @param polygonId identifier of polygon to remove member from
        @param pointId identifier of point
        @param posId identifier of exact position from where to remove a point
        @return list of all features affected with this operation
        """

        affected=set()
        c=self.getConnection().cursor()
        c.execute("delete from way_member where way_id=:polygonId and pos_id=:posId and u=1"
                ,{"polygonId":str(polygonId),"posId":str(posId)})

        c.execute("update way_member set pos_id=pos_id-1 where way_id=:polygonId and pos_id>:posId and u=1"
                ,{"polygonId":str(polygonId),"posId":str(posId)})
        c.close()

        affected.add((polygonId,'Polygon'))
        affected.add((pointId,'Point'))

        return affected


    def __removePointFromAllLines(self,pointId):
        """Function removes given point from all lines.
        It's possible that point has multiple occurance in the same line,
        function removes all the occurances in all lines.

        @param pointId identifier of point to remove
        @return list of all features affected with this operation
        """

        affected=set()
        c=self.getConnection().cursor()
        c.execute("select w.id from way w where w.closed=0 and w.u=1 and exists(select 1 from way_member wm \
                   where wm.node_id=:pointId and wm.way_id=w.id and wm.u=1)"
                ,{"pointId":str(pointId)})

        for rec in c:
            lineId=rec[0]
            aff=self.__removePointFromLine(lineId,pointId)
            affected.update(aff)

        # and that's all
        c.close()
        self.commit()

        affected.add((pointId,'Point'))
        return affected


    def __removePointFromLine(self,lineId,pointId):
        """Function removes given point from given line.
        It's possible that point has multiple occurance in the line,
        function removes all such occurances.

        @param lineId identifier of line to remove point from
        @param pointId identifier of point to remove
        @return list of all features affected with this operation
        """

        affected=set()
        c=self.getConnection().cursor()
        # find all occurences of point in line (get list of position identifiers)
        c.execute("select pos_id from way_member where way_id=:lineId and node_id=:pointId and u=1"
                ,{"lineId":str(lineId),"pointId":str(pointId)})

        for rec in c:
            aff=self.__removeLineMember(lineId,pointId,rec[0])
            affected.update(aff)

        c.execute("update node set usage=usage-1 where id=:pointId and u=1 and usage>0"
                ,{"pointId":str(pointId)})

        # mark line as 'Updated' and clear its geometry to make provider able to generate a new one
        c.execute("update way set wkb=:wkb where id=:lineId and u=1"
                ,{"wkb":sqlite3.Binary(""),"lineId":str(lineId)})
        self.changeLineStatus(lineId,"N","U")

        # verify that line has still sufficient number of members (>1) required by line definition
        c.execute("select count(pos_id) from way_member where way_id=:lineId and u=1"
                ,{"lineId":str(lineId)})

        lineMembersCnt=0
        for rec in c:
            lineMembersCnt=rec[0]

        c.close()
        self.commit()

        if lineMembersCnt<2:
            # the feature can no longer be called "line", is it a point now?
            aff=self.__convertPseudoLineToPoint(lineId)
            affected.update(aff)

        # a-a-and that's all folks!
        affected.add((pointId,'Point'))
        affected.add((lineId,'Line'))

        return affected


    def __removeLineMember(self,lineId,pointId,posId):
        """Function removes exactly one member given by its position in line.

        "Line member" is interpreted as exactly one occurance of a point in a line.
        So... two members of the same line can still be the same point.

        All line members that were at higher positions will get new position numbers (their oldPosition-1).

        @param lineId identifier of line to remove member from
        @param pointId identifier of point
        @param posId identifier of exact position from where to remove a point
        @return list of all features affected with this operation
        """

        affected=set()
        c=self.getConnection().cursor()
        c.execute("delete from way_member where way_id=:lineId and pos_id=:posId and u=1"
                ,{"lineId":str(lineId),"posId":str(posId)})

        c.execute("update way_member set pos_id=pos_id-1 where way_id=:lineId and pos_id>:posId and u=1"
                ,{"lineId":str(lineId),"posId":str(posId)})
        c.close()

        affected.add((lineId,'Line'))
        affected.add((pointId,'Point'))

        return affected


    def removeLine(self,lineId,delMembers=True):
        """Function removes an existing line.
        Line is given by its identifier.

        @param lineId identifier of line to remove
        @param delMembers if True line will be removed with all its "isolated" (after line's removing) points; if False points won't be removed
        @return list of all features affected with this operation
        """

        affected=set()
        # first change status of line to 'R' ~ Removed
        self.changeLineStatus(lineId,None,"R")

        # remove all lines' tags
        self.removeFeaturesTags(lineId,"Line")

        if delMembers:
            # just remove all points for which this line is the only line/polygon they are members of
            points=[]
            c=self.getConnection().cursor()
            c.execute("select id from node where exists( select 1 from way_member wm where wm.node_id=id and wm.way_id=:lineId and wm.u=1 ) \
                       and usage=1 and u=1 and status<>'R'",{"lineId":str(lineId)})
            # collection ids of line members
            for rec in c:
                points.append(rec[0])
            c.close()

            # now remove them one by one
            for pId in points:
                self.__removeIsolatedPoint(pId)
                affected.add((pId,'Point'))
        else:
            # all points has to be removed from line
            points=[]
            c=self.getConnection().cursor()
            c.execute("select node_id from way_member wm where wm.way_id=:lineId and wm.u=1"
                    ,{"lineId":str(lineId)})
            # collection ids of line members
            for rec in c:
                points.append(rec[0])
            c.close()

            # now remove them one by one
            for pId in points:
                aff=self.__removePointFromLine(lineId,pId)
                affected.update(aff)

        # remove all relevant line-point connections
        c=self.getConnection().cursor()
        c.execute("delete from way_member where way_id=:lineId and u=1"
                ,{"lineId":str(lineId)})

        c.close()

        # don't forget to remove line from existing relations
        self.__removeFeatureFromAllRelations(lineId,"Line")
        self.commit()

        affected.add((lineId,'Line'))
        return affected


    def removePolygon(self,polId,delMembers=True):
        """Function removes an existing polygon.
        Polygon is given by its identifier.

        @param polId identifier of polygon to remove
        @param delMembers if True polygon will be removed with all its "isolated" (after line's removing) points; if False points won't be removed
        @return list of all features affected with this operation
        """

        affected=set()
        # first change status of polygon to 'R' ~ Removed
        self.changeLineStatus(polId,None,"R")

        # remove all polygons' tags
        self.removeFeaturesTags(polId,"Line")

        if delMembers:
            # just remove all points for which this polygon is the only line/polygon they are members of
            points=[]
            c=self.getConnection().cursor()
            c.execute("select id from node where exists( select 1 from way_member wm where wm.node_id=id and wm.way_id=:polId and wm.u=1 ) \
                       and usage=1 and u=1",{"polId":str(polId)})
            # collection ids of line members
            for rec in c:
                points.append(rec[0])
            c.close()

            # now remove them one by one
            for pId in points:
                self.__removeIsolatedPoint(pId)
                affected.add((pId,'Point'))

        else:
            # all points has to be removed from polygon
            points=[]
            c=self.getConnection().cursor()
            c.execute("select node_id from way_member wm where wm.way_id=:polId and wm.u=1"
                    ,{"polId":str(polId)})

            # collection ids of line members
            for rec in c:
                points.append(rec[0])
            c.close()

            # now remove them one by one
            for pId in points:
                aff=self.__removePointFromPolygon(polId,pId)
                affected.update(aff)

        # remove all relevant polygon-point connections
        c=self.getConnection().cursor()
        c.execute("delete from way_member where way_id=:polId and u=1"
                ,{"polId":str(polId)})

        c.close()

        # don't forget to remove polygon from existing relations
        self.__removeFeatureFromAllRelations(polId,"Polygon")

        self.commit()

        affected.add((polId,'Polygon'))
        return affected


    def removeRelation(self,relId):
        """Function removes an existing relation.
        Relation is given by its identifier.

        @param relId identifier of relation to remove
        """

        c=self.getConnection().cursor()
        c.execute("delete from relation_member where relation_id=:relId and u=1"
                ,{"relId":str(relId)})

        c.execute("update relation set status='R' where id=:relId and u=1"
                ,{"relId":str(relId)})

        # remove all tags connected to the relation
        c.execute("delete from tag where object_id=:relId and object_type='relation' and u=1"
                ,{"relId":str(relId)})

        # don't forget to remove relation (as member) from existing relations
        self.__removeFeatureFromAllRelations(relId,"Relation")

        # finish
        c.close()


    def __removeFeatureFromAllRelations(self,featId,featType):
        """Removes feature from all its relations.

        If feature occures at more than one position in some relation, all its occurences will be removed.
        If some relation becomes empty with members deletion, the whole relation is removed.

        @param featId identifier of feature to remove
        @param featType type of feature to remove
        """

        c=self.getConnection().cursor()
        osmType=self.convertToOsmType(featType)

        # remove feature from all its relations
        c.execute("select relation_id from relation_member where member_id=:memId and member_type=:memType and u=1"
                ,{"memId":str(featId),"memType":osmType})

        for rec in c:
            relId=rec[0]
            self.__removeFeatureFromRelation(relId,featId,featType)

        # and that's all
        c.close()


    def changeAllRelationMembers(self,relId,newMembers):
        """Function first removes all relation members and then inserts new ones.

        @param relId identifier of relation
        @param newMembers new relation members
        """

        c=self.getConnection().cursor()
        c.execute("delete from relation_member where relation_id=:relId"
                ,{"relId":str(relId)})

        # insert relation members records into "relation_member" table
        posId=1
        for relMem in newMembers:

            memId=relMem[0]
            memType=relMem[1]
            memRole=relMem[2]

            osmType=self.convertToOsmType(memType)
            c.execute("insert into relation_member (relation_id,pos_id,member_id,member_type,role) values (?,?,?,?,?)"
                    ,(str(relId),posId,memId,osmType,memRole))

            # increase position number
            posId=posId+1
        c.close()


    def __removeFeatureFromRelation(self,relId,featId,featType):
        """Funcion removes feature from given relation.

        If feature occures at more than one position in relation, all its occurences are removed.
        If given relation becomes empty with members deletion, the whole relation is removed.

        @param relId identifier of relation
        @param featId identifier of feature to remove
        @param featType type of feature to remove
        """

        c=self.getConnection().cursor()
        osmType=self.convertToOsmType(featType)

        # find all occurences of feature in relation (get list of position identifiers)
        c.execute("select pos_id from relation_member where relation_id=:relId and member_id=:memId and member_type=:memType and u=1"
                ,{"relId":str(relId),"memId":str(featId),"memType":osmType})

        for rec in c:
            self.__removeRelationMember(relId,rec[0])

        c.execute("select count(pos_id) from relation_member where relation_id=:relId and u=1"
                ,{"relId":str(relId)})

        relMembersCnt=0
        for rec in c:
            relMembersCnt=rec[0]

        if relMembersCnt==0:
            self.removeRelation(relId)

        c.close()


    def __removeRelationMember(self,relId,posId):
        """Function removes exactly one member given by its position in line.

        @param relId identifier of relation to remove member from
        @param posId member position
        """

        c=self.getConnection().cursor()
        c.execute("delete from relation_member where relation_id=:relId and pos_id=:posId and u=1"
                ,{"relId":str(relId),"posId":str(posId)})

        c.execute("update relation_member set pos_id=pos_id-1 where relation_id=:relId and pos_id>:posId and u=1"
                ,{"relId":str(relId),"posId":str(posId)})
        c.close()


    def movePoint(self,feat,deltaX,deltaY,snapFeat=None,snapFeatType=None):
        """Function moves an existing point.
        It performs commit.

        @param feat QgsFeature object of feature to move
        @param deltaX how far to move on X axis
        @param deltaY how far to move on Y axis
        @param snapFeat QgsFeature object of feature to where snapping is performed
        @param snapFeatType type of feature to where snapping is performed
        @return list of all features affected with this operation
        """

        affected=set()
        # count new coordinates
        point=feat.geometry().asPoint()
        newLat=point.y()+deltaY
        newLon=point.x()+deltaX
        targetPoint=QgsPoint(newLon,newLat)
        d=self.getConnection().cursor()

        if not snapFeat:
            # change node record; new coordinates
            d.execute("update node set lat=:lat,lon=:lon where id=:nodeId and u=1"
                    ,{"lat":str(newLat),"lon":str(newLon),"nodeId":str(feat.id())})
            self.changePointStatus(feat.id(),"N","U")

            # if point belongs to line/polygon, geometry of them must be deleted (osm provider will make new one later)
            d.execute("select wm.way_id,w.closed from way_member wm,way w where wm.node_id=:nodeId and wm.way_id=w.id and w.u=1 and wm.u=1"
                    ,{"nodeId":str(feat.id())})

            ways=[]
            for rec in d:
                t='Line'
                if rec[1]==1:
                    t='Polygon'
                ways.append((rec[0],t))

            for (wayId,t) in ways:

                d.execute("update way set wkb=:wkb where id=:wayId and u=1"
                        ,{"wkb":sqlite3.Binary(""),"wayId":str(wayId)})
                if t=='Line':
                    self.changeLineStatus(wayId,"N","U")
                else:
                    self.changePolygonStatus(wayId,"N","U")
                affected.add((wayId,t))

            d.close()
            # well, finishing..
            self.commit()

            affected.add((feat.id(),'Point'))
            return affected

        # well, snapping was done
        if snapFeatType=='Point':
            # merging two points
            aff=self.mergeTwoPoints(snapFeat.id(),feat.id())
            affected.update(aff)
            d.close()
            self.commit()
            return affected

        # well, we snapped to 'Line' or 'Polygon', actions are same in both cases
        d.execute("update node set lat=:lat,lon=:lon,usage=usage+1 where id=:nodeId and u=1"
                ,{"lat":str(newLat),"lon":str(newLon),"nodeId":str(feat.id())})
        self.changePointStatus(feat.id(),"N","U")
        affected.add((feat.id(),'Point'))

        # finding out exact position index of closest vertex in line (or polygon) geometry
        (a,b,vertexIx)=snapFeat.geometry().closestSegmentWithContext(targetPoint)
        newMemberIx=vertexIx+1

        # we need to shift indexes of all vertexes that will be after new vertex
        d.execute("select pos_id from way_member where way_id=:wayId and pos_id>:posId and u=1 order by pos_id desc"
                ,{"wayId":str(snapFeat.id()),"posId":str(vertexIx)})

        # for all such vertexes
        for rec in d:
            posId=rec[0]    # original position index

            e=self.getConnection().cursor()
            e.execute("update way_member set pos_id=:newPosId where way_id=:wayId and pos_id=:posId and u=1"
                    ,{"wayId":str(snapFeat.id()),"posId":str(posId),"newPosId":str(posId+1)})
            e.close()
            affected.add((snapFeat.id(),snapFeatType))

        # putting new node into set of lines/polygons members
        d.execute("insert into way_member (way_id,node_id,pos_id) values (:wayId,:nodeId,:posId)"
                ,{"wayId":str(snapFeat.id()),"nodeId":str(feat.id()),"posId":str(newMemberIx)})


        # if point belongs to line/polygon, geometry of them must be deleted (osm provider will make new one later)
        d.execute("select wm.way_id,w.closed from way_member wm,way w where wm.node_id=:nodeId and wm.way_id=w.id and w.u=1 and wm.u=1"
                ,{"nodeId":str(feat.id())})

        ways=[]
        for rec in d:
            t='Line'
            if rec[1]==1:
                t='Polygon'
            ways.append((rec[0],t))

        for (wayId,t) in ways:
            d.execute("update way set wkb=:wkb where id=:wayId and u=1"
                    ,{"wkb":sqlite3.Binary(""),"wayId":str(wayId)})
            if t=='Line':
                self.changeLineStatus(wayId,"N","U")
            else:
                self.changePolygonStatus(wayId,"N","U")
            affected.add((wayId,t))

        # ending transaction
        d.close()
        self.commit()

        return affected


    def moveLine(self,feat,deltaX,deltaY,snapFeat=None,snapFeatType=None,snapIndex=-1):
        """Function moves an existing line.
        It performs commit.

        @param feat QgsFeature object of feature to move
        @param deltaX how far to move on X axis
        @param deltaY how far to move on Y axis
        @param snapFeat QgsFeature object of feature to where snapping is performed
        @param snapFeatType type of feature to where snapping is performed
        @param snapIndex vertex index of snapFeat to which snapping was done
        @return list of all features affected with this operation
        """

        affected=set()
        if snapFeat and snapFeatType<>'Point':
            return affected

        # we are moving line with its context; now if snapFeat is not None, exactly one of lines' members
        # has to be merged with another existing node
        c=self.getConnection().cursor()
        c.execute("select n.id,n.lat,n.lon,wm.pos_id from node n, way_member wm where wm.node_id=n.id and wm.way_id=:wayId and n.u=1 and wm.u=1"
                ,{"wayId":str(feat.id())})

        # going through all line members
        for rec in c:
            nodeId=rec[0]
            # count new coordinates
            newLat=rec[1]+deltaY
            newLon=rec[2]+deltaX
            posId=rec[3]

            if snapFeat and posId==snapIndex+1:
                # merging two points => snapping two features of non-point type together
                aff=self.mergeTwoPoints(nodeId,snapFeat.id())
                affected.update(aff)

            # and changing their coordinates
            d=self.getConnection().cursor()
            d.execute("update node set lat=:lat,lon=:lon where id=:nodeId and u=1"
                     ,{"lat":str(newLat),"lon":str(newLon),"nodeId":str(nodeId)})
            self.changePointStatus(nodeId,"N","U")
            affected.add((nodeId,'Point'))

            # delete all geometries that contains node being moved
            d.execute("select wm.way_id,w.closed from way_member wm,way w where wm.node_id=:nodeId and wm.way_id=w.id and w.u=1 and wm.u=1"
                    ,{"nodeId":str(nodeId)})

            ways=[]
            for rec in d:
                t='Line'
                if rec[1]==1:
                    t='Polygon'
                ways.append((rec[0],t))

            for (wayId,t) in ways:
                d.execute("update way set wkb=:wkb where id=:wayId and u=1"
                        ,{"wkb":sqlite3.Binary(""),"wayId":str(wayId)})
                if t=='Line':
                    self.changeLineStatus(wayId,"N","U")
                else:
                    self.changePolygonStatus(wayId,"N","U")
                affected.add((wayId,t))
            d.close()

        # finally delete geometry of line (osm provider will make new one later)
        c.execute("update way set wkb=:wkb where id=:wayId and u=1"
                 ,{"wkb":sqlite3.Binary(""),"wayId":str(feat.id())})
        self.changeLineStatus(feat.id(),"N","U")

        # finish it
        c.close()
        self.commit()

        affected.add((feat.id(),'Line'))
        return affected


    def movePolygon(self,feat,deltaX,deltaY,snapFeat=None,snapFeatType=None,snapIndex=-1):
        """Function moves an existing polygon.
        It performs commit.

        @param feat QgsFeature object of feature to move
        @param deltaX how far to move on X axis
        @param deltaY how far to move on Y axis
        @param snapFeat QgsFeature object of feature to where snapping is performed
        @param snapFeatType type of feature to where snapping is performed
        @param snapIndex vertex index of snapFeat to which snapping was done
        @return list of all features affected with this operation
        """

        affected=set()
        if snapFeat and snapFeatType<>'Point':
            return affected

        # we are moving polygon with its context; now if snapFeat is not None, exactly one of polygons' members
        # has to be merged with another existing node
        c=self.getConnection().cursor()
        c.execute("select n.id,n.lat,n.lon,wm.pos_id from node n, way_member wm where wm.node_id=n.id and wm.way_id=:wayId and n.u=1 and wm.u=1"
                ,{"wayId":str(feat.id())})

        # going through all polygon members
        for rec in c:
            nodeId=rec[0]
            # count new coordinates
            newLat=rec[1]+deltaY
            newLon=rec[2]+deltaX
            posId=rec[3]

            if snapFeat and posId==snapIndex+1:
                # merging two points => snapping two features of non-point type together
                aff=self.mergeTwoPoints(nodeId,snapFeat.id())
                affected.update(aff)

            # and changing their coordinates..
            d=self.getConnection().cursor()
            d.execute("update node set lat=:lat,lon=:lon where id=:nodeId and u=1"
                    ,{"lat":str(newLat),"lon":str(newLon),"nodeId":str(nodeId)})
            self.changePointStatus(nodeId,"N","U")
            affected.add((nodeId,'Point'))

            # delete all geometries that contains node being moved
            d.execute("select wm.way_id,w.closed from way_member wm,way w where wm.node_id=:nodeId and wm.way_id=w.id and w.u=1 and wm.u=1"
                    ,{"nodeId":str(nodeId)})

            ways=[]
            for rec in d:
                t='Line'
                if rec[1]==1:
                    t='Polygon'
                ways.append((rec[0],t))

            for (wayId,t) in ways:
                d.execute("update way set wkb=:wkb where id=:wayId and u=1"
                        ,{"wkb":sqlite3.Binary(""),"wayId":str(wayId)})
                if t=='Line':
                    self.changeLineStatus(wayId,"N","U")
                else:
                    self.changePolygonStatus(wayId,"N","U")
                affected.add((wayId,t))
            d.close()

        # finally delete geometry of polygon (osm provider will make new one later)
        c.execute("update way set wkb=:wkb where id=:wayId and u=1"
                 ,{"wkb":sqlite3.Binary(""),"wayId":str(feat.id())})
        self.changeLineStatus(feat.id(),"N","U")

        # finish it
        c.close()
        self.commit()

        affected.add((feat.id(),'Polygon'))
        return affected


    def mergeTwoPoints(self,firstId,secondId):
        """Function merges two existing points.

        Second point (second parameter of function) will be removed.
        First point will get all tags/properties of second point. If both points has tag with the same key,
        new tag is created for the first point with key "oldkey_1" and value of second point's tag.

        All features and relations that contain the second point will contain the first point instead.

        @param firstId id of first node to merge
        @param firstId id of second node to merge
        @return list of all features affected with this operation
        """

        affected=set()

        # delete all geometries that contain secondId-node
        d=self.getConnection().cursor()
        d.execute("select wm.way_id, w.closed, w.membercnt, \
                  (select node_id from way_member where way_id=w.id and pos_id=1) first_id, \
                  (select node_id from way_member where way_id=w.id and pos_id=w.membercnt) last_id \
                   from way_member wm,way w where wm.node_id=:nodeId and wm.way_id=w.id and w.u=1 and wm.u=1"
                ,{"nodeId":str(secondId)})
        ways=[]
        for rec in d:
            t='Line'
            if rec[1]==1:
                t='Polygon'
            ways.append((rec[0],t,rec[2],rec[3],rec[4]))

        for (wayId,t,memcnt,fid,lid) in ways:

            if t=='Line':
                # check if the line is changing into polygon with this mergePoints action
                if fid==secondId:
                    fid=firstId
                if lid==secondId:
                    lid=firstId

                if fid==lid:
                    d.execute("update way set wkb=:wkb,closed=1 where id=:wayId and u=1"
                            ,{"wkb":sqlite3.Binary(""),"wayId":str(wayId)})
                    self.changePolygonStatus(wayId,"N","U")
                    d.execute("delete from way_member where way_id=:wayId and node_id=:lastId and pos_id=:posId and u=1"
                            ,{"wayId":str(wayId),"lastId":str(lid),"posId":str(memcnt)})
                    affected.add((wayId,'Polygon'))
                else:
                    d.execute("update way set wkb=:wkb where id=:wayId and u=1"
                            ,{"wkb":sqlite3.Binary(""),"wayId":str(wayId)})
                    self.changeLineStatus(wayId,"N","U")
            else:
                d.execute("update way set wkb=:wkb where id=:wayId and u=1"
                        ,{"wkb":sqlite3.Binary(""),"wayId":str(wayId)})
                self.changePolygonStatus(wayId,"N","U")

            affected.add((wayId,t))

        # finding out new "usage" column value for firstId-node
        d.execute("select count(distinct way_id) from way_member where node_id in (:firstId,:secondId) and u=1"
                ,{"firstId":str(firstId),"secondId":str(secondId)})
        row=d.fetchone()
        usage=row[0]

        d.execute("update way_member set node_id=:firstId where node_id=:secondId and u=1"
                ,{"firstId":str(firstId),"secondId":str(secondId)})

        d.execute("update tag set object_id=:firstId, key=key||'_1' where object_id=:secondId and object_type='node' and u=1"
                ,{"firstId":str(firstId),"secondId":str(secondId)})

        d.execute("update node set status='R' where id=:secondId and u=1"
                ,{"secondId":str(secondId)})

        d.execute("update node set usage=:usage where id=:firstId and u=1"
                ,{"usage":str(usage),"firstId":str(firstId)})

        d.execute("update relation_member set member_id=:firstId where member_id=:secondId and member_type='node' and u=1"
                ,{"firstId":str(firstId),"secondId":str(secondId)})
        d.close()

        affected.add((firstId,'Point'))
        affected.add((secondId,'Point'))
        return affected


    def getFeatureOwner(self,featId,featType):
        """Gets login of user who created/uploaded given feature.
        Feature is given by its identifier and type.

        @param featId id of feature
        @param featType type of feature
        @return login of user who creates/lately-edits this feature
        """

        featOwner=None
        c=self.getConnection().cursor()
        c.execute("select user from node where id=:featId and status<>'R' and u=1 union select user from way where id=:featId and status<>'R' and u=1"
                ,{"featId":str(featId)})

        for rec in c:
            featOwner=rec[0]

        c.close()
        return featOwner


    def getFeatureCreated(self,featId,featType):
        """Gets timestamp of when given feature was created/uploaded.
        Feature is given by its identifier and type.

        @param featId id of feature
        @param featType type of feature
        @return timestamp of when this feature was created/uploaded.
        """

        featCreated=None
        c=self.getConnection().cursor()
        c.execute("select timestamp from node where id=:featId and status<>'R' and u=1 \
                   union select timestamp from way where id=:featId and status<>'R' and u=1"
                ,{"featId":str(featId)})

        row=c.fetchone()
        if row<>None:
            featCreated=row[0]

        c.close()
        return featCreated


    def getFeatureGeometry(self,featId,featType):
        """Function constructs geometry of given feature.
        Feature is given by its identifier and type.

        @param featId id of feature
        @param featType type of feature
        @return geometry of given feature
        """

        featGeom=None
        if featType=='Point':

            c=self.getConnection().cursor()
            c.execute("select lat,lon from node where status<>'R' and id=:featId and u=1"
                    ,{"featId":str(featId)})

            for rec in c:
                featGeom=QgsGeometry.fromPoint(QgsPoint(rec[1],rec[0]))    # QgsPoint(lon,lat)

            c.close()

        # it's not a point
        elif featType in ('Line','Polygon'):

            c=self.getConnection().cursor()
            c.execute("select w.wkb FROM way w WHERE w.status<>'R' and id=:featId and u=1"
                    ,{"featId":str(featId)})

            for rec in c:
                featWKB=str(rec[0])
                theGeom = QgsGeometry()
                theGeom.fromWkb(featWKB)
                featGeom=theGeom
                break

            c.close()

        # well, finish now
        return featGeom


    def getFeatureTags(self,featId,featType):
        """Gets all tags of given feature.
        Feature is given by its identifier and type.
        The tags are returned no matter what's feature status: 'N','A','U','R'.

        @param featId id of feature
        @param featType type of feature
        @return list of pairs (tagKey,tagValue)
        """

        tags=[]
        osmType=self.convertToOsmType(featType)
        c=self.getConnection().cursor()

        c.execute("select key, val from tag where object_id=:objId and object_type=:objType and u=1"
                ,{"objId":str(featId),"objType":str(osmType)})

        for tagRec in c:
            tags.append((tagRec[0],tagRec[1]))

        c.close()
        return tags


    def getTagValue(self,featId,featType,tagKey):
        """Gets tag value of given feature.
        Feature is given by its identifier and type.
        Tag is returned no matter what's feature status: 'N','A','U','R'.

        @param featId identifier of feature
        @param featType type of feature
        @param tagKey key of tag to which we search the value
        @return list of pairs (tagKey,tagValue)
        """

        val=""
        osmType=self.convertToOsmType(featType)
        c=self.getConnection().cursor()

        c.execute("select val from tag where object_id=:objId and object_type=:objType and key=:tagKey and u=1"
                ,{"objId":str(featId),"objType":osmType,"tagKey":tagKey})

        for rec in c:
            val=rec[0]
            c.close()
            return val

        c.close()
        return val


    def setTagValue(self,featId,featType,tagKey,tagValue):
        """Function sets value of given feature and given key.
        Feature is given by its identifier and type.
        Tag is updated no matter what's feature status: 'N','A','U','R'.

        @param featId identifier of feature
        @param featType type of feature
        @param tagKey key of tag
        """

        val=""
        osmType=self.convertToOsmType(featType)
        c=self.getConnection().cursor()

        c.execute("update tag set val=:tagVal where object_id=:objId and object_type=:objType and key=:tagKey and u=1"
                ,{"tagVal":tagValue,"objId":str(featId),"objType":osmType,"tagKey":tagKey})

        for rec in c:
            val=rec[0]
            c.close()
            return val

        c.close()
        self.commit()
        return val


    def getFeatureRelations(self,featId,featType):
        """Gets all relations connected to given feature.
        Feature is given by its identifier and type.

        @param featId id of feature
        @param featType type of feature
        @return list of relation identifiers.
        """

        rels=[]
        c=self.getConnection().cursor()

        c.execute("select distinct relation_id from relation_member where member_id=:objId and u=1"
                ,{"objId":str(featId)})
        for record in c:
            rels.append(record[0])

        c.close()
        return rels


    def changePointStatus(self,pointId,oldStatus,newStatus):
        """Function changes status of given point.
        Allowed statuses and their meanings: 'U'=Updated,'R'=Removed,'N'=Normal,'A'=Added.
        If oldStatus is not specified, status is changed to newStatus no matter what was its previous value.

        @param pointId id of feature
        @param oldStatus old status that should be change
        @param newStatus new feature status
        """

        c=self.getConnection().cursor()

        if not oldStatus:
            c.execute("update node set status=:newStatus where id=:pointId and u=1"
                    ,{"newStatus":newStatus,"pointId":str(pointId)})
        else:
            c.execute("update node set status=:newStatus where id=:pointId and status=:oldStatus and u=1"
                    ,{"newStatus":newStatus,"pointId":str(pointId),"oldStatus":oldStatus})
        c.close()
        self.commit()


    def changeLineStatus(self,lineId,oldStatus,newStatus):
        """Function changes status of given line.
        Allowed statuses and their meanings: 'U'=Updated,'R'=Removed,'N'=Normal,'A'=Added.
        If oldStatus is not specified, status is changed to newStatus no matter what was its previous value.

        @param lineId id of feature
        @param oldStatus old status that should be change
        @param newStatus new feature status
        """

        c=self.getConnection().cursor()

        if not oldStatus:
            c.execute("update way set status=:newStatus where id=:lineId and u=1"
                    ,{"newStatus":newStatus,"lineId":str(lineId)})
        else:
            c.execute("update way set status=:newStatus where id=:lineId and status=:oldStatus and u=1"
                    ,{"newStatus":newStatus,"lineId":str(lineId),"oldStatus":oldStatus})
        c.close()
        self.commit()


    def changePolygonStatus(self,polygonId,oldStatus,newStatus):
        """Function changes status of given polygon.
        Allowed statuses and their meanings: 'U'=Updated,'R'=Removed,'N'=Normal,'A'=Added.
        If oldStatus is not specified, status is changed to newStatus no matter what was its previous value.

        @param polygonId id of feature
        @param oldStatus old status that should be change
        @param newStatus new feature status
        """

        c=self.getConnection().cursor()

        if not oldStatus:
            c.execute("update way set status=:newStatus where id=:polygonId and u=1"
                    ,{"newStatus":newStatus,"polygonId":str(polygonId)})
        else:
            c.execute("update way set status=:newStatus where id=:polygonId and status=:oldStatus and u=1"
                    ,{"newStatus":newStatus,"polygonId":str(polygonId),"oldStatus":oldStatus})
        c.close()
        self.commit()


    def changeWayStatus(self,wayId,oldStatus,newStatus):
        """Function changes status of given OSM way
        no matter if exact feature type is 'Line' or 'Polygon'.
        Allowed statuses and their meanings: 'U'=Updated,'R'=Removed,'N'=Normal,'A'=Added.
        If oldStatus is not specified, status is changed to newStatus no matter what was its previous value.

        @param wayId id of feature
        @param oldStatus old status that should be change
        @param newStatus new feature status
        """

        c=self.getConnection().cursor()

        if not oldStatus:
            c.execute("update way set status=:newStatus where id=:wayId and u=1"
                    ,{"newStatus":newStatus,"wayId":str(wayId)})
        else:
            c.execute("update way set status=:newStatus where id=:wayId and status=:oldStatus and u=1"
                    ,{"newStatus":newStatus,"wayId":str(wayId),"oldStatus":oldStatus})
        c.close()
        self.commit()


    def changeRelationStatus(self,relId,oldStatus,newStatus):
        """Function changes status of given relation.
        Allowed statuses and their meanings: 'U'=Updated,'R'=Removed,'N'=Normal,'A'=Added
        If oldStatus is not specified, status is changed to newStatus no matter what was its previous value.

        @param relId id of feature
        @param oldStatus old status that should be change
        @param newStatus new feature status
        """

        c=self.getConnection().cursor()

        if not oldStatus:
            c.execute("update relation set status=:newStatus where id=:relId and u=1"
                    ,{"newStatus":newStatus,"relId":str(relId)})
        else:
            c.execute("update relation set status=:newStatus where id=:relId and status=:oldStatus and u=1"
                    ,{"newStatus":newStatus,"relId":str(relId),"oldStatus":oldStatus})
        c.close()
        self.commit()


    def changeRelationType(self,relId,newType):
        """Function changes type of relation.

        If relation status is 'N' (Normal), it is automatically changed to 'U'
        (Updated) to keep data consistent.

        @param relId identifier of relation
        @param newType name of new relation type
        """

        c=self.getConnection().cursor()
        c.execute("update relation set type=:newType where id=:relId and u=1"
                ,{"newType":newType,"relId":str(relId)})
        c.close()

        # update relation record in database; mark relation as updated
        self.changeRelationStatus(relId,'N','U')

        # update tag with key "type"
        self.changeTagValue(relId,'Relation','type',newType)
        self.commit()


    def changeTagValue(self,featId,featType,key,value):
        """Function changes value for given feature and tag key.
        Feature is given by its id and type.

        @param featId id of feature
        @param featType type of feature
        @param key unique key of tag
        @param value new tag value of feature
        """

        osmType=self.convertToOsmType(featType)
        c=self.getConnection().cursor()
        c.execute("update tag set val=:val where object_id=:objId and object_type=:objType and key=:key and u=1"
                 ,{"val":value,"objId":str(featId),"objType":osmType,"key":key})
        c.close()
        self.commit()


    def isTagDefined(self,featId,featType,key):
        """Finds out if tag with given key is defined for given feature.

        @param featId id of feature
        @param featType type of feature
        @return True if given tag already exists for given feature; False otherwise
        """

        tagEx=False
        osmType=self.convertToOsmType(featType)
        c=self.getConnection().cursor()
        c.execute("select 1 from tag where object_id=:objId and object_type=:objType and key=:key and u=1"
                ,{"objId":str(featId),"objType":osmType,"key":key})

        for rec in c:
            tagEx=True

        c.close()
        return tagEx


    def updateVersionId(self,featId,featOSMType,newVerId):
        """Function updates version identifier of given feature.
        It performs commit.

        @param featId identifier of feature
        @param featOSMType OSM type of feature (one of 'node','way','relation')
        @param newVerId new version id
        """

        if not newVerId:
            return

        c=self.getConnection().cursor()
        c.execute("update version set version_id=:verId where object_id=:featId and object_type=:featType"
                ,{"verId":str(newVerId),"featId":str(featId),"featType":featOSMType})
        c.close()
        self.commit()


    def updateUser(self,featId,featOSMType,user):
        """Function updates user (owner) of given feature.
        It performs commit.

        @param featId identifier of feature
        @param featOSMType OSM type of feature (one of 'node','way','relation')
        @param user new owner of feature
        """

        c=self.getConnection().cursor()

        if featOSMType=='node':
            c.execute("update node set user=:user where id=:featId and u=1"
                    ,{"user":user.toAscii().data(),"featId":str(featId)})

        elif featOSMType=='way':
            c.execute("update way set user=:user where id=:featId and u=1"
                    ,{"user":user.toAscii().data(),"featId":str(featId)})

        elif featOSMType=='relation':
            c.execute("update relation set user=:user where id=:featId and u=1"
                    ,{"user":user.toAscii().data(),"featId":str(featId)})

        c.close()
        self.commit()


    def convertToOsmType(self,featType):
        """Function converts feature type ('Point','Line','Polygon','Relation')
        to relevant osm type ('way','node','relation').

        @param featType type of feature
        @return OSM type that is corresponding to featType
        """

        osmType=""
        if featType in ('Point','point'):
            osmType="node"
        elif featType in ('Polygon','Line','polygon','line'):
            osmType="way"
        elif featType in ('Relation','relation'):
            osmType="relation"

        return osmType


    def insertTag(self,featId,featType,key,value, doCommit=True):
        """Function inserts new tag to given feature.

        @param featId id of feature
        @param featType type of feature
        @param key key of new feature tag
        @param value value of new feature tag
        @param doCommit if True then commit is performed after tag insertion
        """

        osmType=self.convertToOsmType(featType)
        c=self.getConnection().cursor()
        c.execute("insert into tag (object_id, object_type, key, val) values (:objId, :objType, :key, :val)"
                 ,{"objId":str(featId),"objType":osmType,"key":key,"val":value})
        c.close()
        if doCommit:
          self.commit()


    def insertTags(self,featId,featType,tagRecords):
        """Function inserts new tags to given feature.
        It doesn't verify that tag keys are unique.
        Function performs commit.

        @param featId id of feature
        @param featType type of feature
        @param tagRecords list of new features' tags
        """

        for tagRecord in tagRecords:
            key=tagRecord[0]
            val=tagRecord[1]
            self.insertTag(featId,featType,key,val,False)

        self.commit()


    def removeTag(self,featId,featType,key):
        """Function removes tag of given feature. Tag is given by its key.
        It performs commit.

        @param featId id of feature
        @param featType type of feature
        @param key unique key of tag to remove
        """

        osmType=self.convertToOsmType(featType)
        c=self.getConnection().cursor()
        c.execute("delete from tag where object_id=:objId and object_type=:objType and key=:key and u=1"
                ,{"objId":str(featId),"objType":osmType,"key":key})
        c.close()
        self.commit()


    def removeFeaturesTags(self,featId,featType):
        """Function removes all features' tags.
        It performs commit.

        @param featId id of feature
        @param featType type of feature
        """

        osmType=self.convertToOsmType(featType)
        c=self.getConnection().cursor()
        c.execute("delete from tag where object_id=:objId and object_type=:objType and u=1"
                ,{"objId":str(featId),"objType":osmType})
        c.close()
        self.commit()


    def getLinePolygonMembers(self,featId):
        """Function returns all lines'/polygons' members.

        @param featId id of feature
        @return list of all lines'/polygons' members
        """

        mems=[]
        c=self.getConnection().cursor()

        c.execute("select n.id,n.lat,n.lon from node n,way_member wm where n.u=1 and wm.u=1 and wm.way_id=:featId and wm.node_id=n.id and n.status<>'R'"
                ,{"featId":str(featId)})

        for rec in c:
            mems.append((rec[0],rec[1],rec[2]))

        c.close()
        return mems


    def getRelationMembers(self,relId):
        """Function returns all members of given relation.
        It doesn't check if relation has non-removed (<>'R') status.

        @param relId identifier of relation
        @return list of all relations' members
        """

        c=self.getConnection().cursor()

        c.execute("select member_id, member_type, role from relation_member where relation_id=:relId and u=1"
                ,{"relId":str(relId)})

        mems=[]
        for row in c:
            mems.append((row[0],row[1],row[2]))
        c.close()

        out=[]
        for mem in mems:

            featId=mem[0]
            osmType=mem[1]
            featRole=mem[2]

            # finding out if feature is polygon
            isPol=self.isPolygon(featId)

            if osmType=="way" and isPol:
                featType="Polygon"
            elif osmType=="way":
                featType="Line"
            elif osmType=="node":
                featType="Point"
            elif osmType=="relation":
                featType = "Relation"

            out.append((featId,featType,featRole))
        return out


    def getNodeParents(self,node):
        """Function gets all features (lines,polygons) where given node is part of them.

        It returns list where each item consists of three values: (parentGeometry,memberIndex,isPolygonFlag).
        <parentGeometry> is QgsGeometry of one node's parent. Position of node in this geometry is determined by <memberIndex>.
        <isPolygonFlag> just says if return geometry is of type "polygon".

        @param node QgsFeature object of node
        @return list of parents
        """

        if not self.currentKey:    # no layer loaded
            return []

         # initialization
        parentFeats=[]
        memberIndexes=[]
        isPolygonFlags=[]

        mapPoint=node.geometry().asPoint()
        area=QgsRectangle(mapPoint.x()-0.00001,mapPoint.y()-0.00001,mapPoint.x()+0.00001,mapPoint.y()+0.00001)

        lay=self.lineLayers[self.currentKey]
        lay.select([],area,True,True)
        feat=QgsFeature()
        result=lay.nextFeature(feat)

        while result:
            parentFeats.append(feat)

            pos=[]
            c=self.getConnection().cursor()
            c.execute("select pos_id from way_member where way_id=:polId and node_id=:pointId and u=1 order by 1 asc"
                    ,{"polId":feat.id(),"pointId":node.id()})
            for rec in c:
                pos.append(rec[0])
            c.close()

            memberIndexes.append(pos)
            isPolygonFlags.append(False)

            feat=QgsFeature()
            result=lay.nextFeature(feat)

        lay=self.polygonLayers[self.currentKey]
        lay.select([],area,True,True)
        feat=QgsFeature()
        result=lay.nextFeature(feat)

        while result:
            parentFeats.append(feat)

            pos=[]
            c=self.getConnection().cursor()
            c.execute("select pos_id from way_member where way_id=:polId and node_id=:pointId and u=1 order by 1 asc"
                    ,{"polId":feat.id(),"pointId":node.id()})
            for rec in c:
                pos.append(rec[0])
            c.close()

            memberIndexes.append(pos)
            isPolygonFlags.append(True)

            feat=QgsFeature()
            result=lay.nextFeature(feat)

        return (parentFeats,memberIndexes,isPolygonFlags)


    def removeFromRelation(self,relId,memberId):
        """Function removes (all occurances of) given feature from existing relation.
        It performs commit.

        @param relId identifier of relation
        @param memberId identifier of feature to remove
        """

        c=self.getConnection().cursor()
        c.execute("delete from relation_member where relation_id=:relId and member_id=:memberId and u=1"
                ,{"relId":relId,"memberId":str(memberId)})
        c.execute("update relation set status='U' where id=:relId and u=1 and status='N'"
                ,{"relId":relId})
        c.close()
        self.commit()


    def __convertPseudoLineToPoint(self,lineId):
        """Function converts line with only 1 member ("pseudo-line") to point.
        It performs commit.

        @param lineId identifier of line
        """

        affected=set()
        # first change status of old line to 'R' ~ Removed
        self.changeLineStatus(lineId,None,"R")

        c=self.getConnection().cursor()
        c.execute("select node_id from way_member where u=1 and way_id=:lineId"
                ,{"lineId":str(lineId)})

        for rec in c:
            d=self.getConnection().cursor()
            d.execute("update node set usage=usage-1 where id=:pointId and u=1"
                    ,{"pointId":str(rec[0])})
            affected.add((rec[0],'Point'))
            d.close()

        c.execute("delete from way_member where way_id=:lineId and u=1"
                ,{"lineId":str(lineId)})

        # remove all lines' tags
        self.removeFeaturesTags(lineId,"Line")

        # and that's all...
        c.close()

        # don't forget to remove pseudo-line from existing relations
        # note: we won't put node into relations instead of the line!
        self.__removeFeatureFromAllRelations(lineId,"Line")
        self.commit()

        affected.add((lineId,'Line'))
        return affected


    def __convertPseudoPolygonToLine(self,polId):
        """Function converts polygon with only 2 members ("pseudo-polygon") to line.
        Identifier of resulting line is the same as id of original polygon.

        @param polId identifier of pseudo-polygon
        """

        affected=set()

        # it's quite simple!
        c=self.getConnection().cursor()
        c.execute("update way set closed=0,wkb=:wkb where id=:polId and u=1"
                ,{"polId":str(polId),"wkb":sqlite3.Binary("")})
        c.close()
        self.commit()

        affected.add((polId,'Polygon'))
        affected.add((polId,'Line'))    # there's not a mistake here!

        return affected


    def isPolygon(self,featId):
        """Function finds out if feature given by its identifier is polygon or not.

        @param featId identifier of feature
        @return True if feature is polygon; False otherwise
        """

        isPol=False
        c=self.getConnection().cursor()

        c.execute("select 1 from way where id=:featId and closed=1 and u=1"
                ,{"featId":str(featId)})

        for rec in c:
            isPol=True
        c.close()

        return isPol


    def recacheAffectedNow(self,affected):
        """Function calls recaching of all features that are given in parameter.
        Recaching is called only if it is provided by vector layer interface.

        @param affected list of features to be recached
        """

        if affected==None or len(affected)<1 or not self.currentKey:
            return

        settings=QSettings()
        cacheMode=settings.value("qgis/vectorLayerCacheMode",QVariant("heuristics")).toString()

        if cacheMode=="nothing":
            return

        if not hasattr(self.pointLayers[self.currentKey],'recacheFeature'):
            return

        for (fid,ftype) in affected:
            # cache is enabled; cache must refetch all affected features
            if ftype=='Point':
                self.pointLayers[self.currentKey].recacheFeature(fid)
            elif ftype=='Line':
                self.lineLayers[self.currentKey].recacheFeature(fid)
            elif ftype=='Polygon':
                self.polygonLayers[self.currentKey].recacheFeature(fid)


    def updateNodeId(self, nodePseudoId, newNodeId, user):
        """Function updates node identification in sqlite3 database.
        Used after uploading.

        @param nodePseudoId identifier which was used for temporary identification of the node in database
        @param newNodeId identifier that is for node valid in OSM server database
        @param user new user name
        """

        c=self.getConnection().cursor()
        now = datetime.datetime.now()
        nowfmt = now.strftime("%Y-%m-%dT%H:%M:%SZ")

        c.execute("update node set id=?, user=?, timestamp=?, status=? where id=?"
                 ,(newNodeId, user.toAscii().data(), nowfmt, 'N', nodePseudoId))
        c.execute("update tag set object_id=? where object_id=?",(newNodeId, nodePseudoId))
        c.execute("update way_member set node_id=? where node_id=?",(newNodeId, nodePseudoId))
        c.execute("insert into version (object_id,object_type,version_id) values (?,'node',?)",(newNodeId,1))

        c.close()
        self.commit()


    def updateWayId(self, pseudoId, newId, user):
        """Function updates way identification in sqlite3 database.
        Used after uploading.

        @param pseudoId identifier which was used for temporary identification of the way in database
        @param newId identifier that is for that way valid in OSM server database
        @param user new user name
        """

        c=self.getConnection().cursor()
        now=datetime.datetime.now()
        nowfmt=now.strftime("%Y-%m-%dT%H:%M:%SZ")

        c.execute("update way set id=?, user=?, timestamp=?, status=? where id=?"
                 ,(newId, user.toAscii().data(), nowfmt, 'N', pseudoId))
        c.execute("update tag set object_id=? where object_id=? and object_type='way'",(newId, pseudoId))
        c.execute("update way_member set way_id=? where way_id=?",(newId, pseudoId))
        c.execute("insert into version (object_id,object_type,version_id) values (?,'way',?)",(newId,1))

        c.close()
        self.commit()


    def updateRelationId(self, relPseudoId, newRelId, user):
        """Function updates relation identification in sqlite3 database.
        Used after uploading.

        @param relPseudoId identifier which was used for temporary identification of the relation in database
        @param newRelId identifier that is for relation valid in OSM server database
        @param user new user name
        """

        c=self.getConnection().cursor()
        now = datetime.datetime.now()
        nowfmt = now.strftime("%Y-%m-%dT%H:%M:%SZ")

        c.execute("update relation set id=?, user=?, timestamp=?, status=? where id=?"
                 ,(newRelId, user.toAscii().data(), nowfmt, 'N', relPseudoId))
        c.execute("update tag set object_id=? where object_id=? and object_type='relation'",(newRelId, relPseudoId))
        c.execute("update relation_member set relation_id=? where relation_id=?",(newRelId, relPseudoId))
        c.execute("insert into version (object_id,object_type,version_id) values (?,'relation',?)",(newRelId,1))

        c.close()
        self.commit()


    def removeUselessRecords(self):
        """Function removes all records with 'R' status.

        Uploader calls this function after upload process finishes.
        It performs commit.
        """

        c=self.getConnection().cursor()

        c.execute("delete from version where object_type='way' and exists( select 1 from way w where object_id=w.id and w.status='R' )")
        c.execute("delete from version where object_type='node' and exists( select 1 from node n where object_id=n.id and n.status='R' )")
        c.execute("delete from version where object_type='relation' and exists( select 1 from relation r where object_id=r.id and r.status='R' )")

        c.execute("delete from node where status='R'")
        c.execute("delete from way where status='R'")
        c.execute("delete from relation where status='R'")

        c.close()
        self.commit()


    def removePointRecord(self,featId):
        """Function removes database record of given point.

        This deletion is not just setting point status to 'R' (Removed) like removePoint() function does.
        This function removes point permanently and it should be called after upload operation only.

        Function performs commit.

        @param featId identifier of feature/point
        """

        c=self.getConnection().cursor()
        c.execute("delete from node where status='R' and id=:pointId and u=1",{"pointId":str(featId)})
        c.close()
        self.commit()


    def removeWayRecord(self,featId):
        """Function removes database record of given OSM way.

        This deletion is not just setting way status to 'R' (Removed) like removeLine()/removePolygon() function does.
        This function removes way permanently and it should be called after upload operation only.

        Function performs commit.

        @param featId identifier of feature (way)
        """

        c=self.getConnection().cursor()
        c.execute("delete from way where status='R' and id=:wayId and u=1",{"wayId":str(featId)})
        c.close()
        self.commit()


    def removeRelationRecord(self,featId):
        """Function removes database record of given relation.

        This deletion is not just setting relation status to 'R' (Removed) like removeRelation() function does.
        This function removes relation permanently and it should be called after upload operation only.

        Function performs commit.

        @param featId identifier of feature/relation
        """

        c=self.getConnection().cursor()
        c.execute("delete from relation where status='R' and id=:relId and u=1",{"relId":str(featId)})
        c.close()
        self.commit()


    def commit(self):
        """Performs commit on current database.
        """

        self.getConnection().commit()


    def rollback(self):
        """Performs rollback on current database.
        """

        self.getConnection().rollback()


    def getCurrentActionNumber(self):
        """Function finds out the highest identifier in editing history.

        @return the highest identifier in editing history
        """

        c=self.getConnection().cursor()
        c.execute("select max(change_id) from change_step")

        for rec in c:
            c.close()
            if not rec[0]:
                return 0
            return rec[0]


    def removeAllChangeSteps(self):
        """Function removes all records of editing history.
        It performs commit.
        """

        c=self.getConnection().cursor()
        c.execute("delete from change_step")
        c.close()
        self.commit()


    def removeChangeStepsBetween(self,fromId,toId):
        """Function removes all records of editing history that falls to given interval.
        It performs commit.

        @param fromId identifier of the row
        @param toId identifier of the row
        """

        c=self.getConnection().cursor()
        c.execute("delete from change_step where change_id>=:fromId and change_id<=:toId"
                ,{"fromId":str(fromId),"toId":str(toId)})
        c.close()
        self.commit()


    def getChangeSteps(self,startId,stopId):
        """Function returns all records of editing history that falls to given interval.

        @param startId identifier of the row
        @param stopId identifier of the row
        @return list of editing history records
        """

        c=self.getConnection().cursor()
        c.execute("select change_type,tab_name,row_id,col_name,old_value,new_value from change_step \
                   where change_id>:startId and change_id<=:stopId order by change_id desc"
                ,{"startId":str(startId),"stopId":str(stopId)})
        chSteps=[]
        for rec in c:
            chSteps.append(rec)

        c.close()
        return chSteps


    def setRowDeleted(self,tabName,rowId):
        """Function marks given row of given table as deleted.

        @param tabName name of table
        @param rowId identifier of the row
        """

        d=self.getConnection().cursor()
        d.execute("update "+tabName+" set u=0 where i=:rowId"
                ,{"rowId":rowId})
        d.close()


    def setRowNotDeleted(self,tabName,rowId):
        """Function marks given row of given table as not deleted.

        @param tabName name of table
        @param rowId identifier of the row
        """

        d=self.getConnection().cursor()
        d.execute("update "+tabName+" set u=1 where i=:rowId"
                ,{"rowId":rowId})
        d.close()


    def setRowColumnValue(self,tabName,colName,newValue,rowId):
        """Function sets new value in given table, row and column.
        It doesn't perform commit.

        @param tabName name of table
        @param colName name of column
        @param newValue new value to set
        @param rowId identifier of the row
        """

        d=self.getConnection().cursor()
        d.execute("update "+tabName+" set "+colName+"='"+newValue+"' where i=:rowId"
                ,{"rowId":str(rowId)})
        d.close()


