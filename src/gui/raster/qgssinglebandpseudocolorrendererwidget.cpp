/***************************************************************************
                         qgssinglebandpseudocolorrendererwidget.cpp
                         ------------------------------------------
    begin                : February 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssinglebandpseudocolorrendererwidget.h"
#include "qgssinglebandpseudocolorrenderer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsrastershader.h"
#include "qgsrasterminmaxwidget.h"
#include "qgstreewidgetitem.h"
#include "qgssettings.h"

// for color ramps - todo add rasterStyle and refactor raster vs. vector ramps
#include "qgsstyle.h"
#include "qgscolorramp.h"
#include "qgscolorrampbutton.h"
#include "qgscolordialog.h"

#include <QCursor>
#include <QPushButton>
#include <QInputDialog>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QTextStream>
#include <QTreeView>

QgsSingleBandPseudoColorRendererWidget::QgsSingleBandPseudoColorRendererWidget( QgsRasterLayer *layer, const QgsRectangle &extent )
  : QgsRasterRendererWidget( layer, extent )
  , mDisableMinMaxWidgetRefresh( false )
  , mMinMaxOrigin( 0 )
{
  QgsSettings settings;

  setupUi( this );
  connect( mAddEntryButton, &QPushButton::clicked, this, &QgsSingleBandPseudoColorRendererWidget::mAddEntryButton_clicked );
  connect( mDeleteEntryButton, &QPushButton::clicked, this, &QgsSingleBandPseudoColorRendererWidget::mDeleteEntryButton_clicked );
  connect( mLoadFromBandButton, &QPushButton::clicked, this, &QgsSingleBandPseudoColorRendererWidget::mLoadFromBandButton_clicked );
  connect( mLoadFromFileButton, &QPushButton::clicked, this, &QgsSingleBandPseudoColorRendererWidget::mLoadFromFileButton_clicked );
  connect( mExportToFileButton, &QPushButton::clicked, this, &QgsSingleBandPseudoColorRendererWidget::mExportToFileButton_clicked );
  connect( mUnitLineEdit, &QLineEdit::textEdited, this, &QgsSingleBandPseudoColorRendererWidget::mUnitLineEdit_textEdited );
  connect( mColormapTreeWidget, &QTreeWidget::itemDoubleClicked, this, &QgsSingleBandPseudoColorRendererWidget::mColormapTreeWidget_itemDoubleClicked );
  connect( mColorInterpolationComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsSingleBandPseudoColorRendererWidget::mColorInterpolationComboBox_currentIndexChanged );
  connect( mMinLineEdit, &QLineEdit::textChanged, this, &QgsSingleBandPseudoColorRendererWidget::mMinLineEdit_textChanged );
  connect( mMaxLineEdit, &QLineEdit::textChanged, this, &QgsSingleBandPseudoColorRendererWidget::mMaxLineEdit_textChanged );
  connect( mMinLineEdit, &QLineEdit::textEdited, this, &QgsSingleBandPseudoColorRendererWidget::mMinLineEdit_textEdited );
  connect( mMaxLineEdit, &QLineEdit::textEdited, this, &QgsSingleBandPseudoColorRendererWidget::mMaxLineEdit_textEdited );
  connect( mClassificationModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsSingleBandPseudoColorRendererWidget::mClassificationModeComboBox_currentIndexChanged );

  contextMenu = new QMenu( tr( "Options" ), this );
  contextMenu->addAction( tr( "Change color" ), this, SLOT( changeColor() ) );
  contextMenu->addAction( tr( "Change opacity" ), this, SLOT( changeOpacity() ) );

  mColormapTreeWidget->setColumnWidth( ColorColumn, 50 );
  mColormapTreeWidget->setContextMenuPolicy( Qt::CustomContextMenu );
  mColormapTreeWidget->setSelectionMode( QAbstractItemView::ExtendedSelection );
  connect( mColormapTreeWidget, &QTreeView::customContextMenuRequested, this, [ = ]( const QPoint & ) { contextMenu->exec( QCursor::pos() ); }
         );

  QString defaultPalette = settings.value( QStringLiteral( "Raster/defaultPalette" ), "" ).toString();
  btnColorRamp->setColorRampFromName( defaultPalette );

  if ( !mRasterLayer )
  {
    return;
  }

  QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
  if ( !provider )
  {
    return;
  }

  // Must be before adding items to mBandComboBox (signal)
  mMinLineEdit->setValidator( new QDoubleValidator( mMinLineEdit ) );
  mMaxLineEdit->setValidator( new QDoubleValidator( mMaxLineEdit ) );

  mMinMaxWidget = new QgsRasterMinMaxWidget( layer, this );
  mMinMaxWidget->setExtent( extent );
  mMinMaxWidget->setMapCanvas( mCanvas );

  QHBoxLayout *layout = new QHBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  mMinMaxContainerWidget->setLayout( layout );
  layout->addWidget( mMinMaxWidget );

  connect( mMinMaxWidget, &QgsRasterMinMaxWidget::widgetChanged, this, &QgsSingleBandPseudoColorRendererWidget::widgetChanged );
  connect( mMinMaxWidget, &QgsRasterMinMaxWidget::load, this, &QgsSingleBandPseudoColorRendererWidget::loadMinMax );

  mBandComboBox->setLayer( mRasterLayer );

  mColorInterpolationComboBox->addItem( tr( "Discrete" ), QgsColorRampShader::Discrete );
  mColorInterpolationComboBox->addItem( tr( "Linear" ), QgsColorRampShader::Interpolated );
  mColorInterpolationComboBox->addItem( tr( "Exact" ), QgsColorRampShader::Exact );
  mColorInterpolationComboBox->setCurrentIndex( mColorInterpolationComboBox->findData( QgsColorRampShader::Interpolated ) );

  mClassificationModeComboBox->addItem( tr( "Continuous" ), QgsColorRampShader::Continuous );
  mClassificationModeComboBox->addItem( tr( "Equal interval" ), QgsColorRampShader::EqualInterval );
  mClassificationModeComboBox->addItem( tr( "Quantile" ), QgsColorRampShader::Quantile );
  mClassificationModeComboBox->setCurrentIndex( mClassificationModeComboBox->findData( QgsColorRampShader::Continuous ) );

  mNumberOfEntriesSpinBox->setValue( 5 ); // some default

  setFromRenderer( layer->renderer() );

  // If there is currently no min/max, load default with user current default options
  if ( mMinLineEdit->text().isEmpty() || mMaxLineEdit->text().isEmpty() )
  {
    QgsRasterMinMaxOrigin minMaxOrigin = mMinMaxWidget->minMaxOrigin();
    if ( minMaxOrigin.limits() == QgsRasterMinMaxOrigin::None )
    {
      minMaxOrigin.setLimits( QgsRasterMinMaxOrigin::MinMax );
      mMinMaxWidget->setFromMinMaxOrigin( minMaxOrigin );
    }
    mMinMaxWidget->doComputations();
  }

  mClassificationModeComboBox_currentIndexChanged( 0 );

  resetClassifyButton();

  connect( mClassificationModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsSingleBandPseudoColorRendererWidget::classify );
  connect( mClassifyButton, &QPushButton::clicked, this, &QgsSingleBandPseudoColorRendererWidget::applyColorRamp );
  connect( btnColorRamp, &QgsColorRampButton::colorRampChanged, this, &QgsSingleBandPseudoColorRendererWidget::applyColorRamp );
  connect( mNumberOfEntriesSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsSingleBandPseudoColorRendererWidget::classify );
  connect( mBandComboBox, &QgsRasterBandComboBox::bandChanged, this, &QgsSingleBandPseudoColorRendererWidget::classify );
  connect( mBandComboBox, &QgsRasterBandComboBox::bandChanged, this, &QgsSingleBandPseudoColorRendererWidget::bandChanged );
  connect( mClipCheckBox, &QAbstractButton::toggled, this, &QgsRasterRendererWidget::widgetChanged );
}

QgsRasterRenderer *QgsSingleBandPseudoColorRendererWidget::renderer()
{
  QgsRasterShader *rasterShader = new QgsRasterShader();
  QgsColorRampShader *colorRampShader = new QgsColorRampShader( lineEditValue( mMinLineEdit ), lineEditValue( mMaxLineEdit ) );
  colorRampShader->setColorRampType( static_cast< QgsColorRampShader::Type >( mColorInterpolationComboBox->currentData().toInt() ) );
  colorRampShader->setClassificationMode( static_cast< QgsColorRampShader::ClassificationMode >( mClassificationModeComboBox->currentData().toInt() ) );
  colorRampShader->setClip( mClipCheckBox->isChecked() );

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
    newColorRampItem.value = currentItem->text( ValueColumn ).toDouble();
    newColorRampItem.color = currentItem->background( ColorColumn ).color();
    newColorRampItem.label = currentItem->text( LabelColumn );
    colorRampItems.append( newColorRampItem );
  }
  // sort the shader items
  std::sort( colorRampItems.begin(), colorRampItems.end() );
  colorRampShader->setColorRampItemList( colorRampItems );

  if ( !btnColorRamp->isNull() )
  {
    colorRampShader->setSourceColorRamp( btnColorRamp->colorRamp() );
  }

  rasterShader->setRasterShaderFunction( colorRampShader );

  int bandNumber = mBandComboBox->currentBand();
  QgsSingleBandPseudoColorRenderer *renderer = new QgsSingleBandPseudoColorRenderer( mRasterLayer->dataProvider(), bandNumber, rasterShader );
  renderer->setClassificationMin( lineEditValue( mMinLineEdit ) );
  renderer->setClassificationMax( lineEditValue( mMaxLineEdit ) );
  renderer->setMinMaxOrigin( mMinMaxWidget->minMaxOrigin() );
  return renderer;
}

void QgsSingleBandPseudoColorRendererWidget::doComputations()
{
  mMinMaxWidget->doComputations();
  if ( mColormapTreeWidget->topLevelItemCount() == 0 )
    applyColorRamp();
}

void QgsSingleBandPseudoColorRendererWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  QgsRasterRendererWidget::setMapCanvas( canvas );
  mMinMaxWidget->setMapCanvas( canvas );
}

void QgsSingleBandPseudoColorRendererWidget::autoLabel()
{
  QgsColorRampShader::Type interpolation = static_cast< QgsColorRampShader::Type >( mColorInterpolationComboBox->currentData().toInt() );
  bool discrete = interpolation == QgsColorRampShader::Discrete;
  QString unit = mUnitLineEdit->text();
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

    if ( discrete )
    {
      if ( i == 0 )
      {
        label = "<= " + currentItem->text( ValueColumn ) + unit;
      }
      else if ( currentItem->text( ValueColumn ).toDouble() == std::numeric_limits<double>::infinity() )
      {
        label = "> " + mColormapTreeWidget->topLevelItem( i - 1 )->text( ValueColumn ) + unit;
      }
      else
      {
        label = mColormapTreeWidget->topLevelItem( i - 1 )->text( ValueColumn ) + " - " + currentItem->text( ValueColumn ) + unit;
      }
    }
    else
    {
      label = currentItem->text( ValueColumn ) + unit;
    }

    if ( currentItem->text( LabelColumn ).isEmpty() || currentItem->text( LabelColumn ) == label || currentItem->foreground( LabelColumn ).color() == QColor( Qt::gray ) )
    {
      currentItem->setText( LabelColumn, label );
      currentItem->setForeground( LabelColumn, QBrush( QColor( Qt::gray ) ) );
    }
  }
}

void QgsSingleBandPseudoColorRendererWidget::setUnitFromLabels()
{
  QgsColorRampShader::Type interpolation = static_cast< QgsColorRampShader::Type >( mColorInterpolationComboBox->currentData().toInt() );
  bool discrete = interpolation == QgsColorRampShader::Discrete;
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

    if ( discrete )
    {
      if ( i == 0 )
      {
        label = "<= " + currentItem->text( ValueColumn );
      }
      else if ( currentItem->text( ValueColumn ).toDouble() == std::numeric_limits<double>::infinity() )
      {
        label = "> " + mColormapTreeWidget->topLevelItem( i - 1 )->text( ValueColumn );
      }
      else
      {
        label = mColormapTreeWidget->topLevelItem( i - 1 )->text( ValueColumn ) + " - " + currentItem->text( ValueColumn );
      }
    }
    else
    {
      label = currentItem->text( ValueColumn );
    }

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
  autoLabel();
}

void QgsSingleBandPseudoColorRendererWidget::mAddEntryButton_clicked()
{
  QgsTreeWidgetItemObject *newItem = new QgsTreeWidgetItemObject( mColormapTreeWidget );
  newItem->setText( ValueColumn, QStringLiteral( "0" ) );
  newItem->setBackground( ColorColumn, QBrush( QColor( Qt::magenta ) ) );
  newItem->setText( LabelColumn, QString() );
  newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable );
  connect( newItem, &QgsTreeWidgetItemObject::itemEdited,
           this, &QgsSingleBandPseudoColorRendererWidget::mColormapTreeWidget_itemEdited );
  mColormapTreeWidget->sortItems( ValueColumn, Qt::AscendingOrder );
  autoLabel();
  emit widgetChanged();
}

void QgsSingleBandPseudoColorRendererWidget::mDeleteEntryButton_clicked()
{
  QList<QTreeWidgetItem *> itemList;
  itemList = mColormapTreeWidget->selectedItems();
  if ( itemList.isEmpty() )
  {
    return;
  }

  Q_FOREACH ( QTreeWidgetItem *item, itemList )
  {
    delete item;
  }
  emit widgetChanged();
}

void QgsSingleBandPseudoColorRendererWidget::classify()
{
  std::unique_ptr< QgsColorRamp > ramp( btnColorRamp->colorRamp() );
  if ( !ramp || std::isnan( lineEditValue( mMinLineEdit ) ) || std::isnan( lineEditValue( mMaxLineEdit ) ) )
  {
    return;
  }

  QgsSingleBandPseudoColorRenderer *pr = new QgsSingleBandPseudoColorRenderer( mRasterLayer->dataProvider(), mBandComboBox->currentBand(), nullptr );
  pr->setClassificationMin( lineEditValue( mMinLineEdit ) );
  pr->setClassificationMax( lineEditValue( mMaxLineEdit ) );
  pr->createShader( ramp.get(), static_cast< QgsColorRampShader::Type >( mColorInterpolationComboBox->currentData().toInt() ), static_cast< QgsColorRampShader::ClassificationMode >( mClassificationModeComboBox->currentData().toInt() ), mNumberOfEntriesSpinBox->value(), mClipCheckBox->isChecked(), minMaxWidget()->extent() );

  const QgsRasterShader *rasterShader = pr->shader();
  if ( rasterShader )
  {
    const QgsColorRampShader *colorRampShader = dynamic_cast<const QgsColorRampShader *>( rasterShader->rasterShaderFunction() );
    if ( colorRampShader )
    {
      mColormapTreeWidget->clear();

      const QList<QgsColorRampShader::ColorRampItem> colorRampItemList = colorRampShader->colorRampItemList();
      QList<QgsColorRampShader::ColorRampItem>::const_iterator it = colorRampItemList.constBegin();
      for ( ; it != colorRampItemList.end(); ++it )
      {
        QgsTreeWidgetItemObject *newItem = new QgsTreeWidgetItemObject( mColormapTreeWidget );
        newItem->setText( ValueColumn, QString::number( it->value, 'g', 15 ) );
        newItem->setBackground( ColorColumn, QBrush( it->color ) );
        newItem->setText( LabelColumn, it->label );
        newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable );
        connect( newItem, &QgsTreeWidgetItemObject::itemEdited,
                 this, &QgsSingleBandPseudoColorRendererWidget::mColormapTreeWidget_itemEdited );
      }
      mClipCheckBox->setChecked( colorRampShader->clip() );
    }
  }

  autoLabel();
  emit widgetChanged();
}

void QgsSingleBandPseudoColorRendererWidget::mClassificationModeComboBox_currentIndexChanged( int index )
{
  QgsColorRampShader::ClassificationMode mode = static_cast< QgsColorRampShader::ClassificationMode >( mClassificationModeComboBox->itemData( index ).toInt() );
  mNumberOfEntriesSpinBox->setEnabled( mode != QgsColorRampShader::Continuous );
  mMinLineEdit->setEnabled( mode != QgsColorRampShader::Quantile );
  mMaxLineEdit->setEnabled( mode != QgsColorRampShader::Quantile );
}

void QgsSingleBandPseudoColorRendererWidget::applyColorRamp()
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

  classify();
}

void QgsSingleBandPseudoColorRendererWidget::populateColormapTreeWidget( const QList<QgsColorRampShader::ColorRampItem> &colorRampItems )
{
  mColormapTreeWidget->clear();
  QList<QgsColorRampShader::ColorRampItem>::const_iterator it = colorRampItems.constBegin();
  for ( ; it != colorRampItems.constEnd(); ++it )
  {
    QgsTreeWidgetItemObject *newItem = new QgsTreeWidgetItemObject( mColormapTreeWidget );
    newItem->setText( ValueColumn, QString::number( it->value, 'g', 15 ) );
    newItem->setBackground( ColorColumn, QBrush( it->color ) );
    newItem->setText( LabelColumn, it->label );
    newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable );
    connect( newItem, &QgsTreeWidgetItemObject::itemEdited,
             this, &QgsSingleBandPseudoColorRendererWidget::mColormapTreeWidget_itemEdited );
  }
  setUnitFromLabels();
}

void QgsSingleBandPseudoColorRendererWidget::mLoadFromBandButton_clicked()
{
  if ( !mRasterLayer || !mRasterLayer->dataProvider() )
  {
    return;
  }

  int bandIndex = mBandComboBox->currentBand();


  QList<QgsColorRampShader::ColorRampItem> colorRampList = mRasterLayer->dataProvider()->colorTable( bandIndex );
  if ( !colorRampList.isEmpty() )
  {
    populateColormapTreeWidget( colorRampList );
    mColorInterpolationComboBox->setCurrentIndex( mColorInterpolationComboBox->findData( QgsColorRampShader::Interpolated ) );
  }
  else
  {
    QMessageBox::warning( this, tr( "Load Color Map" ), tr( "The color map for band %1 has no entries" ).arg( bandIndex ) );
  }
  emit widgetChanged();
}

void QgsSingleBandPseudoColorRendererWidget::mLoadFromFileButton_clicked()
{
  int lineCounter = 0;
  bool importError = false;
  QString badLines;
  QgsSettings settings;
  QString lastDir = settings.value( QStringLiteral( "lastColorMapDir" ), QDir::homePath() ).toString();
  QString fileName = QFileDialog::getOpenFileName( this, tr( "Open file" ), lastDir, tr( "Textfile (*.txt)" ) );
  QFile inputFile( fileName );
  if ( inputFile.open( QFile::ReadOnly ) )
  {
    //clear the current tree
    mColormapTreeWidget->clear();

    QTextStream inputStream( &inputFile );
    QString inputLine;
    QStringList inputStringComponents;
    QList<QgsColorRampShader::ColorRampItem> colorRampItems;

    //read through the input looking for valid data
    while ( !inputStream.atEnd() )
    {
      lineCounter++;
      inputLine = inputStream.readLine();
      if ( !inputLine.isEmpty() )
      {
        if ( !inputLine.simplified().startsWith( '#' ) )
        {
          if ( inputLine.contains( QLatin1String( "INTERPOLATION" ), Qt::CaseInsensitive ) )
          {
            inputStringComponents = inputLine.split( ':' );
            if ( inputStringComponents.size() == 2 )
            {
              if ( inputStringComponents[1].trimmed().toUpper().compare( QLatin1String( "INTERPOLATED" ), Qt::CaseInsensitive ) == 0 )
              {
                mColorInterpolationComboBox->setCurrentIndex( mColorInterpolationComboBox->findData( QgsColorRampShader::Interpolated ) );
              }
              else if ( inputStringComponents[1].trimmed().toUpper().compare( QLatin1String( "DISCRETE" ), Qt::CaseInsensitive ) == 0 )
              {
                mColorInterpolationComboBox->setCurrentIndex( mColorInterpolationComboBox->findData( QgsColorRampShader::Discrete ) );
              }
              else
              {
                mColorInterpolationComboBox->setCurrentIndex( mColorInterpolationComboBox->findData( QgsColorRampShader::Exact ) );
              }
            }
            else
            {
              importError = true;
              badLines = badLines + QString::number( lineCounter ) + ":\t[" + inputLine + "]\n";
            }
          }
          else
          {
            inputStringComponents = inputLine.split( ',' );
            if ( inputStringComponents.size() == 6 )
            {
              QgsColorRampShader::ColorRampItem currentItem( inputStringComponents[0].toDouble(),
                  QColor::fromRgb( inputStringComponents[1].toInt(), inputStringComponents[2].toInt(),
                                   inputStringComponents[3].toInt(), inputStringComponents[4].toInt() ),
                  inputStringComponents[5] );
              colorRampItems.push_back( currentItem );
            }
            else
            {
              importError = true;
              badLines = badLines + QString::number( lineCounter ) + ":\t[" + inputLine + "]\n";
            }
          }
        }
      }
      lineCounter++;
    }
    populateColormapTreeWidget( colorRampItems );

    QFileInfo fileInfo( fileName );
    settings.setValue( QStringLiteral( "lastColorMapDir" ), fileInfo.absoluteDir().absolutePath() );

    if ( importError )
    {
      QMessageBox::warning( this, tr( "Import Error" ), tr( "The following lines contained errors\n\n" ) + badLines );
    }
  }
  else if ( !fileName.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Read access denied" ), tr( "Read access denied. Adjust the file permissions and try again.\n\n" ) );
  }
  emit widgetChanged();
}

void QgsSingleBandPseudoColorRendererWidget::mExportToFileButton_clicked()
{
  QgsSettings settings;
  QString lastDir = settings.value( QStringLiteral( "lastColorMapDir" ), QDir::homePath() ).toString();
  QString fileName = QFileDialog::getSaveFileName( this, tr( "Save file" ), lastDir, tr( "Textfile (*.txt)" ) );
  if ( !fileName.isEmpty() )
  {
    if ( !fileName.endsWith( QLatin1String( ".txt" ), Qt::CaseInsensitive ) )
    {
      fileName = fileName + ".txt";
    }

    QFile outputFile( fileName );
    if ( outputFile.open( QFile::WriteOnly | QIODevice::Truncate ) )
    {
      QTextStream outputStream( &outputFile );
      outputStream << "# " << tr( "QGIS Generated Color Map Export File" ) << '\n';
      outputStream << "INTERPOLATION:";
      QgsColorRampShader::Type interpolation = static_cast< QgsColorRampShader::Type >( mColorInterpolationComboBox->currentData().toInt() );
      switch ( interpolation )
      {
        case QgsColorRampShader::Interpolated:
          outputStream << "INTERPOLATED\n";
          break;
        case QgsColorRampShader::Discrete:
          outputStream << "DISCRETE\n";
          break;
        case QgsColorRampShader::Exact:
          outputStream << "EXACT\n";
          break;
      }

      int topLevelItemCount = mColormapTreeWidget->topLevelItemCount();
      QTreeWidgetItem *currentItem = nullptr;
      QColor color;
      for ( int i = 0; i < topLevelItemCount; ++i )
      {
        currentItem = mColormapTreeWidget->topLevelItem( i );
        if ( !currentItem )
        {
          continue;
        }
        color = currentItem->background( ColorColumn ).color();
        outputStream << currentItem->text( ValueColumn ).toDouble() << ',';
        outputStream << color.red() << ',' << color.green() << ',' << color.blue() << ',' << color.alpha() << ',';
        if ( currentItem->text( LabelColumn ).isEmpty() )
        {
          outputStream << "Color entry " << i + 1 << '\n';
        }
        else
        {
          outputStream << currentItem->text( LabelColumn ) << '\n';
        }
      }
      outputStream.flush();
      outputFile.close();

      QFileInfo fileInfo( fileName );
      settings.setValue( QStringLiteral( "lastColorMapDir" ), fileInfo.absoluteDir().absolutePath() );
    }
    else
    {
      QMessageBox::warning( this, tr( "Write access denied" ), tr( "Write access denied. Adjust the file permissions and try again.\n\n" ) );
    }
  }
}

void QgsSingleBandPseudoColorRendererWidget::mColormapTreeWidget_itemDoubleClicked( QTreeWidgetItem *item, int column )
{
  if ( !item )
  {
    return;
  }

  if ( column == ColorColumn )
  {
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    QColor newColor = QgsColorDialog::getColor( item->background( column ).color(), this, QStringLiteral( "Change color" ), true );
    if ( newColor.isValid() )
    {
      item->setBackground( ColorColumn, QBrush( newColor ) );
      emit widgetChanged();
    }
  }
  else
  {
    if ( column == LabelColumn )
    {
      // Set text color to default black, which signifies a manually edited label
      item->setForeground( LabelColumn, QBrush() );
    }
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable );
  }
}

void QgsSingleBandPseudoColorRendererWidget::mColormapTreeWidget_itemEdited( QTreeWidgetItem *item, int column )
{
  Q_UNUSED( item );

  if ( column == ValueColumn )
  {
    mColormapTreeWidget->sortItems( ValueColumn, Qt::AscendingOrder );
    autoLabel();
    emit widgetChanged();
  }
  else if ( column == LabelColumn )
  {
    // call autoLabel to fill when empty or gray out when same as autoLabel
    autoLabel();
    emit widgetChanged();
  }
}

void QgsSingleBandPseudoColorRendererWidget::setFromRenderer( const QgsRasterRenderer *r )
{
  const QgsSingleBandPseudoColorRenderer *pr = dynamic_cast<const QgsSingleBandPseudoColorRenderer *>( r );
  if ( pr )
  {
    mBandComboBox->setBand( pr->band() );
    mMinMaxWidget->setBands( QList< int >() << pr->band() );

    const QgsRasterShader *rasterShader = pr->shader();
    if ( rasterShader )
    {
      const QgsColorRampShader *colorRampShader = dynamic_cast<const QgsColorRampShader *>( rasterShader->rasterShaderFunction() );
      if ( colorRampShader )
      {
        if ( colorRampShader->sourceColorRamp() )
        {
          btnColorRamp->setColorRamp( colorRampShader->sourceColorRamp() );
        }
        else
        {
          QgsSettings settings;
          QString defaultPalette = settings.value( QStringLiteral( "/Raster/defaultPalette" ), "Spectral" ).toString();
          btnColorRamp->setColorRampFromName( defaultPalette );
        }

        mColorInterpolationComboBox->setCurrentIndex( mColorInterpolationComboBox->findData( colorRampShader->colorRampType() ) );

        const QList<QgsColorRampShader::ColorRampItem> colorRampItemList = colorRampShader->colorRampItemList();
        QList<QgsColorRampShader::ColorRampItem>::const_iterator it = colorRampItemList.constBegin();
        for ( ; it != colorRampItemList.end(); ++it )
        {
          QgsTreeWidgetItemObject *newItem = new QgsTreeWidgetItemObject( mColormapTreeWidget );
          newItem->setText( ValueColumn, QString::number( it->value, 'g', 15 ) );
          newItem->setBackground( ColorColumn, QBrush( it->color ) );
          newItem->setText( LabelColumn, it->label );
          newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable );
          connect( newItem, &QgsTreeWidgetItemObject::itemEdited,
                   this, &QgsSingleBandPseudoColorRendererWidget::mColormapTreeWidget_itemEdited );
        }
        setUnitFromLabels();
        mClipCheckBox->setChecked( colorRampShader->clip() );

        mClassificationModeComboBox->setCurrentIndex( mClassificationModeComboBox->findData( colorRampShader->classificationMode() ) );
        mNumberOfEntriesSpinBox->setValue( colorRampShader->colorRampItemList().count() ); // some default
      }
    }
    setLineEditValue( mMinLineEdit, pr->classificationMin() );
    setLineEditValue( mMaxLineEdit, pr->classificationMax() );

    mMinMaxWidget->setFromMinMaxOrigin( pr->minMaxOrigin() );
  }
  else
  {
    mMinMaxWidget->setBands( QList< int >() << mBandComboBox->currentBand() );
  }
}

void QgsSingleBandPseudoColorRendererWidget::bandChanged()
{
  QList<int> bands;
  bands.append( mBandComboBox->currentBand() );
  mMinMaxWidget->setBands( bands );
}

void QgsSingleBandPseudoColorRendererWidget::mColorInterpolationComboBox_currentIndexChanged( int index )
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
      break;
    case QgsColorRampShader::Discrete:
      valueLabel = tr( "Value <=" );
      valueToolTip = tr( "Maximum value for class" );
      break;
    case QgsColorRampShader::Exact:
      valueLabel = tr( "Value =" );
      valueToolTip = tr( "Value for color" );
      break;
  }

  QTreeWidgetItem *header = mColormapTreeWidget->headerItem();
  header->setText( ValueColumn, valueLabel );
  header->setToolTip( ValueColumn, valueToolTip );

  autoLabel();
  emit widgetChanged();
}

void QgsSingleBandPseudoColorRendererWidget::loadMinMax( int bandNo, double min, double max )
{
  Q_UNUSED( bandNo );
  QgsDebugMsg( QString( "theBandNo = %1 min = %2 max = %3" ).arg( bandNo ).arg( min ).arg( max ) );

  mDisableMinMaxWidgetRefresh = true;
  if ( std::isnan( min ) )
  {
    mMinLineEdit->clear();
  }
  else
  {
    mMinLineEdit->setText( QString::number( min ) );
  }

  if ( std::isnan( max ) )
  {
    mMaxLineEdit->clear();
  }
  else
  {
    mMaxLineEdit->setText( QString::number( max ) );
  }
  mDisableMinMaxWidgetRefresh = false;
  classify();
}

void QgsSingleBandPseudoColorRendererWidget::setLineEditValue( QLineEdit *lineEdit, double value )
{
  QString s;
  if ( !std::isnan( value ) )
  {
    s = QString::number( value );
  }
  lineEdit->setText( s );
}

double QgsSingleBandPseudoColorRendererWidget::lineEditValue( const QLineEdit *lineEdit ) const
{
  if ( lineEdit->text().isEmpty() )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  return lineEdit->text().toDouble();
}

void QgsSingleBandPseudoColorRendererWidget::resetClassifyButton()
{
  mClassifyButton->setEnabled( true );
  double min = lineEditValue( mMinLineEdit );
  double max = lineEditValue( mMaxLineEdit );
  if ( std::isnan( min ) || std::isnan( max ) || min >= max )
  {
    mClassifyButton->setEnabled( false );
  }
}

void QgsSingleBandPseudoColorRendererWidget::changeColor()
{
  QList<QTreeWidgetItem *> itemList;
  itemList = mColormapTreeWidget->selectedItems();
  if ( itemList.isEmpty() )
  {
    return;
  }
  QTreeWidgetItem *firstItem = itemList.first();

  QColor newColor = QgsColorDialog::getColor( firstItem->background( ColorColumn ).color(), this, QStringLiteral( "Change color" ), true );
  if ( newColor.isValid() )
  {
    Q_FOREACH ( QTreeWidgetItem *item, itemList )
    {
      item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
      item->setBackground( ColorColumn, QBrush( newColor ) );
    }
    emit widgetChanged();
  }
}

void QgsSingleBandPseudoColorRendererWidget::changeOpacity()
{
  QList<QTreeWidgetItem *> itemList;
  itemList = mColormapTreeWidget->selectedItems();
  if ( itemList.isEmpty() )
  {
    return;
  }
  QTreeWidgetItem *firstItem = itemList.first();

  bool ok;
  double oldOpacity = firstItem->background( ColorColumn ).color().alpha() / 255 * 100;
  double opacity = QInputDialog::getDouble( this, tr( "Opacity" ), tr( "Change color opacity [%]" ), oldOpacity, 0.0, 100.0, 0, &ok );
  if ( ok )
  {
    int newOpacity = opacity / 100 * 255;
    Q_FOREACH ( QTreeWidgetItem *item, itemList )
    {
      QColor newColor = item->background( ColorColumn ).color();
      newColor.setAlpha( newOpacity );
      item->setBackground( ColorColumn, QBrush( newColor ) );
    }
    emit widgetChanged();
  }
}

void QgsSingleBandPseudoColorRendererWidget::mMinLineEdit_textEdited( const QString & )
{
  minMaxModified();
  classify();
}

void QgsSingleBandPseudoColorRendererWidget::mMaxLineEdit_textEdited( const QString & )
{
  minMaxModified();
  classify();
}

void QgsSingleBandPseudoColorRendererWidget::minMaxModified()
{
  if ( !mDisableMinMaxWidgetRefresh )
  {
    mMinMaxWidget->userHasSetManualMinMaxValues();
  }
}
