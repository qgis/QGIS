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
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>

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

#ifdef WIN32
// Open files in binary mode
#include <fcntl.h> /*  _O_BINARY */
#ifdef MSVC
#undef _fmode
int _fmode = _O_BINARY;
#else
#endif  //_MSC_VER
#else
#include <getopt.h>
#endif

TestQgsWcsPublicServers::TestQgsWcsPublicServers( const QString & cacheDirPath, int maxCoverages, const QString & server, const QString & coverage, const QString &version, bool force ):
    mCacheDirPath( cacheDirPath )
    , mMaxCoverages( maxCoverages )
    , mServer( server )
    , mCoverage( coverage )
    , mVersion( version )
    , mForce( force )
    , mTimeout( 300000 )
    , mOrigTimeout( 20000 )
{

}

TestQgsWcsPublicServers::~TestQgsWcsPublicServers()
{
  QSettings settings;
  settings.setValue( "/qgis/networkAndProxy/networkTimeout", mOrigTimeout );
}

//runs before all tests
void TestQgsWcsPublicServers::init()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsDebugMsg( "Entered" );

  // Unfortunately this seems to be the only way to set timeout, we try to reset it
  // at the end but it can be canceled before ...
  QSettings settings;
  mOrigTimeout = settings.value( "/qgis/networkAndProxy/networkTimeout", "20000" ).toInt();
  settings.setValue( "/qgis/networkAndProxy/networkTimeout", mTimeout );

  //mCacheDir = QDir( "./wcstestcache" );
  mCacheDir = QDir( mCacheDirPath );
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

  QStringList providers;
  providers << "wcs" << "gdal";
  foreach ( QString provider, providers )
  {
    QString prefix = provider == "gdal" ? "GDAL " : "";
    mHead << prefix + "CRS";
    mHead << prefix + "Width";
    mHead << prefix + "Height";
    mHead << prefix + "Extent";
    mHead << prefix + "Snap";
    mHead << prefix + "Bands";
    mHead << prefix + "Type";
    mHead << prefix + "Min";
    mHead << prefix + "Max";
    mHead << prefix + "Values";
    mHead << prefix + "Colors";
    mHead << prefix + "Time (s)";
  }

  // read servers + issues list
  QString path = QgsApplication::pkgDataPath() +  "/resources/wcs-servers.json";
  QFile file( path );
  if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    QString data = file.readAll();
    //QgsDebugMsg("servers: \n"  + str );
    file.close();
    QScriptEngine engine;
    QScriptValue result = engine.evaluate( data );

    QScriptValueIterator serverIt( result );
    while ( serverIt.hasNext() )
    {
      serverIt.next();
      QScriptValue serverValue = serverIt.value();

      QString serverUrl = serverValue.property( "url" ).toString();
      QgsDebugMsg( "serverUrl: " + serverUrl );

      Server server( serverUrl );
      server.description = serverValue.property( "description" ).toString();

      QScriptValueIterator paramsIt( serverValue.property( "params" ) );
      while ( paramsIt.hasNext() )
      {
        paramsIt.next();
        QgsDebugMsg( QString( "params value: %1" ).arg( paramsIt.value().toString() ) );
        server.params.insert( paramsIt.name(), paramsIt.value().toString() );
      }

      QScriptValue issuesValue = serverValue.property( "issues" );

      QScriptValueIterator issuesIt( issuesValue );
      while ( issuesIt.hasNext() )
      {
        issuesIt.next();
        QScriptValue issueValue = issuesIt.value();

        QString description = issueValue.property( "description" ).toString();
        QgsDebugMsg( "description: " + description );
        Issue issue( description );

        issue.offender = issueValue.property( "offender" ).toString();

        QScriptValue coveragesValue = issueValue.property( "coverages" );
        QScriptValueIterator coveragesIt( coveragesValue );
        while ( coveragesIt.hasNext() )
        {
          coveragesIt.next();
          issue.coverages << coveragesIt.value().toString();
        }

        QScriptValue versionsValue = issueValue.property( "versions" );
        QScriptValueIterator versionsIt( versionsValue );
        while ( versionsIt.hasNext() )
        {
          versionsIt.next();
          issue.versions << versionsIt.value().toString();
        }

        server.issues << issue;
      }

      mServers << server;
    }
  }
  else
  {
    QgsDebugMsg( "Cannot open " + path );
  }
}

