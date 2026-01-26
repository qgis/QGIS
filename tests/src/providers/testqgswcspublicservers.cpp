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

#include "testqgswcspublicservers.h"

#include "qgsapplication.h"
#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "qgsmaprenderersequentialjob.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsproject.h"
#include "qgsproviderregistry.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterinterface.h"
#include "qgsrasterlayer.h"
#include "qgswcscapabilities.h"

#include <QApplication>
#include <QImage>
#include <QObject>
#include <QPainter>
#include <QRegularExpression>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QTextStream>

#ifdef Q_OS_WIN
#include <fcntl.h> /*  _O_BINARY */
#else
#include <getopt.h>
#endif

TestQgsWcsPublicServers::TestQgsWcsPublicServers( const QString &cacheDirPath, int maxCoverages, const QString &server, const QString &coverage, const QString &version, bool force )
  : mCacheDirPath( cacheDirPath )
  , mMaxCoverages( maxCoverages )
  , mServer( server )
  , mCoverage( coverage )
  , mVersion( version )
  , mForce( force )
{
}

TestQgsWcsPublicServers::~TestQgsWcsPublicServers()
{
  QgsNetworkAccessManager::settingsNetworkTimeout->setValue( mOrigTimeout );
}

//runs before all tests
void TestQgsWcsPublicServers::init()
{
  // init QGIS's paths - true means that all path will be inited from prefix

  // Unfortunately this seems to be the only way to set timeout, we try to reset it
  // at the end but it can be canceled before ...
  mOrigTimeout = QgsNetworkAccessManager::settingsNetworkTimeout->value();
  QgsNetworkAccessManager::settingsNetworkTimeout->setValue( mTimeout );

  //mCacheDir = QDir( "./wcstestcache" );
  mCacheDir = QDir( mCacheDirPath );
  if ( !mCacheDir.exists() )
  {
    QDir myDir = QDir::root();
    if ( !myDir.mkpath( mCacheDir.absolutePath() ) )
    {
      QgsDebugError( "Cannot create cache dir " + mCacheDir.absolutePath() );
      QCoreApplication::exit( 1 );
    }
  }

  mHead << u"Coverage"_s;

  QStringList providers;
  providers << u"wcs"_s << u"gdal"_s;
  for ( const QString &provider : providers )
  {
    QString prefix = provider == "gdal"_L1 ? "GDAL " : "";
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
  QString path = QgsApplication::pkgDataPath() + "/resources/wcs-servers.json";
  QFile file( path );
  if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    QString data = file.readAll();
    //QgsDebugMsgLevel("servers: \n"  + str, 1 );
    file.close();
    QJsonDocument doc = QJsonDocument::fromJson( data.toUtf8() );
    const QJsonObject result = doc.object();

    QJsonObject::ConstIterator serverIt = result.constBegin();
    for ( ; serverIt != result.constEnd(); serverIt++ )
    {
      const QJsonObject serverObject = serverIt.value().toObject();
      const QString serverUrl = serverObject.value( "url"_L1 ).toString();

      QgsDebugMsgLevel( "serverUrl: " + serverUrl, 1 );

      Server server( serverUrl );
      server.description = serverObject.value( "description"_L1 ).toString();

      const QJsonObject serverParams = serverObject.value( "params"_L1 ).toObject();
      QJsonObject::ConstIterator paramsIt = serverParams.constBegin();
      for ( ; paramsIt != serverParams.constEnd(); paramsIt++ )
      {
        QgsDebugMsgLevel( u"params value: %1"_s.arg( paramsIt.value().toString() ), 1 );
        server.params.insert( paramsIt.key(), paramsIt.value().toString() );
      }

      QJsonObject issuesObject = serverObject.value( "issues"_L1 ).toObject();

      QJsonObject::ConstIterator issuesIt = issuesObject.constBegin();
      for ( ; issuesIt != issuesObject.constEnd(); ++issuesIt )
      {
        QJsonObject issueObject = issuesIt.value().toObject();

        QString description = issueObject.value( "description"_L1 ).toString();
        QgsDebugMsgLevel( "description: " + description, 1 );
        Issue issue( description );

        issue.offender = issueObject.value( "offender"_L1 ).toString();

        QJsonObject coveragesObject = issueObject.value( "coverages"_L1 ).toObject();
        QJsonObject::ConstIterator coverageIt = coveragesObject.constBegin();
        for ( ; coverageIt != coveragesObject.constEnd(); ++coverageIt )
        {
          issue.coverages << coverageIt.value().toString();
        }

        QJsonObject versionsObject = issueObject.value( "versions"_L1 ).toObject();
        QJsonObject::ConstIterator versionsIt = versionsObject.constBegin();
        for ( ; versionsIt != versionsObject.constEnd(); ++versionsIt )
        {
          issue.versions << versionsIt.value().toString();
        }

        server.issues << issue;
      }

      mServers << server;
    }
  }
  else
  {
    QgsDebugError( "Cannot open " + path );
  }
}

