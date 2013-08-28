# -*- coding: utf-8 -*-
"""Convenience interface to a local QGIS Server, e.g. for unit tests

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = 'Larry Shaffer'
__date__ = '07/15/2013'
__copyright__ = 'Copyright 2013, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import sys
import os
import ConfigParser
import urllib
import tempfile

# allow import error to be raised if qgis is not on sys.path
try:
    from qgis.core import QgsRectangle, QgsCoordinateReferenceSystem
except ImportError, e:
    raise ImportError(str(e) + '\n\nPlace path to pyqgis modules on sys.path,'
                               ' or assign to PYTHONPATH')


class ServerNotAccessibleError(Exception):

    def __init__(self, cgiurl):
        self.msg = """
    #----------------------------------------------------------------#
    Local test QGIS Server is not accessible at:
    {0}
    #----------------------------------------------------------------#
    """.format(cgiurl)

    def __str__(self):
        return self.msg


class QgisLocalServer(object):

    def __init__(self, cgiurl, chkcapa=False):
        self.cgiurl = cgiurl
        self.params = {}
        self.active = False

        # check capabilities to verify server is accessible
        if chkcapa:
            params = {
                'SERVICE': 'WMS',
                'VERSION': '1.3.0',
                'REQUEST': 'GetCapabilities'
            }
            if not self.getCapabilities(params, False)[0]:
                raise ServerNotAccessibleError(self.cgiurl)
        self.active = True

    def activeServer(self):
        return self.active

    def cgiUrl(self):
        return self.cgiurl

    def getCapabilities(self, params, browser=False):
        if (('REQUEST' in params and params['REQUEST'] != 'GetCapabilities') or
                'REQUEST' not in params):
            params['REQUEST'] = 'GetCapabilities'

        self.params = params
        url = self.cgiurl + '?' + self._processParams()
        self.params = {}

        if browser:
            openInBrowserTab(url)
            return False, ''

        res = urllib.urlopen(url)
        xml = res.read()
        success = ('perhaps you left off the .qgs extension' in xml or
                   'WMS_Capabilities' in xml)
        return success, xml

    def getMap(self, params, browser=False):
        assert self.active, 'Server not acessible'

        msg = 'Parameters should be passed in as a dict'
        assert isinstance(params, dict), msg

        if (('REQUEST' in params and params['REQUEST'] != 'GetMap') or
                'REQUEST' not in params):
            params['REQUEST'] = 'GetMap'

        self.params = params
        url = self.cgiurl + '?' + self._processParams()
        self.params = {}

        if browser:
            openInBrowserTab(url)
            return False, ''

        tmp = tempfile.NamedTemporaryFile(suffix=".png", delete=False)
        tmp.close()
        res = urllib.urlretrieve(url, tmp.name)
        filepath = res[0]
        success = True
        if (res[1].getmaintype() != 'image' or
                res[1].getheader('Content-Type') != 'image/png'):
            success = False

        return success, filepath

    def _processParams(self):
        # set all keys to uppercase
        self.params = dict((k.upper(), v) for k, v in self.params.items())
        # convert all convenience objects to compatible strings
        self._convertInstances()
        # encode params
        return urllib.urlencode(self.params, True)

    def _convertInstances(self):
        if not self.params:
            return
        if ('LAYERS' in self.params and
                isinstance(self.params['LAYERS'], list)):
            self.params['LAYERS'] = ','.join(self.params['LAYERS'])
        if ('BBOX' in self.params and
                isinstance(self.params['BBOX'], QgsRectangle)):
            # not needed for QGIS's 1.3.0 server?
            # # invert x, y of rect and set precision to 16
            # rect = self.params['BBOX']
            # bbox = ','.join(map(lambda x: '{0:0.16f}'.format(x),
            #                     [rect.yMinimum(), rect.xMinimum(),
            #                      rect.yMaximum(), rect.xMaximum()]))
            self.params['BBOX'] = \
                self.params['BBOX'].toString(1).replace(' : ', ',')

        if ('CRS' in self.params and
                isinstance(self.params['CRS'], QgsCoordinateReferenceSystem)):
            self.params['CRS'] = self.params['CRS'].authid()


class ServerConfigNotAccessibleError(Exception):

    def __init__(self, err=''):
        self.msg = '\n\n' + str(err) + '\n'
        self.msg += """
    #----------------------------------------------------------------#
    Local test QGIS Server is not accessible
    Check local server configuration settings in:

    /<current user>/.qgis2/qgis_local_server.cfg

    Adjust settings under the LocalServer section:
    protocol = http (recommended)
    host = localhost, domain.tld or IP address
    port = 80 or a user-defined port above 1024
    fcgipath = path to working qgis_mapserv.fcgi as known by server
    sourceurl = DO NOT ADJUST
    projdir = path WRITEABLE by this user and READABLE by www server

    Sample configuration (default):
    sourceurl (built) = http://localhost:80/cgi-bin/qgis_mapserv.fcgi
    projdir = /var/www/qgis/test-projects
    #----------------------------------------------------------------#
    """

    def __str__(self):
        return self.msg


class QgisLocalServerConfig(QgisLocalServer):

    def __init__(self, cfgdir, chkcapa=False):
        msg = 'Server configuration directory required'
        assert cfgdir, msg

        self.cfgdir = cfgdir
        self.cfg = os.path.normpath(os.path.join(self.cfgdir,
                                                 'qgis_local_server.cfg'))
        if not os.path.exists(self.cfg):
            msg = ('Default server configuration file could not be written'
                   ' to {0}'.format(self.cfg))
            assert self._writeDefaultServerConfig(), msg

        self._checkItemFound('file', self.cfg)
        self._checkItemReadable('file', self.cfg)

        cgiurl, self.projdir = self._readServerConfig()

        try:
            self._checkItemFound('project directory', self.projdir)
            self._checkItemReadable('project directory', self.projdir)
            self._checkItemWriteable('project directory', self.projdir)
            super(QgisLocalServerConfig, self).__init__(cgiurl, chkcapa)
        except Exception, err:
            raise ServerConfigNotAccessibleError(err)

    def projectDir(self):
        return self.projdir

    def getMap(self, params, browser=False):
        msg = ('Map request parameters should be passed in as a dict '
               '(key case can be mixed)')
        assert isinstance(params, dict), msg

        params = dict((k.upper(), v) for k, v in params.items())
        try:
            proj = params['MAP']
        except KeyError, e:
            raise KeyError(str(e) + '\nMAP not found in parameters dict')

        if not os.path.exists(proj):
            proj = os.path.join(self.projdir, proj)
        msg = 'Project could not be found at {0}'.format(proj)
        assert os.path.exists(proj), msg

        return super(QgisLocalServerConfig, self).getMap(params, browser)

    def _checkItemFound(self, item, path):
        msg = ('Server configuration {0} could not be found at:\n'
               '  {1}'.format(item, path))
        assert os.path.exists(path), msg

    def _checkItemReadable(self, item, path):
        msg = ('Server configuration {0} is not readable from:\n'
               '  {1}'.format(item, path))
        assert os.access(path, os.R_OK), msg

    def _checkItemWriteable(self, item, path):
        msg = ('Server configuration {0} is not writeable from:\n'
               '  {1}'.format(item, path))
        assert os.access(path, os.W_OK), msg

    def _writeDefaultServerConfig(self):
        """Overwrites any existing server configuration file with default"""
        # http://hub.qgis.org/projects/quantum-gis/wiki/QGIS_Server_Tutorial
        # linux: http://localhost/cgi-bin/qgis_mapserv.fcgi?  <-- default
        # mac: http://localhost/qgis-mapserv/qgis_mapserv.fcgi?
        # win: http://localhost/qgis/qgis_mapserv.fcgi?
        config = ConfigParser.SafeConfigParser(
            {
                'sourceurl': 'http://localhost:80/cgi-bin/qgis_mapserv.fcgi',
                'projdir': '/var/www/qgis/test-projects'
            }
        )
        config.add_section('LocalServer')
        config.set('LocalServer', 'protocol', 'http')
        config.set('LocalServer', 'host', 'localhost')
        config.set('LocalServer', 'port', '80')
        config.set('LocalServer', 'fcgipath', '/cgi-bin/qgis_mapserv.fcgi')
        config.set('LocalServer', 'sourceurl',
                   '%(protocol)s://%(host)s:%(port)s%(fcgipath)s')
        config.set('LocalServer', 'projdir', '/var/www/qgis/test-projects')

        with open(self.cfg, 'w+') as configfile:
            config.write(configfile)
        return os.path.exists(self.cfg)

    def _readServerConfig(self):
        config = ConfigParser.SafeConfigParser()
        config.read(self.cfg)
        url = config.get('LocalServer', 'sourceurl')
        projdir = config.get('LocalServer', 'projdir')
        return url, projdir


def openInBrowserTab(url):
    if sys.platform[:3] in ('win', 'dar'):
        import webbrowser
        webbrowser.open_new_tab(url)
    else:
        # some Linux OS pause execution on webbrowser open, so background it
        import subprocess
        cmd = 'import webbrowser;' \
              'webbrowser.open_new_tab({0})'.format(url)
        p = subprocess.Popen([sys.executable, "-c", cmd],
                             stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT).pid


if __name__ == '__main__':
    qgishome = os.path.join(os.path.expanduser('~'), '.qgis2')
    server = QgisLocalServerConfig(qgishome, True)
    # print '\nServer accessible and returned capabilities'

    # creating crs needs app instance to access /resources/srs.db
    # crs = QgsCoordinateReferenceSystem()
    # # default for labeling test data sources: WGS 84 / UTM zone 13N
    # crs.createFromSrid(32613)
    ext = QgsRectangle(606510, 4823130, 612510, 4827130)
    params = {
        'SERVICE': 'WMS',
        'VERSION': '1.3.0',
        'REQUEST': 'GetMap',
        'MAP': '/test-projects/tests/tests.qgs',
        # layer stacking order for rendering: bottom,to,top
        'LAYERS': ['background', 'point'],  # or 'background,point'
        'STYLES': ',',
        'CRS': 'EPSG:32613',  # or: QgsCoordinateReferenceSystem obj
        'BBOX': ext,  # or: '606510,4823130,612510,4827130'
        'FORMAT': 'image/png',  # or: 'image/png; mode=8bit'
        'WIDTH': '600',
        'HEIGHT': '400',
        'DPI': '72',
        'MAP_RESOLUTION': '72',
        'FORMAT_OPTIONS': 'dpi:72',
        'TRANSPARENT': 'TRUE',
        'IgnoreGetMapUrl': '1'
    }
    if 'QGISSERVER_PNG' in os.environ:
        # open resultant png with system
        res, filepath = server.getMap(params, False)
        openInBrowserTab('file://' + filepath)
    else:
        # open GetMap url in browser
        res, filepath = server.getMap(params, True)
