"""
Map exceptions to test outcomes.

This plugin implements :func:`setTestOutcome` to enable simple mapping
of exception classes to existing test outcomes.

By setting a list of exception classes in a nose2 config file, you can
configure exceptions that would otherwise be treated as test errors,
to be treated as failures or skips instead:

.. code-block :: ini

  [outcomes]
  always-on = True
  treat-as-fail = NotImplementedError
  treat-as-skip = TodoError
                  IOError

"""

from nose2.events import Plugin


__unittest = True


class Outcomes(Plugin):

    """Map exceptions to other test outcomes"""

    configSection = 'outcomes'
    commandLineSwitch = (None, 'set-outcomes',
                         'Treat some configured exceptions as failure or skips')

    def __init__(self):
        self.treatAsFail = set(self.config.as_list('treat-as-fail', []))
        self.treatAsSkip = set(self.config.as_list('treat-as-skip', []))

    def setTestOutcome(self, event):
        """Update outcome, exc_info and reason based on configured mappings"""
        if event.exc_info:
            ec, ev, tb = event.exc_info
            classname = ec.__name__
            if classname in self.treatAsFail:
                short, long_ = self.labels(classname)
                self._setOutcome(event, 'failed', short, long_)
            elif classname in self.treatAsSkip:
                short, long_ = self.labels(classname, upper=False)
                self._setOutcome(
                    event, 'skipped', short, "%s: '%s'" % (long_, ev), str(ev))

    def labels(self, label, upper=True):
        if upper:
            label = label.upper()
        else:
            label = label.lower()
        short = label[0]
        return short, label

    def _setOutcome(self, event, outcome, shortLabel, longLabel, reason=None):
        event.outcome = outcome
        event.shortLabel = shortLabel
        event.longLabel = longLabel
        if reason:
            event.exc_info = None
            event.reason = reason
