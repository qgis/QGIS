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
#include "qgsrasterattributetableaddrowdialog.h"
#include "qgscolorbutton.h"
#include "qgsgradientcolorrampdialog.h"

#include <QToolBar>
#include <QAction>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <QFileDialog>

QgsRasterAttributeTableWidget::QgsRasterAttributeTableWidget( QWidget *parent, QgsRasterLayer *rasterLayer, const int bandNumber )
  : QgsPanelWidget( parent )
  , mRasterLayer( rasterLayer )
{
  setupUi( this );

  // Create the toolbar
  QToolBar *editToolBar = new QToolBar( this );
  editToolBar->setIconSize( QgsGuiUtils::iconSize( true ) );

  mActionToggleEditing = new QAction( QgsApplication::getThemeIcon( "/mActionEditTable.svg" ), tr( "&Edit Attribute Table" ), editToolBar );
  mActionToggleEditing->setCheckable( true );
  connect( mActionToggleEditing, &QAction::triggered, this, [ = ]( bool editable )
  {
    setEditable( editable );
  } );

  editToolBar->addAction( mActionToggleEditing );

  mActionAddColumn = new QAction( QgsApplication::getThemeIcon( "/mActionNewAttribute.svg" ), tr( "Add &Column…" ), editToolBar );
  connect( mActionAddColumn, &QAction::triggered, this, &QgsRasterAttributeTableWidget::addColumn );
  editToolBar->addAction( mActionAddColumn );

  mActionAddRow = new QAction( QgsApplication::getThemeIcon( "/mActionNewTableRow.svg" ), tr( "&Add Row…" ), editToolBar );
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
  return mAttributeTableBuffer && mAttributeTableBuffer->isDirty();
}

void QgsRasterAttributeTableWidget::init( int bandNumber )
{

  disconnect( mRasterBandsComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsRasterAttributeTableWidget::bandChanged );
  mAttributeTableBuffer = nullptr;
  mCurrentBand = 0;
  mRasterBandsComboBox->clear();

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
      }
      else if ( ! availableRats.isEmpty() )
      {
        mCurrentBand = availableRats.first();
      }

      mAttributeTableBuffer = std::make_unique<QgsRasterAttributeTable>( *mRasterLayer->attributeTable( mCurrentBand ) );
      mRasterBandsComboBox->setCurrentIndex( availableRats.indexOf( mCurrentBand ) );
    }
  }

  if ( mAttributeTableBuffer )
  {
    mModel.reset( new QgsRasterAttributeTableModel( mAttributeTableBuffer.get() ) );
    mModel->setEditable( mEditable );

    connect( mModel.get(), &QgsRasterAttributeTableModel::dataChanged, this, [ = ]( const QModelIndex &, const QModelIndex &, const QVector<int> & )
    {
      updateButtons();
    } );

    connect( mModel.get(), &QgsRasterAttributeTableModel::columnsInserted, this, [ = ]( const QModelIndex &, int, int )
    {
      setDelegates();
    } );

    connect( mModel.get(), &QgsRasterAttributeTableModel::columnsRemoved, this, [ = ]( const QModelIndex &, int, int )
    {
      setDelegates();
    } );

    static_cast<QSortFilterProxyModel *>( mRATView->model() )->setSourceModel( mModel.get() );
    setDelegates();
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
  const bool enableEditingButtons( static_cast<bool>( mAttributeTableBuffer ) && mEditable && mRATView->selectionModel()->currentIndex().isValid() );
  mActionToggleEditing->setChecked( mEditable );
  mActionAddColumn->setEnabled( mEditable );
  mActionRemoveColumn->setEnabled( enableEditingButtons );
  mActionAddRow->setEnabled( enableEditingButtons );
  mActionRemoveRow->setEnabled( enableEditingButtons );
  mActionSaveChanges->setEnabled( mAttributeTableBuffer && mAttributeTableBuffer->isDirty() );
  mClassifyButton->setEnabled( mAttributeTableBuffer && mRasterLayer );
  mClassifyComboBox->setEnabled( mAttributeTableBuffer && mRasterLayer );
}

void QgsRasterAttributeTableWidget::setDockMode( bool dockMode )
{
  QgsPanelWidget::setDockMode( dockMode );
  static_cast<QToolBar *>( layout()->menuBar() )->setIconSize( QgsGuiUtils::iconSize( dockMode ) );
}

void QgsRasterAttributeTableWidget::setMessageBar( QgsMessageBar *bar )
{
  mMessageBar = bar;
}

