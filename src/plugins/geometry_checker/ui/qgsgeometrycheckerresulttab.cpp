/***************************************************************************
 *  qgsgeometrycheckerresulttab.cpp                                        *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QFileDialog>
#include <QGroupBox>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QPlainTextEdit>

#include "qgsgeometrycheckerresulttab.h"
#include "qgsgeometrycheckfixdialog.h"
#include "../qgsgeometrychecker.h"
#include "../checks/qgsgeometrycheck.h"
#include "../utils/qgsfeaturepool.h"
#include "qgsgeometry.h"
#include "qgisinterface.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerregistry.h"
#include "qgsproviderregistry.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorfilewriter.h"

QString QgsGeometryCheckerResultTab::sSettingsGroup = "/geometry_checker/default_fix_methods/";

QgsGeometryCheckerResultTab::QgsGeometryCheckerResultTab( QgisInterface* iface, QgsGeometryChecker* checker, QgsFeaturePool *featurePool, QTabWidget* tabWidget, QWidget* parent )
    : QWidget( parent )
    , mTabWidget( tabWidget )
    , mIface( iface )
    , mChecker( checker )
    , mFeaturePool( featurePool )
{
  ui.setupUi( this );
  mErrorCount = 0;
  mFixedCount = 0;
  mCloseable = true;
  mAttribTableDialog = nullptr;

  for ( int i = 0, n = mFeaturePool->getLayer()->fields().count(); i < n; ++i )
  {
    ui.comboBoxMergeAttribute->addItem( mFeaturePool->getLayer()->fields().at( i ).name() );
  }

  connect( checker, SIGNAL( errorAdded( QgsGeometryCheckError* ) ), this, SLOT( addError( QgsGeometryCheckError* ) ) );
  connect( checker, SIGNAL( errorUpdated( QgsGeometryCheckError*, bool ) ), this, SLOT( updateError( QgsGeometryCheckError*, bool ) ) );
  connect( ui.comboBoxMergeAttribute, SIGNAL( currentIndexChanged( int ) ), checker, SLOT( setMergeAttributeIndex( int ) ) );
  connect( ui.tableWidgetErrors->selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), this, SLOT( onSelectionChanged( QItemSelection, QItemSelection ) ) );
  connect( ui.buttonGroupSelectAction, SIGNAL( buttonClicked( int ) ), this, SLOT( highlightErrors() ) );
  connect( ui.pushButtonOpenAttributeTable, SIGNAL( clicked() ), this, SLOT( openAttributeTable() ) );
  connect( ui.pushButtonFixWithDefault, SIGNAL( clicked() ), this, SLOT( fixErrorsWithDefault() ) );
  connect( ui.pushButtonFixWithPrompt, SIGNAL( clicked() ), this, SLOT( fixErrorsWithPrompt() ) );
  connect( ui.pushButtonErrorResolutionSettings, SIGNAL( clicked() ), this, SLOT( setDefaultResolutionMethods() ) );
  connect( ui.checkBoxHighlight, SIGNAL( clicked() ), this, SLOT( highlightErrors() ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersWillBeRemoved( QStringList ) ), this, SLOT( checkRemovedLayer( QStringList ) ) );
  connect( ui.pushButtonExport, SIGNAL( clicked() ), this, SLOT( exportErrors() ) );

  if (( mFeaturePool->getLayer()->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeGeometries ) == 0 )
  {
    ui.pushButtonFixWithDefault->setEnabled( false );
    ui.pushButtonFixWithPrompt->setEnabled( false );
  }

  ui.progressBarFixErrors->setVisible( false );
  ui.tableWidgetErrors->horizontalHeader()->setSortIndicator( 0, Qt::AscendingOrder );
  // Not sure why, but this is needed...
  ui.tableWidgetErrors->setSortingEnabled( true );
  ui.tableWidgetErrors->setSortingEnabled( false );
}

QgsGeometryCheckerResultTab::~QgsGeometryCheckerResultTab()
{
  if ( mFeaturePool->getLayer() )
    mFeaturePool->getLayer()->setReadOnly( false );
  delete mChecker;
  delete mFeaturePool;
  qDeleteAll( mCurrentRubberBands );
}

void QgsGeometryCheckerResultTab::finalize()
{
  ui.tableWidgetErrors->setSortingEnabled( true );
  if ( !mChecker->getMessages().isEmpty() )
  {
    QDialog dialog;
    dialog.setLayout( new QVBoxLayout() );
    dialog.layout()->addWidget( new QLabel( tr( "The following checks reported errors:" ) ) );
    dialog.layout()->addWidget( new QPlainTextEdit( mChecker->getMessages().join( "\n" ) ) );
    QDialogButtonBox* bbox = new QDialogButtonBox( QDialogButtonBox::Close, Qt::Horizontal );
    dialog.layout()->addWidget( bbox );
    connect( bbox, SIGNAL( accepted() ), &dialog, SLOT( accept() ) );
    connect( bbox, SIGNAL( rejected() ), &dialog, SLOT( reject() ) );
    dialog.setWindowTitle( tr( "Check errors occurred" ) );
    dialog.exec();
  }
}

void QgsGeometryCheckerResultTab::addError( QgsGeometryCheckError *error )
{
  int row = ui.tableWidgetErrors->rowCount();
  int prec = 7 - std::floor( qMax( 0., std::log10( qMax( error->location().x(), error->location().y() ) ) ) );
  QString posStr = QString( "%1, %2" ).arg( error->location().x(), 0, 'f', prec ).arg( error->location().y(), 0, 'f', prec );
  double layerToMap = mIface->mapCanvas()->mapSettings().layerToMapUnits( mFeaturePool->getLayer() );
  QVariant value;
  if ( error->valueType() == QgsGeometryCheckError::ValueLength )
  {
    value = QVariant::fromValue( error->value().toDouble() * layerToMap );
  }
  else if ( error->valueType() == QgsGeometryCheckError::ValueArea )
  {
    value = QVariant::fromValue( error->value().toDouble() * layerToMap * layerToMap );
  }
  else
  {
    value = error->value();
  }
  ui.tableWidgetErrors->insertRow( row );
  QTableWidgetItem* idItem = new QTableWidgetItem();
  idItem->setData( Qt::EditRole, error->featureId() != FEATUREID_NULL ? QVariant( error->featureId() ) : QVariant() );
  ui.tableWidgetErrors->setItem( row, 0, idItem );
  ui.tableWidgetErrors->setItem( row, 1, new QTableWidgetItem( error->description() ) );
  ui.tableWidgetErrors->setItem( row, 2, new QTableWidgetItem( posStr ) );
  QTableWidgetItem* valueItem = new QTableWidgetItem();
  valueItem->setData( Qt::EditRole, value );
  ui.tableWidgetErrors->setItem( row, 3, valueItem );
  ui.tableWidgetErrors->setItem( row, 4, new QTableWidgetItem( "" ) );
  ui.tableWidgetErrors->item( row, 0 )->setData( Qt::UserRole, QVariant::fromValue( error ) );
  ++mErrorCount;
  ui.labelErrorCount->setText( tr( "Total errors: %1, fixed errors: %2" ).arg( mErrorCount ).arg( mFixedCount ) );
  mStatistics.newErrors.insert( error );
  mErrorMap.insert( error, QPersistentModelIndex( ui.tableWidgetErrors->model()->index( row, 0 ) ) );
}

void QgsGeometryCheckerResultTab::updateError( QgsGeometryCheckError *error, bool statusChanged )
{
  if ( !mErrorMap.contains( error ) )
  {
    return;
  }
  // Disable sorting to prevent crashes: if i.e. sorting by col 0, as soon as the item(row, 0)
  // is set, the row is potentially moved due to sorting, and subsequent item(row, col) reference wrong
  // item
  ui.tableWidgetErrors->setSortingEnabled( false );

  int row = mErrorMap.value( error ).row();
  int prec = 7 - std::floor( qMax( 0., std::log10( qMax( error->location().x(), error->location().y() ) ) ) );
  QString posStr = QString( "%1, %2" ).arg( error->location().x(), 0, 'f', prec ).arg( error->location().y(), 0, 'f', prec );
  double layerToMap = mIface->mapCanvas()->mapSettings().layerToMapUnits( mFeaturePool->getLayer() );
  QVariant value;
  if ( error->valueType() == QgsGeometryCheckError::ValueLength )
  {
    value = QVariant::fromValue( error->value().toDouble() * layerToMap );
  }
  else if ( error->valueType() == QgsGeometryCheckError::ValueArea )
  {
    value = QVariant::fromValue( error->value().toDouble() * layerToMap * layerToMap );
  }
  else
  {
    value = error->value();
  }
  ui.tableWidgetErrors->item( row, 2 )->setText( posStr );
  ui.tableWidgetErrors->item( row, 3 )->setData( Qt::EditRole, value );
  if ( error->status() == QgsGeometryCheckError::StatusFixed )
  {
    setRowStatus( row, Qt::green, tr( "Fixed: %1" ).arg( error->resolutionMessage() ), true );
    ++mFixedCount;
    if ( statusChanged )
    {
      mStatistics.fixedErrors.insert( error );
    }
  }
  else if ( error->status() == QgsGeometryCheckError::StatusFixFailed )
  {
    setRowStatus( row, Qt::red, tr( "Fix failed: %1" ).arg( error->resolutionMessage() ), true );
    if ( statusChanged )
    {
      mStatistics.failedErrors.insert( error );
    }
  }
  else if ( error->status() == QgsGeometryCheckError::StatusObsolete )
  {
    ui.tableWidgetErrors->setRowHidden( row, true );
//    setRowStatus( row, Qt::gray, tr( "Obsolete" ), false );
    --mErrorCount;
    // If error was new, don't report it as obsolete since the user never got to see the new error anyways
    if ( statusChanged && !mStatistics.newErrors.remove( error ) )
    {
      mStatistics.obsoleteErrors.insert( error );
    }
  }
  ui.labelErrorCount->setText( tr( "Total errors: %1, fixed errors: %2" ).arg( mErrorCount ).arg( mFixedCount ) );

  ui.tableWidgetErrors->setSortingEnabled( true );
}

void QgsGeometryCheckerResultTab::exportErrors()
{
  QString initialdir;
  QDir dir = QFileInfo( mFeaturePool->getLayer()->dataProvider()->dataSourceUri() ).dir();
  if ( dir.exists() )
  {
    initialdir = dir.absolutePath();
  }

  QString file = QFileDialog::getSaveFileName( this, tr( "Select Output File" ), initialdir, tr( "ESRI Shapefile (*.shp);;" ) );
  if ( file.isEmpty() )
  {
    return;
  }
  if ( !exportErrorsDo( file ) )
  {
    QMessageBox::critical( this, tr( "Error" ), tr( "Failed to export errors to shapefile." ) );
  }
}

bool QgsGeometryCheckerResultTab::exportErrorsDo( const QString& file )
{
  QList< QPair<QString, QString> > attributes;
  attributes.append( qMakePair( QString( "FeatureID" ), QString( "String;10;" ) ) );
  attributes.append( qMakePair( QString( "ErrorDesc" ), QString( "String;80;" ) ) );

  QLibrary ogrLib( QgsProviderRegistry::instance()->library( "ogr" ) );
  if ( !ogrLib.load() )
  {
    return false;
  }
  typedef bool ( *createEmptyDataSourceProc )( const QString&, const QString&, const QString&, QGis::WkbType, const QList< QPair<QString, QString> >&, const QgsCoordinateReferenceSystem * );
  createEmptyDataSourceProc createEmptyDataSource = ( createEmptyDataSourceProc ) cast_to_fptr( ogrLib.resolve( "createEmptyDataSource" ) );
  if ( !createEmptyDataSource )
  {
    return false;
  }
  if ( !createEmptyDataSource( file, "ESRI Shapefile", mFeaturePool->getLayer()->dataProvider()->encoding(), QGis::WKBPoint, attributes, &mFeaturePool->getLayer()->crs() ) )
  {
    return false;
  }
  QgsVectorLayer* layer = new QgsVectorLayer( file, QFileInfo( file ).baseName(), "ogr" );
  if ( !layer->isValid() )
  {
    delete layer;
    return false;
  }

  int fieldFeatureId = layer->fieldNameIndex( "FeatureID" );
  int fieldErrDesc = layer->fieldNameIndex( "ErrorDesc" );
  for ( int row = 0, nRows = ui.tableWidgetErrors->rowCount(); row < nRows; ++row )
  {
    QgsGeometryCheckError* error = ui.tableWidgetErrors->item( row, 0 )->data( Qt::UserRole ).value<QgsGeometryCheckError*>();

    QgsFeature f( layer->fields() );
    f.setAttribute( fieldFeatureId, error->featureId() );
    f.setAttribute( fieldErrDesc, error->description() );
    f.setGeometry( new QgsGeometry( error->location().clone() ) );
    layer->dataProvider()->addFeatures( QgsFeatureList() << f );
  }

  // Remove existing layer with same uri
  QStringList toRemove;
  Q_FOREACH ( QgsMapLayer* maplayer, QgsMapLayerRegistry::instance()->mapLayers() )
  {
    if ( dynamic_cast<QgsVectorLayer*>( maplayer ) &&
         static_cast<QgsVectorLayer*>( maplayer )->dataProvider()->dataSourceUri() == layer->dataProvider()->dataSourceUri() )
    {
      toRemove.append( maplayer->id() );
    }
  }
  if ( !toRemove.isEmpty() )
  {
    QgsMapLayerRegistry::instance()->removeMapLayers( toRemove );
  }

  QgsMapLayerRegistry::instance()->addMapLayers( QList<QgsMapLayer*>() << layer );
  return true;
}

void QgsGeometryCheckerResultTab::highlightError( QgsGeometryCheckError* error )
{
  if ( !mErrorMap.contains( error ) )
  {
    return;
  }
  int row = mErrorMap.value( error ).row();
  ui.tableWidgetErrors->setCurrentIndex( ui.tableWidgetErrors->model()->index( row, 0 ) );
  highlightErrors( true );
}

void QgsGeometryCheckerResultTab::highlightErrors( bool current )
{
  qDeleteAll( mCurrentRubberBands );
  mCurrentRubberBands.clear();

  QList<QTableWidgetItem*> items;
  QVector<QgsPoint> errorPositions;
  QgsRectangle totextent;

  if ( current )
  {
    items.append( ui.tableWidgetErrors->currentItem() );
  }
  else
  {
    items.append( ui.tableWidgetErrors->selectedItems() );
  }
  Q_FOREACH ( QTableWidgetItem* item, items )
  {
    QgsGeometryCheckError* error = ui.tableWidgetErrors->item( item->row(), 0 )->data( Qt::UserRole ).value<QgsGeometryCheckError*>();

    QgsAbstractGeometryV2* geometry = error->geometry();
    if ( ui.checkBoxHighlight->isChecked() && geometry )
    {
      QgsRubberBand* featureRubberBand = new QgsRubberBand( mIface->mapCanvas() );
      QgsGeometry geom( geometry->clone() );
      featureRubberBand->addGeometry( &geom, mFeaturePool->getLayer() );
      featureRubberBand->setWidth( 5 );
      featureRubberBand->setColor( Qt::yellow );
      mCurrentRubberBands.append( featureRubberBand );
    }
    else
    {
      // QgsGeometry above takes ownership of geometry and deletes it when it goes out of scope
      delete geometry;
      geometry = nullptr;
    }

    if ( ui.radioButtonError->isChecked() || current || error->status() == QgsGeometryCheckError::StatusFixed )
    {
      QgsRubberBand* pointRubberBand = new QgsRubberBand( mIface->mapCanvas(), QGis::Point );
      QgsPoint pos = mIface->mapCanvas()->mapSettings().layerToMapCoordinates( mFeaturePool->getLayer(), QgsPoint( error->location().x(), error->location().y() ) );
      pointRubberBand->addPoint( pos );
      pointRubberBand->setWidth( 20 );
      pointRubberBand->setColor( Qt::red );
      mCurrentRubberBands.append( pointRubberBand );
      errorPositions.append( pos );
    }
    else if ( ui.radioButtonFeature->isChecked() && geometry )
    {
      QgsRectangle geomextent = mIface->mapCanvas()->mapSettings().layerExtentToOutputExtent( mFeaturePool->getLayer(), geometry->boundingBox() );
      if ( totextent.isEmpty() )
      {
        totextent = geomextent;
      }
      else
      {
        totextent.unionRect( geomextent );
      }
    }
  }

  // If error positions positions are marked, pan to the center of all positions,
  // and zoom out if necessary to make all points fit.
  if ( !errorPositions.isEmpty() )
  {
    double cx = 0., cy = 0.;
    QgsRectangle pointExtent( errorPositions.first(), errorPositions.first() );
    Q_FOREACH ( const QgsPoint& p, errorPositions )
    {
      cx += p.x();
      cy += p.y();
      pointExtent.include( p );
    }
    QgsPoint center = QgsPoint( cx / errorPositions.size(), cy / errorPositions.size() );
    if ( totextent.isEmpty() )
    {
      QgsRectangle extent = mIface->mapCanvas()->extent();
      QgsVector diff = center - extent.center();
      extent.setXMinimum( extent.xMinimum() + diff.x() );
      extent.setXMaximum( extent.xMaximum() + diff.x() );
      extent.setYMinimum( extent.yMinimum() + diff.y() );
      extent.setYMaximum( extent.yMaximum() + diff.y() );
      extent.unionRect( pointExtent );
      totextent = extent;
    }
    else
    {
      totextent.unionRect( pointExtent );
    }
  }

  mIface->mapCanvas()->setExtent( totextent );
  mIface->mapCanvas()->refresh();
}

void QgsGeometryCheckerResultTab::onSelectionChanged( const QItemSelection &newSel, const QItemSelection &/*oldSel*/ )
{
  QModelIndex idx = ui.tableWidgetErrors->currentIndex();
  if ( idx.isValid() && !ui.tableWidgetErrors->isRowHidden( idx.row() )  && ui.tableWidgetErrors->selectionModel()->selectedIndexes().contains( idx ) )
  {
    highlightErrors();
  }
  else
  {
    qDeleteAll( mCurrentRubberBands );
    mCurrentRubberBands.clear();
  }
  ui.pushButtonOpenAttributeTable->setEnabled( !newSel.isEmpty() );
}

