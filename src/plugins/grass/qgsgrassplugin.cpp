/***************************************************************************
    qgsgrassplugin.cpp  -  GRASS menu
                             -------------------
    begin                : March, 2004
    copyright            : (C) 2004 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgrassplugin.h"
#include "qgis.h"
#include "qgsgrass.h"
#include "qgsgrassprovider.h"

#include "qgsgrassaddfeature.h"
#include "qgsgrasseditrenderer.h"
#include "qgsgrassnewmapset.h"
#include "qgsgrassregion.h"
#include "qgsgrassselect.h"
#include "qgsgrasstools.h"
#include "qgsgrassutils.h"

// includes
#include "qgisinterface.h"
#include "qgsapplication.h"
#include "qgslayertreeview.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsrubberband.h"
#include "qgsproject.h"
#include "qgsproviderregistry.h"
#include "qgsrendererv2registry.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayerregistry.h"

#include <QAction>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QToolBar>

extern "C"
{
#if GRASS_VERSION_MAJOR < 7
#include <grass/Vect.h>
#else
#include <grass/vector.h>
#endif
#include <grass/version.h>
}

static const QString pluginName = QObject::tr( "GRASS %1" ).arg( GRASS_VERSION_MAJOR );
static const QString pluginDescription = QObject::tr( "GRASS %1 (Geographic Resources Analysis Support System)" ).arg( GRASS_VERSION_MAJOR );
static const QString pluginCategory = QObject::tr( "Plugins" );
static const QString pluginVersion = QObject::tr( "Version 2.0" );
static const QString pluginIcon = ":/images/themes/default/grass/grass_tools.png";

/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param theQGisApp Pointer to the QGIS main window
 * @param theQgisInterFace Pointer to the QGIS interface object
 */
QgsGrassPlugin::QgsGrassPlugin( QgisInterface * theQgisInterFace )
    : mToolBarPointer( 0 )
    , qGisInterface( theQgisInterFace )
    , mCanvas( 0 )
    , mRegionAction( 0 )
    , mRegionBand( 0 )
    , mTools( 0 )
    , mNewMapset( 0 )
    , mOpenMapsetAction( 0 )
    , mNewMapsetAction( 0 )
    , mCloseMapsetAction( 0 )
    , mOpenToolsAction( 0 )
    , mOptionsAction( 0 )
    , mAddFeatureAction( 0 )
    , mAddPointAction( 0 )
    , mAddLineAction( 0 )
    , mAddBoundaryAction( 0 )
    , mAddCentroidAction( 0 )
    , mAddAreaAction( 0 )
    , mAddPoint( 0 )
    , mAddLine( 0 )
    , mAddBoundary( 0 )
    , mAddCentroid( 0 )
    , mAddArea()
{
}

QgsGrassPlugin::~QgsGrassPlugin()
{
  // When main app is closed, QgsGrassTools (probably because of dock widget) are destroyed before QgsGrassPlugin
  // -> do not call mTools here
  //if ( mTools )
  //  mTools->closeTools();
  QgsGrass::instance()->closeMapsetWarn();
}

/* Following functions return name, description, version, and type for the plugin */
QString QgsGrassPlugin::name()
{
  return pluginName;
}

QString QgsGrassPlugin::version()
{
  return pluginVersion;
}

QString QgsGrassPlugin::description()
{
  return pluginDescription;
}

QString QgsGrassPlugin::category()
{
  return pluginCategory;
}

void QgsGrassPlugin::help()
{
  //TODO
}

int QgsGrassPlugin::type()
{
  return QgisPlugin::UI;
}

/*
 * Initialize the GUI interface for the plugin
 */
