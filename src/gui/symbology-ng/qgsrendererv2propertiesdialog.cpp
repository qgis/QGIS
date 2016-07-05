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
#include "qgsinvertedpolygonrendererwidget.h"
#include "qgsheatmaprendererwidget.h"
#include "qgs25drendererwidget.h"
#include "qgsnullsymbolrendererwidget.h"
#include "qgspanelwidget.h"

#include "qgsorderbydialog.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"

#include <QKeyEvent>
#include <QMessageBox>

static bool _initRenderer( const QString& name, QgsRendererV2WidgetFunc f, const QString& iconName = QString() )
{
  QgsRendererV2Registry* reg = QgsRendererV2Registry::instance();
  QgsRendererV2AbstractMetadata* am = reg->rendererMetadata( name );
  if ( !am )
    return false;
  QgsRendererV2Metadata* m = dynamic_cast<QgsRendererV2Metadata*>( am );
  if ( !m )
    return false;

  m->setWidgetFunction( f );

  if ( !iconName.isEmpty() )
  {
    QString iconPath = QgsApplication::defaultThemePath() + iconName;
    QPixmap pix;
    if ( pix.load( iconPath ) )
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

  _initRenderer( "singleSymbol", QgsSingleSymbolRendererV2Widget::create, "rendererSingleSymbol.svg" );
  _initRenderer( "categorizedSymbol", QgsCategorizedSymbolRendererV2Widget::create, "rendererCategorizedSymbol.svg" );
  _initRenderer( "graduatedSymbol", QgsGraduatedSymbolRendererV2Widget::create, "rendererGraduatedSymbol.svg" );
  _initRenderer( "RuleRenderer", QgsRuleBasedRendererV2Widget::create, "rendererRuleBasedSymbol.svg" );
  _initRenderer( "pointDisplacement", QgsPointDisplacementRendererWidget::create, "rendererPointDisplacementSymbol.svg" );
  _initRenderer( "invertedPolygonRenderer", QgsInvertedPolygonRendererWidget::create, "rendererInvertedSymbol.svg" );
  _initRenderer( "heatmapRenderer", QgsHeatmapRendererWidget::create, "rendererHeatmapSymbol.svg" );
  _initRenderer( "25dRenderer", Qgs25DRendererWidget::create, "renderer25dSymbol.svg" );
  _initRenderer( "nullSymbol", QgsNullSymbolRendererWidget::create, "rendererNullSymbol.svg" );
  initialized = true;
}

QgsRendererV2PropertiesDialog::QgsRendererV2PropertiesDialog( QgsVectorLayer* layer, QgsStyleV2* style, bool embedded, QWidget* parent )
    : QDialog( parent )
    , mLayer( layer )
    , mStyle( style )
    , mActiveWidget( nullptr )
    , mPaintEffect( nullptr )
    , mMapCanvas( nullptr )
{
  setupUi( this );
  mLayerRenderingGroupBox->setSettingGroup( "layerRenderingGroupBox" );

  // can be embedded in vector layer properties
  if ( embedded )
  {
    buttonBox->hide();
    layout()->setContentsMargins( 0, 0, 0, 0 );
  }

  // initialize registry's widget functions
  _initRendererWidgetFunctions();

  QgsRendererV2Registry* reg = QgsRendererV2Registry::instance();
  QStringList renderers = reg->renderersList( mLayer );
  Q_FOREACH ( const QString& name, renderers )
  {
    QgsRendererV2AbstractMetadata* m = reg->rendererMetadata( name );
    cboRenderers->addItem( m->icon(), m->visibleName(), name );
  }

  cboRenderers->setCurrentIndex( -1 ); // set no current renderer

  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( onOK() ) );

  // connect layer transparency slider and spin box
  connect( mLayerTransparencySlider, SIGNAL( valueChanged( int ) ), mLayerTransparencySpnBx, SLOT( setValue( int ) ) );
  connect( mLayerTransparencySpnBx, SIGNAL( valueChanged( int ) ), mLayerTransparencySlider, SLOT( setValue( int ) ) );

  connect( cboRenderers, SIGNAL( currentIndexChanged( int ) ), this, SLOT( rendererChanged() ) );
  connect( checkboxEnableOrderBy, SIGNAL( toggled( bool ) ), btnOrderBy, SLOT( setEnabled( bool ) ) );
  connect( btnOrderBy, SIGNAL( clicked( bool ) ), this, SLOT( showOrderByDialog() ) );

  syncToLayer();

  QList<QWidget*> widgets;
  widgets << mLayerTransparencySpnBx
  << cboRenderers
  << checkboxEnableOrderBy
  << mBlendModeComboBox
  << mFeatureBlendComboBox
  << mEffectWidget;

  connectValueChanged( widgets, SIGNAL( widgetChanged() ) );
  connect( mEffectWidget, SIGNAL( showPanel( QgsPanelWidget* ) ), this, SLOT( openPanel( QgsPanelWidget* ) ) );
}

