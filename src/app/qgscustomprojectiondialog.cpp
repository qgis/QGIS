/***************************************************************************
                          qgscustomprojectiondialog.cpp

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

#include "qgscustomprojectiondialog.h"

//qgis includes
#include "qgis.h" //<--magick numbers
#include "qgisapp.h" //<--theme icons
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsprojectionselectiondialog.h"
#include "qgssettings.h"
#include "qgsgui.h"
#include "qgscoordinatereferencesystemregistry.h"

//qt includes
#include <QFileInfo>
#include <QMessageBox>
#include <QLocale>
#include <QRegularExpression>

//proj includes
#include "qgsprojutils.h"
#include <proj.h>

#include "qgsogrutils.h"
#include <ogr_srs_api.h>

QgsCustomProjectionDialog::QgsCustomProjectionDialog( QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( pbnCalculate, &QPushButton::clicked, this, &QgsCustomProjectionDialog::pbnCalculate_clicked );
  connect( pbnAdd, &QPushButton::clicked, this, &QgsCustomProjectionDialog::pbnAdd_clicked );
  connect( pbnRemove, &QPushButton::clicked, this, &QgsCustomProjectionDialog::pbnRemove_clicked );
  connect( pbnCopyCRS, &QPushButton::clicked, this, &QgsCustomProjectionDialog::pbnCopyCRS_clicked );
  connect( leNameList, &QTreeWidget::currentItemChanged, this, &QgsCustomProjectionDialog::leNameList_currentItemChanged );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsCustomProjectionDialog::buttonBox_accepted );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsCustomProjectionDialog::showHelp );
  connect( mButtonValidate, &QPushButton::clicked, this, &QgsCustomProjectionDialog::validateCurrent );

  leNameList->setSelectionMode( QAbstractItemView::ExtendedSelection );

  mFormatComboBox->addItem( tr( "WKT (Recommended)" ), static_cast< int >( QgsCoordinateReferenceSystem::FormatWkt ) );
  mFormatComboBox->addItem( tr( "Proj String (Legacy — Not Recommended)" ), static_cast< int >( QgsCoordinateReferenceSystem::FormatProj ) );
  mFormatComboBox->setCurrentIndex( mFormatComboBox->findData( static_cast< int >( QgsCoordinateReferenceSystem::FormatWkt ) ) );

  // user database is created at QGIS startup in QgisApp::createDB
  // we just check whether there is our database [MD]
  QFileInfo fileInfo;
  fileInfo.setFile( QgsApplication::qgisSettingsDirPath() );
  if ( !fileInfo.exists() )
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
  whileBlocking( teParameters )->setPlainText( mDefinitions[0].wkt.isEmpty() ? mDefinitions[0].proj : mDefinitions[0].wkt );
  mFormatComboBox->setCurrentIndex( mFormatComboBox->findData( static_cast< int >( mDefinitions[0].wkt.isEmpty() ? QgsCoordinateReferenceSystem::FormatProj : QgsCoordinateReferenceSystem::FormatWkt ) ) );
  leNameList->setCurrentItem( leNameList->topLevelItem( 0 ) );

  leNameList->hideColumn( QgisCrsIdColumn );

  connect( leName, &QLineEdit::textChanged, this, &QgsCustomProjectionDialog::updateListFromCurrentItem );
  connect( teParameters, &QPlainTextEdit::textChanged, this, &QgsCustomProjectionDialog::updateListFromCurrentItem );
  connect( mFormatComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsCustomProjectionDialog::formatChanged );
}

void QgsCustomProjectionDialog::populateList()
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

bool QgsCustomProjectionDialog::saveCrs( QgsCoordinateReferenceSystem crs, const QString &name, const QString &existingId, bool newEntry, QgsCoordinateReferenceSystem::Format format )
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

  mExistingCRSwkt[id] = format == QgsCoordinateReferenceSystem::FormatWkt ? crs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED, false ) : QString();
  mExistingCRSproj[id] = format == QgsCoordinateReferenceSystem::FormatProj ? crs.toProj() : QString();
  mExistingCRSnames[id] = name;

  return true;
}

void QgsCustomProjectionDialog::pbnAdd_clicked()
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

  mFormatComboBox->setCurrentIndex( mFormatComboBox->findData( QgsCoordinateReferenceSystem::FormatWkt ) );
}

void QgsCustomProjectionDialog::pbnRemove_clicked()
{
  const QModelIndexList selection = leNameList->selectionModel()->selectedRows();
  if ( selection.empty() )
    return;

  // make sure the user really wants to delete these definitions
  if ( QMessageBox::No == QMessageBox::question( this, tr( "Delete Projections" ),
       tr( "Are you sure you want to delete %n projections(s)?", "number of rows", selection.size() ),
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

void QgsCustomProjectionDialog::leNameList_currentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem *previous )
{
  //Store the modifications made to the current element before moving on
  int currentIndex, previousIndex;
  if ( previous )
  {
    previousIndex = leNameList->indexOfTopLevelItem( previous );

    mDefinitions[previousIndex].name = leName->text();
    switch ( static_cast< QgsCoordinateReferenceSystem::Format >( mFormatComboBox->currentData().toInt() ) )
    {
      case QgsCoordinateReferenceSystem::FormatWkt:
        mDefinitions[previousIndex].wkt = teParameters->toPlainText();
        mDefinitions[previousIndex].proj.clear();
        break;

      case QgsCoordinateReferenceSystem::FormatProj:
        mDefinitions[previousIndex].proj = teParameters->toPlainText();
        mDefinitions[previousIndex].wkt.clear();
        break;
    }

    previous->setText( QgisCrsNameColumn, leName->text() );
    previous->setText( QgisCrsParametersColumn, multiLineWktToSingleLine( teParameters->toPlainText() ) );
    previous->setData( 0, FormattedWktRole, teParameters->toPlainText() );
  }

  if ( current )
  {
    currentIndex = leNameList->indexOfTopLevelItem( current );
    whileBlocking( leName )->setText( mDefinitions[currentIndex].name );
    whileBlocking( teParameters )->setPlainText( !mDefinitions[currentIndex].wkt.isEmpty() ? current->data( 0, FormattedWktRole ).toString() : mDefinitions[currentIndex].proj );
    whileBlocking( mFormatComboBox )->setCurrentIndex( mFormatComboBox->findData( static_cast< int >( mDefinitions[currentIndex].wkt.isEmpty() ? QgsCoordinateReferenceSystem::FormatProj : QgsCoordinateReferenceSystem::FormatWkt ) ) );
  }
  else
  {
    //Can happen that current is null, for example if we just deleted the last element
    leName->clear();
    teParameters->clear();
    whileBlocking( mFormatComboBox )->setCurrentIndex( mFormatComboBox->findData( static_cast< int >( QgsCoordinateReferenceSystem::FormatWkt ) ) );
    return;
  }
}

void QgsCustomProjectionDialog::pbnCopyCRS_clicked()
{
  std::unique_ptr< QgsProjectionSelectionDialog > selector = std::make_unique< QgsProjectionSelectionDialog >( this );
  if ( selector->exec() )
  {
    QgsCoordinateReferenceSystem srs = selector->crs();
    if ( leNameList->topLevelItemCount() == 0 )
    {
      pbnAdd_clicked();
    }

    whileBlocking( mFormatComboBox )->setCurrentIndex( mFormatComboBox->findData( static_cast< int >( QgsCoordinateReferenceSystem::FormatWkt ) ) );
    teParameters->setPlainText( srs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED, true ) );
    mDefinitions[leNameList->currentIndex().row()].wkt = srs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED, false );
    mDefinitions[leNameList->currentIndex().row()].proj.clear();

    leNameList->currentItem()->setText( QgisCrsParametersColumn, srs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED, false ) );
    leNameList->currentItem()->setData( 0, FormattedWktRole, srs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED, true ) );
  }
}

void QgsCustomProjectionDialog::buttonBox_accepted()
{
  updateListFromCurrentItem();

  QgsDebugMsgLevel( QStringLiteral( "We save the modified CRS." ), 4 );

  //Check if all CRS are valid:
  QgsCoordinateReferenceSystem crs;
  for ( const Definition &def : std::as_const( mDefinitions ) )
  {
    if ( !def.wkt.isEmpty() )
      crs.createFromWkt( def.wkt );
    else
      crs.createFromProj( def.proj );

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
      return;
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
      return;
    }
  }

  //Modify the CRS changed:
  bool saveSuccess = true;
  for ( const Definition &def : std::as_const( mDefinitions ) )
  {
    if ( !def.wkt.isEmpty() )
      crs.createFromWkt( def.wkt );
    else
      crs.createFromProj( def.proj );

    //Test if we just added this CRS (if it has no existing ID)
    if ( def.id.isEmpty() )
    {
      saveSuccess &= saveCrs( crs, def.name, QString(), true, !def.wkt.isEmpty() ? QgsCoordinateReferenceSystem::FormatWkt : QgsCoordinateReferenceSystem::FormatProj );
    }
    else
    {
      if ( mExistingCRSnames[def.id] != def.name
           || ( !def.wkt.isEmpty() && mExistingCRSwkt[def.id] != def.wkt )
           || ( !def.proj.isEmpty() && mExistingCRSproj[def.id] != def.proj )
         )
      {
        saveSuccess &= saveCrs( crs, def.name, def.id, false, !def.wkt.isEmpty() ? QgsCoordinateReferenceSystem::FormatWkt : QgsCoordinateReferenceSystem::FormatProj );
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
      QgsDebugMsg( QStringLiteral( "Error deleting CRS for '%1'" ).arg( mDefinitions[i].name ) );
    }
  }
  if ( saveSuccess )
  {
    accept();
  }
}

void QgsCustomProjectionDialog::updateListFromCurrentItem()
{
  QTreeWidgetItem *item = leNameList->currentItem();
  if ( !item )
    return;

  int currentIndex = leNameList->indexOfTopLevelItem( item );
  if ( currentIndex < 0 )
    return;

  mDefinitions[currentIndex].name = leName->text();
  switch ( static_cast< QgsCoordinateReferenceSystem::Format >( mFormatComboBox->currentData().toInt() ) )
  {
    case QgsCoordinateReferenceSystem::FormatWkt:
      mDefinitions[currentIndex].wkt = teParameters->toPlainText();
      mDefinitions[currentIndex].proj.clear();
      break;

    case QgsCoordinateReferenceSystem::FormatProj:
      mDefinitions[currentIndex].proj = teParameters->toPlainText();
      mDefinitions[currentIndex].wkt.clear();
      break;
  }

  item->setText( QgisCrsNameColumn, leName->text() );
  item->setText( QgisCrsParametersColumn, multiLineWktToSingleLine( teParameters->toPlainText() ) );
  item->setData( 0, FormattedWktRole, teParameters->toPlainText() );
}

static void proj_collecting_logger( void *user_data, int /*level*/, const char *message )
{
  QStringList *dest = reinterpret_cast< QStringList * >( user_data );
  QString messageString( message );
  messageString.replace( QLatin1String( "internal_proj_create: " ), QString() );
  dest->append( messageString );
}