void QgsGrassPlugin::initGui()
{
  mToolBarPointer = 0;
  mTools = 0;
  mNewMapset = 0;

  mCanvas = qGisInterface->mapCanvas();

  // Create region rubber band
  mRegionBand = new QgsRubberBand( mCanvas, QGis::Polygon );
  mRegionBand->setZValue( 20 );

  // Create the action for tool (the icons are set later by calling setCurrentTheme)
  mOpenMapsetAction = new QAction( QIcon(), tr( "Open Mapset" ), this );
  mOpenMapsetAction->setObjectName( "mOpenMapsetAction" );
  mNewMapsetAction = new QAction( QIcon(), tr( "New Mapset" ), this );
  mNewMapsetAction->setObjectName( "mNewMapsetAction" );
  mCloseMapsetAction = new QAction( QIcon(), tr( "Close Mapset" ), this );
  mCloseMapsetAction->setObjectName( "mCloseMapsetAction" );

  mOpenToolsAction = new QAction( QIcon(), tr( "Open GRASS Tools" ), this );
  mOpenToolsAction->setObjectName( "mAddPolygonActionmOpenToolsAction" );
  mOpenToolsAction->setWhatsThis( tr( "Open GRASS tools" ) );

  mRegionAction = new QAction( QIcon(), tr( "Display Current Grass Region" ), this );
  mRegionAction->setObjectName( "mRegionAction" );
  mRegionAction->setWhatsThis( tr( "Displays the current GRASS region as a rectangle on the map canvas" ) );
  mRegionAction->setCheckable( true );

  mOptionsAction = new QAction( QIcon(), tr( "GRASS Options" ), this );
  mOptionsAction->setObjectName( "mOptionsAction" );

  // Connect the actions
  connect( mOpenMapsetAction, SIGNAL( triggered() ), this, SLOT( openMapset() ) );
  connect( mNewMapsetAction, SIGNAL( triggered() ), this, SLOT( newMapset() ) );
  connect( mCloseMapsetAction, SIGNAL( triggered() ), SLOT( closeMapset() ) );
  connect( mOpenToolsAction, SIGNAL( triggered() ), this, SLOT( openTools() ) );
  connect( mRegionAction, SIGNAL( toggled( bool ) ), this, SLOT( switchRegion( bool ) ) );
  connect( mOptionsAction, SIGNAL( triggered() ), QgsGrass::instance(), SLOT( openOptions() ) );

  // Add actions to a GRASS plugin menu
  QString menu = tr( "&GRASS" );
  qGisInterface->addPluginToMenu( menu, mOpenMapsetAction );
  qGisInterface->addPluginToMenu( menu, mNewMapsetAction );
  qGisInterface->addPluginToMenu( menu, mCloseMapsetAction );
  qGisInterface->addPluginToMenu( menu, mOpenToolsAction );
  qGisInterface->addPluginToMenu( menu, mRegionAction );
  qGisInterface->addPluginToMenu( menu, mOptionsAction );

  // Add the toolbar to the main window
  mToolBarPointer = qGisInterface->addToolBar( tr( "GRASS" ) );
  mToolBarPointer->setObjectName( "GRASS" );

  // Add to the toolbar
#if 0
  mToolBarPointer->addAction( mOpenMapsetAction );
  mToolBarPointer->addAction( mNewMapsetAction );
  mToolBarPointer->addAction( mCloseMapsetAction );
  mToolBarPointer->addSeparator();
#endif
  mToolBarPointer->addAction( mOpenToolsAction );
  mToolBarPointer->addAction( mRegionAction );

  // Editing
  mAddPointAction = new QAction( QgsApplication::getThemeIcon( "/mActionCapturePoint.png" ), tr( "Add Point" ), this );
  mAddPointAction->setObjectName( "mAddPointAction" );
  mAddPointAction->setCheckable( true );

  mAddLineAction = new QAction( QgsApplication::getThemeIcon( "/mActionCaptureLine.png" ), tr( "Add Line" ), this );
  mAddLineAction->setObjectName( "mAddLineAction" );
  mAddLineAction->setCheckable( true );

  mAddBoundaryAction = new QAction( getThemeIcon( "mActionCaptureBoundary.png" ), tr( "Add Boundary" ), this );
  mAddBoundaryAction->setObjectName( "mAddBoundaryAction" );
  mAddBoundaryAction->setCheckable( true );

  mAddCentroidAction = new QAction( getThemeIcon( "mActionCaptureCentroid.png" ), tr( "Add Centroid" ), this );
  mAddCentroidAction->setObjectName( "mAddCentroidAction" );
  mAddCentroidAction->setCheckable( true );

  mAddAreaAction = new QAction( QgsApplication::getThemeIcon( "/mActionCapturePolygon.png" ), tr( "Add Closed Boundary" ), this );
  mAddAreaAction->setObjectName( "mAddAreaAction" );
  mAddAreaAction->setCheckable( true );

  connect( mAddPointAction, SIGNAL( triggered() ), SLOT( addFeature() ) );
  connect( mAddLineAction, SIGNAL( triggered() ), SLOT( addFeature() ) );
  connect( mAddBoundaryAction, SIGNAL( triggered() ), SLOT( addFeature() ) );
  connect( mAddCentroidAction, SIGNAL( triggered() ), SLOT( addFeature() ) );
  connect( mAddAreaAction, SIGNAL( triggered() ), SLOT( addFeature() ) );

  mAddFeatureAction = qGisInterface->actionAddFeature();

  mAddFeatureAction->actionGroup()->addAction( mAddPointAction );
  mAddFeatureAction->actionGroup()->addAction( mAddLineAction );
  mAddFeatureAction->actionGroup()->addAction( mAddBoundaryAction );
  mAddFeatureAction->actionGroup()->addAction( mAddCentroidAction );
  mAddFeatureAction->actionGroup()->addAction( mAddAreaAction );

  qGisInterface->digitizeToolBar()->insertAction( mAddFeatureAction, mAddPointAction );
  qGisInterface->digitizeToolBar()->insertAction( mAddFeatureAction, mAddLineAction );
  qGisInterface->digitizeToolBar()->insertAction( mAddFeatureAction, mAddBoundaryAction );
  qGisInterface->digitizeToolBar()->insertAction( mAddFeatureAction, mAddCentroidAction );
  qGisInterface->digitizeToolBar()->insertAction( mAddFeatureAction, mAddAreaAction );

  resetEditActions();

  mAddPoint = new QgsGrassAddFeature( qGisInterface->mapCanvas(), QgsMapToolAdvancedDigitizing::CapturePoint );
  mAddPoint->setAction( mAddPointAction );
  mAddLine = new QgsGrassAddFeature( qGisInterface->mapCanvas(), QgsMapToolAdvancedDigitizing::CaptureLine );
  mAddLine->setAction( mAddLineAction );
  mAddBoundary = new QgsGrassAddFeature( qGisInterface->mapCanvas(), QgsMapToolAdvancedDigitizing::CaptureLine );
  mAddBoundary->setAction( mAddBoundaryAction );
  mAddCentroid = new QgsGrassAddFeature( qGisInterface->mapCanvas(), QgsMapToolAdvancedDigitizing::CapturePoint );
  mAddCentroid->setAction( mAddCentroidAction );
  mAddArea = new QgsGrassAddFeature( qGisInterface->mapCanvas(), QgsMapToolAdvancedDigitizing::CapturePolygon );
  mAddArea->setAction( mAddAreaAction );

  connect( qGisInterface->actionSplitFeatures(), SIGNAL( triggered( bool ) ), SLOT( onSplitFeaturesTriggered( bool ) ) );

  // Connect project
  QWidget* qgis = qGisInterface->mainWindow();
  connect( qgis, SIGNAL( projectRead() ), this, SLOT( projectRead() ) );
  connect( qgis, SIGNAL( newProject() ), this, SLOT( newProject() ) );

  // Set icons to current theme
  setCurrentTheme( "" );
  // Connect theme change signal
  connect( qGisInterface, SIGNAL( currentThemeChanged( QString ) ), this, SLOT( setCurrentTheme( QString ) ) );

  connect( mCanvas, SIGNAL( destinationCrsChanged() ), this, SLOT( setTransform() ) );

  // Connect display region
  connect( mCanvas, SIGNAL( renderComplete( QPainter * ) ), this, SLOT( postRender( QPainter * ) ) );

  connect( QgsGrass::instance(), SIGNAL( gisbaseChanged() ), SLOT( onGisbaseChanged() ) );
  connect( QgsGrass::instance(), SIGNAL( mapsetChanged() ), SLOT( mapsetChanged() ) );
  connect( QgsGrass::instance(), SIGNAL( regionChanged() ), SLOT( displayRegion() ) );
  connect( QgsGrass::instance(), SIGNAL( regionPenChanged() ), SLOT( displayRegion() ) );
  connect( QgsGrass::instance(), SIGNAL( newLayer( QString, QString ) ), SLOT( onNewLayer( QString, QString ) ) );

  // Connect start/stop editing
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWasAdded( QgsMapLayer* ) ), this, SLOT( onLayerWasAdded( QgsMapLayer* ) ) );

  connect( qGisInterface->layerTreeView(), SIGNAL( currentLayerChanged( QgsMapLayer* ) ),
           SLOT( onCurrentLayerChanged( QgsMapLayer* ) ) );

  // open tools when plugin is loaded so that main app restores tools dock widget state
  mTools = new QgsGrassTools( qGisInterface, qGisInterface->mainWindow() );
  qGisInterface->addDockWidget( Qt::RightDockWidgetArea, mTools );

  // add edit renderer immediately so that if project was saved during editing, the layer can be loaded
  if ( !QgsRendererV2Registry::instance()->renderersList().contains( "grassEdit" ) )
  {
    QgsRendererV2Registry::instance()->addRenderer( new QgsRendererV2Metadata( "grassEdit",
        QObject::tr( "GRASS edit" ),
        QgsGrassEditRenderer::create,
        QIcon( QgsApplication::defaultThemePath() + "rendererGrassSymbol.svg" ),
        QgsGrassEditRendererWidget::create ) );
  }

  onGisbaseChanged();
  mapsetChanged();
}