bool QgsRasterAttributeTableWidget::setEditable( bool editable, bool allowCancel )
{
  const bool isDirty { mAttributeTableBuffer &&mAttributeTableBuffer->isDirty() &&mCurrentBand > 0 && mRasterLayer->attributeTable( mCurrentBand ) };
  bool retVal { true };
  // Switch to read-only
  if ( ! editable && isDirty )
  {
    QMessageBox::StandardButtons buttons { QMessageBox::Button::Yes |  QMessageBox::Button::No };

    if ( allowCancel )
    {
      buttons |= QMessageBox::Button::Cancel;
    }

    switch ( QMessageBox::question( nullptr, tr( "Save Attribute Table" ), tr( "Attribute table contains unsaved changes, do you want to save the changes?" ), buttons ) )
    {
      case QMessageBox::Button::Cancel:
      {
        retVal = false;
        break;
      }
      case QMessageBox::Button::Yes:
      {
        saveChanges();
        retVal = true;
        break;
      }
      case QMessageBox::Button::No:
      default:
      {
        // Reset to its original state
        mAttributeTableBuffer = std::make_unique<QgsRasterAttributeTable>( *mRasterLayer->attributeTable( mCurrentBand ) );
        init( mCurrentBand );
        retVal = true;
        break;
      }
    }
  }

  if ( retVal )
  {
    mEditable = editable;
    mModel->setEditable( editable );
  }

  updateButtons();

  return retVal;
}

