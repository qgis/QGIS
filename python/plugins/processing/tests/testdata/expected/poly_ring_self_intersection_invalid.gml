<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation="http://ogr.maptools.org/ poly_ring_self_intersection_invalid.xsd"
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml">
  <gml:boundedBy>
    <gml:Box>
      <gml:coord><gml:X>200</gml:X><gml:Y>200</gml:Y></gml:coord>
      <gml:coord><gml:X>400</gml:X><gml:Y>400</gml:Y></gml:coord>
    </gml:Box>
  </gml:boundedBy>
                                                                                                                                                        
  <gml:featureMember>
    <ogr:poly_ring_self_intersection_invalid fid="poly_ring_self_intersection.0">
      <ogr:geometryProperty><gml:Polygon srsName="EPSG:28356"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>200,400 400,400 400,200 300,200 350,250 250,250 300,200 200,200 200,400</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></ogr:geometryProperty>
      <ogr:_errors>Ring self-intersection</ogr:_errors>
    </ogr:poly_ring_self_intersection_invalid>
  </gml:featureMember>
</ogr:FeatureCollection>