TestQgsWcsPublicServers::Server TestQgsWcsPublicServers::getServer( const QString & url )
{
  foreach ( Server server, mServers )
  {
    if ( server.url == url ) return server;
  }
  return Server();
}

QList<TestQgsWcsPublicServers::Issue> TestQgsWcsPublicServers::issues( const QString & url, const QString & coverage, const QString &version )
{
  QList<Issue> issues;
  foreach ( Server server, mServers )
  {
    if ( server.url == url )
    {
      foreach ( Issue issue, server.issues )
      {
        if (( issue.coverages.size() == 0 || issue.coverages.contains( coverage ) ) &&
            ( issue.versions.size() == 0 || issue.versions.contains( version ) ) )
        {
          issues << issue;
        }
      }
    }
  }
  return issues;
}

QStringList TestQgsWcsPublicServers::issueDescriptions( const QString & url, const QString & coverage, const QString &version )
{
  QStringList descriptions;
  foreach ( Issue myIssue, issues( url, coverage, version ) )
  {
    descriptions << myIssue.description;
  }
  return descriptions;
}

int TestQgsWcsPublicServers::issueOffender( const QString & url, const QString & coverage, const QString &version )
{
  int offender = NoOffender;
  foreach ( Issue myIssue, issues( url, coverage, version ) )
  {
    if ( myIssue.offender == "server" )
    {
      offender |= ServerOffender;
    }
    else
    {
      offender |= QGisOffender;
    }
  }
  return offender;
}