void QgsRasterAttributeTableWidget::saveChanges()
{
  if ( mRasterLayer && mAttributeTableBuffer && mAttributeTableBuffer->isDirty() && mCurrentBand > 0 )
  {
    QgsRasterAttributeTable *attributeTable { mRasterLayer->dataProvider()->attributeTable( mCurrentBand ) };
    if ( ! attributeTable )
    {
      QgsDebugMsg( QStringLiteral( "Error saving RAT: RAT for band %1 is unexpectedly gone!" ).arg( mCurrentBand ) );
    }
    else
    {
      *attributeTable = *mAttributeTableBuffer;
      QString errorMessage;
      QString newPath { attributeTable->filePath() };
      const bool nativeRatSupported = mRasterLayer->dataProvider()->providerCapabilities().testFlag( QgsRasterDataProvider::ProviderCapability::NativeRasterAttributeTable );
      bool saveToNative { false };

      if ( newPath.isEmpty() && ! nativeRatSupported )
      {
        newPath = QFileDialog::getOpenFileName( nullptr, tr( "Save Raster Attribute Table (band %1) To File" ).arg( mCurrentBand ), QFile::exists( mRasterLayer->dataProvider()->dataSourceUri( ) ) ? mRasterLayer->dataProvider()->dataSourceUri( ) + ".vat.dbf" : QString(), QStringLiteral( "VAT DBF Files (*.vat.dbf)" ) );
        if ( newPath.isEmpty() )
        {
          // Aborted by user
          return;
        }
      }
      else if ( newPath.isEmpty() )
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
        writeSuccess = mRasterLayer->dataProvider()->writeNativeAttributeTable( &errorMessage );  //#spellok
      }

      if ( writeSuccess )
      {
        mAttributeTableBuffer->setDirty( false );
        notify( tr( "Attribute Table Write Success" ), tr( "The raster attribute table has been successfully saved." ), Qgis::MessageLevel::Success );
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

  updateButtons();
}

void QgsRasterAttributeTableWidget::classify()
{

  if ( ! mAttributeTableBuffer )
  {
    notify( tr( "Classification Error" ), tr( "The raster attribute table is not set." ), Qgis::MessageLevel::Critical );
    return;
  }

  if ( ! mRasterLayer )
  {
    notify( tr( "Classification Error" ), tr( "The raster layer is not set." ), Qgis::MessageLevel::Critical );
    return;
  }

  QString confirmMessage;
  QString errorMessage;

  if ( ! mAttributeTableBuffer->isValid( &errorMessage ) )
  {
    confirmMessage = tr( "The attribute table does not seem to be valid and it may produce an unusable symbology, validation errors:<br>%1<br>" ).arg( errorMessage );
  }

  if ( QMessageBox::question( nullptr, tr( "Apply Style From Attribute Table" ), confirmMessage.append( tr( "The existing symbology for the raster will be replaced by a new symbology from the attribute table and any unsaved changes to the current symbology will be lost, do you want to proceed?" ) ) ) == QMessageBox::Yes )
  {

    if ( QgsRasterRenderer *renderer = mAttributeTableBuffer->createRenderer( mRasterLayer->dataProvider(), mCurrentBand, mClassifyComboBox->currentData().toInt() ) )
    {
      mRasterLayer->setRenderer( renderer );
      mRasterLayer->triggerRepaint( );
      emit rendererChanged();
    }
    else
    {
      notify( tr( "Classification Error" ), tr( "The classification returned no classes." ), Qgis::MessageLevel::Critical );
    }
  }
}

void QgsRasterAttributeTableWidget::addColumn()
{
  if ( mAttributeTableBuffer )
  {
    QgsRasterAttributeTableAddColumnDialog dlg { mAttributeTableBuffer.get() };
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
    setDelegates();
  }
}

void QgsRasterAttributeTableWidget::removeColumn()
{
  const QModelIndex currentIndex { mProxyModel->mapToSource( mRATView->selectionModel()->currentIndex() ) };
  if ( mAttributeTableBuffer && currentIndex.isValid() && currentIndex.column() < mAttributeTableBuffer->fields().count() )
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
  if ( mAttributeTableBuffer )
  {
    // Default to append
    int position { mModel->rowCount( QModelIndex() ) };
    const QModelIndex currentIndex { mProxyModel->mapToSource( mRATView->selectionModel()->currentIndex() ) };

    // If there is a selected row, ask if before of after.
    if ( currentIndex.isValid() )
    {
      // Ask the user where to insert the new row (before or after the currently
      // selected row).
      QgsRasterAttributeTableAddRowDialog dlg;
      if ( dlg.exec() != QDialog::DialogCode::Accepted )
      {
        return;
      }
      else
      {
        position = currentIndex.row() + ( dlg.insertAfter() ? 1 : 0 );
      }
    }

    bool result { true };
    QString errorMessage;

    QVariantList rowData;

    QList<QgsRasterAttributeTable::Field> fields { mAttributeTableBuffer->fields() };
    for ( const QgsRasterAttributeTable::Field &field : std::as_const( fields ) )
    {
      rowData.push_back( QVariant( field.type ) );
    }

    result = mModel->insertRow( position, rowData, &errorMessage );

    if ( ! result )
    {
      notify( tr( "Error adding row" ), errorMessage,  Qgis::MessageLevel::Critical );
    }
    else
    {
      mRATView->scrollTo( mRATView->model()->index( position, 0 ) );
    }
  }
}

void QgsRasterAttributeTableWidget::removeRow()
{
  if ( mAttributeTableBuffer && mRATView->selectionModel()->currentIndex().isValid() )
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
    if ( mEditable )
    {
      setEditable( false );
    }
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

void QgsRasterAttributeTableWidget::setDelegates()
{
  mClassifyComboBox->clear();
  if ( mAttributeTableBuffer )
  {
    const QList<QgsRasterAttributeTable::Field> tableFields { mAttributeTableBuffer->fields() };
    int fieldIdx { 0 };
    const QHash<Qgis::RasterAttributeTableFieldUsage, QgsRasterAttributeTable::UsageInformation> usageInfo { QgsRasterAttributeTable::usageInformation() };

    for ( const QgsRasterAttributeTable::Field &f : std::as_const( tableFields ) )
    {
      // Clear all delegates.
      mRATView->setItemDelegateForColumn( fieldIdx, nullptr );

      if ( usageInfo[f.usage].maybeClass )
      {
        mClassifyComboBox->addItem( QgsFields::iconForFieldType( f.type ), f.name, QVariant( fieldIdx ) );
      }

      // Set delegates for doubles
      if ( ( ! f.isColor() && ! f.isRamp() ) && f.type == QVariant::Type::Double )
      {
        mRATView->setItemDelegateForColumn( fieldIdx, new LocalizedDoubleDelegate( mRATView ) );
      }
      fieldIdx++;
    }

    // Set delegates for color and ramp
    if ( mAttributeTableBuffer->hasColor() )
    {
      if ( mAttributeTableBuffer->usages().contains( Qgis::RasterAttributeTableFieldUsage::Alpha ) )
      {
        mRATView->setItemDelegateForColumn( mAttributeTableBuffer->fields().count( ), new ColorAlphaDelegate( mRATView ) );
      }
      else
      {
        mRATView->setItemDelegateForColumn( mAttributeTableBuffer->fields().count( ), new ColorDelegate( mRATView ) );
      }
    }
    else if ( mAttributeTableBuffer->hasRamp() )
    {
      if ( mAttributeTableBuffer->usages().contains( Qgis::RasterAttributeTableFieldUsage::AlphaMin ) )
      {
        mRATView->setItemDelegateForColumn( mAttributeTableBuffer->fields().count( ), new ColorRampAlphaDelegate( mRATView ) );
      }
      else
      {
        mRATView->setItemDelegateForColumn( mAttributeTableBuffer->fields().count( ), new ColorRampDelegate( mRATView ) );
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


QString LocalizedDoubleDelegate::displayText( const QVariant &value, const QLocale &locale ) const
{
  Q_UNUSED( locale );
  const QString s( value.toString() );
  const int dotPosition( s.indexOf( '.' ) );
  int precision;
  if ( dotPosition < 0 && s.indexOf( 'e' ) < 0 )
  {
    precision = 0;
    return QLocale().toString( value.toDouble(), 'f', precision );
  }
  else
  {
    if ( dotPosition < 0 ) precision = 0;
    else precision = s.length() - dotPosition - 1;

    if ( -1 < value.toDouble() && value.toDouble() < 1 )
    {
      return QLocale().toString( value.toDouble(), 'g', precision );
    }
    else
    {
      return QLocale().toString( value.toDouble(), 'f', precision );
    }
  }
  return QLocale().toString( value.toDouble( ), 'f' );
}

///@endcond private

