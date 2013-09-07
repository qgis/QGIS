# Copyright (c) 2005, 2006 Allan Saddi <allan@saddi.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
# $Id$

__author__ = 'Allan Saddi <allan@saddi.com>'
__version__ = '$Revision$'

import sys
import socket
import select
import struct
import signal
import logging
import errno
import datetime
import time
import traceback

# Unfortunately, for now, threads are required.
import thread
import threading

from flup.server import NoDefault

__all__ = ['BaseAJPServer']

# Packet header prefixes.
SERVER_PREFIX = '\x12\x34'
CONTAINER_PREFIX = 'AB'

# Server packet types.
PKTTYPE_FWD_REQ = '\x02'
PKTTYPE_SHUTDOWN = '\x07'
PKTTYPE_PING = '\x08'
PKTTYPE_CPING = '\x0a'

# Container packet types.
PKTTYPE_SEND_BODY = '\x03'
PKTTYPE_SEND_HEADERS = '\x04'
PKTTYPE_END_RESPONSE = '\x05'
PKTTYPE_GET_BODY = '\x06'
PKTTYPE_CPONG = '\x09'

# Code tables for methods/headers/attributes.
methodTable = [
    None,
    'OPTIONS',
    'GET',
    'HEAD',
    'POST',
    'PUT',
    'DELETE',
    'TRACE',
    'PROPFIND',
    'PROPPATCH',
    'MKCOL',
    'COPY',
    'MOVE',
    'LOCK',
    'UNLOCK',
    'ACL',
    'REPORT',
    'VERSION-CONTROL',
    'CHECKIN',
    'CHECKOUT',
    'UNCHECKOUT',
    'SEARCH',
    'MKWORKSPACE',
    'UPDATE',
    'LABEL',
    'MERGE',
    'BASELINE_CONTROL',
    'MKACTIVITY'
    ]

requestHeaderTable = [
    None,
    'Accept',
    'Accept-Charset',
    'Accept-Encoding',
    'Accept-Language',
    'Authorization',
    'Connection',
    'Content-Type',
    'Content-Length',
    'Cookie',
    'Cookie2',
    'Host',
    'Pragma',
    'Referer',
    'User-Agent'
    ]

attributeTable = [
    None,
    'CONTEXT',
    'SERVLET_PATH',
    'REMOTE_USER',
    'AUTH_TYPE',
    'QUERY_STRING',
    'JVM_ROUTE',
    'SSL_CERT',
    'SSL_CIPHER',
    'SSL_SESSION',
    None, # name follows
    'SSL_KEY_SIZE'
    ]

responseHeaderTable = [
    None,
    'content-type',
    'content-language',
    'content-length',
    'date',
    'last-modified',
    'location',
    'set-cookie',
    'set-cookie2',
    'servlet-engine',
    'status',
    'www-authenticate'
    ]

# The main classes use this name for logging.
LoggerName = 'ajp-wsgi'

# Set up module-level logger.
console = logging.StreamHandler()
console.setLevel(logging.DEBUG)
console.setFormatter(logging.Formatter('%(asctime)s : %(message)s',
                                       '%Y-%m-%d %H:%M:%S'))
logging.getLogger(LoggerName).addHandler(console)
del console

class ProtocolError(Exception):
    """
    Exception raised when the server does something unexpected or
    sends garbled data. Usually leads to a Connection closing.
    """
    pass

def decodeString(data, pos=0):
    """Decode a string."""
    try:
        length = struct.unpack('>H', data[pos:pos+2])[0]
        pos += 2
        if length == 0xffff: # This was undocumented!
            return '', pos
        s = data[pos:pos+length]
        return s, pos+length+1 # Don't forget NUL
    except Exception, e:
        raise ProtocolError, 'decodeString: '+str(e)

def decodeRequestHeader(data, pos=0):
    """Decode a request header/value pair."""
    try:
        if data[pos] == '\xa0':
            # Use table
            i = ord(data[pos+1])
            name = requestHeaderTable[i]
            if name is None:
                raise ValueError, 'bad request header code'
            pos += 2
        else:
            name, pos = decodeString(data, pos)
        value, pos = decodeString(data, pos)
        return name, value, pos
    except Exception, e:
        raise ProtocolError, 'decodeRequestHeader: '+str(e)

