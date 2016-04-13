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

// for color ramps - todo add rasterStyle and refactor raster vs. vector ramps
#include "qgsstylev2.h"
#include "qgsvectorcolorrampv2.h"
#include "qgscolordialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTextStream>

// override setData to emit signal when edited. By default the itemChanged signal fires way too often
void QgsTreeWidgetItem::setData( int column, int role, const QVariant & value )
{
  QTreeWidgetItem::setData( column, role, value );
  if ( role == Qt::EditRole )
  {
    emit itemEdited( this, column );
  }
}

// override < operator to allow numeric sorting
/** Returns true if the text in the item is less than the text in the other item, otherwise returns false.
 *
 *  Compares on numeric value of text if possible, otherwise on text.
 */
bool QgsTreeWidgetItem::operator<( const QTreeWidgetItem & other ) const
{
  int column = treeWidget()->sortColumn();
  bool ok1, ok2, val;
  val = text( column ).toDouble( &ok1 ) < other.text( column ).toDouble( &ok2 );
  if ( ok1 && ok2 )
  {
    return val;
  }
  else if ( ok1 || ok2 )
  {
    // sort numbers before strings
    return ok1;
  }
  else
  {
    return text( column ) < other.text( column );
  }
}

QgsSingleBandPseudoColorRendererWidget::QgsSingleBandPseudoColorRendererWidget( QgsRasterLayer* layer, const QgsRectangle &extent )
    : QgsRasterRendererWidget( layer, extent )
    , mMinMaxWidget( nullptr )
    , mMinMaxOrigin( 0 )
{
  QSettings settings;

  setupUi( this );

  mColormapTreeWidget->setColumnWidth( ColorColumn, 50 );

  QString defaultPalette = settings.value( "/Raster/defaultPalette", "Spectral" ).toString();

  mColorRampComboBox->populate( QgsStyleV2::defaultStyle() );

  QgsDebugMsg( "defaultPalette = " + defaultPalette );
  mColorRampComboBox->setCurrentIndex( mColorRampComboBox->findText( defaultPalette ) );
  connect( mButtonEditRamp, SIGNAL( clicked() ), mColorRampComboBox, SLOT( editSourceRamp() ) );

  if ( !mRasterLayer )
  {
    return;
  }

  QgsRasterDataProvider* provider = mRasterLayer->dataProvider();
  if ( !provider )
  {
    return;
  }

  // Must be before adding items to mBandComboBox (signal)
  mMinLineEdit->setValidator( new QDoubleValidator( mMinLineEdit ) );
  mMaxLineEdit->setValidator( new QDoubleValidator( mMaxLineEdit ) );

  mMinMaxWidget = new QgsRasterMinMaxWidget( layer, this );
  mMinMaxWidget->setExtent( extent );
  QHBoxLayout *layout = new QHBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  mMinMaxContainerWidget->setLayout( layout );
  layout->addWidget( mMinMaxWidget );
  connect( mMinMaxWidget, SIGNAL( load( int, double, double, int ) ),
           this, SLOT( loadMinMax( int, double, double, int ) ) );


  //fill available bands into combo box
  int nBands = provider->bandCount();
  for ( int i = 1; i <= nBands; ++i ) //band numbering seem to start at 1
  {
    mBandComboBox->addItem( displayBandName( i ), i );
  }

  mColorInterpolationComboBox->addItem( tr( "Discrete" ), 0 );
  mColorInterpolationComboBox->addItem( tr( "Linear" ), 1 );
  mColorInterpolationComboBox->addItem( tr( "Exact" ), 2 );
  mColorInterpolationComboBox->setCurrentIndex( 1 );
  mClassificationModeComboBox->addItem( tr( "Continuous" ), Continuous );
  mClassificationModeComboBox->addItem( tr( "Equal interval" ), EqualInterval );
  mClassificationModeComboBox->addItem( tr( "Quantile" ), Quantile );

  mNumberOfEntriesSpinBox->setValue( 5 ); // some default

  setFromRenderer( layer->renderer() );

  // If there is currently no min/max, load default with user current default options
  if ( mMinLineEdit->text().isEmpty() || mMaxLineEdit->text().isEmpty() )
  {
    mMinMaxWidget->load();
  }

  on_mClassificationModeComboBox_currentIndexChanged( 0 );

  resetClassifyButton();
}

