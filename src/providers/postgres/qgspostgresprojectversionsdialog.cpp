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

#include "qgspostgresprojectstorage.h"
#include "qgsproject.h"

#include <QPushButton>
#include <QVBoxLayout>

#include "moc_qgspostgresprojectversionsdialog.cpp"

QgsPostgresProjectVersionsDialog::QgsPostgresProjectVersionsDialog( const QString &connectionName, const QString &schema, const QString &project, QWidget *parent )
  : QDialog { parent }
{
  setWindowTitle( tr( "Project Versions for “%1” in Schema “%2”" ).arg( project, schema ) );
  setMinimumWidth( 600 );

  QVBoxLayout *layout = new QVBoxLayout();
  setLayout( layout );

  mTreeView = new QTreeView( this );
  mTreeView->setSelectionBehavior( QAbstractItemView::SelectRows );
  mTreeView->setSelectionMode( QAbstractItemView::SingleSelection );

  mModel = new QgsPostgresProjectVersionsModel( connectionName, this );
  mTreeView->setModel( mModel );
  mModel->populateVersions( schema, project );
  mTreeView->resizeColumnToContents( 0 );
  mTreeView->setCurrentIndex( mModel->index( 0, 0, QModelIndex() ) );

  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Yes, this );
  QPushButton *yesButton = mButtonBox->button( QDialogButtonBox::Yes );
  if ( yesButton )
  {
    yesButton->setText( tr( "Load selected version of the project" ) );
  }

  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsPostgresProjectVersionsDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsPostgresProjectVersionsDialog::accept );

  layout->addWidget( mTreeView );
  layout->addWidget( mButtonBox );
}

void QgsPostgresProjectVersionsDialog::accept()
{
  QgsPostgresProjectUri projectUri = mModel->projectUriForRow( mTableView->currentIndex().row() );
  QString encodedUri = QgsPostgresProjectStorage::encodeUri( projectUri );
  QgsProject::instance()->read( encodedUri );

  QDialog::accept();
}
