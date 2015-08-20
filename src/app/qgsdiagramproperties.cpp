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
#include "qgsproject.h"
#include "qgsapplication.h"
#include "qgsdiagramproperties.h"
#include "qgsdiagramrendererv2.h"
#include "qgslabelengineconfigdialog.h"
#include "qgsmessagebar.h"
#include "qgsvectorlayerproperties.h"
#include "qgsvectordataprovider.h"
#include "qgsfeatureiterator.h"
#include "qgscolordialog.h"
#include "qgisgui.h"

#include <QList>
#include <QMessageBox>
#include <QSettings>

static QgsExpressionContext _getExpressionContext( const void* context )
{
  QgsExpressionContext expContext;
  expContext << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope();

  const QgsVectorLayer* layer = ( const QgsVectorLayer* ) context;
  if ( layer )
    expContext << QgsExpressionContextUtils::layerScope( layer );

  return expContext;
}

QgsDiagramProperties::QgsDiagramProperties( QgsVectorLayer* layer, QWidget* parent )
    : QWidget( parent )
{
  mLayer = layer;
  if ( !layer )
  {
    return;
  }

  setupUi( this );

  // get rid of annoying outer focus rect on Mac
  mDiagramOptionsListWidget->setAttribute( Qt::WA_MacShowFocusRect, false );

  connect( mEnableDiagramsCheckBox, SIGNAL( toggled( bool ) ), mDiagramTypeFrame, SLOT( setEnabled( bool ) ) );
  connect( mEnableDiagramsCheckBox, SIGNAL( toggled( bool ) ), mDiagramFrame, SLOT( setEnabled( bool ) ) );

  mScaleRangeWidget->setMapCanvas( QgisApp::instance()->mapCanvas() );
  mSizeFieldExpressionWidget->registerGetExpressionContextCallback( &_getExpressionContext, mLayer );

  mBackgroundColorButton->setColorDialogTitle( tr( "Select background color" ) );
  mBackgroundColorButton->setAllowAlpha( true );
  mBackgroundColorButton->setContext( "symbology" );
  mBackgroundColorButton->setShowNoColor( true );
  mBackgroundColorButton->setNoColorString( tr( "Transparent background" ) );
  mDiagramPenColorButton->setColorDialogTitle( tr( "Select pen color" ) );
  mDiagramPenColorButton->setAllowAlpha( true );
  mDiagramPenColorButton->setContext( "symbology" );
  mDiagramPenColorButton->setShowNoColor( true );
  mDiagramPenColorButton->setNoColorString( tr( "Transparent outline" ) );

  mMaxValueSpinBox->setShowClearButton( false );

  mDiagramUnitComboBox->insertItem( 0, tr( "mm" ), QgsDiagramSettings::MM );
  mDiagramUnitComboBox->insertItem( 1, tr( "Map units" ), QgsDiagramSettings::MapUnits );

  QGis::GeometryType layerType = layer->geometryType();
  if ( layerType == QGis::UnknownGeometry || layerType == QGis::NoGeometry )
  {
    mEnableDiagramsCheckBox->setChecked( false );
    mEnableDiagramsCheckBox->setEnabled( false );
    mDiagramTypeFrame->setEnabled( false );
    mDiagramFrame->setEnabled( false );
  }

  //insert placement options
  mPlacementComboBox->blockSignals( true );
  switch ( layerType )
  {
    case QGis::Point:
      mPlacementComboBox->addItem( tr( "Around Point" ), QgsDiagramLayerSettings::AroundPoint );
      mPlacementComboBox->addItem( tr( "Over Point" ), QgsDiagramLayerSettings::OverPoint );
      mLinePlacementFrame->setVisible( false );
      break;
    case QGis::Line:
      mPlacementComboBox->addItem( tr( "Around Line" ), QgsDiagramLayerSettings::Line );
      mPlacementComboBox->addItem( tr( "Over Line" ), QgsDiagramLayerSettings::Horizontal );
      mLinePlacementFrame->setVisible( true );
      break;
    case QGis::Polygon:
      mPlacementComboBox->addItem( tr( "Around Centroid" ), QgsDiagramLayerSettings::AroundPoint );
      mPlacementComboBox->addItem( tr( "Over Centroid" ), QgsDiagramLayerSettings::OverPoint );
      mPlacementComboBox->addItem( tr( "Perimeter" ), QgsDiagramLayerSettings::Line );
      mPlacementComboBox->addItem( tr( "Inside Polygon" ), QgsDiagramLayerSettings::Horizontal );
      mLinePlacementFrame->setVisible( false );
      break;
    default:
      break;
  }
  mPlacementComboBox->blockSignals( false );

  mDiagramTypeComboBox->blockSignals( true );
  QPixmap pix = QgsApplication::getThemePixmap( "pie-chart" );
  mDiagramTypeComboBox->addItem( pix, tr( "Pie chart" ), DIAGRAM_NAME_PIE );
  pix = QgsApplication::getThemePixmap( "text" );
  mDiagramTypeComboBox->addItem( pix, tr( "Text diagram" ), DIAGRAM_NAME_TEXT );
  pix = QgsApplication::getThemePixmap( "histogram" );
  mDiagramTypeComboBox->addItem( pix, tr( "Histogram" ), DIAGRAM_NAME_HISTOGRAM );
  mDiagramTypeComboBox->blockSignals( false );

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

  QSettings settings;

  // reset horiz strech of left side of options splitter (set to 1 for previewing in Qt Designer)
  QSizePolicy policy( mDiagramOptionsListFrame->sizePolicy() );
  policy.setHorizontalStretch( 0 );
  mDiagramOptionsListFrame->setSizePolicy( policy );
  if ( !settings.contains( QString( "/Windows/Diagrams/OptionsSplitState" ) ) )
  {
    // set left list widget width on intial showing
    QList<int> splitsizes;
    splitsizes << 115;
    mDiagramOptionsSplitter->setSizes( splitsizes );
  }

  // restore dialog, splitters and current tab
  mDiagramOptionsSplitter->restoreState( settings.value( QString( "/Windows/Diagrams/OptionsSplitState" ) ).toByteArray() );
  mDiagramOptionsListWidget->setCurrentRow( settings.value( QString( "/Windows/Diagrams/Tab" ), 0 ).toInt() );

  // field combo and expression button
  mSizeFieldExpressionWidget->setLayer( mLayer );
  QgsDistanceArea myDa;
  myDa.setSourceCrs( mLayer->crs().srsid() );
  myDa.setEllipsoidalMode( QgisApp::instance()->mapCanvas()->mapSettings().hasCrsTransformEnabled() );
  myDa.setEllipsoid( QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ) );
  mSizeFieldExpressionWidget->setGeomCalculator( myDa );

  //insert all attributes into the combo boxes
  const QgsFields& layerFields = layer->fields();
  for ( int idx = 0; idx < layerFields.count(); ++idx )
  {
    QTreeWidgetItem *newItem = new QTreeWidgetItem( mAttributesTreeWidget );
    QString name = QString( "\"%1\"" ).arg( layerFields[idx].name() );
    newItem->setText( 0, name );
    newItem->setData( 0, Qt::UserRole, name );
    newItem->setFlags( newItem->flags() & ~Qt::ItemIsDropEnabled );

    mDataDefinedXComboBox->addItem( layerFields[idx].name(), idx );
    mDataDefinedYComboBox->addItem( layerFields[idx].name(), idx );
  }

  const QgsDiagramRendererV2* dr = layer->diagramRenderer();
  if ( !dr ) //no diagram renderer yet, insert reasonable default
  {
    mEnableDiagramsCheckBox->setChecked( false );
    mDiagramTypeFrame->setEnabled( false );
    mDiagramFrame->setEnabled( false );
    mFixedSizeRadio->setChecked( true );
    mDiagramUnitComboBox->setCurrentIndex( mDiagramUnitComboBox->findText( tr( "mm" ) ) );
    mLabelPlacementComboBox->setCurrentIndex( mLabelPlacementComboBox->findText( tr( "x-height" ) ) );
    mDiagramSizeSpinBox->setEnabled( true );
    mDiagramSizeSpinBox->setValue( 15 );
    mLinearScaleFrame->setEnabled( false );
    mIncreaseMinimumSizeSpinBox->setEnabled( false );
    mIncreaseMinimumSizeLabel->setEnabled( false );
    mBarWidthSpinBox->setValue( 5 );
    mScaleVisibilityGroupBox->setChecked( layer->hasScaleBasedVisibility() );
    mScaleRangeWidget->setScaleRange( 1.0 / layer->maximumScale(), 1.0 / layer->minimumScale() ); // caution: layer uses scale denoms, widget uses true scales
    mShowAllCheckBox->setChecked( true );

    switch ( layerType )
    {
      case QGis::Point:
        mPlacementComboBox->setCurrentIndex( mPlacementComboBox->findData( QgsDiagramLayerSettings::AroundPoint ) );
        break;
      case QGis::Line:
        mPlacementComboBox->setCurrentIndex( mPlacementComboBox->findData( QgsDiagramLayerSettings::Line ) );
        chkLineAbove->setChecked( true );
        chkLineBelow->setChecked( false );
        chkLineOn->setChecked( false );
        chkLineOrientationDependent->setChecked( false );
        break;
      case QGis::Polygon:
        mPlacementComboBox->setCurrentIndex( mPlacementComboBox->findData( QgsDiagramLayerSettings::AroundPoint ) );
        break;
      case QGis::UnknownGeometry:
      case QGis::NoGeometry:
        break;
    }
    mBackgroundColorButton->setColor( QColor( 255, 255, 255, 255 ) );
    mDiagramTypeComboBox->blockSignals( true );
    mDiagramTypeComboBox->setCurrentIndex( 0 );
    mDiagramTypeComboBox->blockSignals( false );
    //force a refresh of widget status to match diagram type
    on_mDiagramTypeComboBox_currentIndexChanged( mDiagramTypeComboBox->currentIndex() );
  }
  else // already a diagram renderer present
  {
    //single category renderer or interpolated one?
    if ( dr->rendererName() == "SingleCategory" )
    {
      mFixedSizeRadio->setChecked( true );
    }
    else
    {
      mAttributeBasedScalingRadio->setChecked( true );
    }
    mDiagramSizeSpinBox->setEnabled( mFixedSizeRadio->isChecked() );
    mLinearScaleFrame->setEnabled( mAttributeBasedScalingRadio->isChecked() );

    //assume single category or linearly interpolated diagram renderer for now
    QList<QgsDiagramSettings> settingList = dr->diagramSettings();
    if ( settingList.size() > 0 )
    {
      mEnableDiagramsCheckBox->setChecked( settingList.at( 0 ).enabled );
      mDiagramTypeFrame->setEnabled( mEnableDiagramsCheckBox->isChecked() );
      mDiagramFrame->setEnabled( mEnableDiagramsCheckBox->isChecked() );
      mDiagramFont = settingList.at( 0 ).font;
      QSizeF size = settingList.at( 0 ).size;
      mBackgroundColorButton->setColor( settingList.at( 0 ).backgroundColor );
      mTransparencySpinBox->setValue( settingList.at( 0 ).transparency * 100.0 / 255.0 );
      mDiagramPenColorButton->setColor( settingList.at( 0 ).penColor );
      mPenWidthSpinBox->setValue( settingList.at( 0 ).penWidth );
      mDiagramSizeSpinBox->setValue(( size.width() + size.height() ) / 2.0 );
      // caution: layer uses scale denoms, widget uses true scales
      mScaleRangeWidget->setScaleRange( 1.0 / ( settingList.at( 0 ).maxScaleDenominator > 0 ? settingList.at( 0 ).maxScaleDenominator : layer->maximumScale() ),
                                        1.0 / ( settingList.at( 0 ).minScaleDenominator > 0 ? settingList.at( 0 ).minScaleDenominator : layer->minimumScale() ) );
      mScaleVisibilityGroupBox->setChecked( settingList.at( 0 ).scaleBasedVisibility );
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

      mIncreaseSmallDiagramsCheck->setChecked( settingList.at( 0 ).minimumSize != 0 );
      mIncreaseMinimumSizeSpinBox->setEnabled( mIncreaseSmallDiagramsCheck->isChecked() );
      mIncreaseMinimumSizeLabel->setEnabled( mIncreaseSmallDiagramsCheck->isChecked() );

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
      QList< QString > categoryAttributes = settingList.at( 0 ).categoryAttributes;
      QList< QString > categoryLabels = settingList.at( 0 ).categoryLabels;
      QList< QString >::const_iterator catIt = categoryAttributes.constBegin();
      QList< QColor >::const_iterator coIt = categoryColors.constBegin();
      QList< QString >::const_iterator labIt = categoryLabels.constBegin();
      for ( ; catIt != categoryAttributes.constEnd(); ++catIt, ++coIt, ++labIt )
      {
        QTreeWidgetItem *newItem = new QTreeWidgetItem( mDiagramAttributesTreeWidget );
        newItem->setText( 0, *catIt );
        newItem->setData( 0, Qt::UserRole, *catIt );
        newItem->setFlags( newItem->flags() & ~Qt::ItemIsDropEnabled );
        QColor col( *coIt );
        col.setAlpha( 255 );
        newItem->setBackground( 1, QBrush( col ) );
        newItem->setText( 2, *labIt );
        newItem->setFlags( newItem->flags() | Qt::ItemIsEditable );
      }
    }

    if ( dr->rendererName() == "LinearlyInterpolated" )
    {
      const QgsLinearlyInterpolatedDiagramRenderer* lidr = dynamic_cast<const QgsLinearlyInterpolatedDiagramRenderer*>( dr );
      if ( lidr )
      {
        mDiagramSizeSpinBox->setEnabled( false );
        mLinearScaleFrame->setEnabled( true );
        mMaxValueSpinBox->setValue( lidr->upperValue() );
        mSizeSpinBox->setValue(( lidr->upperSize().width() + lidr->upperSize().height() ) / 2 );
        if ( lidr->classificationAttributeIsExpression() )
        {
          mSizeFieldExpressionWidget->setField( lidr->classificationAttributeExpression() );
        }
        else
        {
          mSizeFieldExpressionWidget->setField( mLayer->fields().at( lidr->classificationAttribute() ).name() );
        }
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

      chkLineAbove->setChecked( dls->placementFlags & QgsDiagramLayerSettings::AboveLine );
      chkLineBelow->setChecked( dls->placementFlags & QgsDiagramLayerSettings::BelowLine );
      chkLineOn->setChecked( dls->placementFlags & QgsDiagramLayerSettings::OnLine );
      if ( !( dls->placementFlags & QgsDiagramLayerSettings::MapOrientation ) )
        chkLineOrientationDependent->setChecked( true );

      mShowAllCheckBox->setChecked( dls->showAll );
    }

    if ( dr->diagram() )
    {
      QString diagramName = dr->diagram()->diagramName();
      mDiagramTypeComboBox->blockSignals( true );
      mDiagramTypeComboBox->setCurrentIndex( mDiagramTypeComboBox->findData( diagramName ) );
      mDiagramTypeComboBox->blockSignals( false );
      //force a refresh of widget status to match diagram type
      on_mDiagramTypeComboBox_currentIndexChanged( mDiagramTypeComboBox->currentIndex() );
      if ( mDiagramTypeComboBox->currentIndex() == -1 )
      {
        QMessageBox::warning( this, tr( "Unknown diagram type." ),
                              tr( "The diagram type '%1' is unknown. A default type is selected for you." ).arg( diagramName ), QMessageBox::Ok );
        mDiagramTypeComboBox->setCurrentIndex( mDiagramTypeComboBox->findData( DIAGRAM_NAME_PIE ) );
      }
    }
  } // if ( !dr )

  connect( mAddAttributeExpression, SIGNAL( clicked() ), this, SLOT( showAddAttributeExpressionDialog() ) );
  connect( mTransparencySlider, SIGNAL( valueChanged( int ) ), mTransparencySpinBox, SLOT( setValue( int ) ) );
  connect( mTransparencySpinBox, SIGNAL( valueChanged( int ) ), mTransparencySlider, SLOT( setValue( int ) ) );
}

