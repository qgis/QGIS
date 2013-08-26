#!/usr/bin/env python
"""
################################################################################
# 2013-09-01  Larry Shaffer <larrys@dakotacarto.com>
#
# NOTE: THIS IS ONLY A DEVELOPMENT TESTING SCRIPT. TO BE REMOVED AT LATER DATE.
################################################################################

<description>

This file is part of ZTC and distributed under the same license.
http://bitbucket.org/rvs/ztc/

Copyright (c) 2011 Vladimir Rusinov <vladimir@greenmice.info>
"""

import flup_fcgi_client as fcgi_client
# from flup.client.fcgi_app import FCGIApp as fcgi_client

def load_page(fcgi_host,fcgi_port, script, query):
    """ load fastcgi page """
    try:
        fcgi = fcgi_client.FCGIApp(host = fcgi_host,
                                   port = fcgi_port)
        env = {
           'SCRIPT_FILENAME': script,
           'QUERY_STRING': query,
           'REQUEST_METHOD': 'GET',
           'SCRIPT_NAME': script,
           'REQUEST_URI': script + '?' + query,
           'GATEWAY_INTERFACE': 'CGI/1.1',
           'SERVER_SOFTWARE': '',
           'REDIRECT_STATUS': '200',
           'CONTENT_TYPE': '',
           'CONTENT_LENGTH': '0',
           'DOCUMENT_ROOT': '/qgisserver/',
           'SERVER_ADDR': fcgi_host,
           'SERVER_PORT': str(fcgi_port),
           'SERVER_PROTOCOL': 'HTTP/1.0',
           'SERVER_NAME': fcgi_host
           }
        ret = fcgi(env)
        return ret
    except:
        print 'fastcgi load failed'
        return '500', [], '', ''


if __name__ == "__main__":
    fcgi_host = '127.0.0.1'
    fcgi_port = '8448'
    script = 'qgis_mapserv.fcgi'
    query = 'SERVICE=WMS&VERSION=1.3.0&REQUEST=GetCapabilities&MAP=/test-projects/tests/pal_test.qgs'
    print load_page(fcgi_host,fcgi_port, script, query)[2]
