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
#include "qgsdiagramrendererv2.h"
#include "qgslabelengineconfigdialog.h"
#include "qgsmessagebar.h"
#include "qgsvectorlayerproperties.h"
#include "qgsvectordataprovider.h"

#include <QColorDialog>
#include <QFontDialog>
#include <QList>
#include <QMessageBox>
#include <QSettings>

QgsDiagramProperties::QgsDiagramProperties( QgsVectorLayer* layer, QWidget* parent )
    : QWidget( parent )
{
  mLayer = layer;

  if ( !layer )
  {
    return;
  }

  setupUi( this );

  int tabIdx = QSettings().value( "/Windows/VectorLayerProperties/diagram/tab", 0 ).toInt();

  mDiagramPropertiesTabWidget->setCurrentIndex( tabIdx );

  mBackgroundColorButton->setColorDialogTitle( tr( "Background color" ) );
  mBackgroundColorButton->setColorDialogOptions( QColorDialog::ShowAlphaChannel );
  mDiagramPenColorButton->setColorDialogTitle( tr( "Pen color" ) );
  mDiagramPenColorButton->setColorDialogOptions( QColorDialog::ShowAlphaChannel );

  mValueLineEdit->setValidator( new QDoubleValidator( mValueLineEdit ) );
  mMinimumDiagramScaleLineEdit->setValidator( new QDoubleValidator( mMinimumDiagramScaleLineEdit ) );
  mMaximumDiagramScaleLineEdit->setValidator( new QDoubleValidator( mMaximumDiagramScaleLineEdit ) );

  mDiagramUnitComboBox->insertItem( 0, tr( "mm" ), QgsDiagramSettings::MM );
  mDiagramUnitComboBox->insertItem( 1, tr( "Map units" ), QgsDiagramSettings::MapUnits );

  QGis::GeometryType layerType = layer->geometryType();
  if ( layerType == QGis::UnknownGeometry || layerType == QGis::NoGeometry )
  {
    mDisplayDiagramsGroupBox->setChecked( false );
    mDisplayDiagramsGroupBox->setEnabled( false );
  }

  //insert placement options

  if ( layerType == QGis::Point || layerType == QGis::Polygon )
  {
    mPlacementComboBox->addItem( tr( "Around Point" ), QgsDiagramLayerSettings::AroundPoint );
    mPlacementComboBox->addItem( tr( "Over Point" ), QgsDiagramLayerSettings::OverPoint );
  }

  if ( layerType == QGis::Line || layerType == QGis::Polygon )
  {
    mPlacementComboBox->addItem( tr( "Line" ), QgsDiagramLayerSettings::Line );
    mPlacementComboBox->addItem( tr( "Horizontal" ), QgsDiagramLayerSettings::Horizontal );
  }

  if ( layerType == QGis::Polygon )
  {
    mPlacementComboBox->addItem( tr( "Free" ), QgsDiagramLayerSettings::Free );
  }

  if ( layerType == QGis::Line )
  {
    mLineOptionsComboBox->addItem( tr( "On line" ), QgsDiagramLayerSettings::OnLine );
    mLineOptionsComboBox->addItem( tr( "Above line" ), QgsDiagramLayerSettings::AboveLine );
    mLineOptionsComboBox->addItem( tr( "Below Line" ), QgsDiagramLayerSettings::BelowLine );
    mLineOptionsComboBox->addItem( tr( "Map orientation" ), QgsDiagramLayerSettings::MapOrientation );
  }
  else
  {
    mLineOptionsComboBox->setVisible( false );
    mLineOptionsLabel->setVisible( false );
  }

  QPixmap pix = QgsApplication::getThemePixmap( "pie-chart" );
  mDiagramTypeComboBox->addItem( pix, tr( "Pie chart" ), DIAGRAM_NAME_PIE );
  pix = QgsApplication::getThemePixmap( "text" );
  mDiagramTypeComboBox->addItem( pix, tr( "Text diagram" ), DIAGRAM_NAME_TEXT );
  pix = QgsApplication::getThemePixmap( "histogram" );
  mDiagramTypeComboBox->addItem( pix, tr( "Histogram" ), DIAGRAM_NAME_HISTOGRAM );

  mLabelPlacementComboBox->addItem( tr( "Height" ), QgsDiagramSettings::Height );
  mLabelPlacementComboBox->addItem( tr( "x-height" ), QgsDiagramSettings::XHeight );

  mScaleDependencyComboBox->addItem( tr( "Area" ), true );
  mScaleDependencyComboBox->addItem( tr( "Diameter" ), false );

  mDataDefinedXComboBox->addItem( tr( "None" ), -1 );
  mDataDefinedYComboBox->addItem( tr( "None" ), -1 );

  mAngleOffsetComboBox->addItem( tr( "Top" ), 90 * 16 );
  mAngleOffsetComboBox->addItem( tr( "Right" ), 0 );
  mAngleOffsetComboBox->addItem( tr( "Bottom" ), 270 * 16 );
  mAngleOffsetComboBox->addItem( tr( "Left" ), 180 * 16 );

  //insert all attributes into the combo boxes
  const QgsFields& layerFields = layer->pendingFields();
  for ( int idx = 0; idx < layerFields.count(); ++idx )
  {
    QTreeWidgetItem *newItem = new QTreeWidgetItem( mAttributesTreeWidget );
    newItem->setText( 0, layerFields[idx].name() );
    newItem->setData( 0, Qt::UserRole, idx );
    newItem->setFlags( newItem->flags() & ~Qt::ItemIsDropEnabled );
    if ( layerFields[idx].type() != QVariant::String )
    {
      mSizeAttributeComboBox->addItem( layerFields[idx].name(), idx );
    }

    mDataDefinedXComboBox->addItem( layerFields[idx].name(), idx );
    mDataDefinedYComboBox->addItem( layerFields[idx].name(), idx );
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
    mVisibilityGroupBox->setChecked( layer->hasScaleBasedVisibility() );
    mMaximumDiagramScaleLineEdit->setText( QString::number( layer->maximumScale() ) );
    mMinimumDiagramScaleLineEdit->setText( QString::number( layer->minimumScale() ) );

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
      mMinimumDiagramScaleLineEdit->setText( QString::number( settingList.at( 0 ).minScaleDenominator, 'f' ) );
      mMaximumDiagramScaleLineEdit->setText( QString::number( settingList.at( 0 ).maxScaleDenominator, 'f' ) );
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

      mAngleOffsetComboBox->setCurrentIndex( mAngleOffsetComboBox->findData( settingList.at( 0 ).angleOffset ) );

      mOrientationLeftButton->setProperty( "direction", QgsDiagramSettings::Left );
      mOrientationRightButton->setProperty( "direction", QgsDiagramSettings::Right );
      mOrientationUpButton->setProperty( "direction", QgsDiagramSettings::Up );
      mOrientationDownButton->setProperty( "direction", QgsDiagramSettings::Down );
      switch ( settingList.at( 0 ).diagramOrientation )
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
        mValueLineEdit->setText( QString::number( lidr->upperValue(), 'f' ) );
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
      mDiagramTypeComboBox->setCurrentIndex( mDiagramTypeComboBox->findData( diagramName ) );
      if ( mDiagramTypeComboBox->currentIndex() == -1 )
      {
        QMessageBox::warning( this, tr( "Unknown diagram type." ),
                              tr( "The diagram type '%1' is unknown. A default type is selected for you." ).arg( diagramName ), QMessageBox::Ok );
        mDiagramTypeComboBox->setCurrentIndex( mDiagramTypeComboBox->findData( DIAGRAM_NAME_PIE ) );
      }
    }
  } // if ( !dr )

  // Trigger a clicked event, so all the items get properly enabled and disabled
  on_mDisplayDiagramsGroupBox_toggled( mDisplayDiagramsGroupBox->isChecked() );
}

