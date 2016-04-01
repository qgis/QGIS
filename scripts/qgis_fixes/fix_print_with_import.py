from libfuturize.fixes.fix_print_with_import import FixPrintWithImport as FixPrintWithImportOrig
from lib2to3.fixer_util import Node, Leaf, syms


class FixPrintWithImport(FixPrintWithImportOrig):

    def match(self, node):
        isPrint = super(FixPrintWithImport, self).match(node)
        if isPrint and
            len(node.children) == 2 and \
                    isinstance(node.children[0], Leaf) and \
                    isinstance(node.children[1], Node) and node.children[1].type == syms.atom and \
                    isinstance(node.children[1].children[0], Leaf) and node.children[1].children[0].value == '(' and \
                    isinstance(node.children[1].children[-1], Leaf) and node.children[1].children[-1].value == ')':
            return False

        return ok
