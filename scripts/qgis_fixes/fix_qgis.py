"""Fix changes imports of urllib which are now incompatible.
   This is rather similar to fix_imports, but because of the more
   complex nature of the fixing for urllib, it has its own fixer.
"""
# Author: Nick Edds

# Local imports
from lib2to3.fixes.fix_imports import alternates, FixImports
from lib2to3 import fixer_base
from lib2to3.fixer_util import (Name, Comma, FromImport, Newline,
                                find_indentation, Node, syms)

MAPPING = {
    "qgis.core": {
        "QgsWKBTypes": "QgsWkbTypes",
        "QGis": "Qgis"
    }
}


def build_pattern():
    yield """
          import_from< 'from' imp=any 'import' ['('] any [')'] >
          |
          import_name< 'import' imp=any >
                  """


class FixQgis(FixImports):

    def build_pattern(self):
        return "|".join(build_pattern())

    def transform_imp(self, node, results):
        import_mod = results.get("imp")
        module_name = str(import_mod)[1:]
        if module_name in MAPPING:
            mapping = MAPPING[module_name]
            for member in node.children[4].children:
                if member.type == 1 and member.value in mapping:
                    member.value = mapping[member.value]
                    member.replace(member)

    def transform(self, node, results):
        # Uncomment for debugging
        # print('{}\n{}').format(node, results)
        if results.get("imp"):
            self.transform_imp(node, results)
