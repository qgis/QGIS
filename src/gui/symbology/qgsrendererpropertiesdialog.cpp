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
#include "qgspointclusterrendererwidget.h"
#include "qgsinvertedpolygonrendererwidget.h"
#include "qgsmergedfeaturerendererwidget.h"
#include "qgsheatmaprendererwidget.h"
#include "qgs25drendererwidget.h"
#include "qgsnullsymbolrendererwidget.h"
#include "qgsembeddedsymbolrendererwidget.h"
#include "qgspanelwidget.h"
#include "qgspainteffect.h"
#include "qgsproject.h"
#include "qgsprojectutils.h"

#include "qgsorderbydialog.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"

#include <QKeyEvent>
#include <QMessageBox>

static bool _initRenderer( const QString &name, QgsRendererWidgetFunc f, const QString &iconName = QString() )
{
  QgsRendererRegistry *reg = QgsApplication::rendererRegistry();
  QgsRendererAbstractMetadata *am = reg->rendererMetadata( name );
  if ( !am )
    return false;
  QgsRendererMetadata *m = dynamic_cast<QgsRendererMetadata *>( am );
  if ( !m )
    return false;

  m->setWidgetFunction( f );

  if ( !iconName.isEmpty() )
  {
    m->setIcon( QgsApplication::getThemeIcon( iconName ) );
  }

  QgsDebugMsgLevel( "Set for " + name, 2 );
  return true;
}

static void _initRendererWidgetFunctions()
{
  static bool sInitialized = false;
  if ( sInitialized )
    return;

  _initRenderer( QStringLiteral( "singleSymbol" ), QgsSingleSymbolRendererWidget::create, QStringLiteral( "rendererSingleSymbol.svg" ) );
  _initRenderer( QStringLiteral( "categorizedSymbol" ), QgsCategorizedSymbolRendererWidget::create, QStringLiteral( "rendererCategorizedSymbol.svg" ) );
  _initRenderer( QStringLiteral( "graduatedSymbol" ), QgsGraduatedSymbolRendererWidget::create, QStringLiteral( "rendererGraduatedSymbol.svg" ) );
  _initRenderer( QStringLiteral( "RuleRenderer" ), QgsRuleBasedRendererWidget::create, QStringLiteral( "rendererRuleBasedSymbol.svg" ) );
  _initRenderer( QStringLiteral( "pointDisplacement" ), QgsPointDisplacementRendererWidget::create, QStringLiteral( "rendererPointDisplacementSymbol.svg" ) );
  _initRenderer( QStringLiteral( "pointCluster" ), QgsPointClusterRendererWidget::create, QStringLiteral( "rendererPointClusterSymbol.svg" ) );
  _initRenderer( QStringLiteral( "invertedPolygonRenderer" ), QgsInvertedPolygonRendererWidget::create, QStringLiteral( "rendererInvertedSymbol.svg" ) );
  _initRenderer( QStringLiteral( "mergedFeatureRenderer" ), QgsMergedFeatureRendererWidget::create, QStringLiteral( "rendererMergedFeatures.svg" ) );
  _initRenderer( QStringLiteral( "heatmapRenderer" ), QgsHeatmapRendererWidget::create, QStringLiteral( "rendererHeatmapSymbol.svg" ) );
  _initRenderer( QStringLiteral( "25dRenderer" ), Qgs25DRendererWidget::create, QStringLiteral( "renderer25dSymbol.svg" ) );
  _initRenderer( QStringLiteral( "nullSymbol" ), QgsNullSymbolRendererWidget::create, QStringLiteral( "rendererNullSymbol.svg" ) );
  _initRenderer( QStringLiteral( "embeddedSymbol" ), QgsEmbeddedSymbolRendererWidget::create );
  sInitialized = true;
}

QgsRendererPropertiesDialog::QgsRendererPropertiesDialog( QgsVectorLayer *layer, QgsStyle *style, bool embedded, QWidget *parent )
  : QDialog( parent )
  , mLayer( layer )
  , mStyle( style )

