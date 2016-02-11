"""
This plugin implements :func:`startTestRun`, setting a test executor
(``event.executeTests``) that just collects tests without executing
them. To do so it calls result.startTest, result.addSuccess and
result.stopTest for ech test, without calling the test itself.
"""
from nose2.events import Plugin
from nose2.compat import unittest


__unittest = True


class CollectOnly(Plugin):

    """Collect but don't run tests"""

    configSection = 'collect-only'
    commandLineSwitch = (None, 'collect-only',
                         'Collect and output test names, do not run any tests')
    _mpmode = False

    def registerInSubprocess(self, event):
        event.pluginClasses.append(self.__class__)
        self._mpmode = True

    def startTestRun(self, event):
        """Replace event.executeTests"""
        if self._mpmode:
            return
        event.executeTests = self.collectTests

    def startSubprocess(self, event):
        event.executeTests = self.collectTests

    def collectTests(self, suite, result):
        """Collect tests but don't run them"""
        for test in suite:
            if isinstance(test, unittest.TestSuite):
                self.collectTests(test, result)
                continue
            result.startTest(test)
            result.addSuccess(test)
            result.stopTest(test)
