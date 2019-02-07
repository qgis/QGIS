#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
QGIS Server HTTP wrapper for testing purposes
================================================================================

This script launches a QGIS Server listening on port 8081 or on the port
specified on the environment variable QGIS_SERVER_PORT.
Hostname is set by environment variable QGIS_SERVER_HOST (defaults to 127.0.0.1)

The server can be configured to support any of the following auth systems
(mutually exclusive):

  * PKI
  * HTTP Basic
  * OAuth2 (requires python package oauthlib, installable with:
            with "pip install oauthlib")


!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
SECURITY WARNING:
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

This script was developed for testing purposes and was not meant to be secure,
please do not use in a production server any of the authentication systems
implemented here.


HTTPS
--------------------------------------------------------------------------------

HTTPS is automatically enabled for PKI and OAuth2


HTTP Basic
--------------------------------------------------------------------------------

A XYZ map service is also available for multithreading testing:

  ?MAP=/path/to/projects.qgs&SERVICE=XYZ&X=1&Y=0&Z=1&LAYERS=world

Set MULTITHREADING environment variable to 1 to activate.


For testing purposes, HTTP Basic can be enabled by setting the following
environment variables:

  * QGIS_SERVER_HTTP_BASIC_AUTH (default not set, set to anything to enable)
  * QGIS_SERVER_USERNAME (default ="username")
  * QGIS_SERVER_PASSWORD (default ="password")


PKI
--------------------------------------------------------------------------------

PKI authentication with HTTPS can be enabled with:

  * QGIS_SERVER_PKI_CERTIFICATE (server certificate)
  * QGIS_SERVER_PKI_KEY (server private key)
  * QGIS_SERVER_PKI_AUTHORITY (root CA)
  * QGIS_SERVER_PKI_USERNAME (valid username)


OAuth2 Resource Owner Grant Flow
--------------------------------------------------------------------------------

OAuth2 Resource Owner Grant Flow with HTTPS can be enabled with:

  * QGIS_SERVER_OAUTH2_AUTHORITY (no default)
  * QGIS_SERVER_OAUTH2_KEY (server private key)
  * QGIS_SERVER_OAUTH2_CERTIFICATE (server certificate)
  * QGIS_SERVER_OAUTH2_USERNAME (default ="username")
  * QGIS_SERVER_OAUTH2_PASSWORD (default ="password")
  * QGIS_SERVER_OAUTH2_TOKEN_EXPIRES_IN (default = 3600)

Available endpoints:

  - /token (returns a new access_token),
            optionally specify an expiration time in seconds with ?ttl=<int>
  - /refresh (returns a new access_token from a refresh token),
             optionally specify an expiration time in seconds with ?ttl=<int>
  - /result (check the Bearer token and returns a short sentence if it validates)


Sample runs
--------------------------------------------------------------------------------

PKI:

QGIS_SERVER_PKI_USERNAME=Gerardus QGIS_SERVER_PORT=47547 QGIS_SERVER_HOST=localhost \
    QGIS_SERVER_PKI_KEY=/home/$USER/dev/QGIS/tests/testdata/auth_system/certs_keys/localhost_ssl_key.pem \
    QGIS_SERVER_PKI_CERTIFICATE=/home/$USER/dev/QGIS/tests/testdata/auth_system/certs_keys/localhost_ssl_cert.pem \
    QGIS_SERVER_PKI_AUTHORITY=/home/$USER/dev/QGIS/tests/testdata/auth_system/certs_keys/chains_subissuer-issuer-root_issuer2-root2.pem \
    python3 /home/$USER/dev/QGIS/tests/src/python/qgis_wrapped_server.py


OAuth2:

QGIS_SERVER_PORT=8443 \
    QGIS_SERVER_HOST=127.0.0.1 \
    QGIS_SERVER_OAUTH2_AUTHORITY=/home/$USER/dev/QGIS/tests/testdata/auth_system/certs_keys/chains_subissuer-issuer-root_issuer2-root2.pem \
    QGIS_SERVER_OAUTH2_CERTIFICATE=/home/$USER/dev/QGIS/tests/testdata/auth_system/certs_keys/127_0_0_1_ssl_cert.pem \
    QGIS_SERVER_OAUTH2_KEY=/home/$USER/dev/QGIS/tests/testdata/auth_system/certs_keys/127_0_0_1_ssl_key.pem \
    python3 \
    /home/$USER/dev/QGIS/tests/src/python/qgis_wrapped_server.py



