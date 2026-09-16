<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation="http://ogr.maptools.org/ lines_within_distance.xsd"
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml">
  <gml:boundedBy>
    <gml:Box>
      <gml:coord><gml:X>-1</gml:X><gml:Y>-3</gml:Y></gml:coord>
      <gml:coord><gml:X>10</gml:X><gml:Y>3</gml:Y></gml:coord>
    </gml:Box>
  </gml:boundedBy>
                                                                                                                                                             
  <gml:featureMember>
    <ogr:lines_within_distance fid="lines.1">
      <ogr:geometryProperty><gml:LineString srsName="EPSG:4326"><gml:coordinates>-1,-1 1,-1</gml:coordinates></gml:LineString></ogr:geometryProperty>
    </ogr:lines_within_distance>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:lines_within_distance fid="lines.2">
      <ogr:geometryProperty><gml:LineString srsName="EPSG:4326"><gml:coordinates>2,0 2,2 3,2 3,3</gml:coordinates></gml:LineString></ogr:geometryProperty>
    </ogr:lines_within_distance>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:lines_within_distance fid="lines.3">
      <ogr:geometryProperty><gml:LineString srsName="EPSG:4326"><gml:coordinates>3,1 5,1</gml:coordinates></gml:LineString></ogr:geometryProperty>
    </ogr:lines_within_distance>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:lines_within_distance fid="lines.5">
      <ogr:geometryProperty><gml:LineString srsName="EPSG:4326"><gml:coordinates>6,-3 10,1</gml:coordinates></gml:LineString></ogr:geometryProperty>
    </ogr:lines_within_distance>
  </gml:featureMember>
</ogr:FeatureCollection>