void QgsGrassPlugin::onGisbaseChanged()
{
  if ( !QgsGrass::init() )
  {
    // TODO: save init error and get it here more reliably
    QString error = tr( "GRASS init error" );
    qGisInterface->messageBar()->pushMessage( error, QgsGrass::initError(), QgsMessageBar::WARNING );

    mOpenToolsAction->setDisabled( false ); // allow opening to see that tools are disabled
    mRegionAction->setDisabled( true );
    mOpenMapsetAction->setDisabled( true );
    mNewMapsetAction->setDisabled( true );
    mCloseMapsetAction->setDisabled( true );

    mTools->setWindowTitle( error + " : " + QgsGrass::initError() );
    mTools->setDisabled( true );
  }
  else
  {
    mOpenToolsAction->setDisabled( false );
    mRegionAction->setDisabled( !QgsGrass::activeMode() );
    mOpenMapsetAction->setDisabled( false );
    mNewMapsetAction->setDisabled( false );
    mCloseMapsetAction->setDisabled( !QgsGrass::activeMode() );

    mTools->setDisabled( false );
    mTools->resetTitle();
  }
}

void QgsGrassPlugin::onLayerWasAdded( QgsMapLayer* theMapLayer )
{
  QgsDebugMsg( "name = " + theMapLayer->name() );
  QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( theMapLayer );
  if ( !vectorLayer )
    return;
  QgsGrassProvider* grassProvider = dynamic_cast<QgsGrassProvider*>( vectorLayer->dataProvider() );
  if ( !grassProvider )
    return;

  QgsDebugMsg( "connect editing" );
  connect( vectorLayer, SIGNAL( editingStarted() ), this, SLOT( onEditingStarted() ) );
}

