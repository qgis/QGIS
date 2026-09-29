<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation="http://ogr.maptools.org/ clip_lines_by_multipolygon.xsd"
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml">
  <gml:boundedBy>
    <gml:Box>
      <gml:coord><gml:X>2</gml:X><gml:Y>-1</gml:Y></gml:coord>
      <gml:coord><gml:X>8</gml:X><gml:Y>3</gml:Y></gml:coord>
    </gml:Box>
  </gml:boundedBy>

  <gml:featureMember>
    <ogr:lines fid="lines.0">
      <ogr:geometryProperty><gml:LineString srsName="EPSG:4326"><gml:coordinates>7,2 8,2</gml:coordinates></gml:LineString></ogr:geometryProperty>
    </ogr:lines>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:lines fid="lines.2">
      <ogr:geometryProperty><gml:MultiLineString srsName="EPSG:4326"><gml:lineStringMember><gml:LineString><gml:coordinates>3,2 3,3</gml:coordinates></gml:LineString></gml:lineStringMember><gml:lineStringMember><gml:LineString><gml:coordinates>2,1 2,2</gml:coordinates></gml:LineString></gml:lineStringMember><gml:lineStringMember><gml:LineString><gml:coordinates>2,2 3,2</gml:coordinates></gml:LineString></gml:lineStringMember></gml:MultiLineString></ogr:geometryProperty>
    </ogr:lines>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:lines fid="lines.3">
      <ogr:geometryProperty><gml:LineString srsName="EPSG:4326"><gml:coordinates>4,1 3,1</gml:coordinates></gml:LineString></ogr:geometryProperty>
    </ogr:lines>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:lines fid="lines.6">
    </ogr:lines>
  </gml:featureMember>
</ogr:FeatureCollection>
