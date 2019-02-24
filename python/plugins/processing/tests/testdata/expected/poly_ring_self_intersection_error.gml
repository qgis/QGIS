<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation="http://ogr.maptools.org/ poly_ring_self_intersection_error.xsd"
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml">
  <gml:boundedBy>
    <gml:Box>
      <gml:coord><gml:X>300</gml:X><gml:Y>200</gml:Y></gml:coord>
      <gml:coord><gml:X>300</gml:X><gml:Y>200</gml:Y></gml:coord>
    </gml:Box>
  </gml:boundedBy>
                                                                                                                                                        
  <gml:featureMember>
    <ogr:poly_ring_self_intersection_error fid="poly_ring_self_intersection_error.0">
      <ogr:geometryProperty><gml:Point srsName="EPSG:28356"><gml:coordinates>300,200</gml:coordinates></gml:Point></ogr:geometryProperty>
      <ogr:message>Ring self-intersection</ogr:message>
    </ogr:poly_ring_self_intersection_error>
  </gml:featureMember>
</ogr:FeatureCollection>
