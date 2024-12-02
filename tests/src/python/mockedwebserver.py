#!/usr/bin/env python
###############################################################################
# $Id$
#
# Project:  Adapted from GDAL/OGR Test Suite
# Purpose:  Fake HTTP server
# Author:   Even Rouault <even dot rouault at spatialys.com>
#
###############################################################################
# Copyright (c) 2010-2020, Even Rouault <even dot rouault at spatialys.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
###############################################################################

import contextlib
import sys
import time
import tempfile

from http.server import BaseHTTPRequestHandler, HTTPServer
from threading import Thread

do_log = False
custom_handler = None


@contextlib.contextmanager
def install_http_handler(handler_instance):
    global custom_handler
    custom_handler = handler_instance
    try:
        yield
    finally:
        handler_instance.final_check()
        custom_handler = None


class RequestResponse:
    def __init__(
        self,
        method,
        path,
        code,
        headers=None,
        body=None,
        custom_method=None,
        expected_headers=None,
        expected_body=None,
        add_content_length_header=True,
        unexpected_headers=[],
    ):
        self.method = method
        self.path = path
        self.code = code
        self.headers = {} if headers is None else headers
        self.body = body
        self.custom_method = custom_method
        self.expected_headers = {} if expected_headers is None else expected_headers
        self.expected_body = expected_body
        self.add_content_length_header = add_content_length_header
        self.unexpected_headers = unexpected_headers


class SequentialHandler:
    def __init__(self):
        self.req_count = 0
        self.req_resp = []
        self.unexpected = False

    def final_check(self):
        assert not self.unexpected
        assert self.req_count == len(self.req_resp), (
            self.req_count,
            len(self.req_resp),
        )

    def add(
        self,
        method,
        path,
        code=None,
        headers=None,
        body=None,
        custom_method=None,
        expected_headers=None,
        expected_body=None,
        add_content_length_header=True,
        unexpected_headers=[],
    ):
        hdrs = {} if headers is None else headers
        expected_hdrs = {} if expected_headers is None else expected_headers
        req = RequestResponse(
            method,
            path,
            code,
            hdrs,
            body,
            custom_method,
            expected_hdrs,
            expected_body,
            add_content_length_header,
            unexpected_headers,
        )
        self.req_resp.append(req)
        return req

    def _process_req_resp(self, req_resp, request):
        if req_resp.custom_method:
            req_resp.custom_method(request)
        else:

            if req_resp.expected_headers:
                for k in req_resp.expected_headers:
                    if (
                        k not in request.headers
                        or request.headers[k] != req_resp.expected_headers[k]
                    ):
                        sys.stderr.write(
                            "Did not get expected headers: %s\n" % str(request.headers)
                        )
                        request.send_response(400)
                        request.send_header("Content-Length", 0)
                        request.end_headers()
                        self.unexpected = True
                        return

            for k in req_resp.unexpected_headers:
                if k in request.headers:
                    sys.stderr.write("Did not expect header: %s\n" % k)
                    request.send_response(400)
                    request.send_header("Content-Length", 0)
                    request.end_headers()
                    self.unexpected = True
                    return

            if req_resp.expected_body:
                content = request.rfile.read(int(request.headers["Content-Length"]))
                if content != req_resp.expected_body:
                    sys.stderr.write("Did not get expected content: %s\n" % content)
                    request.send_response(400)
                    request.send_header("Content-Length", 0)
                    request.end_headers()
                    self.unexpected = True
                    return

            request.send_response(req_resp.code)
            for k in req_resp.headers:
                request.send_header(k, req_resp.headers[k])
            if req_resp.add_content_length_header:
                if req_resp.body:
                    request.send_header("Content-Length", len(req_resp.body))
                elif "Content-Length" not in req_resp.headers:
                    request.send_header("Content-Length", "0")
            request.end_headers()
            if req_resp.body:
                try:
                    request.wfile.write(req_resp.body)
                except:
                    request.wfile.write(req_resp.body.encode("ascii"))

    def process(self, method, request):
        if self.req_count < len(self.req_resp):
            req_resp = self.req_resp[self.req_count]
            if method == req_resp.method and request.path == req_resp.path:
                self.req_count += 1
                self._process_req_resp(req_resp, request)
                return

        request.send_error(
            500,
            "Unexpected %s request for %s, req_count = %d"
            % (method, request.path, self.req_count),
        )
        self.unexpected = True

    def do_HEAD(self, request):
        self.process("HEAD", request)

    def do_GET(self, request):
        self.process("GET", request)

    def do_POST(self, request):
        self.process("POST", request)

    def do_PUT(self, request):
        self.process("PUT", request)

    def do_DELETE(self, request):
        self.process("DELETE", request)


class DispatcherHttpHandler(BaseHTTPRequestHandler):

    # protocol_version = 'HTTP/1.1'

    def log_request(self, code="-", size="-"):
        # pylint: disable=unused-argument
        pass

    def do_HEAD(self):

        if do_log:
            f = open(tempfile.gettempdir() + "/log.txt", "a")
            f.write("HEAD %s\n" % self.path)
            f.close()

        custom_handler.do_HEAD(self)

    def do_DELETE(self):

        if do_log:
            f = open(tempfile.gettempdir() + "/log.txt", "a")
            f.write("DELETE %s\n" % self.path)
            f.close()

        custom_handler.do_DELETE(self)

    def do_POST(self):

        if do_log:
            f = open(tempfile.gettempdir() + "/log.txt", "a")
            f.write("POST %s\n" % self.path)
            f.close()

        custom_handler.do_POST(self)

    def do_PUT(self):

        if do_log:
            f = open(tempfile.gettempdir() + "/log.txt", "a")
            f.write("PUT %s\n" % self.path)
            f.close()

        custom_handler.do_PUT(self)

    def do_GET(self):

        if do_log:
            f = open(tempfile.gettempdir() + "/log.txt", "a")
            f.write("GET %s\n" % self.path)
            f.close()

        custom_handler.do_GET(self)


class ThreadedHttpServer(Thread):

    def __init__(self, handlerClass):
        Thread.__init__(self)
        self.server = HTTPServer(("", 0), handlerClass)
        self.running = False

    def getPort(self):
        return self.server.server_address[1]

    def run(self):
        try:
            self.running = True
            self.server.serve_forever()
        except KeyboardInterrupt:
            print("^C received, shutting down server")
            self.stop()

    def start_and_wait_ready(self):
        self.start()
        while not self.running:
            time.sleep(1)

    def stop(self):
        self.server.shutdown()
        self.server.server_close()


def launch(handler=None):
    if handler is None:
        handler = DispatcherHttpHandler
    server = ThreadedHttpServer(handler)
    server.start_and_wait_ready()
    return server, server.getPort()


@contextlib.contextmanager
def install_http_server(handler=None):
    server, port = launch(handler)
    try:
        yield port
    finally:
        server.stop()
