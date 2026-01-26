/***************************************************************************
    qgspostgresimportprojectdialog.cpp
    ---------------------
    begin                : September 2025
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

#include "qgspostgresimportprojectdialog.h"

#include "qgsapplication.h"
#include "qgsguiutils.h"
#include "qgspostgresconn.h"
#include "qgspostgresutils.h"
#include "qgsproject.h"

#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QVBoxLayout>

#include "moc_qgspostgresimportprojectdialog.cpp"

QgsPostgresImportProjectDialog::QgsPostgresImportProjectDialog( const QString connectionName, const QString targetSchema, QWidget *parent )
  : QDialog { parent }, mSchemaToImportTo( targetSchema )
{
  setWindowTitle( tr( "Import Projects to Schema “%1”" ).arg( targetSchema ) );
  setMinimumWidth( 600 );

  const QgsDataSourceUri uri = QgsPostgresConn::connUri( connectionName );
  mDbConnection = QgsPostgresConn::connectDb( uri, false );

  QVBoxLayout *mainLayout = new QVBoxLayout();
  setLayout( mainLayout );

  QStringList fileFilter;
  fileFilter << u"*.qgs"_s << u"*.qgz"_s << u"*.QGS"_s << u"*.QGZ"_s;

  mButtonAdd = new QToolButton( this );
  mButtonAdd->setIcon( QIcon( QgsApplication::iconPath( "mActionAdd.svg" ) ) );
  mButtonAdd->setPopupMode( QToolButton::InstantPopup );
  mButtonAdd->setToolTip( tr( "Add selected project or projects from folder" ) );

  mButtonRemove = new QToolButton( this );
  mButtonRemove->setIcon( QIcon( QgsApplication::iconPath( "mActionRemove.svg" ) ) );
  mButtonRemove->setToolTip( tr( "Remove selected row" ) );

  QVBoxLayout *buttonLayout = new QVBoxLayout();
  buttonLayout->addWidget( mButtonAdd );
  buttonLayout->addWidget( mButtonRemove );
  buttonLayout->addItem( new QSpacerItem( 0, 0, QSizePolicy::Maximum, QSizePolicy::Expanding ) );

  // Create a menu for the button
  QMenu *menu = new QMenu( mButtonAdd );

  menu->addAction( tr( "Add Project File…" ), this, [this, fileFilter]() {
    QString projPath = QFileDialog::getOpenFileName( this, tr( "Select Project File" ), lastUsedDir(), tr( "QGIS files" ) + fileFilter.join( " " ) );
    if ( projPath.isEmpty() )
      return;
    QString projName = prepareProjectName( projPath );
    addProject( projName, projPath );
  } );

  menu->addAction( tr( "Add Projects from Folder…" ), this, [this, fileFilter]() {
    QString searchPath = QFileDialog::getExistingDirectory( this, tr( "Select Projects from Folder" ), lastUsedDir() );

    if ( searchPath.isEmpty() )
      return;

    QSettings settings;
    settings.setValue( u"UI/lastFileNameWidgetDir"_s, searchPath );

    QgsTemporaryCursorOverride busyCursor( Qt::WaitCursor );
    QDirIterator it( searchPath, fileFilter, QDir::Files );
    while ( it.hasNext() )
    {
      QString projPath = it.next();
      QString projName = prepareProjectName( projPath );
      addProject( projName, projPath );
    }
  } );

  menu->addAction( tr( "Add Projects from Folder Recursively…" ), this, [this, fileFilter]() {
    QString searchPath = QFileDialog::getExistingDirectory( this, tr( "Select Projects from Folder Recursively" ), lastUsedDir() );

    if ( searchPath.isEmpty() )
      return;

    QSettings settings;
    settings.setValue( u"UI/lastFileNameWidgetDir"_s, searchPath );

    QgsTemporaryCursorOverride busyCursor( Qt::WaitCursor );
    QDirIterator it( searchPath, fileFilter, QDir::Files, QDirIterator::Subdirectories );
    while ( it.hasNext() )
    {
      QString projPath = it.next();
      QString projName = prepareProjectName( projPath );
      addProject( projName, projPath );
    }
  } );

  mButtonAdd->setMenu( menu );

  connect( mButtonRemove, &QToolButton::clicked, this, [this]() {
    QList<QTableWidgetItem *> items = mFilesTableWidget->selectedItems();

    while ( !items.empty() )
    {
      QTableWidgetItem *item = items.takeLast();
      mFilesTableWidget->removeRow( item->row() );
    }
  } );

  mFilesTableWidget = new QTableWidget( 0, 2, this );
  mFilesTableWidget->setHorizontalHeaderLabels( { tr( "Project Name" ), tr( "Project Path" ) } );
  mFilesTableWidget->horizontalHeader()->setSectionResizeMode( QHeaderView::Interactive );
  mFilesTableWidget->horizontalHeader()->setStretchLastSection( true );

  QHBoxLayout *tableButtonsLayout = new QHBoxLayout();
  tableButtonsLayout->addWidget( mFilesTableWidget );
  tableButtonsLayout->addLayout( buttonLayout );

  mButtonBox = new QDialogButtonBox( this );
  mButtonBox->setOrientation( Qt::Horizontal );
  mButtonBox->setStandardButtons( QDialogButtonBox::Cancel | QDialogButtonBox::Ok );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsPostgresImportProjectDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsPostgresImportProjectDialog::accept );

  mainLayout->addLayout( tableButtonsLayout );
  mainLayout->addWidget( mButtonBox );

  mExistingProjectNames = projectNamesInSchema();
}

QgsPostgresImportProjectDialog::~QgsPostgresImportProjectDialog()
{
  mDbConnection->unref();
}

QSet<QString> QgsPostgresImportProjectDialog::projectNamesInSchema()
{
  QSet<QString> existingProjects;

  if ( QgsPostgresUtils::projectsTableExists( mDbConnection, mSchemaToImportTo ) )
  {
    QString existingProjectsSql = u"SELECT name FROM %1.qgis_projects;"_s.arg( QgsPostgresConn::quotedIdentifier( mSchemaToImportTo ) );
    QgsPostgresResult res( mDbConnection->PQexec( existingProjectsSql ) );

    for ( int i = 0; i < res.PQntuples(); i++ )
    {
      existingProjects << res.PQgetvalue( i, 0 );
    }
  }

  return existingProjects;
}

QString QgsPostgresImportProjectDialog::prepareProjectName( const QString &fullFilePath )
{
  QgsProject project;
  project.read( fullFilePath );
  QString projectName = project.title().isEmpty() ? project.baseName() : project.title();

  projectName = createUniqueProjectName( projectName );

  mExistingProjectNames.insert( projectName );
  return projectName;
}

QString QgsPostgresImportProjectDialog::createUniqueProjectName( const QString &projectName )
{
  QString lastAddedProjectName = projectName;

  if ( mExistingProjectNames.contains( lastAddedProjectName ) )
  {
    const thread_local QRegularExpression reEndsNumber( "_([0-9]+)$" );
    QRegularExpressionMatch match;

    while ( mExistingProjectNames.find( lastAddedProjectName ) != mExistingProjectNames.end() )
    {
      match = reEndsNumber.match( lastAddedProjectName );
      if ( match.hasMatch() )
      {
        const int number = match.capturedTexts().constLast().toInt();
        QString paddedNumber;

        if ( match.captured( 0 ).startsWith( "_0" ) )
        {
          paddedNumber = QString::number( number + 1 ).rightJustified( match.capturedLength() - 1, QChar( '0' ) );
        }
        else
        {
          paddedNumber = QString::number( number + 1 );
        }

        lastAddedProjectName = lastAddedProjectName.left( lastAddedProjectName.length() - match.capturedLength() + 1 ) + paddedNumber;
      }
      else
      {
        lastAddedProjectName = lastAddedProjectName.append( "_1" );
      }
    }
  }

  return lastAddedProjectName;
}

void QgsPostgresImportProjectDialog::addProject( const QString &name, const QString &path )
{
  int row = mFilesTableWidget->rowCount();
  mFilesTableWidget->insertRow( row );

  QTableWidgetItem *projectNameItem = new QTableWidgetItem( name );
  projectNameItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
  mFilesTableWidget->setItem( row, 0, projectNameItem );

  QTableWidgetItem *projectPathItem = new QTableWidgetItem( path );
  projectPathItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  mFilesTableWidget->setItem( row, 1, projectPathItem );

  mFilesTableWidget->resizeColumnsToContents();
}

QList<QPair<QString, QString>> QgsPostgresImportProjectDialog::projectsToSave()
{
  mExistingProjectNames = projectNamesInSchema();

  QList<QPair<QString, QString>> projectsWithName;

  for ( int i = 0; i < mFilesTableWidget->rowCount(); i++ )
  {
    QString projectName = createUniqueProjectName( mFilesTableWidget->item( i, 0 )->text() );
    mExistingProjectNames << projectName;
    projectsWithName << QPair<QString, QString>( mFilesTableWidget->item( i, 1 )->text(), projectName );
  }

  return projectsWithName;
}

QString QgsPostgresImportProjectDialog::lastUsedDir()
{
  QgsSettings settings;
  QString prevPath;

  QString defPath = QDir::cleanPath( QFileInfo( QgsProject::instance()->absoluteFilePath() ).path() );
  if ( defPath.isEmpty() )
  {
    defPath = QDir::homePath();
  }
  prevPath = settings.value( u"UI/lastFileNameWidgetDir"_s, defPath ).toString();

  return prevPath;
}