void QgsCustomProjectionDialog::validateCurrent()
{
  const QString projDef = teParameters->toPlainText();

  PJ_CONTEXT *context = proj_context_create();

  QStringList projErrors;
  proj_log_func( context, &projErrors, proj_collecting_logger );
  QgsProjUtils::proj_pj_unique_ptr crs;

  switch ( static_cast< QgsCoordinateReferenceSystem::Format >( mFormatComboBox->currentData().toInt() ) )
  {
    case QgsCoordinateReferenceSystem::FormatWkt:
    {
      PROJ_STRING_LIST warnings = nullptr;
      PROJ_STRING_LIST grammerErrors = nullptr;
      crs.reset( proj_create_from_wkt( context, projDef.toLatin1().constData(), nullptr, &warnings, &grammerErrors ) );
      QStringList warningStrings;
      QStringList grammerStrings;
      for ( auto iter = warnings; iter && *iter; ++iter )
        warningStrings << QString( *iter );
      for ( auto iter = grammerErrors; iter && *iter; ++iter )
        grammerStrings << QString( *iter );
      proj_string_list_destroy( warnings );
      proj_string_list_destroy( grammerErrors );

      if ( crs )
      {
        QMessageBox::information( this, tr( "Custom Coordinate Reference System" ),
                                  tr( "This WKT projection definition is valid." ) );
      }
      else
      {
        QMessageBox::warning( this, tr( "Custom Coordinate Reference System" ),
                              tr( "This WKT projection definition is not valid:" ) + QStringLiteral( "\n\n" ) + warningStrings.join( '\n' ) + grammerStrings.join( '\n' ) );
      }
      break;
    }

    case QgsCoordinateReferenceSystem::FormatProj:
    {
      const QString projCrsString = projDef + ( projDef.contains( QStringLiteral( "+type=crs" ) ) ? QString() : QStringLiteral( " +type=crs" ) );
      crs.reset( proj_create( context, projCrsString.toLatin1().constData() ) );
      if ( crs )
      {
        QMessageBox::information( this, tr( "Custom Coordinate Reference System" ),
                                  tr( "This proj projection definition is valid." ) );
      }
      else
      {
        QMessageBox::warning( this, tr( "Custom Coordinate Reference System" ),
                              tr( "This proj projection definition is not valid:" ) + QStringLiteral( "\n\n" ) + projErrors.join( '\n' ) );
      }
      break;
    }
  }

  // reset logger to terminal output
  proj_log_func( context, nullptr, nullptr );
  proj_context_destroy( context );
  context = nullptr;
}

