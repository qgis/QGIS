"""
***************************************************************************
    TestData.py
    ---------------------
    Date                 : March 2013
    Copyright            : (C) 2013 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Victor Olaya"
__date__ = "March 2013"
__copyright__ = "(C) 2013, Victor Olaya"

import os.path
import tempfile

from qgis.core import QgsRasterLayer
from qgis.testing import QgisTestCase

testDataPath = os.path.join(os.path.dirname(__file__), "testdata")


def table():
    return os.path.join(testDataPath, "table.dbf")


def points():
    return os.path.join(testDataPath, "points.gml")


def invalid_geometries():
    return os.path.join(testDataPath, "invalidgeometries.gml")


def wms_layer_1_1_1(test_id: str, crs: str) -> QgsRasterLayer:
    basetestpath = tempfile.mkdtemp().replace("\\", "/")

    endpoint = basetestpath + f"/{test_id}_fake_qgis_http_endpoint"
    with open(
        QgisTestCase.sanitize_local_url(
            endpoint, "?SERVICE=WMS&REQUEST=GetCapabilities"
        ),
        "wb",
    ) as f:
        f.write(
            b"""
            <WMT_MS_Capabilities version="1.1.1">
            <Service>
              <Name>OGC:WMS</Name>
              <Title>OpenStreetMap WMS</Title>
              <Abstract>OpenStreetMap WMS, bereitgestellt durch terrestris GmbH und Co. KG. Beschleunigt mit MapProxy (http://mapproxy.org/)</Abstract>
              <OnlineResource xmlns:xlink="http://www.w3.org/1999/xlink" xlink:href="http://www.terrestris.de"/>
              <AccessConstraints>(c) OpenStreetMap contributors (http://www.openstreetmap.org/copyright) (c) OpenStreetMap Data (http://openstreetmapdata.com) (c) Natural Earth Data (http://www.naturalearthdata.com) (c) GEBCO Compilation Group (2021) GEBCO 2021 Grid (doi:10.5285/c6612cbe-50b3-0cff-e053-6c86abc09f8f) (c) SRTM 450m by ViewfinderPanoramas (http://viewfinderpanoramas.org/) (c) Great Lakes Bathymetry by NGDC (http://www.ngdc.noaa.gov/mgg/greatlakes/) (c) SRTM 30m by NASA EOSDIS Land Processes Distributed Active Archive Center (LP DAAC, https://lpdaac.usgs.gov/) by using this service you agree to the privacy policy mentioned at https://www.terrestris.de/en/datenschutzerklaerung/</AccessConstraints>
            </Service>
            <Capability>
              <Request>
                <GetCapabilities>
                  <Format>application/vnd.ogc.wms_xml</Format>
                </GetCapabilities>
                <GetMap>
                    <Format>image/jpeg</Format>
                    <Format>image/png</Format>
                </GetMap>
                <GetFeatureInfo>
                  <Format>text/plain</Format>
                  <Format>text/html</Format>
                  <Format>application/vnd.ogc.gml</Format>
                </GetFeatureInfo>
                <GetLegendGraphic>
                    <Format>image/jpeg</Format>
                    <Format>image/png</Format>
                    <DCPType>
                        <HTTP>
                            <Get><OnlineResource xmlns:xlink="http://www.w3.org/1999/xlink" xlink:href="https://ows.terrestris.de/osm/service?"/></Get>
                        </HTTP>
                    </DCPType>
                </GetLegendGraphic>
              </Request>
              <Exception>
                <Format>application/vnd.ogc.se_xml</Format>
                <Format>application/vnd.ogc.se_inimage</Format>
                <Format>application/vnd.ogc.se_blank</Format>
              </Exception>
              <Layer queryable="1">
                <Title>OpenStreetMap WMS</Title>
                <SRS>EPSG:3857</SRS>
                <SRS>EPSG:4326</SRS>
                <SRS>EPSG:4686</SRS>
                <LatLonBoundingBox minx="-180" miny="-89.999999" maxx="180" maxy="89.999999" />
                <BoundingBox SRS="EPSG:900913" minx="-20037508.3428" miny="-147730762.67" maxx="20037508.3428" maxy="147730758.195" />
                <BoundingBox SRS="EPSG:4326" minx="-180.0" miny="-90.0" maxx="180.0" maxy="90.0" />
                <BoundingBox SRS="EPSG:3857" minx="-20037508.3428" miny="-147730762.67" maxx="20037508.3428" maxy="147730758.195" />
                <Layer queryable="1">
                  <Name>OSM-WMS</Name>
                  <Title>OpenStreetMap WMS - by terrestris</Title>
                  <LatLonBoundingBox minx="-180" miny="-88" maxx="180" maxy="88" />
                  <BoundingBox SRS="EPSG:900913" minx="-20037508.3428" miny="-25819498.5135" maxx="20037508.3428" maxy="25819498.5135" />
                  <BoundingBox SRS="EPSG:4326" minx="-180" miny="-88" maxx="180" maxy="88" />
                  <BoundingBox SRS="EPSG:3857" minx="-20037508.3428" miny="-25819498.5135" maxx="20037508.3428" maxy="25819498.5135" />
                  <Style>
                      <Name>default</Name>
                      <Title>default</Title>
                      <LegendURL width="155" height="344">
                          <Format>image/png</Format>
                          <OnlineResource xmlns:xlink="http://www.w3.org/1999/xlink" xlink:type="simple" xlink:href="https://ows.terrestris.de/osm/service?styles=&amp;layer=OSM-WMS&amp;service=WMS&amp;format=image%2Fpng&amp;sld_version=1.1.0&amp;request=GetLegendGraphic&amp;version=1.1.1"/>
                      </LegendURL>
                  </Style>
                </Layer>
              </Layer>
            </Capability>
            </WMT_MS_Capabilities>"""
        )
    return QgsRasterLayer(
        f"crs={crs}&layers=OSM-WMS&styles&url=file://"
        + QgisTestCase.sanitize_local_url(
            endpoint, "?SERVICE=WMS&REQUEST=GetCapabilities"
        ),
        "test",
        "wms",
    )


def wms_layer_1_3_0(test_id: str, crs: str) -> QgsRasterLayer:
    basetestpath = tempfile.mkdtemp().replace("\\", "/")

    endpoint = basetestpath + f"/{test_id}_fake_qgis_http_endpoint"
    with open(
        QgisTestCase.sanitize_local_url(
            endpoint, "?SERVICE=WMS&REQUEST=GetCapabilities"
        ),
        "wb",
    ) as f:
        f.write(
            b"""
<WMS_Capabilities version="1.3.0" updateSequence="2872"
xmlns="http://www.opengis.net/wms"
xmlns:xlink="http://www.w3.org/1999/xlink"
xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.opengis.net/wms https://stadtplan.goettingen.de/geoserver/schemas/wms/1.3.0/capabilities_1_3_0.xsd">
<Service>
        <Name>WMS</Name>
        <Title>GeoServer Web Map Service</Title>
        <Abstract>A compliant implementation of WMS plus most of the SLD extension (dynamic styling). Can also generate PDF, SVG, KML, GeoRSS</Abstract>
</Service>
<Capability>
        <Request>
                <GetCapabilities>
                        <Format>text/xml</Format>
                </GetCapabilities>
                <GetMap>
                        <Format>image/png</Format>
                        <Format>image/jpeg</Format>
                        <Format>image/gif</Format>
                        <Format>image/GeoTIFF</Format>
                        <Format>image/tiff</Format>
                </GetMap>
                <GetFeatureInfo>
                        <Format>text/plain</Format>
                        <Format>text/html</Format>
                        <Format>text/xml</Format>
                </GetFeatureInfo>
        </Request>
        <Exception>
                <Format>XML</Format>
                <Format>INIMAGE</Format>
                <Format>BLANK</Format>
        </Exception>
        <Layer queryable="0" opaque="0" cascaded="1">
                <Name>de_basemapde_web_raster_farbe</Name>
                <Title>basemap.de Web Raster Farbe</Title>
                <Abstract>Der Layer enthaelt eine kombinierte Darstellung von ATKIS(c)- Landschaftsmodellen, der Hauskoordinaten und der Hausumringe,  die nach basemap.de WebSK in der jeweils aktuellen Version eine Kartensignatur haben, inklusive ihrer Beschriftung.</Abstract>
                <KeywordList/>
                <CRS>EPSG:3857</CRS>
                <CRS>CRS:84</CRS>
                <EX_GeographicBoundingBox>
                        <westBoundLongitude>0.1059467424056892</westBoundLongitude>
                        <eastBoundLongitude>20.448891294525627</eastBoundLongitude>
                        <southBoundLatitude>45.2375427360256</southBoundLatitude>
                        <northBoundLatitude>56.84787345153812</northBoundLatitude>
                </EX_GeographicBoundingBox>
                <BoundingBox CRS="CRS:84" minx="0.1059467424056892" miny="45.2375427360256" maxx="20.448891294525627" maxy="56.84787345153812"/>
                <BoundingBox CRS="EPSG:3857" minx="11793.937415807442" miny="5658995.571407319" maxx="2276360.1661935975" maxy="7729088.680470819"/>
                <Style>
                        <Name/>
                        <Title/>
                        <LegendURL width="2" height="1">
                                <Format>image/png</Format>
                                <OnlineResource
                                        xmlns:xlink="http://www.w3.org/1999/xlink" xlink:type="simple" xlink:href="https://stadtplan.goettingen.de/geoserver/goettingen/ows?service=WMS&amp;version=1.3.0&amp;request=GetLegendGraphic&amp;format=image%2Fpng&amp;width=20&amp;height=20&amp;layer=de_basemapde_web_raster_farbe"/>
                                </LegendURL>
                        </Style>
                </Layer>
        </Capability>
</WMS_Capabilities>"""
        )
    return QgsRasterLayer(
        f"crs={crs}&layers=de_basemapde_web_raster_farbe&styles&url=file://"
        + QgisTestCase.sanitize_local_url(
            endpoint, "?SERVICE=WMS&REQUEST=GetCapabilities"
        ),
        "test",
        "wms",
    )


def wms_layer_1_3_0_frankfurt(test_id: str, crs: str) -> QgsRasterLayer:
    basetestpath = tempfile.mkdtemp().replace("\\", "/")

    endpoint = basetestpath + f"/{test_id}_fake_qgis_http_endpoint"
    with open(
        QgisTestCase.sanitize_local_url(
            endpoint, "?SERVICE=WMS&REQUEST=GetCapabilities"
        ),
        "wb",
    ) as f:
        f.write(
            b"""
            <WMS_Capabilities xmlns="http://www.opengis.net/wms" xmlns:sld="http://www.opengis.net/sld" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" version="1.3.0" xsi:schemaLocation="http://www.opengis.net/wms http://schemas.opengis.net/wms/1.3.0/capabilities_1_3_0.xsd http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/sld_capabilities.xsd">
            <Service>
              <Name>WMS</Name>
              <Title>MapProxy-WMS Rasterbilder Bebauungsplaene rv auf Basiskarte grau</Title>
              <Abstract>WMS mit Rasterbilder der rechtsverbindlichen Bebauungsplaene auf grauer Basiskarte ueber MapProxy</Abstract>
                <MaxWidth>4000</MaxWidth>
                <MaxHeight>4000</MaxHeight>
            </Service>
            <Capability>
              <Request>
                <GetCapabilities>
                  <Format>text/xml</Format>
                </GetCapabilities>
                <GetMap>
                  <Format>image/png</Format>
                  <Format>image/jpeg</Format>
                  <Format>image/gif</Format>
                  <Format>image/GeoTIFF</Format>
                  <Format>image/tiff</Format>
                </GetMap>
                <GetFeatureInfo>
                  <Format>text/plain</Format>
                  <Format>text/html</Format>
                  <Format>text/xml</Format>
                </GetFeatureInfo>
              </Request>
              <Exception>
                <Format>XML</Format>
                <Format>INIMAGE</Format>
                <Format>BLANK</Format>
              </Exception>
              <Layer>
                <Name>bplan_stadtkarte</Name>
                <Title>Bebauungsplaene Rasterbilder auf grauer Basiskarte</Title>
                <CRS>EPSG:25832</CRS>
                <CRS>EPSG:4326</CRS>
                <EX_GeographicBoundingBox>
                  <westBoundLongitude>8.471329688231897</westBoundLongitude>
                  <eastBoundLongitude>8.801042621684477</eastBoundLongitude>
                  <southBoundLatitude>50.01482739264088</southBoundLatitude>
                  <northBoundLatitude>50.22734893698391</northBoundLatitude>
                </EX_GeographicBoundingBox>
                <BoundingBox CRS="CRS:84" minx="8.471329688231897" miny="50.01482739264088" maxx="8.801042621684477" maxy="50.22734893698391" />
                <BoundingBox CRS="EPSG:4326" minx="50.01482739264088" miny="8.471329688231897" maxx="50.22734893698391" maxy="8.801042621684477" />
                <BoundingBox CRS="EPSG:25832" minx="462290" miny="5540412" maxx="485746" maxy="5563928" />
              </Layer>
            </Capability>
            </WMS_Capabilities>
            """
        )
    return QgsRasterLayer(
        f"crs={crs}&layers=bplan_stadtkarte&styles&url=file://"
        + QgisTestCase.sanitize_local_url(
            endpoint, "?SERVICE=WMS&REQUEST=GetCapabilities"
        ),
        "test",
        "wms",
    )