TestQgsWcsPublicServers::Server TestQgsWcsPublicServers::getServer( const QString &url )
{
  for ( const Server &server : std::as_const( mServers ) )
  {
    if ( server.url == url )
      return server;
  }
  return Server();
}

QList<TestQgsWcsPublicServers::Issue> TestQgsWcsPublicServers::issues( const QString &url, const QString &coverage, const QString &version )
{
  QList<Issue> issues;
  for ( const Server &server : std::as_const( mServers ) )
  {
    if ( server.url == url )
    {
      for ( const Issue &issue : server.issues )
      {
        if ( ( issue.coverages.isEmpty() || issue.coverages.contains( coverage ) ) && ( issue.versions.isEmpty() || issue.versions.contains( version ) ) )
        {
          issues << issue;
        }
      }
    }
  }
  return issues;
}

QStringList TestQgsWcsPublicServers::issueDescriptions( const QString &url, const QString &coverage, const QString &version )
{
  QStringList descriptions;
  for ( const Issue &myIssue : issues( url, coverage, version ) )
  {
    descriptions << myIssue.description;
  }
  return descriptions;
}

int TestQgsWcsPublicServers::issueOffender( const QString &url, const QString &coverage, const QString &version )
{
  int offender = NoOffender;
  for ( const Issue &myIssue : issues( url, coverage, version ) )
  {
    if ( myIssue.offender == "server"_L1 )
    {
      offender |= ServerOffender;
    }
    else
    {
      offender |= QgisOffender;
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
    versions << u"1.0.0"_s << u"1.1.0"_s;
  }


  if ( !mServer.isEmpty() )
  {
    serverUrls << mServer;
  }
  else
  {
    for ( const Server &server : std::as_const( mServers ) )
    {
      serverUrls << server.url;
    }
  }

  for ( const QString &serverUrl : serverUrls )
  {
    Server myServer = getServer( serverUrl );
    QStringList myServerLog;
    myServerLog << "server:" + serverUrl;
    QString myServerDirName = serverUrl;
    myServerDirName.replace( QRegularExpression( "[:/]+" ), u"."_s );
    myServerDirName.remove( QRegularExpression( "\\.$" ) );
    QgsDebugMsgLevel( "myServerDirName = " + myServerDirName, 1 );

    QDir myServerDir( mCacheDir.absolutePath() + '/' + myServerDirName );

    if ( !myServerDir.exists() )
    {
      mCacheDir.mkdir( myServerDirName );
    }

    QString myServerLogPath = myServerDir.absolutePath() + "/server.log";

    for ( const QString &version : versions )
    {
      QgsDebugMsgLevel( "server: " + serverUrl + " version: " + version, 1 );
      QStringList myVersionLog;
      myVersionLog << "version:" + version;

      QString myVersionDirName = "v" + version;
      QString myVersionDirPath = myServerDir.absolutePath() + '/' + myVersionDirName;

      QString myVersionLogPath = myVersionDirPath + "/version.log";

      QDir myVersionDir( myVersionDirPath );
      if ( !myVersionDir.exists() )
      {
        myServerDir.mkdir( myVersionDirName );
      }

      QgsDataSourceUri myServerUri;

      myServerUri.setParam( u"url"_s, serverUrl );
      if ( !version.isEmpty() )
      {
        myServerUri.setParam( u"version"_s, version );
      }
      myServerUri.setParam( u"cache"_s, u"AlwaysNetwork"_s );

      for ( auto it = myServer.params.constBegin(); it != myServer.params.constEnd(); it++ )
      {
        myServerUri.setParam( it.key(), it.value() );
      }

      QgsWcsCapabilities myCapabilities;
      myCapabilities.setUri( myServerUri );


      if ( !myCapabilities.lastError().isEmpty() )
      {
        QgsDebugError( myCapabilities.lastError() );
        myVersionLog << "error:" + myCapabilities.lastError().replace( '\n', ' ' );
        continue;
      }

      myVersionLog << "getCapabilitiesUrl:" + myCapabilities.getCapabilitiesUrl();

      QVector<QgsWcsCoverageSummary> myCoverages;
      if ( !myCapabilities.supportedCoverages( myCoverages ) )
      {
        QgsDebugError( u"Cannot get list of coverages"_s );
        myVersionLog << u"error:Cannot get list of coverages"_s;
        continue;
      }

      myVersionLog << u"totalCoverages:%1"_s.arg( myCoverages.size() );

      int myCoverageCount = 0;
      int myStep = myCoverages.size() / std::min<int>( mMaxCoverages, myCoverages.size() );
      int myStepCount = -1;
      bool myCoverageFound = false;
      for ( QgsWcsCoverageSummary myCoverage : myCoverages )
      {
        QgsDebugMsgLevel( "coverage: " + myCoverage.identifier, 1 );
        if ( !mCoverage.isEmpty() && myCoverage.identifier != mCoverage )
          continue;
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
        if ( myCoverageCount > mMaxCoverages )
          break;


        QString myPath = myVersionDirPath + '/' + myCoverage.identifier;
        QString myLogPath = myPath + ".log";

        if ( QFileInfo::exists( myLogPath ) && !mForce )
        {
          //QMap<QString, QString> log = readLog( myLogPath );
          //if ( !log.value( "identifier" ).isEmpty() && log.value( "error" ).isEmpty() ) continue;
          continue;
        }

        QStringList myLog;
        myLog << "identifier:" + myCoverage.identifier;
        myCapabilities.describeCoverage( myCoverage.identifier );
        myCoverage = myCapabilities.coverage( myCoverage.identifier ); // get described
        QgsDataSourceUri myUri = myServerUri;
        myUri.setParam( u"identifier"_s, myCoverage.identifier );
        if ( !myCoverage.times.isEmpty() )
        {
          myUri.setParam( u"time"_s, myCoverage.times.value( 0 ) );
        }
        myLog << "version:" + version;
        myLog << "describeCoverageUrl:" + myCapabilities.getDescribeCoverageUrl( myCoverage.identifier );
        // Test time
        //myLog << "date:" + QString( "%1").arg( QDateTime::currentDateTime().toTime_t() );
        myLog << "date:" + QDateTime::currentDateTime().toString();

        int myWidth = 100;
        int myHeight = 100;
        if ( myCoverage.hasSize )
        {
          myHeight = static_cast<int>( std::round( 1.0 * myWidth * myCoverage.height / myCoverage.width ) );
        }
        myLog << u"hasSize:%1"_s.arg( myCoverage.hasSize );

        // Test QGIS provider and via GDAL
        QStringList providers;
        providers << u"wcs"_s << u"gdal"_s;

        for ( const QString &provider : providers )
        {
          QElapsedTimer time;
          time.start();
          QString uri;
          if ( provider == "wcs"_L1 )
          {
            uri = myUri.encodedUri();
          }
          else // gdal
          {
            uri = myPath + "-gdal.xml";
            QFile myGdalXmlFile( uri );
            Q_ASSERT( myGdalXmlFile.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) );
            QTextStream myStream( &myGdalXmlFile );
            myStream << "<WCS_GDAL>\n";
            myStream << "  <ServiceURL>" + serverUrl + '?' + "</ServiceURL>\n";
            myStream << "  <CoverageName>" + myCoverage.identifier + "</CoverageName>\n";
            myStream << "  <Version>" + version + "</Version>\n";
            myStream << u"  <Timeout>%1</Timeout>\n"_s.arg( mTimeout / 1000., 0, 'd' );
            myStream << "</WCS_GDAL>\n";

            myGdalXmlFile.close();
          }

          QgsRasterLayer *myLayer = new QgsRasterLayer( uri, myCoverage.identifier, provider );
          if ( myLayer->isValid() )
          {
            myLog << provider + "_crs:" + myLayer->dataProvider()->crs().authid();
            myLog << provider + "_width:" + QString::number( myLayer->dataProvider()->xSize() );
            myLog << provider + "_height:" + QString::number( myLayer->dataProvider()->ySize() );
            QgsRectangle extent = myLayer->dataProvider()->extent();
            myLog << provider + "_extent:"
                       + QgsRasterBlock::printValue( extent.xMinimum() ) + ','
                       + QgsRasterBlock::printValue( extent.yMinimum() ) + ','
                       + QgsRasterBlock::printValue( extent.xMaximum() ) + ','
                       + QgsRasterBlock::printValue( extent.yMaximum() ) + ',';
            int myBandCount = myLayer->dataProvider()->bandCount();
            myLog << provider + "_bandCount:" + QString::number( myBandCount );
            if ( myBandCount > 0 )
            {
              myLog << provider + "_srcType:" + qgsEnumValueToKey<Qgis::DataType>( myLayer->dataProvider()->sourceDataType( 1 ) );

              QgsRasterBandStats myStats = myLayer->dataProvider()->bandStatistics( 1, Qgis::RasterBandStatistic::All, QgsRectangle(), myWidth * myHeight );
              myLog << provider + "_min:" + QString::number( myStats.minimumValue );
              myLog << provider + "_max:" + QString::number( myStats.maximumValue );
            }

            QgsProject::instance()->addMapLayer( myLayer, false );

            QgsMapSettings mapSettings;
            mapSettings.setLayers( QList<QgsMapLayer *>() << myLayer );
            mapSettings.setExtent( myLayer->extent() );
            mapSettings.setOutputSize( QSize( myWidth, myHeight ) );

            QgsMapRendererSequentialJob job( mapSettings );
            job.start();
            job.waitForFinished();
            QImage myImage( job.renderedImage() );

            // Save rendered image
            QString myPngPath = myPath + "-" + provider + ".png";
            QgsDebugMsgLevel( "myPngPath = " + myPngPath, 1 );
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
                  if ( !myValues.contains( valueStr ) )
                    myValues.insert( valueStr );
                }
              }
              delete myBlock;
            }
            QgsDebugMsgLevel( u"%1 values"_s.arg( myValues.size() ), 1 );
            myLog << provider + u"_valuesCount:%1"_s.arg( myValues.size() );

            // Verify image colors
            QSet<QRgb> myColors;
            for ( int row = 0; row < myHeight; row++ )
            {
              for ( int col = 0; col < myWidth; col++ )
              {
                QRgb color = myImage.pixel( col, row );
                if ( !myColors.contains( color ) )
                  myColors.insert( color );
              }
            }
            QgsDebugMsgLevel( u"%1 colors"_s.arg( myColors.size() ), 1 );
            myLog << provider + u"_colorsCount:%1"_s.arg( myColors.size() );
          }
          else
          {
            QgsDebugError( u"Layer is not valid"_s );
            myLog << provider + "_error:Layer is not valid";
          }
          myLog << provider + u"_time:%1"_s.arg( time.elapsed() / 1000., 0, 'f', 2 );
          // Generate report for impatient people
          report();
        }

        QFile myLogFile( myLogPath );

        Q_ASSERT( myLogFile.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) );
        QTextStream myStream( &myLogFile );
        myStream << myLog.join( QLatin1Char( '\n' ) );

        myLogFile.close();
        QgsProject::instance()->removeAllMapLayers();
      }
      if ( !mCoverage.isEmpty() && !myCoverageFound )
      {
        QgsDebugError( u"Coverage not found"_s );
      }
      QFile myVersionLogFile( myVersionLogPath );
      Q_ASSERT( myVersionLogFile.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) );
      QTextStream myVersionStream( &myVersionLogFile );
      myVersionStream << myVersionLog.join( QLatin1Char( '\n' ) );
      myVersionLogFile.close();
    }
    QFile myServerLogFile( myServerLogPath );
    Q_ASSERT( myServerLogFile.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) );
    QTextStream myServerStream( &myServerLogFile );
    myServerStream << myServerLog.join( QLatin1Char( '\n' ) );
    myServerLogFile.close();
  }
}

