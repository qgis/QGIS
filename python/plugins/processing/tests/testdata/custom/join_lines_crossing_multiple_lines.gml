<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation="http://ogr.maptools.org/ join_lines_crossing_multiple_lines.xsd"
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml">
  <gml:boundedBy>
    <gml:Box>
      <gml:coord><gml:X>-2</gml:X><gml:Y>-3</gml:Y></gml:coord>
      <gml:coord><gml:X>10</gml:X><gml:Y>8</gml:Y></gml:coord>
    </gml:Box>
  </gml:boundedBy>
                                                                                                                                                             
  <gml:featureMember>
    <ogr:join_lines_crossing_multiple_lines fid="join_lines_crossing_multiple_lines.0">
      <ogr:geometryProperty><gml:LineString srsName="EPSG:4326"><gml:coordinates>-2,7 -2,-3 3,-3</gml:coordinates></gml:LineString></ogr:geometryProperty>
      <ogr:name2>line2</ogr:name2>
      <ogr:val2>1</ogr:val2>
    </ogr:join_lines_crossing_multiple_lines>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:join_lines_crossing_multiple_lines fid="join_lines_crossing_multiple_lines.1">
      <ogr:geometryProperty><gml:LineString srsName="EPSG:4326"><gml:coordinates>5,3 10,3</gml:coordinates></gml:LineString></ogr:geometryProperty>
      <ogr:name2>line3</ogr:name2>
      <ogr:val2>3</ogr:val2>
    </ogr:join_lines_crossing_multiple_lines>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:join_lines_crossing_multiple_lines fid="join_lines_crossing_multiple_lines.2">
      <ogr:geometryProperty><gml:LineString srsName="EPSG:4326"><gml:coordinates>7,8 7,5</gml:coordinates></gml:LineString></ogr:geometryProperty>
      <ogr:name2>line4</ogr:name2>
      <ogr:val2>4</ogr:val2>
    </ogr:join_lines_crossing_multiple_lines>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:join_lines_crossing_multiple_lines fid="join_lines_crossing_multiple_lines.3">
      <ogr:geometryProperty><gml:LineString srsName="EPSG:4326"><gml:coordinates>0,0 6,0</gml:coordinates></gml:LineString></ogr:geometryProperty>
      <ogr:name2>line5</ogr:name2>
      <ogr:val2>5</ogr:val2>
    </ogr:join_lines_crossing_multiple_lines>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:join_lines_crossing_multiple_lines fid="join_lines_crossing_multiple_lines.4">
      <ogr:name2>line1</ogr:name2>
      <ogr:val2 xsi:nil="true"/>
    </ogr:join_lines_crossing_multiple_lines>
  </gml:featureMember>
</ogr:FeatureCollection>