void QgsDiagramProperties::on_mDiagramTypeComboBox_currentIndexChanged( int index )
{
  QString diagramType = mDiagramTypeComboBox->itemData( index ).toString();

  if ( DIAGRAM_NAME_TEXT == diagramType )
  {
    mLabelPlacementComboBox->show();
    mLabelPlacementLabel->show();
    mBackgroundColorLabel->show();
    mBackgroundColorButton->show();
  }
  else
  {
    mLabelPlacementComboBox->hide();
    mLabelPlacementLabel->hide();
    mBackgroundColorLabel->hide();
    mBackgroundColorButton->hide();
  }

  if ( DIAGRAM_NAME_HISTOGRAM == diagramType )
  {
    mBarWidthLabel->show();
    mBarWidthSpinBox->show();
    mOrientationFrame->show();
    mFixedSizeCheckBox->setChecked( false );
    mFixedSizeCheckBox->setVisible( false );
    mDiagramSizeSpinBox->setVisible( false );
    mLinearlyScalingLabel->setText( tr( "Bar length: Scale linearly, such as the following value matches the specified size." ) );
  }
  else
  {
    mBarWidthLabel->hide();
    mBarWidthSpinBox->hide();
    mOrientationFrame->hide();
    mLinearlyScalingLabel->setText( tr( "Scale linearly between 0 and the following attribute value / diagram size:" ) );
    mAttributeBasedScalingOptions->show();
    mFixedSizeCheckBox->setVisible( true );
    mDiagramSizeSpinBox->setVisible( true );
  }

  if ( DIAGRAM_NAME_HISTOGRAM == diagramType || DIAGRAM_NAME_TEXT == diagramType )
  {
    mDiagramPropertiesTabWidget->setTabEnabled( 3, true );
  }
  else
  {
    mDiagramPropertiesTabWidget->setTabEnabled( 3, false );
  }

  if ( DIAGRAM_NAME_TEXT == diagramType || DIAGRAM_NAME_PIE == diagramType )
  {
    mScaleDependencyComboBox->show();
    mScaleDependencyLabel->show();
  }
  else
  {
    mScaleDependencyComboBox->hide();
    mScaleDependencyLabel->hide();
  }

  if ( DIAGRAM_NAME_PIE == diagramType )
  {
    mAngleOffsetComboBox->show();
    mAngleOffsetLabel->show();
  }
  else
  {
    mAngleOffsetComboBox->hide();
    mAngleOffsetLabel->hide();
  }
}
void QgsDiagramProperties::addAttribute( QTreeWidgetItem * item )
{
  QTreeWidgetItem *newItem = new QTreeWidgetItem( mDiagramAttributesTreeWidget );

  newItem->setText( 0, item->text( 0 ) );
  newItem->setData( 0, Qt::UserRole, item->data( 0, Qt::UserRole ) );
  newItem->setFlags( newItem->flags() & ~Qt::ItemIsDropEnabled );

  //set initial color for diagram category
  int red = 1 + ( int )( 255.0 * rand() / ( RAND_MAX + 1.0 ) );
  int green = 1 + ( int )( 255.0 * rand() / ( RAND_MAX + 1.0 ) );
  int blue = 1 + ( int )( 255.0 * rand() / ( RAND_MAX + 1.0 ) );
  QColor randomColor( red, green, blue );
  newItem->setBackground( 1, QBrush( randomColor ) );
  mDiagramAttributesTreeWidget->addTopLevelItem( newItem );
}


