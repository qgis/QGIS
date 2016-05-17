"""Test testid plugin."""
import os.path
import pickle

from six import StringIO

from nose2 import session
from nose2.events import ReportTestEvent
from nose2.plugins import testid
from nose2.tests._common import (FakeStartTestEvent, FakeLoadFromNameEvent,
                                 FakeLoadFromNamesEvent, TestCase)


class UnitTestTestId(TestCase):

    """Test class TestId.

    Tests are carried out in a temporary directory, since TestId stores state
    to file. The temporary directory is removed after testing.
    """
    tags = ['unit']
    _RUN_IN_TEMP = True

    def setUp(self):
        super(UnitTestTestId, self).setUp()
        self.stream = StringIO()
        self.session = session.Session()
        self.plugin = testid.TestId(session=self.session)

    def test___init__(self):
        """Test the __init__ method."""
        plug = self.plugin
        # Test attributes
        for name, exp_val in [(
            'configSection', 'testid'), ('commandLineSwitch',
                                         ('I', 'with-id', 'Add test ids to output')), ('idfile',
                                                                                       os.path.abspath(
                                                                                           '.noseids')), ('ids', {}), ('tests', {}),
                ('id', 0)]:
            try:
                val = getattr(plug, name)
            except AttributeError:
                self.fail(
                    'TestId instance doesn\'t have attribute %s' % (name,))
            self.assertEqual(val, exp_val, 'Attribute %s should have value '
                             '\'%s\', but has value %s' % (name, exp_val, val))

    def test_start_test(self):
        """Test reportStartTest method."""
        self.session.verbosity = 2
        event = ReportTestEvent(FakeStartTestEvent(self), self.stream)
        plug = self.plugin
        plug.reportStartTest(event)

        self.assertEqual(plug.id, 1)
        test_id = self.id()
        self.assertEqual(plug.ids, {1: test_id})
        self.assertEqual(plug.tests, {test_id: 1})
        self.assertEqual(self.stream.getvalue(), '#1 ')

    def test_start_test_twice(self):
        """Test calling reportStartTest twice."""
        self.session.verbosity = 2
        event = ReportTestEvent(FakeStartTestEvent(self), self.stream)
        plug = self.plugin
        plug.reportStartTest(event)
        plug.reportStartTest(event)

        self.assertEqual(plug.id, 1)
        test_id = self.id()
        self.assertEqual(plug.ids, {1: test_id})
        self.assertEqual(plug.tests, {test_id: 1})
        self.assertEqual(self.stream.getvalue(), '#1 #1 ')

    def test_stop_test_run(self):
        """Test stopTestRun method."""
        plug = self.plugin
        plug.reportStartTest(
            ReportTestEvent(FakeStartTestEvent(self), self.stream))
        plug.stopTestRun(None)

        fh = open(plug.idfile, 'rb')
        try:
            data = pickle.load(fh)
        finally:
            fh.close()
        self.assertEqual(data, {'ids': plug.ids, 'tests': plug.tests})

    def test_load_tests_from_name(self):
        """Test loadTestsFromName method."""
        plug = self.plugin
        # By first starting/stopping a test, an ID is assigned by the plugin
        plug.reportStartTest(
            ReportTestEvent(FakeStartTestEvent(self), self.stream))
        plug.stopTestRun(None)
        event = FakeLoadFromNameEvent('1')
        plug.loadTestsFromName(event)

        # The numeric ID should be translated to this test's ID
        self.assertEqual(event.name, self.id())

    def test_load_tests_from_name_no_ids(self):
        """Test calling loadTestsFromName when no IDs have been saved."""
        plug = self.plugin
        event = FakeLoadFromNameEvent('1')
        plug.loadTestsFromName(event)

        # The event's name should be unchanged, since no IDs should be mapped
        self.assertEqual(event.name, '1')

    def test_load_tests_from_names(self):
        """Test loadTestsFromNames method."""
        plug = self.plugin
        # By first starting/stopping a test, an ID is assigned by the plugin
        plug.reportStartTest(
            ReportTestEvent(FakeStartTestEvent(self), self.stream))
        plug.stopTestRun(None)
        event = FakeLoadFromNamesEvent(['1', '2'])
        plug.loadTestsFromNames(event)

        name1, name2 = event.names
        # The first numeric ID should be translated to this test's ID
        self.assertEqual(name1, self.id())
        # The second one should not have a match
        self.assertEqual(name2, '2')
