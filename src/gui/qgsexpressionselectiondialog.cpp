/***************************************************************************
    qgisexpressionselectiondialog.cpp
     --------------------------------------
    Date                 : 24.1.2013
    Copyright            : (C) 2013 by Matthias kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexpressionselectiondialog.h"

#include "qgsapplication.h"
#include "qgsexpression.h"
#include "qgsgeometry.h"
#include "qgsmessagebar.h"
#include "qgsvectorlayer.h"
#include "qgssettings.h"


QgsExpressionSelectionDialog::QgsExpressionSelectionDialog( QgsVectorLayer *layer, const QString &startText, QWidget *parent )
  : QDialog( parent )
  , mLayer( layer )
  , mMessageBar( nullptr )
  , mMapCanvas( nullptr )
{
  setupUi( this );

  setWindowTitle( QStringLiteral( "Select by Expression - %1" ).arg( layer->name() ) );

  mActionSelect->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconExpressionSelect.svg" ) ) );
  mActionAddToSelection->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconSelectAdd.svg" ) ) );
  mActionRemoveFromSelection->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconSelectRemove.svg" ) ) );
  mActionSelectIntersect->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconSelectIntersect.svg" ) ) );

  mButtonSelect->addAction( mActionSelect );
  mButtonSelect->addAction( mActionAddToSelection );
  mButtonSelect->addAction( mActionRemoveFromSelection );
  mButtonSelect->addAction( mActionSelectIntersect );
  mButtonSelect->setDefaultAction( mActionSelect );

  mExpressionBuilder->setLayer( layer );
  mExpressionBuilder->setExpressionText( startText );
  mExpressionBuilder->loadFieldNames();
  mExpressionBuilder->loadRecent( QStringLiteral( "Selection" ) );

  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );
  mExpressionBuilder->setExpressionContext( context );

  // by default, zoom to features is hidden, shown only if canvas is set
  mButtonZoomToFeatures->setVisible( false );

  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/ExpressionSelectionDialog/geometry" ) ).toByteArray() );
}

QgsExpressionBuilderWidget *QgsExpressionSelectionDialog::expressionBuilder()
{
  return mExpressionBuilder;
}

void QgsExpressionSelectionDialog::setExpressionText( const QString &text )
{
  mExpressionBuilder->setExpressionText( text );
}

QString QgsExpressionSelectionDialog::expressionText()
{
  return mExpressionBuilder->expressionText();
}

void QgsExpressionSelectionDialog::setGeomCalculator( const QgsDistanceArea &da )
{
  // Store in child widget only.
  mExpressionBuilder->setGeomCalculator( da );
}

void QgsExpressionSelectionDialog::setMessageBar( QgsMessageBar *messageBar )
{
  mMessageBar = messageBar;
}

void QgsExpressionSelectionDialog::setMapCanvas( QgsMapCanvas *canvas )
{
  mMapCanvas = canvas;
  mButtonZoomToFeatures->setVisible( true );
}

void QgsExpressionSelectionDialog::on_mActionSelect_triggered()
{
  mLayer->selectByExpression( mExpressionBuilder->expressionText(),
                              QgsVectorLayer::SetSelection );
  saveRecent();
}

void QgsExpressionSelectionDialog::on_mActionAddToSelection_triggered()
{
  mLayer->selectByExpression( mExpressionBuilder->expressionText(),
                              QgsVectorLayer::AddToSelection );
  saveRecent();
}

void QgsExpressionSelectionDialog::on_mActionSelectIntersect_triggered()
{
  mLayer->selectByExpression( mExpressionBuilder->expressionText(),
                              QgsVectorLayer::IntersectSelection );
  saveRecent();
}

void QgsExpressionSelectionDialog::on_mActionRemoveFromSelection_triggered()
{
  mLayer->selectByExpression( mExpressionBuilder->expressionText(),
                              QgsVectorLayer::RemoveFromSelection );
  saveRecent();
}

void QgsExpressionSelectionDialog::on_mButtonZoomToFeatures_clicked()
{
  if ( mExpressionBuilder->expressionText().isEmpty() || !mMapCanvas )
    return;

  QgsFeatureIds ids;

  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );

  QgsFeatureRequest request = QgsFeatureRequest().setFilterExpression( mExpressionBuilder->expressionText() )
                              .setExpressionContext( context )
                              .setSubsetOfAttributes( QgsAttributeList() );

  QgsFeatureIterator features = mLayer->getFeatures( request );

  QgsRectangle bbox;
  bbox.setMinimal();
  QgsFeature feat;
  int featureCount = 0;
  while ( features.nextFeature( feat ) )
  {
    QgsGeometry geom = feat.geometry();
    if ( geom.isNull() || geom.geometry()->isEmpty() )
      continue;

    QgsRectangle r = mMapCanvas->mapSettings().layerExtentToOutputExtent( mLayer, geom.boundingBox() );
    bbox.combineExtentWith( r );
    featureCount++;
  }
  features.close();

  QgsSettings settings;
  int timeout = settings.value( QStringLiteral( "qgis/messageTimeout" ), 5 ).toInt();
  if ( featureCount > 0 )
  {
    mMapCanvas->zoomToFeatureExtent( bbox );
    if ( mMessageBar )
    {
      mMessageBar->pushMessage( QString(),
                                tr( "Zoomed to %n matching feature(s)", "number of matching features", featureCount ),
                                QgsMessageBar::INFO,
                                timeout );
    }
  }
  else if ( mMessageBar )
  {
    mMessageBar->pushMessage( QString(),
                              tr( "No matching features found" ),
                              QgsMessageBar::INFO,
                              timeout );
  }
  saveRecent();
}

void QgsExpressionSelectionDialog::closeEvent( QCloseEvent *closeEvent )
{
  QDialog::closeEvent( closeEvent );

  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/ExpressionSelectionDialog/geometry" ), saveGeometry() );
}

void QgsExpressionSelectionDialog::on_mPbnClose_clicked()
{
  close();
}

void QgsExpressionSelectionDialog::done( int r )
{
  QDialog::done( r );
  close();
}

void QgsExpressionSelectionDialog::saveRecent()
{
  mExpressionBuilder->saveToRecent( QStringLiteral( "Selection" ) );
}
