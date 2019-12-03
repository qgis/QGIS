libqemf GNU GPL v. 3.0
------------------------
AUTHOR: Ion Vasilief
------------------------
FEATURES:  libqemf enables Qt based applications to draw the contents of EMF files onto paint devices.
           The Enhanced MetaFile format (EMF) is the native vector graphics file format on Windows.
————————————————————————————————————————————————————————————————————————————————————————————————————
DEPENDENCIES: You need Qt (http://www.qt.io) installed on your system in order to build libqemf.
————————————————————————————————————————————————————————————————————————————————————————————————————
LICENSE: GNU GPL v. 3.0
————————————————————————————————————————————————————————————————————————————————————————————————————
CREDITS: libqemf is based on the libemf library from Calligra Suite (https://www.calligra.org/).
	The original source code can be found in the Calligra project repository:
	https://projects.kde.org/projects/calligra/repository/revisions/master/show/libs/vectorimage/libemf
————————————————————————————————————————————————————————————————————————————————————————————————————
COMPILING: libqemf uses qmake for the building process. 
	qmake is part of a Qt distribution: 
	qmake reads project files, that contain the options and rules how to build a certain project. 
	A project file ends with the suffix "*.pro". Please read the qmake documentation for more details.

After installing Qt on your system, type the following command lines: 
	$ qmake
	$ make
————————————————————————————————————————————————————————————————————————————————————————————————————
USE: a short demo application is provided in the “viewer” folder of the source archive.
————————————————————————————————————————————————————————————————————————————————————————————————————
