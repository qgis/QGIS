<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation="http://ogr.maptools.org/ intersection3.xsd"
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml">
  <gml:boundedBy>
    <gml:Box>
      <gml:coord><gml:X>2</gml:X><gml:Y>2</gml:Y></gml:coord>
      <gml:coord><gml:X>6</gml:X><gml:Y>3</gml:Y></gml:coord>
    </gml:Box>
  </gml:boundedBy>
                                                                                                                                                                
  <gml:featureMember>
    <ogr:intersection3 fid="intersection3.0">
      <ogr:geometryProperty><gml:MultiPolygon srsName="EPSG:3857"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>3,2 2,2 2,3 3,3 3,2</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon></ogr:geometryProperty>
      <ogr:id_a>A1</ogr:id_a>
      <ogr:id_b>B1</ogr:id_b>
    </ogr:intersection3>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:intersection3 fid="intersection3.1">
      <ogr:geometryProperty><gml:MultiPolygon srsName="EPSG:3857"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>6,2 5,2 5,3 6,3 6,2</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon></ogr:geometryProperty>
      <ogr:id_a>A1</ogr:id_a>
      <ogr:id_b>B4</ogr:id_b>
    </ogr:intersection3>
  </gml:featureMember>
</ogr:FeatureCollection>
