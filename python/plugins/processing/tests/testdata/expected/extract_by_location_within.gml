<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation=""
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml">
  <gml:boundedBy>
    <gml:Box>
      <gml:coord><gml:X>1</gml:X><gml:Y>1</gml:Y></gml:coord>
      <gml:coord><gml:X>4</gml:X><gml:Y>1</gml:Y></gml:coord>
    </gml:Box>
  </gml:boundedBy>
                                                                                                                                                                
  <gml:featureMember>
    <ogr:extract_by_location_within fid="extract_by_location_within.0">
      <ogr:geometryProperty><gml:Point srsName="EPSG:4326"><gml:coordinates>1,1</gml:coordinates></gml:Point></ogr:geometryProperty>
    </ogr:extract_by_location_within>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:extract_by_location_within fid="extract_by_location_within.1">
      <ogr:geometryProperty><gml:Point srsName="EPSG:4326"><gml:coordinates>4,1</gml:coordinates></gml:Point></ogr:geometryProperty>
    </ogr:extract_by_location_within>
  </gml:featureMember>
</ogr:FeatureCollection>