void QgsCustomProjectionDialog::formatChanged()
{
  QgsCoordinateReferenceSystem crs;
  QString newFormatString;
  switch ( static_cast< QgsCoordinateReferenceSystem::Format >( mFormatComboBox->currentData().toInt() ) )
  {
    case QgsCoordinateReferenceSystem::FormatProj:
    {
      crs.createFromWkt( multiLineWktToSingleLine( teParameters->toPlainText() ) );
      if ( crs.isValid() )
        newFormatString = crs.toProj();
      break;
    }

    case QgsCoordinateReferenceSystem::FormatWkt:
    {
      PJ_CONTEXT *pjContext = QgsProjContext::get();
      QString proj = teParameters->toPlainText();
      proj.replace( QLatin1String( "+type=crs" ), QString() );
      proj += QLatin1String( " +type=crs" );
      QgsProjUtils::proj_pj_unique_ptr crs( proj_create( QgsProjContext::get(), proj.toLatin1().constData() ) );
      if ( crs )
      {
        const QByteArray multiLineOption = QStringLiteral( "MULTILINE=YES" ).toLocal8Bit();
        const char *const options[] = {multiLineOption.constData(), nullptr};
        newFormatString = QString( proj_as_wkt( pjContext, crs.get(), PJ_WKT2_2019, options ) );
      }
      break;
    }
  }
  if ( !newFormatString.isEmpty() )
    teParameters->setPlainText( newFormatString );
}

