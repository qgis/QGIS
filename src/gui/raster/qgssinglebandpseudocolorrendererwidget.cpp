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

#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTextStream>

QgsSingleBandPseudoColorRendererWidget::QgsSingleBandPseudoColorRendererWidget( QgsRasterLayer* layer, const QgsRectangle &extent ):
    QgsRasterRendererWidget( layer, extent )
{
  QSettings settings;

  setupUi( this );

  mColormapTreeWidget->setColumnWidth( 1, 50 );

  QString defaultPalette = settings.value( "/Raster/defaultPalette", "Spectral" ).toString();

  mColorRampComboBox->populate( QgsStyleV2::defaultStyle() );

  QgsDebugMsg( "defaultPalette = " + defaultPalette );
  mColorRampComboBox->setCurrentIndex( mColorRampComboBox->findText( defaultPalette ) );

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
  //quantile would be nice as well

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
    newColorRampItem.value = currentItem->text( 0 ).toDouble();
    newColorRampItem.color = currentItem->background( 1 ).color();
    newColorRampItem.label = currentItem->text( 2 );
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

void QgsSingleBandPseudoColorRendererWidget::on_mAddEntryButton_clicked()
{
  QTreeWidgetItem* newItem = new QTreeWidgetItem( mColormapTreeWidget );
  newItem->setText( 0, "0.0" );
  newItem->setBackground( 1, QBrush( QColor( Qt::magenta ) ) );
  newItem->setText( 2, tr( "Custom color map entry" ) );
}

void QgsSingleBandPseudoColorRendererWidget::on_mDeleteEntryButton_clicked()
{
  QTreeWidgetItem* currentItem = mColormapTreeWidget->currentItem();
  if ( currentItem )
  {
    delete currentItem;
  }
}

void QgsSingleBandPseudoColorRendererWidget::on_mSortButton_clicked()
{
  bool inserted = false;
  int myCurrentIndex = 0;
  int myTopLevelItemCount = mColormapTreeWidget->topLevelItemCount();
  QTreeWidgetItem* myCurrentItem;
  QList<QgsColorRampShader::ColorRampItem> myColorRampItems;
  for ( int i = 0; i < myTopLevelItemCount; ++i )
  {
    myCurrentItem = mColormapTreeWidget->topLevelItem( i );
    //If the item is null or does not have a pixel values set, skip
    if ( !myCurrentItem || myCurrentItem->text( 0 ) == "" )
    {
      continue;
    }

    //Create a copy of the new Color ramp Item
    QgsColorRampShader::ColorRampItem myNewColorRampItem;
    myNewColorRampItem.value = myCurrentItem->text( 0 ).toDouble();
    myNewColorRampItem.color = myCurrentItem->background( 1 ).color();
    myNewColorRampItem.label = myCurrentItem->text( 2 );

    //Simple insertion sort - speed is not a huge factor here
    inserted = false;
    myCurrentIndex = 0;
    while ( !inserted )
    {
      if ( 0 == myColorRampItems.size() || myCurrentIndex == myColorRampItems.size() )
      {
        myColorRampItems.push_back( myNewColorRampItem );
        inserted = true;
      }
      else if ( myColorRampItems[myCurrentIndex].value > myNewColorRampItem.value )
      {
        myColorRampItems.insert( myCurrentIndex, myNewColorRampItem );
        inserted = true;
      }
      else if ( myColorRampItems[myCurrentIndex].value <= myNewColorRampItem.value  && myCurrentIndex == myColorRampItems.size() - 1 )
      {
        myColorRampItems.push_back( myNewColorRampItem );
        inserted = true;
      }
      else if ( myColorRampItems[myCurrentIndex].value <= myNewColorRampItem.value && myColorRampItems[myCurrentIndex+1].value > myNewColorRampItem.value )
      {
        myColorRampItems.insert( myCurrentIndex + 1, myNewColorRampItem );
        inserted = true;
      }
      myCurrentIndex++;
    }
  }
  populateColormapTreeWidget( myColorRampItems );
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
  QList<QColor> entryColors;

  double min = lineEditValue( mMinLineEdit );
  double max = lineEditValue( mMaxLineEdit );

  QgsVectorColorRampV2* colorRamp = mColorRampComboBox->currentColorRamp();

  if ( mClassificationModeComboBox->itemData( mClassificationModeComboBox->currentIndex() ).toInt() == Continuous )
  {
    if ( colorRamp )
    {
      numberOfEntries = colorRamp->count();
      for ( int i = 0; i < colorRamp->count(); ++i )
      {
        double value = colorRamp->value( i );
        entryValues.push_back( min + value * ( max - min ) );
      }
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
    for ( int i = 0; i < numberOfEntries; ++i )
    {
      int idx = mInvertCheckBox->isChecked() ? numberOfEntries - i - 1 : i;
      entryColors.push_back( colorRamp->color((( double ) idx ) / numberOfEntries ) );
    }
  }

  mColormapTreeWidget->clear();

  QList<double>::const_iterator value_it = entryValues.begin();
  QList<QColor>::const_iterator color_it = entryColors.begin();

  for ( ; value_it != entryValues.end(); ++value_it, ++color_it )
  {
    QTreeWidgetItem* newItem = new QTreeWidgetItem( mColormapTreeWidget );
    newItem->setText( 0, QString::number( *value_it, 'f' ) );
    newItem->setBackground( 1, QBrush( *color_it ) );
    newItem->setText( 2, QString::number( *value_it, 'f' ) );
    newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable );
  }
}

void QgsSingleBandPseudoColorRendererWidget::on_mClassificationModeComboBox_currentIndexChanged( int index )
{
  mNumberOfEntriesSpinBox->setEnabled( mClassificationModeComboBox->itemData( index ).toInt() == EqualInterval );
}

void QgsSingleBandPseudoColorRendererWidget::on_mColorRampComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  QSettings settings;
  settings.setValue( "/Raster/defaultPalette", mColorRampComboBox->currentText() );
}

