<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation="http://ogr.maptools.org/ remove_duplicates1.xsd"
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml">
  <gml:boundedBy>
    <gml:Box>
      <gml:coord><gml:X>-18</gml:X><gml:Y>-14</gml:Y></gml:coord>
      <gml:coord><gml:X>-16</gml:X><gml:Y>-12</gml:Y></gml:coord>
    </gml:Box>
  </gml:boundedBy>
                                                                                                                                                        
  <gml:featureMember>
    <ogr:remove_duplicates1 fid="duplicate_attributes.0">
      <ogr:geometryProperty><gml:Point srsName="EPSG:4326"><gml:coordinates>-18,-12</gml:coordinates></gml:Point></ogr:geometryProperty>
      <ogr:num_field1>1</ogr:num_field1>
      <ogr:num_field2 xsi:nil="true"/>
      <ogr:text_field>val1</ogr:text_field>
      <ogr:text_field2>val2</ogr:text_field2>
    </ogr:remove_duplicates1>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:remove_duplicates1 fid="duplicate_attributes.1">
      <ogr:geometryProperty><gml:Point srsName="EPSG:4326"><gml:coordinates>-17,-12</gml:coordinates></gml:Point></ogr:geometryProperty>
      <ogr:num_field1 xsi:nil="true"/>
      <ogr:num_field2>2</ogr:num_field2>
      <ogr:text_field>val1</ogr:text_field>
      <ogr:text_field2 xsi:nil="true"/>
    </ogr:remove_duplicates1>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:remove_duplicates1 fid="duplicate_attributes.2">
      <ogr:geometryProperty><gml:Point srsName="EPSG:4326"><gml:coordinates>-17,-13</gml:coordinates></gml:Point></ogr:geometryProperty>
      <ogr:num_field1>3</ogr:num_field1>
      <ogr:num_field2>1</ogr:num_field2>
      <ogr:text_field>val2</ogr:text_field>
      <ogr:text_field2>val1</ogr:text_field2>
    </ogr:remove_duplicates1>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:remove_duplicates1 fid="duplicate_attributes.3">
      <ogr:geometryProperty><gml:Point srsName="EPSG:4326"><gml:coordinates>-17,-14</gml:coordinates></gml:Point></ogr:geometryProperty>
      <ogr:num_field1>99</ogr:num_field1>
      <ogr:num_field2>2</ogr:num_field2>
      <ogr:text_field>val2</ogr:text_field>
      <ogr:text_field2>1</ogr:text_field2>
    </ogr:remove_duplicates1>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:remove_duplicates1 fid="duplicate_attributes.5">
      <ogr:geometryProperty><gml:Point srsName="EPSG:4326"><gml:coordinates>-16,-14</gml:coordinates></gml:Point></ogr:geometryProperty>
      <ogr:num_field1>7</ogr:num_field1>
      <ogr:num_field2>2</ogr:num_field2>
      <ogr:text_field>VAL1</ogr:text_field>
      <ogr:text_field2>VAL2</ogr:text_field2>
    </ogr:remove_duplicates1>
  </gml:featureMember>
</ogr:FeatureCollection>
