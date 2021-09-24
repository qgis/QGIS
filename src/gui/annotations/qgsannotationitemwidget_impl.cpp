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
#include "qgsexpressionbuilderdialog.h"
#include "qgstextformatwidget.h"
#include "qgsapplication.h"
#include "qgsrecentstylehandler.h"

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
}

QgsAnnotationItem *QgsAnnotationPointTextItemWidget::createItem()
{
  QgsAnnotationPointTextItem *newItem = mItem->clone();
  newItem->setFormat( mTextFormatWidget->format() );
  newItem->setText( mTextEdit->toPlainText() );
  mPropertiesWidget->updateItem( newItem );
  return newItem;
}

void QgsAnnotationPointTextItemWidget::updateItem( QgsAnnotationItem *item )
{
  if ( QgsAnnotationPointTextItem *pointTextItem = dynamic_cast< QgsAnnotationPointTextItem * >( item ) )
  {
    pointTextItem->setFormat( mTextFormatWidget->format() );
    pointTextItem->setText( mTextEdit->toPlainText() );
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
  mPropertiesWidget->setItem( mItem.get() );
  mBlockChangedSignal = false;

  return true;
}

void QgsAnnotationPointTextItemWidget::mInsertExpressionButton_clicked()
{
  QString selText = mTextEdit->textCursor().selectedText();

  // html editor replaces newlines with Paragraph Separator characters - see https://github.com/qgis/QGIS/issues/27568
  selText = selText.replace( QChar( 0x2029 ), QChar( '\n' ) );

  // edit the selected expression if there's one
  if ( selText.startsWith( QLatin1String( "[%" ) ) && selText.endsWith( QLatin1String( "%]" ) ) )
    selText = selText.mid( 2, selText.size() - 4 );

  QgsExpressionContext expressionContext;
  if ( context().expressionContext() )
    expressionContext = *( context().expressionContext() );
  else
    expressionContext = QgsProject::instance()->createExpressionContext();

  QgsExpressionBuilderDialog exprDlg( nullptr, selText, this, QStringLiteral( "generic" ), expressionContext );

  exprDlg.setWindowTitle( tr( "Insert Expression" ) );
  if ( exprDlg.exec() == QDialog::Accepted )
  {
    QString expression = exprDlg.expressionText();
    if ( !expression.isEmpty() )
    {
      mTextEdit->insertPlainText( "[%" + expression + "%]" );
    }
  }
}

///@endcond PRIVATE