void TestQgsWcsPublicServers::writeReport( const QString &report )
{
  QString myReportFile = mCacheDir.absolutePath() + "/index.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    QTextStream myStream( &myFile );
    myStream << report;
    myFile.close();
  }
  QgsDebugMsgLevel( "Report written to " + myReportFile, 1 );
}

void TestQgsWcsPublicServers::report()
{
  QString myReport;

  int myServerCount = 0;
  int myServerErrCount = 0;  // at least one error
  int myServerWarnCount = 0; // at least one error
  int myCoverageCount = 0;
  int myCoverageErrCount = 0;
  int myCoverageWarnCount = 0;

  for ( const QString &myServerDirName : mCacheDir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) )
  {
    myServerCount++;
    QDir myServerDir( mCacheDir.absolutePath() + '/' + myServerDirName );

    QString myServerLogPath = myServerDir.absolutePath() + "/server.log";
    QMap<QString, QString> myServerLog = readLog( myServerLogPath );

    myReport += u"<h2>Server: %1</h2>"_s.arg( myServerLog.value( u"server"_s ) );
    Server myServer = getServer( myServerLog.value( u"server"_s ) );
    if ( !myServer.description.isEmpty() )
    {
      myReport += myServer.description + "<br>\n";
    }
    if ( !myServer.params.isEmpty() )
    {
      myReport += "<br>Additional params: "_L1;
      for ( auto it = myServer.params.constBegin(); it != myServer.params.constEnd(); it++ )
      {
        myReport += it.key() + '=' + it.value() + " ";
      }
      myReport += "<br>\n"_L1;
    }

    QString myServerReport;

    bool myServerErr = false;
    bool myServerWarn = false;
    for ( const QString &myVersionDirName : myServerDir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) )
    {
      QString myVersionReport;
      int myVersionCoverageCount = 0;
      int myVersionErrCount = 0;
      int myVersionWarnCount = 0;

      QString myVersionDirPath = myServerDir.absolutePath() + '/' + myVersionDirName;
      QString myVersionLogPath = myVersionDirPath + "/version.log";
      QMap<QString, QString> myVersionLog = readLog( myVersionLogPath );
      QDir myVersionDir( myVersionDirPath );

      QString myVersion = myVersionLog.value( u"version"_s );
      myServerReport += u"<h3><a href='%1'>Version: %2</a></h3>"_s.arg( myVersionLog.value( u"getCapabilitiesUrl"_s ), myVersion.isEmpty() ? u"(empty)"_s : myVersion );

      if ( !myVersionLog.value( u"error"_s ).isEmpty() )
      {
        // Server may have more errors, for each version
        //for ( QString err : myServerLog.values( "error" ) )
        //{
        //myVersionReport += error( err );
        //}
        myVersionReport += error( myServerLog.value( u"error"_s ) );
        myVersionErrCount++;
      }
      else
      {
        myVersionReport += "<table class='tab'>"_L1;
        myVersionReport += row( mHead );
        QStringList filters;
        filters << u"*.log"_s;
        myVersionDir.setNameFilters( filters );
        for ( const QString &myLogFileName : myVersionDir.entryList( QDir::Files ) )
        {
          if ( myLogFileName == "version.log"_L1 )
            continue;
          myVersionCoverageCount++;
          myCoverageCount++;

          QString myLogPath = myVersionDir.absolutePath() + '/' + myLogFileName;
          QMap<QString, QString> myLog = readLog( myLogPath );
          myVersionReport += "<tr>"_L1;

          QStringList myValues;
          myValues << u"<a href='%1'>%2</a>"_s.arg( myLog.value( u"describeCoverageUrl"_s ), myLog.value( u"identifier"_s ) );
          //myValues << myLog.value( "hasSize" );
          myVersionReport += cells( myValues, QString(), 1, 2 );
          myValues.clear();

          QStringList issues = issueDescriptions( myServerLog.value( u"server"_s ), myLog.value( u"identifier"_s ), myLog.value( u"version"_s ) );
          QString issuesString = issues.join( "<br>"_L1 );

          QStringList providers;
          providers << u"wcs"_s << u"gdal"_s;

          bool hasErr = false;
          for ( const QString &provider : providers )
          {
            QString imgPath = myVersionDir.absolutePath() + '/' + QFileInfo( myLogPath ).completeBaseName() + "-" + provider + ".png";


            if ( !myLog.value( provider + "_error" ).isEmpty() )
            {
              myValues << myLog.value( provider + "_error" );
              int offender = NoOffender;
              if ( provider == "wcs"_L1 )
              {
                myValues << issuesString;

                offender = issueOffender( myServerLog.value( u"server"_s ), myLog.value( u"identifier"_s ), myLog.value( u"version"_s ) );
                myVersionErrCount++;
                hasErr = true;
              }
              QString cls;
              if ( offender == ServerOffender )
              {
                cls = u"cell-err-server"_s;
              }
              else if ( offender == QgisOffender )
              {
                cls = u"cell-err-qgis"_s;
              }
              else
              {
                cls = u"cell-err"_s;
              }
              myVersionReport += cells( myValues, cls, 12 );
              myValues.clear();
            }
            else
            {
              myValues << myLog.value( provider + "_crs" );
              myValues << myLog.value( provider + "_width" );
              myValues << myLog.value( provider + "_height" );
              myValues << QString( myLog.value( provider + "_extent" ) ).replace( ',', "<br>"_L1 );
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
                cls = u"cell-err"_s;
                if ( provider == "wcs"_L1 )
                {
                  myVersionErrCount++;
                  myCoverageErrCount++;
                }
              }
              else if ( myColorsCount < 4 )
              {
                cls = u"cell-warn"_s;
                if ( provider == "wcs"_L1 )
                {
                  myVersionWarnCount++;
                  myCoverageWarnCount++;
                }
              }
              myVersionReport += cells( myValues, cls );
              myValues.clear();
            }
          }
          myVersionReport += "<tr>\n"_L1;
          QString cls;
          if ( !issuesString.isEmpty() && !hasErr )
          {
            myValues << issuesString;
          }
          else
          {
            myValues << QString();
            cls = u"cell-empty"_s;
          }
          myVersionReport += cells( myValues, cls, 24 );
          myValues.clear();
          myVersionReport += "</tr>\n"_L1;
        } // coverages
        myVersionReport += "</table>\n"_L1;
        // prepend counts
        myVersionReport.prepend( u"<b>Total coverages: %1</b><br>\n"_s.arg( myVersionLog.value( u"totalCoverages"_s ) ) + u"<b>Tested coverages: %1</b><br>\n"_s.arg( myVersionCoverageCount ) + u"<b>Errors: %1</b><br>\n"_s.arg( myVersionErrCount ) + u"<b>Warnings: %1</b><br><br>"_s.arg( myVersionWarnCount ) );
        myServerReport += myVersionReport;
      }
      if ( myVersionErrCount > 0 )
        myServerErr = true;
      if ( myVersionWarnCount > 0 )
        myServerWarn = true;
    } // versions
    myReport += myServerReport;
    if ( myServerErr )
      myServerErrCount++;
    if ( myServerWarn )
      myServerWarnCount++;
  } // servers

  QString mySettings = QgsApplication::showSettings();
  mySettings = mySettings.replace( '\n', "<br />"_L1 );
  QString myRep = u"<h1>WCS public servers test</h1>\n"_s;
  myRep += "<p>" + mySettings + "</p>";

  myRep += "<style>"_L1;
  myRep += ".tab { border-spacing: 0px; border-width: 1px 1px 0 0; border-style: solid; }"_L1;
  myRep += ".cell { border-width: 0 0 1px 1px; border-style: solid; font-size: smaller; text-align: center}"_L1;
  myRep += ".cell-empty { border-width: 0; height:0; padding:0 }"_L1;
  myRep += ".cell-ok { background: #ffffff; }"_L1;
  myRep += ".cell-warn { background: #ffcc00; }"_L1;
  myRep += ".cell-err { background: #ff0000; }"_L1;
  myRep += ".cell-err-server { background: #ffff00; }"_L1;
  myRep += ".cell-err-qgis { background: #ff0000; }"_L1;
  myRep += ".errmsg { color: #ff0000; }"_L1;
  myRep += "</style>"_L1;

  myRep += u"<b>Servers: %1</b><br>\n"_s.arg( myServerCount );
  myRep += u"<b>Servers with error: %1</b><br>\n"_s.arg( myServerErrCount );
  myRep += u"<b>Servers with warning: %1</b><br>\n"_s.arg( myServerWarnCount );
  myRep += u"<b>Coverages: %1</b><br>\n"_s.arg( myCoverageCount );
  myRep += u"<b>Coverage errors: %1</b><br>\n"_s.arg( myCoverageErrCount );
  myRep += u"<b>Coverage warnings: %1</b><br>\n"_s.arg( myCoverageWarnCount );

  myRep += myReport;

  writeReport( myRep );
}