void QgsGeometryCheckerResultTab::openAttributeTable()
{
  QSet<int> ids;
  Q_FOREACH ( QModelIndex idx, ui.tableWidgetErrors->selectionModel()->selectedRows() )
  {
    QgsFeatureId id = ui.tableWidgetErrors->item( idx.row(), 0 )->data( Qt::UserRole ).value<QgsGeometryCheckError*>()->featureId();
    if ( id >= 0 )
    {
      ids.insert( id );
    }
  }
  if ( ids.isEmpty() )
  {
    return;
  }
  QStringList expr;
  Q_FOREACH ( int id, ids )
  {
    expr.append( QString( "$id = %1 " ).arg( id ) );
  }
  if ( mAttribTableDialog )
  {
    disconnect( mAttribTableDialog, SIGNAL( destroyed() ), this, SLOT( clearAttribTableDialog() ) );
    mAttribTableDialog->close();
  }
  mAttribTableDialog = mIface->showAttributeTable( mFeaturePool->getLayer(), expr.join( " or " ) );
  connect( mAttribTableDialog, SIGNAL( destroyed() ), this, SLOT( clearAttribTableDialog() ) );
}

void QgsGeometryCheckerResultTab::fixErrors( bool prompt )
{

  /** Collect errors to fix **/
  QModelIndexList rows = ui.tableWidgetErrors->selectionModel()->selectedRows();
  if ( rows.isEmpty() )
  {
    ui.tableWidgetErrors->selectAll();
    rows = ui.tableWidgetErrors->selectionModel()->selectedRows();
  }
  QList<QgsGeometryCheckError*> errors;
  Q_FOREACH ( const QModelIndex& index, rows )
  {
    QgsGeometryCheckError* error = ui.tableWidgetErrors->item( index.row(), 0 )->data( Qt::UserRole ).value<QgsGeometryCheckError*>();
    if ( error->status() < QgsGeometryCheckError::StatusFixed )
    {
      errors.append( error );
    }
  }
  if ( errors.isEmpty() )
  {
    return;
  }
  if ( QMessageBox::Yes != QMessageBox::question( this, tr( "Fix errors?" ), tr( "Do you want to fix %1 errors?" ).arg( errors.size() ), QMessageBox::Yes, QMessageBox::No ) )
  {
    return;
  }

  /** Reset statistics, clear rubberbands **/
  mStatistics = QgsGeometryCheckerFixSummaryDialog::Statistics();
  qDeleteAll( mCurrentRubberBands );
  mCurrentRubberBands.clear();


  /** Fix errors **/
  mCloseable = false;
  if ( prompt )
  {
    QgsGeometryCheckerFixDialog fixdialog( mChecker, errors, mIface, mIface->mainWindow() );
    QEventLoop loop;
    connect( &fixdialog, SIGNAL( currentErrorChanged( QgsGeometryCheckError* ) ), this, SLOT( highlightError( QgsGeometryCheckError* ) ) );
    connect( &fixdialog, SIGNAL( finished( int ) ), &loop, SLOT( quit() ) );
    fixdialog.show();
    parentWidget()->parentWidget()->parentWidget()->setEnabled( false );
    loop.exec();
    parentWidget()->parentWidget()->parentWidget()->setEnabled( true );
  }
  else
  {
    setCursor( Qt::WaitCursor );
    ui.progressBarFixErrors->setVisible( true );
    ui.progressBarFixErrors->setRange( 0, errors.size() );

    Q_FOREACH ( QgsGeometryCheckError* error, errors )
    {
      int fixMethod = QSettings().value( sSettingsGroup + error->check()->errorName(), QVariant::fromValue<int>( 0 ) ).toInt();
      mChecker->fixError( error, fixMethod );
      ui.progressBarFixErrors->setValue( ui.progressBarFixErrors->value() + 1 );
      QApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
    }

    ui.progressBarFixErrors->setVisible( false );
    unsetCursor();
  }

  mIface->mapCanvas()->refresh();

  if ( mStatistics.itemCount() > 0 )
  {
    QgsGeometryCheckerFixSummaryDialog summarydialog( mIface, mFeaturePool->getLayer(), mStatistics, mChecker->getMessages(), mIface->mainWindow() );
    QEventLoop loop;
    connect( &summarydialog, SIGNAL( errorSelected( QgsGeometryCheckError* ) ), this, SLOT( highlightError( QgsGeometryCheckError* ) ) );
    connect( &summarydialog, SIGNAL( finished( int ) ), &loop, SLOT( quit() ) );
    summarydialog.show();
    parentWidget()->parentWidget()->parentWidget()->setEnabled( false );
    loop.exec();
    parentWidget()->parentWidget()->parentWidget()->setEnabled( true );
  }
  mCloseable = true;
}