def decodeAttribute(data, pos=0):
    """Decode a request attribute."""
    try:
        i = ord(data[pos])
        pos += 1
        if i == 0xff:
            # end
            return None, None, pos
        elif i == 0x0a:
            # name follows
            name, pos = decodeString(data, pos)
        elif i == 0x0b:
            # Special handling of SSL_KEY_SIZE.
            name = attributeTable[i]
            # Value is an int, not a string.
            value = struct.unpack('>H', data[pos:pos+2])[0]
            return name, str(value), pos+2
        else:
            name = attributeTable[i]
            if name is None:
                raise ValueError, 'bad attribute code'
        value, pos = decodeString(data, pos)
        return name, value, pos
    except Exception, e:
        raise ProtocolError, 'decodeAttribute: '+str(e)

def encodeString(s):
    """Encode a string."""
    return struct.pack('>H', len(s)) + s + '\x00'

def encodeResponseHeader(name, value):
    """Encode a response header/value pair."""
    lname = name.lower()
    if lname in responseHeaderTable:
        # Use table
        i = responseHeaderTable.index(lname)
        out = '\xa0' + chr(i)
    else:
        out = encodeString(name)
    out += encodeString(value)
    return out

class Packet(object):
    """An AJP message packet."""
    def __init__(self):
        self.data = ''
        # Don't set this on write, it will be calculated automatically.
        self.length = 0

    def _recvall(sock, length):
        """
        Attempts to receive length bytes from a socket, blocking if necessary.
        (Socket may be blocking or non-blocking.)
        """
        dataList = []
        recvLen = 0
        while length:
            try:
                data = sock.recv(length)
            except socket.error, e:
                if e[0] == errno.EAGAIN:
                    select.select([sock], [], [])
                    continue
                else:
                    raise
            if not data: # EOF
                break
            dataList.append(data)
            dataLen = len(data)
            recvLen += dataLen
            length -= dataLen
        return ''.join(dataList), recvLen
    _recvall = staticmethod(_recvall)

    def read(self, sock):
        """Attempt to read a packet from the server."""
        try:
            header, length = self._recvall(sock, 4)
        except socket.error:
            # Treat any sort of socket errors as EOF (close Connection).
            raise EOFError

        if length < 4:
            raise EOFError

        if header[:2] != SERVER_PREFIX:
            raise ProtocolError, 'invalid header'

        self.length = struct.unpack('>H', header[2:4])[0]
        if self.length:
            try:
                self.data, length = self._recvall(sock, self.length)
            except socket.error:
                raise EOFError

            if length < self.length:
                raise EOFError

    def _sendall(sock, data):
        """
        Writes data to a socket and does not return until all the data is sent.
        """
        length = len(data)
        while length:
            try:
                sent = sock.send(data)
            except socket.error, e:
                if e[0] == errno.EAGAIN:
                    select.select([], [sock], [])
                    continue
                else:
                    raise
            data = data[sent:]
            length -= sent
    _sendall = staticmethod(_sendall)

    def write(self, sock):
        """Send a packet to the server."""
        self.length = len(self.data)
        self._sendall(sock, CONTAINER_PREFIX + struct.pack('>H', self.length))
        if self.length:
            self._sendall(sock, self.data)

