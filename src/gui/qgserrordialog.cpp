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
#include "qgssettings.h"

#include <QMessageBox>

QgsErrorDialog::QgsErrorDialog( const QgsError &error, const QString &title, QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mError( error )
{
  setupUi( this );
  connect( mDetailPushButton, &QPushButton::clicked, this, &QgsErrorDialog::mDetailPushButton_clicked );
  connect( mDetailCheckBox, &QCheckBox::stateChanged, this, &QgsErrorDialog::mDetailCheckBox_stateChanged );

  if ( title.isEmpty() )
    setWindowTitle( tr( "Error" ) );
  else
    setWindowTitle( title );

  // QMessageBox has static standardIcon( Icon icon ), but it is marked as obsolete
  const QMessageBox messageBox( QMessageBox::Critical, QString(), QString() );
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

  const QgsSettings settings;
  const Qt::CheckState state = ( Qt::CheckState ) settings.value( QStringLiteral( "Error/dialog/detail" ), 0 ).toInt();
  mDetailCheckBox->setCheckState( state );
  if ( state == Qt::Checked )
    mDetailPushButton_clicked();
}

void QgsErrorDialog::show( const QgsError &error, const QString &title, QWidget *parent, Qt::WindowFlags fl )
{
  QgsErrorDialog d( error, title, parent, fl );
  d.exec();
}

void QgsErrorDialog::mDetailPushButton_clicked()
{
  mSummaryTextBrowser->hide();
  mDetailTextBrowser->show();
  mDetailCheckBox->show();
  mDetailPushButton->hide();
  resize( width(), 400 );
}

void QgsErrorDialog::mDetailCheckBox_stateChanged( int state )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Error/dialog/detail" ), state );
}

