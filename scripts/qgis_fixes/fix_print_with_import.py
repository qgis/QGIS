from libfuturize.fixes.fix_print_with_import import FixPrintWithImport as FixPrintWithImportOrig
from lib2to3.fixer_util import Node, Leaf, syms, find_indentation

import re


class FixPrintWithImport(FixPrintWithImportOrig):

    def transform(self, node, results):
        if "fix_print_with_import" in node.prefix:
            return node

        r = super(FixPrintWithImport, self).transform(node, results)

        if not r or r == node:
            return r

        if not r.prefix:
            indentation = find_indentation(node)
            r.prefix = "# fix_print_with_import\n" + indentation
        else:
            r.prefix = re.sub('([ \t]*$)', r'\1# fix_print_with_import\n\1', r.prefix)

        return r
