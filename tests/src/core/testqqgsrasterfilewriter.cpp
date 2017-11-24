/***************************************************************************
testqqgsrasterfilewriter.cpp
--------------------------------------
Date : Friday, Nov 24, 2017
Copyright: (C) 2017 by Pierre Assali and Guilhem Villemin
Email: pierre.assali@altametris.com guilhem.villemin@altametris.com
***************************************************************************
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
***************************************************************************/


#include <QtTest/QtTest>
//Qt includes...
#include <QObject>
#include <QString>
#include <QObject>
#include <QApplication>
#include <QFileInfo>
#include <QDir>

//qgis includes...
#include <qgsrasterlayer.h>
#include <qgsrasterfilewriter.h>
#include <qgsapplication.h>


/** \ingroup UnitTests
* This is a unit test for the QgsFileWriter class.
*/

class TestQgsFileWriter : public QObject
{
	Q_OBJECT

		private slots:
	// will be called before the first testfunction is executed.
	void initTestCase();
	// will be called after the last testfunction was executed.
	void cleanupTestCase();
	// will be called before each testfunction is executed.
	void init() {};
	// will be called after every testfunction.
	void cleanup() {};

	//
	// Functional Testing
	//

	/** Check if a raster is valid. */
	void isValid();

	// more functional tests here ...

private:
	// Here we have any data structures that may need to
	// be used in many test cases.
	double xmin_vrt, ymin_vrt, xmin_original, ymin_original;
	QgsRasterLayer * myVRTRasterLayer;
	QgsRasterFileWriter * myRasterFileWriter;
	QgsRasterLayer * myRasterLayer;
};


void TestQgsFileWriter::initTestCase()
{
	// init QGIS's paths - true means that all path will be inited from prefix
	QString qgisPath = QCoreApplication::applicationDirPath();
	QgsApplication::setPrefixPath(qgisPath, true);
#ifdef Q_OS_LINUX
	QgsApplication::setPkgDataPath(qgisPath + "/../share/qgis");
#endif
	//create some objects that will be used in all tests...

	std::cout << "PrefixPATH: " << QgsApplication::prefixPath().toLocal8Bit().data() << std::endl;
	std::cout << "PluginPATH: " << QgsApplication::pluginPath().toLocal8Bit().data() << std::endl;
	std::cout << "PkgData PATH: " << QgsApplication::pkgDataPath().toLocal8Bit().data() << std::endl;
	std::cout << "User DB PATH: " << QgsApplication::qgisUserDatabaseFilePath().toLocal8Bit().data() << std::endl;

	//create a raster layer that will be used in all tests...
	QString myFileName(TEST_DATA_DIR); //defined in CmakeLists.txt
	myFileName = myFileName + QDir::separator() + "ALLINGES_RGF93_CC46_1_1.tif";
	QFileInfo myRasterFileInfo(myFileName);
	myRasterLayer = new QgsRasterLayer(myRasterFileInfo.absoluteFilePath(), myRasterFileInfo.completeBaseName());
	myRasterFileWriter = new QgsRasterFileWriter(myRasterFileInfo.absoluteFilePath() + "\\" + myRasterFileInfo.completeBaseName());

	//2. Definition of the pyramid levels
	QList<int> myLevelList;
	myLevelList << 2 << 4 << 8 << 16 << 32 << 64 << 128;
	myRasterFileWriter->setPyramidsList(myLevelList);
	//3. Pyramid format
	myRasterFileWriter->setPyramidsFormat(QgsRaster::PyramidsGTiff);
	//4. Resampling method
	myRasterFileWriter->setPyramidsResampling("NEAREST");
	//5. Tiled mode => true for vrt creation
	myRasterFileWriter->setTiledMode(true);
	//6. Tile size
	myRasterFileWriter->setMaxTileWidth(500);
	myRasterFileWriter->setMaxTileHeight(500);
	//7. Coordinate Reference System
	QgsCoordinateReferenceSystem myCRS;
	myCRS.createFromString("EPSG:3946");
	//8. Prepare raster pipe
	QgsRasterPipe pipe;
	pipe.set(myRasterLayer->dataProvider()->clone());
	// Let's do it !
	QgsRasterFileWriter::WriterError res = myRasterFileWriter->writeRaster(&pipe, myRasterLayer->width(), myRasterLayer->height(), myRasterLayer->extent(), myCRS);

	// Now let's compare the georef of the original raster with the georef of the generated vrt file
	myVRTRasterLayer = new QgsRasterLayer(QDir::fromNativeSeparators(myRasterFileInfo.absolutePath() + "\\" + myRasterFileInfo.completeBaseName() + "\\" + myRasterFileInfo.completeBaseName() + ".vrt"), myRasterFileInfo.completeBaseName());

	xmin_vrt = myVRTRasterLayer->extent().xMinimum();
	ymin_vrt = myVRTRasterLayer->extent().yMaximum();
	xmin_original = myRasterLayer->extent().xMinimum();
	ymin_original = myRasterLayer->extent().yMaximum();
}

void TestQgsFileWriter::cleanupTestCase()
{
	delete 	myVRTRasterLayer;
	delete 	myRasterFileWriter;
	delete 	myRasterLayer;
}

void TestQgsFileWriter::isValid()
{
	// Let's check if the georef of the original raster with the georef of the generated vrt file
	QVERIFY((std::abs(xmin_vrt - xmin_original) < myRasterLayer->rasterUnitsPerPixelX() / 4) && (std::abs(ymin_vrt - ymin_original) < myRasterLayer->rasterUnitsPerPixelY() / 4) );
}

QTEST_MAIN(TestQgsFileWriter)
#include "testqqgsrasterfilewriter.moc"
