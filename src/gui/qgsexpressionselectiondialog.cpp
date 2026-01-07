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
#include "qgsexpressioncontextutils.h"
#include "qgsgeometry.h"
#include "qgsgui.h"
#include "qgsmapcanvas.h"
#include "qgsmessagebar.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"

#include "moc_qgsexpressionselectiondialog.cpp"

QgsExpressionSelectionDialog::QgsExpressionSelectionDialog( QgsVectorLayer *layer, const QString &startText, QWidget *parent )
  : QDialog( parent )
  , mLayer( layer )

{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  connect( mActionSelect, &QAction::triggered, this, &QgsExpressionSelectionDialog::mActionSelect_triggered );
  connect( mActionAddToSelection, &QAction::triggered, this, &QgsExpressionSelectionDialog::mActionAddToSelection_triggered );
  connect( mActionRemoveFromSelection, &QAction::triggered, this, &QgsExpressionSelectionDialog::mActionRemoveFromSelection_triggered );
  connect( mActionSelectIntersect, &QAction::triggered, this, &QgsExpressionSelectionDialog::mActionSelectIntersect_triggered );
  connect( mButtonZoomToFeatures, &QToolButton::clicked, this, &QgsExpressionSelectionDialog::mButtonZoomToFeatures_clicked );
  connect( mPbnClose, &QPushButton::clicked, this, &QgsExpressionSelectionDialog::mPbnClose_clicked );
  connect( mLayer, &QgsVectorLayer::willBeDeleted, this, &QgsExpressionSelectionDialog::close );

  setWindowTitle( tr( "%1 — Select by Expression" ).arg( layer->name() ) );

  mActionSelect->setIcon( QgsApplication::getThemeIcon( u"/mIconExpressionSelect.svg"_s ) );
  mActionAddToSelection->setIcon( QgsApplication::getThemeIcon( u"/mIconSelectAdd.svg"_s ) );
  mActionRemoveFromSelection->setIcon( QgsApplication::getThemeIcon( u"/mIconSelectRemove.svg"_s ) );
  mActionSelectIntersect->setIcon( QgsApplication::getThemeIcon( u"/mIconSelectIntersect.svg"_s ) );

  mButtonSelect->addAction( mActionSelect );
  mButtonSelect->addAction( mActionAddToSelection );
  mButtonSelect->addAction( mActionRemoveFromSelection );
  mButtonSelect->addAction( mActionSelectIntersect );
  mButtonSelect->setDefaultAction( mActionSelect );

  const QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );
  mExpressionBuilder->initWithLayer( layer, context, u"selection"_s );
  mExpressionBuilder->setExpressionText( startText );

  // by default, zoom to features is hidden, shown only if canvas is set
  mButtonZoomToFeatures->setVisible( false );

  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsExpressionSelectionDialog::showHelp );
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

void QgsExpressionSelectionDialog::mActionSelect_triggered()
{
  mLayer->selectByExpression( mExpressionBuilder->expressionText(), Qgis::SelectBehavior::SetSelection );
  pushSelectedFeaturesMessage();
  saveRecent();
}

void QgsExpressionSelectionDialog::mActionAddToSelection_triggered()
{
  mLayer->selectByExpression( mExpressionBuilder->expressionText(), Qgis::SelectBehavior::AddToSelection );
  pushSelectedFeaturesMessage();
  saveRecent();
}

void QgsExpressionSelectionDialog::mActionSelectIntersect_triggered()
{
  mLayer->selectByExpression( mExpressionBuilder->expressionText(), Qgis::SelectBehavior::IntersectSelection );
  pushSelectedFeaturesMessage();
  saveRecent();
}

void QgsExpressionSelectionDialog::mActionRemoveFromSelection_triggered()
{
  mLayer->selectByExpression( mExpressionBuilder->expressionText(), Qgis::SelectBehavior::RemoveFromSelection );
  pushSelectedFeaturesMessage();
  saveRecent();
}

void QgsExpressionSelectionDialog::pushSelectedFeaturesMessage()
{
  if ( !mMessageBar )
    return;

  const int count = mLayer->selectedFeatureCount();
  if ( count > 0 )
  {
    mMessageBar->pushMessage( QString(), tr( "%n matching feature(s) selected", "matching features", count ), Qgis::MessageLevel::Info );
  }
  else
  {
    mMessageBar->pushMessage( QString(), tr( "No matching features found" ), Qgis::MessageLevel::Info );
  }
}

void QgsExpressionSelectionDialog::mButtonZoomToFeatures_clicked()
{
  if ( mExpressionBuilder->expressionText().isEmpty() || !mMapCanvas )
    return;

  const QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );

  const QgsFeatureRequest request = QgsFeatureRequest().setFilterExpression( mExpressionBuilder->expressionText() ).setExpressionContext( context ).setNoAttributes();

  QgsFeatureIterator features = mLayer->getFeatures( request );

  QgsRectangle bbox;
  bbox.setNull();
  QgsFeature feat;
  int featureCount = 0;
  while ( features.nextFeature( feat ) )
  {
    const QgsGeometry geom = feat.geometry();
    if ( geom.isNull() || geom.constGet()->isEmpty() )
      continue;

    const QgsRectangle r = mMapCanvas->mapSettings().layerExtentToOutputExtent( mLayer, geom.boundingBox() );
    bbox.combineExtentWith( r );
    featureCount++;
  }
  features.close();

  if ( featureCount > 0 )
  {
    mMapCanvas->zoomToFeatureExtent( bbox );
    if ( mMessageBar )
    {
      mMessageBar->pushMessage( QString(), tr( "Zoomed to %n matching feature(s)", "number of matching features", featureCount ), Qgis::MessageLevel::Info );
    }
  }
  else if ( mMessageBar )
  {
    mMessageBar->pushMessage( QString(), tr( "No matching features found" ), Qgis::MessageLevel::Info );
  }
  saveRecent();
}

void QgsExpressionSelectionDialog::closeEvent( QCloseEvent *closeEvent )
{
  QDialog::closeEvent( closeEvent );
}

void QgsExpressionSelectionDialog::mPbnClose_clicked()
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
  mExpressionBuilder->expressionTree()->saveToRecent( mExpressionBuilder->expressionText(), u"selection"_s );
}

void QgsExpressionSelectionDialog::showHelp()
{
  QgsHelp::openHelp( u"introduction/general_tools.html#automatic-selection"_s );
}
