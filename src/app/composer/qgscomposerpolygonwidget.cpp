/***************************************************************************
                         qgscomposerpolygonwidget.cpp
    begin                : March 2016
    copyright            : (C) 2016 Paul Blottiere, Oslandia
    email                : paul dot blottiere at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerpolygonwidget.h"
#include "qgscomposerpolygon.h"
#include "qgscomposeritemwidget.h"
#include "qgssymbolselectordialog.h"
#include "qgsstyle.h"
#include "qgssymbollayerutils.h"

QgsComposerPolygonWidget::QgsComposerPolygonWidget( QgsComposerPolygon* composerPolygon ):
    QgsComposerItemBaseWidget( nullptr, composerPolygon )
    , mComposerPolygon( composerPolygon )
{
  setupUi( this );
  setPanelTitle( tr( "Polygon properties" ) );

  //add widget for general composer item properties
  QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, composerPolygon );

  //shapes don't use background or frame, since the symbol style is set through a QgsSymbolSelectorWidget
  itemPropertiesWidget->showBackgroundGroup( false );
  itemPropertiesWidget->showFrameGroup( false );
  mainLayout->addWidget( itemPropertiesWidget );

  // update style icon
  updatePolygonStyle();

  if ( mComposerPolygon )
  {
    connect( mComposerPolygon, SIGNAL( itemChanged() ), this, SLOT( setGuiElementValues() ) );
  }
}

QgsComposerPolygonWidget::~QgsComposerPolygonWidget()
{
}

void QgsComposerPolygonWidget::on_mPolygonStyleButton_clicked()
{
  if ( !mComposerPolygon )
  {
    return;
  }

  // use the atlas coverage layer, if any
  QgsVectorLayer* coverageLayer = atlasCoverageLayer();

  QgsFillSymbol* newSymbol = mComposerPolygon->polygonStyleSymbol()->clone();
  QgsExpressionContext context = mComposerPolygon->createExpressionContext();

  QgsSymbolSelectorWidget* d = new QgsSymbolSelectorWidget( newSymbol, QgsStyle::defaultStyle(), coverageLayer, nullptr );
  QgsSymbolWidgetContext symbolContext;
  symbolContext.setExpressionContext( &context );
  d->setContext( symbolContext );

  connect( d, SIGNAL( widgetChanged() ), this, SLOT( updateStyleFromWidget() ) );
  connect( d, SIGNAL( panelAccepted( QgsPanelWidget* ) ), this, SLOT( cleanUpStyleSelector( QgsPanelWidget* ) ) );
  openPanel( d );
  mComposerPolygon->beginCommand( tr( "Polygon style changed" ) );
}

void QgsComposerPolygonWidget::setGuiElementValues()
{
  if ( !mComposerPolygon )
  {
    return;
  }

  updatePolygonStyle();
}

void QgsComposerPolygonWidget::updateStyleFromWidget()
{
  QgsSymbolSelectorWidget* w = qobject_cast<QgsSymbolSelectorWidget*>( sender() );
  mComposerPolygon->setPolygonStyleSymbol( dynamic_cast< QgsFillSymbol* >( w->symbol() ) );
  mComposerPolygon->update();
}

void QgsComposerPolygonWidget::cleanUpStyleSelector( QgsPanelWidget* container )
{
  QgsSymbolSelectorWidget* w = qobject_cast<QgsSymbolSelectorWidget*>( container );
  if ( !w )
    return;

  delete w->symbol();
  updatePolygonStyle();
  mComposerPolygon->endCommand();
}

void QgsComposerPolygonWidget::updatePolygonStyle()
{
  if ( mComposerPolygon )
  {
    QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( mComposerPolygon->polygonStyleSymbol(), mPolygonStyleButton->iconSize() );
    mPolygonStyleButton->setIcon( icon );
  }
}