.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import copy
import os
import signal
import ssl
import sys
import urllib.parse

from http.server import BaseHTTPRequestHandler, HTTPServer
from qgis.core import QgsApplication
from qgis.server import (QgsBufferServerRequest, QgsBufferServerResponse,
                         QgsServer, QgsServerRequest)

__author__ = 'Alessandro Pasotti'
__date__ = '05/15/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


# Needed on Qt 5 so that the serialization of XML is consistent among all
# executions
os.environ['QT_HASH_SEED'] = '1'

import sys
import signal
import ssl
import math
import copy
import urllib.parse
from http.server import BaseHTTPRequestHandler, HTTPServer
from socketserver import ThreadingMixIn
import threading

from qgis.core import QgsApplication, QgsCoordinateTransform, QgsCoordinateReferenceSystem
from qgis.server import QgsServer, QgsServerRequest, QgsBufferServerRequest, QgsBufferServerResponse, QgsServerFilter

QGIS_SERVER_PORT = int(os.environ.get('QGIS_SERVER_PORT', '8081'))
QGIS_SERVER_HOST = os.environ.get('QGIS_SERVER_HOST', '127.0.0.1')

# HTTP Basic
QGIS_SERVER_HTTP_BASIC_AUTH = os.environ.get(
    'QGIS_SERVER_HTTP_BASIC_AUTH', False)
QGIS_SERVER_USERNAME = os.environ.get('QGIS_SERVER_USERNAME', 'username')
QGIS_SERVER_PASSWORD = os.environ.get('QGIS_SERVER_PASSWORD', 'password')

# PKI authentication
QGIS_SERVER_PKI_CERTIFICATE = os.environ.get('QGIS_SERVER_PKI_CERTIFICATE')
QGIS_SERVER_PKI_KEY = os.environ.get('QGIS_SERVER_PKI_KEY')
QGIS_SERVER_PKI_AUTHORITY = os.environ.get('QGIS_SERVER_PKI_AUTHORITY')
QGIS_SERVER_PKI_USERNAME = os.environ.get('QGIS_SERVER_PKI_USERNAME')

# OAuth2 authentication
QGIS_SERVER_OAUTH2_CERTIFICATE = os.environ.get(
    'QGIS_SERVER_OAUTH2_CERTIFICATE')
QGIS_SERVER_OAUTH2_KEY = os.environ.get('QGIS_SERVER_OAUTH2_KEY')
QGIS_SERVER_OAUTH2_AUTHORITY = os.environ.get('QGIS_SERVER_OAUTH2_AUTHORITY')
QGIS_SERVER_OAUTH2_USERNAME = os.environ.get(
    'QGIS_SERVER_OAUTH2_USERNAME', 'username')
QGIS_SERVER_OAUTH2_PASSWORD = os.environ.get(
    'QGIS_SERVER_OAUTH2_PASSWORD', 'password')
QGIS_SERVER_OAUTH2_TOKEN_EXPIRES_IN = os.environ.get(
    'QGIS_SERVER_OAUTH2_TOKEN_EXPIRES_IN', 3600)

# Check if PKI is enabled
QGIS_SERVER_PKI_AUTH = (
    QGIS_SERVER_PKI_CERTIFICATE is not None and
    os.path.isfile(QGIS_SERVER_PKI_CERTIFICATE) and
    QGIS_SERVER_PKI_KEY is not None and
    os.path.isfile(QGIS_SERVER_PKI_KEY) and
    QGIS_SERVER_PKI_AUTHORITY is not None and
    os.path.isfile(QGIS_SERVER_PKI_AUTHORITY) and
    QGIS_SERVER_PKI_USERNAME)