void TestQgsWcsPublicServers::test()
{
  QStringList versions;
  QStringList serverUrls;

  // It may happen that server supports 1.1.1, but does not accept 1.1 (http://zeus.pin.unifi.it/gi-wcs/http)

  if ( !mVersion.isEmpty() )
  {
    versions << mVersion;
  }
  else
  {
    //versions << "" << "1.0.0" << "1.1.0"; // empty for default
    // Empty is version is the same like "1.0.0" because QGIS will try "1.0.0" first
    versions << "1.0.0" << "1.1.0";
  }


  if ( !mServer.isEmpty() )
  {
    serverUrls << mServer;
  }
  else
  {
    foreach ( Server server, mServers )
    {
      serverUrls << server.url;
    }
  }

  foreach ( QString serverUrl, serverUrls )
  {
    Server myServer = getServer( serverUrl );
    QStringList myServerLog;
    myServerLog << "server:" + serverUrl;
    QString myServerDirName = serverUrl;
    myServerDirName.replace( QRegExp( "[:/]+" ), "." );
    myServerDirName.replace( QRegExp( "\\.$" ), "" );
    QgsDebugMsg( "myServerDirName = " + myServerDirName );

    QDir myServerDir( mCacheDir.absolutePath() + QDir::separator() + myServerDirName );

    if ( !myServerDir.exists() )
    {
      mCacheDir.mkdir( myServerDirName );
    }

    QString myServerLogPath = myServerDir.absolutePath() + QDir::separator() + "server.log";

    foreach ( QString version, versions )
    {
      QgsDebugMsg( "server: " + serverUrl + " version: " + version );
      QStringList myVersionLog;
      myVersionLog << "version:" + version;

      QString myVersionDirName = "v" + version;
      QString myVersionDirPath = myServerDir.absolutePath() + QDir::separator() + myVersionDirName;

      QString myVersionLogPath = myVersionDirPath + QDir::separator() + "version.log";

      QDir myVersionDir( myVersionDirPath );
      if ( !myVersionDir.exists() )
      {
        myServerDir.mkdir( myVersionDirName );
      }

      QgsDataSourceURI myServerUri;

      myServerUri.setParam( "url", serverUrl );
      if ( !version.isEmpty() )
      {
        myServerUri.setParam( "version", version );
      }
      myServerUri.setParam( "cache", "AlwaysNetwork" );

      foreach ( QString key, myServer.params.keys() )
      {
        myServerUri.setParam( key, myServer.params.value( key ) );
      }

      QgsWcsCapabilities myCapabilities;
      myCapabilities.setUri( myServerUri );


      if ( !myCapabilities.lastError().isEmpty() )
      {
        QgsDebugMsg( myCapabilities.lastError() );
        myVersionLog << "error:" +  myCapabilities.lastError().replace( "\n", " " );
        continue;
      }

      myVersionLog << "getCapabilitiesUrl:" + myCapabilities.getCapabilitiesUrl();

      QVector<QgsWcsCoverageSummary> myCoverages;
      if ( !myCapabilities.supportedCoverages( myCoverages ) )
      {
        QgsDebugMsg( "Cannot get list of coverages" );
        myVersionLog << "error:Cannot get list of coverages";
        continue;
      }

      myVersionLog << QString( "totalCoverages:%1" ).arg( myCoverages.size() );

      int myCoverageCount = 0;
      int myStep = myCoverages.size() / qMin( mMaxCoverages, myCoverages.size() );
      int myStepCount = -1;
      bool myCoverageFound = false;
      foreach ( QgsWcsCoverageSummary myCoverage, myCoverages )
      {
        QgsDebugMsg( "coverage: " + myCoverage.identifier );
        if ( !mCoverage.isEmpty() && myCoverage.identifier != mCoverage ) continue;
        myCoverageFound = true;

        // Go in steps to get more success/errors
        if ( myStepCount == -1 || myStepCount >= myStep )
        {
          myStepCount = 1;
        }
        else
        {
          myStepCount++;
          continue;
        }

        myCoverageCount++;
        if ( myCoverageCount > mMaxCoverages ) break;


        QString myPath = myVersionDirPath + QDir::separator() + myCoverage.identifier;
        QString myLogPath = myPath + ".log";

        if ( QFileInfo( myLogPath ).exists() && !mForce )
        {
          //QMap<QString, QString> log = readLog( myLogPath );
          //if ( !log.value( "identifier" ).isEmpty() && log.value( "error" ).isEmpty() ) continue;
          continue;
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
        myLog << "describeCoverageUrl:" + myCapabilities.getDescribeCoverageUrl( myCoverage.identifier );
        // Test time
        //myLog << "date:" + QString( "%1").arg( QDateTime::currentDateTime().toTime_t() );
        myLog << "date:" + QString( "%1" ).arg( QDateTime::currentDateTime().toString() );

        int myWidth = 100;
        int myHeight = 100;
        if ( myCoverage.hasSize )
        {
          myHeight = static_cast<int>( qRound( 1.0 * myWidth * myCoverage.height / myCoverage.width ) );
        }
        myLog << QString( "hasSize:%1" ).arg( myCoverage.hasSize );

        // Test QGIS provider and via GDAL
        QStringList providers;
        providers << "wcs" << "gdal";

        foreach ( QString provider, providers )
        {
          QTime time;
          time.start();
          QString uri;
          if ( provider == "wcs" )
          {
            uri = myUri.encodedUri();
          }
          else // gdal
          {
            uri = myPath + "-gdal.xml";
            QFile myGdalXmlFile( uri );
            myGdalXmlFile.open( QIODevice::WriteOnly | QIODevice::Text );
            QTextStream myStream( &myGdalXmlFile );
            myStream << "<WCS_GDAL>\n";
            myStream << "  <ServiceURL>" + serverUrl + "?" + "</ServiceURL>\n";
            myStream << "  <CoverageName>" + myCoverage.identifier + "</CoverageName>\n";
            myStream << "  <Version>" + version + "</Version>\n";
            myStream << QString( "  <Timeout>%1</Timeout>\n" ).arg( mTimeout / 1000., 0, 'd' );
            myStream << "</WCS_GDAL>\n";

            myGdalXmlFile.close();
          }

          QgsRasterLayer * myLayer = new QgsRasterLayer( uri, myCoverage.identifier, provider );
          if ( myLayer->isValid() )
          {
            myLog << provider + "_crs:" + myLayer->dataProvider()->crs().authid();
            myLog << provider + "_width:" + QString::number( myLayer->dataProvider()->xSize() );
            myLog << provider + "_height:" + QString::number( myLayer->dataProvider()->ySize() );
            QgsRectangle extent = myLayer->dataProvider()->extent();
            myLog << provider + "_extent:"
            + QgsRasterBlock::printValue( extent.xMinimum() ) + ","
            + QgsRasterBlock::printValue( extent.yMinimum() ) + ","
            + QgsRasterBlock::printValue( extent.xMaximum() ) + ","
            + QgsRasterBlock::printValue( extent.yMaximum() ) + ",";
            int myBandCount = myLayer->dataProvider()->bandCount();
            myLog << provider + "_bandCount:" + QString::number( myBandCount );
            if ( myBandCount > 0 )
            {
              myLog << provider + "_srcType:" + QString::number( myLayer->dataProvider()->srcDataType( 1 ) );

              QgsRasterBandStats myStats = myLayer->dataProvider()->bandStatistics( 1, QgsRasterBandStats::All, QgsRectangle(), myWidth * myHeight );
              myLog << provider + "_min:" + QString::number( myStats.minimumValue );
              myLog << provider + "_max:" + QString::number( myStats.maximumValue );
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
            QString myPngPath = myPath + "-" + provider + ".png";
            QgsDebugMsg( "myPngPath = " + myPngPath );
            myImage.save( myPngPath );

            // Verify data
            QSet<QString> myValues; // cannot be QSet<double>
            //void *myData = myLayer->dataProvider()->readBlock( 1, myLayer->extent(), myWidth, myHeight );
            QgsRasterBlock *myBlock = myLayer->dataProvider()->block( 1, myLayer->extent(), myWidth, myHeight );
            if ( myBlock )
            {
              for ( int row = 0; row < myHeight; row++ )
              {
                for ( int col = 0; col < myWidth; col++ )
                {
                  double value = myBlock->value( row, col );
                  QString valueStr = QString::number( value );
                  if ( !myValues.contains( valueStr ) ) myValues.insert( valueStr );
                }
              }
              delete myBlock;
            }
            QgsDebugMsg( QString( "%1 values" ).arg( myValues.size() ) );
            myLog << provider + QString( "_valuesCount:%1" ).arg( myValues.size() );

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
            myLog << provider + QString( "_colorsCount:%1" ).arg( myColors.size() );
          }
          else
          {
            QgsDebugMsg( "Layer is not valid" );
            myLog << provider + "_error:Layer is not valid";
          }
          myLog << provider + QString( "_time:%1" ).arg( time.elapsed() / 1000., 0, 'f', 2 );
          // Generate report for impatient people
          report();
        }

        QFile myLogFile( myLogPath );

        myLogFile.open( QIODevice::WriteOnly | QIODevice::Text );
        QTextStream myStream( &myLogFile );
        myStream << myLog.join( "\n" );

        myLogFile.close();
        QgsMapLayerRegistry::instance()->removeAllMapLayers();
      }
      if ( !mCoverage.isEmpty() && ! myCoverageFound )
      {
        QgsDebugMsg( "Coverage not found" );
      }
      QFile myVersionLogFile( myVersionLogPath );
      myVersionLogFile.open( QIODevice::WriteOnly | QIODevice::Text );
      QTextStream myVersionStream( &myVersionLogFile );
      myVersionStream << myVersionLog.join( "\n" );
      myVersionLogFile.close();
    }
    QFile myServerLogFile( myServerLogPath );
    Q_ASSERT( myServerLogFile.open( QIODevice::WriteOnly | QIODevice::Text ) );
    QTextStream myServerStream( &myServerLogFile );
    myServerStream << myServerLog.join( "\n" );
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
  int myServerErrCount = 0; // at least one error
  int myServerWarnCount = 0; // at least one error
  int myCoverageCount = 0;
  int myCoverageErrCount = 0;
  int myCoverageWarnCount = 0;

  foreach ( QString myServerDirName, mCacheDir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) )
  {
    myServerCount++;
    QDir myServerDir( mCacheDir.absolutePath() + QDir::separator() + myServerDirName );

    QString myServerLogPath = myServerDir.absolutePath() + QDir::separator() + "server.log";
    QMap<QString, QString> myServerLog = readLog( myServerLogPath );

    myReport += QString( "<h2>Server: %1</h2>" ).arg( myServerLog.value( "server" ) );
    Server myServer = getServer( myServerLog.value( "server" ) );
    if ( !myServer.description.isEmpty() )
    {
      myReport += myServer.description + "<br>\n";
    }
    if ( myServer.params.size() > 0 )
    {
      myReport += "<br>Additional params: ";
      foreach ( QString key, myServer.params.keys() )
      {
        myReport += key + "=" + myServer.params.value( key ) + " ";
      }
      myReport += "<br>\n";
    }

    QString myServerReport;

    bool myServerErr = false;
    bool myServerWarn = false;
    foreach ( QString myVersionDirName, myServerDir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) )
    {
      QString myVersionReport;
      int myVersionCoverageCount = 0;
      int myVersionErrCount = 0;
      int myVersionWarnCount = 0;

      QString myVersionDirPath = myServerDir.absolutePath() + QDir::separator() + myVersionDirName;
      QString myVersionLogPath = myVersionDirPath + QDir::separator() + "version.log";
      QMap<QString, QString> myVersionLog = readLog( myVersionLogPath );
      QDir myVersionDir( myVersionDirPath );

      QString myVersion = myVersionLog.value( "version" );
      myServerReport += QString( "<h3><a href='%1'>Version: %2</a></h3>" ).arg( myVersionLog.value( "getCapabilitiesUrl" ) ).arg( myVersion.isEmpty() ? "(empty)" : myVersion );

      if ( !myVersionLog.value( "error" ).isEmpty() )
      {
        // Server may have more errors, for each version
        //foreach ( QString err, myServerLog.values( "error" ) )
        //{
        //myVersionReport += error( err );
        //}
        myVersionReport += error( myServerLog.value( "error" ) );
        myVersionErrCount++;
      }
      else
      {
        myVersionReport += "<table class='tab'>";
        myVersionReport += row( mHead );
        QStringList filters;
        filters << "*.log";
        myVersionDir.setNameFilters( filters );
        foreach ( QString myLogFileName, myVersionDir.entryList( QDir::Files ) )
        {
          if ( myLogFileName == "version.log" ) continue;
          myVersionCoverageCount++;
          myCoverageCount++;

          QString myLogPath = myVersionDir.absolutePath() + QDir::separator() + myLogFileName;
          QMap<QString, QString>myLog = readLog( myLogPath );
          myVersionReport += "<tr>";

          QStringList myValues;
          myValues << QString( "<a href='%1'>%2</a>" ).arg( myLog.value( "describeCoverageUrl" ) ).arg( myLog.value( "identifier" ) );
          //myValues << myLog.value( "hasSize" );
          myVersionReport += cells( myValues, "", 1, 2 );
          myValues.clear();

          QStringList issues = issueDescriptions( myServerLog.value( "server" ), myLog.value( "identifier" ), myLog.value( "version" ) );
          QString issuesString = issues.join( "<br>" );

          QStringList providers;
          providers << "wcs" << "gdal";

          bool hasErr = false;
          foreach ( QString provider, providers )
          {
            QString imgPath = myVersionDir.absolutePath() + QDir::separator() + QFileInfo( myLogPath ).completeBaseName() + "-" + provider + ".png";


            if ( !myLog.value( provider + "_error" ).isEmpty() )
            {
              myValues << myLog.value( provider + "_error" );
              int offender = NoOffender;
              if ( provider == "wcs" )
              {
                myValues << issuesString;;

                offender = issueOffender( myServerLog.value( "server" ), myLog.value( "identifier" ), myLog.value( "version" ) );
                myVersionErrCount++;
                hasErr = true;
              }
              QString cls;
              if ( offender == ServerOffender )
              {
                cls = "cell-err-server";
              }
              else if ( offender == QGisOffender )
              {
                cls = "cell-err-qgis";
              }
              else
              {
                cls = "cell-err";
              }
              myVersionReport += cells( myValues, cls, 12 );
              myValues.clear();
            }
            else
            {
              myValues << myLog.value( provider + "_crs" );
              myValues << myLog.value( provider + "_width" );
              myValues << myLog.value( provider + "_height" );
              myValues << QString( myLog.value( provider + "_extent" ) ).replace( ",", "<br>" );
              myValues << "<img src='" + imgPath + "'>";
              myValues << myLog.value( provider + "_bandCount" );
              myValues << myLog.value( provider + "_srcType" );
              myValues << myLog.value( provider + "_min" );
              myValues << myLog.value( provider + "_max" );
              myValues << myLog.value( provider + "_valuesCount" );
              myValues << myLog.value( provider + "_colorsCount" );
              myValues << myLog.value( provider + "_time" );

              QString cls;
              int myValuesCount = myLog.value( provider + "_valuesCount" ).toInt();
              int myColorsCount = myLog.value( provider + "_colorsCount" ).toInt();
              if ( myValuesCount < 4 )
              {
                cls = "cell-err";
                if ( provider == "wcs" )
                {
                  myVersionErrCount++;
                  myCoverageErrCount++;
                }
              }
              else if ( myColorsCount < 4 )
              {
                cls = "cell-warn";
                if ( provider == "wcs" )
                {
                  myVersionWarnCount++;
                  myCoverageWarnCount++;
                }
              }
              myVersionReport += cells( myValues, cls );
              myValues.clear();
            }
          }
          myVersionReport += "<tr>\n";
          QString cls;
          if ( !issuesString.isEmpty() && !hasErr )
          {
            myValues << issuesString;
          }
          else
          {
            myValues << "";
            cls = "cell-empty";
          }
          myVersionReport += cells( myValues, cls, 24 );
          myValues.clear();
          myVersionReport += "</tr>\n";
        } // coverages
        myVersionReport += "</table>\n";
        // prepend counts
        myVersionReport.prepend( QString( "<b>Total coverages: %1</b><br>\n" ).arg( myVersionLog.value( "totalCoverages" ) ) +
                                 QString( "<b>Tested coverages: %1</b><br>\n" ).arg( myVersionCoverageCount ) +
                                 QString( "<b>Errors: %1</b><br>\n" ).arg( myVersionErrCount ) +
                                 QString( "<b>Warnings: %1</b><br><br>" ).arg( myVersionWarnCount ) );
        myServerReport += myVersionReport;
      }
      if ( myVersionErrCount > 0 ) myServerErr = true;
      if ( myVersionWarnCount > 0 ) myServerWarn = true;
    } // versions
    myReport += myServerReport;
    if ( myServerErr ) myServerErrCount++;
    if ( myServerWarn ) myServerWarnCount++;
  } // servers

  QString mySettings = QgsApplication::showSettings();
  mySettings = mySettings.replace( "\n", "<br />" );
  QString myRep = "<h1>WCS public servers test</h1>\n";
  myRep += "<p>" + mySettings + "</p>";

  myRep += "<style>";
  myRep += ".tab { border-spacing: 0px; border-width: 1px 1px 0 0; border-style: solid; }";
  myRep += ".cell { border-width: 0 0 1px 1px; border-style: solid; font-size: smaller; text-align: center}";
  myRep += ".cell-empty { border-width: 0; height:0; padding:0 }";
  myRep += ".cell-ok { background: #ffffff; }";
  myRep += ".cell-warn { background: #ffcc00; }";
  myRep += ".cell-err { background: #ff0000; }";
  myRep += ".cell-err-server { background: #ffff00; }";
  myRep += ".cell-err-qgis { background: #ff0000; }";
  myRep += ".errmsg { color: #ff0000; }";
  myRep += "</style>";

  myRep += QString( "<b>Servers: %1</b><br>\n" ).arg( myServerCount );
  myRep += QString( "<b>Servers with error: %1</b><br>\n" ).arg( myServerErrCount );
  myRep += QString( "<b>Servers with warning: %1</b><br>\n" ).arg( myServerWarnCount );
  myRep += QString( "<b>Coverages: %1</b><br>\n" ).arg( myCoverageCount );
  myRep += QString( "<b>Coverage errors: %1</b><br>\n" ).arg( myCoverageErrCount );
  myRep += QString( "<b>Coverage warnings: %1</b><br>\n" ).arg( myCoverageWarnCount );

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

QString TestQgsWcsPublicServers::cells( QStringList theValues, QString theClass, int colspan, int rowspan )
{
  QString myRow;
  for ( int i = 0; i < theValues.size(); i++ )
  {
    QString val = theValues.value( i );
    QString colspanStr, rowspanStr;
    if ( colspan > 1 && i == theValues.size() - 1 )
    {
      colspanStr = QString( "colspan=%1" ).arg( colspan - theValues.size() + 1 );
    }
    if ( rowspan > 1 )
    {
      rowspanStr = QString( "rowspan=%1" ).arg( rowspan );
    }
    myRow += QString( "<td class='cell %1' %2 %3>%4</td>" ).arg( theClass ).arg( colspanStr ).arg( rowspanStr ).arg( val );
  }
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
      colspan = QString( "colspan=%1" ).arg( mHead.size() - theValues.size() + 1 );
    }
    myRow += QString( "<td class='cell %1' %2>%3</td>" ).arg( theClass ).arg( colspan ).arg( val );
  }
  myRow += "</tr>\n";
  return myRow;
}