QMap<QString, QString> TestQgsWcsPublicServers::readLog( const QString &fileName )
{
  QMap<QString, QString> myMap;

  QFile myFile( fileName );
  if ( myFile.open( QIODevice::ReadOnly ) )
  {
    QTextStream myStream( &myFile );
    for ( const QString &row : myStream.readAll().split( '\n' ) )
    {
      int sepIdx = row.indexOf( ':' );
      myMap.insert( row.left( sepIdx ), row.mid( sepIdx + 1 ) );
    }
    myFile.close();
  }
  return myMap;
}

QString TestQgsWcsPublicServers::error( const QString &message )
{
  QString myRow = u"<font class='errmsg'>Error: "_s;
  myRow += message;
  myRow += "</font>"_L1;
  return myRow;
}

QString TestQgsWcsPublicServers::cells( const QStringList &values, const QString &classStr, int colspan, int rowspan )
{
  QString myRow;
  for ( int i = 0; i < values.size(); i++ )
  {
    QString val = values.value( i );
    QString colspanStr, rowspanStr;
    if ( colspan > 1 && i == values.size() - 1 )
    {
      colspanStr = u"colspan=%1"_s.arg( colspan - values.size() + 1 );
    }
    if ( rowspan > 1 )
    {
      rowspanStr = u"rowspan=%1"_s.arg( rowspan );
    }
    myRow += u"<td class='cell %1' %2 %3>%4</td>"_s.arg( classStr, colspanStr, rowspanStr, val );
  }
  return myRow;
}