QgsDiagramProperties::~QgsDiagramProperties()
{
  QSettings settings;
  settings.setValue( QString( "/Windows/Diagrams/OptionsSplitState" ), mDiagramOptionsSplitter->saveState() );
  settings.setValue( QString( "/Windows/Diagrams/Tab" ), mDiagramOptionsListWidget->currentRow() );
}

void QgsDiagramProperties::on_mDiagramTypeComboBox_currentIndexChanged( int index )
{
  QString diagramType = mDiagramTypeComboBox->itemData( index ).toString();

  if ( DIAGRAM_NAME_TEXT == diagramType )
  {
    mTextOptionsFrame->show();
    mBackgroundColorLabel->show();
    mBackgroundColorButton->show();
    mDiagramFontButton->show();
  }
  else
  {
    mTextOptionsFrame->hide();
    mBackgroundColorLabel->hide();
    mBackgroundColorButton->hide();
    mDiagramFontButton->hide();
  }

  if ( DIAGRAM_NAME_HISTOGRAM == diagramType )
  {
    mBarWidthLabel->show();
    mBarWidthSpinBox->show();
    mBarOptionsFrame->show();
    mAttributeBasedScalingRadio->setChecked( true );
    mFixedSizeRadio->setEnabled( false );
    mDiagramSizeSpinBox->setEnabled( false );
    mLinearlyScalingLabel->setText( tr( "Bar length: Scale linearly, so that the following value matches the specified bar length:" ) );
    mSizeLabel->setText( tr( "Bar length" ) );
    mFrameIncreaseSize->setVisible( false );
  }
  else
  {
    mBarWidthLabel->hide();
    mBarWidthSpinBox->hide();
    mBarOptionsFrame->hide();
    mLinearlyScalingLabel->setText( tr( "Scale linearly between 0 and the following attribute value / diagram size:" ) );
    mSizeLabel->setText( tr( "Size" ) );
    mAttributeBasedScalingRadio->setEnabled( true );
    mFixedSizeRadio->setEnabled( true );
    mDiagramSizeSpinBox->setEnabled( mFixedSizeRadio->isChecked() );
    mFrameIncreaseSize->setVisible( true );
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

QString QgsDiagramProperties::guessLegendText( const QString& expression )
{
  //trim unwanted characters from expression text for legend
  QString text = expression.mid( expression.startsWith( "\"" ) ? 1 : 0 );
  if ( text.endsWith( "\"" ) )
    text.chop( 1 );
  return text;
}

void QgsDiagramProperties::addAttribute( QTreeWidgetItem * item )
{
  QTreeWidgetItem *newItem = new QTreeWidgetItem( mDiagramAttributesTreeWidget );

  newItem->setText( 0, item->text( 0 ) );
  newItem->setText( 2, guessLegendText( item->text( 0 ) ) );
  newItem->setData( 0, Qt::UserRole, item->data( 0, Qt::UserRole ) );
  newItem->setFlags( newItem->flags() & ~Qt::ItemIsDropEnabled );

  //set initial color for diagram category
  int red = 1 + ( int )( 255.0 * qrand() / ( RAND_MAX + 1.0 ) );
  int green = 1 + ( int )( 255.0 * qrand() / ( RAND_MAX + 1.0 ) );
  int blue = 1 + ( int )( 255.0 * qrand() / ( RAND_MAX + 1.0 ) );
  QColor randomColor( red, green, blue );
  newItem->setBackground( 1, QBrush( randomColor ) );
  mDiagramAttributesTreeWidget->addTopLevelItem( newItem );
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
  if ( !mLayer )
    return;

  float maxValue = 0.0;

  bool isExpression;
  QString sizeFieldNameOrExp = mSizeFieldExpressionWidget->currentField( &isExpression );
  if ( isExpression )
  {
    QgsExpression exp( sizeFieldNameOrExp );
    QgsExpressionContext context;
    context << QgsExpressionContextUtils::globalScope()
    << QgsExpressionContextUtils::projectScope()
    << QgsExpressionContextUtils::layerScope( mLayer );

    exp.prepare( &context );
    if ( !exp.hasEvalError() )
    {
      QgsFeature feature;
      QgsFeatureIterator features = mLayer->getFeatures();
      while ( features.nextFeature( *&feature ) )
      {
        context.setFeature( feature );
        maxValue = qMax( maxValue, exp.evaluate( &context ).toFloat() );
      }
    }
    else
    {
      QgsDebugMsgLevel( "Prepare error:" + exp.evalErrorString(), 4 );
    }
  }
  else
  {
    int attributeNumber = mLayer->fields().fieldNameIndex( sizeFieldNameOrExp );
    maxValue = mLayer->maximumValue( attributeNumber ).toFloat();
  }

  mMaxValueSpinBox->setValue( maxValue );
}

void QgsDiagramProperties::on_mDiagramFontButton_clicked()
{
  bool ok;
  mDiagramFont = QgisGui::getFont( ok, mDiagramFont );
}


void QgsDiagramProperties::on_mDiagramAttributesTreeWidget_itemDoubleClicked( QTreeWidgetItem * item, int column )
{
  if ( column == 1 ) //change color
  {
    QColor newColor = QgsColorDialogV2::getColor( item->background( 1 ).color(), 0 );
    if ( newColor.isValid() )
    {
      item->setBackground( 1, QBrush( newColor ) );
    }
  }
}

void QgsDiagramProperties::on_mEngineSettingsButton_clicked()
{
  QgsLabelEngineConfigDialog dlg( this );
  dlg.exec();
}

void QgsDiagramProperties::apply()
{
  bool diagramsEnabled = mEnableDiagramsCheckBox->isChecked();

  QgsDiagram* diagram = 0;
  int index = mDiagramTypeComboBox->currentIndex();
  QString diagramType = mDiagramTypeComboBox->itemData( index ).toString();

  if ( diagramsEnabled && 0 == mDiagramAttributesTreeWidget->topLevelItemCount() )
  {
    QgisApp::instance()->messageBar()->pushMessage(
      tr( "Diagrams: No attributes added." ),
      tr( "You did not add any attributes to this diagram layer. Please specify the attributes to visualize on the diagrams or disable diagrams." ),
      QgsMessageBar::WARNING );
  }

#if 0
  bool scaleAttributeValueOk = false;
  // Check if a (usable) scale attribute value is inserted
  mValueLineEdit->text().toDouble( &scaleAttributeValueOk );

  if ( !mFixedSizeRadio->isChecked() && !scaleAttributeValueOk )
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
          QString fldName = mDiagramAttributesTreeWidget->topLevelItem( i )->data( 0, Qt::UserRole ).toString();
          if ( fldName.count() >= 2 && fldName.at( 0 ) == '"' && fldName.at( fldName.count() - 1 ) == '"' )
            fldName = fldName.mid( 1, fldName.count() - 2 ); // remove enclosing double quotes
          int fld = provider->fieldNameIndex( fldName );
          if ( fld != -1 )
          {
            bool ok = false;
            double val = provider->maximumValue( fld ).toDouble( &ok );
            if ( ok )
              maxVal = qMax( maxVal, val );
          }
        }
      }
      else
      {
        maxVal = provider->maximumValue( mSizeAttributeComboBox->itemData( mSizeAttributeComboBox->currentIndex() ).toInt() ).toDouble();
      }
    }

    if ( diagramsEnabled && maxVal != DBL_MIN )
    {
      QgisApp::instance()->messageBar()->pushMessage(
        tr( "Interpolation value" ),
        tr( "You did not specify an interpolation value. A default value of %1 has been set." ).arg( QString::number( maxVal ) ),
        QgsMessageBar::INFO,
        5 );

      mMaxValueSpinBox->setValue( maxVal );
    }
  }
