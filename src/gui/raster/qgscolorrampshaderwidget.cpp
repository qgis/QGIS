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

  contextMenu = new QMenu( tr( "Options" ), this );
  contextMenu->addAction( tr( "Change Color…" ), this, SLOT( changeColor() ) );
  contextMenu->addAction( tr( "Change Opacity…" ), this, SLOT( changeOpacity() ) );

  mColormapTreeWidget->setColumnWidth( ColorColumn, 50 );
  mColormapTreeWidget->setContextMenuPolicy( Qt::CustomContextMenu );
  mColormapTreeWidget->setSelectionMode( QAbstractItemView::ExtendedSelection );
  connect( mColormapTreeWidget, &QTreeView::customContextMenuRequested, this, [ = ]( QPoint ) { contextMenu->exec( QCursor::pos() ); }
         );

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
  connect( mClassifyButton, &QPushButton::clicked, this, &QgsColorRampShaderWidget::applyColorRamp );
  connect( btnColorRamp, &QgsColorRampButton::colorRampChanged, this, &QgsColorRampShaderWidget::applyColorRamp );
  connect( mNumberOfEntriesSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsColorRampShaderWidget::classify );
  connect( mClipCheckBox, &QAbstractButton::toggled, this, &QgsColorRampShaderWidget::widgetChanged );
}

void QgsColorRampShaderWidget::initializeForUseWithRasterLayer()
{
  Q_ASSERT( mClassificationModeComboBox->findData( QgsColorRampShader::Quantile < 0 ) );
  mClassificationModeComboBox->addItem( tr( "Quantile" ), QgsColorRampShader::Quantile );
}

void QgsColorRampShaderWidget::setRasterDataProvider( QgsRasterDataProvider *dp )
{
  mRasterDataProvider = dp;
  mLoadFromBandButton->setVisible( bool( mRasterDataProvider ) ); // only for raster version
}

void QgsColorRampShaderWidget::setRasterBand( int band )
{
  mBand = band;
}

void QgsColorRampShaderWidget::setExtent( const QgsRectangle &extent )
{
  mExtent = extent;
}

QgsColorRampShader QgsColorRampShaderWidget::shader() const
{
  QgsColorRampShader colorRampShader( mMin, mMax );
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
    newColorRampItem.value = currentItem->text( ValueColumn ).toDouble();
    newColorRampItem.color = currentItem->background( ColorColumn ).color();
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
  return colorRampShader;
}

void QgsColorRampShaderWidget::autoLabel()
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

void QgsColorRampShaderWidget::setUnitFromLabels()
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


void QgsColorRampShaderWidget::mAddEntryButton_clicked()
{
  QgsTreeWidgetItemObject *newItem = new QgsTreeWidgetItemObject( mColormapTreeWidget );
  newItem->setText( ValueColumn, QStringLiteral( "0" ) );
  newItem->setBackground( ColorColumn, QBrush( QColor( Qt::magenta ) ) );
  newItem->setText( LabelColumn, QString() );
  newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable );
  connect( newItem, &QgsTreeWidgetItemObject::itemEdited,
           this, &QgsColorRampShaderWidget::mColormapTreeWidget_itemEdited );
  mColormapTreeWidget->sortItems( ValueColumn, Qt::AscendingOrder );
  autoLabel();

  loadMinimumMaximumFromTree();
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

  Q_FOREACH ( QTreeWidgetItem *item, itemList )
  {
    delete item;
  }

  loadMinimumMaximumFromTree();
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
    newItem->setText( ValueColumn, QString::number( it->value, 'g', 15 ) );
    newItem->setBackground( ColorColumn, QBrush( it->color ) );
    newItem->setText( LabelColumn, it->label );
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

  classify();
}

void QgsColorRampShaderWidget::populateColormapTreeWidget( const QList<QgsColorRampShader::ColorRampItem> &colorRampItems )
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
             this, &QgsColorRampShaderWidget::mColormapTreeWidget_itemEdited );
  }
  setUnitFromLabels();

  autoLabel();
  emit widgetChanged();
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
  int lineCounter = 0;
  bool importError = false;
  QString badLines;
  QgsSettings settings;
  QString lastDir = settings.value( QStringLiteral( "lastColorMapDir" ), QDir::homePath() ).toString();
  QString fileName = QFileDialog::getOpenFileName( this, tr( "Load Color Map from File" ), lastDir, tr( "Textfile (*.txt)" ) );
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
      QMessageBox::warning( this, tr( "Load Color Map from File" ), tr( "The following lines contained errors\n\n" ) + badLines );
    }
  }
  else if ( !fileName.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Load Color Map from File" ), tr( "Read access denied. Adjust the file permissions and try again.\n\n" ) );
  }

  loadMinimumMaximumFromTree();
  emit widgetChanged();
}

void QgsColorRampShaderWidget::mExportToFileButton_clicked()
{
  QgsSettings settings;
  QString lastDir = settings.value( QStringLiteral( "lastColorMapDir" ), QDir::homePath() ).toString();
  QString fileName = QFileDialog::getSaveFileName( this, tr( "Save Color Map as File" ), lastDir, tr( "Textfile (*.txt)" ) );
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
      QMessageBox::warning( this, tr( "Save Color Map as File" ), tr( "Write access denied. Adjust the file permissions and try again.\n\n" ) );
    }
  }
}