void QgsDiagramProperties::on_mTransparencySlider_valueChanged( int value )
{
  mTransparencyLabel->setText( tr( "Transparency: %1%" ).arg( value * 100 / 255 ) );
}

void QgsDiagramProperties::on_mAddCategoryPushButton_clicked()
{
  foreach ( QTreeWidgetItem *attributeItem, mAttributesTreeWidget->selectedItems() )
  {
    addAttribute( attributeItem );
  }
}

void QgsDiagramProperties::on_mAttributesTreeWidget_itemDoubleClicked( QTreeWidgetItem * item, int column )
{
  Q_UNUSED( column );
  addAttribute( item );
}

void QgsDiagramProperties::on_mRemoveCategoryPushButton_clicked()
{
  foreach ( QTreeWidgetItem *attributeItem, mDiagramAttributesTreeWidget->selectedItems() )
  {
    delete attributeItem;
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

void QgsDiagramProperties::on_mDisplayDiagramsGroupBox_toggled( bool checked )
{
  // if enabled show diagram specific options
  if ( checked )
  {
    on_mDiagramTypeComboBox_currentIndexChanged( mDiagramTypeComboBox->currentIndex() );
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
  QSettings().setValue( "/Windows/VectorLayerProperties/diagram/tab",
                        mDiagramPropertiesTabWidget->currentIndex() );

  if ( !mDisplayDiagramsGroupBox->isChecked() )
  {
    mLayer->setDiagramRenderer( 0 );
  }
  else
  {
    QgsDiagram* diagram = 0;
    int index = mDiagramTypeComboBox->currentIndex();
    QString diagramType = mDiagramTypeComboBox->itemData( index ).toString();

    if ( 0 == mDiagramAttributesTreeWidget->topLevelItemCount() )
    {
      QgisApp::instance()->messageBar()->pushMessage(
        tr( "Diagrams: No attributes added." ),
        tr( "You did not add any attributes to this diagram layer. Please specify the attributes to visualize on the diagrams or disable diagrams." ),
        QgsMessageBar::WARNING );
    }

    bool scaleAttributeValueOk = false;
    // Check if a (usable) scale attribute value is inserted
    mValueLineEdit->text().toDouble( &scaleAttributeValueOk );

    if ( !mFixedSizeCheckBox->isChecked() && !scaleAttributeValueOk )
    {
      double maxVal = DBL_MIN;
      QgsVectorDataProvider* provider = mLayer->dataProvider();

      if ( provider )
      {
        if ( diagramType == DIAGRAM_NAME_HISTOGRAM )
        {
          // Find maximum value
          for ( int i = 0; i < mDiagramAttributesTreeWidget->topLevelItemCount(); ++i )
          {
            maxVal = qMax( maxVal, provider->maximumValue( mDiagramAttributesTreeWidget->topLevelItem( i )->data( 0, Qt::UserRole ).toInt() ).toDouble() );
          }
        }
        else
        {
          maxVal = provider->maximumValue( mSizeAttributeComboBox->itemData( mSizeAttributeComboBox->currentIndex() ).toInt() ).toDouble();
        }
      }

      if ( maxVal != DBL_MIN )
      {
        QgisApp::instance()->messageBar()->pushMessage(
          tr( "Interpolation value" ),
          tr( "You did not specify an interpolation value. A default value of %1 has been set." ).arg( QString::number( maxVal ) ),
          QgsMessageBar::INFO,
          5 );

        mValueLineEdit->setText( QString::number( maxVal ) );
      }
    }

    if ( diagramType == DIAGRAM_NAME_TEXT )
    {
      diagram = new QgsTextDiagram();
    }
    else if ( diagramType == DIAGRAM_NAME_PIE )
    {
      diagram = new QgsPieDiagram();
    }
    else // if ( diagramType == DIAGRAM_NAME_HISTOGRAM )
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
    ds.sizeType = static_cast<QgsDiagramSettings::SizeType>( mDiagramUnitComboBox->itemData( mDiagramUnitComboBox->currentIndex() ).toInt() );
    ds.labelPlacementMethod = static_cast<QgsDiagramSettings::LabelPlacementMethod>( mLabelPlacementComboBox->itemData( mLabelPlacementComboBox->currentIndex() ).toInt() );
    ds.scaleByArea = mScaleDependencyComboBox->itemData( mScaleDependencyComboBox->currentIndex() ).toBool();

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

    // Diagram angle offset (pie)
    ds.angleOffset = mAngleOffsetComboBox->itemData( mAngleOffsetComboBox->currentIndex() ).toInt();

    // Diagram orientation (histogram)
    ds.diagramOrientation = static_cast<QgsDiagramSettings::DiagramOrientation>( mOrientationButtonGroup->checkedButton()->property( "direction" ).toInt() );

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
      dls.placementFlags = static_cast<QgsDiagramLayerSettings::LinePlacementFlags>( mLineOptionsComboBox->itemData( mLineOptionsComboBox->currentIndex() ).toInt() );
    }
    mLayer->setDiagramLayerSettings( dls );
  }
}