#endif

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
  ds.enabled = mEnableDiagramsCheckBox->isChecked();
  ds.font = mDiagramFont;
  ds.transparency = mTransparencySpinBox->value() * 255.0 / 100.0;

  QList<QColor> categoryColors;
  QList<QString> categoryAttributes;
  QList<QString> categoryLabels;
  for ( int i = 0; i < mDiagramAttributesTreeWidget->topLevelItemCount(); ++i )
  {
    QColor color = mDiagramAttributesTreeWidget->topLevelItem( i )->background( 1 ).color();
    color.setAlpha( 255 - ds.transparency );
    categoryColors.append( color );
    categoryAttributes.append( mDiagramAttributesTreeWidget->topLevelItem( i )->data( 0, Qt::UserRole ).toString() );
    categoryLabels.append( mDiagramAttributesTreeWidget->topLevelItem( i )->text( 2 ) );
  }
  ds.categoryColors = categoryColors;
  ds.categoryAttributes = categoryAttributes;
  ds.categoryLabels = categoryLabels;
  ds.size = QSizeF( mDiagramSizeSpinBox->value(), mDiagramSizeSpinBox->value() );
  ds.sizeType = static_cast<QgsDiagramSettings::SizeType>( mDiagramUnitComboBox->itemData( mDiagramUnitComboBox->currentIndex() ).toInt() );
  ds.labelPlacementMethod = static_cast<QgsDiagramSettings::LabelPlacementMethod>( mLabelPlacementComboBox->itemData( mLabelPlacementComboBox->currentIndex() ).toInt() );
  ds.scaleByArea = mScaleDependencyComboBox->itemData( mScaleDependencyComboBox->currentIndex() ).toBool();

  if ( mIncreaseSmallDiagramsCheck->isChecked() )
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
  // caution: layer uses scale denoms, widget uses true scales
  ds.maxScaleDenominator = 1.0 / mScaleRangeWidget->minimumScale();
  ds.minScaleDenominator = 1.0 / mScaleRangeWidget->maximumScale();
  ds.scaleBasedVisibility = mScaleVisibilityGroupBox->isChecked();

  // Diagram angle offset (pie)
  ds.angleOffset = mAngleOffsetComboBox->itemData( mAngleOffsetComboBox->currentIndex() ).toInt();

  // Diagram orientation (histogram)
  ds.diagramOrientation = static_cast<QgsDiagramSettings::DiagramOrientation>( mOrientationButtonGroup->checkedButton()->property( "direction" ).toInt() );

  ds.barWidth = mBarWidthSpinBox->value();

  if ( mFixedSizeRadio->isChecked() )
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
    dr->setUpperValue( mMaxValueSpinBox->value() );
    dr->setUpperSize( QSizeF( mSizeSpinBox->value(), mSizeSpinBox->value() ) );

    bool isExpression;
    QString sizeFieldNameOrExp = mSizeFieldExpressionWidget->currentField( &isExpression );
    dr->setClassificationAttributeIsExpression( isExpression );
    if ( isExpression )
    {
      dr->setClassificationAttributeExpression( sizeFieldNameOrExp );
    }
    else
    {
      int attributeNumber = mLayer->fields().fieldNameIndex( sizeFieldNameOrExp );
      dr->setClassificationAttribute( attributeNumber );
    }
    dr->setDiagram( diagram );
    dr->setDiagramSettings( ds );
    mLayer->setDiagramRenderer( dr );
  }

  QgsDiagramLayerSettings dls;
  dls.dist = mDiagramDistanceSpinBox->value();
  dls.priority = mPrioritySlider->value();
  dls.showAll = mShowAllCheckBox->isChecked();
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
  dls.placementFlags = ( QgsDiagramLayerSettings::LinePlacementFlags )0;
  if ( chkLineAbove->isChecked() )
    dls.placementFlags |= QgsDiagramLayerSettings::AboveLine;
  if ( chkLineBelow->isChecked() )
    dls.placementFlags |= QgsDiagramLayerSettings::BelowLine;
  if ( chkLineOn->isChecked() )
    dls.placementFlags |= QgsDiagramLayerSettings::OnLine;
  if ( ! chkLineOrientationDependent->isChecked() )
    dls.placementFlags |= QgsDiagramLayerSettings::MapOrientation;

  mLayer->setDiagramLayerSettings( dls );
}