void QgsColorRampShaderWidget::mColormapTreeWidget_itemDoubleClicked( QTreeWidgetItem *item, int column )
{
  if ( !item )
  {
    return;
  }

  if ( column == ColorColumn )
  {
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    QColor newColor = QgsColorDialog::getColor( item->background( column ).color(), this, QStringLiteral( "Change Color" ), true );
    if ( newColor.isValid() )
    {
      item->setBackground( ColorColumn, QBrush( newColor ) );
      loadMinimumMaximumFromTree();
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

void QgsColorRampShaderWidget::mColormapTreeWidget_itemEdited( QTreeWidgetItem *item, int column )
{
  Q_UNUSED( item );

  if ( column == ValueColumn )
  {
    mColormapTreeWidget->sortItems( ValueColumn, Qt::AscendingOrder );
    autoLabel();

    loadMinimumMaximumFromTree();

    emit widgetChanged();
  }
  else if ( column == LabelColumn )
  {
    // call autoLabel to fill when empty or gray out when same as autoLabel
    autoLabel();
    emit widgetChanged();
  }
}

void QgsColorRampShaderWidget::setFromShader( const QgsColorRampShader &colorRampShader )
{
  if ( colorRampShader.sourceColorRamp() )
  {
    btnColorRamp->setColorRamp( colorRampShader.sourceColorRamp() );
  }
  else
  {
    QgsSettings settings;
    QString defaultPalette = settings.value( QStringLiteral( "/Raster/defaultPalette" ), "Spectral" ).toString();
    btnColorRamp->setColorRampFromName( defaultPalette );
  }

  mColorInterpolationComboBox->setCurrentIndex( mColorInterpolationComboBox->findData( colorRampShader.colorRampType() ) );

  mColormapTreeWidget->clear();
  const QList<QgsColorRampShader::ColorRampItem> colorRampItemList = colorRampShader.colorRampItemList();
  QList<QgsColorRampShader::ColorRampItem>::const_iterator it = colorRampItemList.constBegin();
  for ( ; it != colorRampItemList.end(); ++it )
  {
    QgsTreeWidgetItemObject *newItem = new QgsTreeWidgetItemObject( mColormapTreeWidget );
    newItem->setText( ValueColumn, QString::number( it->value, 'g', 15 ) );
    newItem->setBackground( ColorColumn, QBrush( it->color ) );
    newItem->setText( LabelColumn, it->label );
    newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable );
    connect( newItem, &QgsTreeWidgetItemObject::itemEdited,
             this, &QgsColorRampShaderWidget::mColormapTreeWidget_itemEdited );
  }
  setUnitFromLabels();
  mClipCheckBox->setChecked( colorRampShader.clip() );
  mClassificationModeComboBox->setCurrentIndex( mClassificationModeComboBox->findData( colorRampShader.classificationMode() ) );
  mNumberOfEntriesSpinBox->setValue( colorRampShader.colorRampItemList().count() ); // some default
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



void QgsColorRampShaderWidget::loadMinimumMaximumFromTree()
{
  QTreeWidgetItem *item = mColormapTreeWidget->topLevelItem( 0 );
  if ( !item )
  {
    return;
  }

  double min = item->text( ValueColumn ).toDouble();
  item = mColormapTreeWidget->topLevelItem( mColormapTreeWidget->topLevelItemCount() - 1 );
  double max = item->text( ValueColumn ).toDouble();

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

void QgsColorRampShaderWidget::changeColor()
{
  QList<QTreeWidgetItem *> itemList;
  itemList = mColormapTreeWidget->selectedItems();
  if ( itemList.isEmpty() )
  {
    return;
  }
  QTreeWidgetItem *firstItem = itemList.first();

  QColor newColor = QgsColorDialog::getColor( firstItem->background( ColorColumn ).color(), this, QStringLiteral( "Change Color" ), true );
  if ( newColor.isValid() )
  {
    Q_FOREACH ( QTreeWidgetItem *item, itemList )
    {
      item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
      item->setBackground( ColorColumn, QBrush( newColor ) );
    }

    loadMinimumMaximumFromTree();
    emit widgetChanged();
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
  double oldOpacity = firstItem->background( ColorColumn ).color().alpha() / 255 * 100;
  double opacity = QInputDialog::getDouble( this, tr( "Opacity" ), tr( "Change color opacity [%]" ), oldOpacity, 0.0, 100.0, 0, &ok );
  if ( ok )
  {
    int newOpacity = static_cast<int>( opacity / 100 * 255 );
    Q_FOREACH ( QTreeWidgetItem *item, itemList )
    {
      QColor newColor = item->background( ColorColumn ).color();
      newColor.setAlpha( newOpacity );
      item->setBackground( ColorColumn, QBrush( newColor ) );
    }

    loadMinimumMaximumFromTree();
    emit widgetChanged();
  }
}
