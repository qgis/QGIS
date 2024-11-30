"""Fixer for import of generated UIs
"""

# Local imports
from lib2to3 import fixer_base
from lib2to3.fixer_util import FromImport, Leaf, Node, syms
from lib2to3.fixes.fix_import import FixImport


class FixUiimport(fixer_base.BaseFix):
    BM_compatible = True

    PATTERN = """
    import_from< 'from' imp=any 'import' ['('] any [')'] >
    |
    import_name< 'import' imp=any >
    """

    def transform(self, node, results):
        imp = results.get("imp")

        if node.type == syms.import_from:
            # Some imps are top-level (e.g., 'import ham')
            # some are first level (e.g., 'import ham.eggs')
            # some are third level (e.g., 'import ham.eggs as spam')
            # Hence, the loop
            while not hasattr(imp, "value"):
                imp = imp.children[0]
            if imp.value.startswith("ui_"):
                imp.value = "." + imp.value
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