void QgsCustomProjectionDialog::pbnCalculate_clicked()
{
  // We must check the prj def is valid!
  PJ_CONTEXT *pContext = QgsProjContext::get();
  QString projDef = teParameters->toPlainText();
  QgsDebugMsgLevel( QStringLiteral( "Proj: %1" ).arg( projDef ), 3 );

  // Get the WGS84 coordinates
  bool okN, okE;
  double latitude = northWGS84->text().toDouble( &okN );
  double longitude = eastWGS84->text().toDouble( &okE );

  if ( !okN || !okE )
  {
    QMessageBox::warning( this, tr( "Custom Coordinate Reference System" ),
                          tr( "Northing and Easting must be in decimal form." ) );
    projectedX->clear();
    projectedY->clear();
    return;
  }

  if ( static_cast< QgsCoordinateReferenceSystem::Format >( mFormatComboBox->currentData().toInt() ) == QgsCoordinateReferenceSystem::FormatProj )
    projDef = projDef + ( projDef.contains( QStringLiteral( "+type=crs" ) ) ? QString() : QStringLiteral( " +type=crs" ) );
  QgsProjUtils::proj_pj_unique_ptr res( proj_create_crs_to_crs( pContext, "EPSG:4326", projDef.toUtf8(), nullptr ) );
  if ( !res )
  {
    QMessageBox::warning( this, tr( "Custom Coordinate Reference System" ),
                          tr( "This CRS projection definition is not valid." ) );
    projectedX->clear();
    projectedY->clear();
    return;
  }

  // careful -- proj 6 respects CRS axis, so we've got latitude/longitude flowing in, and ....?? coming out?
  proj_trans_generic( res.get(), PJ_FWD,
                      &latitude, sizeof( double ), 1,
                      &longitude, sizeof( double ), 1,
                      nullptr, sizeof( double ), 0,
                      nullptr, sizeof( double ), 0 );
  int projResult = proj_errno( res.get() );

  if ( projResult != 0 )
  {
    projectedX->setText( tr( "Error" ) );
    projectedY->setText( tr( "Error" ) );
    QgsDebugMsg( proj_errno_string( projResult ) );
  }
  else
  {
    QString tmp;

    int precision = 4;
    bool isLatLong = false;

    isLatLong = QgsProjUtils::usesAngularUnit( projDef );
    if ( isLatLong )
    {
      precision = 7;
    }

    tmp = QLocale().toString( longitude, 'f', precision );
    projectedX->setText( tmp );
    tmp = QLocale().toString( latitude, 'f', precision );
    projectedY->setText( tmp );
  }
}

void QgsCustomProjectionDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_projections/working_with_projections.html" ) );
}

QString QgsCustomProjectionDialog::multiLineWktToSingleLine( const QString &wkt )
{
  QString res = wkt;
  QRegularExpression re( QStringLiteral( "\\s*\\n\\s*" ) );
  re.setPatternOptions( QRegularExpression::MultilineOption );
  res.replace( re, QString() );
  return res;
}

