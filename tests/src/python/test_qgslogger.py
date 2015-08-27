# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsComposition.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Tim Sutton'
__date__ = '20/08/2012'
__copyright__ = 'Copyright 2012, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis
import tempfile
import os

(myFileHandle, myFilename) = tempfile.mkstemp()
os.environ['QGIS_DEBUG'] = '2'
os.environ['QGIS_LOG_FILE'] = myFilename

from qgis.core import QgsLogger
from utilities import TestCase, unittest

# Convenience instances in case you may need them
# not used in this test
#from utilities import getQgisTestApp
#QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()


class TestQgsLogger(TestCase):

    def testLogger(self):
        try:
            myFile = os.fdopen(myFileHandle, "w")
            myFile.write("QGIS Logger Unit Test\n")
            myFile.close()
            myLogger = QgsLogger()
            myLogger.debug('This is a debug')
            myLogger.warning('This is a warning')
            myLogger.critical('This is critical')
            #myLogger.fatal('Aaaargh...fatal');  #kills QGIS not testable
            myFile = open(myFilename, 'rt')
            myText = myFile.readlines()
            myFile.close()
            myExpectedText = ['QGIS Logger Unit Test\n',
                              'This is a debug\n',
                              'This is a warning\n',
                              'This is critical\n']
            myMessage = ('Expected:\n---\n%s\n---\nGot:\n---\n%s\n---\n' %
                         (myExpectedText, myText))
            self.assertEquals(myText, myExpectedText, myMessage)
        finally:
            pass
            os.remove(myFilename)

if __name__ == '__main__':
    unittest.main()