QgsSingleBandPseudoColorRendererWidget::~QgsSingleBandPseudoColorRendererWidget()
{
}

QgsRasterRenderer* QgsSingleBandPseudoColorRendererWidget::renderer()
{
  QgsRasterShader* rasterShader = new QgsRasterShader();
  QgsColorRampShader* colorRampShader = new QgsColorRampShader();
  colorRampShader->setClip( mClipCheckBox->isChecked() );

  //iterate through mColormapTreeWidget and set colormap info of layer
  QList<QgsColorRampShader::ColorRampItem> colorRampItems;
  int topLevelItemCount = mColormapTreeWidget->topLevelItemCount();
  QTreeWidgetItem* currentItem;
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
  qSort( colorRampItems );
  colorRampShader->setColorRampItemList( colorRampItems );

  if ( mColorInterpolationComboBox->currentText() == tr( "Linear" ) )
  {
    colorRampShader->setColorRampType( QgsColorRampShader::INTERPOLATED );
  }
  else if ( mColorInterpolationComboBox->currentText() == tr( "Discrete" ) )
  {
    colorRampShader->setColorRampType( QgsColorRampShader::DISCRETE );
  }
  else
  {
    colorRampShader->setColorRampType( QgsColorRampShader::EXACT );
  }
  rasterShader->setRasterShaderFunction( colorRampShader );

  int bandNumber = mBandComboBox->itemData( mBandComboBox->currentIndex() ).toInt();
  QgsSingleBandPseudoColorRenderer *renderer = new QgsSingleBandPseudoColorRenderer( mRasterLayer->dataProvider(), bandNumber, rasterShader );

  renderer->setClassificationMin( lineEditValue( mMinLineEdit ) );
  renderer->setClassificationMax( lineEditValue( mMaxLineEdit ) );
  renderer->setClassificationMinMaxOrigin( mMinMaxOrigin );
  return renderer;
}

/** Generate labels from the values in the color map.
 *  Skip labels which were manually edited (black text).
 *  Text of generated labels is made gray
 */