class InputStream(object):
    """
    File-like object that represents the request body (if any). Supports
    the bare mininum methods required by the WSGI spec. Thanks to
    StringIO for ideas.
    """
    def __init__(self, conn):
        self._conn = conn

        # See WSGIServer.
        self._shrinkThreshold = conn.server.inputStreamShrinkThreshold

        self._buf = ''
        self._bufList = []
        self._pos = 0 # Current read position.
        self._avail = 0 # Number of bytes currently available.
        self._length = 0 # Set to Content-Length in request.

        self.logger = logging.getLogger(LoggerName)

    def bytesAvailForAdd(self):
        return self._length - self._avail

    def _shrinkBuffer(self):
        """Gets rid of already read data (since we can't rewind)."""
        if self._pos >= self._shrinkThreshold:
            self._buf = self._buf[self._pos:]
            self._avail -= self._pos
            self._length -= self._pos
            self._pos = 0

            assert self._avail >= 0 and self._length >= 0

    def _waitForData(self):
        toAdd = min(self.bytesAvailForAdd(), 0xffff)
        assert toAdd > 0
        pkt = Packet()
        pkt.data = PKTTYPE_GET_BODY + \
                   struct.pack('>H', toAdd)
        self._conn.writePacket(pkt)
        self._conn.processInput()

    def read(self, n=-1):
        if self._pos == self._length:
            return ''
        while True:
            if n < 0 or (self._avail - self._pos) < n:
                # Not enough data available.
                if not self.bytesAvailForAdd():
                    # And there's no more coming.
                    newPos = self._avail
                    break
                else:
                    # Ask for more data and wait.
                    self._waitForData()
                    continue
            else:
                newPos = self._pos + n
                break
        # Merge buffer list, if necessary.
        if self._bufList:
            self._buf += ''.join(self._bufList)
            self._bufList = []
        r = self._buf[self._pos:newPos]
        self._pos = newPos
        self._shrinkBuffer()
        return r

    def readline(self, length=None):
        if self._pos == self._length:
            return ''
        while True:
            # Unfortunately, we need to merge the buffer list early.
            if self._bufList:
                self._buf += ''.join(self._bufList)
                self._bufList = []
            # Find newline.
            i = self._buf.find('\n', self._pos)
            if i < 0:
                # Not found?
                if not self.bytesAvailForAdd():
                    # No more data coming.
                    newPos = self._avail
                    break
                else:
                    if length is not None and len(self._buf) >= length + self._pos:
                        newPos = self._pos + length
                        break
                    # Wait for more to come.
                    self._waitForData()
                    continue
            else:
                newPos = i + 1
                break
        r = self._buf[self._pos:newPos]
        self._pos = newPos
        self._shrinkBuffer()
        return r

    def readlines(self, sizehint=0):
        total = 0
        lines = []
        line = self.readline()
        while line:
            lines.append(line)
            total += len(line)
            if 0 < sizehint <= total:
                break
            line = self.readline()
        return lines

    def __iter__(self):
        return self

    def next(self):
        r = self.readline()
        if not r:
            raise StopIteration
        return r

    def setDataLength(self, length):
        """
        Once Content-Length is known, Request calls this method to set it.
        """
        self._length = length

    def addData(self, data):
        """
        Adds data from the server to this InputStream. Note that we never ask
        the server for data beyond the Content-Length, so the server should
        never send us an EOF (empty string argument).
        """
        if not data:
            raise ProtocolError, 'short data'
        self._bufList.append(data)
        length = len(data)
        self._avail += length
        if self._avail > self._length:
            raise ProtocolError, 'too much data'

class Request(object):
    """
    A Request object. A more fitting name would probably be Transaction, but
    it's named Request to mirror my FastCGI driver. :) This object
    encapsulates all the data about the HTTP request and allows the handler
    to send a response.

    The only attributes/methods that the handler should concern itself
    with are: environ, input, startResponse(), and write().
    """
    # Do not ever change the following value.
    _maxWrite = 8192 - 4 - 3 - 1 # 8k - pkt header - send body header - NUL

    def __init__(self, conn):
        self._conn = conn

        self.environ = {}
        self.input = InputStream(conn)

        self._headersSent = False

        self.logger = logging.getLogger(LoggerName)

    def run(self):
        self.logger.info('%s %s',
                         self.environ['REQUEST_METHOD'],
                         self.environ['REQUEST_URI'])

        start = datetime.datetime.now()

        try:
            self._conn.server.handler(self)
        except:
            self.logger.exception('Exception caught from handler')
            if not self._headersSent:
                self._conn.server.error(self)

        end = datetime.datetime.now()

        # Notify server of end of response (reuse flag is set to true).
        pkt = Packet()
        pkt.data = PKTTYPE_END_RESPONSE + '\x01'
        self._conn.writePacket(pkt)

        handlerTime = end - start
        self.logger.debug('%s %s done (%.3f secs)',
                          self.environ['REQUEST_METHOD'],
                          self.environ['REQUEST_URI'],
                          handlerTime.seconds +
                          handlerTime.microseconds / 1000000.0)

    # The following methods are called from the Connection to set up this
    # Request.

    def setMethod(self, value):
        self.environ['REQUEST_METHOD'] = value

    def setProtocol(self, value):
        self.environ['SERVER_PROTOCOL'] = value

    def setRequestURI(self, value):
        self.environ['REQUEST_URI'] = value

    def setRemoteAddr(self, value):
        self.environ['REMOTE_ADDR'] = value

    def setRemoteHost(self, value):
        self.environ['REMOTE_HOST'] = value

    def setServerName(self, value):
        self.environ['SERVER_NAME'] = value

    def setServerPort(self, value):
        self.environ['SERVER_PORT'] = str(value)

    def setIsSSL(self, value):
        if value:
            self.environ['HTTPS'] = 'on'

    def addHeader(self, name, value):
        name = name.replace('-', '_').upper()
        if name in ('CONTENT_TYPE', 'CONTENT_LENGTH'):
            self.environ[name] = value
            if name == 'CONTENT_LENGTH':
                length = int(value)
                self.input.setDataLength(length)
        else:
            self.environ['HTTP_'+name] = value

    def addAttribute(self, name, value):
        self.environ[name] = value

    # The only two methods that should be called from the handler.

    def startResponse(self, statusCode, statusMsg, headers):
        """
        Begin the HTTP response. This must only be called once and it
        must be called before any calls to write().

        statusCode is the integer status code (e.g. 200). statusMsg
        is the associated reason message (e.g.'OK'). headers is a list
        of 2-tuples - header name/value pairs. (Both header name and value
        must be strings.)
        """
        assert not self._headersSent, 'Headers already sent!'

        pkt = Packet()
        pkt.data = PKTTYPE_SEND_HEADERS + \
                   struct.pack('>H', statusCode) + \
                   encodeString(statusMsg) + \
                   struct.pack('>H', len(headers)) + \
                   ''.join([encodeResponseHeader(name, value)
                            for name,value in headers])

        self._conn.writePacket(pkt)

        self._headersSent = True

    def write(self, data):
        """
        Write data (which comprises the response body). Note that due to
        restrictions on AJP packet size, we limit our writes to 8185 bytes
        each packet.
        """
        assert self._headersSent, 'Headers must be sent first!'

        bytesLeft = len(data)
        while bytesLeft:
            toWrite = min(bytesLeft, self._maxWrite)

            pkt = Packet()
            pkt.data = PKTTYPE_SEND_BODY + \
                       struct.pack('>H', toWrite) + \
                       data[:toWrite] + '\x00' # Undocumented
            self._conn.writePacket(pkt)

            data = data[toWrite:]
            bytesLeft -= toWrite