void QgsGrassPlugin::onCurrentLayerChanged( QgsMapLayer* layer )
{
  Q_UNUSED( layer );
  resetEditActions();
}

void QgsGrassPlugin::resetEditActions()
{

  QgsGrassProvider* grassProvider = 0;
  QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( qGisInterface->activeLayer() );
  if ( vectorLayer )
  {
    grassProvider = dynamic_cast<QgsGrassProvider*>( vectorLayer->dataProvider() );
  }
  if ( grassProvider && vectorLayer->editBuffer() )
  {
    mAddFeatureAction->setVisible( false );
    qGisInterface->actionSaveActiveLayerEdits()->setVisible( false );
    mAddPointAction->setVisible( true );
    mAddLineAction->setVisible( true );
    mAddBoundaryAction->setVisible( true );
    mAddCentroidAction->setVisible( true );
    mAddAreaAction->setVisible( true );
  }
  else
  {
    mAddFeatureAction->setVisible( true );
    qGisInterface->actionSaveActiveLayerEdits()->setVisible( true );
    mAddPointAction->setVisible( false );
    mAddLineAction->setVisible( false );
    mAddBoundaryAction->setVisible( false );
    mAddCentroidAction->setVisible( false );
    mAddAreaAction->setVisible( false );
  }
}

void QgsGrassPlugin::onEditingStarted()
{
  QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( sender() );
  if ( !vectorLayer )
    return;
  QgsDebugMsg( "started editing of layer " + vectorLayer->name() );

  // Set editing renderer
  QgsGrassProvider* grassProvider = dynamic_cast<QgsGrassProvider*>( vectorLayer->dataProvider() );
  if ( !grassProvider )
    return;

  mOldStyles[vectorLayer] = vectorLayer->styleManager()->currentStyle();
  mFormSuppress[vectorLayer] = vectorLayer->editFormConfig()->suppress();

  // Because the edit style may be stored to project:
  // - do not translate because it may be loaded in QGIS running with different language
  // - do not change the name until really necessary because it could not be found in project
  QString editStyleName = "GRASS Edit"; // should not be translated

  if ( vectorLayer->styleManager()->styles().contains( editStyleName ) )
  {
    QgsDebugMsg( editStyleName + " style exists -> set as current" );
    vectorLayer->styleManager()->setCurrentStyle( editStyleName );
  }
  else
  {
    QgsDebugMsg( "create and set style " + editStyleName );
    vectorLayer->styleManager()->addStyleFromLayer( editStyleName );

    //vectorLayer->styleManager()->addStyle( editStyleName, QgsMapLayerStyle() );
    vectorLayer->styleManager()->setCurrentStyle( editStyleName );

    QgsGrassEditRenderer *renderer = new QgsGrassEditRenderer();

    vectorLayer->setRendererV2( renderer );
  }

  grassProvider->startEditing( vectorLayer );
  vectorLayer->updateFields();

  connect( vectorLayer, SIGNAL( editingStopped() ), SLOT( onEditingStopped() ) );
  connect( grassProvider, SIGNAL( fieldsChanged() ), SLOT( onFieldsChanged() ) );

  resetEditActions();
}