void QgsSingleBandPseudoColorRendererWidget::autoLabel()
{
  bool discrete = mColorInterpolationComboBox->currentText() == tr( "Discrete" );
  QString unit = mUnitLineEdit->text();
  QString label;
  int topLevelItemCount = mColormapTreeWidget->topLevelItemCount();
  QTreeWidgetItem* currentItem;
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

/** Extract the unit out of the current labels and set the unit field. */
void QgsSingleBandPseudoColorRendererWidget::setUnitFromLabels()
{
  bool discrete = mColorInterpolationComboBox->currentText() == tr( "Discrete" );
  QStringList allSuffixes;
  QString label;
  int topLevelItemCount = mColormapTreeWidget->topLevelItemCount();
  QTreeWidgetItem* currentItem;
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

void QgsSingleBandPseudoColorRendererWidget::on_mAddEntryButton_clicked()
{
  QgsTreeWidgetItem* newItem = new QgsTreeWidgetItem( mColormapTreeWidget );
  newItem->setText( ValueColumn, "0" );
  newItem->setBackground( ColorColumn, QBrush( QColor( Qt::magenta ) ) );
  newItem->setText( LabelColumn, QString() );
  newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable );
  connect( newItem, SIGNAL( itemEdited( QTreeWidgetItem*, int ) ),
           this, SLOT( mColormapTreeWidget_itemEdited( QTreeWidgetItem*, int ) ) );
  mColormapTreeWidget->sortItems( ValueColumn, Qt::AscendingOrder );
  autoLabel();
}

void QgsSingleBandPseudoColorRendererWidget::on_mDeleteEntryButton_clicked()
{
  QTreeWidgetItem* currentItem = mColormapTreeWidget->currentItem();
  if ( currentItem )
  {
    delete currentItem;
  }
}

void QgsSingleBandPseudoColorRendererWidget::on_mClassifyButton_clicked()
{
  int bandComboIndex = mBandComboBox->currentIndex();
  if ( bandComboIndex == -1 || !mRasterLayer )
  {
    return;
  }

  //int bandNr = mBandComboBox->itemData( bandComboIndex ).toInt();
  //QgsRasterBandStats myRasterBandStats = mRasterLayer->dataProvider()->bandStatistics( bandNr );
  int numberOfEntries = 0;

  QList<double> entryValues;
  QVector<QColor> entryColors;

  double min = lineEditValue( mMinLineEdit );
  double max = lineEditValue( mMaxLineEdit );

  QgsVectorColorRampV2* colorRamp = mColorRampComboBox->currentColorRamp();

  if ( mClassificationModeComboBox->itemData( mClassificationModeComboBox->currentIndex() ).toInt() == Continuous )
  {
    if ( colorRamp )
    {
      numberOfEntries = colorRamp->count();
      entryValues.reserve( colorRamp->count() );
      for ( int i = 0; i < colorRamp->count(); ++i )
      {
        double value = colorRamp->value( i );
        entryValues.push_back( min + value * ( max - min ) );
      }
    }
  }
  else if ( mClassificationModeComboBox->itemData( mClassificationModeComboBox->currentIndex() ).toInt() == Quantile )
  { // Quantile
    mMinMaxWidget->load();

    numberOfEntries = mNumberOfEntriesSpinBox->value();

    int bandNr = mBandComboBox->itemData( bandComboIndex ).toInt();
    //QgsRasterHistogram rasterHistogram = mRasterLayer->dataProvider()->histogram( bandNr );

    double cut1 = std::numeric_limits<double>::quiet_NaN();
    double cut2 = std::numeric_limits<double>::quiet_NaN();

    QgsRectangle extent = mMinMaxWidget->extent();
    int sampleSize = mMinMaxWidget->sampleSize();

    double intervalDiff;
    if ( numberOfEntries > 1 )
    {
      intervalDiff = 1.0 / ( numberOfEntries - 1 );
      entryValues.reserve( numberOfEntries );
      for ( int i = 0; i < numberOfEntries; ++i )
      {
        mRasterLayer->dataProvider()->cumulativeCut( bandNr, 0.0, i * intervalDiff, cut1, cut2, extent, sampleSize );
        entryValues.push_back( cut2 );
      }
    }
    else if ( numberOfEntries == 1 )
    {
      mRasterLayer->dataProvider()->cumulativeCut( bandNr, 0.0, 0.5, cut1, cut2, extent, sampleSize );
      entryValues.push_back( cut2 );
    }
  }
  else // EqualInterval
  {
    numberOfEntries = mNumberOfEntriesSpinBox->value();
    //double currentValue = myRasterBandStats.minimumValue;
    double currentValue = min;
    double intervalDiff;
    if ( numberOfEntries > 1 )
    {
      //because the highest value is also an entry, there are (numberOfEntries - 1)
      //intervals
      //intervalDiff = ( myRasterBandStats.maximumValue - myRasterBandStats.minimumValue ) /
      intervalDiff = ( max - min ) / ( numberOfEntries - 1 );
    }
    else
    {
      //intervalDiff = myRasterBandStats.maximumValue - myRasterBandStats.minimumValue;
      intervalDiff = max - min;
    }

    entryValues.reserve( numberOfEntries );
    for ( int i = 0; i < numberOfEntries; ++i )
    {
      entryValues.push_back( currentValue );
      currentValue += intervalDiff;
    }
  }

#if 0
  //hard code color range from blue -> red for now. Allow choice of ramps in future
  int colorDiff = 0;
  if ( numberOfEntries != 0 )
  {
    colorDiff = ( int )( 255 / numberOfEntries );
  }
  for ( int i = 0; i < numberOfEntries; ++i )
  {
    QColor currentColor;
    currentColor.setRgb( colorDiff*i, 0, 255 - colorDiff * i );
    entryColors.push_back( currentColor );
  }
#endif

  if ( ! colorRamp )
  {
    //hard code color range from blue -> red (previous default)
    int colorDiff = 0;
    if ( numberOfEntries != 0 )
    {
      colorDiff = ( int )( 255 / numberOfEntries );
    }

    entryColors.reserve( numberOfEntries );
    for ( int i = 0; i < numberOfEntries; ++i )
    {
      QColor currentColor;
      int idx = mInvertCheckBox->isChecked() ? numberOfEntries - i - 1 : i;
      currentColor.setRgb( colorDiff*idx, 0, 255 - colorDiff * idx );
      entryColors.push_back( currentColor );
    }
  }
  else
  {
    entryColors.reserve( numberOfEntries );
    for ( int i = 0; i < numberOfEntries; ++i )
    {
      int idx = mInvertCheckBox->isChecked() ? numberOfEntries - i - 1 : i;
      entryColors.push_back( colorRamp->color((( double ) idx ) / ( numberOfEntries - 1 ) ) );
    }
  }

  mColormapTreeWidget->clear();

  QList<double>::const_iterator value_it = entryValues.begin();
  QVector<QColor>::const_iterator color_it = entryColors.begin();

  // calculate a reasonable number of decimals to display
  double maxabs = log10( qMax( qAbs( max ), qAbs( min ) ) );
  int nDecimals = qRound( qMax( 3.0 + maxabs - log10( max - min ), maxabs <= 6.0 ? maxabs + 0.49 : 0.0 ) );

  for ( ; value_it != entryValues.end(); ++value_it, ++color_it )
  {
    QgsTreeWidgetItem* newItem = new QgsTreeWidgetItem( mColormapTreeWidget );
    newItem->setText( ValueColumn, QString::number( *value_it, 'g', nDecimals ) );
    newItem->setBackground( ColorColumn, QBrush( *color_it ) );
    newItem->setText( LabelColumn, QString() );
    newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable );
    connect( newItem, SIGNAL( itemEdited( QTreeWidgetItem*, int ) ),
             this, SLOT( mColormapTreeWidget_itemEdited( QTreeWidgetItem*, int ) ) );
  }
  autoLabel();
}

