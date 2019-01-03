<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation="http://ogr.maptools.org/ hstore_two_fields.xsd"
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml">
  <gml:boundedBy>
    <gml:Box>
      <gml:coord><gml:X>-1</gml:X><gml:Y>-3</gml:Y></gml:coord>
      <gml:coord><gml:X>10</gml:X><gml:Y>6</gml:Y></gml:coord>
    </gml:Box>
  </gml:boundedBy>
                                                                                                                                                             
  <gml:featureMember>
    <ogr:hstore_two_fields fid="polys.0">
      <ogr:geometryProperty><gml:MultiPolygon srsName="EPSG:4326"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>-1,-1 -1,3 3,3 3,2 2,2 2,-1 -1,-1</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon></ogr:geometryProperty>
      <ogr:hstore>&quot;amenity&quot;=&gt;&quot;restaurant&quot;,&quot;barrier&quot;=&gt;&quot;wall&quot;</ogr:hstore>
      <ogr:internet_access>yes</ogr:internet_access>
      <ogr:cuisine>chinese</ogr:cuisine>
      <ogr:doesntexist xsi:nil="true"/>
    </ogr:hstore_two_fields>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:hstore_two_fields fid="polys.1">
      <ogr:geometryProperty><gml:MultiPolygon srsName="EPSG:4326"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>5,5 6,4 4,4 5,5</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon></ogr:geometryProperty>
      <ogr:hstore>&quot;amenity&quot;=&gt;&quot;fuel&quot;,&quot;building&quot;=&gt;&quot;roof&quot;,&quot;name&quot;=&gt;&quot;foo&quot;</ogr:hstore>
      <ogr:internet_access xsi:nil="true"/>
      <ogr:cuisine xsi:nil="true"/>
      <ogr:doesntexist xsi:nil="true"/>
    </ogr:hstore_two_fields>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:hstore_two_fields fid="polys.2">
      <ogr:geometryProperty><gml:MultiPolygon srsName="EPSG:4326"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>2,5 2,6 3,6 3,5 2,5</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon></ogr:geometryProperty>
      <ogr:hstore>&quot;building&quot;=&gt;&quot;yes&quot;</ogr:hstore>
      <ogr:internet_access xsi:nil="true"/>
      <ogr:cuisine xsi:nil="true"/>
      <ogr:doesntexist xsi:nil="true"/>
    </ogr:hstore_two_fields>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:hstore_two_fields fid="polys.3">
      <ogr:geometryProperty><gml:MultiPolygon srsName="EPSG:4326"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>6,1 10,1 10,-3 6,-3 6,1</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs><gml:innerBoundaryIs><gml:LinearRing><gml:coordinates>7,0 7,-2 9,-2 9,0 7,0</gml:coordinates></gml:LinearRing></gml:innerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon></ogr:geometryProperty>
      <ogr:hstore>&quot;amenity&quot;=&gt;&quot;restaurant&quot;,&quot;name&quot;=&gt;&quot;bar&quot;,&quot;operator&quot;=&gt;&quot;foo&quot;</ogr:hstore>
      <ogr:internet_access xsi:nil="true"/>
      <ogr:cuisine>burger</ogr:cuisine>
      <ogr:doesntexist xsi:nil="true"/>
    </ogr:hstore_two_fields>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:hstore_two_fields fid="polys.4">
      <ogr:hstore>&quot;amenity&quot;=&gt;&quot;bank&quot;,&quot;atm&quot;=&gt;&quot;yes&quot;</ogr:hstore>
      <ogr:internet_access xsi:nil="true"/>
      <ogr:cuisine xsi:nil="true"/>
      <ogr:doesntexist xsi:nil="true"/>
    </ogr:hstore_two_fields>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:hstore_two_fields fid="polys.5">
      <ogr:geometryProperty><gml:MultiPolygon srsName="EPSG:4326"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>3,2 6,1 6,-3 2,-1 2,2 3,2</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon></ogr:geometryProperty>
      <ogr:hstore>&quot;stars&quot;=&gt;&quot;5&quot;,&quot;tourism&quot;=&gt;&quot;hotel&quot;</ogr:hstore>
      <ogr:internet_access xsi:nil="true"/>
      <ogr:cuisine xsi:nil="true"/>
      <ogr:doesntexist xsi:nil="true"/>
    </ogr:hstore_two_fields>
  </gml:featureMember>
</ogr:FeatureCollection>
