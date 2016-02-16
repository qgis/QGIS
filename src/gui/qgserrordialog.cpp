/***************************************************************************
                         qgserrordialog.cpp  -  error description
                             -------------------
    begin                : October 2012
    copyright            : (C) October 2012 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgserrordialog.h"

#include <QMessageBox>
#include <QSettings>

QgsErrorDialog::QgsErrorDialog( const QgsError & theError, const QString & theTitle, QWidget *parent, const Qt::WindowFlags& fl )
    : QDialog( parent, fl )
    , mError( theError )
{
  setupUi( this );
  QString title = theTitle;
  if ( title.isEmpty() ) title = tr( "Error" );
  setWindowTitle( title );

  // QMessageBox has static standardIcon( Icon icon ), but it is marked as obsolete
  QMessageBox messageBox( QMessageBox::Critical, "", "" );
  mIconLabel->setPixmap( messageBox.iconPixmap() );
  mSummaryTextBrowser->setOpenExternalLinks( true );
  mDetailTextBrowser->setOpenExternalLinks( true );
  mDetailTextBrowser->hide();

  QPalette p = palette();
  p.setColor( QPalette::Base, Qt::transparent );
  mSummaryTextBrowser->setPalette( p );

  mDetailCheckBox->hide();

  mSummaryTextBrowser->setText( mError.summary() );
  mDetailTextBrowser->setText( mError.message( QgsErrorMessage::Html ) );

  resize( width(), 150 );

  QSettings settings;
  Qt::CheckState state = ( Qt::CheckState ) settings.value( "/Error/dialog/detail", 0 ).toInt();
  mDetailCheckBox->setCheckState( state );
  if ( state == Qt::Checked ) on_mDetailPushButton_clicked();
}

QgsErrorDialog::~QgsErrorDialog()
{
}

void QgsErrorDialog::show( const QgsError & theError, const QString & theTitle, QWidget *parent, const Qt::WindowFlags& fl )
{
  QgsErrorDialog d( theError, theTitle, parent, fl );
  d.exec();
}

void QgsErrorDialog::on_mDetailPushButton_clicked()
{
  mSummaryTextBrowser->hide();
  mDetailTextBrowser->show();
  mDetailCheckBox->show();
  mDetailPushButton->hide();
  resize( width(), 400 );
}

void QgsErrorDialog::on_mDetailCheckBox_stateChanged( int state )
{
  QSettings settings;
  settings.setValue( "/Error/dialog/detail", state );
}

