""" unzip.py
Version: 1.1
By Doug Tolton (http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/252508)
"""

import zipfile
import os

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
            os.makedirs(dir)

        zf = zipfile.ZipFile(file)

        # create directory structure to house files
        #print "Creating plugin structure:"
        self._createstructure(file, dir)

        num_files = len(zf.namelist())
        percent = self.percent
        divisions = 100 / percent
        perc = int(num_files / divisions)

        # extract files to directory structure
        for i, name in enumerate(zf.namelist()):

            if self.verbose == True:
                pass
                #print "Extracting %s" % name
            elif perc > 0 and (i % perc) == 0 and i > 0:
                complete = int (i / perc) * percent
                #print "%s%% complete" % complete

            if not name.endswith('/'):
                outfile = open(os.path.join(dir, name), 'wb')
                outfile.write(zf.read(name))
                outfile.flush()
                outfile.close()

    def _createstructure(self, file, dir):
        self._makedirs(self._listdirs(file), dir)

    def _makedirs(self, directories, basedir):
        """ Create any directories that don't currently exist """
        #print "Processing directories contained in the zip file: %s" % directories
        for dir in directories:
            curdir = os.path.join(basedir, dir)
            # normalize the path
            curdir = os.path.normpath(curdir)
            #print "Checking to see if we should create %s" % curdir
            if not os.path.exists(curdir):
                # use makedirs to create parent directories as well
                #print "Creating %s" % curdir
                os.makedirs(curdir)

    def _listdirs(self, file):
        """ Grabs all the directories in the zip structure
        This is necessary to create the structure before trying
        to extract the file to it. """
        zf = zipfile.ZipFile(file)

        dirs = []

        for name in zf.namelist():
          (path, filename) = os.path.split(name)

          if path not in dirs:
            dirs.append(path)

        dirs.sort()
        return dirs