/* print usage text */
void usage( std::string const & appName )
{
  std::cerr << "QGIS public WCS servers test - " << VERSION << " '" << RELEASE_NAME << "'\n"
            << "Console application for QGIS WCS provider (WCS client) testing.\n"
            << "Usage: " << appName <<  " [options] CACHE_DIR\n"
            << "  options: \n"
            << "\t[--server URL]\tWCS server URL to be tested.\n"
            << "\t[--coverage coverage]\tCoverage name to be tested.\n"
            << "\t[--num count]\tMaximum number of coverages to test per server. Default 2.\n"
            << "\t[--version version]\tWCS version to be tested.\n"
            << "\t[--force]\tForce retrieve, overwrite cache.\n"
            << "  FILES:\n"
            << "    Path to directory where cached results are stored.\n"
            << "    Coverage once retrieved (success or fail) is not requested again until the cache is deleted.\n";
}

int main( int argc, char *argv[] )
{
#ifdef WIN32  // Windows
#ifdef _MSC_VER
  _set_fmode( _O_BINARY );
#else //MinGW
  _fmode = _O_BINARY;
#endif  // _MSC_VER
#endif  // WIN32

  QString myServer;
  QString myCoverage;
  QString myVersion;
  int myMaxCoverages = 2;
  bool myForce = false;

#ifndef WIN32
  int optionChar;
  static struct option long_options[] =
  {
    {"help",     no_argument,       0, 'h'},
    {"server",   required_argument, 0, 's'},
    {"coverage", required_argument, 0, 'c'},
    {"num",      required_argument, 0, 'n'},
    {"version",  required_argument, 0, 'v'},
    {"force",    no_argument,       0, 'f'},
    {0, 0, 0, 0}
  };

  while ( 1 )
  {
    /* getopt_long stores the option index here. */
    int option_index = 0;

    optionChar = getopt_long( argc, argv, "hscnvf",
                              long_options, &option_index );

    /* Detect the end of the options. */
    if ( optionChar == -1 )
      break;

    switch ( optionChar )
    {
      case 0:
        /* If this option set a flag, do nothing else now. */
        if ( long_options[option_index].flag != 0 )
          break;
        printf( "option %s", long_options[option_index].name );
        if ( optarg )
          printf( " with arg %s", optarg );
        printf( "\n" );
        break;

      case 's':
        myServer = QString( optarg );
        break;

      case 'c':
        myCoverage = QString( optarg );
        break;

      case 'n':
        myMaxCoverages = QString( optarg ).toInt();
        break;

      case 'v':
        myVersion = QString( optarg );
        break;

      case 'f':
        myForce = true;
        break;

      case 'h':
        usage( argv[0] );
        return 2;   // XXX need standard exit codes
        break;

      default:
        QgsDebugMsg( QString( "%1: getopt returned character code %2" ).arg( argv[0] ).arg( optionChar ) );
        return 1;   // XXX need standard exit codes
    }

  }

  QgsDebugMsg( QString( "myServer = %1" ).arg( myServer ) );
  QgsDebugMsg( QString( "myCoverage = %1" ).arg( myCoverage ) );
  QgsDebugMsg( QString( "myMaxCoverages = %1" ).arg( myMaxCoverages ) );
  QgsDebugMsg( QString( "myVersion = %1" ).arg( myVersion ) );

  if ( !myCoverage.isEmpty() && myServer.isEmpty() )
  {
    std::cerr << "--coverage can only be specified if --server is also used";
    return 1;
  }

  QgsDebugMsg( QString( "optind = %1 argc = %2" ).arg( optind ).arg( argc ) );
  if ( optind > argc - 1 )
  {
    std::cerr << "CACHE_DIR missing.\n";
    usage( argv[0] );
    return 1;
  }
  else if ( optind < argc - 1 )
  {
    std::cerr << "One CACHE_DIR only allowed.\n";
    usage( argv[0] );
    return 1;
  }

  QString myCacheDirPath = QDir::toNativeSeparators( QFileInfo( QFile::decodeName( argv[optind] ) ).absoluteFilePath() );

  QgsDebugMsg( "myCacheDirPath = " + myCacheDirPath );

#else
  // Not yet supported on Windows (missing options parser)
  std::cerr << "Not supported on Windows";
  QCoreApplication::exit( 0 );
#endif

  QgsApplication myApp( argc, argv, false );
  QgsApplication::init( QString() );
  QgsApplication::initQgis();

  TestQgsWcsPublicServers myTest( myCacheDirPath, myMaxCoverages, myServer, myCoverage, myVersion, myForce );
  myTest.init();
  myTest.test();
  myTest.report();

  QCoreApplication::exit( 0 );
}
