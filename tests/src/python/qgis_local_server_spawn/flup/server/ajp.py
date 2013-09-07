"""
.. highlight:: python
   :linenothreshold: 5

.. highlight:: bash
   :linenothreshold: 5

ajp - an AJP 1.3/WSGI gateway.

:copyright: Copyright (c) 2005, 2006 Allan Saddi <allan@saddi.com>
  All rights reserved.
:license:

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS **AS IS** AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 SUCH DAMAGE.

For more information about AJP and AJP connectors for your web server, see
http://jakarta.apache.org/tomcat/connectors-doc/.

For more information about the Web Server Gateway Interface, see
http://www.python.org/peps/pep-0333.html.

Example usage::

  #!/usr/bin/env python
  import sys
  from myapplication import app # Assume app is your WSGI application object
  from ajp import WSGIServer
  ret = WSGIServer(app).run()
  sys.exit(ret and 42 or 0)

See the documentation for WSGIServer for more information.

About the bit of logic at the end:
Upon receiving SIGHUP, the python script will exit with status code 42. This
can be used by a wrapper script to determine if the python script should be
re-run. When a SIGINT or SIGTERM is received, the script exits with status
code 0, possibly indicating a normal exit.

Example wrapper script::

  #!/bin/sh
  STATUS=42
  while test $STATUS -eq 42; do
    python "$@" that_script_above.py
    STATUS=$?
  done

Example workers.properties (for mod_jk)::

  worker.list=foo
  worker.foo.port=8009
  worker.foo.host=localhost
  worker.foo.type=ajp13

Example httpd.conf (for mod_jk)::

  JkWorkersFile /path/to/workers.properties
  JkMount /* foo

Note that if you mount your ajp application anywhere but the root ("/"), you
SHOULD specifiy scriptName to the WSGIServer constructor. This will ensure
that SCRIPT_NAME/PATH_INFO are correctly deduced.
"""

__author__ = 'Allan Saddi <allan@saddi.com>'
__version__ = '$Revision$'

import socket
import logging

from flup.server.ajp_base import BaseAJPServer, Connection
from flup.server.threadedserver import ThreadedServer

__all__ = ['WSGIServer']

class WSGIServer(BaseAJPServer, ThreadedServer):
    """
    AJP1.3/WSGI server. Runs your WSGI application as a persistant program
    that understands AJP1.3. Opens up a TCP socket, binds it, and then
    waits for forwarded requests from your webserver.

    Why AJP? Two good reasons are that AJP provides load-balancing and
    fail-over support. Personally, I just wanted something new to
    implement. :)

    Of course you will need an AJP1.3 connector for your webserver (e.g.
    mod_jk) - see http://jakarta.apache.org/tomcat/connectors-doc/.
    """
    def __init__(self, application, scriptName='', environ=None,
                 multithreaded=True, multiprocess=False,
                 bindAddress=('localhost', 8009), allowedServers=None,
                 loggingLevel=logging.INFO, debug=False, **kw):
        """
        scriptName is the initial portion of the URL path that "belongs"
        to your application. It is used to determine PATH_INFO (which doesn't
        seem to be passed in). An empty scriptName means your application
        is mounted at the root of your virtual host.

        environ, which must be a dictionary, can contain any additional
        environment variables you want to pass to your application.

        bindAddress is the address to bind to, which must be a tuple of
        length 2. The first element is a string, which is the host name
        or IPv4 address of a local interface. The 2nd element is the port
        number.

        allowedServers must be None or a list of strings representing the
        IPv4 addresses of servers allowed to connect. None means accept
        connections from anywhere.

        loggingLevel sets the logging level of the module-level logger.
        """
        BaseAJPServer.__init__(self, application,
                               scriptName=scriptName,
                               environ=environ,
                               multithreaded=multithreaded,
                               multiprocess=multiprocess,
                               bindAddress=bindAddress,
                               allowedServers=allowedServers,
                               loggingLevel=loggingLevel,
                               debug=debug)
        for key in ('jobClass', 'jobArgs'):
            if kw.has_key(key):
                del kw[key]
        ThreadedServer.__init__(self, jobClass=Connection,
                                jobArgs=(self, None), **kw)

    def run(self):
        """
        Main loop. Call this after instantiating WSGIServer. SIGHUP, SIGINT,
        SIGQUIT, SIGTERM cause it to cleanup and return. (If a SIGHUP
        is caught, this method returns True. Returns False otherwise.)
        """
        self.logger.info('%s starting up', self.__class__.__name__)

        try:
            sock = self._setupSocket()
        except socket.error, e:
            self.logger.error('Failed to bind socket (%s), exiting', e[1])
            return False

        ret = ThreadedServer.run(self, sock)

        self._cleanupSocket(sock)
        # AJP connections are more or less persistent. .shutdown() will
        # not return until the web server lets go. So don't bother calling
        # it...
        #self.shutdown()

        self.logger.info('%s shutting down%s', self.__class__.__name__,
                         self._hupReceived and ' (reload requested)' or '')

        return ret

if __name__ == '__main__':
    def test_app(environ, start_response):
        """Probably not the most efficient example."""
        import cgi
        start_response('200 OK', [('Content-Type', 'text/html')])
        yield '<html><head><title>Hello World!</title></head>\n' \
              '<body>\n' \
              '<p>Hello World!</p>\n' \
              '<table border="1">'
        names = environ.keys()
        names.sort()
        for name in names:
            yield '<tr><td>%s</td><td>%s</td></tr>\n' % (
                name, cgi.escape(`environ[name]`))

        form = cgi.FieldStorage(fp=environ['wsgi.input'], environ=environ,
                                keep_blank_values=1)
        if form.list:
            yield '<tr><th colspan="2">Form data</th></tr>'

        for field in form.list:
            yield '<tr><td>%s</td><td>%s</td></tr>\n' % (
                field.name, field.value)

        yield '</table>\n' \
              '</body></html>\n'

    from wsgiref import validate
    test_app = validate.validator(test_app)
    # Explicitly set bindAddress to *:8009 for testing.
    WSGIServer(test_app,
               bindAddress=('', 8009), allowedServers=None,
               loggingLevel=logging.DEBUG).run()
