/***************************************************************************
    qgsselectbyformdialog.cpp
     ------------------------
    Date                 : June 2016
    Copyright            : (C) 2016 nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsselectbyformdialog.h"
#include "qgsattributeform.h"
#include "qgsmapcanvas.h"
#include "qgssettings.h"

#include <QLayout>

QgsSelectByFormDialog::QgsSelectByFormDialog( QgsVectorLayer *layer, const QgsAttributeEditorContext &context, QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mLayer( layer )

{
  QgsAttributeEditorContext dlgContext = context;
  dlgContext.setFormMode( QgsAttributeEditorContext::StandaloneDialog );
  dlgContext.setAllowCustomUi( false );

  mForm = new QgsAttributeForm( layer, QgsFeature(), dlgContext, this );
  mForm->setMode( QgsAttributeForm::SearchMode );

  QVBoxLayout *vLayout = new QVBoxLayout();
  vLayout->setMargin( 0 );
  vLayout->setContentsMargins( 0, 0, 0, 0 );
  setLayout( vLayout );

  vLayout->addWidget( mForm );

  connect( mForm, &QgsAttributeForm::closed, this, &QWidget::close );

  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/SelectByForm/geometry" ) ).toByteArray() );

  setWindowTitle( tr( "Select Features by Value" ) );
}

QgsSelectByFormDialog::~QgsSelectByFormDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/SelectByForm/geometry" ), saveGeometry() );
}

void QgsSelectByFormDialog::setMessageBar( QgsMessageBar *messageBar )
{
  mMessageBar = messageBar;
  mForm->setMessageBar( messageBar );
}

void QgsSelectByFormDialog::setMapCanvas( QgsMapCanvas *canvas )
{
  mMapCanvas = canvas;
  connect( mForm, &QgsAttributeForm::zoomToFeatures, this, &QgsSelectByFormDialog::zoomToFeatures );
  connect( mForm, &QgsAttributeForm::flashFeatures, this, &QgsSelectByFormDialog::flashFeatures );
}

void QgsSelectByFormDialog::zoomToFeatures( const QString &filter )
{
  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );

  QgsFeatureRequest request = QgsFeatureRequest().setFilterExpression( filter )
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
    if ( geom.isNull() || geom.constGet()->isEmpty() )
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
}

void QgsSelectByFormDialog::flashFeatures( const QString &filter )
{
  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );

  QgsFeatureRequest request = QgsFeatureRequest().setFilterExpression( filter )
                              .setExpressionContext( context )
                              .setSubsetOfAttributes( QgsAttributeList() );

  QgsFeatureIterator features = mLayer->getFeatures( request );
  QgsFeature feat;
  QList< QgsGeometry > geoms;
  while ( features.nextFeature( feat ) )
  {
    if ( feat.hasGeometry() )
      geoms << feat.geometry();
  }

  QgsSettings settings;
  int timeout = settings.value( QStringLiteral( "qgis/messageTimeout" ), 5 ).toInt();
  if ( !geoms.empty() )
  {
    mMapCanvas->flashGeometries( geoms, mLayer->crs() );
  }
  else if ( mMessageBar )
  {
    mMessageBar->pushMessage( QString(),
                              tr( "No matching features found" ),
                              QgsMessageBar::INFO,
                              timeout );
  }
}
