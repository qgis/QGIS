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
#include <QRegularExpression>

#include "qgsapplication.h"
#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgsmaprenderersequentialjob.h"
#include "qgsproviderregistry.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterinterface.h"
#include "qgsrasterlayer.h"
#include "qgswcscapabilities.h"
#include "testqgswcspublicservers.h"
#include "qgssettings.h"

#ifdef Q_OS_WIN
#include <fcntl.h> /*  _O_BINARY */
#else
#include <getopt.h>
#endif

TestQgsWcsPublicServers::TestQgsWcsPublicServers( const QString &cacheDirPath, int maxCoverages, const QString &server, const QString &coverage, const QString &version, bool force ):
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
  QgsSettings settings;
  settings.setValue( QStringLiteral( "/qgis/networkAndProxy/networkTimeout" ), mOrigTimeout );
}

//runs before all tests
void TestQgsWcsPublicServers::init()
{
  // init QGIS's paths - true means that all path will be inited from prefix

  // Unfortunately this seems to be the only way to set timeout, we try to reset it
  // at the end but it can be canceled before ...
  QgsSettings settings;
  mOrigTimeout = settings.value( QStringLiteral( "/qgis/networkAndProxy/networkTimeout" ), "60000" ).toInt();
  settings.setValue( QStringLiteral( "/qgis/networkAndProxy/networkTimeout" ), mTimeout );

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

  mHead << QStringLiteral( "Coverage" );

  QStringList providers;
  providers << QStringLiteral( "wcs" ) << QStringLiteral( "gdal" );
  for ( const QString &provider : providers )
  {
    QString prefix = provider == QLatin1String( "gdal" ) ? "GDAL " : "";
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
    QJsonDocument doc = QJsonDocument::fromJson( data.toUtf8() );
    const QJsonObject result = doc.object();

    QJsonObject::ConstIterator serverIt = result.constBegin();
    for ( ; serverIt != result.constEnd(); serverIt++ )
    {
      const QJsonObject serverObject = serverIt.value().toObject();
      const QString serverUrl = serverObject.value( QLatin1String( "url" ) ).toString();

      QgsDebugMsg( "serverUrl: " + serverUrl );

      Server server( serverUrl );
      server.description = serverObject.value( QLatin1String( "description" ) ).toString();

      const QJsonObject serverParams = serverObject.value( QLatin1String( "params" ) ).toObject();
      QJsonObject::ConstIterator paramsIt = serverParams.constBegin();
      for ( ; paramsIt != serverParams.constEnd(); paramsIt++ )
      {
        QgsDebugMsg( QStringLiteral( "params value: %1" ).arg( paramsIt.value().toString() ) );
        server.params.insert( paramsIt.key(), paramsIt.value().toString() );
      }

      QJsonObject issuesObject = serverObject.value( QLatin1String( "issues" ) ).toObject();

      QJsonObject::ConstIterator issuesIt = issuesObject.constBegin();
      for ( ; issuesIt != issuesObject.constEnd(); ++issuesIt )
      {
        QJsonObject issueObject = issuesIt.value().toObject();

        QString description = issueObject.value( QLatin1String( "description" ) ).toString();
        QgsDebugMsg( "description: " + description );
        Issue issue( description );

        issue.offender = issueObject.value( QLatin1String( "offender" ) ).toString();

        QJsonObject coveragesObject = issueObject.value( QLatin1String( "coverages" ) ).toObject();
        QJsonObject::ConstIterator coverageIt = coveragesObject.constBegin();
        for ( ; coverageIt != coveragesObject.constEnd(); ++coverageIt )
        {
          issue.coverages << coverageIt.value().toString();
        }

        QJsonObject versionsObject = issueObject.value( QLatin1String( "versions" ) ).toObject();
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
    QgsDebugMsg( "Cannot open " + path );
  }
}

TestQgsWcsPublicServers::Server TestQgsWcsPublicServers::getServer( const QString &url )
{
  for ( const Server &server : std::as_const( mServers ) )
  {
    if ( server.url == url ) return server;
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
        if ( ( issue.coverages.isEmpty() || issue.coverages.contains( coverage ) ) &&
             ( issue.versions.isEmpty() || issue.versions.contains( version ) ) )
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
    if ( myIssue.offender == QLatin1String( "server" ) )
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
    versions << QStringLiteral( "1.0.0" ) << QStringLiteral( "1.1.0" );
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
    myServerDirName.replace( QRegularExpression( "[:/]+" ), QStringLiteral( "." ) );
    myServerDirName.remove( QRegularExpression( "\\.$" ) );
    QgsDebugMsg( "myServerDirName = " + myServerDirName );

    QDir myServerDir( mCacheDir.absolutePath() + '/' + myServerDirName );

    if ( !myServerDir.exists() )
    {
      mCacheDir.mkdir( myServerDirName );
    }

    QString myServerLogPath = myServerDir.absolutePath() + "/server.log";

    for ( const QString &version : versions )
    {
      QgsDebugMsg( "server: " + serverUrl + " version: " + version );
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

      myServerUri.setParam( QStringLiteral( "url" ), serverUrl );
      if ( !version.isEmpty() )
      {
        myServerUri.setParam( QStringLiteral( "version" ), version );
      }
      myServerUri.setParam( QStringLiteral( "cache" ), QStringLiteral( "AlwaysNetwork" ) );

      for ( auto it = myServer.params.constBegin(); it != myServer.params.constEnd(); it++ )
      {
        myServerUri.setParam( it.key(), it.value() );
      }

      QgsWcsCapabilities myCapabilities;
      myCapabilities.setUri( myServerUri );


      if ( !myCapabilities.lastError().isEmpty() )
      {
        QgsDebugMsg( myCapabilities.lastError() );
        myVersionLog << "error:" +  myCapabilities.lastError().replace( '\n', ' ' );
        continue;
      }

      myVersionLog << "getCapabilitiesUrl:" + myCapabilities.getCapabilitiesUrl();

      QVector<QgsWcsCoverageSummary> myCoverages;
      if ( !myCapabilities.supportedCoverages( myCoverages ) )
      {
        QgsDebugMsg( QStringLiteral( "Cannot get list of coverages" ) );
        myVersionLog << QStringLiteral( "error:Cannot get list of coverages" );
        continue;
      }

      myVersionLog << QStringLiteral( "totalCoverages:%1" ).arg( myCoverages.size() );

      int myCoverageCount = 0;
      int myStep = myCoverages.size() / std::min< int >( mMaxCoverages, myCoverages.size() );
      int myStepCount = -1;
      bool myCoverageFound = false;
      for ( QgsWcsCoverageSummary myCoverage : myCoverages )
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
        myUri.setParam( QStringLiteral( "identifier" ), myCoverage.identifier );
        if ( !myCoverage.times.isEmpty() )
        {
          myUri.setParam( QStringLiteral( "time" ), myCoverage.times.value( 0 ) );
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
        myLog << QStringLiteral( "hasSize:%1" ).arg( myCoverage.hasSize );

        // Test QGIS provider and via GDAL
        QStringList providers;
        providers << QStringLiteral( "wcs" ) << QStringLiteral( "gdal" );

        for ( const QString &provider : providers )
        {
          QElapsedTimer time;
          time.start();
          QString uri;
          if ( provider == QLatin1String( "wcs" ) )
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
            myStream << QStringLiteral( "  <Timeout>%1</Timeout>\n" ).arg( mTimeout / 1000., 0, 'd' );
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
              myLog << provider + "_srcType:" + qgsEnumValueToKey< Qgis::DataType >( myLayer->dataProvider()->sourceDataType( 1 ) );

              QgsRasterBandStats myStats = myLayer->dataProvider()->bandStatistics( 1, QgsRasterBandStats::All, QgsRectangle(), myWidth * myHeight );
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
            QgsDebugMsg( QStringLiteral( "%1 values" ).arg( myValues.size() ) );
            myLog << provider + QStringLiteral( "_valuesCount:%1" ).arg( myValues.size() );

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
            QgsDebugMsg( QStringLiteral( "%1 colors" ).arg( myColors.size() ) );
            myLog << provider + QStringLiteral( "_colorsCount:%1" ).arg( myColors.size() );
          }
          else
          {
            QgsDebugMsg( QStringLiteral( "Layer is not valid" ) );
            myLog << provider + "_error:Layer is not valid";
          }
          myLog << provider + QStringLiteral( "_time:%1" ).arg( time.elapsed() / 1000., 0, 'f', 2 );
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
      if ( !mCoverage.isEmpty() && ! myCoverageFound )
      {
        QgsDebugMsg( QStringLiteral( "Coverage not found" ) );
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

  for ( const QString &myServerDirName : mCacheDir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) )
  {
    myServerCount++;
    QDir myServerDir( mCacheDir.absolutePath() + '/' + myServerDirName );

    QString myServerLogPath = myServerDir.absolutePath() + "/server.log";
    QMap<QString, QString> myServerLog = readLog( myServerLogPath );

    myReport += QStringLiteral( "<h2>Server: %1</h2>" ).arg( myServerLog.value( QStringLiteral( "server" ) ) );
    Server myServer = getServer( myServerLog.value( QStringLiteral( "server" ) ) );
    if ( !myServer.description.isEmpty() )
    {
      myReport += myServer.description + "<br>\n";
    }
    if ( !myServer.params.isEmpty() )
    {
      myReport += QLatin1String( "<br>Additional params: " );
      for ( auto it = myServer.params.constBegin(); it != myServer.params.constEnd(); it++ )
      {
        myReport += it.key() + '=' + it.value() + " ";
      }
      myReport += QLatin1String( "<br>\n" );
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

      QString myVersion = myVersionLog.value( QStringLiteral( "version" ) );
      myServerReport += QStringLiteral( "<h3><a href='%1'>Version: %2</a></h3>" ).arg( myVersionLog.value( QStringLiteral( "getCapabilitiesUrl" ) ), myVersion.isEmpty() ? QStringLiteral( "(empty)" ) : myVersion );

      if ( !myVersionLog.value( QStringLiteral( "error" ) ).isEmpty() )
      {
        // Server may have more errors, for each version
        //for ( QString err : myServerLog.values( "error" ) )
        //{
        //myVersionReport += error( err );
        //}
        myVersionReport += error( myServerLog.value( QStringLiteral( "error" ) ) );
        myVersionErrCount++;
      }
      else
      {
        myVersionReport += QLatin1String( "<table class='tab'>" );
        myVersionReport += row( mHead );
        QStringList filters;
        filters << QStringLiteral( "*.log" );
        myVersionDir.setNameFilters( filters );
        for ( const QString &myLogFileName : myVersionDir.entryList( QDir::Files ) )
        {
          if ( myLogFileName == QLatin1String( "version.log" ) ) continue;
          myVersionCoverageCount++;
          myCoverageCount++;

          QString myLogPath = myVersionDir.absolutePath() + '/' + myLogFileName;
          QMap<QString, QString>myLog = readLog( myLogPath );
          myVersionReport += QLatin1String( "<tr>" );

          QStringList myValues;
          myValues << QStringLiteral( "<a href='%1'>%2</a>" ).arg( myLog.value( QStringLiteral( "describeCoverageUrl" ) ), myLog.value( QStringLiteral( "identifier" ) ) );
          //myValues << myLog.value( "hasSize" );
          myVersionReport += cells( myValues, QString(), 1, 2 );
          myValues.clear();

          QStringList issues = issueDescriptions( myServerLog.value( QStringLiteral( "server" ) ), myLog.value( QStringLiteral( "identifier" ) ), myLog.value( QStringLiteral( "version" ) ) );
          QString issuesString = issues.join( QLatin1String( "<br>" ) );

          QStringList providers;
          providers << QStringLiteral( "wcs" ) << QStringLiteral( "gdal" );

          bool hasErr = false;
          for ( const QString &provider : providers )
          {
            QString imgPath = myVersionDir.absolutePath() + '/' + QFileInfo( myLogPath ).completeBaseName() + "-" + provider + ".png";


            if ( !myLog.value( provider + "_error" ).isEmpty() )
            {
              myValues << myLog.value( provider + "_error" );
              int offender = NoOffender;
              if ( provider == QLatin1String( "wcs" ) )
              {
                myValues << issuesString;

                offender = issueOffender( myServerLog.value( QStringLiteral( "server" ) ), myLog.value( QStringLiteral( "identifier" ) ), myLog.value( QStringLiteral( "version" ) ) );
                myVersionErrCount++;
                hasErr = true;
              }
              QString cls;
              if ( offender == ServerOffender )
              {
                cls = QStringLiteral( "cell-err-server" );
              }
              else if ( offender == QgisOffender )
              {
                cls = QStringLiteral( "cell-err-qgis" );
              }
              else
              {
                cls = QStringLiteral( "cell-err" );
              }
              myVersionReport += cells( myValues, cls, 12 );
              myValues.clear();
            }
            else
            {
              myValues << myLog.value( provider + "_crs" );
              myValues << myLog.value( provider + "_width" );
              myValues << myLog.value( provider + "_height" );
              myValues << QString( myLog.value( provider + "_extent" ) ).replace( ',', QLatin1String( "<br>" ) );
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
                cls = QStringLiteral( "cell-err" );
                if ( provider == QLatin1String( "wcs" ) )
                {
                  myVersionErrCount++;
                  myCoverageErrCount++;
                }
              }
              else if ( myColorsCount < 4 )
              {
                cls = QStringLiteral( "cell-warn" );
                if ( provider == QLatin1String( "wcs" ) )
                {
                  myVersionWarnCount++;
                  myCoverageWarnCount++;
                }
              }
              myVersionReport += cells( myValues, cls );
              myValues.clear();
            }
          }
          myVersionReport += QLatin1String( "<tr>\n" );
          QString cls;
          if ( !issuesString.isEmpty() && !hasErr )
          {
            myValues << issuesString;
          }
          else
          {
            myValues << QString();
            cls = QStringLiteral( "cell-empty" );
          }
          myVersionReport += cells( myValues, cls, 24 );
          myValues.clear();
          myVersionReport += QLatin1String( "</tr>\n" );
        } // coverages
        myVersionReport += QLatin1String( "</table>\n" );
        // prepend counts
        myVersionReport.prepend( QStringLiteral( "<b>Total coverages: %1</b><br>\n" ).arg( myVersionLog.value( QStringLiteral( "totalCoverages" ) ) ) +
                                 QStringLiteral( "<b>Tested coverages: %1</b><br>\n" ).arg( myVersionCoverageCount ) +
                                 QStringLiteral( "<b>Errors: %1</b><br>\n" ).arg( myVersionErrCount ) +
                                 QStringLiteral( "<b>Warnings: %1</b><br><br>" ).arg( myVersionWarnCount ) );
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
  mySettings = mySettings.replace( '\n', QLatin1String( "<br />" ) );
  QString myRep = QStringLiteral( "<h1>WCS public servers test</h1>\n" );
  myRep += "<p>" + mySettings + "</p>";

  myRep += QLatin1String( "<style>" );
  myRep += QLatin1String( ".tab { border-spacing: 0px; border-width: 1px 1px 0 0; border-style: solid; }" );
  myRep += QLatin1String( ".cell { border-width: 0 0 1px 1px; border-style: solid; font-size: smaller; text-align: center}" );
  myRep += QLatin1String( ".cell-empty { border-width: 0; height:0; padding:0 }" );
  myRep += QLatin1String( ".cell-ok { background: #ffffff; }" );
  myRep += QLatin1String( ".cell-warn { background: #ffcc00; }" );
  myRep += QLatin1String( ".cell-err { background: #ff0000; }" );
  myRep += QLatin1String( ".cell-err-server { background: #ffff00; }" );
  myRep += QLatin1String( ".cell-err-qgis { background: #ff0000; }" );
  myRep += QLatin1String( ".errmsg { color: #ff0000; }" );
  myRep += QLatin1String( "</style>" );

  myRep += QStringLiteral( "<b>Servers: %1</b><br>\n" ).arg( myServerCount );
  myRep += QStringLiteral( "<b>Servers with error: %1</b><br>\n" ).arg( myServerErrCount );
  myRep += QStringLiteral( "<b>Servers with warning: %1</b><br>\n" ).arg( myServerWarnCount );
  myRep += QStringLiteral( "<b>Coverages: %1</b><br>\n" ).arg( myCoverageCount );
  myRep += QStringLiteral( "<b>Coverage errors: %1</b><br>\n" ).arg( myCoverageErrCount );
  myRep += QStringLiteral( "<b>Coverage warnings: %1</b><br>\n" ).arg( myCoverageWarnCount );

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
  QString myRow = QStringLiteral( "<font class='errmsg'>Error: " );
  myRow += message;
  myRow += QLatin1String( "</font>" );
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
      colspanStr = QStringLiteral( "colspan=%1" ).arg( colspan - values.size() + 1 );
    }
    if ( rowspan > 1 )
    {
      rowspanStr = QStringLiteral( "rowspan=%1" ).arg( rowspan );
    }
    myRow += QStringLiteral( "<td class='cell %1' %2 %3>%4</td>" ).arg( classStr, colspanStr, rowspanStr, val );
  }
  return myRow;
}

QString TestQgsWcsPublicServers::row( const QStringList &values, const QString &classStr )
{
  QString myRow = QStringLiteral( "<tr>" );
  for ( int i = 0; i < values.size(); i++ )
  {
    QString val = values.value( i );
    QString colspan;
    if ( values.size() < mHead.size() && i == ( values.size() - 1 ) )
    {
      colspan = QStringLiteral( "colspan=%1" ).arg( mHead.size() - values.size() + 1 );
    }
    myRow += QStringLiteral( "<td class='cell %1' %2>%3</td>" ).arg( classStr, colspan, val );
  }
  myRow += QLatin1String( "</tr>\n" );
  return myRow;
}

/* print usage text */
void usage( std::string const &appName )
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
#ifdef Q_OS_WIN // Windows
#ifdef _MSC_VER
  _set_fmode( _O_BINARY );
#else //MinGW
  _fmode = _O_BINARY;
#endif  // _MSC_VER
#endif  // Q_OS_WIN

  QString myServer;
  QString myCoverage;
  QString myVersion;
  int myMaxCoverages = 2;
  bool myForce = false;

#ifndef Q_OS_WIN
  int optionChar;
  static struct option long_options[] =
  {
    {"help",     no_argument,       nullptr, 'h'},
    {"server",   required_argument, nullptr, 's'},
    {"coverage", required_argument, nullptr, 'c'},
    {"num",      required_argument, nullptr, 'n'},
    {"version",  required_argument, nullptr, 'v'},
    {"force",    no_argument,       nullptr, 'f'},
    {nullptr, 0, nullptr, 0}
  };

  while ( true )
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
        return 2;   // XXX need standard exit codes

      default:
        QgsDebugMsg( QStringLiteral( "%1: getopt returned character code %2" ).arg( argv[0] ).arg( optionChar ) );
        return 1;   // XXX need standard exit codes
    }

  }

  QgsDebugMsg( QStringLiteral( "myServer = %1" ).arg( myServer ) );
  QgsDebugMsg( QStringLiteral( "myCoverage = %1" ).arg( myCoverage ) );
  QgsDebugMsg( QStringLiteral( "myMaxCoverages = %1" ).arg( myMaxCoverages ) );
  QgsDebugMsg( QStringLiteral( "myVersion = %1" ).arg( myVersion ) );

  if ( !myCoverage.isEmpty() && myServer.isEmpty() )
  {
    std::cerr << "--coverage can only be specified if --server is also used";
    return 1;
  }

  QgsDebugMsg( QStringLiteral( "optind = %1 argc = %2" ).arg( optind ).arg( argc ) );
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
