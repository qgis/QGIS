import os.path
from sextante.core.QGisLayers import QGisLayers

dataFolder = os.path.join(os.path.dirname(__file__), 'data')


def table():
    return os.path.join(dataFolder, "table.dbf")

def points():
    return os.path.join(dataFolder, "points.shp")

def points2():
    return os.path.join(dataFolder, "points2.shp")

def raster():
    return os.path.join(dataFolder, "raster.tif")

def lines():
    return os.path.join(dataFolder, "lines.shp")

def polygons():
    return os.path.join(dataFolder, "polygons.shp")

def polygons2():
    return os.path.join(dataFolder, "polygons2.shp")

def polygonsGeoJson():
    return os.path.join(dataFolder, "polygons.geojson")

def union():
    return os.path.join(dataFolder, "union.shp")

def loadTestData():
    QGisLayers.load(points(), "points");
    QGisLayers.load(points2(), "points2");
    QGisLayers.load(polygons(), "polygons");
    QGisLayers.load(polygons2(), "polygons2");
    QGisLayers.load(polygonsGeoJson(), "polygonsGeoJson");
    QGisLayers.load(lines(), "lines");
    QGisLayers.load(raster(), "raster");
    QGisLayers.load(table(), "table");
    QGisLayers.load(union(), "union");