{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );
  mLayerRenderingGroupBox->setSettingGroup( QStringLiteral( "layerRenderingGroupBox" ) );

  // can be embedded in vector layer properties
  if ( embedded )
  {
    buttonBox->hide();
    layout()->setContentsMargins( 0, 0, 0, 0 );
  }

  // initialize registry's widget functions
  _initRendererWidgetFunctions();

  QgsRendererRegistry *reg = QgsApplication::rendererRegistry();
  const QStringList renderers = reg->renderersList( mLayer );
  const auto constRenderers = renderers;
  for ( const QString &name : constRenderers )
  {
    QgsRendererAbstractMetadata *m = reg->rendererMetadata( name );
    cboRenderers->addItem( m->icon(), m->visibleName(), name );
  }

  cboRenderers->setCurrentIndex( -1 ); // set no current renderer

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsRendererPropertiesDialog::onOK );

  // connect layer opacity slider and spin box
  connect( cboRenderers, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsRendererPropertiesDialog::rendererChanged );
  connect( checkboxEnableOrderBy, &QAbstractButton::toggled, btnOrderBy, &QWidget::setEnabled );
  connect( btnOrderBy, &QAbstractButton::clicked, this, &QgsRendererPropertiesDialog::showOrderByDialog );

  syncToLayer();

  QList<QWidget *> widgets;
  widgets << mOpacityWidget
          << cboRenderers
          << checkboxEnableOrderBy
          << mBlendModeComboBox
          << mFeatureBlendComboBox
          << mEffectWidget;

  connectValueChanged( widgets, SIGNAL( widgetChanged() ) );
  connect( mEffectWidget, &QgsPanelWidget::showPanel, this, &QgsRendererPropertiesDialog::openPanel );
}

void QgsRendererPropertiesDialog::connectValueChanged( const QList<QWidget *> &widgets, const char *slot )
{
  for ( QWidget *widget : widgets )
  {
    if ( QgsPropertyOverrideButton *w = qobject_cast<QgsPropertyOverrideButton *>( widget ) )
    {
      connect( w, SIGNAL( changed ), this, slot );
    }
    else if ( QgsFieldExpressionWidget *w = qobject_cast<QgsFieldExpressionWidget *>( widget ) )
    {
      connect( w, SIGNAL( fieldChanged( QString ) ), this,  slot );
    }
    else if ( QgsOpacityWidget *w = qobject_cast<QgsOpacityWidget *>( widget ) )
    {
      connect( w, SIGNAL( opacityChanged( double ) ), this,  slot );
    }
    else if ( QComboBox *w = qobject_cast<QComboBox *>( widget ) )
    {
      connect( w, SIGNAL( currentIndexChanged( int ) ), this, slot );
    }
    else if ( QSpinBox *w = qobject_cast<QSpinBox *>( widget ) )
    {
      connect( w, SIGNAL( valueChanged( int ) ), this, slot );
    }
    else if ( QDoubleSpinBox *w = qobject_cast<QDoubleSpinBox *>( widget ) )
    {
      connect( w, SIGNAL( valueChanged( double ) ), this, slot );
    }
    else if ( QgsColorButton *w = qobject_cast<QgsColorButton *>( widget ) )
    {
      connect( w, SIGNAL( colorChanged( QColor ) ), this, slot );
    }
    else if ( QCheckBox *w = qobject_cast<QCheckBox *>( widget ) )
    {
      connect( w, SIGNAL( toggled( bool ) ), this, slot );
    }
    else if ( QLineEdit *w = qobject_cast<QLineEdit *>( widget ) )
    {
      connect( w, SIGNAL( textEdited( QString ) ), this, slot );
      connect( w, SIGNAL( textChanged( QString ) ), this, slot );
    }
    else if ( QgsEffectStackCompactWidget *w = qobject_cast<QgsEffectStackCompactWidget *>( widget ) )
    {
      connect( w, SIGNAL( changed() ), this, slot );
    }
  }
}

QgsRendererPropertiesDialog::~QgsRendererPropertiesDialog()
{
  delete mPaintEffect;
}

void QgsRendererPropertiesDialog::setMapCanvas( QgsMapCanvas *canvas )
{
  mMapCanvas = canvas;
  if ( mActiveWidget )
  {
    QgsSymbolWidgetContext context;
    context.setMapCanvas( mMapCanvas );
    mActiveWidget->setContext( context );
  }
}

