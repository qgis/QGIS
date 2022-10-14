/***************************************************************************
  qgsrasterattributetablewidget.cpp - QgsRasterAttributeTableWidget

 ---------------------
 begin                : 6.10.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsrasterattributetablewidget.h"
#include "qgsrasterattributetable.h"
#include "qgsrasterlayer.h"
#include "qgsapplication.h"
#include "qgsmessagebar.h"
#include "qgsrasterattributetableaddcolumndialog.h"
#include "qgscolorbutton.h"
#include "qgsgradientcolorrampdialog.h"
#include "qgspalettedrasterrenderer.h"
#include "qgssinglebandpseudocolorrenderer.h"
#include "qgsrastershader.h"

#include <QToolBar>
#include <QAction>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <QFileDialog>

QgsRasterAttributeTableWidget::QgsRasterAttributeTableWidget( QWidget *parent, QgsRasterLayer *rasterLayer, const int bandNumber )
  : QWidget( parent )
  , mRasterLayer( rasterLayer )
{
  setupUi( this );

  // Create the toolbar
  QToolBar *editToolBar = new QToolBar( this );

  mActionToggleEditing = new QAction( QgsApplication::getThemeIcon( "/mActionEditTable.svg" ), tr( "&Edit Attribute Table" ), editToolBar );
  mActionToggleEditing->setCheckable( true );
  connect( mActionToggleEditing, &QAction::triggered, this, &QgsRasterAttributeTableWidget::setEditable );
  editToolBar->addAction( mActionToggleEditing );

  mActionAddColumn = new QAction( QgsApplication::getThemeIcon( "/mActionNewAttribute.svg" ), tr( "Add &Column" ), editToolBar );
  connect( mActionAddColumn, &QAction::triggered, this, &QgsRasterAttributeTableWidget::addColumn );
  editToolBar->addAction( mActionAddColumn );

  mActionAddRow = new QAction( QgsApplication::getThemeIcon( "/mActionNewTableRow.svg" ), tr( "&Add Row" ), editToolBar );
  connect( mActionAddRow, &QAction::triggered, this, &QgsRasterAttributeTableWidget::addRow );
  editToolBar->addAction( mActionAddRow );

  mActionRemoveRow = new QAction( QgsApplication::getThemeIcon( "/mActionRemoveSelectedFeature.svg" ), tr( "Remove Row" ), editToolBar );
  connect( mActionRemoveRow, &QAction::triggered, this, &QgsRasterAttributeTableWidget::removeRow );
  editToolBar->addAction( mActionRemoveRow );

  mActionRemoveColumn = new QAction( QgsApplication::getThemeIcon( "/mActionDeleteAttribute.svg" ), tr( "Remove Column" ), editToolBar );
  connect( mActionRemoveColumn, &QAction::triggered, this, &QgsRasterAttributeTableWidget::removeColumn );
  editToolBar->addAction( mActionRemoveColumn );

  mActionSaveChanges = new QAction( QgsApplication::getThemeIcon( "/mActionSaveAllEdits.svg" ), tr( "&Save Changes" ), editToolBar );
  connect( mActionSaveChanges, &QAction::triggered, this, &QgsRasterAttributeTableWidget::saveChanges );
  editToolBar->addAction( mActionSaveChanges );

  layout()->setMenuBar( editToolBar );

  connect( mClassifyButton, &QPushButton::clicked, this, &QgsRasterAttributeTableWidget::classify );

  mProxyModel = new QSortFilterProxyModel( this );

  mRATView->setModel( mProxyModel );

  connect( mRATView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsRasterAttributeTableWidget::updateButtons );

  if ( rasterLayer )
  {
    init( bandNumber );
  }
}

void QgsRasterAttributeTableWidget::setRasterLayer( QgsRasterLayer *rasterLayer, const int bandNumber )
{
  mRasterLayer = rasterLayer;
  init( bandNumber );
}

bool QgsRasterAttributeTableWidget::isDirty() const
{
  return mAttributeTable && mAttributeTable->isDirty();
}

void QgsRasterAttributeTableWidget::init( int bandNumber )
{

  disconnect( mRasterBandsComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsRasterAttributeTableWidget::bandChanged );
  mAttributeTable = nullptr;
  mCurrentBand = 0;
  mRasterBandsComboBox->clear();
  mClassifyComboBox->clear();

  QList<int> availableRats;

  if ( mRasterLayer )
  {
    for ( int checkBandNumber = 1; checkBandNumber <= mRasterLayer->bandCount(); ++checkBandNumber )
    {
      // Search for RATs
      if ( mRasterLayer->attributeTable( checkBandNumber ) )
      {
        mRasterBandsComboBox->addItem( mRasterLayer->bandName( checkBandNumber ), checkBandNumber );
        availableRats.push_back( checkBandNumber );
      }
    }

    if ( !availableRats.isEmpty() )
    {
      if ( availableRats.contains( bandNumber ) )
      {
        mCurrentBand = bandNumber;
        mAttributeTable = mRasterLayer->attributeTable( mCurrentBand );
        mRasterBandsComboBox->setCurrentIndex( availableRats.indexOf( mCurrentBand ) );
      }
      else if ( ! availableRats.isEmpty() )
      {
        mCurrentBand = availableRats.first();
        mAttributeTable = mRasterLayer->attributeTable( mCurrentBand );
        mRasterBandsComboBox->setCurrentIndex( availableRats.indexOf( mCurrentBand ) );
      }
    }
  }

  if ( mAttributeTable )
  {
    mModel.reset( new QgsRasterAttributeTableModel( mAttributeTable ) );
    mModel->setEditable( mEditable );
    connect( mModel.get(), &QgsRasterAttributeTableModel::dataChanged, this, [ = ]( const QModelIndex &, const QModelIndex &, const QVector<int> & )
    {
      updateButtons();
    } );

    static_cast<QSortFilterProxyModel *>( mRATView->model() )->setSourceModel( mModel.get() );

    const QList<QgsRasterAttributeTable::Field> tableFields { mAttributeTable->fields() };
    int fieldIdx { 0 };
    const QHash<Qgis::RasterAttributeTableFieldUsage, QgsRasterAttributeTable::UsageInformation> usageInfo { QgsRasterAttributeTable::usageInformation() };
    for ( const QgsRasterAttributeTable::Field &f : std::as_const( tableFields ) )
    {
      if ( usageInfo[f.usage].maybeClass )
      {
        mClassifyComboBox->addItem( QgsFields::iconForFieldType( f.type ), f.name, QVariant( fieldIdx ) );
      }
      fieldIdx++;
    }

    if ( mAttributeTable->hasColor() )
    {
      if ( mAttributeTable->usages().contains( Qgis::RasterAttributeTableFieldUsage::Alpha ) )
      {
        mRATView->setItemDelegateForColumn( mAttributeTable->fields().count( ), new ColorAlphaDelegate( mRATView ) );
      }
      else
      {
        mRATView->setItemDelegateForColumn( mAttributeTable->fields().count( ), new ColorDelegate( mRATView ) );
      }
    }
    else if ( mAttributeTable->hasRamp() )
    {
      if ( mAttributeTable->usages().contains( Qgis::RasterAttributeTableFieldUsage::AlphaMin ) )
      {
        mRATView->setItemDelegateForColumn( mAttributeTable->fields().count( ), new ColorRampAlphaDelegate( mRATView ) );
      }
      else
      {
        mRATView->setItemDelegateForColumn( mAttributeTable->fields().count( ), new ColorRampDelegate( mRATView ) );
      }
    }
  }
  else
  {
    notify( tr( "No Attribute Tables Available" ), tr( "The raster layer has no associated attribute tables, you can create a new attribute table or load one from a VAT.DBF file." ), Qgis::Warning );
  }

  connect( mRasterBandsComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsRasterAttributeTableWidget::bandChanged );

  updateButtons();
}

void QgsRasterAttributeTableWidget::updateButtons()
{
  const bool enableEditingButtons { static_cast<bool>( mAttributeTable ) &&mEditable &&mRATView->selectionModel()->currentIndex().isValid() };
  mActionAddColumn->setEnabled( mEditable );
  mActionRemoveColumn->setEnabled( enableEditingButtons );
  mActionAddRow->setEnabled( enableEditingButtons );
  mActionRemoveRow->setEnabled( enableEditingButtons );
  mActionSaveChanges->setEnabled( mAttributeTable && mAttributeTable->isDirty() );
  mClassifyButton->setEnabled( mAttributeTable && mRasterLayer );
  mClassifyComboBox->setEnabled( mAttributeTable && mRasterLayer );
}

void QgsRasterAttributeTableWidget::setMessageBar( QgsMessageBar *bar )
{
  mMessageBar = bar;
}

void QgsRasterAttributeTableWidget::setEditable( bool editable )
{
  bool isDirty { false };

  for ( int bandNo = 1; bandNo <= mRasterLayer->bandCount(); ++bandNo )
  {
    if ( mRasterLayer->dataProvider()->attributeTable( bandNo ) && mRasterLayer->dataProvider()->attributeTable( bandNo )->isDirty() )
    {
      isDirty = true;
      break;
    }
  }
  if ( isDirty && QMessageBox::question( nullptr, tr( "Save Attribute Table" ), tr( "Attribute table contains unsaved changes, do you want to save the changes?" ) ) == QMessageBox::Yes )
  {
    saveChanges();
  }

  mEditable = editable;
  mModel->setEditable( editable );
  updateButtons();
}

void QgsRasterAttributeTableWidget::saveChanges()
{
  if ( mRasterLayer )
  {
    for ( int bandNo = 1; bandNo <= mRasterLayer->bandCount(); ++bandNo )
    {
      QgsRasterAttributeTable *attributeTable { mRasterLayer->dataProvider()->attributeTable( bandNo ) };
      if ( attributeTable && attributeTable->isDirty() )
      {
        QString errorMessage;
        QString newPath { attributeTable->filePath() };
        const bool nativeRatSupported = mRasterLayer->dataProvider()->providerCapabilities().testFlag( QgsRasterDataProvider::ProviderCapability::NativeRasterAttributeTable );
        bool saveToNative { false };

        if ( newPath.isEmpty() && ! nativeRatSupported )
        {
          newPath = QFileDialog::getOpenFileName( nullptr, tr( "Save Raster Attribute Table (band %1) To File" ).arg( bandNo ), QFile::exists( mRasterLayer->dataProvider()->dataSourceUri( ) ) ? mRasterLayer->dataProvider()->dataSourceUri( ) + ".vat.dbf" : QString(), QStringLiteral( "VAT DBF Files (*.vat.dbf)" ) );
          if ( newPath.isEmpty() )
          {
            // Aborted by user
            return;
          }
        }
        else if ( nativeRatSupported )
        {
          saveToNative = true;
        }

        bool writeSuccess { false };

        // Save to file
        if ( ! saveToNative && ! newPath.isEmpty() )
        {
          writeSuccess = attributeTable->writeToFile( attributeTable->filePath(), &errorMessage );
        }
        else if ( saveToNative )
        {
          writeSuccess = mRasterLayer->dataProvider()->writeNativeAttributeTable( &errorMessage );
        }

        if ( writeSuccess )
        {
          notify( tr( "Attribute Table Write Success" ), tr( "The raster attibute table has been successfully saved." ), Qgis::MessageLevel::Success );
        }
        else
        {
          notify( tr( "Attribute Table Write Error" ), errorMessage, Qgis::MessageLevel::Critical );
        }

        // Save to native saves RATs for all bands, no need to loop further.
        if ( saveToNative )
        {
          return;
        }
      }
    }
  }

  updateButtons();
}

void QgsRasterAttributeTableWidget::classify()
{

  if ( ! mAttributeTable )
  {
    notify( tr( "Classification Error" ), tr( "The raster attribute table is not set." ), Qgis::MessageLevel::Critical );
    return;
  }

  if ( ! mRasterLayer )
  {
    notify( tr( "Classification Error" ), tr( "The raster layer is not set." ), Qgis::MessageLevel::Critical );
    return;
  }

  if ( QMessageBox::question( nullptr, tr( "Apply Style From Attribute Table" ), tr( "The existing style for the raster will be replaced by a new style from the attribute table and any unsaved changes to the current style will be lost, do you want to proceed?" ) ) == QMessageBox::Yes )
  {

    if ( QgsRasterRenderer *renderer = mAttributeTable->createRenderer( mRasterLayer->dataProvider(), mCurrentBand, mClassifyComboBox->currentData().toInt() ) )
    {
      mRasterLayer->setRenderer( renderer );
      mRasterLayer->triggerRepaint( );
    }
    else
    {
      notify( tr( "Classification Error" ), tr( "The classification returned no classes." ), Qgis::MessageLevel::Critical );
    }
  }
}

void QgsRasterAttributeTableWidget::addColumn()
{
  if ( mAttributeTable )
  {
    QgsRasterAttributeTableAddColumnDialog dlg { mAttributeTable };
    if ( dlg.exec() == QDialog::Accepted )
    {
      QString errorMessage;
      if ( dlg.isColor() )
      {
        if ( ! mModel->insertColor( dlg.position(), &errorMessage ) )
        {
          notify( tr( "Error adding color column" ), errorMessage,  Qgis::MessageLevel::Critical );
        }
      }
      else if ( dlg.isRamp() )
      {
        if ( ! mModel->insertRamp( dlg.position(), &errorMessage ) )
        {
          notify( tr( "Error adding color ramp column" ), errorMessage,  Qgis::MessageLevel::Critical );
        }
      }
      else
      {
        if ( ! mModel->insertField( dlg.position(), dlg.name(), dlg.usage(), dlg.type(), &errorMessage ) )
        {
          notify( tr( "Error adding new column" ), errorMessage,  Qgis::MessageLevel::Critical );
        }
      }
    }
  }
}

void QgsRasterAttributeTableWidget::removeColumn()
{
  const QModelIndex currentIndex { mProxyModel->mapToSource( mRATView->selectionModel()->currentIndex() ) };
  if ( mAttributeTable && currentIndex.isValid() && currentIndex.column() < mAttributeTable->fields().count() )
  {
    if ( QMessageBox::question( nullptr, tr( "Remove Column" ), tr( "Do you want to remove the selected column? This action cannot be undone." ) ) == QMessageBox::Yes )
    {
      QString errorMessage;
      if ( ! mModel->removeField( currentIndex.column(), &errorMessage ) )
      {
        notify( tr( "Error removing column" ), errorMessage,  Qgis::MessageLevel::Critical );
      }
    }
  }
}

void QgsRasterAttributeTableWidget::addRow()
{
  if ( mAttributeTable )
  {
    bool result { true };
    QString errorMessage;
    QVariantList rowData;

    QList<QgsRasterAttributeTable::Field> fields { mAttributeTable->fields() };
    for ( const QgsRasterAttributeTable::Field &field : std::as_const( fields ) )
    {
      rowData.push_back( QVariant( field.type ) );
    }

    const QModelIndex currentIndex { mProxyModel->mapToSource( mRATView->selectionModel()->currentIndex() ) };
    if ( currentIndex.isValid() )
    {
      result = mModel->insertRow( currentIndex.row(), rowData, &errorMessage );
    }
    else
    {
      result = mModel->insertRow( mModel->rowCount( QModelIndex() ), rowData, &errorMessage );
    }
    if ( ! result )
    {
      notify( tr( "Error adding row" ), errorMessage,  Qgis::MessageLevel::Critical );
    }
  }
}

void QgsRasterAttributeTableWidget::removeRow()
{
  if ( mAttributeTable && mRATView->selectionModel()->currentIndex().isValid() )
  {
    if ( QMessageBox::question( nullptr, tr( "Remove Row" ), tr( "Do you want to remove the selected row? This action cannot be undone." ) ) == QMessageBox::Yes )
    {
      QString errorMessage;
      if ( ! mModel->removeRow( mProxyModel->mapToSource( mRATView->selectionModel()->currentIndex() ).row(), &errorMessage ) )
      {
        notify( tr( "Error removing row" ), errorMessage,  Qgis::MessageLevel::Critical );
      }
    }
  }
}

void QgsRasterAttributeTableWidget::bandChanged( const int index )
{
  const QVariant itemData = mRasterBandsComboBox->itemData( index );
  if ( itemData.isValid() )
  {
    init( itemData.toInt( ) );
  }
}

void QgsRasterAttributeTableWidget::notify( const QString &title, const QString &message, Qgis::MessageLevel level )
{
  if ( mMessageBar )
  {
    mMessageBar->pushMessage( message, level );
  }
  else
  {
    switch ( level )
    {
      case Qgis::MessageLevel::Info:
      case Qgis::MessageLevel::Success:
      case Qgis::MessageLevel::NoLevel:
      {
        QMessageBox::information( nullptr, title, message );
        break;
      }
      case Qgis::MessageLevel::Warning:
      {
        QMessageBox::warning( nullptr, title, message );
        break;
      }
      case Qgis::MessageLevel::Critical:
      {
        QMessageBox::critical( nullptr, title, message );
        break;
      }
    }
  }
}


///@cond private

QWidget *ColorDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &, const QModelIndex & ) const
{
  return new QgsColorButton( parent, tr( "Select Color" ) );
}

void ColorDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  const QColor color { index.data( Qt::ItemDataRole::EditRole ).value<QColor>() };
  static_cast<QgsColorButton *>( editor )->setColor( color );
}

void ColorDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  const QColor color { static_cast<QgsColorButton *>( editor )->color( ) };
  model->setData( index, color );
}

QWidget *ColorAlphaDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QWidget *editor { ColorDelegate::createEditor( parent, option, index ) };
  static_cast<QgsColorButton *>( editor )->setAllowOpacity( true );
  return editor;
}

QWidget *ColorRampDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &, const QModelIndex & ) const
{
  QgsGradientColorRampDialog *editor = new QgsGradientColorRampDialog{ QgsGradientColorRamp(), parent };
  return editor;
}

void ColorRampDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  const QgsGradientColorRamp ramp { qvariant_cast<QgsGradientColorRamp>( index.data( Qt::ItemDataRole::EditRole ) ) };
  static_cast<QgsGradientColorRampDialog *>( editor )->setRamp( ramp );
}

void ColorRampDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  model->setData( index, QVariant::fromValue( static_cast<QgsGradientColorRampDialog *>( editor )->ramp() ) );
}

void ColorRampDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  const QgsGradientColorRamp ramp { qvariant_cast<QgsGradientColorRamp>( index.data( Qt::ItemDataRole::EditRole ) ) };
  QLinearGradient gradient( QPointF( 0, 0 ), QPointF( 1, 0 ) );
  gradient.setCoordinateMode( QGradient::CoordinateMode::ObjectBoundingMode );
  gradient.setColorAt( 0, ramp.color1() );
  gradient.setColorAt( 1, ramp.color2() );
  const QRect r = option.rect.adjusted( 1, 1, -1, -1 );
  const QgsScopedQPainterState painterState( painter );
  painter->fillRect( r, QBrush{ gradient } );
}

QWidget *ColorRampAlphaDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QWidget *editor { ColorRampDelegate::createEditor( parent, option, index ) };
  // No opacity setting for ramps?
  return editor;
}

///@endcond

