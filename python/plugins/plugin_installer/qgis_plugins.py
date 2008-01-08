#!/usr/bin/env python
"""
qgis_plugins.py

author: Matthew Perry
license: GPL
date: 2007-Oct-21
"""
import urllib
import sys
import os
import tempfile
import zipfile
from xml.dom import minidom, Node


class unzip:
    """ unzip.py
    Version: 1.1

    Extract a zipfile to the directory provided
    It first creates the directory structure to house the files
    then it extracts the files to it.

    import unzip
    un = unzip.unzip()
    un.extract(r'c:\testfile.zip', 'c:\testoutput')
    

    By Doug Tolton (http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/252508)
    """
    def __init__(self, verbose = True, percent = 10):
        self.verbose = verbose
        self.percent = percent
        
    def extract(self, file, dir):
        if not dir.endswith(':') and not os.path.exists(dir):
            os.mkdir(dir)

        zf = zipfile.ZipFile(file)

        # create directory structure to house files
        self._createstructure(file, dir)

        num_files = len(zf.namelist())
        percent = self.percent
        divisions = 100 / percent
        perc = int(num_files / divisions)

        # extract files to directory structure
        for i, name in enumerate(zf.namelist()):

            if self.verbose == True:
                print "Extracting %s" % name
            elif perc > 0 and (i % perc) == 0 and i > 0:
                complete = int (i / perc) * percent
                print "%s%% complete" % complete

            if not name.endswith('/'):
                outfile = open(os.path.join(dir, name), 'wb')
                outfile.write(zf.read(name))
                outfile.flush()
                outfile.close()


    def _createstructure(self, file, dir):
        self._makedirs(self._listdirs(file), dir)


    def _makedirs(self, directories, basedir):
        """ Create any directories that don't currently exist """
        for dir in directories:
            curdir = os.path.join(basedir, dir)
            if not os.path.exists(curdir):
                os.mkdir(curdir)

    def _listdirs(self, file):
        """ Grabs all the directories in the zip structure
        This is necessary to create the structure before trying
        to extract the file to it. """
        zf = zipfile.ZipFile(file)

        dirs = []

        for name in zf.namelist():
            if name.endswith('/'):
                dirs.append(name)

        if len(dirs) == 0:
          # this means there is no top level directory in the
          # zip file. We'll assume the first entry contains the
          # directory and use it
          entry = zf.namelist()[0]
          dir = entry.split('/')[0]
          dir += '/'
          dirs.append(dir)

        dirs.sort()
        return dirs


def retrieve_list(repos):
    repos = urllib.urlopen(repos).read()
    repos_xml = minidom.parseString(repos)
    plugin_nodes = repos_xml.getElementsByTagName("pyqgis_plugin")
    plugins = [ 
      {"name"    : x.getAttribute("name").encode(),
       "version" : x.getAttribute("version").encode(),
       "desc"    : x.getElementsByTagName("description")[0].childNodes[0].nodeValue.encode(),
       "author"  : x.getElementsByTagName("author_name")[0].childNodes[0].nodeValue.encode(),
       "url"     : x.getElementsByTagName("download_url")[0].childNodes[0].nodeValue.encode(),
       "filename": x.getElementsByTagName("file_name")[0].childNodes[0].nodeValue.encode()}
       for x in plugin_nodes]
     
    return plugins

def install_plugin(plugin, plugindir, repos):
    plugin_list = retrieve_list(repos)
    target = [x for x in plugin_list if x["name"] == plugin]
    if target:
        # Take the first match
        target = target[0]
        url = target["url"]
        filename = target["filename"]

        print "Retrieving from %s" % url
        try:
            tmpdir = tempfile.gettempdir()
            outfile = os.path.join(tmpdir,filename)
            urllib.urlretrieve(url,outfile)          
        except:
            return (False, "Failed to download file to %s" % outfile)
            return

        print "Extracting to plugin directory (%s)" % plugindir
        try:
	    un = unzip()
            un.extract(outfile, plugindir)        
        except:
            return (False, "Failed to unzip file to %s ... check permissions" % plugindir)

    else:
        return (False, "No plugins found named %s" % plugin)

    return (True, "Python plugin installed. Go to Plugins > Plugin Manager to enable %s." % plugin )

        
