/***************************************************************************
    qgsrendererv2propertiesdialog.cpp
    ---------------------
    begin                : December 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsrendererv2propertiesdialog.h"

#include "qgsrendererv2.h"
#include "qgsrendererv2registry.h"

#include "qgsrendererv2widget.h"
#include "qgssinglesymbolrendererv2widget.h"
#include "qgscategorizedsymbolrendererv2widget.h"
#include "qgsgraduatedsymbolrendererv2widget.h"
#include "qgsrulebasedrendererv2widget.h"
#include "qgspointdisplacementrendererwidget.h"

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"

#include <QKeyEvent>
#include <QMessageBox>

static bool _initRenderer( QString name, QgsRendererV2WidgetFunc f, QString iconName = QString() )
{
  QgsRendererV2Registry* reg = QgsRendererV2Registry::instance();
  QgsRendererV2AbstractMetadata* am = reg->rendererMetadata( name );
  if ( am == NULL )
    return false;
  QgsRendererV2Metadata* m = dynamic_cast<QgsRendererV2Metadata*>( am );
  if ( m == NULL )
    return false;

  m->setWidgetFunction( f );

  if ( !iconName.isEmpty() )
  {
    QString iconPath = QgsApplication::defaultThemePath() + iconName;
    QPixmap pix;
    if ( pix.load( iconPath, "png" ) )
      m->setIcon( pix );
  }

  QgsDebugMsg( "Set for " + name );
  return true;
}

static void _initRendererWidgetFunctions()
{
  static bool initialized = false;
  if ( initialized )
    return;

  _initRenderer( "singleSymbol", QgsSingleSymbolRendererV2Widget::create, "rendererSingleSymbol.png" );
  _initRenderer( "categorizedSymbol", QgsCategorizedSymbolRendererV2Widget::create, "rendererCategorizedSymbol.png" );
  _initRenderer( "graduatedSymbol", QgsGraduatedSymbolRendererV2Widget::create, "rendererGraduatedSymbol.png" );
  _initRenderer( "RuleRenderer", QgsRuleBasedRendererV2Widget::create );
  _initRenderer( "pointDisplacement", QgsPointDisplacementRendererWidget::create );
  initialized = true;
}

QgsRendererV2PropertiesDialog::QgsRendererV2PropertiesDialog( QgsVectorLayer* layer, QgsStyleV2* style, bool embedded )
    : mLayer( layer ), mStyle( style ), mActiveWidget( NULL )
{
  setupUi( this );

  // can be embedded in vector layer properties
  if ( embedded )
  {
    buttonBox->hide();
    layout()->setContentsMargins( 0, 0, 0, 0 );
  }

  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( onOK() ) );

  // initialize registry's widget functions
  _initRendererWidgetFunctions();

  // Blend mode
  mBlendModeComboBox->setBlendMode( mLayer->blendMode() );

  // Feature blend mode
  mFeatureBlendComboBox->setBlendMode( mLayer->featureBlendMode() );
  
  // Layer transparency
  mLayerTransparencySlider->setValue( mLayer->layerTransparency() );
  mLayerTransparencySpnBx->setValue( mLayer->layerTransparency() );

  // connect layer transparency slider and spin box
  connect( mLayerTransparencySlider, SIGNAL( valueChanged( int ) ), mLayerTransparencySpnBx, SLOT( setValue( int ) ) );
  connect( mLayerTransparencySpnBx, SIGNAL( valueChanged( int ) ), mLayerTransparencySlider, SLOT( setValue( int ) ) );    

  QPixmap pix;
  QgsRendererV2Registry* reg = QgsRendererV2Registry::instance();
  QStringList renderers = reg->renderersList();
  foreach ( QString name, renderers )
  {
    QgsRendererV2AbstractMetadata* m = reg->rendererMetadata( name );
    cboRenderers->addItem( m->icon(), m->visibleName(), name );
  }

  cboRenderers->setCurrentIndex( -1 ); // set no current renderer

  // setup slot rendererChanged()
  connect( cboRenderers, SIGNAL( currentIndexChanged( int ) ), this, SLOT( rendererChanged() ) );

  // set current renderer from layer
  QString rendererName = mLayer->rendererV2()->type();
  for ( int i = 0; i < cboRenderers->count(); i++ )
  {
    if ( cboRenderers->itemData( i ).toString() == rendererName )
    {
      cboRenderers->setCurrentIndex( i );
      return;
    }
  }

  // no renderer found... this mustn't happen
  Q_ASSERT( false && "there must be a renderer!" );

}


void QgsRendererV2PropertiesDialog::rendererChanged()
{

  if ( cboRenderers->currentIndex() == -1 )
  {
    QgsDebugMsg( "No current item -- this should never happen!" );
    return;
  }

  QString rendererName = cboRenderers->itemData( cboRenderers->currentIndex() ).toString();

  // get rid of old active widget (if any)
  if ( mActiveWidget )
  {
    stackedWidget->removeWidget( mActiveWidget );

    delete mActiveWidget;
    mActiveWidget = NULL;
  }

  QgsRendererV2Widget* w = NULL;
  QgsRendererV2AbstractMetadata* m = QgsRendererV2Registry::instance()->rendererMetadata( rendererName );
  if ( m != NULL )
    w = m->createRendererWidget( mLayer, mStyle, mLayer->rendererV2()->clone() );

  if ( w != NULL )
  {
    // instantiate the widget and set as active
    mActiveWidget = w;
    stackedWidget->addWidget( mActiveWidget );
    stackedWidget->setCurrentWidget( mActiveWidget );
  }
  else
  {
    // set default "no edit widget available" page
    stackedWidget->setCurrentWidget( pageNoWidget );
  }

}

void QgsRendererV2PropertiesDialog::apply()
{
  if ( !mActiveWidget || !mLayer )
  {
    return;
  }

  QgsFeatureRendererV2* renderer = mActiveWidget->renderer();
  if ( renderer )
  {
    mLayer->setRendererV2( renderer->clone() );
  }

  // set the blend modes for the layer
  mLayer->setBlendMode( mBlendModeComboBox->blendMode() );
  mLayer->setFeatureBlendMode( mFeatureBlendComboBox->blendMode() );
  
  // set transparency for the layer
  mLayer->setLayerTransparency( mLayerTransparencySlider->value() );
}

void QgsRendererV2PropertiesDialog::onOK()
{
  apply();
  accept();
}


void QgsRendererV2PropertiesDialog::keyPressEvent( QKeyEvent * e )
{
  // Ignore the ESC key to avoid close the dialog without the properties window
  if ( !isWindow() && e->key() == Qt::Key_Escape )
  {
    e->ignore();
  }
  else
  {
    QDialog::keyPressEvent( e );
  }
}
