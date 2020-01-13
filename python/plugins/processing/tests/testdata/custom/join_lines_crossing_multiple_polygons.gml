<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation="http://ogr.maptools.org/ join_lines_crossing_multiple_polygons.xsd"
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml">
  <gml:boundedBy>
    <gml:Box>
      <gml:coord><gml:X>-2</gml:X><gml:Y>-3</gml:Y></gml:coord>
      <gml:coord><gml:X>10</gml:X><gml:Y>6</gml:Y></gml:coord>
    </gml:Box>
  </gml:boundedBy>

  <gml:featureMember>
    <ogr:join_lines_crossing_multiple_polygons fid="join_lines_crossing_multiple_polygons.0">
      <ogr:geometryProperty><gml:LineString srsName="EPSG:4326"><gml:coordinates>1,4 1,0 3,0</gml:coordinates></gml:LineString></ogr:geometryProperty>
      <ogr:name>line4</ogr:name>
      <ogr:val xsi:nil="true"/>
    </ogr:join_lines_crossing_multiple_polygons>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:join_lines_crossing_multiple_polygons fid="join_lines_crossing_multiple_polygons.1">
      <ogr:geometryProperty><gml:LineString srsName="EPSG:4326"><gml:coordinates>4,0 5,0</gml:coordinates></gml:LineString></ogr:geometryProperty>
      <ogr:name>line2</ogr:name>
      <ogr:val>3</ogr:val>
    </ogr:join_lines_crossing_multiple_polygons>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:join_lines_crossing_multiple_polygons fid="join_lines_crossing_multiple_polygons.2">
      <ogr:geometryProperty><gml:LineString srsName="EPSG:4326"><gml:coordinates>1,-3 2,-3</gml:coordinates></gml:LineString></ogr:geometryProperty>
      <ogr:name>line3</ogr:name>
      <ogr:val>4</ogr:val>
    </ogr:join_lines_crossing_multiple_polygons>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:join_lines_crossing_multiple_polygons fid="join_lines_crossing_multiple_polygons.3">
      <ogr:geometryProperty><gml:LineString srsName="EPSG:4326"><gml:coordinates>9,6 -1,5 -2,5 -2,0 10,1</gml:coordinates></gml:LineString></ogr:geometryProperty>
      <ogr:name>line1</ogr:name>
      <ogr:val>11</ogr:val>
    </ogr:join_lines_crossing_multiple_polygons>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:join_lines_crossing_multiple_polygons fid="join_lines_crossing_multiple_polygons.4">
      <ogr:name>line5</ogr:name>
      <ogr:val>1</ogr:val>
    </ogr:join_lines_crossing_multiple_polygons>
  </gml:featureMember>
</ogr:FeatureCollection>
