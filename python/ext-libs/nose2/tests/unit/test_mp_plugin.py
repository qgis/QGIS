from nose2 import session
from nose2.tests._common import TestCase, Conn
from nose2.plugins import mp
import sys


class TestMPPlugin(TestCase):

    def setUp(self):
        self.session = session.Session()
        self.plugin = mp.MultiProcess(session=self.session)

    def test_gentests(self):
        conn = Conn([1, 2, 3])
        res = []
        for x in mp.gentests(conn):
            res.append(x)
        self.assertEqual(res, [1, 2, 3])

    def test_recording_plugin_interface(self):
        rpi = mp.RecordingPluginInterface()
        # this one should record
        rpi.setTestOutcome(None)
        # none of these should record
        rpi.getTestCaseNames(None)
        rpi.startSubprocess(None)
        rpi.stopSubprocess(None)
        rpi.registerInSubprocess(None)
        rpi.loadTestsFromModule(None)
        rpi.loadTestsFromTestCase(None)
        self.assertEqual(rpi.flush(), [('setTestOutcome', None)])

    def test_address(self):
        platform = sys.platform
        try:
            sys.platform = "linux"
            host = "1.2.3.4"
            port = 245
            self.plugin.setAddress(host)
            self.assertEqual((self.plugin.bind_host, self.plugin.bind_port),
                             (host, 0))
            self.plugin.setAddress("%s:%i" % (host, port))
            self.assertEqual((self.plugin.bind_host, self.plugin.bind_port),
                             (host, port))
            self.plugin.setAddress(None)
            self.assertEqual((self.plugin.bind_host, self.plugin.bind_port),
                             (None, 0))
            sys.platform = "win32"
            self.plugin.setAddress(host)
            self.assertEqual((self.plugin.bind_host, self.plugin.bind_port),
                             (host, 0))
            self.plugin.setAddress("%s:%i" % (host, port))
            self.assertEqual((self.plugin.bind_host, self.plugin.bind_port),
                             (host, port))
            self.plugin.setAddress(None)
            self.assertEqual((self.plugin.bind_host, self.plugin.bind_port),
                             ("127.116.157.163", 0))
        finally:
            sys.platform = platform

