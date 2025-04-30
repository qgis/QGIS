/***************************************************************************
    qgsdatabaseschemaselectiondialog.cpp
    ---------------------
    begin                : March 2025
    copyright            : (C) 2025 by Jan Caha
    email                : jan.caha at outlook dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QApplication>
#include <QMessageBox>
#include <QGridLayout>
#include <QLabel>

#include "qgsdatabaseschemaselectiondialog.h"
#include "moc_qgsdatabaseschemaselectiondialog.cpp"

QgsDatabaseSchemaSelectionDialog::QgsDatabaseSchemaSelectionDialog( QgsAbstractDatabaseProviderConnection *connection, QWidget *parent )
  : QDialog( parent )
{
  setWindowTitle( tr( "Select Schema" ) );

  QGridLayout *layout = new QGridLayout( this );

  mCboSchema = new QgsDatabaseSchemaComboBox( connection, this );
  mCboSchema->setAllowEmptySchema( false );

  layout->addWidget( new QLabel( tr( "Select schema" ) ), 0, 0 );
  layout->addWidget( mCboSchema, 0, 1 );

  mButtonBox = new QDialogButtonBox( this );
  mButtonBox->setOrientation( Qt::Horizontal );
  mButtonBox->setStandardButtons( QDialogButtonBox::Cancel | QDialogButtonBox::Ok );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsDatabaseSchemaSelectionDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsDatabaseSchemaSelectionDialog::accept );

  layout->addWidget( mButtonBox, 1, 0, 2, 0 );
}

QString QgsDatabaseSchemaSelectionDialog::selectedSchema() const
{
  return mCboSchema->currentSchema();
}
