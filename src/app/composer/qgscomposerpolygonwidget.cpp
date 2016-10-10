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

  //shapes don't use background or frame, since the symbol style is set through a QgsSymbolSelectorDialog
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

  QScopedPointer<QgsFillSymbol> newSymbol;
  newSymbol.reset( mComposerPolygon->polygonStyleSymbol()->clone() );

  QgsExpressionContext context = mComposerPolygon->createExpressionContext();
  QgsSymbolSelectorDialog d( newSymbol.data(), QgsStyle::defaultStyle(),
                             coverageLayer, this );
  QgsSymbolWidgetContext symbolContext;
  symbolContext.setExpressionContext( &context );
  d.setContext( symbolContext );

  if ( d.exec() == QDialog::Accepted )
  {
    mComposerPolygon->beginCommand( tr( "Polygon style changed" ) );
    mComposerPolygon->setPolygonStyleSymbol( newSymbol.data() );
    updatePolygonStyle();
    mComposerPolygon->endCommand();
  }
}

void QgsComposerPolygonWidget::setGuiElementValues()
{
  if ( !mComposerPolygon )
  {
    return;
  }

  updatePolygonStyle();
}

void QgsComposerPolygonWidget::updatePolygonStyle()
{
  if ( mComposerPolygon )
  {
    QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( mComposerPolygon->polygonStyleSymbol(), mPolygonStyleButton->iconSize() );
    mPolygonStyleButton->setIcon( icon );
  }
}
