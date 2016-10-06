"""
For the ``future`` package.

Adds this import line:

    from past.builtins import str as oldstr

at the top and wraps any unadorned string literals 'abc' or explicit byte-string
literals b'abc' in oldstr() calls so the code has the same behaviour on Py3 as
on Py2.6/2.7.
"""

from __future__ import unicode_literals
import re
from lib2to3 import fixer_base
from lib2to3.pgen2 import token
from lib2to3.fixer_util import syms
from libfuturize.fixer_util import (future_import, touch_import_top,
                                    wrap_in_fn_call)


_literal_re = re.compile(r"[^uUrR]?[\'\"]")


class FixOldstrWrap(fixer_base.BaseFix):
    BM_compatible = True
    PATTERN = "STRING"

    def transform(self, node, results):
        if node.type == token.STRING:
            touch_import_top(u'past.types', u'oldstr', node)
            if _literal_re.match(node.value):
                new = node.clone()
                # Strip any leading space or comments:
                # TODO: check: do we really want to do this?
                new.prefix = u''
                new.value = u'b' + new.value
                wrapped = wrap_in_fn_call("oldstr", [new], prefix=node.prefix)
                return wrapped

    def transform(self, node, results):
        expr1, expr2 = results[0].clone(), results[1].clone()
        # Strip any leading space for the first number:
        expr1.prefix = u''
        return wrap_in_fn_call("old_div", expr1, expr2, prefix=node.prefix)


class FixDivisionSafe(fixer_base.BaseFix):
    # BM_compatible = True
    run_order = 4    # this seems to be ignored?

    _accept_type = token.SLASH

    PATTERN = """
    term<(not('/') any)+ '/' ((not('/') any))>
    """

    def match(self, node):
        u"""
        Since the tree needs to be fixed once and only once if and only if it
        matches, then we can start discarding matches after we make the first.
        """
        if (node.type == self.syms.term and 
                    len(node.children) == 3 and
                    match_division(node.children[1])):
            expr1, expr2 = node.children[0], node.children[2]
            return expr1, expr2
        else:
            return False

    def transform(self, node, results):
        future_import(u"division", node)
        touch_import_top(u'past.utils', u'old_div', node)
        expr1, expr2 = results[0].clone(), results[1].clone()
        # Strip any leading space for the first number:
        expr1.prefix = u''
        return wrap_in_fn_call("old_div", expr1, expr2, prefix=node.prefix)