void QgsGrassPlugin::onEditingStopped()
{
  QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( sender() );
  if ( vectorLayer )
  {
    QString style = mOldStyles.value( vectorLayer );
    if ( vectorLayer->styleManager()->currentStyle() == "GRASS Edit" ) // not changed by user
    {
      QgsDebugMsg( "reset style to " + style );
      vectorLayer->styleManager()->setCurrentStyle( style );
    }
  }
  resetEditActions();
}

void QgsGrassPlugin::onFieldsChanged()
{
  QgsGrassProvider* grassProvider = dynamic_cast<QgsGrassProvider*>( sender() );
  if ( !grassProvider )
  {
    return;
  }
  QString uri = grassProvider->dataSourceUri();
  uri.remove( QRegExp( "[^_]*$" ) );
  QgsDebugMsg( "uri = " + uri );
  Q_FOREACH ( QgsMapLayer *layer, QgsMapLayerRegistry::instance()->mapLayers().values() )
  {
    if ( !layer || layer->type() != QgsMapLayer::VectorLayer )
    {
      continue;
    }

    QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( layer );
    if ( vectorLayer && vectorLayer->providerType() == "grass" &&  vectorLayer->dataProvider() )
    {
      if ( vectorLayer->dataProvider()->dataSourceUri().startsWith( uri ) )
      {
        vectorLayer->updateFields();
      }
    }
  }
}

void QgsGrassPlugin::addFeature()
{
  QgsGrassProvider* grassProvider = 0;
  QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( qGisInterface->activeLayer() );
  if ( vectorLayer )
  {
    grassProvider = dynamic_cast<QgsGrassProvider*>( vectorLayer->dataProvider() );
  }
  if ( !grassProvider )
  {
    QgsDebugMsg( "grassProvider is null" );
    return;
  }
  QgsEditFormConfig::FeatureFormSuppress formSuppress = mFormSuppress.value( vectorLayer );
  if ( sender() == mAddPointAction )
  {
    qGisInterface->mapCanvas()->setMapTool( mAddPoint );
    grassProvider->setNewFeatureType( GV_POINT );
  }
  else if ( sender() == mAddLineAction )
  {
    qGisInterface->mapCanvas()->setMapTool( mAddLine );
    grassProvider->setNewFeatureType( GV_LINE );
  }
  else if ( sender() == mAddBoundaryAction )
  {
    qGisInterface->mapCanvas()->setMapTool( mAddBoundary );
    grassProvider->setNewFeatureType( GV_BOUNDARY );
    formSuppress = QgsEditFormConfig::SuppressOn;
  }
  else if ( sender() == mAddCentroidAction )
  {
    qGisInterface->mapCanvas()->setMapTool( mAddCentroid );
    grassProvider->setNewFeatureType( GV_CENTROID );
  }
  else if ( sender() == mAddAreaAction )
  {
    qGisInterface->mapCanvas()->setMapTool( mAddArea );
    grassProvider->setNewFeatureType( GV_AREA );
    formSuppress = QgsEditFormConfig::SuppressOn;
  }
  vectorLayer->editFormConfig()->setSuppress( formSuppress );
}

void QgsGrassPlugin::onSplitFeaturesTriggered( bool checked )
{
  if ( checked )
  {
    QgsGrassProvider* grassProvider = 0;
    QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( qGisInterface->activeLayer() );
    if ( vectorLayer )
    {
      grassProvider = dynamic_cast<QgsGrassProvider*>( vectorLayer->dataProvider() );
    }
    if ( !grassProvider )
    {
      QgsDebugMsg( "grassProvider is null" );
      return;
    }
    grassProvider->setNewFeatureType( QgsGrassProvider::LAST_TYPE );
  }
}

void QgsGrassPlugin::mapsetChanged()
{
  if ( !QgsGrass::activeMode() )
  {
    mRegionAction->setEnabled( false );
    mRegionBand->reset();
    mCloseMapsetAction->setEnabled( false );
  }
  else
  {
    mRegionAction->setEnabled( true );
    mCloseMapsetAction->setEnabled( true );

    QSettings settings;
    bool on = settings.value( "/GRASS/region/on", true ).toBool();
    mRegionAction->setChecked( on );
    switchRegion( on );

    QString gisdbase = QgsGrass::getDefaultGisdbase();
    QString location = QgsGrass::getDefaultLocation();
    try
    {
      mCrs = QgsGrass::crsDirect( gisdbase, location );
    }
    catch ( QgsGrass::Exception &e )
    {
      Q_UNUSED( e );
      QgsDebugMsg( "Cannot read GRASS CRS : " + QString( e.what() ) );
      mCrs = QgsCoordinateReferenceSystem();
    }
    QgsDebugMsg( "mCrs: " + mCrs.toWkt() );
    setTransform();
    redrawRegion();
  }

  if ( mTools )
  {
    mTools->mapsetChanged();
  }
}

