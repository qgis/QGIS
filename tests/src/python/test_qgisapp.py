import unittest

from utilities import getQgisTestApp

QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

class TestQGisApp(unittest.TestCase):

# Can this work at all?
# The theme resources are built into the executable and should only be
# available to plugins and not in test applications.
#
#    def testValidThemeName(self):
#        """That can set the app to use a valid theme"""
#        QGISAPP.setThemeName('gis')
#        myExpectedResult = 'gis'
#        myResult = QGISAPP.themeName()
#        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
#                      (myExpectedResult, myResult))
#
#        mySettings = QGISAPP.showSettings()
#        print mySettings
#
#        assert myExpectedResult == myResult, myMessage

    def testInvalidThemeName(self):
        """That setting the app to use an invalid theme will fallback to 'default'"""
        QGISAPP.setThemeName('fooobar')
        myExpectedResult = 'default'
        myResult = QGISAPP.themeName()        
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                      (myExpectedResult, myResult))
        assert myExpectedResult == myResult, myMessage




if __name__ == '__main__':
    unittest.main()

