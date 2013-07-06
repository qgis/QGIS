from PyQt4.QtCore import QCoreApplication, QSettings

def chunks(l, n):
	for i in xrange(0, len(l), n):
		yield l[i:i+n]

QCoreApplication.setOrganizationName( "QGIS" )
QCoreApplication.setOrganizationDomain( "qgis.org" )
QCoreApplication.setApplicationName( "QGIS2" )

s = QSettings()

ba = s.value("/UI/geometry").toByteArray()

f = open("src/app/ui_defaults.h", "w")

f.write( "#ifndef UI_DEFAULTS_H\n#define UI_DEFAULTS_H\n\nstatic const unsigned char defaultUIgeometry[] =\n{\n" )

for chunk in chunks(ba,16):
	f.write( "  %s,\n" % ", ".join( map( lambda x : "0x%02x" % ord(x), chunk ) ) )

f.write( "};\n\nstatic const unsigned char defaultUIstate[] =\n{\n" )

ba = s.value("/UI/state").toByteArray()

for chunk in chunks(ba,16):
	f.write( "  %s,\n" % ", ".join( map( lambda x : "0x%02x" % ord(x), chunk ) ) )

f.write( "};\n\n#endif // UI_DEFAULTS_H\n" )

f.close()
