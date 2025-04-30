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

#include <QFormLayout>
#include <QApplication>
#include <QMessageBox>

#include "qgsdatabaseschemaselectiondialog.h"
#include "moc_qgsdatabaseschemaselectiondialog.cpp"

QgsDatabaseSchemaSelectionDialog::QgsDatabaseSchemaSelectionDialog( QgsAbstractDatabaseProviderConnection *connection, QWidget *parent )
  : QDialog( parent )
{
  setWindowTitle( tr( "Select Schema" ) );

  QFormLayout *layout = new QFormLayout( this );

  mCboSchema = new QgsDatabaseSchemaComboBox( connection, this );
  mCboSchema->setAllowEmptySchema( false );

  layout->addRow( tr( "Select schema" ), mCboSchema );

  mButtonBox = new QDialogButtonBox( this );
  mButtonBox->setOrientation( Qt::Horizontal );
  mButtonBox->setStandardButtons( QDialogButtonBox::Cancel | QDialogButtonBox::Ok );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsDatabaseSchemaSelectionDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsDatabaseSchemaSelectionDialog::accept );

  layout->addWidget( mButtonBox );
}

QString QgsDatabaseSchemaSelectionDialog::selectedSchema() const
{
  return mCboSchema->currentSchema();
}
