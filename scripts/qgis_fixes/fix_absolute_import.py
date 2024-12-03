from libfuturize.fixes.fix_absolute_import import (
    FixAbsoluteImport as FixAbsoluteImportOrig,
)


class FixAbsoluteImport(FixAbsoluteImportOrig):

    def probably_a_local_import(self, imp_name):
        if imp_name.startswith("PyQt"):
            return False
        if imp_name == "AlgorithmsTestBase":
            return False
        return super().probably_a_local_import(imp_name)
