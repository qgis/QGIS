/***************************************************************************
    qgsfindfilesbypatternwidget.cpp
    -----------------------------
    begin                : April 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfindfilesbypatternwidget.h"
#include "qgsgui.h"
#include "qgssettings.h"

#include <QDir>
#include <QDirIterator>
#include <QDialogButtonBox>


QgsFindFilesByPatternWidget::QgsFindFilesByPatternWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mFolderWidget->setStorageMode( QgsFileWidget::GetDirectory );
  mResultsTable->setColumnCount( 2 );

  mResultsTable->setHorizontalHeaderLabels( QStringList() << tr( "File" ) << tr( "Directory" ) );
  mResultsTable->horizontalHeader()->setSectionResizeMode( 0, QHeaderView::Stretch );
  mResultsTable->horizontalHeader()->setSectionResizeMode( 1, QHeaderView::Stretch );

  connect( mFindButton, &QPushButton::clicked, this, &QgsFindFilesByPatternWidget::find );

  const QgsSettings settings;
  mFolderWidget->setFilePath( settings.value( QStringLiteral( "qgis/lastFindRecursiveFolder" ) ).toString() );
  mFindButton->setEnabled( !mFolderWidget->filePath().isEmpty() );
  connect( mFolderWidget, &QgsFileWidget::fileChanged, this, [ = ]( const QString & filePath )
  {
    QgsSettings settings;
    settings.setValue( QStringLiteral( "qgis/lastFindRecursiveFolder" ), filePath );
    mFindButton->setEnabled( !filePath.isEmpty() );
  } );

}

void QgsFindFilesByPatternWidget::find()
{
  mFindButton->setText( tr( "Cancel" ) );
  disconnect( mFindButton, &QPushButton::clicked, this, &QgsFindFilesByPatternWidget::find );
  connect( mFindButton, &QPushButton::clicked, this, &QgsFindFilesByPatternWidget::cancel );
  mCanceled = false;

  mResultsTable->setRowCount( 0 );
  mFiles.clear();

  const QString pattern = mPatternLineEdit->text();
  const QString path = mFolderWidget->filePath();

  const QDir startDir( path );

  QStringList filter;
  if ( !pattern.isEmpty() )
    filter << pattern;

  QDirIterator it( path, filter, QDir::AllEntries | QDir::NoSymLinks | QDir::NoDotAndDotDot, mRecursiveCheckBox->isChecked() ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags );
  const QStringList files;
  while ( it.hasNext() )
  {
    const QString fullPath = it.next();
    mFiles << fullPath;

    const QFileInfo fi( fullPath );

    const QString toolTip = QDir::toNativeSeparators( fullPath );
    const QString fileName = fi.fileName();
    const QString relativeDirectory = QDir::toNativeSeparators( startDir.relativeFilePath( fi.dir().absolutePath() ) );
    const QString fullDirectory = QDir::toNativeSeparators( fi.dir().absolutePath() );

    QTableWidgetItem *fileNameItem = new QTableWidgetItem( fileName );
    fileNameItem->setToolTip( toolTip );
    fileNameItem->setFlags( fileNameItem->flags() ^ Qt::ItemIsEditable );

    QTableWidgetItem *directoryItem = new QTableWidgetItem( relativeDirectory );
    directoryItem->setToolTip( fullDirectory );
    directoryItem->setFlags( directoryItem->flags() ^ Qt::ItemIsEditable );

    const int row = mResultsTable->rowCount();
    mResultsTable->insertRow( row );
    mResultsTable->setItem( row, 0, fileNameItem );
    mResultsTable->setItem( row, 1, directoryItem );

    QCoreApplication::processEvents();
    if ( mCanceled )
      break;
  }

  mFindButton->setText( tr( "Find Files" ) );
  disconnect( mFindButton, &QPushButton::clicked, this, &QgsFindFilesByPatternWidget::cancel );
  connect( mFindButton, &QPushButton::clicked, this, &QgsFindFilesByPatternWidget::find );

  emit findComplete( mFiles );
}

void QgsFindFilesByPatternWidget::cancel()
{
  mCanceled = true;
}


//
// QgsFindFilesByPatternDialog
//

QgsFindFilesByPatternDialog::QgsFindFilesByPatternDialog( QWidget *parent )
  : QDialog( parent )
{
  setWindowTitle( tr( "Find Files by Pattern" ) );
  setObjectName( "QgsFindFilesByPatternDialog" );

  QgsGui::enableAutoGeometryRestore( this );

  QVBoxLayout *vLayout = new QVBoxLayout();
  mWidget = new QgsFindFilesByPatternWidget();

  vLayout->addWidget( mWidget );
  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  vLayout->addWidget( mButtonBox );
  setLayout( vLayout );

  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
  connect( mWidget, &QgsFindFilesByPatternWidget::findComplete, this, [ = ]( const QStringList & files )
  {
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( !files.empty() );
  } );
}

QStringList QgsFindFilesByPatternDialog::files() const
{
  return mWidget->files();
}
