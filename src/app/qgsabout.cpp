/***************************************************************************
                          qgsabout.cpp  -  description
                             -------------------
    begin                : Sat Aug 10 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsabout.h"
#include "qgsapplication.h"
#include "qgsauthmethodregistry.h"
#include "qgsproviderregistry.h"
#include "qgslogger.h"
#include <QDesktopServices>
#include <QFile>
#include <QTextStream>
#include <QImageReader>
#include <QSqlDatabase>
#include <QTcpSocket>
#include <QUrl>
#include <QRegularExpression>

#ifdef Q_OS_MACX
QgsAbout::QgsAbout( QWidget *parent )
  : QgsOptionsDialogBase( "about", parent, Qt::WindowSystemMenuHint )  // Modeless dialog with close button only
#else
QgsAbout::QgsAbout( QWidget * parent )
  : QgsOptionsDialogBase( QStringLiteral( "about" ), parent )  // Normal dialog in non Mac-OS
#endif
{
  setupUi( this );
  connect( btnQgisUser, &QPushButton::clicked, this, &QgsAbout::btnQgisUser_clicked );
  connect( btnQgisHome, &QPushButton::clicked, this, &QgsAbout::btnQgisHome_clicked );
  initOptionsBase( true, QStringLiteral( "%1 - %2 Bit" ).arg( windowTitle() ).arg( QSysInfo::WordSize ) );
  init();
}

void QgsAbout::init()
{
  setPluginInfo();

  // check internet connection in order to hide/show the developers map widget
  const int DEVELOPERS_MAP_INDEX = 5;
  QTcpSocket socket;
  socket.connectToHost( QgsApplication::QGIS_ORGANIZATION_DOMAIN, 80 );
  if ( socket.waitForConnected( 1000 ) )
  {
    setDevelopersMap();
  }
  else
  {
    mOptionsListWidget->item( DEVELOPERS_MAP_INDEX )->setHidden( true );
    const QModelIndex firstItem = mOptionsListWidget->model()->index( 0, 0, QModelIndex() );
    mOptionsListWidget->setCurrentIndex( firstItem );
  }
  developersMapView->page()->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );
  developersMapView->setContextMenuPolicy( Qt::NoContextMenu );

  connect( developersMapView, &QgsWebView::linkClicked, this, &QgsAbout::openUrl );

  //read the authors file to populate the svn committers list
  QStringList lines;

  //
  // Load the authors (svn committers) list
  //
  QFile file( QgsApplication::authorsFilePath() );
  if ( file.open( QIODevice::ReadOnly ) )
  {
    QTextStream stream( &file );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Always use UTF-8
    stream.setCodec( "UTF-8" );
#endif
    QString line;
    while ( !stream.atEnd() )
    {
      line = stream.readLine(); // line of text excluding '\n'
      //ignore the line if it starts with a hash....
      if ( !line.isEmpty() && line.at( 0 ) == '#' )
        continue;
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
      QStringList myTokens = line.split( '\t', QString::SkipEmptyParts );
#else
      QStringList myTokens = line.split( '\t', Qt::SkipEmptyParts );
#endif
      lines << myTokens[0];
    }
    file.close();
    lstDevelopers->clear();
    lstDevelopers->insertItems( 0, lines );

    if ( lstDevelopers->count() > 0 )
    {
      lstDevelopers->setCurrentRow( 0 );
    }
  }

  lines.clear();
  //
  // Now load up the contributors list
  //
  QFile file2( QgsApplication::contributorsFilePath() );
  if ( file2.open( QIODevice::ReadOnly ) )
  {
    QTextStream stream( &file2 );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Always use UTF-8
    stream.setCodec( "UTF-8" );
#endif
    QString line;
    while ( !stream.atEnd() )
    {
      line = stream.readLine(); // line of text excluding '\n'
      //ignore the line if it starts with a hash....
      if ( !line.isEmpty() && line.at( 0 ) == '#' )
        continue;
      lines += line;
    }
    file2.close();
    lstContributors->clear();
    lstContributors->insertItems( 0, lines );
    if ( lstContributors->count() > 0 )
    {
      lstContributors->setCurrentRow( 0 );
    }
  }

  // read the DONORS file and populate the text widget
  QFile donorsFile( QgsApplication::donorsFilePath() );
  if ( donorsFile.open( QIODevice::ReadOnly ) )
  {
    const QString donorsHTML = tr( "<p>For a list of individuals and institutions who have contributed "
                                   "money to fund QGIS development and other project costs see "
                                   "<a href=\"https://qgis.org/en/site/about/sustaining_members.html#list-of-donors\">"
                                   "https://qgis.org/en/site/about/sustaining_members.html#list-of-donors</a></p>" );
#if 0
    QString website;
    QTextStream donorsStream( &donorsFile );
    // Always use UTF-8
    donorsStream.setCodec( "UTF-8" );
    QString sline;
    while ( !donorsStream.atEnd() )
    {
      sline = donorsStream.readLine(); // line of text excluding '\n'
      //ignore the line if it starts with a hash....
      if ( sline.left( 1 ) == "#" )
        continue;
      QStringList myTokens = sline.split( '|', QString::SkipEmptyParts );
      if ( myTokens.size() > 1 )
      {
        website = "<a href=\"" + myTokens[1].remove( ' ' ) + "\">" + myTokens[1] + "</a>";
      }
      else
      {
        website = "&nbsp;";
      }
      donorsHTML += "<tr>";
      donorsHTML += "<td>" + myTokens[0] + "</td><td>" + website + "</td>";
      // close the row
      donorsHTML += "</tr>";
    }
    donorsHTML += "</table>";
#endif

    txtDonors->clear();
    txtDonors->document()->setDefaultStyleSheet( QgsApplication::reportStyleSheet() );
    txtDonors->setHtml( donorsHTML );
    QgsDebugMsg( QStringLiteral( "donorsHTML:%1" ).arg( donorsHTML.toLatin1().constData() ) );
  }

  // read the TRANSLATORS file and populate the text widget
  QFile translatorFile( QgsApplication::translatorsFilePath() );
  if ( translatorFile.open( QIODevice::ReadOnly ) )
  {
    QString translatorHTML;
    QTextStream translatorStream( &translatorFile );
    // Always use UTF-8
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    translatorStream.setCodec( "UTF-8" );
#endif
    const QString myStyle = QgsApplication::reportStyleSheet();
    translatorHTML += "<style>" + myStyle + "</style>";
    while ( !translatorStream.atEnd() )
    {
      translatorHTML += translatorStream.readLine();
    }
    txtTranslators->setHtml( translatorHTML );
    QgsDebugMsg( QStringLiteral( "translatorHTML:%1" ).arg( translatorHTML.toLatin1().constData() ) );
  }
  setWhatsNew();
  setLicence();
}

void QgsAbout::setLicence()
{
  // read the DONORS file and populate the text widget
  QFile licenceFile( QgsApplication::licenceFilePath() );
#ifdef QGISDEBUG
  printf( "Reading licence file %s.............................................\n",
          licenceFile.fileName().toLocal8Bit().constData() );
#endif
  if ( licenceFile.open( QIODevice::ReadOnly ) )
  {
    txtLicense->setText( licenceFile.readAll() );
  }
}

void QgsAbout::setVersion( const QString &v )
{
  txtVersion->setBackgroundRole( QPalette::NoRole );
  txtVersion->setAutoFillBackground( true );
  txtVersion->setHtml( v );
}

void QgsAbout::setWhatsNew()
{
  txtWhatsNew->clear();
  txtWhatsNew->document()->setDefaultStyleSheet( QgsApplication::reportStyleSheet() );
  if ( !QFile::exists( QgsApplication::pkgDataPath() + "/doc/NEWS.html" ) )
    return;

  txtWhatsNew->setSource( QString( "file:///" + QgsApplication::pkgDataPath() + "/doc/NEWS.html" ) );
}

void QgsAbout::setPluginInfo()
{
  QString myString;
  //provide info about the plugins available
  myString += "<b>" + tr( "Available QGIS Data Provider Plugins" ) + "</b><br>";
  myString += QgsProviderRegistry::instance()->pluginList( true );
  myString += "<b>" + tr( "Available QGIS Authentication Method Plugins" ) + "</b><br>";
  myString += QgsAuthMethodRegistry::instance()->pluginList( true );
  //qt database plugins
  myString += "<b>" + tr( "Available Qt Database Plugins" ) + "</b><br>";
  myString += QLatin1String( "<ol>\n<li>\n" );
  const QStringList myDbDriverList = QSqlDatabase::drivers();
  myString += myDbDriverList.join( QLatin1String( "</li>\n<li>" ) );
  myString += QLatin1String( "</li>\n</ol>\n" );
  //qt image plugins
  myString += "<b>" + tr( "Available Qt Image Plugins" ) + "</b><br>";
  myString += tr( "Qt Image Plugin Search Paths <br>" );
  myString += QApplication::libraryPaths().join( QLatin1String( "<br>" ) );
  myString += QLatin1String( "<ol>\n<li>\n" );
  const QList<QByteArray> myImageFormats = QImageReader::supportedImageFormats();
  QList<QByteArray>::const_iterator myIterator = myImageFormats.constBegin();
  while ( myIterator != myImageFormats.constEnd() )
  {
    const QString myFormat = ( *myIterator ).data();
    myString += myFormat + "</li>\n<li>";
    ++myIterator;
  }
  myString += QLatin1String( "</li>\n</ol>\n" );

  const QString myStyle = QgsApplication::reportStyleSheet();
  txtProviders->clear();
  txtProviders->document()->setDefaultStyleSheet( myStyle );
  txtProviders->setText( myString );
}

void QgsAbout::btnQgisUser_clicked()
{
  openUrl( QStringLiteral( "https://lists.osgeo.org/mailman/listinfo/qgis-user" ) );
}

void QgsAbout::btnQgisHome_clicked()
{
  openUrl( QStringLiteral( "https://qgis.org" ) );
}

void QgsAbout::openUrl( const QUrl &url )
{
  //use the users default browser
  QDesktopServices::openUrl( url );
}

/*
 * The function below makes a name safe for using in most file system
 * Step 1: Code QString as UTF-8
 * Step 2: Replace all bytes of the UTF-8 above 0x7f with the hexcode in lower case.
 * Step 2: Replace all non [a-z][a-Z][0-9] with underscore (backward compatibility)
 */
QString QgsAbout::fileSystemSafe( const QString &fileName )
{
  QString result;
  QByteArray utf8 = fileName.toUtf8();

  for ( int i = 0; i < utf8.size(); i++ )
  {
    const uchar c = utf8[i];

    if ( c > 0x7f )
    {
      result = result + QStringLiteral( "%1" ).arg( c, 2, 16, QChar( '0' ) );
    }
    else
    {
      result = result + QChar( c );
    }
  }

  const thread_local QRegularExpression sNonAlphaNumericRx( QStringLiteral( "[^a-zA-Z0-9]" ) );
  result.replace( sNonAlphaNumericRx, QStringLiteral( "_" ) );
  QgsDebugMsgLevel( result, 3 );

  return result;
}

void QgsAbout::setDevelopersMap()
{
  developersMapView->settings()->setAttribute( QWebSettings::JavascriptEnabled, true );
  const QUrl url = QUrl::fromLocalFile( QgsApplication::developersMapFilePath() );
  developersMapView->load( url );
}