void QgsSingleBandPseudoColorRendererWidget::populateColormapTreeWidget( const QList<QgsColorRampShader::ColorRampItem>& colorRampItems )
{
  mColormapTreeWidget->clear();
  QList<QgsColorRampShader::ColorRampItem>::const_iterator it = colorRampItems.constBegin();
  for ( ; it != colorRampItems.constEnd(); ++it )
  {
    QTreeWidgetItem* newItem = new QTreeWidgetItem( mColormapTreeWidget );
    newItem->setText( 0, QString::number( it->value, 'f' ) );
    newItem->setBackground( 1, QBrush( it->color ) );
    newItem->setText( 2, it->label );
  }
}

void QgsSingleBandPseudoColorRendererWidget::on_mLoadFromBandButton_clicked()
{
  if ( !mRasterLayer )
  {
    return;
  }

  QList<QgsColorRampShader::ColorRampItem> colorRampList;
  int bandIndex = mBandComboBox->itemData( mBandComboBox->currentIndex() ).toInt();

  if ( mRasterLayer->readColorTable( bandIndex, &colorRampList ) )
  {
    populateColormapTreeWidget( colorRampList );
    mColorInterpolationComboBox->setCurrentIndex( mColorInterpolationComboBox->findText( tr( "Linear" ) ) );
  }
  else
  {
    QMessageBox::warning( this, tr( "Load Color Map" ), tr( "The color map for band %1 failed to load" ).arg( bandIndex ) );
  }
}

void QgsSingleBandPseudoColorRendererWidget::on_mLoadFromFileButton_clicked()
{
  int lineCounter = 0;
  bool importError = false;
  QString badLines;
  QSettings settings;
  QString lastDir = settings.value( "lastRasterFileFilterDir", "" ).toString();
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
        if ( !inputLine.simplified().startsWith( "#" ) )
        {
          if ( inputLine.contains( "INTERPOLATION", Qt::CaseInsensitive ) )
          {
            inputStringComponents = inputLine.split( ":" );
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
            inputStringComponents = inputLine.split( "," );
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
  QString lastDir = settings.value( "lastRasterFileFilterDir", "" ).toString();
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
      outputStream << "# " << tr( "QGIS Generated Color Map Export File" ) << "\n";
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
        color = currentItem->background( 1 ).color();
        outputStream << currentItem->text( 0 ).toDouble() << ",";
        outputStream << color.red() << "," << color.green() << "," << color.blue() << "," << color.alpha() << ",";
        if ( currentItem->text( 2 ) == "" )
        {
          outputStream << "Color entry " << i + 1 << "\n";
        }
        else
        {
          outputStream << currentItem->text( 2 ) << "\n";
        }
      }
      outputStream.flush();
      outputFile.close();
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

  if ( column == 1 ) //change item color
  {
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    QColor newColor = QColorDialog::getColor( item->background( column ).color() );
    if ( newColor.isValid() )
    {
      item->setBackground( 1, QBrush( newColor ) );
    }
  }
  else
  {
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable );
  }
}

void QgsSingleBandPseudoColorRendererWidget::setFromRenderer( const QgsRasterRenderer* r )
{
  const QgsSingleBandPseudoColorRenderer* pr = dynamic_cast<const QgsSingleBandPseudoColorRenderer*>( r );
  if ( pr )
  {
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
          QTreeWidgetItem* newItem = new QTreeWidgetItem( mColormapTreeWidget );
          newItem->setText( 0, QString::number( it->value, 'f' ) );
          newItem->setBackground( 1, QBrush( it->color ) );
          newItem->setText( 2, it->label );
        }
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
  QList<int> myBands;
  myBands.append( mBandComboBox->itemData( index ).toInt() );
  mMinMaxWidget->setBands( myBands );
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
