#!/bin/bash

# Testing SLD Rendering in GeoServer using Docker

# cleanup prevous attemps
docker rm -f geoserver
docker rm -f postgis

# stop on error
set -e

echo "start postgis and geoserver"
docker run -d --name postgis -e POSTGRES_USER=admin -e POSTGRES_PASSWORD=geoserver -e POSTGRES_DB=geo -p 5432:5432 postgis/postgis:17-3.5-alpine
docker run -d --name geoserver -p 8080:8080 -e GEOSERVER_ADMIN_USER=admin -e GEOSERVER_ADMIN_PASSWORD=geoserver --link postgis:postgis kartoza/geoserver:2.27.0

echo "wait for db"
echo "waiting for db and geoserver"
sleep 15

echo "load sample point data (world cities)"
docker exec postgis bash -c "psql -U admin -d geo -c 'CREATE TABLE mypoint (id INT, geom GEOMETRY(POINT, 4326)); INSERT INTO mypoint(id, geom) VALUES (1, ST_SetSRID(ST_MakePoint(43, 5), 4326));'"

echo "create workspace"
curl -u admin:geoserver -XPOST -H "Content-type: text/xml" -d "<workspace><name>test</name></workspace>" http://localhost:8080/geoserver/rest/workspaces

echo "configure PostGIS store"
curl -u admin:geoserver -XPOST -H "Content-type: text/xml" -d "
    <dataStore>
        <name>postgis</name>
        <type>PostGIS</type>
        <connectionParameters>
            <host>postgis</host>
            <port>5432</port>
            <database>geo</database>
            <user>admin</user>
            <passwd>geoserver</passwd>
            <dbtype>postgis</dbtype>
        </connectionParameters>
    </dataStore>" \
  http://localhost:8080/geoserver/rest/workspaces/test/datastores

echo "publish mypoint layer"
curl -u admin:geoserver -XPOST -H "Content-type: text/xml" -d "
    <featureType>
        <name>mypoint</name>
        <nativeName>mypoint</nativeName>
        <title>World Cities</title>
        <srs>EPSG:4326</srs>
    </featureType>" \
  http://localhost:8080/geoserver/rest/workspaces/test/datastores/postgis/featuretypes

echo "doing the work"
for file in ./sld/*.sld; do
    # Create the style
    curl -u admin:geoserver -XPOST -H "Content-type: application/vnd.ogc.sld+xml" -d @$file http://localhost:8080/geoserver/rest/styles?name=${file##*/}

    # Assign style to layer
    curl -u admin:geoserver -XPUT -H "Content-type: text/xml" -d "<layer><defaultStyle><name>${file##*/}</name></defaultStyle></layer>" http://localhost:8080/geoserver/rest/layers/test:mypoint

    # Test the rendering - Open in browser:
    curl -u admin:geoserver -o "$file.png" "http://localhost:8080/geoserver/wms?service=WMS&version=1.1.0&request=GetMap&layers=test:mypoint&styles=&bbox=42,4,44,6&width=800&height=600&srs=EPSG:4326&format=image/png"
done

