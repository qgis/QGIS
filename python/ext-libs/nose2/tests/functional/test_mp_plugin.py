import sys

from nose2 import session
from nose2.compat import unittest
from nose2.plugins.mp import MultiProcess, procserver
from nose2.plugins import buffer
from nose2.plugins.loader import discovery, testcases
from nose2.tests._common import FunctionalTestCase, support_file, Conn
import multiprocessing
import threading
import time
from multiprocessing import connection


class TestMpPlugin(FunctionalTestCase):

    def setUp(self):
        super(TestMpPlugin, self).setUp()
        self.session = session.Session()
        self.plugin = MultiProcess(session=self.session)

    def test_flatten_without_fixtures(self):
        sys.path.append(support_file('scenario/slow'))
        import test_slow as mod

        suite = unittest.TestSuite()
        suite.addTest(mod.TestSlow('test_ok'))
        suite.addTest(mod.TestSlow('test_fail'))
        suite.addTest(mod.TestSlow('test_err'))

        flat = list(self.plugin._flatten(suite))
        self.assertEqual(len(flat), 3)

    def test_flatten_nested_suites(self):
        sys.path.append(support_file('scenario/slow'))
        import test_slow as mod

        suite = unittest.TestSuite()
        suite.addTest(mod.TestSlow('test_ok'))
        suite.addTest(mod.TestSlow('test_fail'))
        suite.addTest(mod.TestSlow('test_err'))

        suite2 = unittest.TestSuite()
        suite2.addTest(suite)

        flat = list(self.plugin._flatten(suite2))
        self.assertEqual(len(flat), 3)

    def test_flatten_respects_module_fixtures(self):
        sys.path.append(support_file('scenario/module_fixtures'))
        import test_mf_testcase as mod

        suite = unittest.TestSuite()
        suite.addTest(mod.Test('test_1'))
        suite.addTest(mod.Test('test_2'))

        flat = list(self.plugin._flatten(suite))
        self.assertEqual(flat, ['test_mf_testcase'])

    def test_flatten_respects_class_fixtures(self):
        sys.path.append(support_file('scenario/class_fixtures'))
        import test_cf_testcase as mod

        suite = unittest.TestSuite()
        suite.addTest(mod.Test('test_1'))
        suite.addTest(mod.Test('test_2'))
        suite.addTest(mod.Test2('test_1'))
        suite.addTest(mod.Test2('test_2'))
        suite.addTest(mod.Test3('test_3'))

        flat = list(self.plugin._flatten(suite))
        self.assertEqual(flat, ['test_cf_testcase.Test2.test_1',
                                'test_cf_testcase.Test2.test_2',
                                'test_cf_testcase.Test',
                                'test_cf_testcase.Test3',
                                ])

    def test_conn_prep(self):
        self.plugin.bind_host = None
        (parent_conn, child_conn) = self.plugin._prepConns()
        (parent_pipe, child_pipe) = multiprocessing.Pipe()
        self.assertIsInstance(parent_conn, type(parent_pipe))
        self.assertIsInstance(child_conn, type(child_pipe))

        self.plugin.bind_host = "127.0.0.1"
        self.plugin.bind_port = 0
        (parent_conn, child_conn) = self.plugin._prepConns()
        self.assertIsInstance(parent_conn, connection.Listener)
        self.assertIsInstance(child_conn, tuple)
        self.assertEqual(parent_conn.address, child_conn[:2])

    def test_conn_accept(self):
        (parent_conn, child_conn) = multiprocessing.Pipe()
        self.assertEqual(self.plugin._acceptConns(parent_conn), parent_conn)

        listener = connection.Listener(('127.0.0.1', 0))
        with self.assertRaises(RuntimeError):
            self.plugin._acceptConns(listener)

        def fake_client(address):
            client = connection.Client(address)
            time.sleep(10)
            client.close()

        t = threading.Thread(target=fake_client, args=(listener.address,))
        t.start()
        conn = self.plugin._acceptConns(listener)
        self.assertTrue(hasattr(conn, "send"))
        self.assertTrue(hasattr(conn, "recv"))


