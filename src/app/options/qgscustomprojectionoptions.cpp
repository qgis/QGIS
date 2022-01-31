/***************************************************************************
                          qgscustomprojectionoptions.cpp

                             -------------------
    begin                : 2005
    copyright            : (C) 2005 by Tim Sutton
    email                : tim@linfiniti.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscustomprojectionoptions.h"
#include "qgsapplication.h"
#include "qgscoordinatereferencesystemregistry.h"

#include <QMessageBox>
#include <QLocale>
#include <QRegularExpression>
#include <QFileInfo>

QgsCustomProjectionOptionsWidget::QgsCustomProjectionOptionsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );
  setObjectName( QStringLiteral( "QgsCustomProjectionOptionsWidget" ) );

  connect( pbnAdd, &QPushButton::clicked, this, &QgsCustomProjectionOptionsWidget::pbnAdd_clicked );
  connect( pbnRemove, &QPushButton::clicked, this, &QgsCustomProjectionOptionsWidget::pbnRemove_clicked );
  connect( leNameList, &QTreeWidget::currentItemChanged, this, &QgsCustomProjectionOptionsWidget::leNameList_currentItemChanged );

  leNameList->setSelectionMode( QAbstractItemView::ExtendedSelection );

  // user database is created at QGIS startup in QgisApp::createDB
  // we just check whether there is our database [MD]
  if ( !QFileInfo::exists( QgsApplication::qgisSettingsDirPath() ) )
  {
    QgsDebugMsg( QStringLiteral( "The qgis.db does not exist" ) );
  }

  populateList();
  if ( mDefinitions.empty() )
  {
    // create an empty definition which corresponds to the initial state of the dialog
    mDefinitions << Definition();
    QTreeWidgetItem *newItem = new QTreeWidgetItem( leNameList, QStringList() );
    newItem->setText( QgisCrsNameColumn, QString() );
    newItem->setText( QgisCrsParametersColumn, QString() );
  }
  whileBlocking( leName )->setText( mDefinitions[0].name );

  mBlockUpdates++;

  QgsCoordinateReferenceSystem crs;
  Qgis::CrsDefinitionFormat format;
  if ( mDefinitions.at( 0 ).wkt.isEmpty() )
  {
    crs.createFromProj( mDefinitions[0].proj );
    format = Qgis::CrsDefinitionFormat::Proj;
  }
  else
  {
    crs.createFromWkt( mDefinitions[0].wkt );
    format = Qgis::CrsDefinitionFormat::Wkt;
  }
  mCrsDefinitionWidget->setCrs( crs, format );

  mBlockUpdates--;

  leNameList->setCurrentItem( leNameList->topLevelItem( 0 ) );

  leNameList->hideColumn( QgisCrsIdColumn );

  connect( leName, &QLineEdit::textChanged, this, &QgsCustomProjectionOptionsWidget::updateListFromCurrentItem );
  connect( mCrsDefinitionWidget, &QgsCrsDefinitionWidget::crsChanged, this, [ = ]
  {
    if ( !mBlockUpdates )
      updateListFromCurrentItem();
  } );
}

void QgsCustomProjectionOptionsWidget::populateList()
{
  const QList< QgsCoordinateReferenceSystemRegistry::UserCrsDetails > userCrsList = QgsApplication::coordinateReferenceSystemRegistry()->userCrsList();

  for ( const QgsCoordinateReferenceSystemRegistry::UserCrsDetails &details : userCrsList )
  {
    const QString id = QString::number( details.id );

    mExistingCRSnames[id] = details.name;
    const QString actualWkt = details.crs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED, false );
    const QString actualWktFormatted = details.crs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED, true );
    const QString actualProj = details.crs.toProj();
    mExistingCRSwkt[id] = details.wkt.isEmpty() ? QString() : actualWkt;
    mExistingCRSproj[id] = details.wkt.isEmpty() ? actualProj : QString();

    QTreeWidgetItem *newItem = new QTreeWidgetItem( leNameList, QStringList() );
    newItem->setText( QgisCrsNameColumn, details.name );
    newItem->setText( QgisCrsIdColumn, id );
    newItem->setText( QgisCrsParametersColumn, details.wkt.isEmpty() ? actualProj : actualWkt );
    newItem->setData( 0, FormattedWktRole, actualWktFormatted );
  }

  leNameList->sortByColumn( QgisCrsNameColumn, Qt::AscendingOrder );

  QTreeWidgetItemIterator it( leNameList );
  while ( *it )
  {
    QString id = ( *it )->text( QgisCrsIdColumn );
    Definition def;
    def.id = id;
    def.name = mExistingCRSnames[id];
    def.wkt = mExistingCRSwkt[id];
    def.proj = mExistingCRSproj[id];
    mDefinitions.push_back( def );
    it++;
  }
}

bool QgsCustomProjectionOptionsWidget::saveCrs( const QgsCoordinateReferenceSystem &crs, const QString &name, const QString &existingId, bool newEntry, Qgis::CrsDefinitionFormat format )
{
  QString id = existingId;
  if ( newEntry )
  {
    const long returnId = QgsApplication::coordinateReferenceSystemRegistry()->addUserCrs( crs, name, format );
    if ( returnId == -1 )
      return false;
    else
      id = QString::number( returnId );
  }
  else
  {
    if ( !QgsApplication::coordinateReferenceSystemRegistry()->updateUserCrs( id.toLong(), crs, name, format ) )
    {
      return false;
    }
  }

  mExistingCRSwkt[id] = format == Qgis::CrsDefinitionFormat::Wkt ? crs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED, false ) : QString();
  mExistingCRSproj[id] = format == Qgis::CrsDefinitionFormat::Proj ? crs.toProj() : QString();
  mExistingCRSnames[id] = name;

  return true;
}

void QgsCustomProjectionOptionsWidget::pbnAdd_clicked()
{
  QString name = tr( "new CRS" );

  QTreeWidgetItem *newItem = new QTreeWidgetItem( leNameList, QStringList() );

  newItem->setText( QgisCrsNameColumn, name );
  newItem->setText( QgisCrsIdColumn, QString() );
  newItem->setText( QgisCrsParametersColumn, QString() );
  newItem->setData( 0, FormattedWktRole, QString() );

  Definition def;
  def.name = name;
  mDefinitions.push_back( def );
  leNameList->setCurrentItem( newItem );
  leName->selectAll();
  leName->setFocus();

  mCrsDefinitionWidget->setFormat( Qgis::CrsDefinitionFormat::Wkt );
}

void QgsCustomProjectionOptionsWidget::pbnRemove_clicked()
{
  const QModelIndexList selection = leNameList->selectionModel()->selectedRows();
  if ( selection.empty() )
    return;

  // make sure the user really wants to delete these definitions
  if ( QMessageBox::No == QMessageBox::question( this, tr( "Delete Projections" ),
       tr( "Are you sure you want to delete %n projection(s)?", "number of rows", selection.size() ),
       QMessageBox::Yes | QMessageBox::No ) )
    return;

  std::vector< int > selectedRows;
  selectedRows.reserve( selection.size() );
  for ( const QModelIndex &index : selection )
    selectedRows.emplace_back( index.row() );

  //sort rows in reverse order
  std::sort( selectedRows.begin(), selectedRows.end(), std::greater< int >() );
  for ( const int row : selectedRows )
  {
    if ( row < 0 )
    {
      // shouldn't happen?
      continue;
    }
    delete leNameList->takeTopLevelItem( row );
    if ( !mDefinitions[row].id.isEmpty() )
    {
      mDeletedCRSs.push_back( mDefinitions[row].id );
    }
    mDefinitions.erase( mDefinitions.begin() + row );
  }
}

void QgsCustomProjectionOptionsWidget::leNameList_currentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem *previous )
{
  //Store the modifications made to the current element before moving on
  int currentIndex, previousIndex;
  if ( previous )
  {
    previousIndex = leNameList->indexOfTopLevelItem( previous );

    mDefinitions[previousIndex].name = leName->text();
    switch ( mCrsDefinitionWidget->format() )
    {
      case Qgis::CrsDefinitionFormat::Wkt:
        mDefinitions[previousIndex].wkt = mCrsDefinitionWidget->crs().toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED );
        mDefinitions[previousIndex].proj.clear();
        break;

      case Qgis::CrsDefinitionFormat::Proj:
        mDefinitions[previousIndex].proj = mCrsDefinitionWidget->crs().toProj();
        mDefinitions[previousIndex].wkt.clear();
        break;
    }

    previous->setText( QgisCrsNameColumn, leName->text() );
    previous->setText( QgisCrsParametersColumn, multiLineWktToSingleLine( mCrsDefinitionWidget->definitionString() ) );
    previous->setData( 0, FormattedWktRole,  mCrsDefinitionWidget->definitionString() );
  }

  if ( current )
  {
    currentIndex = leNameList->indexOfTopLevelItem( current );
    whileBlocking( leName )->setText( mDefinitions[currentIndex].name );

    mBlockUpdates++;
    mCrsDefinitionWidget->setDefinitionString( !mDefinitions[currentIndex].wkt.isEmpty() ? current->data( 0, FormattedWktRole ).toString() : mDefinitions[currentIndex].proj );
    mCrsDefinitionWidget->setFormat( mDefinitions.at( currentIndex ).wkt.isEmpty() ? Qgis::CrsDefinitionFormat::Proj : Qgis::CrsDefinitionFormat::Wkt );
    mBlockUpdates--;
  }
  else
  {
    //Can happen that current is null, for example if we just deleted the last element
    leName->clear();
    mBlockUpdates++;
    mCrsDefinitionWidget->setDefinitionString( QString() );
    mCrsDefinitionWidget->setFormat( Qgis::CrsDefinitionFormat::Wkt );
    mBlockUpdates--;
    return;
  }
}

bool QgsCustomProjectionOptionsWidget::isValid()
{
  updateListFromCurrentItem();

  //Check if all CRS are valid:
  QgsCoordinateReferenceSystem crs;
  for ( const Definition &def : std::as_const( mDefinitions ) )
  {
    if ( !def.wkt.trimmed().isEmpty() )
      crs.createFromWkt( def.wkt );
    else if ( !def.proj.trimmed().isEmpty() )
      crs.createFromProj( def.proj );
    else
      continue;

    if ( !crs.isValid() )
    {
      // auto select the invalid CRS row
      for ( int row = 0; row < leNameList->model()->rowCount(); ++row )
      {
        if ( leNameList->model()->data( leNameList->model()->index( row, QgisCrsNameColumn ) ).toString() == def.name )
        {
          leNameList->setCurrentItem( leNameList->invisibleRootItem()->child( row ) );
          break;
        }
      }

      QMessageBox::warning( this, tr( "Custom Coordinate Reference System" ),
                            tr( "The definition of '%1' is not valid." ).arg( def.name ) );
      return false;
    }
    else if ( !crs.authid().isEmpty() && !crs.authid().startsWith( QLatin1String( "USER" ), Qt::CaseInsensitive ) )
    {
      // auto select the invalid CRS row
      for ( int row = 0; row < leNameList->model()->rowCount(); ++row )
      {
        if ( leNameList->model()->data( leNameList->model()->index( row, QgisCrsNameColumn ) ).toString() == def.name )
        {
          leNameList->setCurrentItem( leNameList->invisibleRootItem()->child( row ) );
          break;
        }
      }

      if ( def.wkt.isEmpty() )
      {
        QMessageBox::warning( this, tr( "Custom Coordinate Reference System" ),
                              tr( "Cannot save '%1' — this Proj string definition is equivalent to %2.\n\nTry changing the CRS definition to a WKT format instead." ).arg( def.name, crs.authid() ) );
      }
      else
      {
        const QStringList authparts = crs.authid().split( ':' );
        QString ref;
        if ( authparts.size() == 2 )
        {
          ref = QStringLiteral( "ID[\"%1\",%2]" ).arg( authparts.at( 0 ), authparts.at( 1 ) );
        }
        if ( !ref.isEmpty() && crs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED ).contains( ref ) )
        {
          QMessageBox::warning( this, tr( "Custom Coordinate Reference System" ),
                                tr( "Cannot save '%1' — the definition is equivalent to %2.\n\n(Try removing \"%3\" from the WKT definition.)" ).arg( def.name, crs.authid(), ref ) );
        }
        else
        {
          QMessageBox::warning( this, tr( "Custom Coordinate Reference System" ),
                                tr( "Cannot save '%1' — the definition is equivalent to %2." ).arg( def.name, crs.authid() ) );
        }
      }
      return false;
    }
  }
  return true;
}

void QgsCustomProjectionOptionsWidget::apply()
{
  // note that isValid() will have already been called prior to this, so we don't need to re-validate!

  //Modify the CRS changed:
  bool saveSuccess = true;
  QgsCoordinateReferenceSystem crs;
  for ( const Definition &def : std::as_const( mDefinitions ) )
  {
    if ( !def.wkt.isEmpty() )
      crs.createFromWkt( def.wkt );
    else
      crs.createFromProj( def.proj );

    //Test if we just added this CRS (if it has no existing ID)
    if ( def.id.isEmpty() )
    {
      saveSuccess &= saveCrs( crs, def.name, QString(), true, !def.wkt.isEmpty() ? Qgis::CrsDefinitionFormat::Wkt : Qgis::CrsDefinitionFormat::Proj );
    }
    else
    {
      if ( mExistingCRSnames[def.id] != def.name
           || ( !def.wkt.isEmpty() && mExistingCRSwkt[def.id] != def.wkt )
           || ( !def.proj.isEmpty() && mExistingCRSproj[def.id] != def.proj )
         )
      {
        saveSuccess &= saveCrs( crs, def.name, def.id, false, !def.wkt.isEmpty() ? Qgis::CrsDefinitionFormat::Wkt : Qgis::CrsDefinitionFormat::Proj );
      }
    }
    if ( ! saveSuccess )
    {
      QgsDebugMsg( QStringLiteral( "Error when saving CRS '%1'" ).arg( def.name ) );
    }
  }
  QgsDebugMsgLevel( QStringLiteral( "We remove the deleted CRS." ), 4 );
  for ( int i = 0; i < mDeletedCRSs.size(); ++i )
  {
    saveSuccess &= QgsApplication::coordinateReferenceSystemRegistry()->removeUserCrs( mDeletedCRSs[i].toLong() );
    if ( ! saveSuccess )
    {
      QgsDebugMsg( QStringLiteral( "Error deleting CRS for '%1'" ).arg( mDefinitions.at( i ).name ) );
    }
  }
}

void QgsCustomProjectionOptionsWidget::updateListFromCurrentItem()
{
  QTreeWidgetItem *item = leNameList->currentItem();
  if ( !item )
    return;

  int currentIndex = leNameList->indexOfTopLevelItem( item );
  if ( currentIndex < 0 )
    return;

  mDefinitions[currentIndex].name = leName->text();
  switch ( mCrsDefinitionWidget->format() )
  {
    case Qgis::CrsDefinitionFormat::Wkt:
      mDefinitions[currentIndex].wkt = mCrsDefinitionWidget->definitionString();
      mDefinitions[currentIndex].proj.clear();
      break;

    case Qgis::CrsDefinitionFormat::Proj:
      mDefinitions[currentIndex].proj = mCrsDefinitionWidget->definitionString();
      mDefinitions[currentIndex].wkt.clear();
      break;
  }

  item->setText( QgisCrsNameColumn, leName->text() );
  item->setText( QgisCrsParametersColumn, multiLineWktToSingleLine( mCrsDefinitionWidget->definitionString() ) );
  item->setData( 0, FormattedWktRole, mCrsDefinitionWidget->definitionString() );
}

QString QgsCustomProjectionOptionsWidget::multiLineWktToSingleLine( const QString &wkt )
{
  QString res = wkt;
  QRegularExpression re( QStringLiteral( "\\s*\\n\\s*" ) );
  re.setPatternOptions( QRegularExpression::MultilineOption );
  res.replace( re, QString() );
  return res;
}

QString QgsCustomProjectionOptionsWidget::helpKey() const
{
  return QStringLiteral( "working_with_projections/working_with_projections.html" );
}


//
// QgsCustomProjectionOptionsFactory
//
QgsCustomProjectionOptionsFactory::QgsCustomProjectionOptionsFactory()
  : QgsOptionsWidgetFactory( tr( "User Defined CRS" ), QIcon() )
{

}

QIcon QgsCustomProjectionOptionsFactory::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mActionCustomProjection.svg" ) );
}

QgsOptionsPageWidget *QgsCustomProjectionOptionsFactory::createWidget( QWidget *parent ) const
{
  return new QgsCustomProjectionOptionsWidget( parent );
}

QStringList QgsCustomProjectionOptionsFactory::path() const
{
  return {QStringLiteral( "crs_and_transforms" ) };
}