void QgsDiagramProperties::showAddAttributeExpressionDialog()
{
  QString expression;
  QList<QTreeWidgetItem *> selections = mAttributesTreeWidget->selectedItems();
  if ( !selections.empty() )
  {
    expression = selections[0]->text( 0 );
  }

  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::layerScope( mLayer );

  QgsExpressionBuilderDialog dlg( mLayer, expression, this, "generic", context );
  dlg.setWindowTitle( tr( "Expression based attribute" ) );

  QgsDistanceArea myDa;
  myDa.setSourceCrs( mLayer->crs().srsid() );
  myDa.setEllipsoidalMode( QgisApp::instance()->mapCanvas()->mapSettings().hasCrsTransformEnabled() );
  myDa.setEllipsoid( QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ) );
  dlg.setGeomCalculator( myDa );

  if ( dlg.exec() == QDialog::Accepted )
  {
    QString expression =  dlg.expressionText();
    //Only add the expression if the user has entered some text.
    if ( !expression.isEmpty() )
    {
      QTreeWidgetItem *newItem = new QTreeWidgetItem( mDiagramAttributesTreeWidget );

      newItem->setText( 0, expression );
      newItem->setText( 2, expression );
      newItem->setData( 0, Qt::UserRole, expression );
      newItem->setFlags( newItem->flags() & ~Qt::ItemIsDropEnabled );

      //set initial color for diagram category
      int red = 1 + ( int )( 255.0 * qrand() / ( RAND_MAX + 1.0 ) );
      int green = 1 + ( int )( 255.0 * qrand() / ( RAND_MAX + 1.0 ) );
      int blue = 1 + ( int )( 255.0 * qrand() / ( RAND_MAX + 1.0 ) );
      QColor randomColor( red, green, blue );
      newItem->setBackground( 1, QBrush( randomColor ) );
      mDiagramAttributesTreeWidget->addTopLevelItem( newItem );
    }
  }
  activateWindow(); // set focus back parent
}

