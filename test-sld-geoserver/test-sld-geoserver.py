#!/bin/python3

# Testing SLD Rendering in GeoServer using Docker

import base64
import subprocess

from pathlib import Path
from time import sleep

from requests import Session

# cleanup prevous attemps
# subprocess.check_call(["docker", "rm", "-f", "geoserver"])
# subprocess.check_call(["docker", "rm", "-f", "postgis"])

# print("start postgis and geoserver")
# subprocess.run(
#     [
#         "docker",
#         "run",
#         "-d",
#         "--name",
#         "postgis",
#         "-e",
#         "POSTGRES_USER=admin",
#         "-e",
#         "POSTGRES_PASSWORD=geoserver",
#         "-e",
#         "POSTGRES_DB=geo",
#         "-p",
#         "5432:5432",
#         "postgis/postgis:17-3.5-alpine",
#     ]
# )
# subprocess.run(
#     [
#         "docker",
#         "run",
#         "-d",
#         "--name",
#         "geoserver",
#         "-p",
#         "8080:8080",
#         "-e",
#         "GEOSERVER_ADMIN_USER=admin",
#         "-e",
#         "GEOSERVER_ADMIN_PASSWORD=geoserver",
#         "--link",
#         "postgis:postgis",
#         "kartoza/geoserver:2.23.0",
#     ]
# )

# print("waiting for db and geoserver")
# sleep(20)

# print("load sample point data (world cities)")
# subprocess.run(
#     [
#         "docker",
#         "exec",
#         "postgis",
#         "bash",
#         "-c",
#         "psql -U admin -d geo -c 'CREATE TABLE mypoint (id INT, geom GEOMETRY(POINT, 4326)); INSERT INTO mypoint(id, geom) VALUES (1, ST_SetSRID(ST_MakePoint(43, 5), 4326));'",
#     ]
# )

session = Session()
session.auth = ("admin", "geoserver")

print("create workspace")
session.post(
    "http://localhost:8080/geoserver/rest/workspaces",
    headers={"Content-Type": "text/xml"},
    data="<workspace><name>test</name></workspace>",
)

print("configure PostGIS store")
session.post(
    "http://localhost:8080/geoserver/rest/workspaces/test/datastores",
    headers={"Content-Type": "text/xml"},
    data="""
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
    </dataStore>""",
)


print("publish mypoint layer")
session.post(
    "http://localhost:8080/geoserver/rest/workspaces/test/datastores/postgis/featuretypes",
    headers={"Content-Type": "text/xml"},
    data="""
    <featureType>
        <name>mypoint</name>
        <nativeName>mypoint</nativeName>
        <title>World Cities</title>
        <srs>EPSG:4326</srs>
    </featureType>""",
)


print("upload icon")
icon_file = Path("icon.jpg")
icon_b64 = base64.b64encode(icon_file.read_bytes()).decode()
session.put(
    "http://localhost:8080/geoserver/rest/resource/styles/icon.jpg",
    headers={"Content-Type": "image/jpeg"},
    data=icon_file.read_bytes(),
)


print("doing the work")
sld_template = Path("00-original.sld").read_text()
for name, link, mime in [
    # fmt: off
    ("remote-correct-mime", "https://picsum.photos/id/237/200/300", "image/jpeg"),
    ("remote-incorrect-mime", "https://picsum.photos/id/237/200/300", "image/gif"),
    ("remote-semigeneric-mime", "https://picsum.photos/id/237/200/300", "image/octet-stream"),
    ("remote-generic-mime", "https://picsum.photos/id/237/200/300", "application/binary"),
    ("local-correct-mime", "icon.jpg", "image/jpeg"),
    ("local-incorrect-mime", "icon.jpg", "image/gif"),
    ("local-semigeneric-mime", "icon.jpg", "image/octet-stream"),
    ("local-generic-mime", "icon.jpg", "application/binary"),
    ("embedded-correct-mime", f"data:image/jpeg;base64,{icon_b64}", "image/jpeg"),
    ("embedded-incorrect-mime", f"data:image/gif;base64,{icon_b64}", "image/gif"),
    ("embedded-semigeneric-mime", f"data:image/octet-stream;base64,{icon_b64}", "image/octet-stream"),
    ("embedded-generic-mime", f"data:image/binary;base64,{icon_b64}", "application/binary"),
    # fmt: on
]:
    sld = sld_template.replace(
        "<!--QgsMarkerSymbolLayer RasterMarker not implemented yet-->",
        f"""<se:Graphic>
              <se:ExternalGraphic>
                <se:OnlineResource xlink:type="simple" xlink:href="{link}"/>
                <se:Format>{mime}</se:Format>
              </se:ExternalGraphic>
            </se:Graphic>""",
    )

    # Create the style
    resp = session.delete(
        f"http://localhost:8080/geoserver/rest/styles/{name}",
    )
    resp = session.post(
        f"http://localhost:8080/geoserver/rest/styles?name={name}",
        headers={"Content-type": "application/vnd.ogc.sld+xml"},
        data=sld.encode(),
    )

    # Assign style to layer
    resp = session.put(
        "http://localhost:8080/geoserver/rest/layers/test:mypoint",
        headers={"Content-Type": "text/xml"},
        data=f"<layer><defaultStyle><name>{name}</name></defaultStyle></layer>",
    )

    # Test the rendering - Open in browser:
    resp = session.get(
        "http://localhost:8080/geoserver/wms?service=WMS&version=1.1.0&request=GetMap&layers=test:mypoint&styles=&bbox=42,4,44,6&width=800&height=600&srs=EPSG:4326&format=image/png"
    )

    # Write results
    Path("output").mkdir(exist_ok=True)

    ext = "png" if resp.headers["Content-Type"] == "image/png" else "txt"
    Path("output", f"{name}.{ext}").write_bytes(resp.content)
    Path("output", f"{name}.sld").write_bytes(sld.encode())