void QgsSingleBandPseudoColorRendererWidget::on_mClassificationModeComboBox_currentIndexChanged( int index )
{
  mNumberOfEntriesSpinBox->setEnabled( mClassificationModeComboBox->itemData( index ).toInt() != Continuous );
}

void QgsSingleBandPseudoColorRendererWidget::on_mColorRampComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  QSettings settings;
  settings.setValue( "/Raster/defaultPalette", mColorRampComboBox->currentText() );

  QgsVectorColorRampV2* ramp = mColorRampComboBox->currentColorRamp();
  if ( !ramp )
    return;

  bool enableContinuous = ( ramp->count() > 0 );
  mClassificationModeComboBox->setEnabled( enableContinuous );
  if ( !enableContinuous )
  {
    mClassificationModeComboBox->setCurrentIndex( mClassificationModeComboBox->findData( EqualInterval ) );
  }
}

void QgsSingleBandPseudoColorRendererWidget::populateColormapTreeWidget( const QList<QgsColorRampShader::ColorRampItem>& colorRampItems )
{
  mColormapTreeWidget->clear();
  QList<QgsColorRampShader::ColorRampItem>::const_iterator it = colorRampItems.constBegin();
  for ( ; it != colorRampItems.constEnd(); ++it )
  {
    QgsTreeWidgetItem* newItem = new QgsTreeWidgetItem( mColormapTreeWidget );
    newItem->setText( ValueColumn, QString::number( it->value, 'g' ) );
    newItem->setBackground( ColorColumn, QBrush( it->color ) );
    newItem->setText( LabelColumn, it->label );
    newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable );
    connect( newItem, SIGNAL( itemEdited( QTreeWidgetItem*, int ) ),
             this, SLOT( mColormapTreeWidget_itemEdited( QTreeWidgetItem*, int ) ) );
  }
  setUnitFromLabels();
}

void QgsSingleBandPseudoColorRendererWidget::on_mLoadFromBandButton_clicked()
{
  if ( !mRasterLayer || !mRasterLayer->dataProvider() )
  {
    return;
  }

  int bandIndex = mBandComboBox->itemData( mBandComboBox->currentIndex() ).toInt();


  QList<QgsColorRampShader::ColorRampItem> colorRampList = mRasterLayer->dataProvider()->colorTable( bandIndex );
  if ( !colorRampList.isEmpty() )
  {
    populateColormapTreeWidget( colorRampList );
    mColorInterpolationComboBox->setCurrentIndex( mColorInterpolationComboBox->findText( tr( "Linear" ) ) );
  }
  else
  {
    QMessageBox::warning( this, tr( "Load Color Map" ), tr( "The color map for band %1 has no entries" ).arg( bandIndex ) );
  }
}