# Check if OAuth2 is enabled
QGIS_SERVER_OAUTH2_AUTH = (
    QGIS_SERVER_OAUTH2_CERTIFICATE is not None and
    os.path.isfile(QGIS_SERVER_OAUTH2_CERTIFICATE) and
    QGIS_SERVER_OAUTH2_KEY is not None and
    os.path.isfile(QGIS_SERVER_OAUTH2_KEY) and
    QGIS_SERVER_OAUTH2_AUTHORITY is not None and
    os.path.isfile(QGIS_SERVER_OAUTH2_AUTHORITY) and
    QGIS_SERVER_OAUTH2_USERNAME and QGIS_SERVER_OAUTH2_PASSWORD)

HTTPS_ENABLED = QGIS_SERVER_PKI_AUTH or QGIS_SERVER_OAUTH2_AUTH


qgs_app = QgsApplication([], False)
qgs_server = QgsServer()


if QGIS_SERVER_HTTP_BASIC_AUTH:
    from qgis.server import QgsServerFilter
    import base64

    class HTTPBasicFilter(QgsServerFilter):

        def requestReady(self):
            handler = self.serverInterface().requestHandler()
            auth = self.serverInterface().requestHandler().requestHeader('HTTP_AUTHORIZATION')
            if auth:
                username, password = base64.b64decode(auth[6:]).split(b':')
                if (username.decode('utf-8') == os.environ.get('QGIS_SERVER_USERNAME', 'username') and
                        password.decode('utf-8') == os.environ.get('QGIS_SERVER_PASSWORD', 'password')):
                    return
            handler.setParameter('SERVICE', 'ACCESS_DENIED')

        def responseComplete(self):
            handler = self.serverInterface().requestHandler()
            auth = handler.requestHeader('HTTP_AUTHORIZATION')
            if auth:
                username, password = base64.b64decode(auth[6:]).split(b':')
                if (username.decode('utf-8') == os.environ.get('QGIS_SERVER_USERNAME', 'username') and
                        password.decode('utf-8') == os.environ.get('QGIS_SERVER_PASSWORD', 'password')):
                    return
            # No auth ...
            handler.clear()
            handler.setResponseHeader('Status', '401 Authorization required')
            handler.setResponseHeader(
                'WWW-Authenticate', 'Basic realm="QGIS Server"')
            handler.appendBody(b'<h1>Authorization required</h1>')

    filter = HTTPBasicFilter(qgs_server.serverInterface())
    qgs_server.serverInterface().registerFilter(filter)


def num2deg(xtile, ytile, zoom):
    """This returns the NW-corner of the square. Use the function with xtile+1 and/or ytile+1
    to get the other corners. With xtile+0.5 & ytile+0.5 it will return the center of the tile."""
    n = 2.0 ** zoom
    lon_deg = xtile / n * 360.0 - 180.0
    lat_rad = math.atan(math.sinh(math.pi * (1 - 2 * ytile / n)))
    lat_deg = math.degrees(lat_rad)
    return (lat_deg, lon_deg)


class XYZFilter(QgsServerFilter):
    """XYZ server, example: ?MAP=/path/to/projects.qgs&SERVICE=XYZ&X=1&Y=0&Z=1&LAYERS=world"""

    def requestReady(self):
        handler = self.serverInterface().requestHandler()
        if handler.parameter('SERVICE') == 'XYZ':
            x = int(handler.parameter('X'))
            y = int(handler.parameter('Y'))
            z = int(handler.parameter('Z'))
            # NW corner
            lat_deg, lon_deg = num2deg(x, y, z)
            # SE corner
            lat_deg2, lon_deg2 = num2deg(x + 1, y + 1, z)
            handler.setParameter('SERVICE', 'WMS')
            handler.setParameter('REQUEST', 'GetMap')
            handler.setParameter('VERSION', '1.3.0')
            handler.setParameter('SRS', 'EPSG:4326')
            handler.setParameter('HEIGHT', '256')
            handler.setParameter('WIDTH', '256')
            handler.setParameter('BBOX', "{},{},{},{}".format(lat_deg2, lon_deg, lat_deg, lon_deg2))


xyzfilter = XYZFilter(qgs_server.serverInterface())
qgs_server.serverInterface().registerFilter(xyzfilter)

