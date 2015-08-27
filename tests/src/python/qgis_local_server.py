# -*- coding: utf-8 -*-
"""Convenience interface to a locally spawned QGIS Server, e.g. for unit tests

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = 'Larry Shaffer'
__date__ = '2014/02/11'
__copyright__ = 'Copyright 2014, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import sys
import os
import shutil
import platform
import subprocess
import time
import urllib
import urllib2
import tempfile

from utilities import (
    unitTestDataPath,
    getExecutablePath,
    openInBrowserTab
)

# allow import error to be raised if qgis is not on sys.path
try:
    # noinspection PyUnresolvedReferences
    from qgis.core import QgsRectangle, QgsCoordinateReferenceSystem
except ImportError as e:
    raise ImportError(str(e) + '\n\nPlace path to pyqgis modules on sys.path,'
                               ' or assign to PYTHONPATH')

FCGIBIN = None
MAPSERV = None
SERVRUN = False


class ServerProcessError(Exception):

    def __init__(self, title, msg, err=''):
        msg += '\n' + ('\n' + str(err).strip() + '\n' if err else '')
        self.msg = """
#----------------------------------------------------------------#
{0}
{1}
#----------------------------------------------------------------#
    """.format(title, msg)

    def __str__(self):
        return self.msg


class ServerProcess(object):

    def __init__(self):
        self._startenv = None
        self._startcmd = []
        self._stopcmd = []
        self._restartcmd = []
        self._statuscmd = []
        self._process = None
        """:type : subprocess.Popen"""
        self._win = self._mac = self._linux = self._unix = False
        self._dist = ()
        self._resolve_platform()

    # noinspection PyMethodMayBeStatic
    def _run(self, cmd, env=None):
        # print repr(cmd)
        p = subprocess.Popen(cmd,
                             stderr=subprocess.PIPE,
                             stdout=subprocess.PIPE,
                             env=env,
                             close_fds=True)
        err = p.communicate()[1]
        if err:
            if p:
                p.kill()
                # noinspection PyUnusedLocal
                p = None
            cmd_s = repr(cmd).strip() + ('\n' + 'ENV: ' + repr(env).strip() +
                                         '\n' if env is not None else '')
            raise ServerProcessError('Server process command failed',
                                     cmd_s, err)
        return p

    def start(self):
        if self.running():
            return
        self._process = self._run(self._startcmd, env=self._startenv)

    def stop(self):
        if not self.running():
            return False
        if self._stopcmd:
            self._run(self._stopcmd)
        else:
            self._process.terminate()
        self._process = None
        return True

    def restart(self):
        if self._restartcmd:
            self._run(self._restartcmd)
        else:
            self.stop()
            self.start()

    def running(self):
        running = False
        if self._statuscmd:
            try:
                subprocess.check_call(self._statuscmd,
                                      stdout=subprocess.PIPE,
                                      stderr=subprocess.PIPE)
                running = True
            except subprocess.CalledProcessError:
                running = False
        elif self._process:
            running = self._process.poll() is None
        return running

    def set_startenv(self, env):
        self._startenv = env

    def set_startcmd(self, cmd):
        self._startcmd = cmd

    def set_stopcmd(self, cmd):
        self._stopcmd = cmd

    def set_restartcmd(self, cmd):
        self._restartcmd = cmd

    def set_statuscmd(self, cmd):
        self._statuscmd = cmd

    def process(self):
        return self._process

    def pid(self):
        pid = 0
        if self._process:
            pid = self._process.pid
        return pid

    def _resolve_platform(self):
        s = platform.system().lower()
        self._linux = s.startswith('lin')
        self._mac = s.startswith('dar')
        self._unix = self._linux or self._mac
        self._win = s.startswith('win')
        self._dist = platform.dist()


class WebServerProcess(ServerProcess):

    def __init__(self, kind, exe, conf_dir, temp_dir):
        ServerProcess.__init__(self)
        sufx = 'unix' if self._unix else 'win'
        if kind == 'lighttpd':
            conf = os.path.join(conf_dir, 'lighttpd', 'config',
                                'lighttpd_{0}.conf'.format(sufx))
            self.set_startenv({'QGIS_SERVER_TEMP_DIR': temp_dir})
            init_scr_dir = os.path.join(conf_dir, 'lighttpd', 'scripts')
            if self._mac:
                init_scr = os.path.join(init_scr_dir, 'lighttpd_mac.sh')
                self.set_startcmd([init_scr, 'start', exe, conf, temp_dir])
                self.set_stopcmd([init_scr, 'stop'])
                self.set_restartcmd([init_scr, 'restart', exe, conf, temp_dir])
                self.set_statuscmd([init_scr, 'status'])
            elif self._linux:
                dist = self._dist[0].lower()
                if dist == 'debian' or dist == 'ubuntu':
                    init_scr = os.path.join(init_scr_dir, 'lighttpd_debian.sh')
                    self.set_startcmd([
                        init_scr, 'start', exe, temp_dir, conf])
                    self.set_stopcmd([init_scr, 'stop', exe, temp_dir])
                    self.set_restartcmd([
                        init_scr, 'restart', exe, temp_dir, conf])
                    self.set_statuscmd([init_scr, 'status', exe, temp_dir])
                elif dist == 'fedora' or dist == 'rhel':  # are these correct?
                    pass
            else:  # win
                pass


class FcgiServerProcess(ServerProcess):

    def __init__(self, kind, exe, fcgi_bin, conf_dir, temp_dir):
        ServerProcess.__init__(self)
        if kind == 'spawn-fcgi':
            if self._unix:
                fcgi_sock = os.path.join(temp_dir, 'var', 'run',
                                         'qgs_mapserv.sock')
                init_scr_dir = os.path.join(conf_dir, 'fcgi', 'scripts')
                self.set_startenv({
                    'QGIS_LOG_FILE':
                    os.path.join(temp_dir, 'log', 'qgis_server.log')})
                if self._mac:
                    init_scr = os.path.join(init_scr_dir, 'spawn_fcgi_mac.sh')
                    self.set_startcmd([init_scr, 'start', exe, fcgi_sock,
                                       temp_dir + fcgi_bin, temp_dir])
                    self.set_stopcmd([init_scr, 'stop'])
                    self.set_restartcmd([init_scr, 'restart', exe, fcgi_sock,
                                         temp_dir + fcgi_bin, temp_dir])
                    self.set_statuscmd([init_scr, 'status'])
                elif self._linux:
                    dist = self._dist[0].lower()
                    if dist == 'debian' or dist == 'ubuntu':
                        init_scr = os.path.join(init_scr_dir,
                                                'spawn_fcgi_debian.sh')
                        self.set_startcmd([
                            init_scr, 'start', exe, fcgi_sock,
                            temp_dir + fcgi_bin, temp_dir])
                        self.set_stopcmd([
                            init_scr, 'stop', exe, fcgi_sock,
                            temp_dir + fcgi_bin, temp_dir])
                        self.set_restartcmd([
                            init_scr, 'restart', exe, fcgi_sock,
                            temp_dir + fcgi_bin, temp_dir])
                        self.set_statuscmd([
                            init_scr, 'status', exe, fcgi_sock,
                            temp_dir + fcgi_bin, temp_dir])
                    elif dist == 'fedora' or dist == 'rhel':
                        pass
            else:  # win
                pass


# noinspection PyPep8Naming,PyShadowingNames
class QgisLocalServer(object):

    def __init__(self, fcgi_bin):
        msg = 'FCGI binary not found at:\n{0}'.format(fcgi_bin)
        assert os.path.exists(fcgi_bin), msg

        msg = "FCGI binary not 'qgis_mapserv.fcgi':"
        assert fcgi_bin.endswith('qgis_mapserv.fcgi'), msg

        # hardcoded url, makes all this automated
        self._ip = '127.0.0.1'
        self._port = '8448'
        self._web_url = 'http://{0}:{1}'.format(self._ip, self._port)
        self._fcgibin_path = '/cgi-bin/qgis_mapserv.fcgi'
        self._fcgi_url = '{0}{1}'.format(self._web_url, self._fcgibin_path)
        self._conf_dir = unitTestDataPath('qgis_local_server')

        self._fcgiserv_process = self._webserv_process = None
        self._fcgiserv_bin = fcgi_bin
        self._fcgiserv_path = self._webserv_path = ''
        self._fcgiserv_kind = self._webserv_kind = ''
        self._temp_dir = ''
        self._web_dir = ''

        servers = [
            ('spawn-fcgi', 'lighttpd')
            #('fcgiwrap', 'nginx'),
            #('uwsgi', 'nginx'),
        ]

        chkd = ''
        for fcgi, web in servers:
            fcgi_path = getExecutablePath(fcgi)
            web_path = getExecutablePath(web)
            if fcgi_path and web_path:
                self._fcgiserv_path = fcgi_path
                self._webserv_path = web_path
                self._fcgiserv_kind = fcgi
                self._webserv_kind = web
                break
            else:
                chkd += "Find '{0}': {1}\n".format(fcgi, fcgi_path)
                chkd += "Find '{0}': {1}\n\n".format(web, web_path)

        if not (self._fcgiserv_path and self._webserv_path):
            raise ServerProcessError(
                'Could not locate server binaries',
                chkd,
                'Make sure one of the sets of servers is available on PATH'
            )

        self._temp_dir = tempfile.mkdtemp()
        self._setup_temp_dir()

        # initialize the servers
        self._fcgiserv_process = FcgiServerProcess(
            self._fcgiserv_kind, self._fcgiserv_path,
            self._fcgibin_path, self._conf_dir, self._temp_dir)
        self._webserv_process = WebServerProcess(
            self._webserv_kind, self._webserv_path,
            self._conf_dir, self._temp_dir)

        # stop any leftover processes, if possible
        self.stop_processes()

    def startup(self, chkcapa=False):
        if not os.path.exists(self._temp_dir):
            self._setup_temp_dir()
        self.start_processes()
        if chkcapa:
            self.check_server_capabilities()

    def shutdown(self):
        self.stop_processes()
        self.remove_temp_dir()

    def start_processes(self):
        self._fcgiserv_process.start()
        self._webserv_process.start()

    def stop_processes(self):
        self._fcgiserv_process.stop()
        self._webserv_process.stop()

    def restart_processes(self):
        self._fcgiserv_process.restart()
        self._webserv_process.restart()

    def fcgi_server_process(self):
        return self._fcgiserv_process

    def web_server_process(self):
        return self._webserv_process

    def processes_running(self):
        return (self._fcgiserv_process.running() and
                self._webserv_process.running())

    def config_dir(self):
        return self._conf_dir

    def web_dir(self):
        return self._web_dir

    def open_web_dir(self):
        self._open_fs_item(self._web_dir)

    def web_dir_install(self, items, src_dir=''):
        msg = 'Items parameter should be passed in as a list'
        assert isinstance(items, list), msg
        for item in items:
            if item.startswith('.') or item.endswith('~'):
                continue
            path = item
            if src_dir:
                path = os.path.join(src_dir, item)
            try:
                if os.path.isfile(path):
                    shutil.copy2(path, self._web_dir)
                elif os.path.isdir(path):
                    shutil.copytree(path, self._web_dir)
            except Exception as err:
                raise ServerProcessError('Failed to copy to web directory:',
                                         item,
                                         str(err))

    def clear_web_dir(self):
        for f in os.listdir(self._web_dir):
            path = os.path.join(self._web_dir, f)
            try:
                if os.path.isfile(path):
                    os.unlink(path)
                else:
                    shutil.rmtree(path)
            except Exception as err:
                raise ServerProcessError('Failed to clear web directory', err)

    def temp_dir(self):
        return self._temp_dir

    def open_temp_dir(self):
        self._open_fs_item(self._temp_dir)

    def remove_temp_dir(self):
        if os.path.exists(self._temp_dir):
            shutil.rmtree(self._temp_dir)

    def ip(self):
        return self._ip

    def port(self):
        return self._port

    def web_url(self):
        return self._web_url

    def fcgi_url(self):
        return self._fcgi_url

    def check_server_capabilities(self):
        params = {
            'SERVICE': 'WMS',
            'VERSION': '1.3.0',
            'REQUEST': 'GetCapabilities'
        }
        if not self.get_capabilities(params, False)[0]:
            self.shutdown()
            raise ServerProcessError(
                'Local QGIS Server shutdown',
                'Test QGIS Server is not accessible at:\n' + self._fcgi_url,
                'Error: failed to retrieve server capabilities'
            )

    def get_capabilities(self, params, browser=False):
        assert self.processes_running(), 'Server processes not running'

        params = self._params_to_upper(params)
        if (('REQUEST' in params and params['REQUEST'] != 'GetCapabilities') or
                'REQUEST' not in params):
            params['REQUEST'] = 'GetCapabilities'

        url = self._fcgi_url + '?' + self.process_params(params)

        res = urllib.urlopen(url)
        xml = res.read()
        if browser:
            tmp = tempfile.NamedTemporaryFile(suffix=".html", delete=False)
            tmp.write(xml)
            url = tmp.name
            tmp.close()
            openInBrowserTab(url)
            return False, ''

        success = ('error reading the project file' in xml or
                   'WMS_Capabilities' in xml)
        return success, xml

    def get_map(self, params, browser=False):
        assert self.processes_running(), 'Server processes not running'

        msg = ('Map request parameters should be passed in as a dict '
               '(key case can be mixed)')
        assert isinstance(params, dict), msg

        params = self._params_to_upper(params)
        try:
            proj = params['MAP']
        except KeyError as err:
            raise KeyError(str(err) + '\nMAP not found in parameters dict')

        if not os.path.exists(proj):
            msg = '{0}'.format(proj)
            w_proj = os.path.join(self._web_dir, proj)
            if os.path.exists(w_proj):
                params['MAP'] = w_proj
            else:
                msg += '\n  or\n' + w_proj
                raise ServerProcessError(
                    'GetMap Request Error',
                    'Project not found at:\n{0}'.format(msg)
                )

        if (('REQUEST' in params and params['REQUEST'] != 'GetMap') or
                'REQUEST' not in params):
            params['REQUEST'] = 'GetMap'

        url = self._fcgi_url + '?' + self.process_params(params)

        if browser:
            openInBrowserTab(url)
            return False, ''

        # try until qgis_mapserv.fcgi process is available (for 20 seconds)
        # on some platforms the fcgi_server_process is a daemon handling the
        # launch of the fcgi-spawner, which may be running quickly, but the
        # qgis_mapserv.fcgi spawned process is not yet accepting connections
        resp = None
        tmp_png = None
        # noinspection PyUnusedLocal
        filepath = ''
        # noinspection PyUnusedLocal
        success = False
        start_time = time.time()
        while time.time() - start_time < 20:
            resp = None
            try:
                tmp_png = urllib2.urlopen(url)
            except urllib2.HTTPError as resp:
                if resp.code == 503 or resp.code == 500:
                    time.sleep(1)
                else:
                    raise ServerProcessError(
                        'Web/FCGI Process Request HTTPError',
                        'Cound not connect to process: ' + str(resp.code),
                        resp.message
                    )
            except urllib2.URLError as resp:
                raise ServerProcessError(
                    'Web/FCGI Process Request URLError',
                    'Cound not connect to process: ' + str(resp.code),
                    resp.reason
                )
            else:
                delta = time.time() - start_time
                print 'Seconds elapsed for server GetMap: ' + str(delta)
                break

        if resp is not None:
            raise ServerProcessError(
                'Web/FCGI Process Request Error',
                'Cound not connect to process: ' + str(resp.code)
            )

        if (tmp_png is not None
                and tmp_png.info().getmaintype() == 'image'
                and tmp_png.info().getheader('Content-Type') == 'image/png'):

            tmp = tempfile.NamedTemporaryFile(suffix=".png", delete=False)
            filepath = tmp.name
            tmp.write(tmp_png.read())
            tmp.close()
            success = True
        else:
            raise ServerProcessError(
                'FCGI Process Request Error',
                'No valid PNG output'
            )

        return success, filepath, url

    def process_params(self, params):
        # set all keys to uppercase
        params = self._params_to_upper(params)
        # convert all convenience objects to compatible strings
        self._convert_instances(params)
        # encode params
        return urllib.urlencode(params, True)

    @staticmethod
    def _params_to_upper(params):
        return dict((k.upper(), v) for k, v in params.items())

    @staticmethod
    def _convert_instances(params):
        if not params:
            return
        if ('LAYERS' in params and
                isinstance(params['LAYERS'], list)):
            params['LAYERS'] = ','.join(params['LAYERS'])
        if ('BBOX' in params and
                isinstance(params['BBOX'], QgsRectangle)):
            # not needed for QGIS's 1.3.0 server?
            # # invert x, y of rect and set precision to 16
            # rect = self.params['BBOX']
            # bbox = ','.join(map(lambda x: '{0:0.16f}'.format(x),
            #                     [rect.yMinimum(), rect.xMinimum(),
            #                      rect.yMaximum(), rect.xMaximum()]))
            params['BBOX'] = \
                params['BBOX'].toString(1).replace(' : ', ',')

        if ('CRS' in params and
                isinstance(params['CRS'], QgsCoordinateReferenceSystem)):
            params['CRS'] = params['CRS'].authid()

    def _setup_temp_dir(self):
        self._web_dir = os.path.join(self._temp_dir, 'www', 'htdocs')
        cgi_bin = os.path.join(self._temp_dir, 'cgi-bin')

        os.makedirs(cgi_bin, mode=0o755)
        os.makedirs(os.path.join(self._temp_dir, 'log'), mode=0o755)
        os.makedirs(os.path.join(self._temp_dir, 'var', 'run'), mode=0o755)
        os.makedirs(self._web_dir, mode=0o755)

        # symlink or copy in components
        shutil.copy2(os.path.join(self._conf_dir, 'index.html'), self._web_dir)
        if not platform.system().lower().startswith('win'):
            # symlink allow valid runningFromBuildDir results
            os.symlink(self._fcgiserv_bin,
                       os.path.join(cgi_bin,
                                    os.path.basename(self._fcgiserv_bin)))
        else:
            # TODO: what to do here for Win runningFromBuildDir?
            #       copy qgisbuildpath.txt from output/bin directory, too?
            shutil.copy2(self._fcgiserv_bin, cgi_bin)

    @staticmethod
    def _exe_path(exe):
        exe_exts = []
        if (platform.system().lower().startswith('win') and
                "PATHEXT" in os.environ):
            exe_exts = os.environ["PATHEXT"].split(os.pathsep)

        for path in os.environ["PATH"].split(os.pathsep):
            exe_path = os.path.join(path, exe)
            if os.path.exists(exe_path):
                return exe_path
            for ext in exe_exts:
                if os.path.exists(exe_path + ext):
                    return exe_path
        return ''

    @staticmethod
    def _open_fs_item(item):
        if not os.path.exists(item):
            return
        s = platform.system().lower()
        if s.startswith('dar'):
            subprocess.call(['open', item])
        elif s.startswith('lin'):
            # xdg-open "$1" &> /dev/null &
            subprocess.call(['xdg-open', item])
        elif s.startswith('win'):
            subprocess.call([item])
        else:  # ?
            pass


# noinspection PyPep8Naming
def getLocalServer():
    """ Start a local test server controller that independently manages Web and
    FCGI-spawn processes.

    Input
        NIL

    Output
        handle to QgsLocalServer, that's been tested to be valid, then shutdown

    If MAPSERV is already running the handle to it will be returned.

    Before unit test class add:

        MAPSERV = getLocalServer()

    IMPORTANT: When using MAPSERV in a test class, ensure to set these:

        @classmethod
        def setUpClass(cls):
            MAPSERV.startup()

    This ensures the subprocesses are started and the temp directory is created.

        @classmethod
        def tearDownClass(cls):
            MAPSERV.shutdown()
            # or, when testing, instead of shutdown...
            #   MAPSERV.stop_processes()
            #   MAPSERV.open_temp_dir()

    This ensures the subprocesses are stopped and the temp directory is removed.
    If this is not used, the server processes may continue to run after tests.

    If you need to restart the qgis_mapserv.fcgi spawning process to show
    changes to project settings, consider adding:

        def setUp(self):
            '''Run before each test.'''
            # web server stays up across all tests
            MAPSERV.fcgi_server_process().start()

        def tearDown(self):
            '''Run after each test.'''
            # web server stays up across all tests
            MAPSERV.fcgi_server_process().stop()

    :rtype: QgisLocalServer
    """
    global SERVRUN  # pylint: disable=W0603
    global MAPSERV  # pylint: disable=W0603
    if SERVRUN:
        msg = 'Local server has already failed to launch or run'
        assert MAPSERV is not None, msg
    else:
        SERVRUN = True

    global FCGIBIN  # pylint: disable=W0603
    if FCGIBIN is None:
        msg = 'Could not find QGIS_PREFIX_PATH (build directory) in environ'
        assert 'QGIS_PREFIX_PATH' in os.environ, msg

        fcgi_path = os.path.join(os.environ['QGIS_PREFIX_PATH'], 'bin',
                                 'qgis_mapserv.fcgi')
        msg = 'Could not locate qgis_mapserv.fcgi in build/bin directory'
        assert os.path.exists(fcgi_path), msg

        FCGIBIN = fcgi_path

    if MAPSERV is None:
        # On QgisLocalServer init, Web and FCGI-spawn executables are located,
        # configurations to start/stop/restart those processes (relative to
        # host platform) are loaded into controller, a temporary web
        # directory is created, and the FCGI binary copied to its cgi-bin.
        srv = QgisLocalServer(FCGIBIN)
        # noinspection PyStatementEffect
        """:type : QgisLocalServer"""

        try:
            msg = 'Temp web directory could not be created'
            assert os.path.exists(srv.temp_dir()), msg

            # install test project components to temporary web directory
            test_proj_dir = os.path.join(srv.config_dir(), 'test-project')
            srv.web_dir_install(os.listdir(test_proj_dir), test_proj_dir)
            # verify they were copied
            msg = 'Test project could not be copied to temp web directory'
            res = os.path.exists(os.path.join(srv.web_dir(), 'test-server.qgs'))
            assert res, msg

            # verify subprocess' status can be checked
            msg = 'Server processes status could not be checked'
            assert not srv.processes_running(), msg

            # startup server subprocesses, and check capabilities
            srv.startup()
            msg = 'Server processes could not be started'
            assert srv.processes_running(), msg

            # verify web server (try for 30 seconds)
            start_time = time.time()
            res = None
            while time.time() - start_time < 30:
                time.sleep(1)
                try:
                    res = urllib2.urlopen(srv.web_url())
                    if res.getcode() == 200:
                        break
                except urllib2.URLError:
                    pass
            msg = 'Web server basic access to root index.html failed'
            # print repr(res)
            assert (res is not None
                    and res.getcode() == 200
                    and 'Web Server Working' in res.read()), msg

            # verify basic wms service
            params = {
                'SERVICE': 'WMS',
                'VERSION': '1.3.0',
                'REQUEST': 'GetCapabilities'
            }
            msg = '\nFCGI server failed to return capabilities'
            assert srv.get_capabilities(params, False)[0], msg

            params = {
                'SERVICE': 'WMS',
                'VERSION': '1.3.0',
                'REQUEST': 'GetCapabilities',
                'MAP': 'test-server.qgs'
            }
            msg = '\nFCGI server failed to return capabilities for project'
            assert srv.get_capabilities(params, False)[0], msg

            # verify the subprocesses can be stopped and controller shutdown
            srv.shutdown()  # should remove temp directory (and test project)
            msg = 'Server processes could not be stopped'
            assert not srv.processes_running(), msg
            msg = 'Temp web directory could not be removed'
            assert not os.path.exists(srv.temp_dir()), msg

            MAPSERV = srv
        except AssertionError as err:
            srv.shutdown()
            raise AssertionError(err)

    return MAPSERV


if __name__ == '__main__':
    # NOTE: see test_qgis_local_server.py for CTest suite

    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument(
        'fcgi', metavar='fcgi-bin-path',
        help='Path to qgis_mapserv.fcgi'
    )
    args = parser.parse_args()

    fcgi = os.path.realpath(args.fcgi)
    if not os.path.isabs(fcgi) or not os.path.exists(fcgi):
        print 'qgis_mapserv.fcgi not resolved to existing absolute path.'
        sys.exit(1)

    local_srv = QgisLocalServer(fcgi)
    proj_dir = os.path.join(local_srv.config_dir(), 'test-project')
    local_srv.web_dir_install(os.listdir(proj_dir), proj_dir)
    # local_srv.open_temp_dir()
    # sys.exit()
    # creating crs needs app instance to access /resources/srs.db
    #   crs = QgsCoordinateReferenceSystem()
    # default for labeling test data sources: WGS 84 / UTM zone 13N
    #   crs.createFromSrid(32613)
    req_params = {
        'SERVICE': 'WMS',
        'VERSION': '1.3.0',
        'REQUEST': 'GetMap',
        # 'MAP': os.path.join(local_srv.web_dir(), 'test-server.qgs'),
        'MAP': 'test-server.qgs',
        # layer stacking order for rendering: bottom,to,top
        'LAYERS': ['background', 'aoi'],  # or 'background,aoi'
        'STYLES': ',',
        # 'CRS': QgsCoordinateReferenceSystem obj
        'CRS': 'EPSG:32613',
        # 'BBOX': QgsRectangle(606510, 4823130, 612510, 4827130)
        'BBOX': '606510,4823130,612510,4827130',
        'FORMAT': 'image/png',  # or: 'image/png; mode=8bit'
        'WIDTH': '600',
        'HEIGHT': '400',
        'DPI': '72',
        'MAP_RESOLUTION': '72',
        'FORMAT_OPTIONS': 'dpi:72',
        'TRANSPARENT': 'FALSE',
        'IgnoreGetMapUrl': '1'
    }

    # local_srv.web_server_process().start()
    # openInBrowserTab('http://127.0.0.1:8448')
    # local_srv.web_server_process().stop()
    # sys.exit()
    local_srv.startup(False)
    openInBrowserTab('http://127.0.0.1:8448')
    try:
        local_srv.check_server_capabilities()
        # open resultant png with system
        result, png, url = local_srv.get_map(req_params)
    finally:
        local_srv.shutdown()

    if result:
        # print png
        openInBrowserTab('file://' + png)
    else:
        raise ServerProcessError('GetMap Test', 'Failed to generate PNG')
