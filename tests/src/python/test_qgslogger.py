import tempfile
import os
import unittest

from qgis.core import QgsLogger

# Convenience instances in case you may need them
# not used in this test
#from utilities import getQgisTestApp
#QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

class TestQgsLogger(unittest.TestCase):

    def testLogger(self):
        (myFileHandle, myFilename) = tempfile.mkstemp()
        try:
            myFile = os.fdopen(myFileHandle, "w")
            myFile.write("QGIS Logger Unit Test\n")
            myFile.close()
            os.environ['QGIS_DEBUG'] = '2'
            os.environ['QGIS_LOG_FILE'] = myFilename
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