void QgsDiagramProperties::on_mDiagramStackedWidget_currentChanged( int index )
{
  mDiagramOptionsListWidget->blockSignals( true );
  mDiagramOptionsListWidget->setCurrentRow( index );
  mDiagramOptionsListWidget->blockSignals( false );
}

void QgsDiagramProperties::on_mPlacementComboBox_currentIndexChanged( int index )
{
  QgsDiagramLayerSettings::Placement currentPlacement = ( QgsDiagramLayerSettings::Placement )mPlacementComboBox->itemData( index ).toInt();
  if ( currentPlacement == QgsDiagramLayerSettings::AroundPoint ||
       ( currentPlacement == QgsDiagramLayerSettings::Line && mLayer->geometryType() == QGis::Line ) )
  {
    mDiagramDistanceLabel->setEnabled( true );
    mDiagramDistanceSpinBox->setEnabled( true );
  }
  else
  {
    mDiagramDistanceLabel->setEnabled( false );
    mDiagramDistanceSpinBox->setEnabled( false );
  }

  bool linePlacementEnabled = mLayer->geometryType() == QGis::Line && currentPlacement == QgsDiagramLayerSettings::Line;
  chkLineAbove->setEnabled( linePlacementEnabled );
  chkLineBelow->setEnabled( linePlacementEnabled );
  chkLineOn->setEnabled( linePlacementEnabled );
  chkLineOrientationDependent->setEnabled( linePlacementEnabled );
}