// Open tools
void QgsGrassPlugin::openTools()
{
  mTools->show();
}

void QgsGrassPlugin::newVector()
{
  bool ok;
  QString name;

  QgsGrassElementDialog dialog( qGisInterface->mainWindow() );
  name = dialog.getItem( "vector", tr( "New vector name" ),
                         tr( "New vector name" ), "", "", &ok );

  if ( !ok )
    return;

  // Create new map
  QgsGrass::setMapset( QgsGrass::getDefaultGisdbase(),
                       QgsGrass::getDefaultLocation(),
                       QgsGrass::getDefaultMapset() );

  struct Map_info *Map = 0;
  G_TRY
  {
    Map = QgsGrass::vectNewMapStruct();
    Vect_open_new( Map, name.toUtf8().data(), 0 );

#if defined(GRASS_VERSION_MAJOR) && defined(GRASS_VERSION_MINOR) && \
  ( ( GRASS_VERSION_MAJOR == 6 && GRASS_VERSION_MINOR >= 4 ) || GRASS_VERSION_MAJOR > 6 )
    Vect_build( Map );
#else
    Vect_build( Map, stderr );
#endif
    Vect_set_release_support( Map );
    Vect_close( Map );
    QgsGrass::vectDestroyMapStruct( Map );
  }
  G_CATCH( QgsGrass::Exception &e )
  {
    QgsGrass::warning( tr( "Cannot create new vector: %1" ).arg( e.what() ) );
    QgsGrass::vectDestroyMapStruct( Map );
    return;
  }

  // Open in GRASS vector provider

  QString uri = QgsGrass::getDefaultGisdbase() + "/"
                + QgsGrass::getDefaultLocation() + "/"
                + QgsGrass::getDefaultMapset() + "/"
                + name + "/0_point";

  QgsVectorLayer* layer = new QgsVectorLayer( uri, name, "grass" );

  if ( !layer )
  {
    QMessageBox::warning( 0, tr( "Warning" ),
                          tr( "New vector created but cannot be opened by data provider." ) );
    return;
  }

  // TODO: start editing?
}

void QgsGrassPlugin::onNewLayer( QString uri, QString name )
{
  QgsDebugMsg( "uri = " + uri + " name = " + name );
  QgsVectorLayer* vectorLayer = qGisInterface->addVectorLayer( uri, name, "grass" );
  if ( vectorLayer )
  {
    vectorLayer->startEditing();
    qGisInterface->setActiveLayer( vectorLayer );
  }
}

void QgsGrassPlugin::postRender( QPainter *painter )
{
  Q_UNUSED( painter );
  // We have to redraw rectangle, because canvas->mapRenderer()->destinationCrs is set after GRASS plugin constructor! This way it is redrawn also if canvas CRS has changed.
  displayRegion();
}

void QgsGrassPlugin::displayRegion()
{

  mRegionBand->reset();
  if ( !mRegionAction->isChecked() )
  {
    return;
  }

  // Display region of current mapset if in active mode
  if ( !QgsGrass::activeMode() )
  {
    return;
  }

  struct Cell_head window;
  try
  {
    QgsGrass::region( &window );
  }
  catch ( QgsGrass::Exception &e )
  {
    QgsGrass::warning( e );
    return;
  }

  QgsRectangle rect( QgsPoint( window.west, window.north ), QgsPoint( window.east, window.south ) );

  QPen regionPen = QgsGrass::regionPen();
  mRegionBand->setColor( regionPen.color() );
  mRegionBand->setWidth( regionPen.width() );

  QgsGrassRegionEdit::drawRegion( mCanvas, mRegionBand, rect, &mCoordinateTransform );
}

void QgsGrassPlugin::switchRegion( bool on )
{

  QSettings settings;
  settings.setValue( "/GRASS/region/on", on );

  if ( on )
  {
    displayRegion();
  }
  else
  {
    mRegionBand->reset();
  }
}

void QgsGrassPlugin::redrawRegion()
{

  displayRegion();
}

