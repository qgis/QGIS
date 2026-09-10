<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     gml:id="aFeatureCollection"
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation="http://ogr.maptools.org/ buffer_dissolve_keep_disjoint.xsd"
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml/3.2">
  <gml:boundedBy><gml:Envelope srsName="urn:ogc:def:crs:EPSG::4326"><gml:lowerCorner>-1.6 -0.6</gml:lowerCorner><gml:upperCorner>6.6 9.6</gml:upperCorner></gml:Envelope></gml:boundedBy>
                                                                                                                                                                    
  <ogr:featureMember>
    <ogr:buffer_dissolve_keep_disjoint gml:id="buffer_dissolve_keep_disjoint.0">
      <gml:boundedBy><gml:Envelope srsName="urn:ogc:def:crs:EPSG::4326"><gml:lowerCorner>-0.6 -0.6</gml:lowerCorner><gml:upperCorner>3.6 4.6</gml:upperCorner></gml:Envelope></gml:boundedBy>
      <ogr:geometryProperty><gml:MultiSurface srsName="urn:ogc:def:crs:EPSG::4326" gml:id="buffer_dissolve_keep_disjoint.geom.0"><gml:surfaceMember><gml:Polygon gml:id="buffer_dissolve_keep_disjoint.geom.0.0"><gml:exterior><gml:LinearRing><gml:posList>-0.570633909777092 -0.185410196624969 -0.485410196624968 -0.352671151375484 -0.352671151375484 -0.485410196624969 -0.185410196624968 -0.570633909777092 0.0 -0.6 1.0 -0.6 1.18541019662497 -0.570633909777092 1.35267115137548 -0.485410196624968 1.48541019662497 -0.352671151375484 1.57063390977709 -0.185410196624968 1.6 0.0 1.6 1.0 1.57063390977709 1.18541019662497 1.48541019662497 1.35267115137548 1.43808134800045 1.4 2.0 1.4 2.18541019662497 1.42936609022291 2.35267115137548 1.51458980337503 2.48541019662497 1.64732884862452 2.57063390977709 1.81458980337503 2.6 2.0 2.6 2.4 3.0 2.4 3.18541019662497 2.42936609022291 3.35267115137548 2.51458980337503 3.48541019662497 2.64732884862452 3.57063390977709 2.81458980337503 3.6 3.0 3.6 4.0 3.57063390977709 4.18541019662497 3.48541019662497 4.35267115137548 3.35267115137548 4.48541019662497 3.18541019662497 4.57063390977709 3.0 4.6 1.0 4.6 0.814589803375032 4.57063390977709 0.647328848624516 4.48541019662497 0.514589803375032 4.35267115137548 0.429366090222908 4.18541019662497 0.4 4.0 0.4 2.0 0.429366090222908 1.81458980337503 0.514589803375032 1.64732884862452 0.561918651999547 1.6 0.0 1.6 -0.185410196624968 1.57063390977709 -0.352671151375484 1.48541019662497 -0.485410196624968 1.35267115137548 -0.570633909777092 1.18541019662497 -0.6 1.0 -0.6 0.0 -0.570633909777092 -0.185410196624969</gml:posList></gml:LinearRing></gml:exterior></gml:Polygon></gml:surfaceMember></gml:MultiSurface></ogr:geometryProperty>
      <ogr:fid>multipolys.0</ogr:fid>
      <ogr:Bname>Test</ogr:Bname>
      <ogr:Bintval>1</ogr:Bintval>
      <ogr:Bfloatval>0.123</ogr:Bfloatval>
    </ogr:buffer_dissolve_keep_disjoint>
  </ogr:featureMember>
  <ogr:featureMember>
    <ogr:buffer_dissolve_keep_disjoint gml:id="buffer_dissolve_keep_disjoint.1">
      <gml:boundedBy><gml:Envelope srsName="urn:ogc:def:crs:EPSG::4326"><gml:lowerCorner>-1.6 6.4</gml:lowerCorner><gml:upperCorner>6.6 9.6</gml:upperCorner></gml:Envelope></gml:boundedBy>
      <ogr:geometryProperty><gml:MultiSurface srsName="urn:ogc:def:crs:EPSG::4326" gml:id="buffer_dissolve_keep_disjoint.geom.1"><gml:surfaceMember><gml:Polygon gml:id="buffer_dissolve_keep_disjoint.geom.1.0"><gml:exterior><gml:LinearRing><gml:posList>3.18541019662497 6.42936609022291 3.35267115137548 6.51458980337503 3.48541019662497 6.64732884862452 3.5 6.67596295000161 3.51458980337503 6.64732884862452 3.64732884862452 6.51458980337503 3.81458980337503 6.42936609022291 4.0 6.4 6.0 6.4 6.18541019662497 6.42936609022291 6.35267115137548 6.51458980337503 6.48541019662497 6.64732884862452 6.57063390977709 6.81458980337503 6.6 7.0 6.6 9.0 6.57063390977709 9.18541019662497 6.48541019662497 9.35267115137548 6.35267115137548 9.48541019662497 6.18541019662497 9.57063390977709 6.0 9.6 5.0 9.6 4.84470857293849 9.57955549577344 4.7 9.51961524227066 4.57573593128807 9.42426406871193 3.57573593128807 8.42426406871193 3.5037927124169 8.31659343276463 3.48541019662497 8.35267115137548 3.35267115137548 8.48541019662497 3.18541019662497 8.57063390977709 3.0 8.6 -1 8.6 -1.18541019662497 8.57063390977709 -1.35267115137548 8.48541019662497 -1.48541019662497 8.35267115137548 -1.57063390977709 8.18541019662497 -1.6 8.0 -1.6 7.0 -1.57063390977709 6.81458980337503 -1.48541019662497 6.64732884862452 -1.35267115137548 6.51458980337503 -1.18541019662497 6.42936609022291 -1 6.4 3.0 6.4 3.18541019662497 6.42936609022291</gml:posList></gml:LinearRing></gml:exterior></gml:Polygon></gml:surfaceMember></gml:MultiSurface></ogr:geometryProperty>
      <ogr:fid>multipolys.0</ogr:fid>
      <ogr:Bname>Test</ogr:Bname>
      <ogr:Bintval>1</ogr:Bintval>
      <ogr:Bfloatval>0.123</ogr:Bfloatval>
    </ogr:buffer_dissolve_keep_disjoint>
  </ogr:featureMember>
</ogr:FeatureCollection>
