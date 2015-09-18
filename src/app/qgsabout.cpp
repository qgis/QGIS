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
#include "qgsproviderregistry.h"
#include "qgslogger.h"
#include <QDesktopServices>
#include <QFile>
#include <QTextStream>
#include <QImageReader>
#include <QSqlDatabase>
#include <QTcpSocket>

/* Uncomment this block to use preloaded images
#include <map>
std::map<QString, QPixmap> mugs;
*/
#ifdef Q_OS_MACX
QgsAbout::QgsAbout( QWidget *parent )
    : QgsOptionsDialogBase( "about", parent, Qt::WindowSystemMenuHint )  // Modeless dialog with close button only
#else
QgsAbout::QgsAbout( QWidget *parent )
    : QgsOptionsDialogBase( "about", parent )  // Normal dialog in non Mac-OS
#endif
{
  setupUi( this );
  initOptionsBase( true, QString( "%1 - %2 Bit" ).arg( windowTitle() ).arg( QSysInfo::WordSize ) );
  init();
}

QgsAbout::~QgsAbout()
{
}

void QgsAbout::init()
{
  setPluginInfo();

  // check internet connection in order to hide/show the developers map widget
  int DEVELOPERS_MAP_INDEX = 5;
  QTcpSocket socket;
  socket.connectToHost( QgsApplication::QGIS_ORGANIZATION_DOMAIN, 80 );
  if ( socket.waitForConnected( 1000 ) )
  {
    setDevelopersMap();
  }
  else
  {
    mOptionsListWidget->item( DEVELOPERS_MAP_INDEX )->setHidden( true );
    QModelIndex firstItem = mOptionsListWidget->model()->index( 0, 0, QModelIndex() );
    mOptionsListWidget->setCurrentIndex( firstItem );
  }
  developersMapView->page()->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );
  developersMapView->setContextMenuPolicy( Qt::NoContextMenu );

  connect( developersMapView, SIGNAL( linkClicked( const QUrl & ) ), this, SLOT( openUrl( const QUrl & ) ) );

  // set the 60x60 icon pixmap
  QPixmap icon( QgsApplication::iconsPath() + "qgis-icon-60x60.png" );
  qgisIcon->setPixmap( icon );

  //read the authors file to populate the svn committers list
  QStringList lines;

  //
  // Load the authors (svn committers) list
  //
  QFile file( QgsApplication::authorsFilePath() );
  if ( file.open( QIODevice::ReadOnly ) )
  {
    QTextStream stream( &file );
    // Always use UTF-8
    stream.setCodec( "UTF-8" );
    QString line;
    while ( !stream.atEnd() )
    {
      line = stream.readLine(); // line of text excluding '\n'
      //ignore the line if it starts with a hash....
      if ( line.left( 1 ) == "#" )
        continue;
      QStringList myTokens = line.split( "\t", QString::SkipEmptyParts );
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
  printf( "Reading contributors file %s.............................................\n",
          file2.fileName().toLocal8Bit().constData() );
  if ( file2.open( QIODevice::ReadOnly ) )
  {
    QTextStream stream( &file2 );
    // Always use UTF-8
    stream.setCodec( "UTF-8" );
    QString line;
    while ( !stream.atEnd() )
    {
      line = stream.readLine(); // line of text excluding '\n'
      //ignore the line if it starts with a hash....
      if ( line.left( 1 ) == "#" )
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
#ifdef QGISDEBUG
  printf( "Reading donors file %s.............................................\n",
          donorsFile.fileName().toLocal8Bit().constData() );
#endif
  if ( donorsFile.open( QIODevice::ReadOnly ) )
  {
    QString donorsHTML = ""
                         + tr( "<p>For a list of individuals and institutions who have contributed "
                               "money to fund QGIS development and other project costs see "
                               "<a href=\"http://qgis.org/en/site/about/sponsorship.html#list-of-donors\">"
                               "http://qgis.org/en/site/about/sponsorship.html#list-of-donors</a></p>" );
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
      QStringList myTokens = sline.split( "|", QString::SkipEmptyParts );
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
    QgsDebugMsg( QString( "donorsHTML:%1" ).arg( donorsHTML.toAscii().constData() ) );
  }

  // read the TRANSLATORS file and populate the text widget
  QFile translatorFile( QgsApplication::translatorsFilePath() );
#ifdef QGISDEBUG
  printf( "Reading translators file %s.............................................\n",
          translatorFile.fileName().toLocal8Bit().constData() );
#endif
  if ( translatorFile.open( QIODevice::ReadOnly ) )
  {
    QString translatorHTML = "";
    QTextStream translatorStream( &translatorFile );
    // Always use UTF-8
    translatorStream.setCodec( "UTF-8" );
    QString myStyle = QgsApplication::reportStyleSheet();
    translatorHTML += "<style>" + myStyle + "</style>";
    while ( !translatorStream.atEnd() )
    {
      translatorHTML += translatorStream.readLine();
    }
    txtTranslators->setHtml( translatorHTML );
    QgsDebugMsg( QString( "translatorHTML:%1" ).arg( translatorHTML.toAscii().constData() ) );
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

void QgsAbout::setVersion( QString v )
{
  txtVersion->setBackgroundRole( QPalette::NoRole );
  txtVersion->setAutoFillBackground( true );
  txtVersion->setHtml( v );
}

void QgsAbout::setWhatsNew()
{
  txtWhatsNew->clear();
  txtWhatsNew->document()->setDefaultStyleSheet( QgsApplication::reportStyleSheet() );
  txtWhatsNew->setSource( "file:///" + QgsApplication::pkgDataPath() + "/doc/news.html" );
}

void QgsAbout::setPluginInfo()
{
  QString myString;
  //provide info about the plugins available
  myString += "<b>" + tr( "Available QGIS Data Provider Plugins" ) + "</b><br>";
  myString += QgsProviderRegistry::instance()->pluginList( true );
  //qt database plugins
  myString += "<b>" + tr( "Available Qt Database Plugins" ) + "</b><br>";
  myString += "<ol>\n<li>\n";
  QStringList myDbDriverList = QSqlDatabase::drivers();
  myString += myDbDriverList.join( "</li>\n<li>" );
  myString += "</li>\n</ol>\n";
  //qt image plugins
  myString += "<b>" + tr( "Available Qt Image Plugins" ) + "</b><br>";
  myString += tr( "Qt Image Plugin Search Paths <br>" );
  myString += QApplication::libraryPaths().join( "<br>" );
  myString += "<ol>\n<li>\n";
  QList<QByteArray> myImageFormats = QImageReader::supportedImageFormats();
  QList<QByteArray>::const_iterator myIterator = myImageFormats.begin();
  while ( myIterator != myImageFormats.end() )
  {
    QString myFormat = ( *myIterator ).data();
    myString += myFormat + "</li>\n<li>";
    ++myIterator;
  }
  myString += "</li>\n</ol>\n";

  QString myStyle = QgsApplication::reportStyleSheet();
  txtProviders->clear();
  txtProviders->document()->setDefaultStyleSheet( myStyle );
  txtProviders->setText( myString );
}

void QgsAbout::on_btnQgisUser_clicked()
{
  openUrl( QString( "http://lists.osgeo.org/mailman/listinfo/qgis-user" ) );
}

void QgsAbout::on_btnQgisHome_clicked()
{
  openUrl( QString( "http://qgis.org" ) );
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
QString QgsAbout::fileSystemSafe( QString fileName )
{
  QString result;
  QByteArray utf8 = fileName.toUtf8();

  for ( int i = 0; i < utf8.size(); i++ )
  {
    uchar c = utf8[i];

    if ( c > 0x7f )
    {
      result = result + QString( "%1" ).arg( c, 2, 16, QChar( '0' ) );
    }
    else
    {
      result = result + QString( c );
    }
  }
  result.replace( QRegExp( "[^a-z0-9A-Z]" ), "_" );
  QgsDebugMsg( result );

  return result;
}

void QgsAbout::setDevelopersMap()
{
  developersMapView->settings()->setAttribute( QWebSettings::JavascriptEnabled, true );
  QUrl url = QUrl::fromLocalFile( QgsApplication::developersMapFilePath() );
  developersMapView->load( url );
}