class TestProcserver(FunctionalTestCase):

    def setUp(self):
        super(TestProcserver, self).setUp()
        self.session = session.Session()

    def test_dispatch_tests_receive_events(self):
        ssn = {
            'config': self.session.config,
            'verbosity': 1,
            'startDir': support_file('scenario/tests_in_package'),
            'topLevelDir': support_file('scenario/tests_in_package'),
            'logLevel': 100,
            'pluginClasses': [discovery.DiscoveryLoader,
                              testcases.TestCaseLoader,
                              buffer.OutputBufferPlugin]

        }
        conn = Conn(['pkg1.test.test_things.SomeTests.test_ok',
                     'pkg1.test.test_things.SomeTests.test_failed'])
        procserver(ssn, conn)

        # check conn calls
        expect = [('pkg1.test.test_things.SomeTests.test_ok',
                   [('startTest', {}),
                    ('setTestOutcome', {'outcome': 'passed'}),
                    ('testOutcome', {'outcome': 'passed'}),
                    ('stopTest', {})]
                   ),
                  ('pkg1.test.test_things.SomeTests.test_failed',
                   [('startTest', {}),
                    ('setTestOutcome', {
                     'outcome': 'failed',
                     'expected': False,
                     'metadata': {'stdout': 'Hello stdout\n'}}),
                    ('testOutcome', {
                     'outcome': 'failed',
                     'expected': False,
                     'metadata': {'stdout': 'Hello stdout\n'}}),
                    ('stopTest', {})]
                   ),
                  ]
        for val in conn.sent:
            if val is None:
                break
            test, events = val
            exp_test, exp_events = expect.pop(0)
            self.assertEqual(test, exp_test)
            for method, event in events:
                exp_meth, exp_attr = exp_events.pop(0)
                self.assertEqual(method, exp_meth)
                for attr, val in exp_attr.items():
                    self.assertEqual(getattr(event, attr), val)


class MPPluginTestRuns(FunctionalTestCase):

    def test_tests_in_package(self):
        proc = self.runIn(
            'scenario/tests_in_package',
            '-v',
            '--plugin=nose2.plugins.mp',
            '-N=2')
        self.assertTestRunOutputMatches(proc, stderr='Ran 25 tests')
        self.assertEqual(proc.poll(), 1)

    def test_package_in_lib(self):
        proc = self.runIn(
            'scenario/package_in_lib',
            '-v',
            '--plugin=nose2.plugins.mp',
            '-N=2')
        self.assertTestRunOutputMatches(proc, stderr='Ran 3 tests')
        self.assertEqual(proc.poll(), 1)

    def test_module_fixtures(self):
        proc = self.runIn(
            'scenario/module_fixtures',
            '-v',
            '--plugin=nose2.plugins.mp',
            '-N=2')
        self.assertTestRunOutputMatches(proc, stderr='Ran 5 tests')
        self.assertEqual(proc.poll(), 0)

    def test_class_fixtures(self):
        proc = self.runIn(
            'scenario/class_fixtures',
            '-v',
            '--plugin=nose2.plugins.mp',
            '-N=2')
        self.assertTestRunOutputMatches(proc, stderr='Ran 7 tests')
        self.assertEqual(proc.poll(), 0)

    def test_large_number_of_tests_stresstest(self):
        proc = self.runIn(
            'scenario/many_tests',
            '-v',
            '--plugin=nose2.plugins.mp',
            '--plugin=nose2.plugins.loader.generators',
            '-N=1')
        self.assertTestRunOutputMatches(proc, stderr='Ran 600 tests')
        self.assertEqual(proc.poll(), 0)

    def test_socket_stresstest(self):
        proc = self.runIn(
            'scenario/many_tests_socket',
            '-v',
            '-c scenario/many_test_socket/nose2.cfg',
            '--plugin=nose2.plugins.mp',
            '--plugin=nose2.plugins.loader.generators',
            '-N=1')
        self.assertTestRunOutputMatches(proc, stderr='Ran 600 tests')
        self.assertEqual(proc.poll(), 0)