void QgsRendererPropertiesDialog::setContext( const QgsSymbolWidgetContext &context )
{
  mMapCanvas = context.mapCanvas();
  mMessageBar = context.messageBar();
  if ( mActiveWidget )
  {
    mActiveWidget->setContext( context );
  }
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
    QgsDebugMsg( QStringLiteral( "No current item -- this should never happen!" ) );
    return;
  }

  const QString rendererName = cboRenderers->currentData().toString();

  //Retrieve the previous renderer: from the old active widget if possible, otherwise from the layer
  QgsFeatureRenderer *oldRenderer = nullptr;
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

  QgsRendererWidget *w = nullptr;
  QgsRendererAbstractMetadata *m = QgsApplication::rendererRegistry()->rendererMetadata( rendererName );
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
      if ( mMapCanvas || mMessageBar )
      {
        QgsSymbolWidgetContext context;
        context.setMapCanvas( mMapCanvas );
        context.setMessageBar( mMessageBar );
        mActiveWidget->setContext( context );
      }
      changeOrderBy( mActiveWidget->renderer()->orderBy(), mActiveWidget->renderer()->orderByEnabled() );
      connect( mActiveWidget, &QgsRendererWidget::layerVariablesChanged, this, &QgsRendererPropertiesDialog::layerVariablesChanged );
    }
    connect( mActiveWidget, &QgsPanelWidget::widgetChanged, this, &QgsRendererPropertiesDialog::widgetChanged );
    connect( mActiveWidget, &QgsPanelWidget::showPanel, this, &QgsRendererPropertiesDialog::openPanel );
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

  QgsFeatureRenderer *renderer = mActiveWidget->renderer();
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

  // set opacity for the layer
  mLayer->setOpacity( mOpacityWidget->opacity() );
}

void QgsRendererPropertiesDialog::onOK()
{
  apply();
  accept();
}

void QgsRendererPropertiesDialog::openPanel( QgsPanelWidget *panel )
{
  if ( mDockMode )
  {
    emit showPanel( panel );
  }
  else
  {
    // Show the dialog version if no one is connected
    QDialog *dlg = new QDialog();
    const QString key = QStringLiteral( "/UI/paneldialog/%1" ).arg( panel->panelTitle() );
    QgsSettings settings;
    dlg->restoreGeometry( settings.value( key ).toByteArray() );
    dlg->setWindowTitle( panel->panelTitle() );
    dlg->setLayout( new QVBoxLayout() );
    dlg->layout()->addWidget( panel );
    QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok );
    connect( buttonBox, &QDialogButtonBox::accepted, dlg, &QDialog::accept );
    dlg->layout()->addWidget( buttonBox );
    dlg->exec();
    settings.setValue( key, dlg->saveGeometry() );
    panel->acceptPanel();
  }
}

void QgsRendererPropertiesDialog::syncToLayer()
{
  mBlendModeComboBox->setShowClippingModes( QgsProjectUtils::layerIsContainedInGroupLayer( QgsProject::instance(), mLayer ) );
  mFeatureBlendComboBox->setShowClippingModes( mBlendModeComboBox->showClippingModes() );

  // Blend mode
  mBlendModeComboBox->setBlendMode( mLayer->blendMode() );

  // Feature blend mode
  mFeatureBlendComboBox->setBlendMode( mLayer->featureBlendMode() );

  // Layer opacity
  mOpacityWidget->setOpacity( mLayer->opacity() );

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
    const QString rendererName = mLayer->renderer()->type();

    const int rendererIdx = cboRenderers->findData( rendererName );
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

void QgsRendererPropertiesDialog::changeOrderBy( const QgsFeatureRequest::OrderBy &orderBy, bool orderByEnabled )
{
  mOrderBy = orderBy;
  checkboxEnableOrderBy->setChecked( orderByEnabled );
}

void QgsRendererPropertiesDialog::updateUIState( bool hidden )
{
  mLayerRenderingGroupBox->setHidden( hidden );
  cboRenderers->setHidden( hidden );
}


void QgsRendererPropertiesDialog::keyPressEvent( QKeyEvent *e )
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
