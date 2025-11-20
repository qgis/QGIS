/***************************************************************************
    qgspostgresprojectversionsdialog.cpp
    ---------------------
    begin                : October 2025
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


#include "qgspostgresprojectversionsdialog.h"
#include "moc_qgspostgresprojectversionsdialog.cpp"

#include <QVBoxLayout>

#include "qgspostgresprojectstorage.h"
#include "qgsproject.h"

QgsPostgresProjectVersionsDialog::QgsPostgresProjectVersionsDialog( const QString &connectionName, const QString &schema, const QString &project, QWidget *parent )
  : QDialog { parent }
{
  setWindowTitle( tr( "Project Versions for “%1” in Schema “%2”" ).arg( project, schema ) );
  setMinimumWidth( 600 );

  QVBoxLayout *layout = new QVBoxLayout();
  setLayout( layout );

  mTableView = new QTableView( this );
  mTableView->setSelectionBehavior( QAbstractItemView::SelectRows );
  mTableView->setSelectionMode( QAbstractItemView::SingleSelection );

  mModel = new QgsPostgresProjectVersionsModel( connectionName, this );
  mTableView->setModel( mModel );
  mModel->populateVersions( schema, project );
  mTableView->resizeColumnsToContents();
  mTableView->selectRow( 0 );

  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Yes, this );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsPostgresProjectVersionsDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsPostgresProjectVersionsDialog::accept );

  layout->addWidget( mTableView );
  layout->addWidget( mButtonBox );
}

void QgsPostgresProjectVersionsDialog::accept()
{
  QgsPostgresProjectUri projectUri = mModel->projectUriForRow( mTableView->currentIndex().row() );
  QString encodedUri = QgsPostgresProjectStorage::encodeUri( projectUri );
  QgsProject::instance()->read( encodedUri );

  QDialog::accept();
}
