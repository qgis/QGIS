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
#include "qgssymbolv2selectordialog.h"
#include "qgsstylev2.h"
#include "qgssymbollayerv2utils.h"

QgsComposerPolygonWidget::QgsComposerPolygonWidget( QgsComposerPolygon* composerPolygon ):
    QgsComposerItemBaseWidget( nullptr, composerPolygon )
    , mComposerPolygon( composerPolygon )
{
  setupUi( this );

  //add widget for general composer item properties
  QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, composerPolygon );

  //shapes don't use background or frame, since the symbol style is set through a QgsSymbolV2SelectorDialog
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

  QScopedPointer<QgsFillSymbolV2> newSymbol;
  newSymbol.reset( mComposerPolygon->polygonStyleSymbol()->clone() );

  QgsSymbolV2SelectorDialog d( newSymbol.data(), QgsStyleV2::defaultStyle(),
                               coverageLayer, this );
  d.setExpressionContext( mComposerPolygon->createExpressionContext() );

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
    QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( mComposerPolygon->polygonStyleSymbol(), mPolygonStyleButton->iconSize() );
    mPolygonStyleButton->setIcon( icon );
  }
}
