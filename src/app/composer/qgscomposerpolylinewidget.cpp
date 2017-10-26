/***************************************************************************
                         qgscomposerpolylinewidget.cpp
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

#include "qgscomposerpolylinewidget.h"
#include "qgscomposerpolyline.h"
#include "qgscomposeritemwidget.h"
#include "qgssymbolselectordialog.h"
#include "qgsstyle.h"
#include "qgssymbollayerutils.h"

QgsComposerPolylineWidget::QgsComposerPolylineWidget( QgsComposerPolyline *composerPolyline ):
  QgsComposerItemBaseWidget( nullptr, composerPolyline )
  , mComposerPolyline( composerPolyline )
{
  setupUi( this );
  connect( mLineStyleButton, &QPushButton::clicked, this, &QgsComposerPolylineWidget::mLineStyleButton_clicked );
  setPanelTitle( tr( "Polyline properties" ) );

  //add widget for general composer item properties
  QgsComposerItemWidget *itemPropertiesWidget = new QgsComposerItemWidget( this, composerPolyline );

  itemPropertiesWidget->showBackgroundGroup( false );
  itemPropertiesWidget->showFrameGroup( false );
  mainLayout->addWidget( itemPropertiesWidget );

  // update style icon
  updatePolylineStyle();

  if ( mComposerPolyline )
    connect( mComposerPolyline, &QgsComposerObject::itemChanged, this, &QgsComposerPolylineWidget::setGuiElementValues );
}

void QgsComposerPolylineWidget::mLineStyleButton_clicked()
{
  if ( !mComposerPolyline )
    return;

  // use the atlas coverage layer, if any
  QgsVectorLayer *coverageLayer = atlasCoverageLayer();

  QgsLineSymbol *newSymbol = mComposerPolyline->polylineStyleSymbol()->clone();
  QgsExpressionContext context = mComposerPolyline->createExpressionContext();

  QgsSymbolSelectorWidget *d = new QgsSymbolSelectorWidget( newSymbol, QgsStyle::defaultStyle(), coverageLayer, nullptr );
  QgsSymbolWidgetContext symbolContext;
  symbolContext.setExpressionContext( &context );
  d->setContext( symbolContext );

  connect( d, &QgsPanelWidget::widgetChanged, this, &QgsComposerPolylineWidget::updateStyleFromWidget );
  connect( d, &QgsPanelWidget::panelAccepted, this, &QgsComposerPolylineWidget::cleanUpStyleSelector );
  openPanel( d );
  mComposerPolyline->beginCommand( tr( "Polyline style changed" ) );
}

void QgsComposerPolylineWidget::setGuiElementValues()
{
  if ( !mComposerPolyline )
    return;

  updatePolylineStyle();
}

void QgsComposerPolylineWidget::updateStyleFromWidget()
{
  if ( QgsSymbolSelectorWidget *w = qobject_cast<QgsSymbolSelectorWidget *>( sender() ) )
  {
    mComposerPolyline->setPolylineStyleSymbol( static_cast< QgsLineSymbol * >( w->symbol() ) );
    mComposerPolyline->update();
  }
}

void QgsComposerPolylineWidget::cleanUpStyleSelector( QgsPanelWidget *container )
{
  QgsSymbolSelectorWidget *w = qobject_cast<QgsSymbolSelectorWidget *>( container );
  if ( !w )
    return;

  delete w->symbol();
  updatePolylineStyle();
  mComposerPolyline->endCommand();
}

void QgsComposerPolylineWidget::updatePolylineStyle()
{
  if ( mComposerPolyline )
  {
    QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( mComposerPolyline->polylineStyleSymbol(), mLineStyleButton->iconSize() );
    mLineStyleButton->setIcon( icon );
  }
}
