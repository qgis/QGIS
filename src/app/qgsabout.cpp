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
/* $Id$ */

#include "qgsabout.h"
#include "qgsapplication.h"
#include "qgsproviderregistry.h"
#include "qgslogger.h"
#include <QDesktopServices>
#include <QFile>
#include <QTextStream>
#include <QImageReader>
#include <QSqlDatabase>

/* Uncomment this block to use preloaded images
#include <map>
#include "qgslogger.h"
std::map<QString, QPixmap> mugs;
*/
#ifdef Q_OS_MACX
QgsAbout::QgsAbout()
    : QDialog( NULL, Qt::WindowSystemMenuHint )  // Modeless dialog with close button only
#else
QgsAbout::QgsAbout()
    : QDialog( NULL )  // Normal dialog in non Mac-OS
#endif
{
  setupUi( this );
  init();
}

QgsAbout::~QgsAbout()
{
}

void QgsAbout::init()
{
  setPluginInfo();

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
      if ( line.left( 1 ) == "#" ) continue;
      QStringList myTokens = line.split( "\t", QString::SkipEmptyParts );
      lines += myTokens[0];
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
      if ( line.left( 1 ) == "#" ) continue;
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


  // read the SPONSORS file and populate the text widget
  QFile sponsorFile( QgsApplication::sponsorsFilePath() );
#ifdef QGISDEBUG
  printf( "Reading sponsors file %s.............................................\n",
          sponsorFile.fileName().toLocal8Bit().constData() );
#endif
  if ( sponsorFile.open( QIODevice::ReadOnly ) )
  {
    QString sponsorHTML = ""
                          + tr( "<p>QGIS sponsorship programme:"
                                "contribute to QGIS development</p>" )
                          + "<hr>"
                          "<table width='100%'>"
                          "<tr><th>" + tr( "Name" ) + "</th>"
                          "<th>" + tr( "Website" ) + "</th></tr>";
    QString website;
    QTextStream sponsorStream( &sponsorFile );
    // Always use UTF-8
    sponsorStream.setCodec( "UTF-8" );
    QString sline;
    while ( !sponsorStream.atEnd() )
    {
      sline = sponsorStream.readLine(); // line of text excluding '\n'
      //ignore the line if it starts with a hash....
      if ( sline.left( 1 ) == "#" ) continue;
      QStringList myTokens = sline.split( "|", QString::SkipEmptyParts );
      if ( myTokens.size() > 1 )
      {
        website = "<a href=\"" + myTokens[1].remove( ' ' ) + "\">" + myTokens[1] + "</a>";
      }
      else
      {
        website = "&nbsp;";
      }
      sponsorHTML += "<tr>";
      sponsorHTML += "<td>" + myTokens[0] + "</td><td>" + website + "</td>";
      // close the row
      sponsorHTML += "</tr>";
    }
    sponsorHTML += "</table>";

    QString myStyle = QgsApplication::reportStyleSheet();
    txtSponsors->clear();
    txtSponsors->document()->setDefaultStyleSheet( myStyle );
    txtSponsors->setHtml( sponsorHTML );
    QgsDebugMsg( QString( "sponsorHTML:%1" ).arg( sponsorHTML.toAscii().constData() ) );
    QgsDebugMsg( QString( "txtSponsors:%1" ).arg( txtSponsors->toHtml().toAscii().constData() ) );
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
                         + tr( "<p>The following individuals and institutions have contributed "
                               "money to fund QGIS development and other project costs</p>" )
                         + "<hr>"
                         "<table width='100%'>"
                         "<tr><th>" + tr( "Name" ) + "</th>"
                         "<th>" + tr( "Website" ) + "</th></tr>";
    QString website;
    QTextStream donorsStream( &donorsFile );
    // Always use UTF-8
    donorsStream.setCodec( "UTF-8" );
    QString sline;
    while ( !donorsStream.atEnd() )
    {
      sline = donorsStream.readLine(); // line of text excluding '\n'
      //ignore the line if it starts with a hash....
      if ( sline.left( 1 ) == "#" ) continue;
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

    QString myStyle = QgsApplication::reportStyleSheet();
    txtDonors->clear();
    txtDonors->document()->setDefaultStyleSheet( myStyle );
    txtDonors->setHtml( donorsHTML );
    QgsDebugMsg( QString( "donorsHTML:%1" ).arg( donorsHTML.toAscii().constData() ) );
    QgsDebugMsg( QString( "txtDonors:%1" ).arg( txtDonors->toHtml().toAscii().constData() ) );
  }

  // read the TRANSLATORS file and populate the text widget
  QFile translatorFile( QgsApplication::translatorsFilePath() );
#ifdef QGISDEBUG
  printf( "Reading translators file %s.............................................\n",
          translatorFile.fileName().toLocal8Bit().constData() );
#endif
  if ( translatorFile.open( QIODevice::ReadOnly ) )
  {
    QString translatorHTML = ""
                             + tr( "<p>The following have contributed to QGIS"
                                   " by translating the user interface or documentation</p>" )
                             + "<hr>"
                             "<table width='100%'>"
                             "<tr><th>" + tr( "Language" ) + "</th>"
                             "<th>" + tr( "Names" ) + "</th></tr>";
    QString website;
    QTextStream translatorStream( &translatorFile );
    // Always use UTF-8
    translatorStream.setCodec( "UTF-8" );
    QString sline;
    while ( !translatorStream.atEnd() )
    {
      sline = translatorStream.readLine(); // line of text excluding '\n'
      //ignore the line if it starts with a hash....
      if ( sline.left( 1 ) == "#" ) continue;
      QStringList myTokens = sline.split( "|", QString::SkipEmptyParts );
      if ( myTokens.size() > 1 )
      {
        website = myTokens[1];
      }
      else
      {
        website = "&nbsp;";
      }
      translatorHTML += "<tr>";
      translatorHTML += "<td>" + myTokens[0] + "</td><td>" + website + "</td>";
      // close the row
      translatorHTML += "</tr>";
    }
    translatorHTML += "</table>";

    QString myStyle = QgsApplication::reportStyleSheet();
    txtTranslators->clear();
    txtTranslators->document()->setDefaultStyleSheet( myStyle );
    txtTranslators->setHtml( translatorHTML );
    QgsDebugMsg( QString( "translatorHTML:%1" ).arg( translatorHTML.toAscii().constData() ) );
    QgsDebugMsg( QString( "txtTranslators:%1" ).arg( txtTranslators->toHtml().toAscii().constData() ) );
  }
}

void QgsAbout::setVersion( QString v )
{
  lblVersion->setText( v );
}

void QgsAbout::setWhatsNew( QString txt )
{
  QString myStyle = QgsApplication::reportStyleSheet();
  txtWhatsNew->clear();
  txtWhatsNew->document()->setDefaultStyleSheet( myStyle );
  txtWhatsNew->setHtml( txt );
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
  myString += tr( "Qt Image Plugin Search Paths	<br>" );
  myString += QApplication::libraryPaths().join("<br>");
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

void QgsAbout::on_buttonCancel_clicked()
{
  reject();
}

void QgsAbout::on_btnQgisUser_clicked()
{
  // find a browser
  QString url = "http://lists.osgeo.org/mailman/listinfo/qgis-user";
  openUrl( url );
}

void QgsAbout::on_btnQgisHome_clicked()
{
  openUrl( "http://qgis.org" );
}

void QgsAbout::openUrl( QString url )
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
