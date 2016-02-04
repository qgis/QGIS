"""
Allow easy test selection with test ids.

Assigns (and, in verbose mode, prints) a sequential test id for each
test executed. Ids can be fed back in as test names, and this plugin
will translate them back to full test names. Saves typing!

This plugin implements :func:`reportStartTest`,
:func:`loadTestsFromName`, :func:`loadTestsFromNames` and
:func:`stopTest`.

"""
import os
import pickle
import re

from nose2.events import Plugin
from nose2 import util


__unittest = True


class TestId(Plugin):

    """Allow easy test select with ids"""

    configSection = 'testid'
    commandLineSwitch = ('I', 'with-id', 'Add test ids to output')
    idpat = re.compile(r'(\d+)')

    def __init__(self):
        self.idfile = self.config.as_str('id-file', '.noseids')
        self.ids = {}
        self.tests = {}
        if not os.path.isabs(self.idfile):
            # FIXME expand-user?
            self.idfile = os.path.join(os.getcwd(), self.idfile)
        self.id = 0
        self._loaded = False

    def nextId(self):
        """Increment ID and return it."""
        self.id += 1
        return self.id

    def reportStartTest(self, event):
        """Record and possibly output test id"""
        testid = util.test_name(event.testEvent.test)
        if testid not in self.tests:
            id_ = self.nextId()
            self.ids[id_] = testid
            self.tests[testid] = id_
        else:
            id_ = self.tests[testid]
        event.metadata['testid'] = id_
        if self.session.verbosity > 1:
            event.stream.write('#%s ' % id_)

    def loadTestsFromName(self, event):
        """Load tests from a name that is an id

        If the name is a number, it might be an ID assigned by us. If we can
        find a test to which we have assigned that ID, event.name is changed to
        the test's real ID. In this way, tests can be referred to via sequential
        numbers.
        """
        testid = self._testNameFromId(event.name)
        if testid is not None:
            event.name = testid

    def loadTestsFromNames(self, event):
        """Translate test ids into test names"""
        for i, name in enumerate(event.names[:]):
            testid = self._testNameFromId(name)
            if testid is not None:
                event.names[i] = testid

    def stopTestRun(self, event):
        """Write testids file"""
        with open(self.idfile, 'wb') as fh:
            pickle.dump({'ids': self.ids, 'tests': self.tests}, fh)

    def loadIds(self):
        """Load previously pickled 'ids' and 'tests' attributes."""
        if self._loaded:
            return

        try:
            with open(self.idfile, 'rb') as fh:
                data = pickle.load(fh)
        except EnvironmentError:
            self._loaded = True
            return

        if 'ids' in data:
            self.ids = data['ids']
        if 'tests' in data:
            self.tests = data['tests']
        self.id = max(self.ids.keys())
        self._loaded = True

    def _testNameFromId(self, name):
        """Try to translate one of our IDs to real test ID."""
        m = self.idpat.match(name)
        if m is None:
            return None

        id_ = int(m.groups()[0])

        self.loadIds()
        # Translate to test's real ID
        try:
            return self.ids[id_]
        except KeyError:
            return None
