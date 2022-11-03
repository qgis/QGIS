/***************************************************************************
                         qgscolorrampshaderwidget.cpp
                         ----------------------------
    begin                : Jun 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterdataprovider.h"

#include "qgscolorrampshaderwidget.h"
#include "qgssinglebandpseudocolorrenderer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsrastershader.h"
#include "qgsrasterminmaxwidget.h"
#include "qgstreewidgetitem.h"
#include "qgssettings.h"
#include "qgsstyle.h"
#include "qgscolorramp.h"
#include "qgscolorrampbutton.h"
#include "qgscolordialog.h"
#include "qgsrasterrendererutils.h"
#include "qgsfileutils.h"
#include "qgsguiutils.h"
#include "qgsdoublevalidator.h"
#include "qgslocaleawarenumericlineeditdelegate.h"
#include "qgscolorramplegendnodewidget.h"

#include <QCursor>
#include <QPushButton>
#include <QInputDialog>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QTextStream>
#include <QTreeView>


QgsColorRampShaderWidget::QgsColorRampShaderWidget( QWidget *parent )
  : QWidget( parent )
{
  QgsSettings settings;

  setupUi( this );
  mLoadFromBandButton->setVisible( false ); // only for raster version

  connect( mAddEntryButton, &QPushButton::clicked, this, &QgsColorRampShaderWidget::mAddEntryButton_clicked );
  connect( mDeleteEntryButton, &QPushButton::clicked, this, &QgsColorRampShaderWidget::mDeleteEntryButton_clicked );
  connect( mLoadFromBandButton, &QPushButton::clicked, this, &QgsColorRampShaderWidget::mLoadFromBandButton_clicked );
  connect( mLoadFromFileButton, &QPushButton::clicked, this, &QgsColorRampShaderWidget::mLoadFromFileButton_clicked );
  connect( mExportToFileButton, &QPushButton::clicked, this, &QgsColorRampShaderWidget::mExportToFileButton_clicked );
  connect( mUnitLineEdit, &QLineEdit::textEdited, this, &QgsColorRampShaderWidget::mUnitLineEdit_textEdited );
  connect( mColormapTreeWidget, &QTreeWidget::itemDoubleClicked, this, &QgsColorRampShaderWidget::mColormapTreeWidget_itemDoubleClicked );
  connect( mColorInterpolationComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsColorRampShaderWidget::mColorInterpolationComboBox_currentIndexChanged );
  connect( mClassificationModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsColorRampShaderWidget::mClassificationModeComboBox_currentIndexChanged );

  connect( mLegendSettingsButton, &QPushButton::clicked, this, &QgsColorRampShaderWidget::showLegendSettings );

  contextMenu = new QMenu( tr( "Options" ), this );
  contextMenu->addAction( tr( "Change Color…" ), this, SLOT( changeColor() ) );
  contextMenu->addAction( tr( "Change Opacity…" ), this, SLOT( changeOpacity() ) );

  mColormapTreeWidget->setItemDelegateForColumn( ColorColumn, new QgsColorSwatchDelegate( this ) );
  mValueDelegate = new QgsLocaleAwareNumericLineEditDelegate( Qgis::DataType::UnknownDataType, this );
  mColormapTreeWidget->setItemDelegateForColumn( ValueColumn, mValueDelegate );

  mColormapTreeWidget->setColumnWidth( ColorColumn, Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'X' ) * 6.6 );

  mColormapTreeWidget->setContextMenuPolicy( Qt::CustomContextMenu );
  mColormapTreeWidget->setSelectionMode( QAbstractItemView::ExtendedSelection );
  connect( mColormapTreeWidget, &QTreeView::customContextMenuRequested, this, [ = ]( QPoint ) { contextMenu->exec( QCursor::pos() ); } );

  QString defaultPalette = settings.value( QStringLiteral( "Raster/defaultPalette" ), "" ).toString();
  btnColorRamp->setColorRampFromName( defaultPalette );

  mColorInterpolationComboBox->addItem( tr( "Discrete" ), QgsColorRampShader::Discrete );
  mColorInterpolationComboBox->addItem( tr( "Linear" ), QgsColorRampShader::Interpolated );
  mColorInterpolationComboBox->addItem( tr( "Exact" ), QgsColorRampShader::Exact );
  mColorInterpolationComboBox->setCurrentIndex( mColorInterpolationComboBox->findData( QgsColorRampShader::Interpolated ) );

  mClassificationModeComboBox->addItem( tr( "Continuous" ), QgsColorRampShader::Continuous );
  mClassificationModeComboBox->addItem( tr( "Equal Interval" ), QgsColorRampShader::EqualInterval );
  // Quantile added only on demand
  mClassificationModeComboBox->setCurrentIndex( mClassificationModeComboBox->findData( QgsColorRampShader::Continuous ) );

  mNumberOfEntriesSpinBox->setValue( 5 ); // some default

  mClassificationModeComboBox_currentIndexChanged( 0 );

  resetClassifyButton();

  connect( mClassificationModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsColorRampShaderWidget::classify );
  connect( mColorInterpolationComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsColorRampShaderWidget::classify );
  connect( mClassifyButton, &QPushButton::clicked, this, &QgsColorRampShaderWidget::classify );
  connect( btnColorRamp, &QgsColorRampButton::colorRampChanged, this, &QgsColorRampShaderWidget::applyColorRamp );
  connect( mNumberOfEntriesSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsColorRampShaderWidget::classify );
  connect( mClipCheckBox, &QAbstractButton::toggled, this, &QgsColorRampShaderWidget::widgetChanged );
  connect( mLabelPrecisionSpinBox, qOverload<int>( &QSpinBox::valueChanged ), this, [ = ]( int )
  {
    autoLabel();
  } );
}

void QgsColorRampShaderWidget::initializeForUseWithRasterLayer()
{
  Q_ASSERT( mClassificationModeComboBox->findData( QgsColorRampShader::Quantile < 0 ) );
  mClassificationModeComboBox->addItem( tr( "Quantile" ), QgsColorRampShader::Quantile );
}

void QgsColorRampShaderWidget::setRasterDataProvider( QgsRasterDataProvider *dp )
{
  mRasterDataProvider = dp;
  mLoadFromBandButton->setVisible( static_cast< bool>( mRasterDataProvider ) ); // only for raster version
}

void QgsColorRampShaderWidget::setRasterBand( int band )
{
  mBand = band;
  // Assume double by default
  Qgis::DataType dataType { ( mRasterDataProvider &&mBand > 0 ) ? mRasterDataProvider->dataType( mBand ) : Qgis::DataType::Float64 };

  // Set the maximum number of digits in the precision spin box
  const int maxDigits { QgsGuiUtils::significantDigits( dataType ) };
  mLabelPrecisionSpinBox->setMaximum( maxDigits );
  mValueDelegate->setDataType( dataType );
}

void QgsColorRampShaderWidget::setExtent( const QgsRectangle &extent )
{
  mExtent = extent;
}

QgsColorRampShader QgsColorRampShaderWidget::shader() const
{
  QgsColorRampShader colorRampShader( mMin, mMax );
  colorRampShader.setLabelPrecision( mLabelPrecisionSpinBox->value() );
  colorRampShader.setColorRampType( static_cast< QgsColorRampShader::Type >( mColorInterpolationComboBox->currentData().toInt() ) );
  colorRampShader.setClassificationMode( static_cast< QgsColorRampShader::ClassificationMode >( mClassificationModeComboBox->currentData().toInt() ) );
  colorRampShader.setClip( mClipCheckBox->isChecked() );

  //iterate through mColormapTreeWidget and set colormap info of layer
  QList<QgsColorRampShader::ColorRampItem> colorRampItems;
  int topLevelItemCount = mColormapTreeWidget->topLevelItemCount();
  QTreeWidgetItem *currentItem = nullptr;
  for ( int i = 0; i < topLevelItemCount; ++i )
  {
    currentItem = mColormapTreeWidget->topLevelItem( i );
    if ( !currentItem )
    {
      continue;
    }
    QgsColorRampShader::ColorRampItem newColorRampItem;
    newColorRampItem.value = currentItem->data( ValueColumn, Qt::ItemDataRole::DisplayRole ).toDouble();
    newColorRampItem.color = currentItem->data( ColorColumn, Qt::ItemDataRole::EditRole ).value<QColor>();
    newColorRampItem.label = currentItem->text( LabelColumn );
    colorRampItems.append( newColorRampItem );
  }
  // sort the shader items
  std::sort( colorRampItems.begin(), colorRampItems.end() );
  colorRampShader.setColorRampItemList( colorRampItems );

  if ( !btnColorRamp->isNull() )
  {
    colorRampShader.setSourceColorRamp( btnColorRamp->colorRamp() );
  }

  colorRampShader.setLegendSettings( new QgsColorRampLegendNodeSettings( mLegendSettings ) );
  return colorRampShader;
}

void QgsColorRampShaderWidget::autoLabel()
{

  mColormapTreeWidget->sortItems( ValueColumn, Qt::AscendingOrder );

#ifdef QGISDEBUG
  dumpClasses();
#endif

  const QString unit = mUnitLineEdit->text();
  int topLevelItemCount = mColormapTreeWidget->topLevelItemCount();

  QTreeWidgetItem *currentItem = nullptr;
  for ( int i = 0; i < topLevelItemCount; ++i )
  {
    currentItem = mColormapTreeWidget->topLevelItem( i );
    //If the item is null or does not have a pixel values set, skip
    if ( !currentItem || currentItem->data( ValueColumn, Qt::ItemDataRole::DisplayRole ).toString().isEmpty() )
    {
      continue;
    }

    const QString lbl = createLabel( currentItem, i, unit );

    if ( currentItem->text( LabelColumn ).isEmpty() || currentItem->text( LabelColumn ) == lbl || currentItem->foreground( LabelColumn ).color() == QColor( Qt::gray ) )
    {
      currentItem->setText( LabelColumn, lbl );
      currentItem->setForeground( LabelColumn, QBrush( QColor( Qt::gray ) ) );
    }
  }

}

void QgsColorRampShaderWidget::setUnitFromLabels()
{
  QStringList allSuffixes;
  QString label;
  int topLevelItemCount = mColormapTreeWidget->topLevelItemCount();
  QTreeWidgetItem *currentItem = nullptr;
  for ( int i = 0; i < topLevelItemCount; ++i )
  {
    currentItem = mColormapTreeWidget->topLevelItem( i );
    //If the item is null or does not have a pixel values set, skip
    if ( !currentItem || currentItem->text( ValueColumn ).isEmpty() )
    {
      continue;
    }

    label = createLabel( currentItem, i, QString() );

    if ( currentItem->text( LabelColumn ).startsWith( label ) )
    {
      allSuffixes.append( currentItem->text( LabelColumn ).mid( label.length() ) );
    }
  }
  // find most common suffix
  QStringList suffixes = QStringList( allSuffixes );
  suffixes.removeDuplicates();
  int max = 0;
  QString unit;
  for ( int i = 0; i < suffixes.count(); ++i )
  {
    int n = allSuffixes.count( suffixes[i] );
    if ( n > max )
    {
      max = n;
      unit = suffixes[i];
    }
  }
  // Set this suffix as unit if at least used twice
  if ( max >= 2 )
  {
    mUnitLineEdit->setText( unit );
  }
}

#ifdef QGISDEBUG
void QgsColorRampShaderWidget::dumpClasses()
{
  for ( int row = 0; row < mColormapTreeWidget->model()->rowCount(); ++row )
  {
    const auto labelData { mColormapTreeWidget->model()->itemData( mColormapTreeWidget->model()->index( row, LabelColumn ) ) };
    const auto valueData { mColormapTreeWidget->model()->itemData( mColormapTreeWidget->model()->index( row, ValueColumn ) ) };
    QgsDebugMsgLevel( QStringLiteral( "Class %1 : %2 %3" ).arg( row )
                      .arg( labelData[ Qt::ItemDataRole::DisplayRole ].toString(),
                            valueData[ Qt::ItemDataRole::DisplayRole ].toString() ), 2 );
  }
}
#endif

void QgsColorRampShaderWidget::mAddEntryButton_clicked()
{
  QgsTreeWidgetItemObject *newItem = new QgsTreeWidgetItemObject( mColormapTreeWidget );
  newItem->setData( ValueColumn, Qt::ItemDataRole::DisplayRole, 0 );
  newItem->setData( ColorColumn, Qt::ItemDataRole::EditRole, QColor( Qt::magenta ) );
  newItem->setText( LabelColumn, QString() );
  newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable );
  connect( newItem, &QgsTreeWidgetItemObject::itemEdited,
           this, &QgsColorRampShaderWidget::mColormapTreeWidget_itemEdited );
  autoLabel();

  loadMinimumMaximumFromTree();
  updateColorRamp();
  emit widgetChanged();
}

void QgsColorRampShaderWidget::mDeleteEntryButton_clicked()
{
  QList<QTreeWidgetItem *> itemList;
  itemList = mColormapTreeWidget->selectedItems();
  if ( itemList.isEmpty() )
  {
    return;
  }

  const auto constItemList = itemList;
  for ( QTreeWidgetItem *item : constItemList )
  {
    delete item;
  }

  loadMinimumMaximumFromTree();
  updateColorRamp();
  emit widgetChanged();
}

void QgsColorRampShaderWidget::classify()
{
  std::unique_ptr< QgsColorRamp > ramp( btnColorRamp->colorRamp() );
  if ( !ramp || std::isnan( mMin ) || std::isnan( mMax ) )
  {
    return;
  }

  std::unique_ptr< QgsColorRampShader > colorRampShader( new QgsColorRampShader(
        mMin, mMax,
        ramp.release(),
        static_cast< QgsColorRampShader::Type >( mColorInterpolationComboBox->currentData().toInt() ),
        static_cast< QgsColorRampShader::ClassificationMode >( mClassificationModeComboBox->currentData().toInt() ) )
                                                       );

  // only for Quantile we need band and provider and extent
  colorRampShader->classifyColorRamp( mNumberOfEntriesSpinBox->value(),
                                      mBand,
                                      mExtent,
                                      mRasterDataProvider );
  colorRampShader->setClip( mClipCheckBox->isChecked() );

  mColormapTreeWidget->clear();

  const QList<QgsColorRampShader::ColorRampItem> colorRampItemList = colorRampShader->colorRampItemList();
  QList<QgsColorRampShader::ColorRampItem>::const_iterator it = colorRampItemList.constBegin();
  for ( ; it != colorRampItemList.end(); ++it )
  {
    QgsTreeWidgetItemObject *newItem = new QgsTreeWidgetItemObject( mColormapTreeWidget );
    newItem->setData( ValueColumn, Qt::ItemDataRole::DisplayRole, it->value );
    newItem->setData( ColorColumn, Qt::ItemDataRole::EditRole, it->color );
    newItem->setText( LabelColumn, QString() ); // Labels will be populated in autoLabel()
    newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable );
    connect( newItem, &QgsTreeWidgetItemObject::itemEdited,
             this, &QgsColorRampShaderWidget::mColormapTreeWidget_itemEdited );
  }

  mClipCheckBox->setChecked( colorRampShader->clip() );

  autoLabel();
  emit widgetChanged();
}

void QgsColorRampShaderWidget::mClassificationModeComboBox_currentIndexChanged( int index )
{
  QgsColorRampShader::ClassificationMode mode = static_cast< QgsColorRampShader::ClassificationMode >( mClassificationModeComboBox->itemData( index ).toInt() );
  mNumberOfEntriesSpinBox->setEnabled( mode != QgsColorRampShader::Continuous );
  emit classificationModeChanged( mode );
}

void QgsColorRampShaderWidget::updateColorRamp()
{
  std::unique_ptr< QgsColorRamp > ramp( shader().createColorRamp() );
  whileBlocking( btnColorRamp )->setColorRamp( ramp.get() );
}

void QgsColorRampShaderWidget::applyColorRamp()
{
  std::unique_ptr< QgsColorRamp > ramp( btnColorRamp->colorRamp() );
  if ( !ramp )
  {
    return;
  }

  if ( !btnColorRamp->colorRampName().isEmpty() )
  {
    // Remember last used color ramp
    QgsSettings settings;
    settings.setValue( QStringLiteral( "Raster/defaultPalette" ), btnColorRamp->colorRampName() );
  }

  bool enableContinuous = ( ramp->count() > 0 );
  mClassificationModeComboBox->setEnabled( enableContinuous );
  if ( !enableContinuous )
  {
    mClassificationModeComboBox->setCurrentIndex( mClassificationModeComboBox->findData( QgsColorRampShader::EqualInterval ) );
  }

  int topLevelItemCount = mColormapTreeWidget->topLevelItemCount();
  if ( topLevelItemCount > 0 )
  {
    // We need to have valid min/max values here. If we haven't, load from colormap
    double min, max;
    if ( std::isnan( mMin ) || std::isnan( mMax ) )
    {
      colormapMinMax( min, max );
    }
    else
    {
      min = mMin;
      max = mMax;
    }

    // if the list values has been customized, maintain pre-existing values
    QTreeWidgetItem *currentItem = nullptr;
    for ( int i = 0; i < topLevelItemCount; ++i )
    {
      currentItem = mColormapTreeWidget->topLevelItem( i );
      if ( !currentItem )
      {
        continue;
      }

      double value = currentItem->data( ValueColumn, Qt::ItemDataRole::EditRole ).toDouble( );
      double position = ( value - min ) / ( max - min );
      whileBlocking( static_cast<QgsTreeWidgetItemObject *>( currentItem ) )->setData( ColorColumn, Qt::ItemDataRole::EditRole, ramp->color( position ) );
    }

    emit widgetChanged();
  }
  else
  {
    classify();
  }
}

void QgsColorRampShaderWidget::populateColormapTreeWidget( const QList<QgsColorRampShader::ColorRampItem> &colorRampItems )
{
  mColormapTreeWidget->clear();
  QList<QgsColorRampShader::ColorRampItem>::const_iterator it = colorRampItems.constBegin();
  int i = 0;
  for ( ; it != colorRampItems.constEnd(); ++it )
  {
    QgsTreeWidgetItemObject *newItem = new QgsTreeWidgetItemObject( mColormapTreeWidget );
    newItem->setData( ValueColumn, Qt::ItemDataRole::DisplayRole, it->value );
    newItem->setData( ColorColumn, Qt::ItemDataRole::EditRole, it->color );
    newItem->setText( LabelColumn, it->label );
    newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable );
    connect( newItem, &QgsTreeWidgetItemObject::itemEdited,
             this, &QgsColorRampShaderWidget::mColormapTreeWidget_itemEdited );
    ++i;
  }

#ifdef QGISDEBUG
  dumpClasses();
#endif

  setUnitFromLabels();

  // Now we have the suffix
  const QString unit = mUnitLineEdit->text();
  for ( i = 0; i < mColormapTreeWidget->topLevelItemCount(); i++ )
  {
    QgsTreeWidgetItemObject *currentItem { static_cast<QgsTreeWidgetItemObject *>( mColormapTreeWidget->topLevelItem( i ) ) };
    QString lbl { createLabel( currentItem, i, unit )};
    if ( currentItem->text( LabelColumn ).isEmpty() || currentItem->text( LabelColumn ) == lbl || currentItem->foreground( LabelColumn ).color() == QColor( Qt::gray ) )
    {
      currentItem->setText( LabelColumn, lbl );
      currentItem->setForeground( LabelColumn, QBrush( QColor( Qt::gray ) ) );
    }
  }

}

void QgsColorRampShaderWidget::mLoadFromBandButton_clicked()
{
  if ( !mRasterDataProvider )
    return;

  QList<QgsColorRampShader::ColorRampItem> colorRampList = mRasterDataProvider->colorTable( mBand );
  if ( !colorRampList.isEmpty() )
  {
    populateColormapTreeWidget( colorRampList );
    mColorInterpolationComboBox->setCurrentIndex( mColorInterpolationComboBox->findData( QgsColorRampShader::Interpolated ) );
  }
  else
  {
    QMessageBox::warning( this, tr( "Load Color Map" ), tr( "The color map for band %1 has no entries." ).arg( mBand ) );
  }
  loadMinimumMaximumFromTree();
  emit widgetChanged();
}

void QgsColorRampShaderWidget::mLoadFromFileButton_clicked()
{
  QgsSettings settings;
  QString lastDir = settings.value( QStringLiteral( "lastColorMapDir" ), QDir::homePath() ).toString();
  const QString fileName = QFileDialog::getOpenFileName( this, tr( "Load Color Map from File" ), lastDir, tr( "Textfile (*.txt)" ) );
  if ( fileName.isEmpty() )
    return;

  QList<QgsColorRampShader::ColorRampItem> colorRampItems;
  QgsColorRampShader::Type type = QgsColorRampShader::Interpolated;
  QStringList errors;
  if ( QgsRasterRendererUtils::parseColorMapFile( fileName, colorRampItems, type, errors ) )
  {
    //clear the current tree
    mColormapTreeWidget->clear();

    mColorInterpolationComboBox->setCurrentIndex( mColorInterpolationComboBox->findData( type ) );

    populateColormapTreeWidget( colorRampItems );

    if ( !errors.empty() )
    {
      QMessageBox::warning( this, tr( "Load Color Map from File" ), tr( "The following lines contained errors\n\n" ) +  errors.join( '\n' ) );
    }
  }
  else
  {
    const QString error = tr( "An error occurred while reading the color map\n\n" ) + errors.join( '\n' );
    QMessageBox::warning( this, tr( "Load Color Map from File" ), error );
  }

  QFileInfo fileInfo( fileName );
  settings.setValue( QStringLiteral( "lastColorMapDir" ), fileInfo.absoluteDir().absolutePath() );

  loadMinimumMaximumFromTree();
  updateColorRamp();
  emit widgetChanged();
}

void QgsColorRampShaderWidget::mExportToFileButton_clicked()
{
  QgsSettings settings;
  QString lastDir = settings.value( QStringLiteral( "lastColorMapDir" ), QDir::homePath() ).toString();
  QString fileName = QFileDialog::getSaveFileName( this, tr( "Save Color Map as File" ), lastDir, tr( "Textfile (*.txt)" ) );
  if ( fileName.isEmpty() )
    return;

  fileName = QgsFileUtils::ensureFileNameHasExtension( fileName, QStringList() << QStringLiteral( "txt" ) );

  QList<QgsColorRampShader::ColorRampItem> colorRampItems;
  int topLevelItemCount = mColormapTreeWidget->topLevelItemCount();
  for ( int i = 0; i < topLevelItemCount; ++i )
  {
    QTreeWidgetItem *currentItem = mColormapTreeWidget->topLevelItem( i );
    if ( !currentItem )
    {
      continue;
    }

    QgsColorRampShader::ColorRampItem item;
    item.value = currentItem->data( ValueColumn, Qt::ItemDataRole::DisplayRole ).toDouble( );
    item.color = currentItem->data( ColorColumn, Qt::ItemDataRole::EditRole ).value<QColor>();
    item.label = currentItem->text( LabelColumn );
    colorRampItems << item;
  }

  if ( !QgsRasterRendererUtils::saveColorMapFile( fileName, colorRampItems, static_cast< QgsColorRampShader::Type >( mColorInterpolationComboBox->currentData().toInt() ) ) )
  {
    QMessageBox::warning( this, tr( "Save Color Map as File" ), tr( "Write access denied. Adjust the file permissions and try again.\n\n" ) );
  }

  QFileInfo fileInfo( fileName );
  settings.setValue( QStringLiteral( "lastColorMapDir" ), fileInfo.absoluteDir().absolutePath() );
}

void QgsColorRampShaderWidget::mColormapTreeWidget_itemDoubleClicked( QTreeWidgetItem *item, int column )
{
  if ( !item )
  {
    return;
  }

  if ( column == LabelColumn )
  {
    // Set text color to default black, which signifies a manually edited label
    item->setForeground( LabelColumn, QBrush() );
  }
}

void QgsColorRampShaderWidget::mColormapTreeWidget_itemEdited( QTreeWidgetItem *item, int column )
{
  Q_UNUSED( item )

  switch ( column )
  {
    case ValueColumn:
    {
      autoLabel();
      loadMinimumMaximumFromTree();
      updateColorRamp();
      emit widgetChanged();
      break;
    }

    case LabelColumn:
    {
      // call autoLabel to fill when empty or gray out when same as autoLabel
      autoLabel();
      emit widgetChanged();
      break;
    }

    case ColorColumn:
    {
      loadMinimumMaximumFromTree();
      updateColorRamp();
      emit widgetChanged();
      break;
    }
  }
}

void QgsColorRampShaderWidget::setFromShader( const QgsColorRampShader &colorRampShader )
{
  // Those objects are connected to classify() the color ramp shader if they change, or call widget change
  // need to block them to avoid to classify and to alter the color ramp, or to call duplicate widget change
  whileBlocking( mClipCheckBox )->setChecked( colorRampShader.clip() );
  whileBlocking( mColorInterpolationComboBox )->setCurrentIndex( mColorInterpolationComboBox->findData( colorRampShader.colorRampType() ) );
  mColorInterpolationComboBox_currentIndexChanged( mColorInterpolationComboBox->currentIndex() );
  whileBlocking( mClassificationModeComboBox )->setCurrentIndex( mClassificationModeComboBox->findData( colorRampShader.classificationMode() ) );
  mClassificationModeComboBox_currentIndexChanged( mClassificationModeComboBox->currentIndex() );
  whileBlocking( mNumberOfEntriesSpinBox )->setValue( colorRampShader.colorRampItemList().count() ); // some default

  if ( colorRampShader.sourceColorRamp() )
  {
    whileBlocking( btnColorRamp )->setColorRamp( colorRampShader.sourceColorRamp() );
  }
  else
  {
    QgsSettings settings;
    QString defaultPalette = settings.value( QStringLiteral( "/Raster/defaultPalette" ), "Spectral" ).toString();
    btnColorRamp->setColorRampFromName( defaultPalette );
  }

  mLabelPrecisionSpinBox->setValue( colorRampShader.labelPrecision() );

  populateColormapTreeWidget( colorRampShader.colorRampItemList() );

  if ( colorRampShader.legendSettings() )
    mLegendSettings = *colorRampShader.legendSettings();

  emit widgetChanged();
}

void QgsColorRampShaderWidget::mColorInterpolationComboBox_currentIndexChanged( int index )
{
  QgsColorRampShader::Type interpolation = static_cast< QgsColorRampShader::Type >( mColorInterpolationComboBox->itemData( index ).toInt() );

  mClipCheckBox->setEnabled( interpolation == QgsColorRampShader::Interpolated );

  QString valueLabel;
  QString valueToolTip;
  switch ( interpolation )
  {
    case QgsColorRampShader::Interpolated:
      valueLabel = tr( "Value" );
      valueToolTip = tr( "Value for color stop" );
      mLegendSettingsButton->setEnabled( true );
      break;
    case QgsColorRampShader::Discrete:
      valueLabel = tr( "Value <=" );
      valueToolTip = tr( "Maximum value for class" );
      mLegendSettingsButton->setEnabled( false );
      break;
    case QgsColorRampShader::Exact:
      valueLabel = tr( "Value =" );
      valueToolTip = tr( "Value for color" );
      mLegendSettingsButton->setEnabled( false );
      break;
  }

  QTreeWidgetItem *header = mColormapTreeWidget->headerItem();
  header->setText( ValueColumn, valueLabel );
  header->setToolTip( ValueColumn, valueToolTip );

  autoLabel();
  emit widgetChanged();
}

void QgsColorRampShaderWidget::setMinimumMaximumAndClassify( double min, double max )
{
  if ( !qgsDoubleNear( mMin, min ) || !qgsDoubleNear( mMax, max ) )
  {
    setMinimumMaximum( min, max );
    classify();
  }
}

void QgsColorRampShaderWidget::setMinimumMaximum( double min, double max )
{
  mMin = min;
  mMax = max;
  resetClassifyButton();
}

double QgsColorRampShaderWidget::minimum() const
{
  return mMin;
}

double QgsColorRampShaderWidget::maximum() const
{
  return mMax;
}

bool QgsColorRampShaderWidget::colormapMinMax( double &min, double &max ) const
{
  QTreeWidgetItem *item = mColormapTreeWidget->topLevelItem( 0 );
  if ( !item )
  {
    return false;
  }

  // If using discrete, the first and last items contain the upper and lower
  // values of the first and last classes, we don't want these values but real min/max
  if ( ! std::isnan( mMin ) && ! std::isnan( mMax ) && static_cast< QgsColorRampShader::Type >( mColorInterpolationComboBox->currentData().toInt() ) == QgsColorRampShader::Type::Discrete )
  {
    min = mMin;
    max = mMax;
  }
  else
  {
    min = item->data( ValueColumn, Qt::ItemDataRole::DisplayRole ).toDouble();
    item = mColormapTreeWidget->topLevelItem( mColormapTreeWidget->topLevelItemCount() - 1 );
    max = item->data( ValueColumn, Qt::ItemDataRole::DisplayRole ).toDouble();
  }
  return true;
}

void QgsColorRampShaderWidget::loadMinimumMaximumFromTree()
{
  double min = 0, max = 0;
  if ( ! colormapMinMax( min, max ) )
  {
    return;
  }

  if ( !qgsDoubleNear( mMin, min ) || !qgsDoubleNear( mMax, max ) )
  {
    mMin = min;
    mMax = max;
    emit minimumMaximumChangedFromTree( min, max );
  }
}

void QgsColorRampShaderWidget::resetClassifyButton()
{
  mClassifyButton->setEnabled( true );
  if ( std::isnan( mMin ) || std::isnan( mMax ) || mMin >= mMax )
  {
    mClassifyButton->setEnabled( false );
  }
}

QString QgsColorRampShaderWidget::createLabel( QTreeWidgetItem *currentItem, int row, const QString unit )
{
  auto applyPrecision = [ = ]( const QString & value )
  {
    double val { value.toDouble( ) };
    Qgis::DataType dataType { mRasterDataProvider ? mRasterDataProvider->dataType( mBand ) : Qgis::DataType::Float64 };
    switch ( dataType )
    {
      case Qgis::DataType::Int16:
      case Qgis::DataType::UInt16:
      case Qgis::DataType::Int32:
      case Qgis::DataType::UInt32:
      case Qgis::DataType::Byte:
      case Qgis::DataType::CInt16:
      case Qgis::DataType::CInt32:
      case Qgis::DataType::ARGB32:
      case Qgis::DataType::ARGB32_Premultiplied:
      {
        return QLocale().toString( std::round( val ), 'f', 0 );
      }
      case Qgis::DataType::Float32:
      case Qgis::DataType::CFloat32:
      {
        if ( mLabelPrecisionSpinBox->value() <  0 )
        {
          const double factor = std::pow( 10, - mLabelPrecisionSpinBox->value() );
          val = static_cast<qlonglong>( val / factor ) * factor;
          return QLocale().toString( val, 'f', 0 );
        }
        return QLocale().toString( val, 'f', mLabelPrecisionSpinBox->value() );
      }
      case Qgis::DataType::Float64:
      case Qgis::DataType::CFloat64:
      case Qgis::DataType::UnknownDataType:
      {
        if ( mLabelPrecisionSpinBox->value() <  0 )
        {
          const double factor = std::pow( 10, - mLabelPrecisionSpinBox->value() );
          val = static_cast<qlonglong>( val / factor ) * factor;
          return QLocale().toString( val, 'f', 0 );
        }
        return QLocale().toString( val, 'f', mLabelPrecisionSpinBox->value() );
      }
    }
    return QString();
  };

  QgsColorRampShader::Type interpolation = static_cast< QgsColorRampShader::Type >( mColorInterpolationComboBox->currentData().toInt() );
  bool discrete = interpolation == QgsColorRampShader::Discrete;
  QString lbl;

  if ( discrete )
  {
    if ( row == 0 )
    {
      lbl = "<= " + applyPrecision( currentItem->data( ValueColumn, Qt::ItemDataRole::DisplayRole ).toString() ) + unit;
    }
    else if ( currentItem->data( ValueColumn, Qt::ItemDataRole::DisplayRole ).toDouble( ) == std::numeric_limits<double>::infinity() )
    {
      lbl = "> " + applyPrecision( mColormapTreeWidget->topLevelItem( row - 1 )->data( ValueColumn, Qt::ItemDataRole::DisplayRole ).toString() ) + unit;
    }
    else
    {
      lbl = applyPrecision( mColormapTreeWidget->topLevelItem( row - 1 )->data( ValueColumn, Qt::ItemDataRole::DisplayRole ).toString() ) + " - " + applyPrecision( currentItem->data( ValueColumn, Qt::ItemDataRole::DisplayRole ).toString() ) + unit;
    }
  }
  else
  {
    lbl = applyPrecision( currentItem->data( ValueColumn, Qt::ItemDataRole::DisplayRole ).toString() ) + unit;
  }

  return lbl;

}

void QgsColorRampShaderWidget::changeColor()
{
  QList<QTreeWidgetItem *> itemList;
  itemList = mColormapTreeWidget->selectedItems();
  if ( itemList.isEmpty() )
  {
    return;
  }
  QTreeWidgetItem *firstItem = itemList.first();

  QColor currentColor = firstItem->data( ColorColumn, Qt::ItemDataRole::EditRole ).value<QColor>();
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( qobject_cast< QWidget * >( parent() ) );
  if ( panel && panel->dockMode() )
  {
    QgsCompoundColorWidget *colorWidget = new QgsCompoundColorWidget( panel, currentColor, QgsCompoundColorWidget::LayoutVertical );
    colorWidget->setPanelTitle( tr( "Select Color" ) );
    colorWidget->setAllowOpacity( true );
    connect( colorWidget, &QgsCompoundColorWidget::currentColorChanged, this, [ = ]( const QColor & newColor )
    {
      for ( QTreeWidgetItem *item : std::as_const( itemList ) )
      {
        item->setData( ColorColumn, Qt::ItemDataRole::EditRole, newColor );
      }

      loadMinimumMaximumFromTree();
      emit widgetChanged();
    } );
    panel->openPanel( colorWidget );
  }
  else
  {
    // modal dialog version... yuck
    QColor newColor = QgsColorDialog::getColor( currentColor, this, QStringLiteral( "Change Color" ), true );
    if ( newColor.isValid() )
    {
      for ( QTreeWidgetItem *item : std::as_const( itemList ) )
      {
        item->setData( ColorColumn, Qt::ItemDataRole::EditRole, newColor );
      }

      loadMinimumMaximumFromTree();
      emit widgetChanged();
    }
  }
}

void QgsColorRampShaderWidget::changeOpacity()
{
  QList<QTreeWidgetItem *> itemList;
  itemList = mColormapTreeWidget->selectedItems();
  if ( itemList.isEmpty() )
  {
    return;
  }
  QTreeWidgetItem *firstItem = itemList.first();

  bool ok;
  double oldOpacity = firstItem->data( ColorColumn, Qt::ItemDataRole::EditRole ).value<QColor>().alpha() / 255 * 100;
  double opacity = QInputDialog::getDouble( this, tr( "Opacity" ), tr( "Change color opacity [%]" ), oldOpacity, 0.0, 100.0, 0, &ok );
  if ( ok )
  {
    int newOpacity = static_cast<int>( opacity / 100 * 255 );
    const auto constItemList = itemList;
    for ( QTreeWidgetItem *item : constItemList )
    {
      QColor newColor = item->data( ColorColumn, Qt::ItemDataRole::EditRole ).value<QColor>();
      newColor.setAlpha( newOpacity );
      item->setData( ColorColumn, Qt::ItemDataRole::EditRole, newColor );
    }

    loadMinimumMaximumFromTree();
    emit widgetChanged();
  }
}

void QgsColorRampShaderWidget::showLegendSettings()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( qobject_cast< QWidget * >( parent() ) );
  if ( panel && panel->dockMode() )
  {
    QgsColorRampLegendNodeWidget *legendPanel = new QgsColorRampLegendNodeWidget();
    legendPanel->setPanelTitle( tr( "Legend Settings" ) );
    legendPanel->setSettings( mLegendSettings );
    connect( legendPanel, &QgsColorRampLegendNodeWidget::widgetChanged, this, [ = ]
    {
      mLegendSettings = legendPanel->settings();
      emit widgetChanged();
    } );
    panel->openPanel( legendPanel );
  }
  else
  {
    QgsColorRampLegendNodeDialog dialog( mLegendSettings, this );
    dialog.setWindowTitle( tr( "Legend Settings" ) );
    if ( dialog.exec() )
    {
      mLegendSettings = dialog.settings();
      emit widgetChanged();
    }
  }
}
