<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation="http://ogr.maptools.org/ intersection1.xsd"
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml">
  <gml:boundedBy>
    <gml:Box>
      <gml:coord><gml:X>1</gml:X><gml:Y>5</gml:Y></gml:coord>
      <gml:coord><gml:X>8</gml:X><gml:Y>8</gml:Y></gml:coord>
    </gml:Box>
  </gml:boundedBy>

  <gml:featureMember>
    <ogr:intersection1 fid="intersection1.0">
      <ogr:geometryProperty><gml:MultiPolygon srsName="EPSG:3857"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>2,6 2,5 1,5 1,6 2,6</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon></ogr:geometryProperty>
      <ogr:id_a>A1</ogr:id_a>
      <ogr:id_b>B2</ogr:id_b>
    </ogr:intersection1>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:intersection1 fid="intersection1.1">
      <ogr:geometryProperty><gml:MultiPolygon srsName="EPSG:3857"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>4,5 3,5 3,6 4,6 4,5</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon></ogr:geometryProperty>
      <ogr:id_a>A2</ogr:id_a>
      <ogr:id_b>B2</ogr:id_b>
    </ogr:intersection1>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:intersection1 fid="intersection1.2">
      <ogr:geometryProperty><gml:MultiPolygon srsName="EPSG:3857"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>8,7 7,7 7,8 8,8 8,7</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon></ogr:geometryProperty>
      <ogr:id_a>A3</ogr:id_a>
      <ogr:id_b>B1</ogr:id_b>
    </ogr:intersection1>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:intersection1 fid="intersection1.3">
      <ogr:geometryProperty><gml:MultiPolygon srsName="EPSG:3857"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>6,7 5,7 5,8 6,8 6,7</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon></ogr:geometryProperty>
      <ogr:id_a>A3</ogr:id_a>
      <ogr:id_b>B3</ogr:id_b>
    </ogr:intersection1>
  </gml:featureMember>
</ogr:FeatureCollection>
