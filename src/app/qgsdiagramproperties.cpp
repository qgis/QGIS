/***************************************************************************
  qgsdiagramproperties.cpp
  Adjust the properties for diagrams
  -------------------
         begin                : August 2012
         copyright            : (C) Matthias Kuhn
         email                : matthias dot kuhn at gmx dot ch

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "diagram/qgshistogramdiagram.h"
#include "diagram/qgspiediagram.h"
#include "diagram/qgstextdiagram.h"

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsdiagramproperties.h"
#include "qgslabelengineconfigdialog.h"
#include "qgsvectorlayerproperties.h"
#include "qgsdiagramrendererv2.h"
#include "qgsvectordataprovider.h"

#include <QColorDialog>
#include <QFontDialog>
#include <QList>
#include <QMessageBox>

QgsDiagramProperties::QgsDiagramProperties( QgsVectorLayer* layer, QWidget* parent )
    : QWidget( parent )
{
  mLayer = layer;

  if ( !layer )
  {
    return;
  }

  setupUi( this );

  mValueLineEdit->setValidator( new QDoubleValidator( mValueLineEdit ) );
  mMinimumDiagramScaleLineEdit->setValidator( new QDoubleValidator( mMinimumDiagramScaleLineEdit ) );
  mMaximumDiagramScaleLineEdit->setValidator( new QDoubleValidator( mMaximumDiagramScaleLineEdit ) );

  mDiagramUnitComboBox->insertItem( 0, tr( "mm" ) );
  mDiagramUnitComboBox->insertItem( 1, tr( "Map units" ) );

  QGis::GeometryType layerType = layer->geometryType();
  if ( layerType == QGis::UnknownGeometry || layerType == QGis::NoGeometry )
  {
    mDisplayDiagramsGroupBox->setChecked( false );
    mDisplayDiagramsGroupBox->setEnabled( false );
  }

  //insert placement options

  if ( layerType == QGis::Point || layerType == QGis::Polygon )
  {
    mPlacementComboBox->addItem( tr( "AroundPoint" ), 0 );
    mPlacementComboBox->addItem( tr( "OverPoint" ), 1 );
  }

  if ( layerType == QGis::Line || layerType == QGis::Polygon )
  {
    mPlacementComboBox->addItem( tr( "Line" ), 2 );
    mPlacementComboBox->addItem( tr( "Horizontal" ), 3 );
  }

  if ( layerType == QGis::Polygon )
  {
    mPlacementComboBox->addItem( tr( "Free" ), 4 );
  }

  if ( layerType == QGis::Line )
  {
    mLineOptionsComboBox->addItem( tr( "On line" ), 1 );
    mLineOptionsComboBox->addItem( tr( "Above line" ), 2 );
    mLineOptionsComboBox->addItem( tr( "Below Line" ), 4 );
    mLineOptionsComboBox->addItem( tr( "Map orientation" ), 8 );
  }
  else
  {
    mLineOptionsComboBox->setVisible( false );
    mLineOptionsLabel->setVisible( false );
  }

  QString iconPath = QgsApplication::defaultThemePath();
  QPixmap pix;
  pix.load( iconPath + "piechart", "png" );
  mDiagramTypeComboBox->addItem( pix, tr( "Pie chart" ) );
  pix.load( iconPath + "textdiagram", "png" );
  mDiagramTypeComboBox->addItem( pix, tr( "Text diagram" ) );
  pix.load( iconPath + "histogram", "png" );
  mDiagramTypeComboBox->addItem( pix, tr( "Histogram" ) );

  mLabelPlacementComboBox->addItem( tr( "Height" ) );
  mLabelPlacementComboBox->addItem( tr( "x-height" ) );

  mScaleDependencyComboBox->addItem( tr( "Area" ) );
  mScaleDependencyComboBox->addItem( tr( "Diameter" ) );

  //insert all attributes into the combo boxes
  const QgsFieldMap& layerFields = layer->pendingFields();
  QgsFieldMap::const_iterator fieldIt = layerFields.constBegin();
  for ( ; fieldIt != layerFields.constEnd(); ++fieldIt )
  {
    QTreeWidgetItem *newItem = new QTreeWidgetItem( mAttributesTreeWidget );
    newItem->setText( 0, fieldIt.value().name() );
    newItem->setData( 0, Qt::UserRole, fieldIt.key() );
    newItem->setFlags( newItem->flags() & ~Qt::ItemIsDropEnabled );
    if ( fieldIt.value().type() != QVariant::String )
    {
      mSizeAttributeComboBox->addItem( fieldIt.value().name(), fieldIt.key() );
    }
  }

  mDataDefinedXComboBox->addItem( tr( "None" ), -1 );
  for ( fieldIt = layerFields.constBegin(); fieldIt != layerFields.constEnd(); ++fieldIt )
  {
    mDataDefinedXComboBox->addItem( fieldIt.value().name(), fieldIt.key() );
  }
  mDataDefinedYComboBox->addItem( tr( "None" ), -1 );
  for ( fieldIt = layerFields.constBegin(); fieldIt != layerFields.constEnd(); ++fieldIt )
  {
    mDataDefinedYComboBox->addItem( fieldIt.value().name(), fieldIt.key() );
  }

  const QgsDiagramRendererV2* dr = layer->diagramRenderer();
  if ( !dr ) //no diagram renderer yet, insert reasonable default
  {
    mDisplayDiagramsGroupBox->setChecked( false );
    mFixedSizeCheckBox->setChecked( true );
    mDiagramUnitComboBox->setCurrentIndex( mDiagramUnitComboBox->findText( tr( "mm" ) ) );
    mLabelPlacementComboBox->setCurrentIndex( mLabelPlacementComboBox->findText( tr( "x-height" ) ) );
    mDiagramSizeSpinBox->setValue( 30 );
    mBarWidthSpinBox->setValue( 5 );
    mVisibilityGroupBox->setChecked( false );

    switch ( layerType )
    {
      case QGis::Point:
        mPlacementComboBox->setCurrentIndex( mPlacementComboBox->findData( 0 ) );
        break;
      case QGis::Line:
        mPlacementComboBox->setCurrentIndex( mPlacementComboBox->findData( 3 ) );
        mLineOptionsComboBox->setCurrentIndex( mLineOptionsComboBox->findData( 2 ) );
        break;
      case QGis::Polygon:
        mPlacementComboBox->setCurrentIndex( mPlacementComboBox->findData( 0 ) );
        break;
      case QGis::UnknownGeometry:
      case QGis::NoGeometry:
        break;
    }
    mBackgroundColorButton->setColor( QColor( 255, 255, 255, 255 ) );
  }
  else // already a diagram renderer present
  {
    mDisplayDiagramsGroupBox->setChecked( true );

    //single category renderer or interpolated one?
    mFixedSizeCheckBox->setChecked( dr->rendererName() == "SingleCategory" );

    //assume single category or linearly interpolated diagram renderer for now
    QList<QgsDiagramSettings> settingList = dr->diagramSettings();
    if ( settingList.size() > 0 )
    {
      mDiagramFont = settingList.at( 0 ).font;
      QSizeF size = settingList.at( 0 ).size;
      mBackgroundColorButton->setColor( settingList.at( 0 ).backgroundColor );
      mTransparencySlider->setValue( settingList.at( 0 ).transparency );
      mDiagramPenColorButton->setColor( settingList.at( 0 ).penColor );
      mPenWidthSpinBox->setValue( settingList.at( 0 ).penWidth );
      mDiagramSizeSpinBox->setValue(( size.width() + size.height() ) / 2.0 );
      mMinimumDiagramScaleLineEdit->setText( QString::number( settingList.at( 0 ).minScaleDenominator ) );
      mMaximumDiagramScaleLineEdit->setText( QString::number( settingList.at( 0 ).maxScaleDenominator ) );
      mVisibilityGroupBox->setChecked( settingList.at( 0 ).minScaleDenominator != -1 &&
          settingList.at( 0 ).maxScaleDenominator != -1 );
      if ( settingList.at( 0 ).sizeType == QgsDiagramSettings::MM )
      {
        mDiagramUnitComboBox->setCurrentIndex( 0 );
      }
      else
      {
        mDiagramUnitComboBox->setCurrentIndex( 1 );
      }

      if ( settingList.at( 0 ).labelPlacementMethod == QgsDiagramSettings::Height )
      {
        mLabelPlacementComboBox->setCurrentIndex( 0 );
      }
      else
      {
        mLabelPlacementComboBox->setCurrentIndex( 1 );
      }

      switch( settingList.at( 0 ).diagramOrientation )
      {
        case QgsDiagramSettings::Left:
          mOrientationLeftButton->setChecked( true );
          break;

        case QgsDiagramSettings::Right:
          mOrientationRightButton->setChecked( true );
          break;

        case QgsDiagramSettings::Up:
          mOrientationUpButton->setChecked( true );
          break;

        case QgsDiagramSettings::Down:
          mOrientationDownButton->setChecked( true );
          break;
      }

      mBarWidthSpinBox->setValue( settingList.at( 0 ).barWidth );

      mIncreaseSmallDiagramsGroupBox->setChecked( settingList.at( 0 ).minimumSize != 0 );
      mIncreaseMinimumSizeSpinBox->setValue( settingList.at( 0 ).minimumSize );

      if ( settingList.at( 0 ).scaleByArea )
      {
        mScaleDependencyComboBox->setCurrentIndex( 0 );
      }
      else
      {
        mScaleDependencyComboBox->setCurrentIndex( 1 );
      }

      QList< QColor > categoryColors = settingList.at( 0 ).categoryColors;
      QList< int > categoryIndices = settingList.at( 0 ).categoryIndices;
      QList< int >::const_iterator catIt = categoryIndices.constBegin();
      QList< QColor >::const_iterator coIt = categoryColors.constBegin();
      for ( ;catIt != categoryIndices.constEnd(); ++catIt, ++coIt )
      {
        QTreeWidgetItem *newItem = new QTreeWidgetItem( mDiagramAttributesTreeWidget );
        newItem->setText( 0, layer->pendingFields()[*catIt].name() );
        newItem->setData( 0, Qt::UserRole, *catIt );
        newItem->setFlags( newItem->flags() & ~Qt::ItemIsDropEnabled );
        QColor col( *coIt );
        col.setAlpha( 255 );
        newItem->setBackground( 1, QBrush( col ) );
      }
    }

    if ( dr->rendererName() == "LinearlyInterpolated" )
    {
      const QgsLinearlyInterpolatedDiagramRenderer* lidr = dynamic_cast<const QgsLinearlyInterpolatedDiagramRenderer*>( dr );
      if ( lidr )
      {
        mDiagramSizeSpinBox->setEnabled( false );
        mValueLineEdit->setText( QString::number( lidr->upperValue() ) );
        mSizeSpinBox->setValue(( lidr->upperSize().width() + lidr->upperSize().height() ) / 2 );
        mSizeAttributeComboBox->setCurrentIndex( mSizeAttributeComboBox->findData( lidr->classificationAttribute() ) );
      }
    }

    const QgsDiagramLayerSettings *dls = layer->diagramLayerSettings();
    if ( dls )
    {
      mDiagramDistanceSpinBox->setValue( dls->dist );
      mPrioritySlider->setValue( dls->priority );
      mDataDefinedXComboBox->setCurrentIndex( mDataDefinedXComboBox->findData( dls->xPosColumn ) );
      mDataDefinedYComboBox->setCurrentIndex( mDataDefinedYComboBox->findData( dls->yPosColumn ) );
      if ( dls->xPosColumn != -1 || dls->yPosColumn != -1 )
      {
        mDataDefinedPositionGroupBox->setChecked( true );
      }
      mPlacementComboBox->setCurrentIndex( mPlacementComboBox->findData( dls->placement ) );
      mLineOptionsComboBox->setCurrentIndex( mLineOptionsComboBox->findData( dls->placementFlags ) );
    }

    if ( dr->diagram() )
    {
      QString diagramName = dr->diagram()->diagramName();
      if ( diagramName == "Text" )
      {
        mDiagramTypeComboBox->setCurrentIndex( mDiagramTypeComboBox->findText( tr( "Text diagram" ) ) );
      }
      else if ( diagramName == "Pie" )
      {
        mDiagramTypeComboBox->setCurrentIndex( mDiagramTypeComboBox->findText( tr( "Pie chart" ) ) );
      }
      else if ( diagramName == "Histogram" )
      {
        mDiagramTypeComboBox->setCurrentIndex( mDiagramTypeComboBox->findText( tr( "Histogram" ) ) );
      }
      else
      {
        QMessageBox::warning( this, tr( "Unknown diagram type." ),
          tr( "The diagram type '%1' is unknown. A default type is selected for you." ).arg( diagramName ), QMessageBox::Ok );
        mDiagramTypeComboBox->setCurrentIndex( mDiagramTypeComboBox->findText( tr( "Pie chart" ) ) );
      }
    }
  } // if ( !dr )

  // Trigger a clicked event, so all the items get properly enabled and disabled
  on_mDisplayDiagramsGroupBox_toggled( mDisplayDiagramsGroupBox->isChecked() );
}

void QgsDiagramProperties::on_mDiagramTypeComboBox_currentIndexChanged( const QString& itemtext )
{
  if ( tr( "Text diagram" ) == itemtext )
  {
    mLabelPlacementComboBox->show();
    mLabelPlacementLabel->show();
  }
  else {
    mLabelPlacementComboBox->hide();
    mLabelPlacementLabel->hide();
  }

  if ( tr( "Histogram" ) == itemtext )
  {
    mBarWidthLabel->show();
    mBarWidthSpinBox->show();
    mOrientationFrame->show();
  }
  else
  {
    mBarWidthLabel->hide();
    mBarWidthSpinBox->hide();
    mOrientationFrame->hide();
  }

  if ( tr( "Histogram" ) == itemtext || tr( "Text diagram" ) == itemtext )
  {
    mDiagramPropertiesTabWidget->setTabEnabled( 3, true );
  }
  else
  {
    mDiagramPropertiesTabWidget->setTabEnabled( 3, false );
  }

  if ( tr( "Text diagram" ) == itemtext || tr( "Pie chart" ) == itemtext )
  {
    mScaleDependencyComboBox->show();
    mScaleDependencyLabel->show();
  }
  else
  {
    mScaleDependencyComboBox->hide();
    mScaleDependencyLabel->hide();
  }
}

void QgsDiagramProperties::on_mTransparencySlider_valueChanged( int value )
{
  mTransparencyLabel->setText( tr( "Transparency: %1%" ).arg( value * 100 / 255 ) );
}

void QgsDiagramProperties::on_mAddCategoryPushButton_clicked()
{
  foreach( QTreeWidgetItem *attributeItem, mAttributesTreeWidget->selectedItems() )
  {
    QTreeWidgetItem *newItem = new QTreeWidgetItem( mDiagramAttributesTreeWidget );

    newItem->setText( 0, attributeItem->text( 0 ) );
    newItem->setData( 0, Qt::UserRole, attributeItem->data( 0, Qt::UserRole ) );
    newItem->setFlags( newItem->flags() & ~Qt::ItemIsDropEnabled );
    
    //set initial color for diagram category
    int red = 1 + ( int )( 255.0 * rand() / ( RAND_MAX + 1.0 ) );
    int green = 1 + ( int )( 255.0 * rand() / ( RAND_MAX + 1.0 ) );
    int blue = 1 + ( int )( 255.0 * rand() / ( RAND_MAX + 1.0 ) );
    QColor randomColor( red, green, blue );
    newItem->setBackground( 1, QBrush( randomColor ) );
    mDiagramAttributesTreeWidget->addTopLevelItem( newItem );
  }
}


void QgsDiagramProperties::on_mRemoveCategoryPushButton_clicked()
{
  foreach( QTreeWidgetItem *attributeItem, mDiagramAttributesTreeWidget->selectedItems() )
  {
    delete attributeItem;
  }
}

void QgsDiagramProperties::on_mBackgroundColorButton_clicked()
{
#if QT_VERSION >= 0x040500
  QColor newColor = QColorDialog::getColor( mBackgroundColorButton->color(), 0, tr( "Background color" ), QColorDialog::ShowAlphaChannel );
#else
  QColor newColor = QColorDialog::getColor( mBackgroundColorButton->color() );
#endif
  if ( newColor.isValid() )
  {
    mBackgroundColorButton->setColor( newColor );
  }
}

void QgsDiagramProperties::on_mFindMaximumValueButton_clicked()
{
  //get maximum value from provider (ignoring not-commited edits)
  if ( mLayer )
  {
    QgsVectorDataProvider* provider = mLayer->dataProvider();
    if ( provider )
    {
      mValueLineEdit->setText( provider->maximumValue( mSizeAttributeComboBox->itemData( mSizeAttributeComboBox->currentIndex() ).toInt() ).toString() );
    }
  }
}

void QgsDiagramProperties::on_mDiagramPenColorButton_clicked()
{
#if QT_VERSION >= 0x040500
  QColor newColor = QColorDialog::getColor( mDiagramPenColorButton->color(), 0, tr( "Pen color" ), QColorDialog::ShowAlphaChannel );
#else
  QColor newColor = QColorDialog::getColor( mDiagramPenColorButton->color() );
#endif
  if ( newColor.isValid() )
  {
    mDiagramPenColorButton->setColor( newColor );
  }
}

void QgsDiagramProperties::on_mDisplayDiagramsGroupBox_toggled( bool checked )
{
  // if enabled show diagram specific options
  if ( checked )
  {
    on_mDiagramTypeComboBox_currentIndexChanged( mDiagramTypeComboBox->currentText() );
    // Update enabled/disabled state
  }
}

void QgsDiagramProperties::on_mDiagramFontButton_clicked()
{
  bool ok;
  mDiagramFont = QFontDialog::getFont( &ok, mDiagramFont );
}


void QgsDiagramProperties::on_mDiagramAttributesTreeWidget_itemDoubleClicked( QTreeWidgetItem * item, int column )
{
  if ( column == 1 ) //change color
  {
    QColor newColor = QColorDialog::getColor();
    if ( newColor.isValid() )
    {
      item->setBackground( 1, QBrush( newColor ) );
    }
  }
}

void QgsDiagramProperties::on_mEngineSettingsButton_clicked()
{
  QgsLabelEngineConfigDialog dlg( QgisApp::instance()->palLabeling(), this );
  dlg.exec();
}

void QgsDiagramProperties::apply()
{
  if ( !mDisplayDiagramsGroupBox->isChecked() )
  {
    mLayer->setDiagramRenderer( 0 );
  }
  else
  {
    if ( 0 == mDiagramAttributesTreeWidget->topLevelItemCount() )
    {
      QMessageBox::warning( this, tr( "No attributes added." ),
        tr( "You did not add any attributes to this diagram layer. Please specify the attributes to visualize on the diagrams or disable diagrams." ), QMessageBox::Ok );
    }

    QgsDiagram* diagram = 0;
    if ( mDiagramTypeComboBox->currentText() == tr( "Text diagram" ) )
    {
      diagram = new QgsTextDiagram();
    }
    else if ( mDiagramTypeComboBox->currentText() == tr( "Pie chart" ) )
    {
      diagram = new QgsPieDiagram();
    }
    else if ( mDiagramTypeComboBox->currentText() == tr( "Histogram" ) )
    {
      diagram = new QgsHistogramDiagram();
    }

    QgsDiagramSettings ds;
    ds.font = mDiagramFont;
    ds.transparency = mTransparencySlider->value();

    QList<QColor> categoryColors;
    QList<int> categoryAttributes;
    for ( int i = 0; i < mDiagramAttributesTreeWidget->topLevelItemCount(); ++i )
    {
      QColor color = mDiagramAttributesTreeWidget->topLevelItem( i )->background( 1 ).color();
      color.setAlpha( 255 - ds.transparency );
      categoryColors.append( color );
      categoryAttributes.append( mDiagramAttributesTreeWidget->topLevelItem( i )->data( 0, Qt::UserRole ).toInt() );
    }
    ds.categoryColors = categoryColors;
    ds.categoryIndices = categoryAttributes;
    ds.size = QSizeF( mDiagramSizeSpinBox->value(), mDiagramSizeSpinBox->value() );
    if ( mDiagramUnitComboBox->currentText() == tr( "Map units" ) )
    {
      ds.sizeType = QgsDiagramSettings::MapUnits;
    }
    else
    {
      ds.sizeType = QgsDiagramSettings::MM;
    }

    if ( tr( "Height" ) == mLabelPlacementComboBox->currentText() )
    {
     ds.labelPlacementMethod = QgsDiagramSettings::Height;
    }
    else
    {
     ds.labelPlacementMethod = QgsDiagramSettings::XHeight;
    }

    if ( tr( "Area" ) == mScaleDependencyComboBox->currentText() )
    {
      ds.scaleByArea = true;
    }
    else
    {
      ds.scaleByArea = false;
    }

    if ( mIncreaseSmallDiagramsGroupBox->isChecked() )
    {
      ds.minimumSize = mIncreaseMinimumSizeSpinBox->value();
    }
    else
    {
      ds.minimumSize = 0;
    }

    ds.backgroundColor = mBackgroundColorButton->color();
    ds.penColor = mDiagramPenColorButton->color();
    ds.penColor.setAlpha( 255 - ds.transparency );
    ds.penWidth = mPenWidthSpinBox->value();
    if ( mVisibilityGroupBox->isChecked() )
    {
      ds.minScaleDenominator = mMinimumDiagramScaleLineEdit->text().toDouble();
      ds.maxScaleDenominator = mMaximumDiagramScaleLineEdit->text().toDouble();
    }
    else
    {
      ds.minScaleDenominator = -1;
      ds.maxScaleDenominator = -1;
    }

    // Diagram orientation (histogram)
    if ( tr( "Up" ) == mOrientationButtonGroup->checkedButton()->text() )
    {
      ds.diagramOrientation = QgsDiagramSettings::Up;
    }
    else if ( tr( "Down" ) == mOrientationButtonGroup->checkedButton()->text() )
    {
      ds.diagramOrientation = QgsDiagramSettings::Down;
    }
    else if ( tr( "Right" ) == mOrientationButtonGroup->checkedButton()->text() )
    {
      ds.diagramOrientation = QgsDiagramSettings::Right;
    }
    else if ( tr( "Left" ) == mOrientationButtonGroup->checkedButton()->text() )
    {
      ds.diagramOrientation = QgsDiagramSettings::Left;
    }

    ds.barWidth = mBarWidthSpinBox->value();

    if ( mFixedSizeCheckBox->isChecked() )
    {
      QgsSingleCategoryDiagramRenderer* dr = new QgsSingleCategoryDiagramRenderer();
      dr->setDiagram( diagram );
      dr->setDiagramSettings( ds );
      mLayer->setDiagramRenderer( dr );
    }
    else
    {
      QgsLinearlyInterpolatedDiagramRenderer* dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( mValueLineEdit->text().toDouble() );
      dr->setUpperSize( QSizeF( mSizeSpinBox->value(), mSizeSpinBox->value() ) );
      dr->setClassificationAttribute( mSizeAttributeComboBox->itemData( mSizeAttributeComboBox->currentIndex() ).toInt() );
      dr->setDiagram( diagram );
      dr->setDiagramSettings( ds );
      mLayer->setDiagramRenderer( dr );
    }

    QgsDiagramLayerSettings dls;
    dls.dist = mDiagramDistanceSpinBox->value();
    dls.priority = mPrioritySlider->value();
    if ( mDataDefinedPositionGroupBox->isChecked() )
    {
      dls.xPosColumn = mDataDefinedXComboBox->itemData( mDataDefinedXComboBox->currentIndex() ).toInt();
      dls.yPosColumn = mDataDefinedYComboBox->itemData( mDataDefinedYComboBox->currentIndex() ).toInt();
    }
    else
    {
      dls.xPosColumn = -1;
      dls.yPosColumn = -1;
    }
    dls.placement = ( QgsDiagramLayerSettings::Placement )mPlacementComboBox->itemData( mPlacementComboBox->currentIndex() ).toInt();
    if ( mLineOptionsComboBox->isEnabled() )
    {
      dls.placementFlags = ( QgsDiagramLayerSettings::LinePlacementFlags )mLineOptionsComboBox->itemData( mLineOptionsComboBox->currentIndex() ).toInt();
    }
    mLayer->setDiagramLayerSettings( dls );
  }
}
