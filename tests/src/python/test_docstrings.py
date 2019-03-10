import doctest
import inspect
import unittest
import qgis
import glob
import re
from doctest import (
    DocTestFinder,
    DebugRunner,
    DocTestRunner,
    OutputChecker,
    TestResults,
)

from qgis.testing import (
    start_app,
    unittest,
)
from qgis.testing.mocked import get_iface

qgis_app = start_app()

# Import required globals
from qgis.core import *
from qgis.gui import *
from qgis.utils import *
from qgis.PyQt.QtCore import *
from qgis.PyQt.QtGui import *
iface = get_iface()

INCLUDES = ('QgsHighlight',)


class CodeBlockParser(doctest.DocTestParser):
    _EXAMPLE_RE = re.compile(r'''
        ^\.\.[ ]code-block::[ ]python\n
        \n?
        (?P<source>
            (?:(?![ ]*$)    # Not a blank line
               (?P<indent>[ ]*).+$\n?       # But any other line stating with 3 spaces
            )
        *)
        ''', re.MULTILINE | re.VERBOSE)

    def _parse_example(self, m, name, lineno):
        """
        Given a regular expression match from `_EXAMPLE_RE` (`m`),
        return a pair `(source, want)`, where `source` is the matched
        example's source code (with prompts and indentation stripped);
        and `want` is the example's expected output (with indentation
        stripped).

        `name` is the string's name, and `lineno` is the line number
        where the example starts; both are used for error messages.
        """
        # Get the example's indentation level.
        indent = len(m.group('indent'))

        # Divide source into lines; check that they're properly
        # indented; and then strip their indentation & prompts.
        source_lines = m.group('source').split('\n')
        # self._check_prompt_blank(source_lines, indent, name, lineno)
        # self._check_prefix(source_lines[1:], ' '*indent + '.', name, lineno)
        source = '\n'.join([sl[indent:] for sl in source_lines])

        # For now we do not check output
        want = ''

        # If `want` contains a traceback message, then extract it.
        exc_msg = None

        # Extract options from the source.
        options = self._find_options(source, name, lineno)

        return source, options, want, exc_msg


class CodeBlockOutputChecker(OutputChecker):

    def check_output(self, want, got, optionflags):
        return True


class TestDocstrings(unittest.TestCase):

    def setUp(self):
        # HACK HACK HACK
        # doctest compiles its snippets with type 'single'. That is nice
        # for doctest examples but unusable for multi-statement code such
        # as setup code -- to be able to use doctest error reporting with
        # that code nevertheless, we monkey-patch the "compile" it uses.
        doctest.compile = self.compile  # type: ignore

    def tearDown(self):
        """Run after each test."""
        doctest.compile = compile

    def compile(self, code, name, type, flags, dont_inherit):  # spellok
        return compile(code, name, "exec", flags, dont_inherit)  # spellok

    def _to_test(self, m, name):
        to_test = {}
        for valname, val in getattr(m, '__dict__', {}).items():
            if (
                    valname in INCLUDES
                    and inspect.isclass(val)
                    and val.__module__ == name):
                to_test[valname] = val
        return to_test

    def _testmod(self, m=None, name=None, globs=None, verbose=None,
                 report=True, optionflags=0, extraglobs=None,
                 raise_on_error=False):

        m.__test__ = self._to_test(m, name)

        # Find, parse, and run all tests in the given module.
        finder = DocTestFinder(parser=CodeBlockParser())
        checker = CodeBlockOutputChecker()
        if raise_on_error:
            runner = DebugRunner(checker=checker, verbose=verbose, optionflags=optionflags)
        else:
            runner = DocTestRunner(checker=checker, verbose=verbose, optionflags=optionflags)

        for test in finder.find(m, name, globs=globs, extraglobs=extraglobs):
            runner.run(test)

        if report:
            runner.summarize()

        self.assertEqual(runner.failures, 0,
                         "{} tests fails out of {}".format(runner.failures, runner.tries))

    def test_docstrings(self):
        self._testmod(qgis.core, name='qgis._core', globs=globals(), verbose=True)
        self._testmod(qgis.gui, name='qgis._gui', globs=globals(), verbose=True)


if __name__ == '__main__':
    unittest.main()
