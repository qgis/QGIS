/***************************************************************************
    qgsencodingfiledialog.cpp - File dialog which queries the encoding type
     --------------------------------------
    Date                 : 16-Feb-2005
    Copyright            : (C) 2005 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsencodingfiledialog.h"
#include "qgsproject.h"
#include "qgslogger.h"
#include "qgsvectordataprovider.h"
#include "qgssettings.h"

#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QLayout>
#include <QTextCodec>
#include <QDialogButtonBox>

QgsEncodingFileDialog::QgsEncodingFileDialog( QWidget *parent,
    const QString &caption, const QString &directory,
    const QString &filter, const QString &encoding )
  : QFileDialog( parent, caption, directory, filter )
{
  mCancelAll       = false;
  mCancelAllButton = nullptr;
  mEncodingComboBox = new QComboBox( this );
  QLabel *l = new QLabel( tr( "Encoding:" ), this );

  setOption( QFileDialog::DontUseNativeDialog );
  layout()->addWidget( l );
  layout()->addWidget( mEncodingComboBox );

  mEncodingComboBox->addItems( QgsVectorDataProvider::availableEncodings() );

  // Use default encoding if none supplied
  QString enc = encoding;
  if ( encoding.isEmpty() )
  {
    const QgsSettings settings;
    enc = settings.value( QStringLiteral( "UI/encoding" ), "System" ).toString();
  }

  // The specified decoding is added if not existing already, and then set current.
  // This should select it.
  int encindex = mEncodingComboBox->findText( enc );
  if ( encindex < 0 )
  {
    mEncodingComboBox->insertItem( 0, enc );
    encindex = 0;
  }
  mEncodingComboBox->setCurrentIndex( encindex );

  // if this dialog is being invoked from QgisApp::findFiles_(), then we
  // need to force selection of the first filter since that corresponds to
  // the file name we're looking for; even if we're not here from
  // findFiles_(), it won't hurt to force selection of the first file filter
  selectNameFilter( nameFilters().at( 0 ) );

  // Connect our slot to get a signal when the user is done with the file dialog
  connect( this, &QDialog::accepted, this, &QgsEncodingFileDialog::saveUsedEncoding );
}

QString QgsEncodingFileDialog::encoding() const
{
  return mEncodingComboBox->currentText();
}

void QgsEncodingFileDialog::saveUsedEncoding()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "UI/encoding" ), encoding() );
  QgsDebugMsg( QStringLiteral( "Set encoding %1 as default." ).arg( encoding() ) );
}

void QgsEncodingFileDialog::addCancelAll()
{
  if ( ! mCancelAllButton )
  {
    mCancelAllButton = new QPushButton( tr( "Cancel &All" ), nullptr );
    layout()->addWidget( mCancelAllButton ); // Ownership transferred, no need to delete later on
    connect( mCancelAllButton, &QAbstractButton::clicked, this, &QgsEncodingFileDialog::pbnCancelAll_clicked );
  }
}

bool QgsEncodingFileDialog::cancelAll()
{
  return mCancelAll;
}

void QgsEncodingFileDialog::pbnCancelAll_clicked()
{
  mCancelAll = true;
  // Now, continue as the user clicked the cancel button
  reject();
}

QgsEncodingSelectionDialog::QgsEncodingSelectionDialog( QWidget *parent, const QString &caption, const QString &encoding, Qt::WindowFlags flags )
  : QDialog( parent, flags )
{
  QString c = caption;
  if ( c.isEmpty() )
    c = tr( "Encoding" );

  setWindowTitle( tr( "Select Encoding" ) );

  QVBoxLayout *layout = new QVBoxLayout();
  layout->setContentsMargins( 6, 6, 6, 6 );

  mEncodingComboBox = new QComboBox( this );
  QLabel *l = new QLabel( c, this );

  QHBoxLayout *hLayout = new QHBoxLayout();
  hLayout->addWidget( l );
  hLayout->addWidget( mEncodingComboBox, 1 );
  layout->addLayout( hLayout );

  QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
      Qt::Horizontal, this );
  buttonBox->button( QDialogButtonBox::Ok )->setDefault( true );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  layout->addWidget( buttonBox );
  setLayout( layout );

  mEncodingComboBox->addItem( tr( "System" ) );
  mEncodingComboBox->addItems( QgsVectorDataProvider::availableEncodings() );

  // Use default encoding if none supplied
  QString enc = encoding;
  if ( encoding.isEmpty() )
  {
    const QgsSettings settings;
    enc = settings.value( QStringLiteral( "UI/encoding" ), "System" ).toString();
  }

  setEncoding( enc );
}

QString QgsEncodingSelectionDialog::encoding() const
{
  return mEncodingComboBox->currentText();
}

void QgsEncodingSelectionDialog::setEncoding( const QString &encoding )
{
  // The specified decoding is added if not existing already, and then set current.
  // This should select it.

  int encindex = mEncodingComboBox->findText( encoding );
  if ( encindex < 0 )
  {
    mEncodingComboBox->insertItem( 0, encoding );
    encindex = 0;
  }
  mEncodingComboBox->setCurrentIndex( encindex );
}