if QGIS_SERVER_OAUTH2_AUTH:
    from qgis.server import QgsServerFilter
    from oauthlib.oauth2 import RequestValidator, LegacyApplicationServer
    import base64
    from datetime import datetime

    # Naive token storage implementation
    _tokens = {}

    class SimpleValidator(RequestValidator):
        """Validate username and password
        Note: does not support scopes or client_id"""

        def validate_client_id(self, client_id, request):
            return True

        def authenticate_client(self, request, *args, **kwargs):
            """Wide open"""
            request.client = type("Client", (), {'client_id': 'my_id'})
            return True

        def validate_user(self, username, password, client, request, *args, **kwargs):
            if username == QGIS_SERVER_OAUTH2_USERNAME and password == QGIS_SERVER_OAUTH2_PASSWORD:
                return True
            return False

        def validate_grant_type(self, client_id, grant_type, client, request, *args, **kwargs):
            # Clients should only be allowed to use one type of grant.
            return grant_type in ('password', 'refresh_token')

        def get_default_scopes(self, client_id, request, *args, **kwargs):
            # Scopes a client will authorize for if none are supplied in the
            # authorization request.
            return ('my_scope', )

        def validate_scopes(self, client_id, scopes, client, request, *args, **kwargs):
            """Wide open"""
            return True

        def save_bearer_token(self, token, request, *args, **kwargs):
            # Remember to associate it with request.scopes, request.user and
            # request.client. The two former will be set when you validate
            # the authorization code. Don't forget to save both the
            # access_token and the refresh_token and set expiration for the
            # access_token to now + expires_in seconds.
            _tokens[token['access_token']] = copy.copy(token)
            _tokens[token['access_token']]['expiration'] = datetime.now(
            ).timestamp() + int(token['expires_in'])

        def validate_bearer_token(self, token, scopes, request):
            """Check the token"""
            return token in _tokens and _tokens[token]['expiration'] > datetime.now().timestamp()

        def validate_refresh_token(self, refresh_token, client, request, *args, **kwargs):
            """Ensure the Bearer token is valid and authorized access to scopes."""
            for t in _tokens.values():
                if t['refresh_token'] == refresh_token:
                    return True
            return False

        def get_original_scopes(self, refresh_token, request, *args, **kwargs):
            """Get the list of scopes associated with the refresh token."""
            return []

    validator = SimpleValidator()
    oauth_server = LegacyApplicationServer(
        validator, token_expires_in=QGIS_SERVER_OAUTH2_TOKEN_EXPIRES_IN)

    class OAuth2Filter(QgsServerFilter):
        """This filter provides testing endpoint for OAuth2 Resource Owner Grant Flow

        Available endpoints:
        - /token (returns a new access_token),
                 optionally specify an expiration time in seconds with ?ttl=<int>
        - /refresh (returns a new access_token from a refresh token),
                 optionally specify an expiration time in seconds with ?ttl=<int>
        - /result (check the Bearer token and returns a short sentence if it validates)
        """

        def responseComplete(self):

            handler = self.serverInterface().requestHandler()

            def _token(ttl):
                """Common code for new and refresh token"""
                handler.clear()
                body = bytes(handler.data()).decode('utf8')
                old_expires_in = oauth_server.default_token_type.expires_in
                # Hacky way to dynamically set token expiration time
                oauth_server.default_token_type.expires_in = ttl
                headers, payload, code = oauth_server.create_token_response(
                    '/token', 'post', body, {})
                oauth_server.default_token_type.expires_in = old_expires_in
                for k, v in headers.items():
                    handler.setResponseHeader(k, v)
                handler.setStatusCode(code)
                handler.appendBody(payload.encode('utf-8'))

            # Token expiration
            ttl = handler.parameterMap().get('TTL', QGIS_SERVER_OAUTH2_TOKEN_EXPIRES_IN)
            # Issue a new token
            if handler.url().find('/token') != -1:
                _token(ttl)
                return

            # Refresh token
            if handler.url().find('/refresh') != -1:
                _token(ttl)
                return

            # Check for valid token
            auth = handler.requestHeader('HTTP_AUTHORIZATION')
            if auth:
                result, response = oauth_server.verify_request(
                    urllib.parse.quote_plus(handler.url(), safe='/:?=&'), 'post', '', {'Authorization': auth})
                if result:
                    # This is a test endpoint for OAuth2, it requires a valid
                    # token
                    if handler.url().find('/result') != -1:
                        handler.clear()
                        handler.appendBody(b'Valid Token: enjoy OAuth2')
                    # Standard flow
                    return
                else:
                    # Wrong token, default response 401
                    pass

            # No auth ...
            handler.clear()
            handler.setStatusCode(401)
            handler.setResponseHeader('Status', '401 Unauthorized')
            handler.setResponseHeader(
                'WWW-Authenticate', 'Bearer realm="QGIS Server"')
            handler.appendBody(b'Invalid Token: Authorization required.')

    filter = OAuth2Filter(qgs_server.serverInterface())
    qgs_server.serverInterface().registerFilter(filter)


