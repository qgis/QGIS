"""
Stop the test run after the first error or failure.

This plugin implements :func:`testOutcome` and sets
``event.result.shouldStop`` if it sees an outcome with exc_info that
is not expected.

"""

from nose2 import events


__unittest = True


class FailFast(events.Plugin):

    """Stop the test run after error or failure"""
    commandLineSwitch = (
        'F', 'fail-fast', 'Stop the test run after the first error or failure')

    def testOutcome(self, event):
        """Stop on unexpected error or failure"""
        if event.exc_info and not event.expected:
            event.result.shouldStop = True
