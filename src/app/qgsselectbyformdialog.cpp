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
#include <QLayout>
#include <QSettings>

QgsSelectByFormDialog::QgsSelectByFormDialog( QgsVectorLayer* layer, const QgsAttributeEditorContext& context, QWidget* parent, Qt::WindowFlags fl )
    : QDialog( parent, fl )
    , mLayer( layer )
    , mMessageBar( nullptr )
{
  QgsAttributeEditorContext dlgContext = context;
  dlgContext.setFormMode( QgsAttributeEditorContext::StandaloneDialog );
  dlgContext.setAllowCustomUi( false );

  mForm = new QgsAttributeForm( layer, QgsFeature(), dlgContext, this );
  mForm->setMode( QgsAttributeForm::SearchMode );

  QVBoxLayout* vLayout = new QVBoxLayout();
  vLayout->setMargin( 0 );
  vLayout->setContentsMargins( 0, 0, 0, 0 );
  setLayout( vLayout );

  vLayout->addWidget( mForm );

  connect( mForm, SIGNAL( closed() ), this, SLOT( close() ) );

  QSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "/Windows/SelectByForm/geometry" ) ).toByteArray() );

  setWindowTitle( tr( "Select features by value" ) );
}

QgsSelectByFormDialog::~QgsSelectByFormDialog()
{
  QSettings settings;
  settings.setValue( QStringLiteral( "/Windows/SelectByForm/geometry" ), saveGeometry() );
}

void QgsSelectByFormDialog::setMessageBar( QgsMessageBar* messageBar )
{
  mMessageBar = messageBar;
  mForm->setMessageBar( messageBar );
}

void QgsSelectByFormDialog::setMapCanvas( QgsMapCanvas* canvas )
{
  mMapCanvas = canvas;
  connect( mForm, &QgsAttributeForm::zoomToFeatures, this, &QgsSelectByFormDialog::zoomToFeatures );
}

void QgsSelectByFormDialog::zoomToFeatures( const QString& filter )
{
  QgsFeatureIds ids;

  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::layerScope( mLayer );

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
    if ( geom.isEmpty() || geom.geometry()->isEmpty() )
      continue;

    QgsRectangle r = mMapCanvas->mapSettings().layerExtentToOutputExtent( mLayer, geom.boundingBox() );
    bbox.combineExtentWith( r );
    featureCount++;
  }
  features.close();

  QSettings settings;
  int timeout = settings.value( QStringLiteral( "/qgis/messageTimeout" ), 5 ).toInt();
  if ( featureCount > 0 )
  {
    mMapCanvas->zoomToFeatureExtent( bbox );
    if ( mMessageBar )
    {
      mMessageBar->pushMessage( QString(),
                                tr( "Zoomed to %1 matching %2" ).arg( featureCount )
                                .arg( featureCount == 1 ? tr( "feature" ) : tr( "features" ) ),
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
