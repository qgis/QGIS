<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation="http://ogr.maptools.org/ text_to_float.xsd"
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml">
  <gml:boundedBy>
    <gml:Box>
      <gml:coord><gml:X>1</gml:X><gml:Y>1</gml:Y></gml:coord>
      <gml:coord><gml:X>5</gml:X><gml:Y>3</gml:Y></gml:coord>
    </gml:Box>
  </gml:boundedBy>
                                                                                                                                                                
  <gml:featureMember>
    <ogr:text_to_float fid="points.0">
      <ogr:geometryProperty><gml:Point srsName="EPSG:4326"><gml:coordinates>1,1</gml:coordinates></gml:Point></ogr:geometryProperty>
      <ogr:id>1</ogr:id>
      <ogr:id2>2</ogr:id2>
      <ogr:text_float>1</ogr:text_float>
    </ogr:text_to_float>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:text_to_float fid="points.1">
      <ogr:geometryProperty><gml:Point srsName="EPSG:4326"><gml:coordinates>3,3</gml:coordinates></gml:Point></ogr:geometryProperty>
      <ogr:id>2</ogr:id>
      <ogr:id2>1</ogr:id2>
      <ogr:text_float>1.1</ogr:text_float>
    </ogr:text_to_float>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:text_to_float fid="points.2">
      <ogr:geometryProperty><gml:Point srsName="EPSG:4326"><gml:coordinates>2,2</gml:coordinates></gml:Point></ogr:geometryProperty>
      <ogr:id>3</ogr:id>
      <ogr:id2>0</ogr:id2>
      <ogr:text_float>5%</ogr:text_float>
    </ogr:text_to_float>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:text_to_float fid="points.3">
      <ogr:geometryProperty><gml:Point srsName="EPSG:4326"><gml:coordinates>5,2</gml:coordinates></gml:Point></ogr:geometryProperty>
      <ogr:id>4</ogr:id>
      <ogr:id2>2</ogr:id2>
      <ogr:text_float>notfloat</ogr:text_float>
    </ogr:text_to_float>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:text_to_float fid="points.4">
      <ogr:geometryProperty><gml:Point srsName="EPSG:4326"><gml:coordinates>4,1</gml:coordinates></gml:Point></ogr:geometryProperty>
      <ogr:id>5</ogr:id>
      <ogr:id2>1</ogr:id2>
    </ogr:text_to_float>
  </gml:featureMember>
</ogr:FeatureCollection>
