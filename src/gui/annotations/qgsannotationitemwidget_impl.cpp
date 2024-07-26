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
#include "qgsannotationpictureitem.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgstextformatwidget.h"
#include "qgsapplication.h"
#include "qgsrecentstylehandler.h"
#include "qgsexpressionfinder.h"
#include "qgsimagecache.h"
#include "qgssvgcache.h"

///@cond PRIVATE

QgsAnnotationPolygonItemWidget::QgsAnnotationPolygonItemWidget( QWidget *parent )
  : QgsAnnotationItemBaseWidget( parent )
{
  setupUi( this );

  mSelector = new QgsSymbolSelectorWidget( mSymbol.get(), QgsStyle::defaultStyle(), nullptr, nullptr );
  mSelector->setDockMode( dockMode() );
  connect( mSelector, &QgsSymbolSelectorWidget::symbolModified, this, [ = ]
  {
    if ( !mBlockChangedSignal )
    {
      emit itemChanged();
      QgsApplication::recentStyleHandler()->pushRecentSymbol( QStringLiteral( "polygon_annotation_item" ), qgis::down_cast< QgsFillSymbol * >( mSelector->symbol()->clone() ) );
    }
  } );
  connect( mSelector, &QgsPanelWidget::showPanel, this, &QgsPanelWidget::openPanel );

  QVBoxLayout *layout = new QVBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( mSelector );
  mSymbolSelectorFrame->setLayout( layout );

  connect( mPropertiesWidget, &QgsAnnotationItemCommonPropertiesWidget::itemChanged, this, [ = ]
  {
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
  if ( QgsAnnotationPolygonItem *polygonItem = dynamic_cast< QgsAnnotationPolygonItem * >( item ) )
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
  QgsAnnotationPolygonItem *polygonItem = dynamic_cast< QgsAnnotationPolygonItem * >( item );
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
  connect( mSelector, &QgsSymbolSelectorWidget::symbolModified, this, [ = ]
  {
    if ( !mBlockChangedSignal )
    {
      emit itemChanged();
      QgsApplication::recentStyleHandler()->pushRecentSymbol( QStringLiteral( "line_annotation_item" ), qgis::down_cast< QgsLineSymbol * >( mSelector->symbol()->clone() ) );
    }
  } );
  connect( mSelector, &QgsPanelWidget::showPanel, this, &QgsPanelWidget::openPanel );

  QVBoxLayout *layout = new QVBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( mSelector );
  mSymbolSelectorFrame->setLayout( layout );

  connect( mPropertiesWidget, &QgsAnnotationItemCommonPropertiesWidget::itemChanged, this, [ = ]
  {
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
  if ( QgsAnnotationLineItem *lineItem = dynamic_cast< QgsAnnotationLineItem * >( item ) )
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
  QgsAnnotationLineItem *lineItem = dynamic_cast< QgsAnnotationLineItem * >( item );
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
  connect( mSelector, &QgsSymbolSelectorWidget::symbolModified, this, [ = ]
  {
    if ( !mBlockChangedSignal )
    {
      emit itemChanged();
      QgsApplication::recentStyleHandler()->pushRecentSymbol( QStringLiteral( "marker_annotation_item" ), qgis::down_cast< QgsMarkerSymbol * >( mSelector->symbol()->clone() ) );
    }
  } );
  connect( mSelector, &QgsPanelWidget::showPanel, this, &QgsPanelWidget::openPanel );

  QVBoxLayout *layout = new QVBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( mSelector );
  mSymbolSelectorFrame->setLayout( layout );

  connect( mPropertiesWidget, &QgsAnnotationItemCommonPropertiesWidget::itemChanged, this, [ = ]
  {
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
  if ( QgsAnnotationMarkerItem *markerItem = dynamic_cast< QgsAnnotationMarkerItem * >( item ) )
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
  QgsAnnotationMarkerItem *markerItem = dynamic_cast< QgsAnnotationMarkerItem * >( item );
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

  mTextEdit->setMaximumHeight( mTextEdit->fontMetrics().height() * 10 );

  mSpinTextAngle->setClearValue( 0 );

  mRotationModeCombo->addItem( tr( "Ignore Map Rotation" ), QVariant::fromValue( Qgis::SymbolRotationMode::IgnoreMapRotation ) );
  mRotationModeCombo->addItem( tr( "Rotate With Map" ), QVariant::fromValue( Qgis::SymbolRotationMode::RespectMapRotation ) );

  mAlignmentComboBox->setAvailableAlignments( Qt::AlignLeft | Qt::AlignHCenter | Qt::AlignRight );

  mTextFormatWidget->setDockMode( dockMode() );
  connect( mTextFormatWidget, &QgsTextFormatWidget::widgetChanged, this, [ = ]
  {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );
  connect( mTextEdit, &QPlainTextEdit::textChanged, this, [ = ]
  {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );
  connect( mInsertExpressionButton, &QPushButton::clicked, this, &QgsAnnotationPointTextItemWidget::mInsertExpressionButton_clicked );
  connect( mPropertiesWidget, &QgsAnnotationItemCommonPropertiesWidget::itemChanged, this, [ = ]
  {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );

  connect( mSpinTextAngle, qOverload< double >( &QgsDoubleSpinBox::valueChanged ), this, [ = ]
  {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );

  connect( mRotationModeCombo, qOverload< int >( &QComboBox::currentIndexChanged ), this, [ = ]
  {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );

  connect( mAlignmentComboBox, &QgsAlignmentComboBox::changed, this, [ = ]
  {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );
}

QgsAnnotationItem *QgsAnnotationPointTextItemWidget::createItem()
{
  QgsAnnotationPointTextItem *newItem = mItem->clone();
  newItem->setFormat( mTextFormatWidget->format() );
  newItem->setText( mTextEdit->toPlainText() );
  newItem->setAngle( mSpinTextAngle->value() );
  newItem->setRotationMode( mRotationModeCombo->currentData().value< Qgis::SymbolRotationMode >() );
  newItem->setAlignment( mAlignmentComboBox->currentAlignment() );
  mPropertiesWidget->updateItem( newItem );
  return newItem;
}

void QgsAnnotationPointTextItemWidget::updateItem( QgsAnnotationItem *item )
{
  if ( QgsAnnotationPointTextItem *pointTextItem = dynamic_cast< QgsAnnotationPointTextItem * >( item ) )
  {
    pointTextItem->setFormat( mTextFormatWidget->format() );
    pointTextItem->setText( mTextEdit->toPlainText() );
    pointTextItem->setAngle( mSpinTextAngle->value() );
    pointTextItem->setRotationMode( mRotationModeCombo->currentData().value< Qgis::SymbolRotationMode >() );
    pointTextItem->setAlignment( mAlignmentComboBox->currentAlignment() );
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
  mTextEdit->selectAll();
  mTextEdit->setFocus();
}

QgsAnnotationPointTextItemWidget::~QgsAnnotationPointTextItemWidget() = default;

bool QgsAnnotationPointTextItemWidget::setNewItem( QgsAnnotationItem *item )
{
  QgsAnnotationPointTextItem *textItem = dynamic_cast< QgsAnnotationPointTextItem * >( item );
  if ( !textItem )
    return false;

  mItem.reset( textItem->clone() );

  mBlockChangedSignal = true;
  mTextFormatWidget->setFormat( mItem->format() );
  mTextEdit->setPlainText( mItem->text() );
  mSpinTextAngle->setValue( mItem->angle() );
  mRotationModeCombo->setCurrentIndex( mRotationModeCombo->findData( QVariant::fromValue( mItem->rotationMode() ) ) );
  mAlignmentComboBox->setCurrentAlignment( mItem->alignment() & Qt::AlignHorizontal_Mask );
  mPropertiesWidget->setItem( mItem.get() );
  mBlockChangedSignal = false;

  return true;
}

void QgsAnnotationPointTextItemWidget::mInsertExpressionButton_clicked()
{
  QString expression = QgsExpressionFinder::findAndSelectActiveExpression( mTextEdit );

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
      mTextEdit->insertPlainText( "[%" + expression + "%]" );
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

  mTextEdit->setMaximumHeight( mTextEdit->fontMetrics().height() * 10 );

  mTextFormatWidget->setDockMode( dockMode() );
  connect( mTextFormatWidget, &QgsTextFormatWidget::widgetChanged, this, [ = ]
  {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );
  connect( mTextEdit, &QPlainTextEdit::textChanged, this, [ = ]
  {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );
  connect( mInsertExpressionButton, &QPushButton::clicked, this, &QgsAnnotationLineTextItemWidget::mInsertExpressionButton_clicked );
  connect( mPropertiesWidget, &QgsAnnotationItemCommonPropertiesWidget::itemChanged, this, [ = ]
  {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );

  mOffsetUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << Qgis::RenderUnit::Millimeters << Qgis::RenderUnit::MetersInMapUnits << Qgis::RenderUnit::MapUnits << Qgis::RenderUnit::Pixels
                               << Qgis::RenderUnit::Points << Qgis::RenderUnit::Inches );
  mSpinOffset->setClearValue( 0.0 );
  connect( mSpinOffset, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, [ = ]
  {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );

  connect( mOffsetUnitWidget, &QgsUnitSelectionWidget::changed, this, [ = ]
  {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );
}

QgsAnnotationLineTextItemWidget::~QgsAnnotationLineTextItemWidget() = default;

QgsAnnotationItem *QgsAnnotationLineTextItemWidget::createItem()
{
  QgsAnnotationLineTextItem *newItem = mItem->clone();
  newItem->setFormat( mTextFormatWidget->format() );
  newItem->setText( mTextEdit->toPlainText() );

  newItem->setOffsetFromLine( mSpinOffset->value() );
  newItem->setOffsetFromLineUnit( mOffsetUnitWidget->unit() );
  newItem->setOffsetFromLineMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );

  mPropertiesWidget->updateItem( newItem );
  return newItem;
}

void QgsAnnotationLineTextItemWidget::updateItem( QgsAnnotationItem *item )
{
  if ( QgsAnnotationLineTextItem *lineTextItem = dynamic_cast< QgsAnnotationLineTextItem * >( item ) )
  {
    lineTextItem->setFormat( mTextFormatWidget->format() );
    lineTextItem->setText( mTextEdit->toPlainText() );

    lineTextItem->setOffsetFromLine( mSpinOffset->value() );
    lineTextItem->setOffsetFromLineUnit( mOffsetUnitWidget->unit() );
    lineTextItem->setOffsetFromLineMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );

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
  mTextEdit->selectAll();
  mTextEdit->setFocus();
}

bool QgsAnnotationLineTextItemWidget::setNewItem( QgsAnnotationItem *item )
{
  QgsAnnotationLineTextItem *textItem = dynamic_cast< QgsAnnotationLineTextItem * >( item );
  if ( !textItem )
    return false;

  mItem.reset( textItem->clone() );

  mBlockChangedSignal = true;
  mTextFormatWidget->setFormat( mItem->format() );
  mTextEdit->setPlainText( mItem->text() );
  mPropertiesWidget->setItem( mItem.get() );

  mSpinOffset->setValue( mItem->offsetFromLine() );
  mOffsetUnitWidget->setUnit( mItem->offsetFromLineUnit() );
  mOffsetUnitWidget->setMapUnitScale( mItem->offsetFromLineMapUnitScale() );

  mBlockChangedSignal = false;

  return true;
}

void QgsAnnotationLineTextItemWidget::mInsertExpressionButton_clicked()
{
  QString expression = QgsExpressionFinder::findAndSelectActiveExpression( mTextEdit );

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
      mTextEdit->insertPlainText( "[%" + expression + "%]" );
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

  mSizeModeCombo->addItem( tr( "Scale Dependent Size" ), QVariant::fromValue( Qgis::AnnotationPictureSizeMode::SpatialBounds ) );
  mSizeModeCombo->addItem( tr( "Fixed Size" ), QVariant::fromValue( Qgis::AnnotationPictureSizeMode::FixedSize ) );

  mSizeUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << Qgis::RenderUnit::Pixels << Qgis::RenderUnit::Millimeters
                             << Qgis::RenderUnit::Points << Qgis::RenderUnit::Inches << Qgis::RenderUnit::Percentage );

  mBackgroundSymbolButton->setSymbolType( Qgis::SymbolType::Fill );
  mBackgroundSymbolButton->setDialogTitle( tr( "Background" ) );
  mBackgroundSymbolButton->registerExpressionContextGenerator( this );
  mFrameSymbolButton->setSymbolType( Qgis::SymbolType::Fill );
  mFrameSymbolButton->setDialogTitle( tr( "Frame" ) );
  mFrameSymbolButton->registerExpressionContextGenerator( this );

  connect( mPropertiesWidget, &QgsAnnotationItemCommonPropertiesWidget::itemChanged, this, [ = ]
  {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );

  connect( mSizeModeCombo, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsAnnotationPictureItemWidget::sizeModeChanged );

  connect( mRadioSVG, &QRadioButton::toggled, this, &QgsAnnotationPictureItemWidget::modeChanged );
  connect( mRadioRaster, &QRadioButton::toggled, this, &QgsAnnotationPictureItemWidget::modeChanged );
  connect( mSourceLineEdit, &QgsPictureSourceLineEditBase::sourceChanged, this, [ = ]( const QString & source )
  {
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

  connect( mWidthSpinBox, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, &QgsAnnotationPictureItemWidget::setWidth );
  connect( mHeightSpinBox, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, &QgsAnnotationPictureItemWidget::setHeight );
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
  if ( QgsAnnotationPictureItem *pictureItem = dynamic_cast< QgsAnnotationPictureItem * >( item ) )
  {
    const bool svg = mRadioSVG->isChecked();
    const Qgis::PictureFormat newFormat = svg ? Qgis::PictureFormat::SVG : Qgis::PictureFormat::Raster;
    const QString path = mSourceLineEdit->source();
    pictureItem->setPath( newFormat, path );

    pictureItem->setSizeMode( mSizeModeCombo->currentData().value< Qgis::AnnotationPictureSizeMode >() );
    switch ( pictureItem->sizeMode() )
    {
      case Qgis::AnnotationPictureSizeMode::SpatialBounds:
        pictureItem->setLockAspectRatio( mLockAspectRatioCheck->isChecked() );
        break;
      case Qgis::AnnotationPictureSizeMode::FixedSize:
        pictureItem->setLockAspectRatio( mLockAspectRatio->isChecked() );
        break;
    }

    pictureItem->setFixedSize( QSizeF( mWidthSpinBox->value(), mHeightSpinBox->value() ) );
    pictureItem->setFixedSizeUnit( mSizeUnitWidget->unit() );

    pictureItem->setBackgroundEnabled( mBackgroundCheckbox->isChecked() );
    pictureItem->setFrameEnabled( mFrameCheckbox->isChecked() );
    pictureItem->setBackgroundSymbol( mBackgroundSymbolButton->clonedSymbol< QgsFillSymbol >() );
    pictureItem->setFrameSymbol( mFrameSymbolButton->clonedSymbol< QgsFillSymbol >() );

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
  QgsAnnotationPictureItem *pictureItem = dynamic_cast< QgsAnnotationPictureItem * >( item );
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
  mSizeModeCombo->setCurrentIndex( mSizeModeCombo->findData( QVariant::fromValue( pictureItem->sizeMode() ) ) );
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
  const Qgis::AnnotationPictureSizeMode mode = mSizeModeCombo->currentData().value< Qgis::AnnotationPictureSizeMode >();
  switch ( mode )
  {
    case Qgis::AnnotationPictureSizeMode::SpatialBounds:
      mSizeStackedWidget->setCurrentWidget( mPageSpatialBounds );
      break;
    case Qgis::AnnotationPictureSizeMode::FixedSize:
      mSizeStackedWidget->setCurrentWidget( mPageFixedSize );
      break;
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

