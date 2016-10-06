"""Fixer for import of generated UIs
"""

# Local imports
from lib2to3 import fixer_base
from lib2to3.fixes.fix_import import FixImport
from lib2to3.fixer_util import FromImport, Node, Leaf, syms


class FixUiimport(fixer_base.BaseFix):
    BM_compatible = True

    PATTERN = """
    import_from< 'from' imp=any 'import' ['('] any [')'] >
    |
    import_name< 'import' imp=any >
    """

    def transform(self, node, results):
        imp = results.get('imp')

        if node.type == syms.import_from:
            # Some imps are top-level (eg: 'import ham')
            # some are first level (eg: 'import ham.eggs')
            # some are third level (eg: 'import ham.eggs as spam')
            # Hence, the loop
            while not hasattr(imp, 'value'):
                imp = imp.children[0]
            if imp.value.startswith("ui_"):
                imp.value = u"." + imp.value
                imp.changed()
        else:
            first = imp
            if isinstance(imp, Node):
                first = next(imp.leaves())

            if not isinstance(first, Leaf):
                return

            if not first.value.startswith("ui_"):
                return

            new = FromImport(".", [imp])
            new.prefix = node.prefix
            return new