void QgsGrassPlugin::openMapset()
{

  QString element;

  QgsGrassSelect *sel = new QgsGrassSelect( qGisInterface->mainWindow(), QgsGrassSelect::MAPSET );

  if ( !sel->exec() )
    return;

  QString err = QgsGrass::openMapset( sel->gisdbase,
                                      sel->location, sel->mapset );

  if ( !err.isNull() )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot open the mapset. %1" ).arg( err ) );
    return;
  }
  QgsGrass::saveMapset();
}

void QgsGrassPlugin::closeMapset()
{
  QgsGrass::instance()->closeMapsetWarn();
  QgsGrass::saveMapset();
}

void QgsGrassPlugin::newMapset()
{
  if ( !QgsGrassNewMapset::isRunning() )
  {
    mNewMapset = new QgsGrassNewMapset( qGisInterface,
                                        this, qGisInterface->mainWindow() );
  }
  mNewMapset->show();
  mNewMapset->raise();
}

void QgsGrassPlugin::projectRead()
{

  bool ok;
  QString gisdbase = QgsProject::instance()->readPath(
                       QgsProject::instance()->readEntry(
                         "GRASS", "/WorkingGisdbase", "", &ok ).trimmed()
                     );
  QString location = QgsProject::instance()->readEntry(
                       "GRASS", "/WorkingLocation", "", &ok ).trimmed();
  QString mapset = QgsProject::instance()->readEntry(
                     "GRASS", "/WorkingMapset", "", &ok ).trimmed();

  if ( gisdbase.isEmpty() || location.isEmpty() || mapset.isEmpty() )
  {
    return;
  }

  QgsDebugMsg( "Working mapset specified" );

  QString currentPath = QgsGrass::getDefaultGisdbase() + "/"
                        + QgsGrass::getDefaultLocation() + "/"
                        + QgsGrass::getDefaultMapset();

  QString newPath = gisdbase + "/" + location + "/" + mapset;

  if ( QFileInfo( currentPath ).canonicalPath() ==
       QFileInfo( newPath ).canonicalPath() )
  {
    // The same mapset is already open
    return;
  }

  QgsGrass::instance()->closeMapsetWarn();

  QString err = QgsGrass::openMapset( gisdbase, location, mapset );
  QgsGrass::saveMapset();

  if ( !err.isNull() )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot open GRASS mapset. %1" ).arg( err ) );
    return;
  }
}

void QgsGrassPlugin::newProject()
{
}

// Unload the plugin by cleaning up the GUI
void QgsGrassPlugin::unload()
{
  mAddFeatureAction->setVisible( true ); // restore QGIS add feature action

  // Close mapset
  QgsGrass::instance()->closeMapsetWarn();

  // disconnect slots of QgsGrassPlugin so they're not fired also after unload
  QWidget* qgis = qGisInterface->mainWindow();
  disconnect( qgis, SIGNAL( projectRead() ), this, SLOT( projectRead() ) );
  disconnect( qgis, SIGNAL( newProject() ), this, SLOT( newProject() ) );
  disconnect( qGisInterface, SIGNAL( currentThemeChanged( QString ) ), this, SLOT( setCurrentTheme( QString ) ) );
  disconnect( mCanvas, SIGNAL( destinationCrsChanged() ), this, SLOT( setTransform() ) );
  disconnect( mCanvas, SIGNAL( renderComplete( QPainter * ) ), this, SLOT( postRender( QPainter * ) ) );

  disconnect( QgsGrass::instance(), SIGNAL( gisbaseChanged() ), this, SLOT( onGisbaseChanged() ) );
  disconnect( QgsGrass::instance(), SIGNAL( mapsetChanged() ), this, SLOT( mapsetChanged() ) );
  disconnect( QgsGrass::instance(), SIGNAL( regionChanged() ), this, SLOT( displayRegion() ) );
  disconnect( QgsGrass::instance(), SIGNAL( regionPenChanged() ), this, SLOT( displayRegion() ) );
  disconnect( QgsGrass::instance(), SIGNAL( newLayer( QString, QString ) ), this, SLOT( onNewLayer( QString, QString ) ) );

  disconnect( QgsMapLayerRegistry::instance(), SIGNAL( layerWasAdded( QgsMapLayer* ) ), this, SLOT( onLayerWasAdded( QgsMapLayer* ) ) );

  disconnect( qGisInterface->layerTreeView(), SIGNAL( currentLayerChanged( QgsMapLayer* ) ),
              this, SLOT( onCurrentLayerChanged( QgsMapLayer* ) ) );

  Q_FOREACH ( QgsMapLayer *layer, QgsMapLayerRegistry::instance()->mapLayers().values() )
  {
    if ( !layer || layer->type() != QgsMapLayer::VectorLayer )
    {
      continue;
    }

    QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( layer );
    if ( vectorLayer && vectorLayer->providerType() == "grass" )
    {
      disconnect( vectorLayer, SIGNAL( editingStarted() ), this, SLOT( onEditingStarted() ) );
      disconnect( vectorLayer, SIGNAL( editingStopped() ), this, SLOT( onEditingStopped() ) );
    }
  }

  // remove the GUI
  qGisInterface->removePluginMenu( tr( "&GRASS" ), mOpenMapsetAction );
  qGisInterface->removePluginMenu( tr( "&GRASS" ), mNewMapsetAction );
  qGisInterface->removePluginMenu( tr( "&GRASS" ), mCloseMapsetAction );
  qGisInterface->removePluginMenu( tr( "&GRASS" ), mOpenToolsAction );
  qGisInterface->removePluginMenu( tr( "&GRASS" ), mRegionAction );
  qGisInterface->removePluginMenu( tr( "&GRASS" ), mOptionsAction );

  delete mOpenMapsetAction;
  delete mNewMapsetAction;
  delete mCloseMapsetAction;
  delete mOpenToolsAction;
  delete mRegionAction;
  delete mOptionsAction;

  delete mAddPointAction;
  delete mAddLineAction;
  delete mAddBoundaryAction;
  delete mAddCentroidAction;
  delete mAddAreaAction;

  delete mAddPoint;
  delete mAddLine;
  delete mAddBoundary;
  delete mAddCentroid;
  delete mAddArea;

  delete mToolBarPointer;
  mToolBarPointer = 0;

  delete mTools;
  mTools = 0;
}

