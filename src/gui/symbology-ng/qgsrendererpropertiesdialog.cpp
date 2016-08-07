/***************************************************************************
    qgsrendererpropertiesdialog.cpp
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
#include "qgsrendererpropertiesdialog.h"

#include "qgsrenderer.h"
#include "qgsrendererregistry.h"

#include "qgsrendererwidget.h"
#include "qgssinglesymbolrendererwidget.h"
#include "qgscategorizedsymbolrendererwidget.h"
#include "qgsgraduatedsymbolrendererwidget.h"
#include "qgsrulebasedrendererwidget.h"
#include "qgspointdisplacementrendererwidget.h"
#include "qgsinvertedpolygonrendererwidget.h"
#include "qgsheatmaprendererwidget.h"
#include "qgs25drendererwidget.h"
#include "qgsnullsymbolrendererwidget.h"
#include "qgspanelwidget.h"
#include "qgspainteffect.h"

#include "qgsorderbydialog.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"

#include <QKeyEvent>
#include <QMessageBox>

static bool _initRenderer( const QString& name, QgsRendererWidgetFunc f, const QString& iconName = QString() )
{
  QgsRendererRegistry* reg = QgsRendererRegistry::instance();
  QgsRendererAbstractMetadata* am = reg->rendererMetadata( name );
  if ( !am )
    return false;
  QgsRendererMetadata* m = dynamic_cast<QgsRendererMetadata*>( am );
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

  _initRenderer( "singleSymbol", QgsSingleSymbolRendererWidget::create, "rendererSingleSymbol.svg" );
  _initRenderer( "categorizedSymbol", QgsCategorizedSymbolRendererWidget::create, "rendererCategorizedSymbol.svg" );
  _initRenderer( "graduatedSymbol", QgsGraduatedSymbolRendererWidget::create, "rendererGraduatedSymbol.svg" );
  _initRenderer( "RuleRenderer", QgsRuleBasedRendererWidget::create, "rendererRuleBasedSymbol.svg" );
  _initRenderer( "pointDisplacement", QgsPointDisplacementRendererWidget::create, "rendererPointDisplacementSymbol.svg" );
  _initRenderer( "invertedPolygonRenderer", QgsInvertedPolygonRendererWidget::create, "rendererInvertedSymbol.svg" );
  _initRenderer( "heatmapRenderer", QgsHeatmapRendererWidget::create, "rendererHeatmapSymbol.svg" );
  _initRenderer( "25dRenderer", Qgs25DRendererWidget::create, "renderer25dSymbol.svg" );
  _initRenderer( "nullSymbol", QgsNullSymbolRendererWidget::create, "rendererNullSymbol.svg" );
  initialized = true;
}

QgsRendererPropertiesDialog::QgsRendererPropertiesDialog( QgsVectorLayer* layer, QgsStyle* style, bool embedded, QWidget* parent )
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

  QgsRendererRegistry* reg = QgsRendererRegistry::instance();
  QStringList renderers = reg->renderersList( mLayer );
  Q_FOREACH ( const QString& name, renderers )
  {
    QgsRendererAbstractMetadata* m = reg->rendererMetadata( name );
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

void QgsRendererPropertiesDialog::connectValueChanged( QList<QWidget *> widgets, const char *slot )
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
    else if ( QgsColorButton* w =  qobject_cast<QgsColorButton*>( widget ) )
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

QgsRendererPropertiesDialog::~QgsRendererPropertiesDialog()
{
  delete mPaintEffect;
}

void QgsRendererPropertiesDialog::setMapCanvas( QgsMapCanvas* canvas )
{
  mMapCanvas = canvas;
  if ( mActiveWidget )
    mActiveWidget->setMapCanvas( mMapCanvas );
}

void QgsRendererPropertiesDialog::setDockMode( bool dockMode )
{
  mDockMode = dockMode;
  mEffectWidget->setDockMode( dockMode );
  if ( mActiveWidget )
    mActiveWidget->setDockMode( mDockMode );
}


void QgsRendererPropertiesDialog::rendererChanged()
{
  if ( cboRenderers->currentIndex() == -1 )
  {
    QgsDebugMsg( "No current item -- this should never happen!" );
    return;
  }

  QString rendererName = cboRenderers->itemData( cboRenderers->currentIndex() ).toString();

  //Retrieve the previous renderer: from the old active widget if possible, otherwise from the layer
  QgsFeatureRenderer* oldRenderer;
  if ( mActiveWidget && mActiveWidget->renderer() )
  {
    oldRenderer = mActiveWidget->renderer()->clone();
  }
  else
  {
    oldRenderer = mLayer->renderer()->clone();
  }

  // get rid of old active widget (if any)
  if ( mActiveWidget )
  {
    stackedWidget->removeWidget( mActiveWidget );

    delete mActiveWidget;
    mActiveWidget = nullptr;
  }

  QgsRendererWidget* w = nullptr;
  QgsRendererAbstractMetadata* m = QgsRendererRegistry::instance()->rendererMetadata( rendererName );
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

void QgsRendererPropertiesDialog::apply()
{
  if ( !mActiveWidget || !mLayer )
  {
    return;
  }

  mActiveWidget->applyChanges();

  QgsFeatureRenderer* renderer = mActiveWidget->renderer();
  if ( renderer )
  {
    renderer->setPaintEffect( mPaintEffect->clone() );
    // set the order by
    renderer->setOrderBy( mOrderBy );
    renderer->setOrderByEnabled( checkboxEnableOrderBy->isChecked() );

    mLayer->setRenderer( renderer->clone() );
  }

  // set the blend modes for the layer
  mLayer->setBlendMode( mBlendModeComboBox->blendMode() );
  mLayer->setFeatureBlendMode( mFeatureBlendComboBox->blendMode() );

  // set transparency for the layer
  mLayer->setLayerTransparency( mLayerTransparencySlider->value() );
}

void QgsRendererPropertiesDialog::onOK()
{
  apply();
  accept();
}

void QgsRendererPropertiesDialog::openPanel( QgsPanelWidget *panel )
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

void QgsRendererPropertiesDialog::syncToLayer()
{
  // Blend mode
  mBlendModeComboBox->setBlendMode( mLayer->blendMode() );

  // Feature blend mode
  mFeatureBlendComboBox->setBlendMode( mLayer->featureBlendMode() );

  // Layer transparency
  mLayerTransparencySlider->setValue( mLayer->layerTransparency() );
  mLayerTransparencySpnBx->setValue( mLayer->layerTransparency() );

  //paint effect widget
  if ( mLayer->renderer() )
  {
    if ( mLayer->renderer()->paintEffect() )
    {
      mPaintEffect = mLayer->renderer()->paintEffect()->clone();
      mEffectWidget->setPaintEffect( mPaintEffect );
    }

    mOrderBy = mLayer->renderer()->orderBy();
  }

  // setup slot rendererChanged()
  //setup order by
  if ( mLayer->renderer() &&
       mLayer->renderer()->orderByEnabled() )
  {
    checkboxEnableOrderBy->setChecked( true );
  }
  else
  {
    btnOrderBy->setEnabled( false );
    checkboxEnableOrderBy->setChecked( false );
  }

  if ( mLayer->renderer() )
  {
    // set current renderer from layer
    QString rendererName = mLayer->renderer()->type();

    int rendererIdx = cboRenderers->findData( rendererName );
    cboRenderers->setCurrentIndex( rendererIdx );

    // no renderer found... this mustn't happen
    Q_ASSERT( rendererIdx != -1 && "there must be a renderer!" );
  }

}

void QgsRendererPropertiesDialog::showOrderByDialog()
{
  QgsOrderByDialog dlg( mLayer, this );

  dlg.setOrderBy( mOrderBy );
  if ( dlg.exec() )
  {
    mOrderBy = dlg.orderBy();
    emit widgetChanged();
  }
}

void QgsRendererPropertiesDialog::changeOrderBy( const QgsFeatureRequest::OrderBy& orderBy, bool orderByEnabled )
{
  mOrderBy = orderBy;
  checkboxEnableOrderBy->setChecked( orderByEnabled );
}

void QgsRendererPropertiesDialog::updateUIState( bool hidden )
{
  mLayerRenderingGroupBox->setHidden( hidden );
  cboRenderers->setHidden( hidden );
}


void QgsRendererPropertiesDialog::keyPressEvent( QKeyEvent * e )
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