void QgsGeometryCheckerResultTab::setRowStatus( int row, const QColor& color, const QString& message, bool selectable )
{
  for ( int col = 0, nCols = ui.tableWidgetErrors->columnCount(); col < nCols; ++col )
  {
    QTableWidgetItem* item = ui.tableWidgetErrors->item( row, col );
    item->setBackground( color );
    if ( !selectable )
    {
      item->setFlags( item->flags() & ~Qt::ItemIsSelectable );
      item->setForeground( Qt::lightGray );
    }
  }
  ui.tableWidgetErrors->item( row, 4 )->setText( message );
}

void QgsGeometryCheckerResultTab::setDefaultResolutionMethods()
{
  QDialog dialog( this );
  dialog.setWindowTitle( tr( "Set Error Resolutions" ) );

  QVBoxLayout* layout = new QVBoxLayout( &dialog );

  QScrollArea* scrollArea = new QScrollArea( &dialog );
  scrollArea->setFrameShape( QFrame::NoFrame );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( scrollArea );

  QWidget* scrollAreaContents = new QWidget( scrollArea );
  QVBoxLayout* scrollAreaLayout = new QVBoxLayout( scrollAreaContents );

  Q_FOREACH ( const QgsGeometryCheck* check, mChecker->getChecks() )
  {
    QGroupBox* groupBox = new QGroupBox( scrollAreaContents );
    groupBox->setTitle( check->errorDescription() );
    groupBox->setFlat( true );

    QVBoxLayout* groupBoxLayout = new QVBoxLayout( groupBox );
    groupBoxLayout->setContentsMargins( 2, 0, 2, 2 );
    QButtonGroup* radioGroup = new QButtonGroup( groupBox );
    radioGroup->setProperty( "errorType", check->errorName() );
    int id = 0;
    int checkedId = QSettings().value( sSettingsGroup + check->errorName(), QVariant::fromValue<int>( 0 ) ).toInt();
    Q_FOREACH ( const QString& method, check->getResolutionMethods() )
    {
      QRadioButton* radio = new QRadioButton( method, groupBox );
      radio->setChecked( id == checkedId );
      groupBoxLayout->addWidget( radio );
      radioGroup->addButton( radio, id++ );
    }
    connect( radioGroup, SIGNAL( buttonClicked( int ) ), this, SLOT( storeDefaultResolutionMethod( int ) ) );

    scrollAreaLayout->addWidget( groupBox );
  }
  scrollArea->setWidget( scrollAreaContents );

  QDialogButtonBox* buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok, Qt::Horizontal, &dialog );
  connect( buttonBox, SIGNAL( accepted() ), &dialog, SLOT( accept() ) );
  layout->addWidget( buttonBox );
  dialog.exec();
}

void QgsGeometryCheckerResultTab::storeDefaultResolutionMethod( int id ) const
{
  QString errorType = qobject_cast<QButtonGroup*>( QObject::sender() )->property( "errorType" ).toString();
  QSettings().setValue( sSettingsGroup + errorType, id );
}

void QgsGeometryCheckerResultTab::checkRemovedLayer( const QStringList &ids )
{
  if ( ids.contains( mFeaturePool->getLayer()->id() ) && isEnabled() )
  {
    if ( mTabWidget->currentWidget() == this )
    {
      QMessageBox::critical( this, tr( "Layer removed" ), tr( "The layer has been removed." ) );
    }
    setEnabled( false );
    mFeaturePool->clearLayer();
  }
}