class Handler(BaseHTTPRequestHandler):

    def do_GET(self, post_body=None):
        # CGI vars:
        headers = {}
        for k, v in self.headers.items():
            headers['HTTP_%s' % k.replace(' ', '-').replace('-', '_').replace(' ', '-').upper()] = v
        if not self.path.startswith('http'):
            self.path = "%s://%s:%s%s" % ('https' if HTTPS_ENABLED else 'http', QGIS_SERVER_HOST, self.server.server_port, self.path)
        request = QgsBufferServerRequest(
            self.path, (QgsServerRequest.PostMethod if post_body is not None else QgsServerRequest.GetMethod), headers, post_body)
        response = QgsBufferServerResponse()
        qgs_server.handleRequest(request, response)

        headers_dict = response.headers()
        try:
            self.send_response(int(headers_dict['Status'].split(' ')[0]))
        except:
            self.send_response(200)
        for k, v in headers_dict.items():
            self.send_header(k, v)
        self.end_headers()
        self.wfile.write(response.body())
        return

    def do_POST(self):
        content_len = int(self.headers.get('content-length', 0))
        post_body = self.rfile.read(content_len)
        return self.do_GET(post_body)


class ThreadedHTTPServer(ThreadingMixIn, HTTPServer):
    """Handle requests in a separate thread."""
    pass


if __name__ == '__main__':
    if os.environ.get('MULTITHREADING') == '1':
        server = ThreadedHTTPServer((QGIS_SERVER_HOST, QGIS_SERVER_PORT), Handler)
    else:
        server = HTTPServer((QGIS_SERVER_HOST, QGIS_SERVER_PORT), Handler)
    # HTTPS is enabled if any of PKI or OAuth2 are enabled too
    if HTTPS_ENABLED:
        if QGIS_SERVER_OAUTH2_AUTH:
            server.socket = ssl.wrap_socket(
                server.socket,
                certfile=QGIS_SERVER_OAUTH2_CERTIFICATE,
                ca_certs=QGIS_SERVER_OAUTH2_AUTHORITY,
                keyfile=QGIS_SERVER_OAUTH2_KEY,
                server_side=True,
                # cert_reqs=ssl.CERT_REQUIRED,  # No certs for OAuth2
                ssl_version=ssl.PROTOCOL_TLSv1)
        else:
            server.socket = ssl.wrap_socket(
                server.socket,
                certfile=QGIS_SERVER_PKI_CERTIFICATE,
                keyfile=QGIS_SERVER_PKI_KEY,
                ca_certs=QGIS_SERVER_PKI_AUTHORITY,
                cert_reqs=ssl.CERT_REQUIRED,
                server_side=True,
                ssl_version=ssl.PROTOCOL_TLSv1)

    print('Starting server on %s://%s:%s, use <Ctrl-C> to stop' %
          ('https' if HTTPS_ENABLED else 'http', QGIS_SERVER_HOST, server.server_port), flush=True)

    def signal_handler(signal, frame):
        global qgs_app
        print("\nExiting QGIS...")
        qgs_app.exitQgis()
        sys.exit(0)

    signal.signal(signal.SIGINT, signal_handler)
    server.serve_forever()
