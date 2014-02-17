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
import inspect
import urllib
import tempfile

# allow import error to be raised if qgis is not on sys.path
try:
    # noinspection PyUnresolvedReferences
    from qgis.core import QgsRectangle, QgsCoordinateReferenceSystem
except ImportError, e:
    raise ImportError(str(e) + '\n\nPlace path to pyqgis modules on sys.path,'
                               ' or assign to PYTHONPATH')


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
        self._resolve_platform()

    # noinspection PyMethodMayBeStatic
    def _run(self, cmd, env=None):
        # print repr(cmd)
        p = subprocess.Popen(cmd,
                             stderr=subprocess.PIPE,
                             stdout=subprocess.PIPE,
                             env=env,
                             close_fds=True)
        err = p.stderr.read()
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


class WebServerProcess(ServerProcess):

    def __init__(self, kind, exe, conf_dir, temp_dir):
        ServerProcess.__init__(self)
        sufx = 'unix' if self._unix else 'win'
        if kind == 'lighttpd':
            conf = os.path.join(conf_dir, 'lighttpd', 'config',
                                'lighttpd_{0}.conf'.format(sufx))
            self.set_startenv({'QGIS_SERVER_TEMP_DIR': temp_dir})
            if self._mac:
                init_scr = os.path.join(conf_dir, 'lighttpd', 'scripts',
                                        'lighttpd_mac.sh')
                self.set_startcmd([init_scr, 'start', exe, conf, temp_dir])
                self.set_stopcmd([init_scr, 'stop'])
                self.set_restartcmd([init_scr, 'restart', exe, conf, temp_dir])
                self.set_statuscmd([init_scr, 'status'])
            elif self._linux:
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
                if self._mac:
                    init_scr = os.path.join(conf_dir, 'fcgi', 'scripts',
                                            'spawn_fcgi_mac.sh')
                    self.set_startcmd([init_scr, 'start', exe, fcgi_sock,
                                       temp_dir + fcgi_bin])
                    self.set_stopcmd([init_scr, 'stop'])
                    self.set_restartcmd([init_scr, 'start', exe, fcgi_sock,
                                         temp_dir + fcgi_bin])
                    self.set_statuscmd([init_scr, 'status'])
                else:  # linux
                    pass
            else:  # win
                pass


# noinspection PyPep8Naming
class QgisLocalServer(object):

    def __init__(self, fcgi_bin, test_data_dir=''):
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
        if not test_data_dir:
            scr_dir = os.path.dirname(os.path.abspath(
                inspect.getfile(inspect.currentframe())))
            test_data_dir = os.path.abspath(
                os.path.join(scr_dir, '..', '..', 'testdata'))
        self._conf_dir = os.path.join(test_data_dir, 'qgis_local_server')

        self._fcgiserv_process = self._webserv_process = None
        self._fcgiserv_bin = fcgi_bin
        self._fcgiserv_path = self._webserv_path = ''
        self._fcgiserv_kind = self._webserv_kind = ''
        self._temp_dir = ''
        self._web_dir = ''

        servers = [
            ('spawn-fcgi', 'lighttpd'),
            #('fcgiwrap', 'nginx'),
            #('uwsgi', 'nginx'),
        ]

        chkd = ''
        for fcgi, web in servers:
            fcgi_path = self._exe_path(fcgi)
            web_path = self._exe_path(web)
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
        if os.path.exists(self._web_dir):
            subprocess.call(['open', self._web_dir])

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
            except Exception, err:
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
            except Exception, err:
                raise ServerProcessError('Failed to clear web directory', err)

    def temp_dir(self):
        return self._temp_dir

    def open_temp_dir(self):
        if os.path.exists(self._temp_dir):
            subprocess.call(['open', self._temp_dir])

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
            open_in_browser_tab(url)
            return False, ''

        success = ('perhaps you left off the .qgs extension' in xml or
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
        except KeyError, err:
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
            open_in_browser_tab(url)
            return False, ''

        tmp = tempfile.NamedTemporaryFile(suffix=".png", delete=False)
        success = True
        filepath = tmp.name
        # print 'filepath: ' + filepath
        tmp.close()
        filepath2, headers = urllib.urlretrieve(url, tmp.name)

        if (headers.getmaintype() != 'image' or
                headers.getheader('Content-Type') != 'image/png'):
            success = False
            if os.path.exists(filepath):
                os.unlink(filepath)
            filepath = ''

        return success, filepath

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

        os.makedirs(cgi_bin, mode=0755)
        os.makedirs(os.path.join(self._temp_dir, 'log'), mode=0755)
        os.makedirs(os.path.join(self._temp_dir, 'var', 'run'), mode=0755)
        os.makedirs(self._web_dir, mode=0755)

        # copy in components
        shutil.copy2(self._fcgiserv_bin, cgi_bin)
        shutil.copy2(os.path.join(self._conf_dir, 'index.html'),
                     self._web_dir)

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


def open_in_browser_tab(url):
    if sys.platform[:3] in ('win', 'dar'):
        import webbrowser
        webbrowser.open_new_tab(url)
    else:
        # some Linux OS pause execution on webbrowser open, so background it
        import subprocess
        cmd = 'import webbrowser;' \
              'webbrowser.open_new_tab({0})'.format(url)
        subprocess.Popen([sys.executable, "-c", cmd],
                         stdout=subprocess.PIPE,
                         stderr=subprocess.STDOUT)


if __name__ == '__main__':
    # this is a symlink to <build dir>/output/bin/qgis_mapserv.fcgi
    fcgibin = '/opt/qgis_mapserv/qgis_mapserv.fcgi'
    srv = QgisLocalServer(fcgibin)
    proj_dir = os.path.join(srv.config_dir(), 'test-project')
    srv.web_dir_install(os.listdir(proj_dir), proj_dir)
    # srv.open_web_dir()
    # creating crs needs app instance to access /resources/srs.db
    #   crs = QgsCoordinateReferenceSystem()
    # default for labeling test data sources: WGS 84 / UTM zone 13N
    #   crs.createFromSrid(32613)
    req_params = {
        'SERVICE': 'WMS',
        'VERSION': '1.3.0',
        'REQUEST': 'GetMap',
        # 'MAP': os.path.join(srv.web_dir(), 'test-server.qgs'),
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

    srv.startup(True)
    try:
        srv.check_server_capabilities()
        # open resultant png with system
        result, png = srv.get_map(req_params)
    finally:
        srv.shutdown()

    if result:
        # print png
        open_in_browser_tab('file://' + png)
    else:
        raise ServerProcessError('GetMap Test', 'Failed to generate PNG')
