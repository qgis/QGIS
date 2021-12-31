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

#include <QLayout>

#include "qgsselectbyformdialog.h"
#include "qgsattributeform.h"
#include "qgsmapcanvas.h"
#include "qgssettings.h"
#include "qgsmessagebar.h"
#include "qgsexpressioncontextutils.h"
#include "qgsgui.h"
#include "qgsmapcanvasutils.h"

QgsSelectByFormDialog::QgsSelectByFormDialog( QgsVectorLayer *layer, const QgsAttributeEditorContext &context, QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mLayer( layer )

{
  QgsAttributeEditorContext dlgContext = context;
  dlgContext.setFormMode( QgsAttributeEditorContext::StandaloneDialog );
  dlgContext.setAllowCustomUi( false );

  mForm = new QgsAttributeForm( layer, QgsFeature(), dlgContext, this );
  mForm->setMode( QgsAttributeEditorContext::SearchMode );

  QVBoxLayout *vLayout = new QVBoxLayout();
  vLayout->setContentsMargins( 0, 0, 0, 0 );
  setLayout( vLayout );

  vLayout->addWidget( mForm );

  connect( mForm, &QgsAttributeForm::closed, this, &QWidget::close );

  QgsGui::enableAutoGeometryRestore( this );

  setWindowTitle( tr( "%1 — Select Features" ).arg( layer->name() ) );

  connect( mLayer, &QgsVectorLayer::willBeDeleted, this, &QgsSelectByFormDialog::close );
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
  connect( mForm, &QgsAttributeForm::openFilteredFeaturesAttributeTable, this, &QgsSelectByFormDialog::showFilteredFeaturesAttributeTable );
}

void QgsSelectByFormDialog::zoomToFeatures( const QString &filter )
{
  const long featureCount = QgsMapCanvasUtils::zoomToMatchingFeatures( mMapCanvas, mLayer, filter );
  if ( featureCount > 0 )
  {
    if ( mMessageBar )
    {
      mMessageBar->pushMessage( QString(),
                                tr( "Zoomed to %n matching feature(s)", "number of matching features", featureCount ),
                                Qgis::MessageLevel::Info );
    }
  }
  else if ( mMessageBar )
  {
    mMessageBar->pushMessage( QString(),
                              tr( "No matching features found" ),
                              Qgis::MessageLevel::Info );
  }
}

void QgsSelectByFormDialog::flashFeatures( const QString &filter )
{
  const long featureCount = QgsMapCanvasUtils::flashMatchingFeatures( mMapCanvas, mLayer, filter );
  if ( featureCount == 0 && mMessageBar )
  {
    mMessageBar->pushMessage( QString(),
                              tr( "No matching features found" ),
                              Qgis::MessageLevel::Info );
  }
}

void QgsSelectByFormDialog::openFeaturesAttributeTable( const QString &filter )
{
  Q_ASSERT( mLayer );
  QgsFeatureIterator it = mLayer->getFeatures( filter );
  QgsFeature f;
  if ( it.isValid() && it.nextFeature( f ) )
  {
    emit showFilteredFeaturesAttributeTable( filter );
  }
  else
  {
    if ( mMessageBar )
    {
      mMessageBar->pushMessage( QString(),
                                tr( "No matching features found" ),
                                Qgis::MessageLevel::Info );
    }
  }
}
