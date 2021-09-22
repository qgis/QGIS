/***************************************************************************
                             qgslayoutpropertieswidget.cpp
                             -----------------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutpropertieswidget.h"
#include "qgslayout.h"
#include "qgslayoutsnapper.h"
#include "qgslayoutpagecollection.h"
#include "qgslayoutundostack.h"
#include "qgslayoutitemmap.h"
#include "qgsexpressioncontextutils.h"
#include "qgsprintlayout.h"
#include "qgslayoutatlas.h"

QgsLayoutPropertiesWidget::QgsLayoutPropertiesWidget( QWidget *parent, QgsLayout *layout )
  : QgsPanelWidget( parent )
  , mLayout( layout )
{
  Q_ASSERT( mLayout );

  setupUi( this );
  setPanelTitle( tr( "Layout Properties" ) );
  blockSignals( true );

  mVariableEditor->setMinimumHeight( mVariableEditor->fontMetrics().height() * 15 );

  updateSnappingElements();

  mGridSpacingUnitsCombo->linkToWidget( mGridResolutionSpinBox );
  mGridOffsetUnitsComboBox->linkToWidget( mOffsetXSpinBox );
  mGridOffsetUnitsComboBox->linkToWidget( mOffsetYSpinBox );

  blockSignals( false );

  connect( mSnapToleranceSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsLayoutPropertiesWidget::snapToleranceChanged );

  connect( mGridOffsetUnitsComboBox, &QgsLayoutUnitsComboBox::changed, this, &QgsLayoutPropertiesWidget::gridOffsetUnitsChanged );
  connect( mGridSpacingUnitsCombo, &QgsLayoutUnitsComboBox::changed, this, &QgsLayoutPropertiesWidget::gridResolutionUnitsChanged );
  connect( mGridResolutionSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsLayoutPropertiesWidget::gridResolutionChanged );
  connect( mOffsetXSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsLayoutPropertiesWidget::gridOffsetXChanged );
  connect( mOffsetYSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsLayoutPropertiesWidget::gridOffsetYChanged );

  const double leftMargin = mLayout->customProperty( QStringLiteral( "resizeToContentsLeftMargin" ) ).toDouble();
  const double topMargin = mLayout->customProperty( QStringLiteral( "resizeToContentsTopMargin" ) ).toDouble();
  const double bottomMargin = mLayout->customProperty( QStringLiteral( "resizeToContentsBottomMargin" ) ).toDouble();
  const double rightMargin = mLayout->customProperty( QStringLiteral( "resizeToContentsRightMargin" ) ).toDouble();
  const QgsUnitTypes::LayoutUnit marginUnit = static_cast< QgsUnitTypes::LayoutUnit >(
        mLayout->customProperty( QStringLiteral( "imageCropMarginUnit" ), QgsUnitTypes::LayoutMillimeters ).toInt() );

  const bool exportWorldFile = mLayout->customProperty( QStringLiteral( "exportWorldFile" ), false ).toBool();
  mGenerateWorldFileCheckBox->setChecked( exportWorldFile );
  connect( mGenerateWorldFileCheckBox, &QCheckBox::toggled, this, &QgsLayoutPropertiesWidget::worldFileToggled );

  connect( mRasterizeCheckBox, &QCheckBox::toggled, this, &QgsLayoutPropertiesWidget::rasterizeToggled );
  connect( mForceVectorCheckBox, &QCheckBox::toggled, this, &QgsLayoutPropertiesWidget::forceVectorToggled );

  mTopMarginSpinBox->setValue( topMargin );
  mMarginUnitsComboBox->linkToWidget( mTopMarginSpinBox );
  mRightMarginSpinBox->setValue( rightMargin );
  mMarginUnitsComboBox->linkToWidget( mRightMarginSpinBox );
  mBottomMarginSpinBox->setValue( bottomMargin );
  mMarginUnitsComboBox->linkToWidget( mBottomMarginSpinBox );
  mLeftMarginSpinBox->setValue( leftMargin );
  mMarginUnitsComboBox->linkToWidget( mLeftMarginSpinBox );
  mMarginUnitsComboBox->setUnit( marginUnit );
  mMarginUnitsComboBox->setConverter( &mLayout->renderContext().measurementConverter() );

  connect( mTopMarginSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutPropertiesWidget::resizeMarginsChanged );
  connect( mRightMarginSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutPropertiesWidget::resizeMarginsChanged );
  connect( mBottomMarginSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutPropertiesWidget::resizeMarginsChanged );
  connect( mLeftMarginSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutPropertiesWidget::resizeMarginsChanged );
  connect( mResizePageButton, &QPushButton::clicked, this, &QgsLayoutPropertiesWidget::resizeToContents );

  connect( mResolutionSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsLayoutPropertiesWidget::dpiChanged );

  mReferenceMapComboBox->setCurrentLayout( mLayout );
  mReferenceMapComboBox->setItemType( QgsLayoutItemRegistry::LayoutMap );
  connect( mReferenceMapComboBox, &QgsLayoutItemComboBox::itemChanged, this, &QgsLayoutPropertiesWidget::referenceMapChanged );

  connect( mLayout, &QgsLayout::changed, this, &QgsLayoutPropertiesWidget::updateGui );

  updateVariables();
  connect( mVariableEditor, &QgsVariableEditorWidget::scopeChanged, this, &QgsLayoutPropertiesWidget::variablesChanged );
  // listen out for variable edits
  connect( QgsApplication::instance(), &QgsApplication::customVariablesChanged, this, &QgsLayoutPropertiesWidget::updateVariables );
  connect( QgsProject::instance(), &QgsProject::customVariablesChanged, this, &QgsLayoutPropertiesWidget::updateVariables );
  connect( QgsProject::instance(), &QgsProject::metadataChanged, this, &QgsLayoutPropertiesWidget::updateVariables );
  connect( &mLayout->renderContext(), &QgsLayoutRenderContext::dpiChanged, this, &QgsLayoutPropertiesWidget::updateVariables );
  connect( mLayout->pageCollection(), &QgsLayoutPageCollection::changed, this, &QgsLayoutPropertiesWidget::updateVariables );
  connect( mLayout, &QgsLayout::variablesChanged, this, &QgsLayoutPropertiesWidget::updateVariables );

  updateGui();
}

void QgsLayoutPropertiesWidget::setMasterLayout( QgsMasterLayoutInterface *masterLayout )
{
  if ( QgsPrintLayout *printLayout = dynamic_cast< QgsPrintLayout * >( masterLayout ) )
  {
    connect( printLayout, &QgsPrintLayout::nameChanged, this, &QgsLayoutPropertiesWidget::updateVariables );
    connect( printLayout->atlas(), &QgsLayoutAtlas::coverageLayerChanged, this, &QgsLayoutPropertiesWidget::updateVariables );
  }
}

void QgsLayoutPropertiesWidget::updateGui()
{
  whileBlocking( mReferenceMapComboBox )->setItem( mLayout->referenceMap() );
  whileBlocking( mResolutionSpinBox )->setValue( mLayout->renderContext().dpi() );

  const bool rasterize = mLayout->customProperty( QStringLiteral( "rasterize" ), false ).toBool();
  whileBlocking( mRasterizeCheckBox )->setChecked( rasterize );

  const bool forceVectors = mLayout->customProperty( QStringLiteral( "forceVector" ), false ).toBool();
  whileBlocking( mForceVectorCheckBox )->setChecked( forceVectors );

  if ( rasterize )
  {
    mForceVectorCheckBox->setChecked( false );
    mForceVectorCheckBox->setEnabled( false );
  }
  else
  {
    mForceVectorCheckBox->setEnabled( true );
  }
}

void QgsLayoutPropertiesWidget::updateSnappingElements()
{
  mSnapToleranceSpinBox->setValue( mLayout->snapper().snapTolerance() );

  mGridSpacingUnitsCombo->setUnit( mLayout->gridSettings().resolution().units() );
  mGridResolutionSpinBox->setValue( mLayout->gridSettings().resolution().length() );

  mGridOffsetUnitsComboBox->setUnit( mLayout->gridSettings().offset().units() );
  mOffsetXSpinBox->setValue( mLayout->gridSettings().offset().x() );
  mOffsetYSpinBox->setValue( mLayout->gridSettings().offset().y() );
}

void QgsLayoutPropertiesWidget::gridResolutionChanged( double d )
{
  QgsLayoutMeasurement m = mLayout->gridSettings().resolution();
  m.setLength( d );
  mLayout->gridSettings().setResolution( m );
  mLayout->pageCollection()->redraw();
}

void QgsLayoutPropertiesWidget::gridResolutionUnitsChanged( QgsUnitTypes::LayoutUnit unit )
{
  QgsLayoutMeasurement m = mLayout->gridSettings().resolution();
  m.setUnits( unit );
  mLayout->gridSettings().setResolution( m );
  mLayout->pageCollection()->redraw();
}

void QgsLayoutPropertiesWidget::gridOffsetXChanged( double d )
{
  QgsLayoutPoint o = mLayout->gridSettings().offset();
  o.setX( d );
  mLayout->gridSettings().setOffset( o );
  mLayout->pageCollection()->redraw();
}

void QgsLayoutPropertiesWidget::gridOffsetYChanged( double d )
{
  QgsLayoutPoint o = mLayout->gridSettings().offset();
  o.setY( d );
  mLayout->gridSettings().setOffset( o );
  mLayout->pageCollection()->redraw();
}

void QgsLayoutPropertiesWidget::gridOffsetUnitsChanged( QgsUnitTypes::LayoutUnit unit )
{
  QgsLayoutPoint o = mLayout->gridSettings().offset();
  o.setUnits( unit );
  mLayout->gridSettings().setOffset( o );
  mLayout->pageCollection()->redraw();
}

void QgsLayoutPropertiesWidget::snapToleranceChanged( int tolerance )
{
  mLayout->snapper().setSnapTolerance( tolerance );
}

void QgsLayoutPropertiesWidget::resizeMarginsChanged()
{
  mLayout->setCustomProperty( QStringLiteral( "resizeToContentsLeftMargin" ), mLeftMarginSpinBox->value() );
  mLayout->setCustomProperty( QStringLiteral( "resizeToContentsTopMargin" ), mTopMarginSpinBox->value() );
  mLayout->setCustomProperty( QStringLiteral( "resizeToContentsBottomMargin" ), mBottomMarginSpinBox->value() );
  mLayout->setCustomProperty( QStringLiteral( "resizeToContentsRightMargin" ), mRightMarginSpinBox->value() );
  mLayout->setCustomProperty( QStringLiteral( "imageCropMarginUnit" ), mMarginUnitsComboBox->unit() );
}

void QgsLayoutPropertiesWidget::resizeToContents()
{
  mLayout->undoStack()->beginMacro( tr( "Resize to Contents" ) );

  mLayout->pageCollection()->resizeToContents( QgsMargins( mLeftMarginSpinBox->value(),
      mTopMarginSpinBox->value(),
      mRightMarginSpinBox->value(),
      mBottomMarginSpinBox->value() ),
      mMarginUnitsComboBox->unit() );

  mLayout->undoStack()->endMacro();
}

void QgsLayoutPropertiesWidget::referenceMapChanged( QgsLayoutItem *item )
{
  mLayout->undoStack()->beginCommand( mLayout, tr( "Set Reference Map" ) );
  QgsLayoutItemMap *map = qobject_cast< QgsLayoutItemMap * >( item );
  mLayout->setReferenceMap( map );
  mLayout->undoStack()->endCommand();
}

void QgsLayoutPropertiesWidget::dpiChanged( int value )
{
  mLayout->undoStack()->beginCommand( mLayout, tr( "Set Default DPI" ), QgsLayout::UndoLayoutDpi );
  mLayout->renderContext().setDpi( value );
  mLayout->undoStack()->endCommand();

  mLayout->refresh();
}

void QgsLayoutPropertiesWidget::worldFileToggled()
{
  mLayout->setCustomProperty( QStringLiteral( "exportWorldFile" ), mGenerateWorldFileCheckBox->isChecked() );
}

void QgsLayoutPropertiesWidget::rasterizeToggled()
{
  mLayout->setCustomProperty( QStringLiteral( "rasterize" ), mRasterizeCheckBox->isChecked() );

  if ( mRasterizeCheckBox->isChecked() )
  {
    mForceVectorCheckBox->setChecked( false );
    mForceVectorCheckBox->setEnabled( false );
  }
  else
  {
    mForceVectorCheckBox->setEnabled( true );
  }
}

void QgsLayoutPropertiesWidget::forceVectorToggled()
{
  mLayout->setCustomProperty( QStringLiteral( "forceVector" ), mForceVectorCheckBox->isChecked() );
}

void QgsLayoutPropertiesWidget::variablesChanged()
{
  mBlockVariableUpdates = true;
  QgsExpressionContextUtils::setLayoutVariables( mLayout, mVariableEditor->variablesInActiveScope() );
  mBlockVariableUpdates = false;
}

void QgsLayoutPropertiesWidget::updateVariables()
{
  if ( mBlockVariableUpdates )
    return;

  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
          << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
          << QgsExpressionContextUtils::layoutScope( mLayout );
  mVariableEditor->setContext( &context );
  mVariableEditor->setEditableScopeIndex( 2 );
}

void QgsLayoutPropertiesWidget::blockSignals( bool block )
{
  mGridResolutionSpinBox->blockSignals( block );
  mOffsetXSpinBox->blockSignals( block );
  mOffsetYSpinBox->blockSignals( block );
  mSnapToleranceSpinBox->blockSignals( block );
}