void QgsSingleBandPseudoColorRendererWidget::on_mLoadFromFileButton_clicked()
{
  int lineCounter = 0;
  bool importError = false;
  QString badLines;
  QSettings settings;
  QString lastDir = settings.value( "lastColorMapDir", QDir::homePath() ).toString();
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
          if ( inputLine.contains( "INTERPOLATION", Qt::CaseInsensitive ) )
          {
            inputStringComponents = inputLine.split( ':' );
            if ( inputStringComponents.size() == 2 )
            {
              if ( inputStringComponents[1].trimmed().toUpper().compare( "INTERPOLATED", Qt::CaseInsensitive ) == 0 )
              {
                mColorInterpolationComboBox->setCurrentIndex( mColorInterpolationComboBox->findText( tr( "Linear" ) ) );
              }
              else if ( inputStringComponents[1].trimmed().toUpper().compare( "DISCRETE", Qt::CaseInsensitive ) == 0 )
              {
                mColorInterpolationComboBox->setCurrentIndex( mColorInterpolationComboBox->findText( tr( "Discrete" ) ) );
              }
              else
              {
                mColorInterpolationComboBox->setCurrentIndex( mColorInterpolationComboBox->findText( tr( "Exact" ) ) );
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
    settings.setValue( "lastColorMapDir", fileInfo.absoluteDir().absolutePath() );

    if ( importError )
    {
      QMessageBox::warning( this, tr( "Import Error" ), tr( "The following lines contained errors\n\n" ) + badLines );
    }
  }
  else if ( !fileName.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Read access denied" ), tr( "Read access denied. Adjust the file permissions and try again.\n\n" ) );
  }
}

void QgsSingleBandPseudoColorRendererWidget::on_mExportToFileButton_clicked()
{
  QSettings settings;
  QString lastDir = settings.value( "lastColorMapDir", QDir::homePath() ).toString();
  QString fileName = QFileDialog::getSaveFileName( this, tr( "Save file" ), lastDir, tr( "Textfile (*.txt)" ) );
  if ( !fileName.isEmpty() )
  {
    if ( !fileName.endsWith( ".txt", Qt::CaseInsensitive ) )
    {
      fileName = fileName + ".txt";
    }

    QFile outputFile( fileName );
    if ( outputFile.open( QFile::WriteOnly ) )
    {
      QTextStream outputStream( &outputFile );
      outputStream << "# " << tr( "QGIS Generated Color Map Export File" ) << '\n';
      outputStream << "INTERPOLATION:";
      if ( mColorInterpolationComboBox->currentText() == tr( "Linear" ) )
      {
        outputStream << "INTERPOLATED\n";
      }
      else if ( mColorInterpolationComboBox->currentText() == tr( "Discrete" ) )
      {
        outputStream << "DISCRETE\n";
      }
      else
      {
        outputStream << "EXACT\n";
      }

      int topLevelItemCount = mColormapTreeWidget->topLevelItemCount();
      QTreeWidgetItem* currentItem;
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
      settings.setValue( "lastColorMapDir", fileInfo.absoluteDir().absolutePath() );
    }
    else
    {
      QMessageBox::warning( this, tr( "Write access denied" ), tr( "Write access denied. Adjust the file permissions and try again.\n\n" ) );
    }
  }
}

void QgsSingleBandPseudoColorRendererWidget::on_mColormapTreeWidget_itemDoubleClicked( QTreeWidgetItem* item, int column )
{
  if ( !item )
  {
    return;
  }

  if ( column == ColorColumn )
  {
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    QColor newColor = QgsColorDialogV2::getColor( item->background( column ).color(), this, "Change color", true );
    if ( newColor.isValid() )
    {
      item->setBackground( ColorColumn, QBrush( newColor ) );
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

/** Update the colormap table after manual edit. */
void QgsSingleBandPseudoColorRendererWidget::mColormapTreeWidget_itemEdited( QTreeWidgetItem* item, int column )
{
  Q_UNUSED( item );

  if ( column == ValueColumn )
  {
    mColormapTreeWidget->sortItems( ValueColumn, Qt::AscendingOrder );
    autoLabel();
  }
  else if ( column == LabelColumn )
  {
    // call autoLabel to fill when empty or gray out when same as autoLabel
    autoLabel();
  }
}

void QgsSingleBandPseudoColorRendererWidget::setFromRenderer( const QgsRasterRenderer* r )
{
  const QgsSingleBandPseudoColorRenderer* pr = dynamic_cast<const QgsSingleBandPseudoColorRenderer*>( r );
  if ( pr )
  {
    mBandComboBox->setCurrentIndex( mBandComboBox->findData( pr->band() ) );

    const QgsRasterShader* rasterShader = pr->shader();
    if ( rasterShader )
    {
      const QgsColorRampShader* colorRampShader = dynamic_cast<const QgsColorRampShader*>( rasterShader->rasterShaderFunction() );
      if ( colorRampShader )
      {
        if ( colorRampShader->colorRampType() == QgsColorRampShader::INTERPOLATED )
        {
          mColorInterpolationComboBox->setCurrentIndex( mColorInterpolationComboBox->findText( tr( "Linear" ) ) );
        }
        else if ( colorRampShader->colorRampType() == QgsColorRampShader::DISCRETE )
        {
          mColorInterpolationComboBox->setCurrentIndex( mColorInterpolationComboBox->findText( tr( "Discrete" ) ) );
        }
        else
        {
          mColorInterpolationComboBox->setCurrentIndex( mColorInterpolationComboBox->findText( tr( "Exact" ) ) );
        }

        const QList<QgsColorRampShader::ColorRampItem> colorRampItemList = colorRampShader->colorRampItemList();
        QList<QgsColorRampShader::ColorRampItem>::const_iterator it = colorRampItemList.constBegin();
        for ( ; it != colorRampItemList.end(); ++it )
        {
          QgsTreeWidgetItem* newItem = new QgsTreeWidgetItem( mColormapTreeWidget );
          newItem->setText( ValueColumn, QString::number( it->value, 'g' ) );
          newItem->setBackground( ColorColumn, QBrush( it->color ) );
          newItem->setText( LabelColumn, it->label );
          newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable );
          connect( newItem, SIGNAL( itemEdited( QTreeWidgetItem*, int ) ),
                   this, SLOT( mColormapTreeWidget_itemEdited( QTreeWidgetItem*, int ) ) );
        }
        setUnitFromLabels();
        mClipCheckBox->setChecked( colorRampShader->clip() );
      }
    }
    setLineEditValue( mMinLineEdit, pr->classificationMin() );
    setLineEditValue( mMaxLineEdit, pr->classificationMax() );
    mMinMaxOrigin = pr->classificationMinMaxOrigin();
    showMinMaxOrigin();
  }
}

void QgsSingleBandPseudoColorRendererWidget::on_mBandComboBox_currentIndexChanged( int index )
{
  QList<int> bands;
  bands.append( mBandComboBox->itemData( index ).toInt() );
  mMinMaxWidget->setBands( bands );
}

void QgsSingleBandPseudoColorRendererWidget::loadMinMax( int theBandNo, double theMin, double theMax, int theOrigin )
{
  Q_UNUSED( theBandNo );
  QgsDebugMsg( QString( "theBandNo = %1 theMin = %2 theMax = %3" ).arg( theBandNo ).arg( theMin ).arg( theMax ) );

  if ( qIsNaN( theMin ) )
  {
    mMinLineEdit->clear();
  }
  else
  {
    mMinLineEdit->setText( QString::number( theMin ) );
  }

  if ( qIsNaN( theMax ) )
  {
    mMaxLineEdit->clear();
  }
  else
  {
    mMaxLineEdit->setText( QString::number( theMax ) );
  }

  mMinMaxOrigin = theOrigin;
  showMinMaxOrigin();
}

void QgsSingleBandPseudoColorRendererWidget::showMinMaxOrigin()
{
  mMinMaxOriginLabel->setText( QgsRasterRenderer::minMaxOriginLabel( mMinMaxOrigin ) );
}

void QgsSingleBandPseudoColorRendererWidget::setLineEditValue( QLineEdit * theLineEdit, double theValue )
{
  QString s;
  if ( !qIsNaN( theValue ) )
  {
    s = QString::number( theValue );
  }
  theLineEdit->setText( s );
}

double QgsSingleBandPseudoColorRendererWidget::lineEditValue( const QLineEdit * theLineEdit ) const
{
  if ( theLineEdit->text().isEmpty() )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  return theLineEdit->text().toDouble();
}

void QgsSingleBandPseudoColorRendererWidget::resetClassifyButton()
{
  mClassifyButton->setEnabled( true );
  double min = lineEditValue( mMinLineEdit );
  double max = lineEditValue( mMaxLineEdit );
  if ( qIsNaN( min ) || qIsNaN( max ) || min >= max )
  {
    mClassifyButton->setEnabled( false );
  }
}
