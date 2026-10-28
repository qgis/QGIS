<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     gml:id="aFeatureCollection"
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation="http://ogr.maptools.org/ remove_multipolygon_parts_by_area.xsd"
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml/3.2">
  <gml:boundedBy><gml:Envelope srsName="urn:ogc:def:crs:EPSG::4326"><gml:lowerCorner>-1 7</gml:lowerCorner><gml:upperCorner>3 8</gml:upperCorner></gml:Envelope></gml:boundedBy>
                                                                                                                                                                             
  <ogr:featureMember>
    <ogr:remove_multipolygon_parts_by_area gml:id="remove_multipolygon_parts_by_area.0">
      <gml:boundedBy><gml:Envelope srsName="urn:ogc:def:crs:EPSG::4326"><gml:lowerCorner>-1 7</gml:lowerCorner><gml:upperCorner>3 8</gml:upperCorner></gml:Envelope></gml:boundedBy>
      <ogr:geometryProperty><gml:MultiSurface srsName="urn:ogc:def:crs:EPSG::4326" gml:id="remove_multipolygon_parts_by_area.geom.0"><gml:surfaceMember><gml:Polygon gml:id="remove_multipolygon_parts_by_area.geom.0.0"><gml:exterior><gml:LinearRing><gml:posList>-1 7 -1 8 3 8 3 7 -1 7</gml:posList></gml:LinearRing></gml:exterior></gml:Polygon></gml:surfaceMember></gml:MultiSurface></ogr:geometryProperty>
      <ogr:fid>multipolys.1</ogr:fid>
      <ogr:Bname xsi:nil="true"/>
      <ogr:Bintval xsi:nil="true"/>
      <ogr:Bfloatval xsi:nil="true"/>
    </ogr:remove_multipolygon_parts_by_area>
  </ogr:featureMember>
  <ogr:featureMember>
    <ogr:remove_multipolygon_parts_by_area gml:id="remove_multipolygon_parts_by_area.1">
      <ogr:fid>multipolys.3</ogr:fid>
      <ogr:Bname>Test</ogr:Bname>
      <ogr:Bintval>3</ogr:Bintval>
      <ogr:Bfloatval>0</ogr:Bfloatval>
    </ogr:remove_multipolygon_parts_by_area>
  </ogr:featureMember>
</ogr:FeatureCollection>
