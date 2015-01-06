/***************************************************************************
    qgsprojectionselectionwidget.cpp
     --------------------------------------
    Date                 : 05.01.2015
    Copyright            : (C) 2015 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QHBoxLayout>


#include "qgsprojectionselectionwidget.h"
#include "qgsapplication.h"
#include "qgsgenericprojectionselector.h"

QgsProjectionSelectionWidget::QgsProjectionSelectionWidget( QWidget *parent ) :
    QWidget( parent )
{
  mDialog = new QgsGenericProjectionSelector( this );

  QHBoxLayout* layout = new QHBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 0 );
  setLayout( layout );

  mCrsLineEdit = new QLineEdit( tr( "invalid projection" ), this );
  mCrsLineEdit->setReadOnly( true );
  layout->addWidget( mCrsLineEdit );

  mButton = new QToolButton( this );
  mButton->setIcon( QgsApplication::getThemeIcon( "mActionSetProjection.svg" ) );
  mButton->setToolTip( tr( "Select CRS" ) );
  layout->addWidget( mButton );

  setFocusPolicy( Qt::StrongFocus );
  setFocusProxy( mButton );

  connect( mButton, SIGNAL( clicked() ), this, SLOT( selectCrs() ) );
}

void QgsProjectionSelectionWidget::selectCrs()
{
  //find out crs id of current proj4 string
  if ( mCrs.isValid() )
  {
    mDialog->setSelectedCrsId( mCrs.srsid() );
  }

  if ( mDialog->exec() )
  {
    QgsCoordinateReferenceSystem crs;
    crs.createFromOgcWmsCrs( mDialog->selectedAuthId() );
    setCrs( crs );
    emit crsChanged( crs );
  }
  else
  {
    QApplication::restoreOverrideCursor();
  }
}


void QgsProjectionSelectionWidget::setCrs( const QgsCoordinateReferenceSystem& crs )
{
  if ( crs.isValid() )
  {
    mCrsLineEdit->setText( crs.authid() + " - " + crs.description() );
  }
  else
  {
    mCrsLineEdit->setText( tr( "invalid projection" ) );
  }
  mCrs = crs;
}