class TimeoutException(Exception):
    pass

class Connection(object):
    """
    A single Connection with the server. Requests are not multiplexed over the
    same connection, so at any given time, the Connection is either
    waiting for a request, or processing a single request.
    """
    def __init__(self, sock, addr, server, timeout):
        self.server = server
        self._sock = sock
        self._addr = addr
        self._timeout = timeout

        self._request = None

        self.logger = logging.getLogger(LoggerName)

    def timeout_handler(self, signum, frame):
        self.logger.error('Timeout Exceeded')
        self.logger.error("\n".join(traceback.format_stack(frame)))

        raise TimeoutException

    def run(self):
        self.logger.debug('Connection starting up (%s:%d)',
                          self._addr[0], self._addr[1])

        # Main loop. Errors will cause the loop to be exited and
        # the socket to be closed.
        while True:
            try:
                self.processInput()
            except ProtocolError, e:
                self.logger.error("Protocol error '%s'", str(e))
                break
            except (EOFError, KeyboardInterrupt):
                break
            except:
                self.logger.exception('Exception caught in Connection')
                break

        self.logger.debug('Connection shutting down (%s:%d)',
                          self._addr[0], self._addr[1])

        self._sock.close()

    def processInput(self):
        """Wait for and process a single packet."""
        pkt = Packet()
        select.select([self._sock], [], [])
        pkt.read(self._sock)

        # Body chunks have no packet type code.
        if self._request is not None:
            self._processBody(pkt)
            return

        if not pkt.length:
            raise ProtocolError, 'unexpected empty packet'

        pkttype = pkt.data[0]
        if pkttype == PKTTYPE_FWD_REQ:
            self._forwardRequest(pkt)
        elif pkttype == PKTTYPE_SHUTDOWN:
            self._shutdown(pkt)
        elif pkttype == PKTTYPE_PING:
            self._ping(pkt)
        elif pkttype == PKTTYPE_CPING:
            self._cping(pkt)
        else:
            raise ProtocolError, 'unknown packet type'

    def _forwardRequest(self, pkt):
        """
        Creates a Request object, fills it in from the packet, then runs it.
        """
        assert self._request is None

        req = self.server.requestClass(self)
        i = ord(pkt.data[1])
        method = methodTable[i]
        if method is None:
            raise ValueError, 'bad method field'
        req.setMethod(method)
        value, pos = decodeString(pkt.data, 2)
        req.setProtocol(value)
        value, pos = decodeString(pkt.data, pos)
        req.setRequestURI(value)
        value, pos = decodeString(pkt.data, pos)
        req.setRemoteAddr(value)
        value, pos = decodeString(pkt.data, pos)
        req.setRemoteHost(value)
        value, pos = decodeString(pkt.data, pos)
        req.setServerName(value)
        value = struct.unpack('>H', pkt.data[pos:pos+2])[0]
        req.setServerPort(value)
        i = ord(pkt.data[pos+2])
        req.setIsSSL(i != 0)

        # Request headers.
        numHeaders = struct.unpack('>H', pkt.data[pos+3:pos+5])[0]
        pos += 5
        for i in range(numHeaders):
            name, value, pos = decodeRequestHeader(pkt.data, pos)
            req.addHeader(name, value)

        # Attributes.
        while True:
            name, value, pos = decodeAttribute(pkt.data, pos)
            if name is None:
                break
            req.addAttribute(name, value)

        self._request = req

        # Read first body chunk, if needed.
        if req.input.bytesAvailForAdd():
            self.processInput()

        # If there is a timeout
        if self._timeout:
            old_alarm = signal.signal(signal.SIGALRM, self.timeout_handler)
            signal.alarm(self._timeout)

        # Run Request.
        req.run()

        self._request = None

        # Restore old handler if timeout was given
        if self._timeout:
            signal.alarm(0)
            signal.signal(signal.SIGALRM, old_alarm)

    def _shutdown(self, pkt):
        """Not sure what to do with this yet."""
        self.logger.info('Received shutdown request from server')

    def _ping(self, pkt):
        """I have no idea what this packet means."""
        self.logger.debug('Received ping')

    def _cping(self, pkt):
        """Respond to a PING (CPING) packet."""
        self.logger.debug('Received PING, sending PONG')
        pkt = Packet()
        pkt.data = PKTTYPE_CPONG
        self.writePacket(pkt)

    def _processBody(self, pkt):
        """
        Handles a body chunk from the server by appending it to the
        InputStream.
        """
        if pkt.length:
            length = struct.unpack('>H', pkt.data[:2])[0]
            self._request.input.addData(pkt.data[2:2+length])
        else:
            # Shouldn't really ever get here.
            self._request.input.addData('')

    def writePacket(self, pkt):
        """Sends a Packet to the server."""
        pkt.write(self._sock)

