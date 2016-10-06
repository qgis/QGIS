# -*- coding: utf-8 -*-
"""
QGIS Server HTTP wrapper

This script launches a QGIS Server listening on port 8081 or on the port
specified on the environment variable QGIS_SERVER_PORT
QGIS_SERVER_HOST (defaults to 127.0.0.1)

For testing purposes, HTTP Basic can be enabled by setting the following
environment variables:

  * QGIS_SERVER_HTTP_BASIC_AUTH (default not set, set to anything to enable)
  * QGIS_SERVER_USERNAME (default ="username")
  * QGIS_SERVER_PASSWORD (default ="password")

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from future import standard_library
standard_library.install_aliases()

__author__ = 'Alessandro Pasotti'
__date__ = '05/15/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


import os
import sys
import signal
import urllib.parse
from http.server import BaseHTTPRequestHandler, HTTPServer
from qgis.core import QgsApplication
from qgis.server import QgsServer

QGIS_SERVER_PORT = int(os.environ.get('QGIS_SERVER_PORT', '8081'))
QGIS_SERVER_HOST = os.environ.get('QGIS_SERVER_HOST', '127.0.0.1')

qgs_app = QgsApplication([], False)
qgs_server = QgsServer()


if os.environ.get('QGIS_SERVER_HTTP_BASIC_AUTH') is not None:
    from qgis.server import QgsServerFilter
    import base64

    class HTTPBasicFilter(QgsServerFilter):

        def responseComplete(self):
            request = self.serverInterface().requestHandler()
            if self.serverInterface().getEnv('HTTP_AUTHORIZATION'):
                username, password = base64.b64decode(self.serverInterface().getEnv('HTTP_AUTHORIZATION')[6:]).split(b':')
                if (username.decode('utf-8') == os.environ.get('QGIS_SERVER_USERNAME', 'username')
                        and password.decode('utf-8') == os.environ.get('QGIS_SERVER_PASSWORD', 'password')):
                    return
            # No auth ...
            request.clearHeaders()
            request.setHeader('Status', '401 Authorization required')
            request.setHeader('WWW-Authenticate', 'Basic realm="QGIS Server"')
            request.clearBody()
            request.appendBody(b'<h1>Authorization required</h1>')

    filter = HTTPBasicFilter(qgs_server.serverInterface())
    qgs_server.serverInterface().registerFilter(filter)


class Handler(BaseHTTPRequestHandler):

    def do_GET(self):
        # CGI vars:
        for k, v in self.headers.items():
            qgs_server.putenv('HTTP_%s' % k.replace(' ', '-').replace('-', '_').replace(' ', '-').upper(), v)
        qgs_server.putenv('SERVER_PORT', str(self.server.server_port))
        qgs_server.putenv('SERVER_NAME', self.server.server_name)
        qgs_server.putenv('REQUEST_URI', self.path)
        parsed_path = urllib.parse.urlparse(self.path)
        headers, body = qgs_server.handleRequest(parsed_path.query)
        headers_dict = dict(h.split(': ', 1) for h in headers.decode().split('\n') if h)
        try:
            self.send_response(int(headers_dict['Status'].split(' ')[0]))
        except:
            self.send_response(200)
        for k, v in headers_dict.items():
            self.send_header(k, v)
        self.end_headers()
        self.wfile.write(body)
        return

    def do_POST(self):
        content_len = int(self.headers.get('content-length', 0))
        post_body = self.rfile.read(content_len).decode()
        request = post_body[1:post_body.find(' ')]
        self.path = self.path + '&REQUEST_BODY=' + \
            post_body.replace('&amp;', '') + '&REQUEST=' + request
        return self.do_GET()


if __name__ == '__main__':
    server = HTTPServer((QGIS_SERVER_HOST, QGIS_SERVER_PORT), Handler)
    print('Starting server on %s:%s, use <Ctrl-C> to stop' %
          (QGIS_SERVER_HOST, server.server_port), flush=True)

    def signal_handler(signal, frame):
        global qgs_app
        print("\nExiting QGIS...")
        qgs_app.exitQgis()
        sys.exit(0)

    signal.signal(signal.SIGINT, signal_handler)
    server.serve_forever()
