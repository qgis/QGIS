/***************************************************************************
    qgspostgresschemaselectiondialog.cpp
    ---------------------
    begin                : January 2025
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
#include "qgspostgresschemaselectiondialog.h"
#include "moc_qgspostgresschemaselectiondialog.cpp"

#include "qgspostgresconn.h"

#include <QFormLayout>
#include <QApplication>
#include <QMessageBox>

QgsPostgresSchemaSelectionDialog::QgsPostgresSchemaSelectionDialog( QgsDataSourceUri dataSourceUri, QWidget *parent )
  : QDialog( parent ), mDataSourceUri( dataSourceUri )
{
  setWindowTitle( tr( "Select schema" ) );

  QFormLayout *layout = new QFormLayout( this );

  mCboSchema = new QComboBox( this );

  layout->addRow( "Select Schema", mCboSchema );

  mButtonBox = new QDialogButtonBox( this );
  mButtonBox->setOrientation( Qt::Horizontal );
  mButtonBox->setStandardButtons( QDialogButtonBox::Cancel | QDialogButtonBox::Ok );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsPostgresSchemaSelectionDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsPostgresSchemaSelectionDialog::accept );

  layout->addWidget( mButtonBox );

  populateSchemas();
}

void QgsPostgresSchemaSelectionDialog::populateSchemas()
{
  mCboSchema->clear();

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( mDataSourceUri, false );

  QApplication::setOverrideCursor( Qt::WaitCursor );

  if ( !conn )
  {
    QApplication::restoreOverrideCursor();
    QMessageBox::critical( this, tr( "Error" ), tr( "Connection failed" ) + "\n" + mDataSourceUri.connectionInfo( false ) );
    conn->unref();
    return;
  }

  QList<QgsPostgresSchemaProperty> schemas;
  bool ok = conn->getSchemas( schemas );
  conn->unref();

  QApplication::restoreOverrideCursor();

  if ( !ok )
  {
    QMessageBox::critical( this, tr( "Error" ), tr( "Failed to get schemas" ) );
    return;
  }

  for ( const QgsPostgresSchemaProperty &schema : std::as_const( schemas ) )
  {
    mCboSchema->addItem( schema.name );
  }
}

QString QgsPostgresSchemaSelectionDialog::schema()
{
  return mCboSchema->currentText();
}