class BaseAJPServer(object):
    # What Request class to use.
    requestClass = Request

    # Limits the size of the InputStream's string buffer to this size + 8k.
    # Since the InputStream is not seekable, we throw away already-read
    # data once this certain amount has been read. (The 8k is there because
    # it is the maximum size of new data added per chunk.)
    inputStreamShrinkThreshold = 102400 - 8192

    def __init__(self, application, scriptName='', environ=None,
                 multithreaded=True, multiprocess=False,
                 bindAddress=('localhost', 8009), allowedServers=NoDefault,
                 loggingLevel=logging.INFO, debug=False):
        """
        scriptName is the initial portion of the URL path that "belongs"
        to your application. It is used to determine PATH_INFO (which doesn't
        seem to be passed in). An empty scriptName means your application
        is mounted at the root of your virtual host.

        environ, which must be a dictionary, can contain any additional
        environment variables you want to pass to your application.

        Set multithreaded to False if your application is not thread-safe.

        Set multiprocess to True to explicitly set wsgi.multiprocess to
        True. (Only makes sense with threaded servers.)

        bindAddress is the address to bind to, which must be a tuple of
        length 2. The first element is a string, which is the host name
        or IPv4 address of a local interface. The 2nd element is the port
        number.

        allowedServers must be None or a list of strings representing the
        IPv4 addresses of servers allowed to connect. None means accept
        connections from anywhere. By default, it is a list containing
        the single item '127.0.0.1'.

        loggingLevel sets the logging level of the module-level logger.
        """
        if environ is None:
            environ = {}

        self.application = application
        self.scriptName = scriptName
        self.environ = environ
        self.multithreaded = multithreaded
        self.multiprocess = multiprocess
        self.debug = debug
        self._bindAddress = bindAddress
        if allowedServers is NoDefault:
            allowedServers = ['127.0.0.1']
        self._allowedServers = allowedServers

        # Used to force single-threadedness.
        self._appLock = thread.allocate_lock()

        self.logger = logging.getLogger(LoggerName)
        self.logger.setLevel(loggingLevel)

    def _setupSocket(self):
        """Creates and binds the socket for communication with the server."""
        sock = socket.socket()
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind(self._bindAddress)
        sock.listen(socket.SOMAXCONN)
        return sock

    def _cleanupSocket(self, sock):
        """Closes the main socket."""
        sock.close()

    def _isClientAllowed(self, addr):
        ret = self._allowedServers is None or addr[0] in self._allowedServers
        if not ret:
            self.logger.warning('Server connection from %s disallowed',
                                addr[0])
        return ret

    def handler(self, request):
        """
        WSGI handler. Sets up WSGI environment, calls the application,
        and sends the application's response.
        """
        environ = request.environ
        environ.update(self.environ)

        environ['wsgi.version'] = (1,0)
        environ['wsgi.input'] = request.input
        environ['wsgi.errors'] = sys.stderr
        environ['wsgi.multithread'] = self.multithreaded
        environ['wsgi.multiprocess'] = self.multiprocess
        environ['wsgi.run_once'] = False

        if environ.get('HTTPS', 'off') in ('on', '1'):
            environ['wsgi.url_scheme'] = 'https'
        else:
            environ['wsgi.url_scheme'] = 'http'

        self._sanitizeEnv(environ)

        headers_set = []
        headers_sent = []
        result = None

        def write(data):
            assert type(data) is str, 'write() argument must be string'
            assert headers_set, 'write() before start_response()'

            if not headers_sent:
                status, responseHeaders = headers_sent[:] = headers_set
                statusCode = int(status[:3])
                statusMsg = status[4:]
                found = False
                for header,value in responseHeaders:
                    if header.lower() == 'content-length':
                        found = True
                        break
                if not found and result is not None:
                    try:
                        if len(result) == 1:
                            responseHeaders.append(('Content-Length',
                                                    str(len(data))))
                    except:
                        pass
                request.startResponse(statusCode, statusMsg, responseHeaders)

            request.write(data)

        def start_response(status, response_headers, exc_info=None):
            if exc_info:
                try:
                    if headers_sent:
                        # Re-raise if too late
                        raise exc_info[0], exc_info[1], exc_info[2]
                finally:
                    exc_info = None # avoid dangling circular ref
            else:
                assert not headers_set, 'Headers already set!'

            assert type(status) is str, 'Status must be a string'
            assert len(status) >= 4, 'Status must be at least 4 characters'
            assert int(status[:3]), 'Status must begin with 3-digit code'
            assert status[3] == ' ', 'Status must have a space after code'
            assert type(response_headers) is list, 'Headers must be a list'
            if __debug__:
                for name,val in response_headers:
                    assert type(name) is str, 'Header name "%s" must be a string' % name
                    assert type(val) is str, 'Value of header "%s" must be a string' % name

            headers_set[:] = [status, response_headers]
            return write

        if not self.multithreaded:
            self._appLock.acquire()
        try:
            try:
                result = self.application(environ, start_response)
                try:
                    for data in result:
                        if data:
                            write(data)
                    if not headers_sent:
                        write('') # in case body was empty
                finally:
                    if hasattr(result, 'close'):
                        result.close()
            except socket.error, e:
                if e[0] != errno.EPIPE:
                    raise # Don't let EPIPE propagate beyond server
        finally:
            if not self.multithreaded:
                self._appLock.release()

    def _sanitizeEnv(self, environ):
        """Fill-in/deduce missing values in environ."""
        # Namely SCRIPT_NAME/PATH_INFO
        value = environ['REQUEST_URI']
        scriptName = environ.get('WSGI_SCRIPT_NAME', self.scriptName)
        if not value.startswith(scriptName):
            self.logger.warning('scriptName does not match request URI')

        environ['PATH_INFO'] = value[len(scriptName):]
        environ['SCRIPT_NAME'] = scriptName

        reqUri = None
        if environ.has_key('REQUEST_URI'):
            reqUri = environ['REQUEST_URI'].split('?', 1)

        if not environ.has_key('QUERY_STRING') or not environ['QUERY_STRING']:
            if reqUri is not None and len(reqUri) > 1:
                environ['QUERY_STRING'] = reqUri[1]
            else:
                environ['QUERY_STRING'] = ''

    def error(self, request):
        """
        Override to provide custom error handling. Ideally, however,
        all errors should be caught at the application level.
        """
        if self.debug:
            request.startResponse(500, 'Internal Server Error',
                                  [('Content-Type', 'text/html')])
            import cgitb
            request.write(cgitb.html(sys.exc_info()))
        else:
            errorpage = """<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<html><head>
<title>Unhandled Exception</title>
</head><body>
<h1>Unhandled Exception</h1>
<p>An unhandled exception was thrown by the application.</p>
</body></html>
"""
            request.startResponse(500, 'Internal Server Error',
                                  [('Content-Type', 'text/html')])
            request.write(errorpage)
