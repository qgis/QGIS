/***************************************************************************
    progress_dialog.h

    Offline Editing Plugin
    a QGIS plugin
     --------------------------------------
    Date                 : 08-Jul-2010
    Copyright            : (C) 2010 by Sourcepole
    Email                : info at sourcepole.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "offline_editing_progress_dialog.h"

QgsOfflineEditingProgressDialog::QgsOfflineEditingProgressDialog( QWidget* parent /*= 0*/, Qt::WFlags fl /*= 0*/ )
    : QDialog( parent, fl )
{
  setupUi( this );
}

QgsOfflineEditingProgressDialog::~QgsOfflineEditingProgressDialog()
{
}

void QgsOfflineEditingProgressDialog::setTitle( const QString& title )
{
  setWindowTitle( title );
}

void QgsOfflineEditingProgressDialog::setCurrentLayer( int layer, int numLayers )
{
  label->setText( tr( "Layer %1 of %2.." ).arg( layer ).arg( numLayers ) );
  progressBar->reset();
}

void QgsOfflineEditingProgressDialog::setupProgressBar( const QString& format, int maximum )
{
  progressBar->setFormat( format );
  progressBar->setRange( 0, maximum );
  progressBar->reset();

  mProgressUpdate = maximum / 100;
  if ( mProgressUpdate == 0 )
  {
    mProgressUpdate = 1;
  }
}

void QgsOfflineEditingProgressDialog::setProgressValue( int value )
{
  // update progress every nth feature for faster processing
  if ( value == progressBar->maximum() || value % mProgressUpdate == 0 )
  {
    progressBar->setValue( value );
  }
}
