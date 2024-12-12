/***************************************************************************
  qgsdiagramproperties.cpp
  Adjust the properties for diagrams
  -------------------
         begin                : August 2012
         copyright            : (C) Matthias Kuhn
         email                : matthias at opengis dot ch

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
#include "diagram/qgsstackedbardiagram.h"
#include "diagram/qgsstackeddiagram.h"

#include "qgsproject.h"
#include "qgsapplication.h"
#include "qgsdatadefinedsizelegend.h"
#include "qgsdatadefinedsizelegendwidget.h"
#include "qgsdiagramproperties.h"
#include "moc_qgsdiagramproperties.cpp"
#include "qgsdiagramrenderer.h"
#include "qgsfeatureiterator.h"
#include "qgssymbolselectordialog.h"
#include "qgsmapcanvas.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsnewauxiliarylayerdialog.h"
#include "qgsauxiliarystorage.h"
#include "qgsexpressioncontextutils.h"
#include "qgspropertytransformer.h"
#include "qgspainteffectregistry.h"
#include "qgspainteffect.h"
#include "qgslinesymbol.h"

#include <QList>
#include <QMessageBox>
#include <QStyledItemDelegate>
#include <QRandomGenerator>

QgsExpressionContext QgsDiagramProperties::createExpressionContext() const
{
  QgsExpressionContext expContext;
  if ( mMapCanvas )
  {
    expContext = mMapCanvas->createExpressionContext();
  }
  else
  {
    expContext << QgsExpressionContextUtils::globalScope()
               << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
               << QgsExpressionContextUtils::atlasScope( nullptr )
               << QgsExpressionContextUtils::mapSettingsScope( QgsMapSettings() );
  }
  expContext << QgsExpressionContextUtils::layerScope( mLayer );

  return expContext;
}

QgsDiagramProperties::QgsDiagramProperties( QgsVectorLayer *layer, QWidget *parent, QgsMapCanvas *canvas )
  : QgsPanelWidget( parent )
  , mLayer( layer )
  , mMapCanvas( canvas )
{
  if ( !layer )
  {
    return;
  }

  setupUi( this );
  connect( mDiagramTypeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsDiagramProperties::mDiagramTypeComboBox_currentIndexChanged );
  connect( mAddCategoryPushButton, &QPushButton::clicked, this, &QgsDiagramProperties::mAddCategoryPushButton_clicked );
  connect( mAttributesTreeWidget, &QTreeWidget::itemDoubleClicked, this, &QgsDiagramProperties::mAttributesTreeWidget_itemDoubleClicked );
  connect( mFindMaximumValueButton, &QPushButton::clicked, this, &QgsDiagramProperties::mFindMaximumValueButton_clicked );
  connect( mRemoveCategoryPushButton, &QPushButton::clicked, this, &QgsDiagramProperties::mRemoveCategoryPushButton_clicked );
  connect( mDiagramAttributesTreeWidget, &QTreeWidget::itemDoubleClicked, this, &QgsDiagramProperties::mDiagramAttributesTreeWidget_itemDoubleClicked );
  connect( mDiagramStackedWidget, &QStackedWidget::currentChanged, this, &QgsDiagramProperties::mDiagramStackedWidget_currentChanged );

  // get rid of annoying outer focus rect on Mac
  mDiagramOptionsListWidget->setAttribute( Qt::WA_MacShowFocusRect, false );

  const int iconSize = QgsGuiUtils::scaleIconSize( 20 );
  mOptionsTab->setIconSize( QSize( iconSize, iconSize ) );
  mDiagramOptionsListWidget->setIconSize( QSize( iconSize, iconSize ) );

  mBarSpacingSpinBox->setClearValue( 0 );
  mBarSpacingUnitComboBox->setUnits( { Qgis::RenderUnit::Millimeters, Qgis::RenderUnit::MetersInMapUnits, Qgis::RenderUnit::MapUnits, Qgis::RenderUnit::Pixels, Qgis::RenderUnit::Points, Qgis::RenderUnit::Inches } );

  mDiagramFontButton->setMode( QgsFontButton::ModeQFont );

  mDiagramTypeComboBox->blockSignals( true );
  QIcon icon = QgsApplication::getThemeIcon( QStringLiteral( "pie-chart.svg" ) );
  mDiagramTypeComboBox->addItem( icon, tr( "Pie Chart" ), QgsPieDiagram::DIAGRAM_NAME_PIE );
  icon = QgsApplication::getThemeIcon( QStringLiteral( "text.svg" ) );
  mDiagramTypeComboBox->addItem( icon, tr( "Text Diagram" ), QgsTextDiagram::DIAGRAM_NAME_TEXT );
  icon = QgsApplication::getThemeIcon( QStringLiteral( "histogram.svg" ) );
  mDiagramTypeComboBox->addItem( icon, tr( "Histogram" ), QgsHistogramDiagram::DIAGRAM_NAME_HISTOGRAM );
  icon = QgsApplication::getThemeIcon( QStringLiteral( "stacked-bar.svg" ) );
  mDiagramTypeComboBox->addItem( icon, tr( "Stacked Bars" ), QgsStackedBarDiagram::DIAGRAM_NAME_STACKED_BAR );
  mDiagramTypeComboBox->blockSignals( false );

  mAxisLineStyleButton->setSymbolType( Qgis::SymbolType::Line );
  mAxisLineStyleButton->setDialogTitle( tr( "Axis Line Symbol" ) );

  mScaleRangeWidget->setMapCanvas( mMapCanvas );
  mSizeFieldExpressionWidget->registerExpressionContextGenerator( this );

  mBackgroundColorButton->setColorDialogTitle( tr( "Select Background Color" ) );
  mBackgroundColorButton->setAllowOpacity( true );
  mBackgroundColorButton->setContext( QStringLiteral( "symbology" ) );
  mBackgroundColorButton->setShowNoColor( true );
  mBackgroundColorButton->setNoColorString( tr( "Transparent Background" ) );
  mDiagramPenColorButton->setColorDialogTitle( tr( "Select Pen Color" ) );
  mDiagramPenColorButton->setAllowOpacity( true );
  mDiagramPenColorButton->setContext( QStringLiteral( "symbology" ) );
  mDiagramPenColorButton->setShowNoColor( true );
  mDiagramPenColorButton->setNoColorString( tr( "Transparent Stroke" ) );

  mMaxValueSpinBox->setShowClearButton( false );
  mSizeSpinBox->setClearValue( 5 );

  mDiagramAttributesTreeWidget->setItemDelegateForColumn( ColumnAttributeExpression, new EditBlockerDelegate( this ) );
  mDiagramAttributesTreeWidget->setItemDelegateForColumn( ColumnColor, new QgsColorSwatchDelegate( this ) );

  mDiagramAttributesTreeWidget->setColumnWidth( ColumnColor, Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'X' ) * 6.6 );

  connect( mFixedSizeRadio, &QRadioButton::toggled, this, &QgsDiagramProperties::scalingTypeChanged );
  connect( mAttributeBasedScalingRadio, &QRadioButton::toggled, this, &QgsDiagramProperties::scalingTypeChanged );

  mDiagramUnitComboBox->setUnits( { Qgis::RenderUnit::Millimeters, Qgis::RenderUnit::MapUnits, Qgis::RenderUnit::Pixels, Qgis::RenderUnit::Points, Qgis::RenderUnit::Inches } );
  mDiagramLineUnitComboBox->setUnits( { Qgis::RenderUnit::Millimeters, Qgis::RenderUnit::MapUnits, Qgis::RenderUnit::Pixels, Qgis::RenderUnit::Points, Qgis::RenderUnit::Inches } );

  const Qgis::GeometryType layerType = layer->geometryType();
  if ( layerType == Qgis::GeometryType::Unknown || layerType == Qgis::GeometryType::Null )
  {
    mDiagramTypeComboBox->setEnabled( false );
    mOptionsTab->setEnabled( false );
    mDiagramFrame->setEnabled( false );
  }

  // set placement methods page based on geometry type

  switch ( layerType )
  {
    case Qgis::GeometryType::Point:
      stackedPlacement->setCurrentWidget( pagePoint );
      mLinePlacementFrame->setVisible( false );
      break;
    case Qgis::GeometryType::Line:
      stackedPlacement->setCurrentWidget( pageLine );
      mLinePlacementFrame->setVisible( true );
      break;
    case Qgis::GeometryType::Polygon:
      stackedPlacement->setCurrentWidget( pagePolygon );
      mLinePlacementFrame->setVisible( false );
      break;
    case Qgis::GeometryType::Null:
    case Qgis::GeometryType::Unknown:
      break;
  }

  //insert placement options
  // setup point placement button group
  mPlacePointBtnGrp = new QButtonGroup( this );
  mPlacePointBtnGrp->addButton( radAroundPoint );
  mPlacePointBtnGrp->addButton( radOverPoint );
  mPlacePointBtnGrp->setExclusive( true );
  connect( mPlacePointBtnGrp, qOverload<QAbstractButton *>( &QButtonGroup::buttonClicked ), this, &QgsDiagramProperties::updatePlacementWidgets );

  // setup line placement button group
  mPlaceLineBtnGrp = new QButtonGroup( this );
  mPlaceLineBtnGrp->addButton( radAroundLine );
  mPlaceLineBtnGrp->addButton( radOverLine );
  mPlaceLineBtnGrp->setExclusive( true );
  connect( mPlaceLineBtnGrp, qOverload<QAbstractButton *>( &QButtonGroup::buttonClicked ), this, &QgsDiagramProperties::updatePlacementWidgets );

  // setup polygon placement button group
  mPlacePolygonBtnGrp = new QButtonGroup( this );
  mPlacePolygonBtnGrp->addButton( radAroundCentroid );
  mPlacePolygonBtnGrp->addButton( radOverCentroid );
  mPlacePolygonBtnGrp->addButton( radPolygonPerimeter );
  mPlacePolygonBtnGrp->addButton( radInsidePolygon );
  mPlacePolygonBtnGrp->setExclusive( true );
  connect( mPlacePolygonBtnGrp, qOverload<QAbstractButton *>( &QButtonGroup::buttonClicked ), this, &QgsDiagramProperties::updatePlacementWidgets );

  mLabelPlacementComboBox->addItem( tr( "Height" ), QgsDiagramSettings::Height );
  mLabelPlacementComboBox->addItem( tr( "x-height" ), QgsDiagramSettings::XHeight );

  mScaleDependencyComboBox->addItem( tr( "Area" ), true );
  mScaleDependencyComboBox->addItem( tr( "Diameter" ), false );

  mAngleOffsetComboBox->addItem( tr( "Top" ), 270 );
  mAngleOffsetComboBox->addItem( tr( "Right" ), 0 );
  mAngleOffsetComboBox->addItem( tr( "Bottom" ), 90 );
  mAngleOffsetComboBox->addItem( tr( "Left" ), 180 );

  mAngleDirectionComboBox->addItem( tr( "Clockwise" ), QgsDiagramSettings::Clockwise );
  mAngleDirectionComboBox->addItem( tr( "Counter-clockwise" ), QgsDiagramSettings::Counterclockwise );

  const QgsSettings settings;

  // reset horiz stretch of left side of options splitter (set to 1 for previewing in Qt Designer)
  QSizePolicy policy( mDiagramOptionsListFrame->sizePolicy() );
  policy.setHorizontalStretch( 0 );
  mDiagramOptionsListFrame->setSizePolicy( policy );
  if ( !settings.contains( QStringLiteral( "/Windows/Diagrams/OptionsSplitState" ) ) )
  {
    // set left list widget width on initial showing
    QList<int> splitsizes;
    splitsizes << 115;
    mDiagramOptionsSplitter->setSizes( splitsizes );
  }

  // restore dialog, splitters and current tab
  mDiagramOptionsSplitter->restoreState( settings.value( QStringLiteral( "Windows/Diagrams/OptionsSplitState" ) ).toByteArray() );
  mDiagramOptionsListWidget->setCurrentRow( settings.value( QStringLiteral( "Windows/Diagrams/Tab" ), 0 ).toInt() );

  // set correct initial tab to match displayed setting page
  whileBlocking( mOptionsTab )->setCurrentIndex( mDiagramStackedWidget->currentIndex() );
  mOptionsTab->tabBar()->setUsesScrollButtons( true );

  // field combo and expression button
  mSizeFieldExpressionWidget->setLayer( mLayer );
  QgsDistanceArea myDa;
  myDa.setSourceCrs( mLayer->crs(), QgsProject::instance()->transformContext() );
  myDa.setEllipsoid( QgsProject::instance()->ellipsoid() );
  mSizeFieldExpressionWidget->setGeomCalculator( myDa );

  //insert all attributes into the combo boxes
  const QgsFields &layerFields = layer->fields();
  for ( int idx = 0; idx < layerFields.count(); ++idx )
  {
    QTreeWidgetItem *newItem = new QTreeWidgetItem( mAttributesTreeWidget );
    const QString name = QStringLiteral( "\"%1\"" ).arg( layerFields.at( idx ).name() );
    newItem->setText( 0, name );
    newItem->setData( 0, RoleAttributeExpression, name );
    newItem->setFlags( newItem->flags() & ~Qt::ItemIsDropEnabled );
  }

  mPaintEffect.reset( QgsPaintEffectRegistry::defaultStack() );
  mPaintEffect->setEnabled( false );

  mOrientationLeftButton->setProperty( "direction", QgsDiagramSettings::Left );
  mOrientationRightButton->setProperty( "direction", QgsDiagramSettings::Right );
  mOrientationUpButton->setProperty( "direction", QgsDiagramSettings::Up );
  mOrientationDownButton->setProperty( "direction", QgsDiagramSettings::Down );

  // Labels to let users know some widgets are not present
  // when editing sub diagrams in a stacked diagram.
  mDlsLabel_1->hide();
  mDlsLabel_2->hide();

  insertDefaults();
  mPaintEffectWidget->setPaintEffect( mPaintEffect.get() );

  connect( mAddAttributeExpression, &QPushButton::clicked, this, &QgsDiagramProperties::showAddAttributeExpressionDialog );
  registerDataDefinedButton( mBackgroundColorDDBtn, QgsDiagramLayerSettings::Property::BackgroundColor );
  registerDataDefinedButton( mLineColorDDBtn, QgsDiagramLayerSettings::Property::StrokeColor );
  registerDataDefinedButton( mLineWidthDDBtn, QgsDiagramLayerSettings::Property::StrokeWidth );
  registerDataDefinedButton( mCoordXDDBtn, QgsDiagramLayerSettings::Property::PositionX );
  registerDataDefinedButton( mCoordYDDBtn, QgsDiagramLayerSettings::Property::PositionY );
  registerDataDefinedButton( mDistanceDDBtn, QgsDiagramLayerSettings::Property::Distance );
  registerDataDefinedButton( mPriorityDDBtn, QgsDiagramLayerSettings::Property::Priority );
  registerDataDefinedButton( mZOrderDDBtn, QgsDiagramLayerSettings::Property::ZIndex );
  registerDataDefinedButton( mShowDiagramDDBtn, QgsDiagramLayerSettings::Property::Show );
  registerDataDefinedButton( mAlwaysShowDDBtn, QgsDiagramLayerSettings::Property::AlwaysShow );
  registerDataDefinedButton( mIsObstacleDDBtn, QgsDiagramLayerSettings::Property::IsObstacle );
  registerDataDefinedButton( mStartAngleDDBtn, QgsDiagramLayerSettings::Property::StartAngle );

  connect( mButtonSizeLegendSettings, &QPushButton::clicked, this, &QgsDiagramProperties::showSizeLegendDialog );

  QList<QWidget *> widgets;
  widgets << chkLineAbove;
  widgets << chkLineBelow;
  widgets << chkLineOn;
  widgets << chkLineOrientationDependent;
  widgets << mAngleDirectionComboBox;
  widgets << mAngleOffsetComboBox;
  widgets << mAttributeBasedScalingRadio;
  widgets << mAxisLineStyleButton;
  widgets << mBackgroundColorButton;
  widgets << mBarSpacingSpinBox;
  widgets << mBarSpacingUnitComboBox;
  widgets << mBarWidthSpinBox;
  widgets << mCheckBoxAttributeLegend;
  widgets << mDiagramAttributesTreeWidget;
  widgets << mDiagramDistanceSpinBox;
  widgets << mDiagramFontButton;
  widgets << mDiagramPenColorButton;
  widgets << mDiagramSizeSpinBox;
  widgets << mDiagramLineUnitComboBox;
  widgets << mDiagramTypeComboBox;
  widgets << mDiagramUnitComboBox;
  widgets << mEnableDiagramCheckBox;
  widgets << mFixedSizeRadio;
  widgets << mIncreaseMinimumSizeSpinBox;
  widgets << mIncreaseSmallDiagramsGroupBox;
  widgets << mLabelPlacementComboBox;
  widgets << mMaxValueSpinBox;
  widgets << mPaintEffectWidget;
  widgets << mPenWidthSpinBox;
  widgets << mPrioritySlider;
  widgets << mOpacityWidget;
  widgets << mOrientationDownButton;
  widgets << mOrientationLeftButton;
  widgets << mOrientationRightButton;
  widgets << mOrientationUpButton;
  widgets << mScaleDependencyComboBox;
  widgets << mScaleRangeWidget;
  widgets << mScaleVisibilityGroupBox;
  widgets << mShowAllCheckBox;
  widgets << mShowAxisGroupBox;
  widgets << mSizeFieldExpressionWidget;
  widgets << mSizeSpinBox;
  widgets << mZIndexSpinBox;
  widgets << radAroundCentroid;
  widgets << radAroundLine;
  widgets << radAroundPoint;
  widgets << radInsidePolygon;
  widgets << radOverCentroid;
  widgets << radOverLine;
  widgets << radOverPoint;
  widgets << radPolygonPerimeter;

  connectValueChanged( widgets );
}

void QgsDiagramProperties::setDockMode( bool dockMode )
{
  QgsPanelWidget::setDockMode( dockMode );
  mOptionsTab->setVisible( dockMode );
  mOptionsTab->setTabToolTip( 0, tr( "Attributes" ) );
  mOptionsTab->setTabToolTip( 1, tr( "Rendering" ) );
  mOptionsTab->setTabToolTip( 2, tr( "Size" ) );
  mOptionsTab->setTabToolTip( 3, tr( "Placement" ) );
  mOptionsTab->setTabToolTip( 4, tr( "Options" ) );
  mOptionsTab->setTabToolTip( 5, tr( "Legend" ) );
  mDiagramOptionsListFrame->setVisible( !dockMode );
}

void QgsDiagramProperties::setDiagramType( const QString diagramType )
{
  mDiagramType = diagramType;

  mDiagramTypeComboBox->setVisible( false );
  mDiagramTypeComboBox->blockSignals( true );
  mDiagramTypeComboBox->setCurrentIndex( mDiagramTypeComboBox->findData( mDiagramType ) );
  mDiagramTypeComboBox->blockSignals( false );

  //force a refresh of widget status to match diagram type
  mDiagramTypeComboBox_currentIndexChanged( mDiagramTypeComboBox->currentIndex() );
}

void QgsDiagramProperties::insertDefaults()
{
  mFixedSizeRadio->setChecked( true );
  mDiagramUnitComboBox->setUnit( Qgis::RenderUnit::Millimeters );
  mDiagramLineUnitComboBox->setUnit( Qgis::RenderUnit::Millimeters );
  mLabelPlacementComboBox->setCurrentIndex( mLabelPlacementComboBox->findText( tr( "x-height" ) ) );
  mDiagramSizeSpinBox->setEnabled( true );
  mDiagramSizeSpinBox->setValue( 15 );
  mLinearScaleFrame->setEnabled( false );
  mBarWidthSpinBox->setValue( 5 );
  mScaleVisibilityGroupBox->setChecked( mLayer->hasScaleBasedVisibility() );
  mScaleRangeWidget->setScaleRange( mLayer->minimumScale(), mLayer->maximumScale() );
  mShowAllCheckBox->setChecked( true );
  mCheckBoxAttributeLegend->setChecked( true );

  switch ( mLayer->geometryType() )
  {
    case Qgis::GeometryType::Point:
      radAroundPoint->setChecked( true );
      break;

    case Qgis::GeometryType::Line:
      radAroundLine->setChecked( true );
      chkLineAbove->setChecked( true );
      chkLineBelow->setChecked( false );
      chkLineOn->setChecked( false );
      chkLineOrientationDependent->setChecked( false );
      break;

    case Qgis::GeometryType::Polygon:
      radOverCentroid->setChecked( true );
      mDiagramDistanceLabel->setEnabled( false );
      mDiagramDistanceSpinBox->setEnabled( false );
      mDistanceDDBtn->setEnabled( false );
      break;

    case Qgis::GeometryType::Unknown:
    case Qgis::GeometryType::Null:
      break;
  }
  mBackgroundColorButton->setColor( QColor( 255, 255, 255, 255 ) );
  mDiagramPenColorButton->setColor( QColor( 0, 0, 0, 255 ) );
  //force a refresh of widget status to match diagram type
  mDiagramTypeComboBox_currentIndexChanged( mDiagramTypeComboBox->currentIndex() );
}

void QgsDiagramProperties::syncToLayer()
{
  const QgsDiagramRenderer *renderer = mLayer->diagramRenderer();
  if ( renderer && renderer->rendererName() == QgsStackedDiagramRenderer::DIAGRAM_RENDERER_NAME_STACKED )
  {
    const QgsStackedDiagramRenderer *stackedRenderer = static_cast<const QgsStackedDiagramRenderer *>( renderer );
    if ( stackedRenderer->rendererCount() > 0 )
    {
      // If layer has a stacked diagram renderer, take its first sub
      // renderer as the basis for the new single one being created
      renderer = stackedRenderer->renderer( 0 );
    }
  }
  syncToRenderer( renderer );

  const QgsDiagramLayerSettings *layerDls = mLayer->diagramLayerSettings();
  syncToSettings( layerDls );
}

void QgsDiagramProperties::syncToRenderer( const QgsDiagramRenderer *dr )
{
  mDiagramAttributesTreeWidget->clear();

  if ( !dr ) //no diagram renderer yet, insert reasonable default
  {
    insertDefaults();
  }
  else // already a diagram renderer present
  {
    //single category renderer or interpolated one?
    if ( dr->rendererName() == QgsSingleCategoryDiagramRenderer::DIAGRAM_RENDERER_NAME_SINGLE_CATEGORY )
    {
      mFixedSizeRadio->setChecked( true );
    }
    else
    {
      mAttributeBasedScalingRadio->setChecked( true );
    }
    mDiagramSizeSpinBox->setEnabled( mFixedSizeRadio->isChecked() );
    mLinearScaleFrame->setEnabled( mAttributeBasedScalingRadio->isChecked() );
    mCheckBoxAttributeLegend->setChecked( dr->attributeLegend() );

    // Assume single category or linearly interpolated diagram renderer for now.
    const QList<QgsDiagramSettings> settingList = dr->diagramSettings();
    if ( !settingList.isEmpty() )
    {
      setDiagramEnabled( settingList.at( 0 ).enabled );
      mDiagramFontButton->setCurrentFont( settingList.at( 0 ).font );
      const QSizeF size = settingList.at( 0 ).size;
      mBackgroundColorButton->setColor( settingList.at( 0 ).backgroundColor );
      mOpacityWidget->setOpacity( settingList.at( 0 ).opacity );
      mDiagramPenColorButton->setColor( settingList.at( 0 ).penColor );
      mPenWidthSpinBox->setValue( settingList.at( 0 ).penWidth );
      mDiagramSizeSpinBox->setValue( ( size.width() + size.height() ) / 2.0 );
      mScaleRangeWidget->setScaleRange( ( settingList.at( 0 ).minimumScale > 0 ? settingList.at( 0 ).minimumScale : mLayer->minimumScale() ), ( settingList.at( 0 ).maximumScale > 0 ? settingList.at( 0 ).maximumScale : mLayer->maximumScale() ) );
      mScaleVisibilityGroupBox->setChecked( settingList.at( 0 ).scaleBasedVisibility );
      mDiagramUnitComboBox->setUnit( settingList.at( 0 ).sizeType );
      mDiagramUnitComboBox->setMapUnitScale( settingList.at( 0 ).sizeScale );
      mDiagramLineUnitComboBox->setUnit( settingList.at( 0 ).lineSizeUnit );
      mDiagramLineUnitComboBox->setMapUnitScale( settingList.at( 0 ).lineSizeScale );

      if ( settingList.at( 0 ).labelPlacementMethod == QgsDiagramSettings::Height )
      {
        mLabelPlacementComboBox->setCurrentIndex( 0 );
      }
      else
      {
        mLabelPlacementComboBox->setCurrentIndex( 1 );
      }

      if ( settingList.at( 0 ).paintEffect() )
        mPaintEffect.reset( settingList.at( 0 ).paintEffect()->clone() );

      mAngleOffsetComboBox->setCurrentIndex( mAngleOffsetComboBox->findData( settingList.at( 0 ).rotationOffset ) );
      mAngleDirectionComboBox->setCurrentIndex( mAngleDirectionComboBox->findData( settingList.at( 0 ).direction() ) );

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
      mBarSpacingSpinBox->setValue( settingList.at( 0 ).spacing() );
      mBarSpacingUnitComboBox->setUnit( settingList.at( 0 ).spacingUnit() );
      mBarSpacingUnitComboBox->setMapUnitScale( settingList.at( 0 ).spacingMapUnitScale() );

      mShowAxisGroupBox->setChecked( settingList.at( 0 ).showAxis() );
      if ( settingList.at( 0 ).axisLineSymbol() )
        mAxisLineStyleButton->setSymbol( settingList.at( 0 ).axisLineSymbol()->clone() );

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

      const QList<QColor> categoryColors = settingList.at( 0 ).categoryColors;
      const QList<QString> categoryAttributes = settingList.at( 0 ).categoryAttributes;
      const QList<QString> categoryLabels = settingList.at( 0 ).categoryLabels;
      QList<QString>::const_iterator catIt = categoryAttributes.constBegin();
      QList<QColor>::const_iterator coIt = categoryColors.constBegin();
      QList<QString>::const_iterator labIt = categoryLabels.constBegin();
      for ( ; catIt != categoryAttributes.constEnd(); ++catIt, ++coIt, ++labIt )
      {
        QTreeWidgetItem *newItem = new QTreeWidgetItem( mDiagramAttributesTreeWidget );
        newItem->setText( 0, *catIt );
        newItem->setData( 0, RoleAttributeExpression, *catIt );
        newItem->setFlags( newItem->flags() & ~Qt::ItemIsDropEnabled );
        newItem->setData( ColumnColor, Qt::EditRole, *coIt );
        newItem->setText( 2, *labIt );
        newItem->setFlags( newItem->flags() | Qt::ItemIsEditable );
      }
    }

    if ( dr->rendererName() == QgsLinearlyInterpolatedDiagramRenderer::DIAGRAM_RENDERER_NAME_LINEARLY_INTERPOLATED )
    {
      const QgsLinearlyInterpolatedDiagramRenderer *lidr = dynamic_cast<const QgsLinearlyInterpolatedDiagramRenderer *>( dr );
      if ( lidr )
      {
        mDiagramSizeSpinBox->setEnabled( false );
        mLinearScaleFrame->setEnabled( true );
        mMaxValueSpinBox->setValue( lidr->upperValue() );
        mSizeSpinBox->setValue( ( lidr->upperSize().width() + lidr->upperSize().height() ) / 2 );
        if ( lidr->classificationAttributeIsExpression() )
        {
          mSizeFieldExpressionWidget->setField( lidr->classificationAttributeExpression() );
        }
        else
        {
          mSizeFieldExpressionWidget->setField( lidr->classificationField() );
        }

        mSizeLegend.reset( lidr->dataDefinedSizeLegend() ? new QgsDataDefinedSizeLegend( *lidr->dataDefinedSizeLegend() ) : nullptr );
      }
    }

    if ( dr->diagram() )
    {
      mDiagramType = dr->diagram()->diagramName();

      mDiagramTypeComboBox->blockSignals( true );
      mDiagramTypeComboBox->setCurrentIndex( mDiagramTypeComboBox->findData( mDiagramType ) );
      mDiagramTypeComboBox->blockSignals( false );
      //force a refresh of widget status to match diagram type
      mDiagramTypeComboBox_currentIndexChanged( mDiagramTypeComboBox->currentIndex() );
    }
  }
  mPaintEffectWidget->setPaintEffect( mPaintEffect.get() );
}

void QgsDiagramProperties::syncToSettings( const QgsDiagramLayerSettings *dls )
{
  if ( dls )
  {
    mDiagramDistanceSpinBox->setValue( dls->distance() );
    mPrioritySlider->setValue( dls->priority() );
    mZIndexSpinBox->setValue( dls->zIndex() );

    switch ( dls->placement() )
    {
      case QgsDiagramLayerSettings::AroundPoint:
        radAroundPoint->setChecked( true );
        radAroundCentroid->setChecked( true );
        break;

      case QgsDiagramLayerSettings::OverPoint:
        radOverPoint->setChecked( true );
        radOverCentroid->setChecked( true );
        break;

      case QgsDiagramLayerSettings::Line:
        radAroundLine->setChecked( true );
        radPolygonPerimeter->setChecked( true );
        break;

      case QgsDiagramLayerSettings::Horizontal:
        radOverLine->setChecked( true );
        radInsidePolygon->setChecked( true );
        break;

      default:
        break;
    }

    chkLineAbove->setChecked( dls->linePlacementFlags() & QgsDiagramLayerSettings::AboveLine );
    chkLineBelow->setChecked( dls->linePlacementFlags() & QgsDiagramLayerSettings::BelowLine );
    chkLineOn->setChecked( dls->linePlacementFlags() & QgsDiagramLayerSettings::OnLine );
    if ( !( dls->linePlacementFlags() & QgsDiagramLayerSettings::MapOrientation ) )
      chkLineOrientationDependent->setChecked( true );
    updatePlacementWidgets();

    mShowAllCheckBox->setChecked( dls->showAllDiagrams() );

    mDataDefinedProperties = dls->dataDefinedProperties();
  }
}

QgsDiagramProperties::~QgsDiagramProperties()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/Diagrams/OptionsSplitState" ), mDiagramOptionsSplitter->saveState() );
  settings.setValue( QStringLiteral( "Windows/Diagrams/Tab" ), mDiagramOptionsListWidget->currentRow() );
}

void QgsDiagramProperties::registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsDiagramLayerSettings::Property key )
{
  button->init( static_cast<int>( key ), mDataDefinedProperties, QgsDiagramLayerSettings::propertyDefinitions(), mLayer, true );
  connect( button, &QgsPropertyOverrideButton::changed, this, &QgsDiagramProperties::updateProperty );
  connect( button, &QgsPropertyOverrideButton::createAuxiliaryField, this, &QgsDiagramProperties::createAuxiliaryField );
  button->registerExpressionContextGenerator( this );
}

void QgsDiagramProperties::updateProperty()
{
  QgsPropertyOverrideButton *button = qobject_cast<QgsPropertyOverrideButton *>( sender() );
  const QgsDiagramLayerSettings::Property key = static_cast<QgsDiagramLayerSettings::Property>( button->propertyKey() );
  mDataDefinedProperties.setProperty( key, button->toProperty() );
  emit widgetChanged();
}

void QgsDiagramProperties::mDiagramTypeComboBox_currentIndexChanged( int index )
{
  mDiagramType = mDiagramTypeComboBox->itemData( index ).toString();

  if ( QgsTextDiagram::DIAGRAM_NAME_TEXT == mDiagramType )
  {
    mTextOptionsFrame->show();
    mBackgroundColorLabel->show();
    mBackgroundColorButton->show();
    mBackgroundColorDDBtn->show();
    mDiagramFontButton->show();
  }
  else
  {
    mTextOptionsFrame->hide();
    mBackgroundColorLabel->hide();
    mBackgroundColorButton->hide();
    mBackgroundColorDDBtn->hide();
    mDiagramFontButton->hide();
  }

  if ( QgsHistogramDiagram::DIAGRAM_NAME_HISTOGRAM == mDiagramType || QgsStackedBarDiagram::DIAGRAM_NAME_STACKED_BAR == mDiagramType )
  {
    mBarWidthLabel->show();
    mBarWidthSpinBox->show();
    mBarSpacingLabel->show();
    mBarSpacingSpinBox->show();
    mBarSpacingUnitComboBox->show();
    mBarOptionsFrame->show();
    mShowAxisGroupBox->show();
    if ( QgsHistogramDiagram::DIAGRAM_NAME_HISTOGRAM == mDiagramType )
      mAttributeBasedScalingRadio->setChecked( true );
    mFixedSizeRadio->setEnabled( QgsStackedBarDiagram::DIAGRAM_NAME_STACKED_BAR == mDiagramType );
    mDiagramSizeSpinBox->setEnabled( QgsStackedBarDiagram::DIAGRAM_NAME_STACKED_BAR == mDiagramType );
    mLinearlyScalingLabel->setText( tr( "Bar length: Scale linearly, so that the following value matches the specified bar length:" ) );
    mSizeLabel->setText( tr( "Bar length" ) );
    mFrameIncreaseSize->setVisible( false );
  }
  else
  {
    mBarWidthLabel->hide();
    mBarWidthSpinBox->hide();
    mBarSpacingLabel->hide();
    mBarSpacingSpinBox->hide();
    mBarSpacingUnitComboBox->hide();
    mShowAxisGroupBox->hide();
    mBarOptionsFrame->hide();
    mLinearlyScalingLabel->setText( tr( "Scale linearly between 0 and the following attribute value / diagram size:" ) );
    mSizeLabel->setText( tr( "Size" ) );
    mAttributeBasedScalingRadio->setEnabled( true );
    mFixedSizeRadio->setEnabled( true );
    mDiagramSizeSpinBox->setEnabled( mFixedSizeRadio->isChecked() );
    mFrameIncreaseSize->setVisible( true );
  }

  if ( QgsTextDiagram::DIAGRAM_NAME_TEXT == mDiagramType || QgsPieDiagram::DIAGRAM_NAME_PIE == mDiagramType )
  {
    mScaleDependencyComboBox->show();
    mScaleDependencyLabel->show();
  }
  else
  {
    mScaleDependencyComboBox->hide();
    mScaleDependencyLabel->hide();
  }

  if ( QgsPieDiagram::DIAGRAM_NAME_PIE == mDiagramType )
  {
    mAngleOffsetComboBox->show();
    mAngleDirectionComboBox->show();
    mAngleDirectionLabel->show();
    mAngleOffsetLabel->show();
    mStartAngleDDBtn->show();
  }
  else
  {
    mAngleOffsetComboBox->hide();
    mAngleDirectionComboBox->hide();
    mAngleDirectionLabel->hide();
    mAngleOffsetLabel->hide();
    mStartAngleDDBtn->hide();
  }
}

QString QgsDiagramProperties::guessLegendText( const QString &expression )
{
  //trim unwanted characters from expression text for legend
  QString text = expression.mid( expression.startsWith( '\"' ) ? 1 : 0 );
  if ( text.endsWith( '\"' ) )
    text.chop( 1 );
  return text;
}

void QgsDiagramProperties::addAttribute( QTreeWidgetItem *item )
{
  QTreeWidgetItem *newItem = new QTreeWidgetItem( mDiagramAttributesTreeWidget );

  newItem->setText( 0, item->text( 0 ) );
  newItem->setText( 2, guessLegendText( item->text( 0 ) ) );
  newItem->setData( 0, RoleAttributeExpression, item->data( 0, RoleAttributeExpression ) );
  newItem->setFlags( ( newItem->flags() | Qt::ItemIsEditable ) & ~Qt::ItemIsDropEnabled );

  //set initial color for diagram category
  const int red = QRandomGenerator::global()->bounded( 1, 256 );
  const int green = QRandomGenerator::global()->bounded( 1, 256 );
  const int blue = QRandomGenerator::global()->bounded( 1, 256 );
  const QColor randomColor( red, green, blue );
  newItem->setData( ColumnColor, Qt::EditRole, randomColor );
  mDiagramAttributesTreeWidget->addTopLevelItem( newItem );
}

void QgsDiagramProperties::mAddCategoryPushButton_clicked()
{
  const auto constSelectedItems = mAttributesTreeWidget->selectedItems();
  for ( QTreeWidgetItem *attributeItem : constSelectedItems )
  {
    addAttribute( attributeItem );
  }
}

void QgsDiagramProperties::mAttributesTreeWidget_itemDoubleClicked( QTreeWidgetItem *item, int column )
{
  Q_UNUSED( column )
  addAttribute( item );
}

void QgsDiagramProperties::mRemoveCategoryPushButton_clicked()
{
  const auto constSelectedItems = mDiagramAttributesTreeWidget->selectedItems();
  for ( QTreeWidgetItem *attributeItem : constSelectedItems )
  {
    delete attributeItem;
  }
}

void QgsDiagramProperties::mFindMaximumValueButton_clicked()
{
  if ( !mLayer )
    return;

  float maxValue = 0.0;

  bool isExpression;
  const QString sizeFieldNameOrExp = mSizeFieldExpressionWidget->currentField( &isExpression );
  if ( isExpression )
  {
    QgsExpression exp( sizeFieldNameOrExp );
    QgsExpressionContext context;
    context << QgsExpressionContextUtils::globalScope()
            << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
            << QgsExpressionContextUtils::mapSettingsScope( mMapCanvas->mapSettings() )
            << QgsExpressionContextUtils::layerScope( mLayer );

    exp.prepare( &context );
    if ( !exp.hasEvalError() )
    {
      QgsFeature feature;
      QgsFeatureIterator features = mLayer->getFeatures();
      while ( features.nextFeature( *&feature ) )
      {
        context.setFeature( feature );
        maxValue = std::max( maxValue, exp.evaluate( &context ).toFloat() );
      }
    }
    else
    {
      QgsDebugMsgLevel( "Prepare error:" + exp.evalErrorString(), 4 );
    }
  }
  else
  {
    const int attributeNumber = mLayer->fields().lookupField( sizeFieldNameOrExp );
    maxValue = mLayer->maximumValue( attributeNumber ).toFloat();
  }

  mMaxValueSpinBox->setValue( maxValue );
}

void QgsDiagramProperties::mDiagramAttributesTreeWidget_itemDoubleClicked( QTreeWidgetItem *item, int column )
{
  switch ( column )
  {
    case ColumnAttributeExpression:
    {
      const QString currentExpression = item->data( 0, RoleAttributeExpression ).toString();

      const QString newExpression = showExpressionBuilder( currentExpression );
      if ( !newExpression.isEmpty() )
      {
        item->setData( 0, Qt::DisplayRole, newExpression );
        item->setData( 0, RoleAttributeExpression, newExpression );
      }
      break;
    }

    case ColumnColor:
      break;

    case ColumnLegendText:
      break;
  }
}

std::unique_ptr<QgsDiagram> QgsDiagramProperties::createDiagramObject()
{
  std::unique_ptr<QgsDiagram> diagram;

  if ( mDiagramType == QgsTextDiagram::DIAGRAM_NAME_TEXT )
  {
    diagram = std::make_unique<QgsTextDiagram>();
  }
  else if ( mDiagramType == QgsPieDiagram::DIAGRAM_NAME_PIE )
  {
    diagram = std::make_unique<QgsPieDiagram>();
  }
  else if ( mDiagramType == QgsStackedBarDiagram::DIAGRAM_NAME_STACKED_BAR )
  {
    diagram = std::make_unique<QgsStackedBarDiagram>();
  }
  else // if ( diagramType == QgsHistogramDiagram::DIAGRAM_NAME_HISTOGRAM )
  {
    diagram = std::make_unique<QgsHistogramDiagram>();
  }
  return diagram;
}

std::unique_ptr<QgsDiagramSettings> QgsDiagramProperties::createDiagramSettings()
{
  std::unique_ptr<QgsDiagramSettings> ds = std::make_unique<QgsDiagramSettings>();
  ds->enabled = isDiagramEnabled();
  ds->font = mDiagramFontButton->currentFont();
  ds->opacity = mOpacityWidget->opacity();

  QList<QColor> categoryColors;
  QList<QString> categoryAttributes;
  QList<QString> categoryLabels;
  categoryColors.reserve( mDiagramAttributesTreeWidget->topLevelItemCount() );
  categoryAttributes.reserve( mDiagramAttributesTreeWidget->topLevelItemCount() );
  categoryLabels.reserve( mDiagramAttributesTreeWidget->topLevelItemCount() );
  for ( int i = 0; i < mDiagramAttributesTreeWidget->topLevelItemCount(); ++i )
  {
    QColor color = mDiagramAttributesTreeWidget->topLevelItem( i )->data( ColumnColor, Qt::EditRole ).value<QColor>();
    categoryColors.append( color );
    categoryAttributes.append( mDiagramAttributesTreeWidget->topLevelItem( i )->data( 0, RoleAttributeExpression ).toString() );
    categoryLabels.append( mDiagramAttributesTreeWidget->topLevelItem( i )->text( 2 ) );
  }
  ds->categoryColors = categoryColors;
  ds->categoryAttributes = categoryAttributes;
  ds->categoryLabels = categoryLabels;
  ds->size = QSizeF( mDiagramSizeSpinBox->value(), mDiagramSizeSpinBox->value() );
  ds->sizeType = mDiagramUnitComboBox->unit();
  ds->sizeScale = mDiagramUnitComboBox->getMapUnitScale();
  ds->lineSizeUnit = mDiagramLineUnitComboBox->unit();
  ds->lineSizeScale = mDiagramLineUnitComboBox->getMapUnitScale();
  ds->labelPlacementMethod = static_cast<QgsDiagramSettings::LabelPlacementMethod>( mLabelPlacementComboBox->currentData().toInt() );
  ds->scaleByArea = ( mDiagramType == QgsStackedBarDiagram::DIAGRAM_NAME_STACKED_BAR ) ? false : mScaleDependencyComboBox->currentData().toBool();

  if ( mIncreaseSmallDiagramsGroupBox->isChecked() )
  {
    ds->minimumSize = mIncreaseMinimumSizeSpinBox->value();
  }
  else
  {
    ds->minimumSize = 0;
  }

  ds->backgroundColor = mBackgroundColorButton->color();
  ds->penColor = mDiagramPenColorButton->color();
  ds->penWidth = mPenWidthSpinBox->value();
  ds->minimumScale = mScaleRangeWidget->minimumScale();
  ds->maximumScale = mScaleRangeWidget->maximumScale();
  ds->scaleBasedVisibility = mScaleVisibilityGroupBox->isChecked();

  // Diagram angle offset (pie)
  ds->rotationOffset = mAngleOffsetComboBox->currentData().toInt();
  ds->setDirection( static_cast<QgsDiagramSettings::Direction>( mAngleDirectionComboBox->currentData().toInt() ) );

  // Diagram orientation (histogram)
  ds->diagramOrientation = static_cast<QgsDiagramSettings::DiagramOrientation>( mOrientationButtonGroup->checkedButton()->property( "direction" ).toInt() );

  ds->barWidth = mBarWidthSpinBox->value();

  ds->setAxisLineSymbol( mAxisLineStyleButton->clonedSymbol<QgsLineSymbol>() );
  ds->setShowAxis( mShowAxisGroupBox->isChecked() );

  ds->setSpacing( mBarSpacingSpinBox->value() );
  ds->setSpacingUnit( mBarSpacingUnitComboBox->unit() );
  ds->setSpacingMapUnitScale( mBarSpacingUnitComboBox->getMapUnitScale() );

  if ( mPaintEffect && ( !QgsPaintEffectRegistry::isDefaultStack( mPaintEffect.get() ) || mPaintEffect->enabled() ) )
    ds->setPaintEffect( mPaintEffect->clone() );
  else
    ds->setPaintEffect( nullptr );

  return ds;
}

std::unique_ptr<QgsDiagramRenderer> QgsDiagramProperties::createRenderer()
{
  std::unique_ptr<QgsDiagramSettings> ds = createDiagramSettings();

  std::unique_ptr<QgsDiagramRenderer> renderer;
  if ( mFixedSizeRadio->isChecked() )
  {
    std::unique_ptr<QgsSingleCategoryDiagramRenderer> dr = std::make_unique<QgsSingleCategoryDiagramRenderer>();
    dr->setDiagramSettings( *ds );
    renderer = std::move( dr );
  }
  else
  {
    std::unique_ptr<QgsLinearlyInterpolatedDiagramRenderer> dr = std::make_unique<QgsLinearlyInterpolatedDiagramRenderer>();
    dr->setLowerValue( 0.0 );
    dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
    dr->setUpperValue( mMaxValueSpinBox->value() );
    dr->setUpperSize( QSizeF( mSizeSpinBox->value(), mSizeSpinBox->value() ) );

    bool isExpression;
    const QString sizeFieldNameOrExp = mSizeFieldExpressionWidget->currentField( &isExpression );
    dr->setClassificationAttributeIsExpression( isExpression );
    if ( isExpression )
    {
      dr->setClassificationAttributeExpression( sizeFieldNameOrExp );
    }
    else
    {
      dr->setClassificationField( sizeFieldNameOrExp );
    }
    dr->setDiagramSettings( *ds );

    dr->setDataDefinedSizeLegend( mSizeLegend ? new QgsDataDefinedSizeLegend( *mSizeLegend ) : nullptr );

    renderer = std::move( dr );
  }

  renderer->setAttributeLegend( mCheckBoxAttributeLegend->isChecked() );

  std::unique_ptr<QgsDiagram> diagram = createDiagramObject();
  renderer->setDiagram( diagram.release() );

  return renderer;
}

QgsDiagramLayerSettings QgsDiagramProperties::createDiagramLayerSettings()
{
  QgsDiagramLayerSettings dls;
  dls.setDataDefinedProperties( mDataDefinedProperties );
  dls.setDistance( mDiagramDistanceSpinBox->value() );
  dls.setPriority( mPrioritySlider->value() );
  dls.setZIndex( mZIndexSpinBox->value() );
  dls.setShowAllDiagrams( mShowAllCheckBox->isChecked() );

  QWidget *curWdgt = stackedPlacement->currentWidget();
  if ( ( curWdgt == pagePoint && radAroundPoint->isChecked() )
       || ( curWdgt == pagePolygon && radAroundCentroid->isChecked() ) )
  {
    dls.setPlacement( QgsDiagramLayerSettings::AroundPoint );
  }
  else if ( ( curWdgt == pagePoint && radOverPoint->isChecked() )
            || ( curWdgt == pagePolygon && radOverCentroid->isChecked() ) )
  {
    dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
  }
  else if ( ( curWdgt == pageLine && radAroundLine->isChecked() )
            || ( curWdgt == pagePolygon && radPolygonPerimeter->isChecked() ) )
  {
    dls.setPlacement( QgsDiagramLayerSettings::Line );
  }
  else if ( ( curWdgt == pageLine && radOverLine->isChecked() )
            || ( curWdgt == pagePolygon && radInsidePolygon->isChecked() ) )
  {
    dls.setPlacement( QgsDiagramLayerSettings::Horizontal );
  }
  else
  {
    qFatal( "Invalid settings" );
  }

  QgsDiagramLayerSettings::LinePlacementFlags flags = QgsDiagramLayerSettings::LinePlacementFlags();
  if ( chkLineAbove->isChecked() )
    flags |= QgsDiagramLayerSettings::AboveLine;
  if ( chkLineBelow->isChecked() )
    flags |= QgsDiagramLayerSettings::BelowLine;
  if ( chkLineOn->isChecked() )
    flags |= QgsDiagramLayerSettings::OnLine;
  if ( !chkLineOrientationDependent->isChecked() )
    flags |= QgsDiagramLayerSettings::MapOrientation;
  dls.setLinePlacementFlags( flags );

  return dls;
}

void QgsDiagramProperties::apply()
{
  // Avoid this messageBox when in both dock and liveUpdate mode
  QgsSettings settings;
  if ( !dockMode() || !settings.value( QStringLiteral( "UI/autoApplyStyling" ), true ).toBool() )
  {
    if ( isDiagramEnabled() && 0 == mDiagramAttributesTreeWidget->topLevelItemCount() )
    {
      QMessageBox::warning( this, tr( "Diagrams: No attributes added." ), tr( "You did not add any attributes to this diagram layer. Please specify the attributes to visualize on the diagrams or disable diagrams." ) );
    }
  }

  std::unique_ptr<QgsDiagramRenderer> renderer = createRenderer();
  mLayer->setDiagramRenderer( renderer.release() );

  QgsDiagramLayerSettings dls = createDiagramLayerSettings();
  mLayer->setDiagramLayerSettings( dls );

  // refresh
  QgsProject::instance()->setDirty( true );
  mLayer->triggerRepaint();
}

QString QgsDiagramProperties::showExpressionBuilder( const QString &initialExpression )
{
  QgsExpressionContext context = createExpressionContext();

  QgsExpressionBuilderDialog dlg( mLayer, initialExpression, this, QStringLiteral( "generic" ), context );
  dlg.setWindowTitle( tr( "Expression Based Attribute" ) );

  QgsDistanceArea myDa;
  myDa.setSourceCrs( mLayer->crs(), QgsProject::instance()->transformContext() );
  myDa.setEllipsoid( QgsProject::instance()->ellipsoid() );
  dlg.setGeomCalculator( myDa );

  if ( dlg.exec() == QDialog::Accepted )
  {
    return dlg.expressionText();
  }
  else
  {
    return QString();
  }
}

void QgsDiagramProperties::showAddAttributeExpressionDialog()
{
  QString expression;
  QList<QTreeWidgetItem *> selections = mAttributesTreeWidget->selectedItems();
  if ( !selections.empty() )
  {
    expression = selections[0]->text( 0 );
  }

  const QString newExpression = showExpressionBuilder( expression );

  //Only add the expression if the user has entered some text.
  if ( !newExpression.isEmpty() )
  {
    QTreeWidgetItem *newItem = new QTreeWidgetItem( mDiagramAttributesTreeWidget );

    newItem->setText( 0, newExpression );
    newItem->setText( 2, newExpression );
    newItem->setData( 0, RoleAttributeExpression, newExpression );
    newItem->setFlags( ( newItem->flags() | Qt::ItemIsEditable ) & ~Qt::ItemIsDropEnabled );

    //set initial color for diagram category
    QRandomGenerator colorGenerator;
    const int red = colorGenerator.bounded( 1, 256 );
    const int green = colorGenerator.bounded( 1, 256 );
    const int blue = colorGenerator.bounded( 1, 256 );

    const QColor randomColor( red, green, blue );
    newItem->setData( ColumnColor, Qt::EditRole, randomColor );
    mDiagramAttributesTreeWidget->addTopLevelItem( newItem );
  }
  activateWindow(); // set focus back parent
}

void QgsDiagramProperties::mDiagramStackedWidget_currentChanged( int index )
{
  mDiagramOptionsListWidget->blockSignals( true );
  mDiagramOptionsListWidget->setCurrentRow( index );
  mDiagramOptionsListWidget->blockSignals( false );
}

void QgsDiagramProperties::updatePlacementWidgets()
{
  QWidget *curWdgt = stackedPlacement->currentWidget();

  if ( ( curWdgt == pagePoint && radAroundPoint->isChecked() )
       || ( curWdgt == pageLine && radAroundLine->isChecked() )
       || ( curWdgt == pagePolygon && radAroundCentroid->isChecked() ) )
  {
    mDiagramDistanceLabel->setEnabled( true );
    mDiagramDistanceSpinBox->setEnabled( true );
    mDistanceDDBtn->setEnabled( true );
  }
  else
  {
    mDiagramDistanceLabel->setEnabled( false );
    mDiagramDistanceSpinBox->setEnabled( false );
    mDistanceDDBtn->setEnabled( false );
  }

  const bool linePlacementEnabled = mLayer->geometryType() == Qgis::GeometryType::Line && ( curWdgt == pageLine && radAroundLine->isChecked() );
  chkLineAbove->setEnabled( linePlacementEnabled );
  chkLineBelow->setEnabled( linePlacementEnabled );
  chkLineOn->setEnabled( linePlacementEnabled );
  chkLineOrientationDependent->setEnabled( linePlacementEnabled );
}

void QgsDiagramProperties::scalingTypeChanged()
{
  mButtonSizeLegendSettings->setEnabled( mAttributeBasedScalingRadio->isChecked() );
}

void QgsDiagramProperties::setAllowedToEditDiagramLayerSettings( bool allowed )
{
  mAllowedToEditDls = allowed;

  label_16->setVisible( allowed );
  mZIndexSpinBox->setVisible( allowed );
  mZOrderDDBtn->setVisible( allowed );
  mShowAllCheckBox->setVisible( allowed );
  mDlsLabel_1->setVisible( !allowed );

  mCoordinatesGrpBox->setVisible( allowed );
  mLinePlacementFrame->setVisible( allowed );
  mObstaclesGrpBox->setVisible( allowed );
  mPlacementFrame->setVisible( allowed );
  mPriorityGrpBox->setVisible( allowed );
  stackedPlacement->setVisible( allowed );
  mDlsLabel_2->setVisible( !allowed );
}

bool QgsDiagramProperties::isAllowedToEditDiagramLayerSettings() const
{
  return mAllowedToEditDls;
}

void QgsDiagramProperties::showSizeLegendDialog()
{
  // prepare size transformer
  bool isExpression;
  const QString sizeFieldNameOrExp = mSizeFieldExpressionWidget->currentField( &isExpression );
  QgsProperty ddSize = isExpression ? QgsProperty::fromExpression( sizeFieldNameOrExp ) : QgsProperty::fromField( sizeFieldNameOrExp );
  const bool scaleByArea = mScaleDependencyComboBox->currentData().toBool();
  ddSize.setTransformer( new QgsSizeScaleTransformer( scaleByArea ? QgsSizeScaleTransformer::Area : QgsSizeScaleTransformer::Linear, 0.0, mMaxValueSpinBox->value(), 0.0, mSizeSpinBox->value() ) );

  QgsDataDefinedSizeLegendWidget *panel = new QgsDataDefinedSizeLegendWidget( mSizeLegend.get(), ddSize, nullptr, mMapCanvas );

  QDialog dlg;
  dlg.setLayout( new QVBoxLayout() );
  dlg.setWindowTitle( panel->panelTitle() );
  dlg.layout()->addWidget( panel );
  QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Help | QDialogButtonBox::Ok );
  connect( buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsDiagramProperties::showHelp );
  connect( buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject );
  dlg.layout()->addWidget( buttonBox );
  if ( dlg.exec() )
    mSizeLegend.reset( panel->dataDefinedSizeLegend() );
}

void QgsDiagramProperties::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_vector/vector_properties.html#legend" ) );
}

void QgsDiagramProperties::createAuxiliaryField()
{
  // try to create an auxiliary layer if not yet created
  if ( !mLayer->auxiliaryLayer() )
  {
    QgsNewAuxiliaryLayerDialog dlg( mLayer, this );
    dlg.exec();
  }

  // return if still not exists
  if ( !mLayer->auxiliaryLayer() )
    return;

  QgsPropertyOverrideButton *button = qobject_cast<QgsPropertyOverrideButton *>( sender() );
  const QgsDiagramLayerSettings::Property key = static_cast<QgsDiagramLayerSettings::Property>( button->propertyKey() );
  const QgsPropertyDefinition def = QgsDiagramLayerSettings::propertyDefinitions()[static_cast<int>( key )];

  // create property in auxiliary storage if necessary
  if ( !mLayer->auxiliaryLayer()->exists( def ) )
    mLayer->auxiliaryLayer()->addAuxiliaryField( def );

  // update property with join field name from auxiliary storage
  QgsProperty property = button->toProperty();
  property.setField( QgsAuxiliaryLayer::nameFromProperty( def, true ) );
  property.setActive( true );
  button->updateFieldLists();
  button->setToProperty( property );
  mDataDefinedProperties.setProperty( key, button->toProperty() );

  emit auxiliaryFieldCreated();
}

void QgsDiagramProperties::connectValueChanged( const QList<QWidget *> &widgets )
{
  const auto constWidgets = widgets;
  for ( QWidget *widget : constWidgets )
  {
    if ( QgsSymbolButton *w = qobject_cast<QgsSymbolButton *>( widget ) )
    {
      connect( w, &QgsSymbolButton::changed, this, &QgsDiagramProperties::widgetChanged );
    }
    else if ( QgsFieldExpressionWidget *w = qobject_cast<QgsFieldExpressionWidget *>( widget ) )
    {
      connect( w, qOverload<const QString &>( &QgsFieldExpressionWidget::fieldChanged ), this, &QgsDiagramProperties::widgetChanged );
    }
    else if ( QgsOpacityWidget *w = qobject_cast<QgsOpacityWidget *>( widget ) )
    {
      connect( w, &QgsOpacityWidget::opacityChanged, this, &QgsDiagramProperties::widgetChanged );
    }
    else if ( QgsUnitSelectionWidget *w = qobject_cast<QgsUnitSelectionWidget *>( widget ) )
    {
      connect( w, &QgsUnitSelectionWidget::changed, this, &QgsDiagramProperties::widgetChanged );
    }
    else if ( QComboBox *w = qobject_cast<QComboBox *>( widget ) )
    {
      connect( w, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsDiagramProperties::widgetChanged );
    }
    else if ( QSpinBox *w = qobject_cast<QSpinBox *>( widget ) )
    {
      connect( w, qOverload<int>( &QSpinBox::valueChanged ), this, &QgsDiagramProperties::widgetChanged );
    }
    else if ( QDoubleSpinBox *w = qobject_cast<QDoubleSpinBox *>( widget ) )
    {
      connect( w, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsDiagramProperties::widgetChanged );
    }
    else if ( QgsColorButton *w = qobject_cast<QgsColorButton *>( widget ) )
    {
      connect( w, &QgsColorButton::colorChanged, this, &QgsDiagramProperties::widgetChanged );
    }
    else if ( QCheckBox *w = qobject_cast<QCheckBox *>( widget ) )
    {
      connect( w, &QCheckBox::toggled, this, &QgsDiagramProperties::widgetChanged );
    }
    else if ( QRadioButton *w = qobject_cast<QRadioButton *>( widget ) )
    {
      connect( w, &QRadioButton::toggled, this, &QgsDiagramProperties::widgetChanged );
    }
    else if ( QSlider *w = qobject_cast<QSlider *>( widget ) )
    {
      connect( w, &QSlider::valueChanged, this, &QgsDiagramProperties::widgetChanged );
    }
    else if ( QGroupBox *w = qobject_cast<QGroupBox *>( widget ) )
    {
      connect( w, &QGroupBox::toggled, this, &QgsDiagramProperties::widgetChanged );
    }
    else if ( QTreeWidget *w = qobject_cast<QTreeWidget *>( widget ) )
    {
      connect( w, &QTreeWidget::itemChanged, this, &QgsDiagramProperties::widgetChanged );
    }
    else if ( QgsScaleRangeWidget *w = qobject_cast<QgsScaleRangeWidget *>( widget ) )
    {
      connect( w, &QgsScaleRangeWidget::rangeChanged, this, &QgsDiagramProperties::widgetChanged );
    }
    else if ( QgsEffectStackCompactWidget *w = qobject_cast<QgsEffectStackCompactWidget *>( widget ) )
    {
      connect( w, &QgsEffectStackCompactWidget::changed, this, &QgsDiagramProperties::widgetChanged );
    }
    else if ( QgsFontButton *w = qobject_cast<QgsFontButton *>( widget ) )
    {
      connect( w, &QgsFontButton::changed, this, &QgsDiagramProperties::widgetChanged );
    }
    else
    {
      QgsLogger::warning( QStringLiteral( "Could not create connection for widget %1" ).arg( widget->objectName() ) );
    }
  }
}

void QgsDiagramProperties::setDiagramEnabled( bool enabled )
{
  mEnableDiagramCheckBox->setChecked( enabled );
}

bool QgsDiagramProperties::isDiagramEnabled() const
{
  return mEnableDiagramCheckBox->isChecked();
}
