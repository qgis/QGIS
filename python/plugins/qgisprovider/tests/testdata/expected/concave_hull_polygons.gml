<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     gml:id="aFeatureCollection"
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation="http://ogr.maptools.org/ concave_hull_polygons.xsd"
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml/3.2">
  <gml:boundedBy><gml:Envelope srsName="urn:ogc:def:crs:EPSG::4326"><gml:lowerCorner>-1 0</gml:lowerCorner><gml:upperCorner>6 9</gml:upperCorner></gml:Envelope></gml:boundedBy>
                                                                                                                                                                             
  <ogr:featureMember>
    <ogr:concave_hull_polygons gml:id="concave_hull_polygons.0">
      <gml:boundedBy><gml:Envelope srsName="urn:ogc:def:crs:EPSG::4326"><gml:lowerCorner>1 2</gml:lowerCorner><gml:upperCorner>3 4</gml:upperCorner></gml:Envelope></gml:boundedBy>
      <ogr:geometryProperty><gml:Polygon srsName="urn:ogc:def:crs:EPSG::4326" gml:id="concave_hull_polygons.geom.0"><gml:exterior><gml:LinearRing><gml:posList>3 3 3 4 1 4 1 2 2 2 3 3</gml:posList></gml:LinearRing></gml:exterior></gml:Polygon></ogr:geometryProperty>
      <ogr:fid>multipolys.0</ogr:fid>
      <ogr:Bname>Test</ogr:Bname>
      <ogr:Bintval>1</ogr:Bintval>
      <ogr:Bfloatval>0.123</ogr:Bfloatval>
      <ogr:area>3.500000</ogr:area>
      <ogr:perimeter>7.414214</ogr:perimeter>
    </ogr:concave_hull_polygons>
  </ogr:featureMember>
  <ogr:featureMember>
    <ogr:concave_hull_polygons gml:id="concave_hull_polygons.1">
      <gml:boundedBy><gml:Envelope srsName="urn:ogc:def:crs:EPSG::4326"><gml:lowerCorner>-1 7</gml:lowerCorner><gml:upperCorner>6 9</gml:upperCorner></gml:Envelope></gml:boundedBy>
      <ogr:geometryProperty><gml:Polygon srsName="urn:ogc:def:crs:EPSG::4326" gml:id="concave_hull_polygons.geom.1"><gml:exterior><gml:LinearRing><gml:posList>3 8 -1 8 -1 7 3 7 4 7 5 7 6 7 6 9 5 9 3 8</gml:posList></gml:LinearRing></gml:exterior></gml:Polygon></ogr:geometryProperty>
      <ogr:fid>multipolys.1</ogr:fid>
      <ogr:Bname xsi:nil="true"/>
      <ogr:Bintval xsi:nil="true"/>
      <ogr:Bfloatval xsi:nil="true"/>
      <ogr:area>9.000000</ogr:area>
      <ogr:perimeter>17.236068</ogr:perimeter>
    </ogr:concave_hull_polygons>
  </ogr:featureMember>
  <ogr:featureMember>
    <ogr:concave_hull_polygons gml:id="concave_hull_polygons.2">
      <gml:boundedBy><gml:Envelope srsName="urn:ogc:def:crs:EPSG::4326"><gml:lowerCorner>0 0</gml:lowerCorner><gml:upperCorner>1 1</gml:upperCorner></gml:Envelope></gml:boundedBy>
      <ogr:geometryProperty><gml:MultiSurface srsName="urn:ogc:def:crs:EPSG::4326" gml:id="concave_hull_polygons.geom.2"><gml:surfaceMember><gml:Polygon gml:id="concave_hull_polygons.geom.2.0"><gml:exterior><gml:LinearRing><gml:posList>0 0 1 0 1 1 0 1 0 0</gml:posList></gml:LinearRing></gml:exterior></gml:Polygon></gml:surfaceMember></gml:MultiSurface></ogr:geometryProperty>
      <ogr:fid>multipolys.2</ogr:fid>
      <ogr:Bname>Test</ogr:Bname>
      <ogr:Bintval>2</ogr:Bintval>
      <ogr:Bfloatval>-0.123</ogr:Bfloatval>
      <ogr:area>1.000000</ogr:area>
      <ogr:perimeter>4.000000</ogr:perimeter>
    </ogr:concave_hull_polygons>
  </ogr:featureMember>
  <ogr:featureMember>
    <ogr:concave_hull_polygons gml:id="concave_hull_polygons.3">
      <ogr:fid>multipolys.3</ogr:fid>
      <ogr:Bname>Test</ogr:Bname>
      <ogr:Bintval>3</ogr:Bintval>
      <ogr:Bfloatval>0</ogr:Bfloatval>
      <ogr:area xsi:nil="true"/>
      <ogr:perimeter xsi:nil="true"/>
    </ogr:concave_hull_polygons>
  </ogr:featureMember>
</ogr:FeatureCollection>
