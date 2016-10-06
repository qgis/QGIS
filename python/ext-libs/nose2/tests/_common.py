"""Common functionality."""
import os.path
import tempfile
import shutil
import subprocess
import sys

import six

from nose2.compat import unittest
from nose2 import discover, util


HERE = os.path.abspath(os.path.dirname(__file__))
SUPPORT = os.path.join(HERE, 'functional', 'support')


class TestCase(unittest.TestCase):

    """TestCase extension.

    If the class variable _RUN_IN_TEMP is True (default: False), tests will be
    performed in a temporary directory, which is deleted afterwards.
    """
    _RUN_IN_TEMP = False

    def setUp(self):
        super(TestCase, self).setUp()

        if self._RUN_IN_TEMP:
            self._orig_dir = os.getcwd()
            work_dir = self._work_dir = tempfile.mkdtemp()
            os.chdir(self._work_dir)
            # Make sure it's possible to import modules from current directory
            sys.path.insert(0, work_dir)

    def tearDown(self):
        super(TestCase, self).tearDown()

        if self._RUN_IN_TEMP:
            os.chdir(self._orig_dir)
            shutil.rmtree(self._work_dir, ignore_errors=True)


class FunctionalTestCase(unittest.TestCase):
    tags = ['functional']

    def assertTestRunOutputMatches(self, proc, stdout=None, stderr=None):
        cmd_stdout, cmd_stderr = None, None
        try:
            cmd_stdout, cmd_stderr = self._output[proc.pid]
        except AttributeError:
            self._output = {}
        except KeyError:
            pass
        if cmd_stdout is None:
            cmd_stdout, cmd_stderr = proc.communicate()
            self._output[proc.pid] = cmd_stdout, cmd_stderr
        testf = self.assertRegex if hasattr(self, 'assertRegex') \
            else self.assertRegexpMatches
        if stdout:
            testf(util.safe_decode(cmd_stdout), stdout)
        if stderr:
            testf(util.safe_decode(cmd_stderr), stderr)

    def runIn(self, testdir, *args, **kw):
        return run_nose2(*args, cwd=testdir, **kw)

    def runModuleAsMain(self, testmodule):
        return run_module_as_main(testmodule)


class _FakeEventBase(object):

    """Baseclass for fake Events."""

    def __init__(self):
        self.handled = False
        self.version = '0.1'
        self.metadata = {}


class FakeHandleFileEvent(_FakeEventBase):

    """Fake HandleFileEvent."""

    def __init__(self, name):
        super(FakeHandleFileEvent, self).__init__()

        self.loader = Stub()  # FIXME
        self.name = name
        self.path = os.path.split(name)[1]
        self.extraTests = []


class FakeStartTestEvent(_FakeEventBase):

    """Fake StartTestEvent."""

    def __init__(self, test):
        super(FakeStartTestEvent, self).__init__()
        self.test = test
        self.result = test.defaultTestResult()
        import time
        self.startTime = time.time()


class FakeLoadFromNameEvent(_FakeEventBase):

    """Fake LoadFromNameEvent."""

    def __init__(self, name):
        super(FakeLoadFromNameEvent, self).__init__()
        self.name = name


class FakeLoadFromNamesEvent(_FakeEventBase):

    """Fake LoadFromNamesEvent."""

    def __init__(self, names):
        super(FakeLoadFromNamesEvent, self).__init__()
        self.names = names


class FakeStartTestRunEvent(_FakeEventBase):

    """Fake StartTestRunEvent"""

    def __init__(self, runner=None, suite=None, result=None, startTime=None,
                 executeTests=None):
        super(FakeStartTestRunEvent, self).__init__()
        self.suite = suite
        self.runner = runner
        self.result = result
        self.startTime = startTime
        self.executeTests = executeTests


class Stub(object):

    """Stub object for use in tests"""

    def __getattr__(self, attr):
        return Stub()

    def __call__(self, *arg, **kw):
        return Stub()


def support_file(*path_parts):
    return os.path.abspath(os.path.join(SUPPORT, *path_parts))


def run_nose2(*nose2_args, **nose2_kwargs):
    if 'cwd' in nose2_kwargs:
        cwd = nose2_kwargs.pop('cwd')
        if not os.path.isabs(cwd):
            nose2_kwargs['cwd'] = support_file(cwd)
    return NotReallyAProc(nose2_args, **nose2_kwargs)


def run_module_as_main(test_module):
    if not os.path.isabs(test_module):
        test_module = support_file(test_module)
    return subprocess.Popen([sys.executable, test_module],
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE)


class NotReallyAProc(object):

    def __init__(self, args, cwd=None, **kwargs):
        self.args = args
        self.chdir = cwd
        self.kwargs = kwargs
        self.result = None

    def __enter__(self):
        self._stdout = sys.__stdout__
        self._stderr = sys.__stderr__
        self.cwd = os.getcwd()
        if self.chdir:
            os.chdir(self.chdir)
        self.stdout = sys.stdout = sys.__stdout__ = six.StringIO()
        self.stderr = sys.stderr = sys.__stderr__ = six.StringIO()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        sys.stdout = sys.__stdout__ = self._stdout
        sys.stderr = sys.__stderr__ = self._stderr
        if self.chdir:
            os.chdir(self.cwd)
        return False

    def communicate(self):
        with self:
            try:
                self.result = discover(
                    argv=('nose2',) + self.args, exit=False,
                    **self.kwargs)
            except SystemExit as e:
                pass
            return self.stdout.getvalue(), self.stderr.getvalue()

    @property
    def pid(self):
        return id(self)

    def poll(self):
        if self.result is None:
            return 1
        return not self.result.result.wasSuccessful()


class RedirectStdStreams(object):

    """
    Context manager that replaces the stdin/out streams with StringIO
    buffers.
    """

    def __init__(self):
        self.stdout = six.StringIO()
        self.stderr = six.StringIO()

    def __enter__(self):
        self.old_stdout, self.old_stderr = sys.stdout, sys.stderr
        self.old_stdout.flush()
        self.old_stderr.flush()
        sys.stdout, sys.stderr = self.stdout, self.stderr
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.stdout.flush()
        self.stderr.flush()
        sys.stdout = self.old_stdout
        sys.stderr = self.old_stderr


# mock multprocessing Connection
class Conn(object):

    def __init__(self, items):
        self.items = items
        self.sent = []
        self.closed = False

    def recv(self):
        if self.closed:
            raise EOFError("closed")
        try:
            return self.items.pop(0)
        except:
            raise EOFError("EOF")

    def send(self, item):
        self.sent.append(item)

    def close(self):
        self.closed = True