void QgsRendererV2PropertiesDialog::connectValueChanged( QList<QWidget *> widgets, const char *slot )
{
  Q_FOREACH ( QWidget* widget, widgets )
  {
    if ( QgsDataDefinedButton* w = qobject_cast<QgsDataDefinedButton*>( widget ) )
    {
      connect( w, SIGNAL( dataDefinedActivated( bool ) ), this, slot );
      connect( w, SIGNAL( dataDefinedChanged( QString ) ), this, slot );
    }
    else if ( QgsFieldExpressionWidget* w = qobject_cast<QgsFieldExpressionWidget*>( widget ) )
    {
      connect( w, SIGNAL( fieldChanged( QString ) ), this,  slot );
    }
    else if ( QComboBox* w =  qobject_cast<QComboBox*>( widget ) )
    {
      connect( w, SIGNAL( currentIndexChanged( int ) ), this, slot );
    }
    else if ( QSpinBox* w =  qobject_cast<QSpinBox*>( widget ) )
    {
      connect( w, SIGNAL( valueChanged( int ) ), this, slot );
    }
    else if ( QDoubleSpinBox* w =  qobject_cast<QDoubleSpinBox*>( widget ) )
    {
      connect( w , SIGNAL( valueChanged( double ) ), this, slot );
    }
    else if ( QgsColorButtonV2* w =  qobject_cast<QgsColorButtonV2*>( widget ) )
    {
      connect( w, SIGNAL( colorChanged( QColor ) ), this, slot );
    }
    else if ( QCheckBox* w =  qobject_cast<QCheckBox*>( widget ) )
    {
      connect( w, SIGNAL( toggled( bool ) ), this, slot );
    }
    else if ( QLineEdit* w =  qobject_cast<QLineEdit*>( widget ) )
    {
      connect( w, SIGNAL( textEdited( QString ) ), this, slot );
      connect( w, SIGNAL( textChanged( QString ) ), this, slot );
    }
    else if ( QgsEffectStackCompactWidget* w = qobject_cast<QgsEffectStackCompactWidget*>( widget ) )
    {
      connect( w, SIGNAL( changed() ), this, slot );
    }
  }
}

QgsRendererV2PropertiesDialog::~QgsRendererV2PropertiesDialog()
{
  delete mPaintEffect;
}

void QgsRendererV2PropertiesDialog::setMapCanvas( QgsMapCanvas* canvas )
{
  mMapCanvas = canvas;
  if ( mActiveWidget )
    mActiveWidget->setMapCanvas( mMapCanvas );
}

void QgsRendererV2PropertiesDialog::setDockMode( bool dockMode )
{
  mDockMode = dockMode;
  mEffectWidget->setDockMode( dockMode );
  if ( mActiveWidget )
    mActiveWidget->setDockMode( mDockMode );
}


