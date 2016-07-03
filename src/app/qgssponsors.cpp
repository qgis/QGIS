/***************************************************************************
                          qgssponsors.cpp  -  description
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

#include "qgssponsors.h"
#include "qgsapplication.h"
#include <QFile>
#include <QTextStream>

#ifdef Q_OS_MACX
QgsSponsors::QgsSponsors( QWidget *parent )
    : QDialog( parent, Qt::WindowSystemMenuHint )  // Modeless dialog with close button only
#else
QgsSponsors::QgsSponsors( QWidget *parent )
    : QDialog( parent )  // Normal dialog in non Mac-OS
#endif
{
  setupUi( this );
  init();
  QString intro = tr( "<p>We work really hard to make this nice software for you. "
                      "See all the cool features it has? Get a warm fuzzy feeling when you use it? "
                      "QGIS is a labour of love by a dedicated team of developers. We want you to copy "
                      "&amp; share it and put it in the hands of as many people as possible. If QGIS "
                      "is saving you money or you like our work and have the financial ability to "
                      "help, please consider sponsoring the development of QGIS. We use money from "
                      "sponsors to pay for travel and costs related to our bi-annual hackfests, and to "
                      "generally support the goals of our project.</p><p>Please see the <a "
                      "href='http://qgis.org/en/site/getinvolved/governance/sponsorship/sponsorship.html'>QGIS "
                      "Sponsorship Web Page</a> for more details. In the <a "
                      "href='http://qgis.org/en/site/about/sponsorship.html#list-of-sponsors'>Sponsors "
                      "page</a> you can see the fine people and companies that are helping us "
                      "financially - a great big 'thank you' to you all!</p>" );
  txtSponsors->setText( intro );
  // read the SPONSORS file and populate the text widget
  QFile sponsorsFile( QgsApplication::pkgDataPath() + QLatin1String( "/doc/release-sponsors.html" ) );
  if ( sponsorsFile.open( QIODevice::ReadOnly ) )
  {
    QString path = "images/";
    QString newPath = QgsApplication::pkgDataPath() + QLatin1String( "/doc/images/" );
    QTextStream sponsorsStream( &sponsorsFile );
    // Always use UTF-8
    sponsorsStream.setCodec( "UTF-8" );
    QString sponsors = sponsorsStream.readAll();
    sponsors.replace( path, newPath );
    txtSponsors->append( sponsors );

  }
}

QgsSponsors::~QgsSponsors()
{
}

void QgsSponsors::init()
{
  // set the 60x60 icon pixmap
  qgisIcon->setPixmap( QPixmap( QgsApplication::appIconPath() ) );
}
