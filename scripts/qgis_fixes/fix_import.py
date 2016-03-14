from lib2to3.fixes.fix_import import FixImport as FixImportOrig


class FixImport(FixImportOrig):

    def probably_a_local_import(self, imp_name):
        if imp_name.startswith(u"PyQt"):
            return False
        return super(FixImport, self).probably_a_local_import(imp_name)
