/***************************************************************************
                             qgsannotationitemwidget_impl.cpp
                             ------------------------
    Date                 : September 2021
    Copyright            : (C) 2021 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsannotationitemwidget_impl.h"
#include "moc_qgsannotationitemwidget_impl.cpp"

#include "qgssymbolselectordialog.h"
#include "qgsstyle.h"
#include "qgsfillsymbol.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"
#include "qgsannotationpolygonitem.h"
#include "qgsannotationlineitem.h"
#include "qgsannotationmarkeritem.h"
#include "qgsannotationpointtextitem.h"
#include "qgsannotationlinetextitem.h"
#include "qgsannotationrectangletextitem.h"
#include "qgsannotationpictureitem.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgstextformatwidget.h"
#include "qgsapplication.h"
#include "qgsrecentstylehandler.h"
#include "qgsexpressionfinder.h"
#include "qgsimagecache.h"
#include "qgssvgcache.h"
#include "qgsrenderedannotationitemdetails.h"
#include "qgsmapcanvas.h"

///@cond PRIVATE

QgsAnnotationPolygonItemWidget::QgsAnnotationPolygonItemWidget( QWidget *parent )
  : QgsAnnotationItemBaseWidget( parent )
{
  setupUi( this );

  mSelector = new QgsSymbolSelectorWidget( mSymbol.get(), QgsStyle::defaultStyle(), nullptr, nullptr );
  mSelector->setDockMode( dockMode() );
  connect( mSelector, &QgsSymbolSelectorWidget::symbolModified, this, [=] {
    if ( !mBlockChangedSignal )
    {
      emit itemChanged();
      QgsApplication::recentStyleHandler()->pushRecentSymbol( QStringLiteral( "polygon_annotation_item" ), qgis::down_cast<QgsFillSymbol *>( mSelector->symbol()->clone() ) );
    }
  } );
  connect( mSelector, &QgsPanelWidget::showPanel, this, &QgsPanelWidget::openPanel );

  QVBoxLayout *layout = new QVBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( mSelector );
  mSymbolSelectorFrame->setLayout( layout );

  connect( mPropertiesWidget, &QgsAnnotationItemCommonPropertiesWidget::itemChanged, this, [=] {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );
}

QgsAnnotationItem *QgsAnnotationPolygonItemWidget::createItem()
{
  QgsAnnotationPolygonItem *newItem = mItem->clone();
  newItem->setSymbol( mSymbol->clone() );
  mPropertiesWidget->updateItem( newItem );
  return newItem;
}

void QgsAnnotationPolygonItemWidget::updateItem( QgsAnnotationItem *item )
{
  if ( QgsAnnotationPolygonItem *polygonItem = dynamic_cast<QgsAnnotationPolygonItem *>( item ) )
  {
    polygonItem->setSymbol( mSymbol->clone() );
    mPropertiesWidget->updateItem( polygonItem );
  }
}

void QgsAnnotationPolygonItemWidget::setDockMode( bool dockMode )
{
  QgsAnnotationItemBaseWidget::setDockMode( dockMode );
  if ( mSelector )
    mSelector->setDockMode( dockMode );
}

void QgsAnnotationPolygonItemWidget::setContext( const QgsSymbolWidgetContext &context )
{
  QgsAnnotationItemBaseWidget::setContext( context );
  if ( mSelector )
    mSelector->setContext( context );
  mPropertiesWidget->setContext( context );
}

QgsAnnotationPolygonItemWidget::~QgsAnnotationPolygonItemWidget() = default;

bool QgsAnnotationPolygonItemWidget::setNewItem( QgsAnnotationItem *item )
{
  QgsAnnotationPolygonItem *polygonItem = dynamic_cast<QgsAnnotationPolygonItem *>( item );
  if ( !polygonItem )
    return false;

  mItem.reset( polygonItem->clone() );
  if ( mItem->symbol() )
  {
    mSymbol.reset( mItem->symbol()->clone() );
  }
  else
  {
    mSymbol.reset( QgsFillSymbol::createSimple( {} ) );
  }
  mBlockChangedSignal = true;
  mSelector->loadSymbol( mSymbol.get() );
  mSelector->updatePreview();
  mPropertiesWidget->setItem( mItem.get() );
  mBlockChangedSignal = false;

  return true;
}


//
// QgsAnnotationLineItemWidget
//

QgsAnnotationLineItemWidget::QgsAnnotationLineItemWidget( QWidget *parent )
  : QgsAnnotationItemBaseWidget( parent )
{
  setupUi( this );

  mSelector = new QgsSymbolSelectorWidget( mSymbol.get(), QgsStyle::defaultStyle(), nullptr, nullptr );
  mSelector->setDockMode( dockMode() );
  connect( mSelector, &QgsSymbolSelectorWidget::symbolModified, this, [=] {
    if ( !mBlockChangedSignal )
    {
      emit itemChanged();
      QgsApplication::recentStyleHandler()->pushRecentSymbol( QStringLiteral( "line_annotation_item" ), qgis::down_cast<QgsLineSymbol *>( mSelector->symbol()->clone() ) );
    }
  } );
  connect( mSelector, &QgsPanelWidget::showPanel, this, &QgsPanelWidget::openPanel );

  QVBoxLayout *layout = new QVBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( mSelector );
  mSymbolSelectorFrame->setLayout( layout );

  connect( mPropertiesWidget, &QgsAnnotationItemCommonPropertiesWidget::itemChanged, this, [=] {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );
}

QgsAnnotationItem *QgsAnnotationLineItemWidget::createItem()
{
  QgsAnnotationLineItem *newItem = mItem->clone();
  newItem->setSymbol( mSymbol->clone() );
  mPropertiesWidget->updateItem( newItem );
  return newItem;
}

void QgsAnnotationLineItemWidget::updateItem( QgsAnnotationItem *item )
{
  if ( QgsAnnotationLineItem *lineItem = dynamic_cast<QgsAnnotationLineItem *>( item ) )
  {
    lineItem->setSymbol( mSymbol->clone() );
    mPropertiesWidget->updateItem( lineItem );
  }
}

void QgsAnnotationLineItemWidget::setDockMode( bool dockMode )
{
  QgsAnnotationItemBaseWidget::setDockMode( dockMode );
  if ( mSelector )
    mSelector->setDockMode( dockMode );
}

void QgsAnnotationLineItemWidget::setContext( const QgsSymbolWidgetContext &context )
{
  QgsAnnotationItemBaseWidget::setContext( context );
  if ( mSelector )
    mSelector->setContext( context );
  mPropertiesWidget->setContext( context );
}

QgsAnnotationLineItemWidget::~QgsAnnotationLineItemWidget() = default;

bool QgsAnnotationLineItemWidget::setNewItem( QgsAnnotationItem *item )
{
  QgsAnnotationLineItem *lineItem = dynamic_cast<QgsAnnotationLineItem *>( item );
  if ( !lineItem )
    return false;

  mItem.reset( lineItem->clone() );
  if ( mItem->symbol() )
  {
    mSymbol.reset( mItem->symbol()->clone() );
  }
  else
  {
    mSymbol.reset( QgsLineSymbol::createSimple( {} ) );
  }
  mBlockChangedSignal = true;
  mSelector->loadSymbol( mSymbol.get() );
  mSelector->updatePreview();
  mPropertiesWidget->setItem( mItem.get() );
  mBlockChangedSignal = false;

  return true;
}


//
// QgsAnnotationMarkerItemWidget
//

QgsAnnotationMarkerItemWidget::QgsAnnotationMarkerItemWidget( QWidget *parent )
  : QgsAnnotationItemBaseWidget( parent )
{
  setupUi( this );

  mSelector = new QgsSymbolSelectorWidget( mSymbol.get(), QgsStyle::defaultStyle(), nullptr, nullptr );
  mSelector->setDockMode( dockMode() );
  connect( mSelector, &QgsSymbolSelectorWidget::symbolModified, this, [=] {
    if ( !mBlockChangedSignal )
    {
      emit itemChanged();
      QgsApplication::recentStyleHandler()->pushRecentSymbol( QStringLiteral( "marker_annotation_item" ), qgis::down_cast<QgsMarkerSymbol *>( mSelector->symbol()->clone() ) );
    }
  } );
  connect( mSelector, &QgsPanelWidget::showPanel, this, &QgsPanelWidget::openPanel );

  QVBoxLayout *layout = new QVBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( mSelector );
  mSymbolSelectorFrame->setLayout( layout );

  connect( mPropertiesWidget, &QgsAnnotationItemCommonPropertiesWidget::itemChanged, this, [=] {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );
}

QgsAnnotationItem *QgsAnnotationMarkerItemWidget::createItem()
{
  QgsAnnotationMarkerItem *newItem = mItem->clone();
  newItem->setSymbol( mSymbol->clone() );
  mPropertiesWidget->updateItem( newItem );
  return newItem;
}

void QgsAnnotationMarkerItemWidget::updateItem( QgsAnnotationItem *item )
{
  if ( QgsAnnotationMarkerItem *markerItem = dynamic_cast<QgsAnnotationMarkerItem *>( item ) )
  {
    markerItem->setSymbol( mSymbol->clone() );
    mPropertiesWidget->updateItem( markerItem );
  }
}

void QgsAnnotationMarkerItemWidget::setDockMode( bool dockMode )
{
  QgsAnnotationItemBaseWidget::setDockMode( dockMode );
  if ( mSelector )
    mSelector->setDockMode( dockMode );
}

void QgsAnnotationMarkerItemWidget::setContext( const QgsSymbolWidgetContext &context )
{
  QgsAnnotationItemBaseWidget::setContext( context );
  if ( mSelector )
    mSelector->setContext( context );
  mPropertiesWidget->setContext( context );
}

QgsAnnotationMarkerItemWidget::~QgsAnnotationMarkerItemWidget() = default;

bool QgsAnnotationMarkerItemWidget::setNewItem( QgsAnnotationItem *item )
{
  QgsAnnotationMarkerItem *markerItem = dynamic_cast<QgsAnnotationMarkerItem *>( item );
  if ( !markerItem )
    return false;

  mItem.reset( markerItem->clone() );
  if ( mItem->symbol() )
  {
    mSymbol.reset( mItem->symbol()->clone() );
  }
  else
  {
    mSymbol.reset( QgsMarkerSymbol::createSimple( {} ) );
  }
  mBlockChangedSignal = true;
  mSelector->loadSymbol( mSymbol.get() );
  mSelector->updatePreview();
  mPropertiesWidget->setItem( mItem.get() );
  mBlockChangedSignal = false;

  return true;
}


//
// QgsAnnotationPointTextItemWidget
//

QgsAnnotationPointTextItemWidget::QgsAnnotationPointTextItemWidget( QWidget *parent )
  : QgsAnnotationItemBaseWidget( parent )
{
  setupUi( this );

  mTextFormatWidget = new QgsTextFormatWidget();
  QVBoxLayout *vLayout = new QVBoxLayout();
  vLayout->setContentsMargins( 0, 0, 0, 0 );
  vLayout->addWidget( mTextFormatWidget );
  mTextFormatWidgetContainer->setLayout( vLayout );

  mTextEdit->setMode( QgsRichTextEditor::Mode::QgsTextRenderer );
  mTextEdit->setMaximumHeight( mTextEdit->fontMetrics().height() * 10 );

  mSpinTextAngle->setClearValue( 0 );

  mRotationModeCombo->addItem( tr( "Ignore Map Rotation" ), QVariant::fromValue( Qgis::SymbolRotationMode::IgnoreMapRotation ) );
  mRotationModeCombo->addItem( tr( "Rotate With Map" ), QVariant::fromValue( Qgis::SymbolRotationMode::RespectMapRotation ) );

  mAlignmentComboBox->setAvailableAlignments( Qt::AlignLeft | Qt::AlignHCenter | Qt::AlignRight );

  mTextFormatWidget->setDockMode( dockMode() );
  connect( mTextFormatWidget, &QgsTextFormatWidget::widgetChanged, this, [=] {
    mTextEdit->setMode(
      mTextFormatWidget->format().allowHtmlFormatting() ? QgsRichTextEditor::Mode::QgsTextRenderer : QgsRichTextEditor::Mode::PlainText
    );

    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );
  connect( mTextEdit, &QgsRichTextEditor::textChanged, this, [=] {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );
  connect( mInsertExpressionButton, &QPushButton::clicked, this, &QgsAnnotationPointTextItemWidget::mInsertExpressionButton_clicked );
  connect( mPropertiesWidget, &QgsAnnotationItemCommonPropertiesWidget::itemChanged, this, [=] {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );

  connect( mSpinTextAngle, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, [=] {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );

  connect( mRotationModeCombo, qOverload<int>( &QComboBox::currentIndexChanged ), this, [=] {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );

  connect( mAlignmentComboBox, &QgsAlignmentComboBox::changed, this, [=] {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );
}

QgsAnnotationItem *QgsAnnotationPointTextItemWidget::createItem()
{
  QgsAnnotationPointTextItem *newItem = mItem->clone();
  updateItem( newItem );
  return newItem;
}

void QgsAnnotationPointTextItemWidget::updateItem( QgsAnnotationItem *item )
{
  if ( QgsAnnotationPointTextItem *pointTextItem = dynamic_cast<QgsAnnotationPointTextItem *>( item ) )
  {
    mBlockChangedSignal = true;
    pointTextItem->setFormat( mTextFormatWidget->format() );
    pointTextItem->setText( mTextFormatWidget->format().allowHtmlFormatting() ? mTextEdit->toHtml() : mTextEdit->toPlainText() );
    pointTextItem->setAngle( mSpinTextAngle->value() );
    pointTextItem->setRotationMode( mRotationModeCombo->currentData().value<Qgis::SymbolRotationMode>() );
    pointTextItem->setAlignment( mAlignmentComboBox->currentAlignment() );
    mBlockChangedSignal = false;
    mPropertiesWidget->updateItem( pointTextItem );
  }
}

void QgsAnnotationPointTextItemWidget::setDockMode( bool dockMode )
{
  QgsAnnotationItemBaseWidget::setDockMode( dockMode );
  if ( mTextFormatWidget )
    mTextFormatWidget->setDockMode( dockMode );
}

void QgsAnnotationPointTextItemWidget::setContext( const QgsSymbolWidgetContext &context )
{
  QgsAnnotationItemBaseWidget::setContext( context );
  if ( mTextFormatWidget )
    mTextFormatWidget->setContext( context );
  mPropertiesWidget->setContext( context );
}

void QgsAnnotationPointTextItemWidget::focusDefaultWidget()
{
  mTextEdit->textEdit()->selectAll();
  mTextEdit->setFocus();
}

QgsAnnotationPointTextItemWidget::~QgsAnnotationPointTextItemWidget() = default;

bool QgsAnnotationPointTextItemWidget::setNewItem( QgsAnnotationItem *item )
{
  QgsAnnotationPointTextItem *textItem = dynamic_cast<QgsAnnotationPointTextItem *>( item );
  if ( !textItem )
    return false;

  mItem.reset( textItem->clone() );

  mBlockChangedSignal = true;
  mTextFormatWidget->setFormat( mItem->format() );
  mTextEdit->setMode( mItem->format().allowHtmlFormatting() ? QgsRichTextEditor::Mode::QgsTextRenderer : QgsRichTextEditor::Mode::PlainText );
  mTextEdit->setText( mItem->text() );
  mSpinTextAngle->setValue( mItem->angle() );
  mRotationModeCombo->setCurrentIndex( mRotationModeCombo->findData( QVariant::fromValue( mItem->rotationMode() ) ) );
  mAlignmentComboBox->setCurrentAlignment( mItem->alignment() & Qt::AlignHorizontal_Mask );
  mPropertiesWidget->setItem( mItem.get() );
  mBlockChangedSignal = false;

  return true;
}

void QgsAnnotationPointTextItemWidget::mInsertExpressionButton_clicked()
{
  QString expression = QgsExpressionFinder::findAndSelectActiveExpression( mTextEdit->textEdit() );

  QgsExpressionContext expressionContext;
  if ( context().expressionContext() )
    expressionContext = *( context().expressionContext() );
  else
    expressionContext = QgsProject::instance()->createExpressionContext();

  QgsExpressionBuilderDialog exprDlg( nullptr, expression, this, QStringLiteral( "generic" ), expressionContext );

  exprDlg.setWindowTitle( tr( "Insert Expression" ) );
  if ( exprDlg.exec() == QDialog::Accepted )
  {
    expression = exprDlg.expressionText().trimmed();
    if ( !expression.isEmpty() )
    {
      mTextEdit->textEdit()->insertPlainText( "[%" + expression + "%]" );
    }
  }
}


//
// QgsAnnotationLineTextItemWidget
//

QgsAnnotationLineTextItemWidget::QgsAnnotationLineTextItemWidget( QWidget *parent )
  : QgsAnnotationItemBaseWidget( parent )
{
  setupUi( this );

  mTextFormatWidget = new QgsTextFormatWidget();
  QVBoxLayout *vLayout = new QVBoxLayout();
  vLayout->setContentsMargins( 0, 0, 0, 0 );
  vLayout->addWidget( mTextFormatWidget );
  mTextFormatWidgetContainer->setLayout( vLayout );

  mTextEdit->setMode( QgsRichTextEditor::Mode::QgsTextRenderer );
  mTextEdit->setMaximumHeight( mTextEdit->fontMetrics().height() * 10 );

  mTextFormatWidget->setDockMode( dockMode() );
  connect( mTextFormatWidget, &QgsTextFormatWidget::widgetChanged, this, [=] {
    mTextEdit->setMode(
      mTextFormatWidget->format().allowHtmlFormatting() ? QgsRichTextEditor::Mode::QgsTextRenderer : QgsRichTextEditor::Mode::PlainText
    );

    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );
  connect( mTextEdit, &QgsRichTextEditor::textChanged, this, [=] {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );
  connect( mInsertExpressionButton, &QPushButton::clicked, this, &QgsAnnotationLineTextItemWidget::mInsertExpressionButton_clicked );
  connect( mPropertiesWidget, &QgsAnnotationItemCommonPropertiesWidget::itemChanged, this, [=] {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );

  mOffsetUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << Qgis::RenderUnit::Millimeters << Qgis::RenderUnit::MetersInMapUnits << Qgis::RenderUnit::MapUnits << Qgis::RenderUnit::Pixels << Qgis::RenderUnit::Points << Qgis::RenderUnit::Inches );
  mSpinOffset->setClearValue( 0.0 );
  connect( mSpinOffset, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, [=] {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );

  connect( mOffsetUnitWidget, &QgsUnitSelectionWidget::changed, this, [=] {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );
}

QgsAnnotationLineTextItemWidget::~QgsAnnotationLineTextItemWidget() = default;

QgsAnnotationItem *QgsAnnotationLineTextItemWidget::createItem()
{
  QgsAnnotationLineTextItem *newItem = mItem->clone();
  updateItem( newItem );
  return newItem;
}

void QgsAnnotationLineTextItemWidget::updateItem( QgsAnnotationItem *item )
{
  if ( QgsAnnotationLineTextItem *lineTextItem = dynamic_cast<QgsAnnotationLineTextItem *>( item ) )
  {
    mBlockChangedSignal = true;
    lineTextItem->setFormat( mTextFormatWidget->format() );
    lineTextItem->setText( mTextFormatWidget->format().allowHtmlFormatting() ? mTextEdit->toHtml() : mTextEdit->toPlainText() );

    lineTextItem->setOffsetFromLine( mSpinOffset->value() );
    lineTextItem->setOffsetFromLineUnit( mOffsetUnitWidget->unit() );
    lineTextItem->setOffsetFromLineMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );

    mBlockChangedSignal = false;
    mPropertiesWidget->updateItem( lineTextItem );
  }
}

void QgsAnnotationLineTextItemWidget::setDockMode( bool dockMode )
{
  QgsAnnotationItemBaseWidget::setDockMode( dockMode );
  if ( mTextFormatWidget )
    mTextFormatWidget->setDockMode( dockMode );
}

void QgsAnnotationLineTextItemWidget::setContext( const QgsSymbolWidgetContext &context )
{
  QgsAnnotationItemBaseWidget::setContext( context );
  if ( mTextFormatWidget )
    mTextFormatWidget->setContext( context );
  mPropertiesWidget->setContext( context );
}

void QgsAnnotationLineTextItemWidget::focusDefaultWidget()
{
  mTextEdit->textEdit()->selectAll();
  mTextEdit->setFocus();
}

bool QgsAnnotationLineTextItemWidget::setNewItem( QgsAnnotationItem *item )
{
  QgsAnnotationLineTextItem *textItem = dynamic_cast<QgsAnnotationLineTextItem *>( item );
  if ( !textItem )
    return false;

  mItem.reset( textItem->clone() );

  mBlockChangedSignal = true;
  mTextFormatWidget->setFormat( mItem->format() );
  mTextEdit->setMode( mItem->format().allowHtmlFormatting() ? QgsRichTextEditor::Mode::QgsTextRenderer : QgsRichTextEditor::Mode::PlainText );
  mTextEdit->setText( mItem->text() );
  mPropertiesWidget->setItem( mItem.get() );

  mSpinOffset->setValue( mItem->offsetFromLine() );
  mOffsetUnitWidget->setUnit( mItem->offsetFromLineUnit() );
  mOffsetUnitWidget->setMapUnitScale( mItem->offsetFromLineMapUnitScale() );

  mBlockChangedSignal = false;

  return true;
}

void QgsAnnotationLineTextItemWidget::mInsertExpressionButton_clicked()
{
  QString expression = QgsExpressionFinder::findAndSelectActiveExpression( mTextEdit->textEdit() );

  QgsExpressionContext expressionContext;
  if ( context().expressionContext() )
    expressionContext = *( context().expressionContext() );
  else
    expressionContext = QgsProject::instance()->createExpressionContext();

  QgsExpressionBuilderDialog exprDlg( nullptr, expression, this, QStringLiteral( "generic" ), expressionContext );

  exprDlg.setWindowTitle( tr( "Insert Expression" ) );
  if ( exprDlg.exec() == QDialog::Accepted )
  {
    expression = exprDlg.expressionText().trimmed();
    if ( !expression.isEmpty() )
    {
      mTextEdit->textEdit()->insertPlainText( "[%" + expression + "%]" );
    }
  }
}


//
// QgsAnnotationRectangleTextItemWidget
//

QgsAnnotationRectangleTextItemWidget::QgsAnnotationRectangleTextItemWidget( QWidget *parent )
  : QgsAnnotationItemBaseWidget( parent )
{
  setupUi( this );

  mSizeModeCombo->addItem( tr( "Scale Dependent Size" ), QVariant::fromValue( Qgis::AnnotationPlacementMode::SpatialBounds ) );
  mSizeModeCombo->addItem( tr( "Fixed Size" ), QVariant::fromValue( Qgis::AnnotationPlacementMode::FixedSize ) );
  mSizeModeCombo->addItem( tr( "Relative to Map" ), QVariant::fromValue( Qgis::AnnotationPlacementMode::RelativeToMapFrame ) );

  mSizeUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << Qgis::RenderUnit::Pixels << Qgis::RenderUnit::Millimeters << Qgis::RenderUnit::Points << Qgis::RenderUnit::Inches << Qgis::RenderUnit::Percentage );


  mBackgroundSymbolButton->setSymbolType( Qgis::SymbolType::Fill );
  mBackgroundSymbolButton->setDialogTitle( tr( "Background" ) );
  mBackgroundSymbolButton->registerExpressionContextGenerator( this );
  mFrameSymbolButton->setSymbolType( Qgis::SymbolType::Fill );
  mFrameSymbolButton->setDialogTitle( tr( "Frame" ) );
  mFrameSymbolButton->registerExpressionContextGenerator( this );

  mSpinBottomMargin->setClearValue( 0 );
  mSpinTopMargin->setClearValue( 0 );
  mSpinRightMargin->setClearValue( 0 );
  mSpinLeftMargin->setClearValue( 0 );
  mMarginUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << Qgis::RenderUnit::Millimeters << Qgis::RenderUnit::MetersInMapUnits << Qgis::RenderUnit::MapUnits << Qgis::RenderUnit::Pixels << Qgis::RenderUnit::Points << Qgis::RenderUnit::Inches );

  mTextFormatWidget = new QgsTextFormatWidget();
  QVBoxLayout *vLayout = new QVBoxLayout();
  vLayout->setContentsMargins( 0, 0, 0, 0 );
  vLayout->addWidget( mTextFormatWidget );
  mTextFormatWidgetContainer->setLayout( vLayout );

  mTextEdit->setMode( QgsRichTextEditor::Mode::QgsTextRenderer );
  mTextEdit->setMaximumHeight( mTextEdit->fontMetrics().height() * 10 );

  mAlignmentComboBox->setAvailableAlignments( Qt::AlignLeft | Qt::AlignHCenter | Qt::AlignRight | Qt::AlignJustify );
  mVerticalAlignmentComboBox->setAvailableAlignments( Qt::AlignTop | Qt::AlignVCenter | Qt::AlignBottom );

  mTextFormatWidget->setDockMode( dockMode() );
  connect( mTextFormatWidget, &QgsTextFormatWidget::widgetChanged, this, [this] {
    mTextEdit->setMode(
      mTextFormatWidget->format().allowHtmlFormatting() ? QgsRichTextEditor::Mode::QgsTextRenderer : QgsRichTextEditor::Mode::PlainText
    );

    onWidgetChanged();
  } );
  connect( mTextEdit, &QgsRichTextEditor::textChanged, this, &QgsAnnotationRectangleTextItemWidget::onWidgetChanged );
  connect( mInsertExpressionButton, &QPushButton::clicked, this, &QgsAnnotationRectangleTextItemWidget::mInsertExpressionButton_clicked );
  connect( mPropertiesWidget, &QgsAnnotationItemCommonPropertiesWidget::itemChanged, this, &QgsAnnotationRectangleTextItemWidget::onWidgetChanged );
  connect( mAlignmentComboBox, &QgsAlignmentComboBox::changed, this, &QgsAnnotationRectangleTextItemWidget::onWidgetChanged );
  connect( mVerticalAlignmentComboBox, &QgsAlignmentComboBox::changed, this, &QgsAnnotationRectangleTextItemWidget::onWidgetChanged );
  connect( mFrameCheckbox, &QGroupBox::toggled, this, &QgsAnnotationRectangleTextItemWidget::onWidgetChanged );
  connect( mBackgroundCheckbox, &QGroupBox::toggled, this, &QgsAnnotationRectangleTextItemWidget::onWidgetChanged );
  connect( mBackgroundSymbolButton, &QgsSymbolButton::changed, this, &QgsAnnotationRectangleTextItemWidget::onWidgetChanged );
  connect( mFrameSymbolButton, &QgsSymbolButton::changed, this, &QgsAnnotationRectangleTextItemWidget::onWidgetChanged );
  connect( mSpinTopMargin, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsAnnotationRectangleTextItemWidget::onWidgetChanged );
  connect( mSpinRightMargin, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsAnnotationRectangleTextItemWidget::onWidgetChanged );
  connect( mSpinLeftMargin, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsAnnotationRectangleTextItemWidget::onWidgetChanged );
  connect( mSpinBottomMargin, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsAnnotationRectangleTextItemWidget::onWidgetChanged );
  connect( mMarginUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsAnnotationRectangleTextItemWidget::onWidgetChanged );

  connect( mSizeUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsAnnotationRectangleTextItemWidget::onWidgetChanged );

  connect( mWidthSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsAnnotationRectangleTextItemWidget::setWidth );
  connect( mHeightSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsAnnotationRectangleTextItemWidget::setHeight );

  connect( mSizeModeCombo, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsAnnotationRectangleTextItemWidget::sizeModeChanged );
  mWidgetFixedSize->hide();
  sizeModeChanged();
}

QgsAnnotationItem *QgsAnnotationRectangleTextItemWidget::createItem()
{
  QgsAnnotationRectangleTextItem *newItem = mItem->clone();
  updateItem( newItem );
  return newItem;
}

void QgsAnnotationRectangleTextItemWidget::updateItem( QgsAnnotationItem *item )
{
  if ( QgsAnnotationRectangleTextItem *rectTextItem = dynamic_cast<QgsAnnotationRectangleTextItem *>( item ) )
  {
    mBlockChangedSignal = true;
    rectTextItem->setFormat( mTextFormatWidget->format() );
    rectTextItem->setText( mTextFormatWidget->format().allowHtmlFormatting() ? mTextEdit->toHtml() : mTextEdit->toPlainText() );
    rectTextItem->setAlignment( mAlignmentComboBox->currentAlignment() | mVerticalAlignmentComboBox->currentAlignment() );

    rectTextItem->setPlacementMode( mSizeModeCombo->currentData().value<Qgis::AnnotationPlacementMode>() );

    rectTextItem->setFixedSize( QSizeF( mWidthSpinBox->value(), mHeightSpinBox->value() ) );
    rectTextItem->setFixedSizeUnit( mSizeUnitWidget->unit() );

    rectTextItem->setBackgroundEnabled( mBackgroundCheckbox->isChecked() );
    rectTextItem->setFrameEnabled( mFrameCheckbox->isChecked() );
    rectTextItem->setBackgroundSymbol( mBackgroundSymbolButton->clonedSymbol<QgsFillSymbol>() );
    rectTextItem->setFrameSymbol( mFrameSymbolButton->clonedSymbol<QgsFillSymbol>() );

    rectTextItem->setMargins( QgsMargins( mSpinLeftMargin->value(), mSpinTopMargin->value(), mSpinRightMargin->value(), mSpinBottomMargin->value() ) );
    rectTextItem->setMarginsUnit( mMarginUnitWidget->unit() );

    if ( mUpdateItemPosition )
    {
      rectTextItem->setBounds( mItem->bounds() );
      mUpdateItemPosition = false;
    }

    mBlockChangedSignal = false;

    mPropertiesWidget->updateItem( rectTextItem );
  }
}

void QgsAnnotationRectangleTextItemWidget::setDockMode( bool dockMode )
{
  QgsAnnotationItemBaseWidget::setDockMode( dockMode );
  if ( mTextFormatWidget )
    mTextFormatWidget->setDockMode( dockMode );
}

void QgsAnnotationRectangleTextItemWidget::setContext( const QgsSymbolWidgetContext &context )
{
  QgsAnnotationItemBaseWidget::setContext( context );
  if ( mTextFormatWidget )
    mTextFormatWidget->setContext( context );
  mBackgroundSymbolButton->setMapCanvas( context.mapCanvas() );
  mBackgroundSymbolButton->setMessageBar( context.messageBar() );
  mFrameSymbolButton->setMapCanvas( context.mapCanvas() );
  mFrameSymbolButton->setMessageBar( context.messageBar() );
  mPropertiesWidget->setContext( context );
}

QgsExpressionContext QgsAnnotationRectangleTextItemWidget::createExpressionContext() const
{
  QgsExpressionContext expressionContext;
  if ( context().expressionContext() )
    expressionContext = *( context().expressionContext() );
  else
    expressionContext = QgsProject::instance()->createExpressionContext();
  return expressionContext;
}

void QgsAnnotationRectangleTextItemWidget::focusDefaultWidget()
{
  mTextEdit->textEdit()->selectAll();
  mTextEdit->setFocus();
}

QgsAnnotationRectangleTextItemWidget::~QgsAnnotationRectangleTextItemWidget() = default;

bool QgsAnnotationRectangleTextItemWidget::setNewItem( QgsAnnotationItem *item )
{
  QgsAnnotationRectangleTextItem *textItem = dynamic_cast<QgsAnnotationRectangleTextItem *>( item );
  if ( !textItem )
    return false;

  mItem.reset( textItem->clone() );

  mBlockChangedSignal = true;
  mTextFormatWidget->setFormat( mItem->format() );
  mTextEdit->setMode( mItem->format().allowHtmlFormatting() ? QgsRichTextEditor::Mode::QgsTextRenderer : QgsRichTextEditor::Mode::PlainText );
  mTextEdit->setText( mItem->text() );
  mAlignmentComboBox->setCurrentAlignment( mItem->alignment() & Qt::AlignHorizontal_Mask );
  mVerticalAlignmentComboBox->setCurrentAlignment( mItem->alignment() & Qt::AlignVertical_Mask );
  mPropertiesWidget->setItem( mItem.get() );

  mBackgroundCheckbox->setChecked( textItem->backgroundEnabled() );
  if ( const QgsSymbol *symbol = textItem->backgroundSymbol() )
    mBackgroundSymbolButton->setSymbol( symbol->clone() );

  mFrameCheckbox->setChecked( textItem->frameEnabled() );
  if ( const QgsSymbol *symbol = textItem->frameSymbol() )
    mFrameSymbolButton->setSymbol( symbol->clone() );

  mMarginUnitWidget->setUnit( textItem->marginsUnit() );
  mSpinLeftMargin->setValue( textItem->margins().left() );
  mSpinTopMargin->setValue( textItem->margins().top() );
  mSpinRightMargin->setValue( textItem->margins().right() );
  mSpinBottomMargin->setValue( textItem->margins().bottom() );

  mWidthSpinBox->setValue( textItem->fixedSize().width() );
  mHeightSpinBox->setValue( textItem->fixedSize().height() );
  mSizeModeCombo->setCurrentIndex( mSizeModeCombo->findData( QVariant::fromValue( textItem->placementMode() ) ) );

  mBlockChangedSignal = false;

  return true;
}

void QgsAnnotationRectangleTextItemWidget::onWidgetChanged()
{
  if ( !mBlockChangedSignal )
    emit itemChanged();
}

void QgsAnnotationRectangleTextItemWidget::sizeModeChanged()
{
  const Qgis::AnnotationPlacementMode mode = mSizeModeCombo->currentData().value<Qgis::AnnotationPlacementMode>();
  switch ( mode )
  {
    case Qgis::AnnotationPlacementMode::SpatialBounds:
      mWidgetFixedSize->hide();
      break;

    case Qgis::AnnotationPlacementMode::FixedSize:
      mWidgetFixedSize->show();
      break;

    case Qgis::AnnotationPlacementMode::RelativeToMapFrame:
    {
      if ( const QgsRenderedAnnotationItemDetails *details = renderedItemDetails() )
      {
        // convert item bounds to relative position
        const QgsRectangle itemBounds = details->boundingBox();
        if ( QgsMapCanvas *canvas = context().mapCanvas() )
        {
          const double centerX = ( itemBounds.center().x() - canvas->extent().xMinimum() ) / canvas->extent().width();
          const double centerY = ( canvas->extent().yMaximum() - itemBounds.center().y() ) / canvas->extent().height();
          mItem->setBounds( QgsRectangle::fromCenterAndSize( QgsPointXY( centerX, centerY ), 0.5, 0.5 ) );
          mUpdateItemPosition = true;
        }
      }

      mWidgetFixedSize->hide();
      break;
    }
  }

  onWidgetChanged();
}

void QgsAnnotationRectangleTextItemWidget::setWidth()
{
  onWidgetChanged();
}

void QgsAnnotationRectangleTextItemWidget::setHeight()
{
  onWidgetChanged();
}

void QgsAnnotationRectangleTextItemWidget::mInsertExpressionButton_clicked()
{
  QString expression = QgsExpressionFinder::findAndSelectActiveExpression( mTextEdit->textEdit() );

  QgsExpressionBuilderDialog exprDlg( nullptr, expression, this, QStringLiteral( "generic" ), createExpressionContext() );

  exprDlg.setWindowTitle( tr( "Insert Expression" ) );
  if ( exprDlg.exec() == QDialog::Accepted )
  {
    expression = exprDlg.expressionText().trimmed();
    if ( !expression.isEmpty() )
    {
      mTextEdit->textEdit()->insertPlainText( "[%" + expression + "%]" );
    }
  }
}


//
// QgsAnnotationPictureItemWidget
//

QgsAnnotationPictureItemWidget::QgsAnnotationPictureItemWidget( QWidget *parent )
  : QgsAnnotationItemBaseWidget( parent )
{
  setupUi( this );

  mSizeStackedWidget->setSizeMode( QgsStackedWidget::SizeMode::CurrentPageOnly );

  mSizeModeCombo->addItem( tr( "Scale Dependent Size" ), QVariant::fromValue( Qgis::AnnotationPlacementMode::SpatialBounds ) );
  mSizeModeCombo->addItem( tr( "Fixed Size" ), QVariant::fromValue( Qgis::AnnotationPlacementMode::FixedSize ) );
  mSizeModeCombo->addItem( tr( "Relative to Map" ), QVariant::fromValue( Qgis::AnnotationPlacementMode::RelativeToMapFrame ) );

  mSizeUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << Qgis::RenderUnit::Pixels << Qgis::RenderUnit::Millimeters << Qgis::RenderUnit::Points << Qgis::RenderUnit::Inches << Qgis::RenderUnit::Percentage );

  mBackgroundSymbolButton->setSymbolType( Qgis::SymbolType::Fill );
  mBackgroundSymbolButton->setDialogTitle( tr( "Background" ) );
  mBackgroundSymbolButton->registerExpressionContextGenerator( this );
  mFrameSymbolButton->setSymbolType( Qgis::SymbolType::Fill );
  mFrameSymbolButton->setDialogTitle( tr( "Frame" ) );
  mFrameSymbolButton->registerExpressionContextGenerator( this );

  connect( mPropertiesWidget, &QgsAnnotationItemCommonPropertiesWidget::itemChanged, this, [=] {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );

  connect( mSizeModeCombo, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsAnnotationPictureItemWidget::sizeModeChanged );

  connect( mRadioSVG, &QRadioButton::toggled, this, &QgsAnnotationPictureItemWidget::modeChanged );
  connect( mRadioRaster, &QRadioButton::toggled, this, &QgsAnnotationPictureItemWidget::modeChanged );
  connect( mSourceLineEdit, &QgsPictureSourceLineEditBase::sourceChanged, this, [=]( const QString &source ) {
    if ( !mRadioSVG->isChecked() && QFileInfo( source ).suffix().compare( QLatin1String( "svg" ), Qt::CaseInsensitive ) == 0 )
    {
      mRadioSVG->setChecked( true );
    }

    onWidgetChanged();
  } );

  connect( mLockAspectRatioCheck, &QCheckBox::toggled, this, &QgsAnnotationPictureItemWidget::onWidgetChanged );
  connect( mFrameCheckbox, &QGroupBox::toggled, this, &QgsAnnotationPictureItemWidget::onWidgetChanged );
  connect( mBackgroundCheckbox, &QGroupBox::toggled, this, &QgsAnnotationPictureItemWidget::onWidgetChanged );
  connect( mBackgroundSymbolButton, &QgsSymbolButton::changed, this, &QgsAnnotationPictureItemWidget::onWidgetChanged );
  connect( mFrameSymbolButton, &QgsSymbolButton::changed, this, &QgsAnnotationPictureItemWidget::onWidgetChanged );
  connect( mSizeUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsAnnotationPictureItemWidget::onWidgetChanged );

  connect( mWidthSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsAnnotationPictureItemWidget::setWidth );
  connect( mHeightSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsAnnotationPictureItemWidget::setHeight );
  connect( mLockAspectRatio, &QgsRatioLockButton::lockChanged, this, &QgsAnnotationPictureItemWidget::setLockAspectRatio );
}

QgsAnnotationPictureItemWidget::~QgsAnnotationPictureItemWidget() = default;

QgsAnnotationItem *QgsAnnotationPictureItemWidget::createItem()
{
  QgsAnnotationPictureItem *newItem = mItem->clone();
  updateItem( newItem );
  return newItem;
}

void QgsAnnotationPictureItemWidget::updateItem( QgsAnnotationItem *item )
{
  if ( QgsAnnotationPictureItem *pictureItem = dynamic_cast<QgsAnnotationPictureItem *>( item ) )
  {
    const bool svg = mRadioSVG->isChecked();
    const Qgis::PictureFormat newFormat = svg ? Qgis::PictureFormat::SVG : Qgis::PictureFormat::Raster;
    const QString path = mSourceLineEdit->source();
    pictureItem->setPath( newFormat, path );

    pictureItem->setPlacementMode( mSizeModeCombo->currentData().value<Qgis::AnnotationPlacementMode>() );
    switch ( pictureItem->placementMode() )
    {
      case Qgis::AnnotationPlacementMode::SpatialBounds:
        pictureItem->setLockAspectRatio( mLockAspectRatioCheck->isChecked() );
        break;
      case Qgis::AnnotationPlacementMode::FixedSize:
      case Qgis::AnnotationPlacementMode::RelativeToMapFrame:
        pictureItem->setLockAspectRatio( mLockAspectRatio->isChecked() );
        break;
    }

    pictureItem->setFixedSize( QSizeF( mWidthSpinBox->value(), mHeightSpinBox->value() ) );
    pictureItem->setFixedSizeUnit( mSizeUnitWidget->unit() );

    pictureItem->setBackgroundEnabled( mBackgroundCheckbox->isChecked() );
    pictureItem->setFrameEnabled( mFrameCheckbox->isChecked() );
    pictureItem->setBackgroundSymbol( mBackgroundSymbolButton->clonedSymbol<QgsFillSymbol>() );
    pictureItem->setFrameSymbol( mFrameSymbolButton->clonedSymbol<QgsFillSymbol>() );

    if ( mUpdateItemPosition )
    {
      pictureItem->setBounds( mItem->bounds() );
      mUpdateItemPosition = false;
    }

    mPropertiesWidget->updateItem( pictureItem );
  }
}

void QgsAnnotationPictureItemWidget::setDockMode( bool dockMode )
{
  QgsAnnotationItemBaseWidget::setDockMode( dockMode );
}

void QgsAnnotationPictureItemWidget::setContext( const QgsSymbolWidgetContext &context )
{
  QgsAnnotationItemBaseWidget::setContext( context );
  mPropertiesWidget->setContext( context );
  mBackgroundSymbolButton->setMapCanvas( context.mapCanvas() );
  mBackgroundSymbolButton->setMessageBar( context.messageBar() );
  mFrameSymbolButton->setMapCanvas( context.mapCanvas() );
  mFrameSymbolButton->setMessageBar( context.messageBar() );
}

QgsExpressionContext QgsAnnotationPictureItemWidget::createExpressionContext() const
{
  QgsExpressionContext expressionContext;
  if ( context().expressionContext() )
    expressionContext = *( context().expressionContext() );
  else
    expressionContext = QgsProject::instance()->createExpressionContext();
  return expressionContext;
}

void QgsAnnotationPictureItemWidget::focusDefaultWidget()
{
  mSourceLineEdit->setFocus();
}

bool QgsAnnotationPictureItemWidget::setNewItem( QgsAnnotationItem *item )
{
  QgsAnnotationPictureItem *pictureItem = dynamic_cast<QgsAnnotationPictureItem *>( item );
  if ( !pictureItem )
    return false;

  mItem.reset( pictureItem->clone() );

  mBlockChangedSignal = true;
  mPropertiesWidget->setItem( mItem.get() );

  mLockAspectRatioCheck->setChecked( mItem->lockAspectRatio() );
  mLockAspectRatio->setLocked( mItem->lockAspectRatio() );
  switch ( pictureItem->format() )
  {
    case Qgis::PictureFormat::SVG:
      mRadioSVG->setChecked( true );
      break;
    case Qgis::PictureFormat::Raster:
      mRadioRaster->setChecked( true );
      break;
    case Qgis::PictureFormat::Unknown:
      break;
  }

  mSourceLineEdit->setSource( pictureItem->path() );

  mBackgroundCheckbox->setChecked( pictureItem->backgroundEnabled() );
  if ( const QgsSymbol *symbol = pictureItem->backgroundSymbol() )
    mBackgroundSymbolButton->setSymbol( symbol->clone() );

  mFrameCheckbox->setChecked( pictureItem->frameEnabled() );
  if ( const QgsSymbol *symbol = pictureItem->frameSymbol() )
    mFrameSymbolButton->setSymbol( symbol->clone() );

  mWidthSpinBox->setValue( pictureItem->fixedSize().width() );
  mHeightSpinBox->setValue( pictureItem->fixedSize().height() );
  mSizeModeCombo->setCurrentIndex( mSizeModeCombo->findData( QVariant::fromValue( pictureItem->placementMode() ) ) );
  sizeModeChanged();

  mBlockChangedSignal = false;

  return true;
}

void QgsAnnotationPictureItemWidget::onWidgetChanged()
{
  if ( !mBlockChangedSignal )
    emit itemChanged();
}

void QgsAnnotationPictureItemWidget::modeChanged( bool checked )
{
  if ( !checked )
    return;

  const bool svg = mRadioSVG->isChecked();

  if ( svg )
    mSourceLineEdit->setMode( QgsPictureSourceLineEditBase::Svg );
  else
    mSourceLineEdit->setMode( QgsPictureSourceLineEditBase::Image );

  onWidgetChanged();
}

void QgsAnnotationPictureItemWidget::sizeModeChanged()
{
  const Qgis::AnnotationPlacementMode mode = mSizeModeCombo->currentData().value<Qgis::AnnotationPlacementMode>();
  switch ( mode )
  {
    case Qgis::AnnotationPlacementMode::SpatialBounds:
      mSizeStackedWidget->setCurrentWidget( mPageSpatialBounds );
      break;

    case Qgis::AnnotationPlacementMode::FixedSize:
      mSizeStackedWidget->setCurrentWidget( mPageFixedSize );
      break;

    case Qgis::AnnotationPlacementMode::RelativeToMapFrame:
    {
      if ( const QgsRenderedAnnotationItemDetails *details = renderedItemDetails() )
      {
        // convert item bounds to relative position
        const QgsRectangle itemBounds = details->boundingBox();
        if ( QgsMapCanvas *canvas = context().mapCanvas() )
        {
          const double centerX = ( itemBounds.center().x() - canvas->extent().xMinimum() ) / canvas->extent().width();
          const double centerY = ( canvas->extent().yMaximum() - itemBounds.center().y() ) / canvas->extent().height();
          mItem->setBounds( QgsRectangle::fromCenterAndSize( QgsPointXY( centerX, centerY ), 0.5, 0.5 ) );
          mUpdateItemPosition = true;
        }
      }

      mSizeStackedWidget->setCurrentWidget( mPageFixedSize );
      break;
    }
  }

  onWidgetChanged();
}

void QgsAnnotationPictureItemWidget::setWidth()
{
  if ( mLockAspectRatio->locked() )
  {
    const double ratio = pictureAspectRatio();
    if ( ratio > 0 )
      whileBlocking( mHeightSpinBox )->setValue( mWidthSpinBox->value() * ratio );
  }

  onWidgetChanged();
}

void QgsAnnotationPictureItemWidget::setHeight()
{
  if ( mLockAspectRatio->locked() )
  {
    const double ratio = pictureAspectRatio();
    if ( ratio > 0 )
      whileBlocking( mWidthSpinBox )->setValue( mHeightSpinBox->value() / ratio );
  }

  onWidgetChanged();
}

void QgsAnnotationPictureItemWidget::setLockAspectRatio( bool locked )
{
  if ( locked && !mBlockChangedSignal )
  {
    const double ratio = pictureAspectRatio();
    if ( ratio > 0 )
      whileBlocking( mHeightSpinBox )->setValue( mWidthSpinBox->value() * ratio );
  }

  onWidgetChanged();
}

double QgsAnnotationPictureItemWidget::pictureAspectRatio() const
{
  const bool svg = mRadioSVG->isChecked();
  const QString path = mSourceLineEdit->source();
  QSizeF size;
  if ( svg )
  {
    size = QgsApplication::svgCache()->svgViewboxSize( path, 100, QColor(), QColor(), 1, 1 );
  }
  else
  {
    size = QgsApplication::imageCache()->originalSize( path );
  }
  if ( size.isValid() && size.width() > 0 )
    return size.height() / size.width();

  return 0;
}

///@endcond PRIVATE