QString TestQgsWcsPublicServers::row( const QStringList &values, const QString &classStr )
{
  QString myRow = u"<tr>"_s;
  for ( int i = 0; i < values.size(); i++ )
  {
    QString val = values.value( i );
    QString colspan;
    if ( values.size() < mHead.size() && i == ( values.size() - 1 ) )
    {
      colspan = u"colspan=%1"_s.arg( mHead.size() - values.size() + 1 );
    }
    myRow += u"<td class='cell %1' %2>%3</td>"_s.arg( classStr, colspan, val );
  }
  myRow += "</tr>\n"_L1;
  return myRow;
}

/* print usage text */
void usage( std::string const &appName )
{
  std::cerr << "QGIS public WCS servers test - " << VERSION << " '" << RELEASE_NAME << "'\n"
            << "Console application for QGIS WCS provider (WCS client) testing.\n"
            << "Usage: " << appName << " [options] CACHE_DIR\n"
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
#ifdef Q_OS_WIN // Windows
#ifdef _MSC_VER
  _set_fmode( _O_BINARY );
#else  //MinGW
  _fmode = _O_BINARY;
#endif // _MSC_VER
#endif // Q_OS_WIN

  QString myServer;
  QString myCoverage;
  QString myVersion;
  int myMaxCoverages = 2;
  bool myForce = false;

#ifndef Q_OS_WIN
  int optionChar;
  static struct option long_options[] = {
    { "help", no_argument, nullptr, 'h' },
    { "server", required_argument, nullptr, 's' },
    { "coverage", required_argument, nullptr, 'c' },
    { "num", required_argument, nullptr, 'n' },
    { "version", required_argument, nullptr, 'v' },
    { "force", no_argument, nullptr, 'f' },
    { nullptr, 0, nullptr, 0 }
  };

  while ( true )
  {
    /* getopt_long stores the option index here. */
    int option_index = 0;

    optionChar = getopt_long( argc, argv, "hscnvf", long_options, &option_index );

    /* Detect the end of the options. */
    if ( optionChar == -1 )
      break;

    switch ( optionChar )
    {
      case 0:
        /* If this option set a flag, do nothing else now. */
        if ( long_options[option_index].flag != nullptr )
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
        return 2; // XXX need standard exit codes

      default:
        QgsDebugMsgLevel( u"%1: getopt returned character code %2"_s.arg( argv[0] ).arg( optionChar ), 1 );
        return 1; // XXX need standard exit codes
    }
  }

  QgsDebugMsgLevel( u"myServer = %1"_s.arg( myServer ), 1 );
  QgsDebugMsgLevel( u"myCoverage = %1"_s.arg( myCoverage ), 1 );
  QgsDebugMsgLevel( u"myMaxCoverages = %1"_s.arg( myMaxCoverages ), 1 );
  QgsDebugMsgLevel( u"myVersion = %1"_s.arg( myVersion ), 1 );

  if ( !myCoverage.isEmpty() && myServer.isEmpty() )
  {
    std::cerr << "--coverage can only be specified if --server is also used";
    return 1;
  }

  QgsDebugMsgLevel( u"optind = %1 argc = %2"_s.arg( optind ).arg( argc ), 1 );
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

  QgsDebugMsgLevel( "myCacheDirPath = " + myCacheDirPath, 1 );

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