// Set icons to the current theme
void QgsGrassPlugin::setCurrentTheme( QString theThemeName )
{
  Q_UNUSED( theThemeName );
  if ( mToolBarPointer )
  {
    mOpenMapsetAction->setIcon( getThemeIcon( "grass_open_mapset.png" ) );
    mNewMapsetAction->setIcon( getThemeIcon( "grass_new_mapset.png" ) );
    mCloseMapsetAction->setIcon( getThemeIcon( "grass_close_mapset.png" ) );
    mOpenToolsAction->setIcon( getThemeIcon( "grass_tools.png" ) );
    mRegionAction->setIcon( getThemeIcon( "grass_region.png" ) );
    mOptionsAction->setIcon( QgsApplication::getThemeIcon( "propertyicons/general.svg" ) );
  }
}

// Note this code is duplicated from qgisapp.cpp because
// I didnt want to make plugins dependent on qgsapplication
// and because it needs grass specific path into
// the GRASS plugin resource bundle [TS]
QIcon QgsGrassPlugin::getThemeIcon( const QString &theName )
{
  QString myCurThemePath = QgsApplication::activeThemePath() + "/grass/" + theName;
  QString myDefThemePath = QgsApplication::defaultThemePath() + "/grass/" + theName;
  QString myQrcPath = ":/default/grass/" + theName;
  if ( QFile::exists( myCurThemePath ) )
  {
    return QIcon( myCurThemePath );
  }
  else if ( QFile::exists( myDefThemePath ) )
  {
    return QIcon( myDefThemePath );
  }
  else if ( QFile::exists( myQrcPath ) )
  {
    return QIcon( myQrcPath );
  }
  else
  {
    return QIcon();
  }
}

void QgsGrassPlugin::setTransform()
{
  if ( mCrs.isValid() && mCanvas->mapSettings().destinationCrs().isValid() )
  {
    QgsDebugMsg( "srcCrs: " + mCrs.toWkt() );
    QgsDebugMsg( "destCrs " + mCanvas->mapSettings().destinationCrs().toWkt() );
    mCoordinateTransform.setSourceCrs( mCrs );
    mCoordinateTransform.setDestCRS( mCanvas->mapSettings().destinationCrs() );
  }
}

/**
 * Required extern functions needed  for every plugin
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin * classFactory( QgisInterface * theQgisInterfacePointer )
{
  return new QgsGrassPlugin( theQgisInterfacePointer );
}

// Return the name of the plugin - note that we do not user class members as
// the class may not yet be insantiated when this method is called.
QGISEXTERN QString name()
{
  return pluginName;
}

// Return the description
QGISEXTERN QString description()
{
  return pluginDescription;
}

// Return the category
QGISEXTERN QString category()
{
  return pluginCategory;
}

// Return the type (either UI or MapLayer plugin)
QGISEXTERN int type()
{
  return QgisPlugin::UI;
}

// Return the version number for the plugin
QGISEXTERN QString version()
{
  return pluginVersion;
}

QGISEXTERN QString icon()
{
  return pluginIcon;
}

// Delete ourself
QGISEXTERN void unload( QgisPlugin * thePluginPointer )
{
  delete thePluginPointer;
}
