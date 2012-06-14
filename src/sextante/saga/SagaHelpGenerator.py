#!/usr/bin/python

import saga_api as saga
import os

class Library:
    def __init__(self, filename):
        self.sagalib = saga.CSG_Module_Library(saga.CSG_String(str(filename)))
        if not self.sagalib.is_Valid():
            raise ImportError(filename)
        self.libname = filename.split(os.sep)[-1].split(".")[0]
        if self.libname.startswith("lib"):
            self.libname = self.libname[3:]
        self.name = self.sagalib.Get_Name().c_str()
        self._modules = None
    def modules(self):
        if self._modules is not None:
            return self._modules
        self._modules = list()
        for i in range(self.sagalib.Get_Count()):
            try:
                self._modules.append(Module(self.sagalib, i))
            except ImportError:
                pass
        return self._modules
    def __del__(self):
        self.sagalib.Destroy()

class Module:
    def __init__(self, lib, i):
        self.module = lib.Get_Module(i)
        if not self.module:
            raise ImportError("Module #%i is invalid" % i)
        if self.module.is_Interactive():
            raise ImportError("Ignoring interactive module")
        self.name = self.module.Get_Name()
        self.grid = self.module.is_Grid()
        if self.module.is_Grid():
            self.module = lib.Get_Module_Grid(i)
        self.description = self.module.Get_Description()
        self.author = self.module.Get_Author()
        self._parameters = None
    def parameters(self):
        if self._parameters is not None:
            return self._parameters
        params = list()
        params.append(self.module.Get_Parameters())
        for i in range(self.module.Get_Parameters_Count()):
            params.append(self.module.Get_Parameters(i))
        self._parameters = list()
        for p in params:
            for j in range(p.Get_Count()):
                try:
                    self._parameters.append(Parameter(p, j))
                except:
                    pass
        return self._parameters
            

class Parameter:
    def __init__(self, params, i):
        self.parameter = params.Get_Parameter(i)
        self.name = self.parameter.Get_Name()
        self.description = self.parameter.Get_Description()
        self.typeName = self.parameter.Get_Type_Name()
        if self.parameter.is_Output():
            self.typeName = "Output " + self.typeName
        if self.parameter.is_Input():
            self.typeName = "Input " + self.typeName
        typ = self.parameter.Get_Type()
        self.minimum = None
        self.maximum = None
        if  (typ == saga.PARAMETER_TYPE_Int)    or \
            (typ == saga.PARAMETER_TYPE_Double) or \
            (typ == saga.PARAMETER_TYPE_Degree) or \
            (typ == saga.PARAMETER_TYPE_Range):
                parameterValue = self.parameter.asValue()
                if parameterValue.has_Minimum():
                    self.minimum = parameterValue.Get_Minimum()
                if parameterValue.has_Maximum():
                    self.maximum = parameterValue.Get_Maximum()
        self.choices = None
        if typ == saga.PARAMETER_TYPE_Choice:     
            parameterChoice = self.parameter.asChoice()
            self.choices = [parameterChoice.Get_Item(i) for i in
                range(parameterChoice.Get_Count())]
        
        
def getLibraryPaths(userPath = None):
    try:
        paths = os.environ['MLB_PATH'].split(':')
    except KeyError:
        paths = ['/usr/lib/saga/', '/usr/local/lib/saga/']
        noMLBpath = True
    if userPath:
        paths = [userPath] + paths
    print "Looking for libraries in " + ', '.join(paths)
    for p in paths:
        if os.path.exists(p):
            return [os.path.join(p, fn) for fn in os.listdir(p)]
    if noMLBpath:
        print "Warning: MLB_PATH not set."
    return []

def qgisizeString(s):
    try:
        s = str(s)
        s = str.replace(s, "Gridd", "Raster")
        s = str.replace(s, "Grid", "Raster")
        s = str.replace(s, "gridd", "raster")
        s = str.replace(s, "grid", "raster")
    except:
        # Some unicode characters seem to produce exceptions.
        # Just ignore those cases.
        pass
    return s

def writeHTML(path, mod):
    docs = unicode()
    docs += "<h1 class='module'>%s</h1>\n" % mod.name
    docs += "<div class='author'>%s</div>\n" % mod.author
    docs += "<div class='description'>%s</div>\n" % mod.description.replace('\n', '<br/>\n')
    if mod.parameters():
        docs += "<h2>Parameters</h2>\n<dl class='parameters'>\n"
        for p in mod.parameters():
            constraints = list()
            if p.minimum:
                constraints.append("Minimum: " + str(p.minimum))
            if p.maximum:
                constraints.append("Maximum: " + str(p.maximum))
            if p.choices:
                constraints.append("Available choices: " + ', '.join(p.choices))
            
            docs += "\t<dt>%s <div class='type'>%s</div></dt>" % (p.name, p.typeName)
            docs += "<dd>%s <div class='constraints'>%s</div></dd>\n" % (p.description, '; '.join(constraints))
        docs += "</dl>"
    out = open(path, 'w')
    out.write('<html>\n<head><link rel="stylesheet" type="text/css" href="help.css" /></head>\n<body>\n')
    out.write(docs.encode('utf-8'))
    out.write('\n</body></html>\n')
    out.close()

if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(description='Generate SAGA documentation in HTML form.')
    parser.add_argument('dest', metavar='DESTINATION', type = str, help='HTML output path.')
    parser.add_argument('-l', dest='libpath', help='Location of SAGA libraries.')

    args = parser.parse_args()
    
    libs = list()
    paths = getLibraryPaths(args.libpath)
    for p in paths:
        try:
            libs.append(Library(p))
        except ImportError:
            pass
            
    if not libs:
        print "No saga libraries found"
        exit(1)
        
    print "%i libraries loaded." % len(libs)
    for lib in libs:
        mods = lib.modules()
        print "%s (%i modules):" % (lib.name, len(mods))
        for mod in mods:
            path = args.dest + os.sep + mod.name.replace(" ", '').replace("/", '') + ".html"
            print '\t', mod.name,
            writeHTML(path, mod)
            print '\t-> ', path
