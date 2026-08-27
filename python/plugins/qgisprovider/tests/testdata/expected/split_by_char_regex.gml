<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation="http://ogr.maptools.org/ split_by_char_regex.xsd"
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml">
  <gml:boundedBy>
    <gml:Box>
      <gml:coord><gml:X>1</gml:X><gml:Y>1</gml:Y></gml:coord>
      <gml:coord><gml:X>3</gml:X><gml:Y>3</gml:Y></gml:coord>
    </gml:Box>
  </gml:boundedBy>
                                                                                                                                                                
  <gml:featureMember>
    <ogr:split_by_char_regex fid="split_by_char_regex.0">
      <ogr:geometryProperty><gml:Point srsName="EPSG:4326"><gml:coordinates>2,2</gml:coordinates></gml:Point></ogr:geometryProperty>
      <ogr:val>xyz</ogr:val>
      <ogr:val2>a</ogr:val2>
    </ogr:split_by_char_regex>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:split_by_char_regex fid="split_by_char_regex.1">
      <ogr:geometryProperty><gml:Point srsName="EPSG:4326"><gml:coordinates>1,1</gml:coordinates></gml:Point></ogr:geometryProperty>
      <ogr:val>aBaBcc</ogr:val>
      <ogr:val2>ab</ogr:val2>
    </ogr:split_by_char_regex>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:split_by_char_regex fid="split_by_char_regex.2">
      <ogr:geometryProperty><gml:Point srsName="EPSG:4326"><gml:coordinates>1,1</gml:coordinates></gml:Point></ogr:geometryProperty>
      <ogr:val>aBaBcc</ogr:val>
      <ogr:val2>b</ogr:val2>
    </ogr:split_by_char_regex>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:split_by_char_regex fid="split_by_char_regex.3">
      <ogr:geometryProperty><gml:Point srsName="EPSG:4326"><gml:coordinates>3,3</gml:coordinates></gml:Point></ogr:geometryProperty>
      <ogr:val>a B c</ogr:val>
      <ogr:val2>a</ogr:val2>
    </ogr:split_by_char_regex>
  </gml:featureMember>
</ogr:FeatureCollection>
