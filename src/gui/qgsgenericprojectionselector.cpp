/***************************************************************************
                          qgsgenericprojectionselector.cpp
                    Set user defined CRS using projection selector widget
                             -------------------
    begin                : May 28, 2004
    copyright            : (C) 2004 by Gary E.Sherman
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
#include "qgsapplication.h"

#include <qgsgenericprojectionselector.h>
#include <QApplication>
#include <QSettings>

/**
 * \class QgsGenericProjectionSelector
 * \brief A generic dialog to prompt the user for a Coordinate Reference System
 */
QgsGenericProjectionSelector::QgsGenericProjectionSelector( QWidget *parent,
    Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/ProjectionSelector/geometry" ).toByteArray() );

  //we will show this only when a message is set
  textEdit->hide();
}

void QgsGenericProjectionSelector::setMessage( QString theMessage )
{
  //short term kludge to make the layer selector default to showing
  //a layer projection selection message. If you want the selector
  if ( theMessage.isEmpty() )
  {
    // Set up text edit pane
    QString format( "<h1>%1</h1>%2 %3" );
    QString header = tr( "Define this layer's coordinate reference system:" );
    QString sentence1 = tr( "This layer appears to have no projection specification." );
    QString sentence2 = tr( "By default, this layer will now have its projection set to that of the project, "
                            "but you may override this by selecting a different projection below." );
    theMessage = format.arg( header ).arg( sentence1 ).arg( sentence2 );
  }

  QString myStyle = QgsApplication::reportStyleSheet();
  theMessage = "<head><style>" + myStyle + "</style></head><body>" + theMessage + "</body>";
  textEdit->setHtml( theMessage );
  textEdit->show();
}
//! Destructor
QgsGenericProjectionSelector::~QgsGenericProjectionSelector()
{
  QSettings settings;
  settings.setValue( "/Windows/ProjectionSelector/geometry", saveGeometry() );
}

void QgsGenericProjectionSelector::setSelectedEpsg( long theId )
{
  projectionSelector->setSelectedAuthId( QString( "EPSG:%1" ).arg( theId ) );
}

void QgsGenericProjectionSelector::setSelectedCrsName( QString theName )
{
  projectionSelector->setSelectedCrsName( theName );
}

void QgsGenericProjectionSelector::setSelectedCrsId( long theID )
{
  projectionSelector->setSelectedCrsId( theID );
}

void QgsGenericProjectionSelector::setSelectedAuthId( QString theID )
{
  projectionSelector->setSelectedAuthId( theID );
}

QString QgsGenericProjectionSelector::selectedProj4String()
{
  //@note don't use getSelectedWkt as that just returns the name part!
  return projectionSelector->selectedProj4String();
}

long QgsGenericProjectionSelector::selectedCrsId()
{
  //@note don't use getSelectedWkt as that just returns the name part!
  return projectionSelector->selectedCrsId();
}

long QgsGenericProjectionSelector::selectedEpsg()
{
  QString authid = projectionSelector->selectedAuthId();
  if ( authid.startsWith( "EPSG:", Qt::CaseInsensitive ) )
    return authid.mid( 5 ).toLong();
  else
    return 0;
}

QString QgsGenericProjectionSelector::selectedAuthId()
{
  return projectionSelector->selectedAuthId();
}

void QgsGenericProjectionSelector::setOgcWmsCrsFilter( QSet<QString> crsFilter )
{
  projectionSelector->setOgcWmsCrsFilter( crsFilter );
}
