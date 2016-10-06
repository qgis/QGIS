"""
Profile test execution using hotshot.

This plugin implements :func:`startTestRun` and replaces
``event.executeTests`` with :meth:`hotshot.Profile.runcall`. It
implements :func:`beforeSummaryReport` to output profiling information
before the final test summary time. Config file options ``filename``,
``sort`` and ``restrict`` can be used to change where profiling
information is saved and how it is presented.

"""
try:
    import hotshot
    from hotshot import stats
except ImportError:
    hotshot, stats = None, None
import logging
import os
import tempfile

from nose2 import events, util

log = logging.getLogger(__name__)
__unittest = True


class Profiler(events.Plugin):

    """Profile the test run"""

    configSection = 'profiler'
    commandLineSwitch = ('P', 'profile', 'Run tests under profiler')

    def __init__(self):
        self.pfile = self.config.as_str('filename', '')
        self.sort = self.config.as_str('sort', 'cumulative')
        self.restrict = self.config.as_list('restrict', [])
        self.clean = False
        self.fileno = None

    def register(self):
        """Don't register if hotshot is not found"""
        if hotshot is None:
            log.error("Unable to profile: hotshot module not available")
            return
        super(Profiler, self).register()

    def startTestRun(self, event):
        """Set up the profiler"""
        self.createPfile()
        self.prof = hotshot.Profile(self.pfile)
        event.executeTests = self.prof.runcall

    def beforeSummaryReport(self, event):
        """Output profiling results"""
        # write prof output to stream
        class Stream:

            def write(self, *msg):
                for m in msg:
                    event.stream.write(m)
                    event.stream.write(' ')
                event.stream.flush()
        stream = Stream()
        self.prof.close()
        prof_stats = stats.load(self.pfile)
        prof_stats.sort_stats(self.sort)
        event.stream.writeln(util.ln("Profiling results"))
        tmp = prof_stats.stream
        prof_stats.stream = stream
        try:
            if self.restrict:
                prof_stats.print_stats(*self.restrict)
            else:
                prof_stats.print_stats()
        finally:
            prof_stats.stream = tmp
        self.prof.close()
        event.stream.writeln('')

        if self.clean:
            if self.fileno:
                try:
                    os.close(self.fileno)
                except OSError:
                    pass
            try:
                os.unlink(self.pfile)
            except OSError:
                pass

    def createPfile(self):
        if not self.pfile:
            self.fileno, self.pfile = tempfile.mkstemp()
            self.clean = True