void QgsRendererV2PropertiesDialog::rendererChanged()
{
  if ( cboRenderers->currentIndex() == -1 )
  {
    QgsDebugMsg( "No current item -- this should never happen!" );
    return;
  }

  QString rendererName = cboRenderers->itemData( cboRenderers->currentIndex() ).toString();

  //Retrieve the previous renderer: from the old active widget if possible, otherwise from the layer
  QgsFeatureRendererV2* oldRenderer;
  if ( mActiveWidget && mActiveWidget->renderer() )
  {
    oldRenderer = mActiveWidget->renderer()->clone();
  }
  else
  {
    oldRenderer = mLayer->rendererV2()->clone();
  }

  // get rid of old active widget (if any)
  if ( mActiveWidget )
  {
    stackedWidget->removeWidget( mActiveWidget );

    delete mActiveWidget;
    mActiveWidget = nullptr;
  }

  QgsRendererV2Widget* w = nullptr;
  QgsRendererV2AbstractMetadata* m = QgsRendererV2Registry::instance()->rendererMetadata( rendererName );
  if ( m )
    w = m->createRendererWidget( mLayer, mStyle, oldRenderer );
  delete oldRenderer;

  if ( w )
  {
    // instantiate the widget and set as active
    mActiveWidget = w;
    stackedWidget->addWidget( mActiveWidget );
    stackedWidget->setCurrentWidget( mActiveWidget );
    if ( mActiveWidget->renderer() )
    {
      if ( mMapCanvas )
        mActiveWidget->setMapCanvas( mMapCanvas );
      changeOrderBy( mActiveWidget->renderer()->orderBy(), mActiveWidget->renderer()->orderByEnabled() );
      connect( mActiveWidget, SIGNAL( layerVariablesChanged() ), this, SIGNAL( layerVariablesChanged() ) );
    }
    connect( mActiveWidget, SIGNAL( widgetChanged() ), this, SIGNAL( widgetChanged() ) );
    connect( mActiveWidget, SIGNAL( showPanel( QgsPanelWidget* ) ), this, SLOT( openPanel( QgsPanelWidget* ) ) );
    w->setDockMode( mDockMode );
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

  mActiveWidget->applyChanges();

  QgsFeatureRendererV2* renderer = mActiveWidget->renderer();
  if ( renderer )
  {
    renderer->setPaintEffect( mPaintEffect->clone() );
    // set the order by
    renderer->setOrderBy( mOrderBy );
    renderer->setOrderByEnabled( checkboxEnableOrderBy->isChecked() );

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

void QgsRendererV2PropertiesDialog::openPanel( QgsPanelWidget *panel )
{
  QgsDebugMsg( "Open panel!!!" );
  if ( mDockMode )
  {
    QgsDebugMsg( "DOCK MODE" );
    emit showPanel( panel );
  }
  else
  {
    QgsDebugMsg( "DIALOG MODE" );
    // Show the dialog version if no one is connected
    QDialog* dlg = new QDialog();
    QString key =  QString( "/UI/paneldialog/%1" ).arg( panel->panelTitle() );
    QSettings settings;
    dlg->restoreGeometry( settings.value( key ).toByteArray() );
    dlg->setWindowTitle( panel->panelTitle() );
    dlg->setLayout( new QVBoxLayout() );
    dlg->layout()->addWidget( panel );
    QDialogButtonBox* buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok );
    connect( buttonBox, SIGNAL( accepted() ), dlg, SLOT( accept() ) );
    dlg->layout()->addWidget( buttonBox );
    dlg->exec();
    settings.setValue( key, dlg->saveGeometry() );
    panel->acceptPanel();
  }
}

void QgsRendererV2PropertiesDialog::syncToLayer()
{
  // Blend mode
  mBlendModeComboBox->setBlendMode( mLayer->blendMode() );

  // Feature blend mode
  mFeatureBlendComboBox->setBlendMode( mLayer->featureBlendMode() );

  // Layer transparency
  mLayerTransparencySlider->setValue( mLayer->layerTransparency() );
  mLayerTransparencySpnBx->setValue( mLayer->layerTransparency() );

  //paint effect widget
  if ( mLayer->rendererV2() )
  {
    if ( mLayer->rendererV2()->paintEffect() )
    {
      mPaintEffect = mLayer->rendererV2()->paintEffect()->clone();
      mEffectWidget->setPaintEffect( mPaintEffect );
    }

    mOrderBy = mLayer->rendererV2()->orderBy();
  }

  // setup slot rendererChanged()
  //setup order by
  if ( mLayer->rendererV2() &&
       mLayer->rendererV2()->orderByEnabled() )
  {
    checkboxEnableOrderBy->setChecked( true );
  }
  else
  {
    btnOrderBy->setEnabled( false );
    checkboxEnableOrderBy->setChecked( false );
  }

  if ( mLayer->rendererV2() )
  {
    // set current renderer from layer
    QString rendererName = mLayer->rendererV2()->type();

    int rendererIdx = cboRenderers->findData( rendererName );
    cboRenderers->setCurrentIndex( rendererIdx );

    // no renderer found... this mustn't happen
    Q_ASSERT( rendererIdx != -1 && "there must be a renderer!" );
  }

}

void QgsRendererV2PropertiesDialog::showOrderByDialog()
{
  QgsOrderByDialog dlg( mLayer, this );

  dlg.setOrderBy( mOrderBy );
  if ( dlg.exec() )
  {
    mOrderBy = dlg.orderBy();
    emit widgetChanged();
  }
}

void QgsRendererV2PropertiesDialog::changeOrderBy( const QgsFeatureRequest::OrderBy& orderBy, bool orderByEnabled )
{
  mOrderBy = orderBy;
  checkboxEnableOrderBy->setChecked( orderByEnabled );
}

void QgsRendererV2PropertiesDialog::updateUIState( bool hidden )
{
  mLayerRenderingGroupBox->setHidden( hidden );
  cboRenderers->setHidden( hidden );
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
