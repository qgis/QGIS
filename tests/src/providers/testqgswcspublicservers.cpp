/***************************************************************************
     testqgswcspublicservers.cpp
     --------------------------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Radim Blazek
    Email                : radim dot blazek at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QApplication>
#include <QImage>
#include <QObject>
#include <QPainter>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QTextStream>

#include <qgsapplication.h>
#include <qgsdatasourceuri.h>
#include <qgslogger.h>
#include <qgsmaplayerregistry.h>
#include <qgsmaprenderer.h>
#include <qgsproviderregistry.h>
#include <qgsrasterdataprovider.h>
#include <qgsrasterinterface.h>
#include <qgsrasterlayer.h>
#include <qgswcscapabilities.h>
#include <testqgswcspublicservers.h>

//runs before all tests
void TestQgsWcsPublicServers::init()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsDebugMsg( "Entered" );

  mMaxCoverages = 2;

  mCacheDir = QDir( "./wcstestcache" );
  if ( !mCacheDir.exists() )
  {
    QDir myDir = QDir::root();
    if ( !myDir.mkpath( mCacheDir.absolutePath() ) )
    {
      QgsDebugMsg( "Cannot create cache dir " + mCacheDir.absolutePath() );
      QCoreApplication::exit( 1 );
    }
  }
  mHead << "Coverage";
  mHead << "Version";
  mHead << "Snap";
  mHead << "Bands";
  mHead << "Type";
  mHead << "Min";
  mHead << "Max";
  mHead << "Values";
  mHead << "Colors";
  mHead << "Has size";
}

void TestQgsWcsPublicServers::test( )
{
  QStringList versions;
  // It may happen that server supports 1.1.1, but does not accept 1.1 (http://zeus.pin.unifi.it/gi-wcs/http)
  versions << "" << "1.0.0" << "1.1.0"; // empty for default
  QStringList servers;
  // Some (first) coverages do not advertize any supportedCRS and sever gives
  // error both with native CRS (EPSG::561005) and EPSG:4326
  // MOD* coverages work OK
  servers << "http://argon.geogr.uni-jena.de:8080/geoserver/ows";
  servers << "http://demo.geonode.org/geoserver/wcs";
  servers << "http://demo.mapserver.org/cgi-bin/wcs";
  servers << "http://demo.opengeo.org/geoserver/wcs";
  // geobrain.laits.gmu.edu servers are quite slow
  servers << "http://geobrain.laits.gmu.edu/cgi-bin/gbwcs-dem";
  servers << "http://geobrain.laits.gmu.edu/cgi-bin/ows8/wcseo";
  servers << "http://geobrain.laits.gmu.edu/cgi-bin/wcs110";
  servers << "http://geobrain.laits.gmu.edu/cgi-bin/wcs-all";
  servers << "http://iceds.ge.ucl.ac.uk/cgi-bin/icedswcs";
  servers << "http://motherlode.ucar.edu:8080/thredds/wcs/fmrc/NCEP/DGEX/Alaska_12km/NCEP-DGEX-Alaska_12km_best.ncd";
  servers << "http://navigator.state.or.us/ArcGIS/services/Framework/Imagery_Mosaic2009/ImageServer/WCSServer";
  servers << "http://nsidc.org/cgi-bin/atlas_north";
  servers << "http://sedac.ciesin.columbia.edu/geoserver/wcs";
  // Big and slow
  //servers << "http://webmap.ornl.gov/ogcbroker/wcs";
  servers << "http://ws.csiss.gmu.edu/cgi-bin/wcs-t";
  // Big and slow
  //servers << "http://ws.laits.gmu.edu/cgi-bin/wcs-all";
  // Currently very slow or down
  //servers << "http://www.sogeo.ch/geoserver/wcs";
  // Slow and erroneous
  //servers << "http://zeus.pin.unifi.it/gi-wcs/http";

  foreach ( QString server, servers )
  {
    QStringList myServerLog;
    myServerLog << "server:" + server;
    QString myServerDirName = server;
    myServerDirName.replace( QRegExp( "[:/]+" ), "." );
    myServerDirName.replace( QRegExp( "\\.$" ), "" );
    QgsDebugMsg( "myServerDirName = " + myServerDirName );

    QDir myServerDir( mCacheDir.absolutePath() + QDir::separator() + myServerDirName );
    QString myServerLogPath = myServerDir.absolutePath() + QDir::separator() + "server.log";
    if ( QFileInfo( myServerLogPath ).exists() )
    {
      QgsDebugMsg( "cache exists " + myServerDir.absolutePath() );
      continue;
    }

    if ( !myServerDir.exists() )
    {
      mCacheDir.mkdir( myServerDirName );
    }

    foreach ( QString version, versions )
    {
      QgsDebugMsg( "server: " + server + " version: " + version );

      QgsDataSourceURI myServerUri;

      myServerUri.setParam( "url", server );
      if ( !version.isEmpty() )
      {
        myServerUri.setParam( "version", version );
      }

      QgsWcsCapabilities myCapabilities;
      myCapabilities.setUri( myServerUri );

      if ( !myCapabilities.lastError().isEmpty() )
      {
        QgsDebugMsg( myCapabilities.lastError() );
        myServerLog << "error: (version: " + version + ") " +  myCapabilities.lastError().replace( "\n", " " );
        continue;
      }

      QVector<QgsWcsCoverageSummary> myCoverages;
      if ( !myCapabilities.supportedCoverages( myCoverages ) )
      {
        QgsDebugMsg( "Cannot get list of coverages" );
        myServerLog << "error: (version: " + version + ") Cannot get list of coverages";
        continue;
      }

      int myCoverageCount = 0;
      int myStep = myCoverages.size() / mMaxCoverages;
      int myStepCount = -1;
      foreach ( QgsWcsCoverageSummary myCoverage, myCoverages )
      {
        QgsDebugMsg( "coverage: " + myCoverage.identifier );
        // Go in steps to get more success/errors
        if ( myStepCount == -1 || myStepCount > myStep )
        {
          myStepCount = 0;
        }
        else
        {
          myStepCount++;
          continue;
        }

        myCoverageCount++;
        if ( myCoverageCount > mMaxCoverages ) break;

        QString myPath = myServerDir.absolutePath() + QDir::separator() + myCoverage.identifier;

        if ( !version.isEmpty() )
        {
          myPath += "-" + version;
        }
        QString myLogPath = myPath + ".log";

        if ( QFileInfo( myLogPath ).exists() )
        {
          QMap<QString, QString> log = readLog( myLogPath );
          if ( !log.value( "identifier" ).isEmpty() && log.value( "error" ).isEmpty() ) continue;
        }

        QStringList myLog;
        myLog << "identifier:" + myCoverage.identifier;
        myCapabilities.describeCoverage( myCoverage.identifier );
        myCoverage = myCapabilities.coverage( myCoverage.identifier ); // get described
        QgsDataSourceURI myUri = myServerUri;
        myUri.setParam( "identifier", myCoverage.identifier );
        if ( myCoverage.times.size() > 0 )
        {
          myUri.setParam( "time", myCoverage.times.value( 0 ) );
        }
        myLog << "version:" + version;
        myLog << "uri:" + myUri.encodedUri();

        int myWidth = 100;
        int myHeight = 100;
        if ( myCoverage.hasSize )
        {
          myHeight = static_cast<int>( qRound( 1.0 * myWidth * myCoverage.height / myCoverage.width ) );
        }
        myLog << QString( "hasSize:%1" ).arg( myCoverage.hasSize );

        QgsRasterLayer * myLayer = new QgsRasterLayer( myUri.encodedUri(), myCoverage.identifier, "wcs", true );
        if ( myLayer->isValid() )
        {
          int myBandCount = myLayer->dataProvider()->bandCount();
          myLog << "bandCount:" + QString::number( myBandCount );
          if ( myBandCount > 0 )
          {
            myLog << "srcType:" + QString::number( myLayer->dataProvider()->srcDataType( 1 ) );

            QgsRasterBandStats myStats = myLayer->dataProvider()->bandStatistics( 1, QgsRasterBandStats::All, QgsRectangle(), myWidth * myHeight );
            myLog << "min:" + QString::number( myStats.minimumValue );
            myLog << "max:" + QString::number( myStats.maximumValue );
          }

          QgsMapRenderer myMapRenderer;
          QList<QgsMapLayer *> myLayersList;

          myLayersList.append( myLayer );
          QgsMapLayerRegistry::instance()->addMapLayers( myLayersList, false );

          QMap<QString, QgsMapLayer*> myLayersMap = QgsMapLayerRegistry::instance()->mapLayers();

          myMapRenderer.setLayerSet( myLayersMap.keys() );

          myMapRenderer.setExtent( myLayer->extent() );

          QImage myImage( myWidth, myHeight, QImage::Format_ARGB32_Premultiplied );
          myImage.fill( 0 );

          myMapRenderer.setOutputSize( QSize( myWidth, myHeight ), myImage.logicalDpiX() );

          QPainter myPainter( &myImage );
          myMapRenderer.render( &myPainter );

          // Save rendered image
          QString myPngPath = myPath + ".png";
          QgsDebugMsg( "myPngPath = " + myPngPath );
          myImage.save( myPngPath );

          // Verify data
          QSet<QString> myValues; // cannot be QSet<double>
          void *myData = myLayer->dataProvider()->readBlock( 1, myLayer->extent(), myWidth, myHeight );
          if ( myData )
          {
            int myType = myLayer->dataProvider()->dataType( 1 );
            for ( int row = 0; row < myHeight; row++ )
            {
              for ( int col = 0; col < myWidth; col++ )
              {
                double value = myLayer->dataProvider()->readValue( myData, myType, row * myWidth + col );
                QString valueStr = QString::number( value );
                if ( !myValues.contains( valueStr ) ) myValues.insert( valueStr );
              }
            }
            free( myData );
          }
          QgsDebugMsg( QString( "%1 values" ).arg( myValues.size() ) );
          myLog << QString( "valuesCount:%1" ).arg( myValues.size() );

          // Verify image colors
          QSet<QRgb> myColors;
          for ( int row = 0; row < myHeight; row++ )
          {
            for ( int col = 0; col < myWidth; col++ )
            {
              QRgb color = myImage.pixel( col, row );
              if ( !myColors.contains( color ) ) myColors.insert( color );
            }
          }
          QgsDebugMsg( QString( "%1 colors" ).arg( myColors.size() ) );
          myLog << QString( "colorsCount:%1" ).arg( myColors.size() );
        }
        else
        {
          QgsDebugMsg( "Layer is not valid" );
          myLog << "error:Layer is not valid";
        }

        QFile myLogFile( myLogPath );
        myLogFile.open( QIODevice::WriteOnly | QIODevice::Text );
        QTextStream myStream( &myLogFile );
        myStream << myLog.join( "\n" );

        myLogFile.close();
        QgsMapLayerRegistry::instance()->removeAllMapLayers();
      }
    }
    QFile myServerLogFile( myServerLogPath );
    myServerLogFile.open( QIODevice::WriteOnly | QIODevice::Text );
    QTextStream myStream( &myServerLogFile );
    myStream << myServerLog.join( "\n" );
    myServerLogFile.close();
  }
}

void TestQgsWcsPublicServers::writeReport( QString theReport )
{
  QString myReportFile = mCacheDir.absolutePath() + QDir::separator() + "index.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly ) )
  {
    QTextStream myStream( &myFile );
    myStream << theReport;
    myFile.close();
  }
  QgsDebugMsg( "Report written to " + myReportFile );
}

void TestQgsWcsPublicServers::report()
{
  QString myReport;

  int myServerCount = 0;
  int myServerErrCount = 0;
  int myCoverageCount = 0;
  int myCoverageErrCount = 0;
  int myCoverageWarnCount = 0;

  foreach ( QString myDirName, mCacheDir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) )
  {
    int myCount = 0;
    int myErrCount = 0;
    int myWarnCount = 0;
    myServerCount++;
    QDir myDir( mCacheDir.absolutePath() + QDir::separator() + myDirName );
    QString myServerLogPath = myDir.absolutePath() + QDir::separator() + "server.log";
    QMap<QString, QString> myServerLog = readLog( myServerLogPath );

    myReport += QString( "<h2>%1</h2>" ).arg( myServerLog.value( "server" ) );
    QString myServerReport;

    if ( !myServerLog.value( "error" ).isEmpty() )
    {
      // Server may have more errors, for each version
      foreach ( QString err, myServerLog.values( "error" ) )
      {
        myServerReport += error( err );
      }
      myServerErrCount++;
    }
    else
    {
      myServerReport += "<table class='tab'>";
      myServerReport += row( mHead );
      QStringList filters;
      filters << "*.log";
      myDir.setNameFilters( filters );
      foreach ( QString myLogFileName, myDir.entryList( QDir::Files ) )
      {
        if ( myLogFileName == "server.log" ) continue;
        myCount++;
        myCoverageCount++;

        QString myLogPath = myDir.absolutePath() + QDir::separator() + myLogFileName;
        QMap<QString, QString>myLog = readLog( myLogPath );
        QStringList myValues;
        myValues << myLog.value( "identifier" );
        myValues << myLog.value( "version" );
        QString imgPath = myDirName + QDir::separator() + QFileInfo( myLogPath ).completeBaseName() + ".png";

        if ( !myLog.value( "error" ).isEmpty() )
        {
          myValues << myLog.value( "error" );
          myServerReport += row( myValues, "cellerr" );
          myErrCount++;
          myCoverageErrCount++;
        }
        else
        {
          myValues << "<img src='" + imgPath + "'>";
          myValues << myLog.value( "bandCount" );
          myValues << myLog.value( "srcType" );
          myValues << myLog.value( "min" );
          myValues << myLog.value( "max" );
          myValues << myLog.value( "valuesCount" );
          myValues << myLog.value( "colorsCount" );
          myValues << myLog.value( "hasSize" );

          QString cls;
          int myValuesCount = myLog.value( "valuesCount" ).toInt();
          int myColorsCount = myLog.value( "colorsCount" ).toInt();
          if ( myValuesCount < 4 )
          {
            cls = "cellerr";
            myErrCount++;
            myCoverageErrCount++;
          }
          else if ( myColorsCount < 4 )
          {
            cls = "cellwarn";
            myWarnCount++;
            myCoverageWarnCount++;
          }

          myServerReport += row( myValues, cls );
        }
      }
      myServerReport += "</table>";
      // prepend counts
      myServerReport.prepend( QString( "<b>Coverages: %1</b><br>" ).arg( myCount ) +
                              QString( "<b>Errors: %1</b><br>" ).arg( myErrCount ) +
                              QString( "<b>Warnings: %1</b><br><br>" ).arg( myWarnCount ) );
    }
    myReport += myServerReport;
  }

  QString mySettings = QgsApplication::showSettings();
  mySettings = mySettings.replace( "\n", "<br />" );
  QString myRep = "<h1>WCS public servers test</h1>\n";
  myRep += "<p>" + mySettings + "</p>";

  myRep += "<style>";
  myRep += ".tab { border-spacing: 0px; border-width: 1px 1px 0 0; border-style: solid; }";
  myRep += ".cell { border-width: 0 0 1px 1px; border-style: solid; font-size: smaller; text-align: center}";
  //myReport += ".cellok { background: #00ff00; }";
  myRep += ".cellok { background: #ffffff; }";
  myRep += ".cellwarn { background: #ffcc00; }";
  myRep += ".cellerr { background: #ff0000; }";
  myRep += ".errmsg { color: #ff0000; }";
  myRep += "</style>";

  myRep += QString( "<p>Tested first %1 coverages for each server/version</p>" ).arg( mMaxCoverages );
  myRep += QString( "<b>Servers: %1</b><br>" ).arg( myServerCount );
  myRep += QString( "<b>Servers failed: %1</b><br>" ).arg( myServerErrCount );
  myRep += QString( "<b>Coverages: %1</b><br>" ).arg( myCoverageCount );
  myRep += QString( "<b>Coverages errors: %1</b><br>" ).arg( myCoverageErrCount );
  myRep += QString( "<b>Coverages warnings: %1</b><br>" ).arg( myCoverageWarnCount );

  myRep += myReport;

  writeReport( myRep );
}

QMap<QString, QString> TestQgsWcsPublicServers::readLog( QString theFileName )
{
  QMap<QString, QString> myMap;

  QFile myFile( theFileName );
  if ( myFile.open( QIODevice::ReadOnly ) )
  {
    QTextStream myStream( &myFile );
    foreach ( QString row, myStream.readAll().split( "\n" ) )
    {
      int sepIdx = row.indexOf( ":" );
      myMap.insert( row.left( sepIdx ), row.mid( sepIdx + 1 ) );
    }
    myFile.close();
  }
  return myMap;
}

QString TestQgsWcsPublicServers::error( QString theMessage )
{
  QString myRow = "<font class='errmsg'>Error: ";
  myRow += theMessage;
  myRow += "</font>";
  return myRow;
}

QString TestQgsWcsPublicServers::row( QStringList theValues, QString theClass )
{
  QString myRow = "<tr>";
  for ( int i = 0; i < theValues.size(); i++ )
  {
    QString val = theValues.value( i );
    QString colspan;
    if ( theValues.size() < mHead.size() && i == ( theValues.size() - 1 ) )
    {
      colspan = QString( "colspan=%1" ).arg( mHead.size() - theValues.size() + 1 ) ;
    }
    myRow += QString( "<td class='cell %1' %2>%3</td>" ).arg( theClass ).arg( colspan ).arg( val );
  }
  myRow += "</tr>";
  return myRow;
}

int main( int argc, char *argv[] )
{
  QgsApplication myApp( argc, argv, false );
  QgsApplication::init( QString() );
  QgsApplication::initQgis();

  TestQgsWcsPublicServers myTest;
  myTest.init();
  myTest.test();
  myTest.report();

  QCoreApplication::exit( 0 );
}
