/***************************************************************************
                          qgsgrassmoduleoptions.cpp
                             -------------------
    begin                : March, 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDomElement>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <QTextCodec>

#include "qgisinterface.h"

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

#include "qgsgrass.h"
#include "qgsgrassmodule.h"
#include "qgsgrassmoduleinput.h"
#include "qgsgrassmoduleoptions.h"
#include "qgsgrassmoduleparam.h"
#include "qgsgrassplugin.h"

extern "C"
{
#if GRASS_VERSION_MAJOR < 7
#else
#define G_adjust_Cell_head(cellhd,row_flag,col_flag) (G_adjust_Cell_head(cellhd,row_flag,col_flag),0)
#endif
}

/******************* QgsGrassModuleOptions *******************/

QgsGrassModuleOptions::QgsGrassModuleOptions(
  QgsGrassTools *tools, QgsGrassModule *module,
  QgisInterface *iface, bool direct )
    : mIface( iface )
    , mTools( tools )
    , mModule( module )
    , mRegionModeComboBox( 0 )
    , mDirect( direct )
{
  QgsDebugMsg( "called." );

  mCanvas = mIface->mapCanvas();
}

QgsGrassModuleOptions::~QgsGrassModuleOptions()
{
}

QStringList QgsGrassModuleOptions::arguments()
{
  return QStringList();
}

/*************** QgsGrassModuleStandardOptions ***********************/

QgsGrassModuleStandardOptions::QgsGrassModuleStandardOptions(
  QgsGrassTools *tools, QgsGrassModule *module,
  QgisInterface *iface,
  QString xname, QDomElement confDocElem,
  bool direct, QWidget * parent, Qt::WindowFlags f )
    : QWidget( parent, f )
    , QgsGrassModuleOptions( tools, module, iface, direct )
    , mXName( xname )
{
  //QgsDebugMsg( "called." );
  QgsDebugMsg( QString( "PATH = %1" ).arg( getenv( "PATH" ) ) );

  //
  //Set up dynamic inside a scroll box
  //
  QVBoxLayout * mypOuterLayout = new QVBoxLayout( this );
  mypOuterLayout->setContentsMargins( 0, 0, 0, 0 );
  QScrollArea * mypScrollArea = new QScrollArea();
  //transfers scroll area ownership so no need to call delete
  mypOuterLayout->addWidget( mypScrollArea );
  QFrame * mypInnerFrame = new QFrame();
  mypInnerFrame->setFrameShape( QFrame::NoFrame );
  mypInnerFrame->setFrameShadow( QFrame::Plain );
  //transfers frame ownership so no need to call delete
  mypScrollArea->setWidget( mypInnerFrame );
  mypScrollArea->setWidgetResizable( true );
  QVBoxLayout *mypInnerFrameLayout = new QVBoxLayout( mypInnerFrame );

  QFrame * mypRegionModeFrame = new QFrame();
  QHBoxLayout * mypRegionModeFrameLayout = new QHBoxLayout( mypRegionModeFrame );
  QLabel * mypRegionModeLabel = new QLabel( tr( "Region" ) );
  mRegionModeComboBox = new QComboBox();
  mRegionModeComboBox->addItem( tr( "Input layers" ), RegionInput );
  mRegionModeComboBox->addItem( tr( "Current map canvas" ), RegionCurrent );
  mRegionModeComboBox->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred ) );
  mypRegionModeFrameLayout->addWidget( mypRegionModeLabel );
  mypRegionModeFrameLayout->addWidget( mRegionModeComboBox );

  // Add frames for simple/advanced options
  QFrame * mypSimpleFrame = new QFrame();
  mypSimpleFrame->setFrameShape( QFrame::NoFrame );
  mypSimpleFrame->setFrameShadow( QFrame::Plain );
  mAdvancedFrame.setFrameShape( QFrame::NoFrame );
  mXName = xname;
  mAdvancedFrame.setFrameShadow( QFrame::Plain );

  QFrame * mypAdvancedPushButtonFrame = new QFrame();
  QHBoxLayout *mypAdvancedPushButtonFrameLayout = new QHBoxLayout( mypAdvancedPushButtonFrame );
  connect( &mAdvancedPushButton, SIGNAL( clicked() ), this, SLOT( switchAdvanced() ) );
  mypAdvancedPushButtonFrameLayout->addWidget( &mAdvancedPushButton );
  mypAdvancedPushButtonFrameLayout->addStretch( 1 );

  if ( mDirect )
  {
    mypInnerFrameLayout->addWidget( mypRegionModeFrame );
  }

  mypInnerFrameLayout->addWidget( mypSimpleFrame );
  mypInnerFrameLayout->addWidget( mypAdvancedPushButtonFrame );
  mypInnerFrameLayout->addWidget( &mAdvancedFrame );
  mypInnerFrameLayout->addStretch( 1 );

  // Hide advanced and set button next
  switchAdvanced();

  QVBoxLayout *mypSimpleLayout = new QVBoxLayout( mypSimpleFrame );
  QVBoxLayout *mypAdvancedLayout = new QVBoxLayout( &mAdvancedFrame );
  QVBoxLayout *layout = 0;

  QDomDocument gDomDocument = readInterfaceDescription( mXName, mErrors );
  if ( !mErrors.isEmpty() )
  {
    return;
  }

  QDomElement descDocElem = gDomDocument.documentElement();

  // Read QGIS options and create controls
  QDomNode confDomNode = confDocElem.firstChild();
  while ( !confDomNode.isNull() )
  {
    QDomElement confDomElement = confDomNode.toElement();
    if ( !confDomElement.isNull() )
    {
      // Check GRASS version
      QStringList errors;
      if ( !QgsGrassModuleOption::checkVersion( confDomElement.attribute( "version_min" ), confDomElement.attribute( "version_max" ), errors ) )
      {
        mErrors << errors; // checkVersion returns falso also if parsing fails
        confDomNode = confDomNode.nextSibling();
        continue;
      }

      QString optionType = confDomElement.tagName();
      QgsDebugMsg( "optionType = " + optionType );

      if ( confDomElement.attribute( "advanced", "no" ) == "yes" )
      {
        layout = mypAdvancedLayout;
      }
      else
      {
        layout = mypSimpleLayout;
      }

      QString key = confDomElement.attribute( "key" );
      QgsDebugMsg( "key = " + key );

      QDomNode gnode = QgsGrassModuleParam::nodeByKey( descDocElem, key );
      if ( gnode.isNull() )
      {
        mErrors << tr( "Cannot find key %1" ).arg( key );
        confDomNode = confDomNode.nextSibling();
        continue;
      }

      if ( optionType == "option" )
      {
        bool created = false;

        // Check option type and create appropriate control
        QDomNode promptNode = gnode.namedItem( "gisprompt" );
        QDomElement promptElem = promptNode.toElement();
        if ( !promptElem.isNull() )
        {
          QString element = promptElem.attribute( "element" );
          QString age = promptElem.attribute( "age" );

          //QgsDebugMsg("element = " + element + " age = " + age);
          if ( age == "old" && ( element == "vector" || element == "cell" ||
                                 element == "strds" || element == "stvds" ||
                                 element == "str3ds" || element == "stds" )
               &&  confDomElement.attribute( "widget" ) != "text" )
          {
            QgsGrassModuleInput *mi = new QgsGrassModuleInput(
              mModule, this, key, confDomElement, descDocElem, gnode, mDirect, this );

            layout->addWidget( mi );
            created = true;
            mParams.append( mi );
          }
        }

        if ( !created )
        {
          QgsGrassModuleOption *so = new QgsGrassModuleOption(
            mModule, key, confDomElement, descDocElem, gnode, mDirect, this );

          layout->addWidget( so );
          mParams.append( so );

          if ( promptElem.attribute( "prompt" ) == "dbcolumn" )
          {
            // Give only warning if the option is not hidden
            if ( !so->hidden() )
            {
              // G_OPT_DB_COLUMN may be also used for new columns (v.in.db) so we check also if there is at least one input vector
              // but a vector input may also exist (v.random).
              QList<QDomNode> vectorNodes = QgsGrassModuleParam::nodesByType( descDocElem, G_OPT_V_INPUT, "old" );
              QgsDebugMsg( QString( "vectorNodes.size() = %1" ).arg( vectorNodes.size() ) );
              if ( !vectorNodes.isEmpty() )
              {
                mErrors << tr( "Option '%1' should be configured as field" ).arg( so->key() );
              }
            }
          }
        }
      }
      else if ( optionType == "ogr" )
      {
        QgsGrassModuleGdalInput *mi = new QgsGrassModuleGdalInput(
          mModule, QgsGrassModuleGdalInput::Ogr, key, confDomElement,
          descDocElem, gnode, mDirect, this );
        layout->addWidget( mi );
        mParams.append( mi );
      }
      else if ( optionType == "gdal" )
      {
        QgsGrassModuleGdalInput *mi = new QgsGrassModuleGdalInput(
          mModule, QgsGrassModuleGdalInput::Gdal, key, confDomElement,
          descDocElem, gnode, mDirect, this );
        layout->addWidget( mi );
        mParams.append( mi );
      }
      else if ( optionType == "field" )
      {
        if ( confDomElement.hasAttribute( "layer" ) )
        {
          QgsGrassModuleVectorField *mi = new QgsGrassModuleVectorField(
            mModule, this, key, confDomElement,
            descDocElem, gnode, mDirect, this );
          layout->addWidget( mi );
          mParams.append( mi );
        }
        else
        {
          QgsGrassModuleField *mi = new QgsGrassModuleField(
            mModule, key, confDomElement,
            descDocElem, gnode, mDirect, this );
          layout->addWidget( mi );
          mParams.append( mi );
        }
      }
      else if ( optionType == "selection" )
      {
        QgsGrassModuleSelection *mi = new QgsGrassModuleSelection(
          mModule, this, key, confDomElement,
          descDocElem, gnode, mDirect, this );
        layout->addWidget( mi );
        mParams.append( mi );
      }
      else if ( optionType == "file" )
      {
        QgsGrassModuleFile *mi = new QgsGrassModuleFile(
          mModule, key, confDomElement, descDocElem, gnode, mDirect, this );
        layout->addWidget( mi );
        mParams.append( mi );
      }
      else if ( optionType == "flag" )
      {
        QgsGrassModuleFlag *flag = new QgsGrassModuleFlag(
          mModule, key, confDomElement, descDocElem, gnode, mDirect, this );

        layout->addWidget( flag );
        mParams.append( flag );
      }
    }
    confDomNode = confDomNode.nextSibling();
  }

  if ( mParams.size() == 0 )
  {
    QLabel *label = new QLabel( this );
    label->setText( tr( "This module has no options" ) );
    mypSimpleLayout->addWidget( label );
  }

  if ( mypAdvancedLayout->count() == 0 )
  {
    mypAdvancedPushButtonFrame->hide();
  }

  // Create list of flags
  confDomNode = descDocElem.firstChild();
  while ( !confDomNode.isNull() )
  {
    QDomElement confDomElement = confDomNode.toElement();
    if ( !confDomElement.isNull() )
    {
      QString optionType = confDomElement.tagName();
      QgsDebugMsg( "optionType = " + optionType );

      if ( optionType == "flag" )
      {
        QString name = confDomElement.attribute( "name" ).trimmed();
        QgsDebugMsg( "name = " + name );
        mFlagNames.append( name );
      }
    }
    confDomNode = confDomNode.nextSibling();
  }

#if 0
  // This works, but it would mean to desable check if 'field' tag has 'layer' defined in qgm.
  // It is probably better to require 'layer' attribute, so that it is always explicitly defined,
  // and we are sure it is set for modules with more inputs, where auto connection cannot be used
  // (guidependency missing in GRASS 6)
  // Add default inter param relations
  QList<QgsGrassModuleInput *>vectorInputs;
  Q_FOREACH ( QgsGrassModuleParam *param, mParams )
  {
    QgsGrassModuleInput *vectorInput = dynamic_cast<QgsGrassModuleInput *>( param );
    if ( vectorInput )
    {
      vectorInputs << vectorInput;
    }
  }
  if ( vectorInputs.size() == 1 )
  {
    QgsDebugMsg( "One input found, try to connect with column options" );
    QgsGrassModuleInput *vectorInput = vectorInputs[0];
    Q_FOREACH ( QgsGrassModuleParam *param, mParams )
    {
      QgsGrassModuleField *moduleField = dynamic_cast<QgsGrassModuleField *>( param );
      if ( moduleField )
      {
        if ( !moduleField->layerInput() )
        {
          QgsDebugMsg( "Set " + vectorInput->key() + " as layer input for " + moduleField->key() );
          moduleField->setLayerInput( vectorInput );
        }
      }
    }
  }
  else if ( vectorInputs.size() > 1 )
  {
    // TODO: check if type/field/column options have defined input layer
    // TODO: use 'guidependency' interface description attribute once GRASS 6 is dropped
  }
#endif

  if ( layout )
  {
    layout->addStretch();
  }

  Q_FOREACH ( QgsGrassModuleParam* item, mParams )
  {
    mErrors << item->errors();
  }

}

void QgsGrassModuleStandardOptions::switchAdvanced()
{
  if ( mAdvancedFrame.isHidden() )
  {
    mAdvancedFrame.show();
    mAdvancedPushButton.setText( tr( "<< Hide advanced options" ) );
  }
  else
  {
    mAdvancedFrame.hide();
    mAdvancedPushButton.setText( tr( "Show advanced options >>" ) );
  }
}

QStringList QgsGrassModuleStandardOptions::arguments()
{
  QStringList arg;

  for ( int i = 0; i < mParams.size(); i++ )
  {
    QStringList list = mParams[i]->options();

    for ( QStringList::Iterator it = list.begin();
          it != list.end(); ++it )
    {
      arg.append( *it );
    }
  }
  return arg;
}

// id is not used in fact, was intended for field, but key is used instead
QgsGrassModuleParam *QgsGrassModuleStandardOptions::itemByKey( QString key )
{
  QgsDebugMsg( "key = " + key );

  for ( int i = 0; i < mParams.size(); i++ )
  {
    if ( mParams[i]->key() == key )
    {
      return mParams[i];
    }
  }

  mErrors << tr( "Item with key %1 not found" ).arg( key );
  return 0;
}

QgsGrassModuleParam *QgsGrassModuleStandardOptions::item( QString id )
{
  QgsDebugMsg( "id = " + id );

  for ( int i = 0; i < mParams.size(); i++ )
  {
    if ( mParams[i]->id() == id )
    {
      return mParams[i];
    }
  }

  mErrors << tr( "Item with id %1 not found" ).arg( id );
  return 0;
}

QStringList QgsGrassModuleStandardOptions::checkOutput()
{
  QgsDebugMsg( "called." );
  QStringList list;

  for ( int i = 0; i < mParams.size(); i++ )
  {
    QgsGrassModuleOption *opt = dynamic_cast<QgsGrassModuleOption *>( mParams[i] );
    if ( !opt )
      continue;

    QgsDebugMsg( "opt->key() = " + opt->key() );

    if ( opt->isOutput() )
    {
      QString out = opt->outputExists();
      if ( !out.isNull() )
      {
        list.append( out );
      }
    }
  }

  return list;
}

QList<QgsGrassProvider *> QgsGrassModuleStandardOptions::grassProviders()
{
  QList<QgsGrassProvider *> providers;
  Q_FOREACH ( QgsMapLayer *layer, QgsMapLayerRegistry::instance()->mapLayers().values() )
  {
    if ( layer->type() == QgsMapLayer::VectorLayer )
    {
      QgsVectorLayer *vector = qobject_cast<QgsVectorLayer*>( layer );
      if ( vector  && vector->providerType() == "grass" )
      {
        QgsGrassProvider *provider = qobject_cast<QgsGrassProvider *>( vector->dataProvider() );
        if ( provider )
        {
          providers << provider;
        }
      }
    }
  }
  return providers;
}

QList<QgsGrassRasterProvider *> QgsGrassModuleStandardOptions::grassRasterProviders()
{
  QList<QgsGrassRasterProvider *> providers;
  Q_FOREACH ( QgsMapLayer *layer, QgsMapLayerRegistry::instance()->mapLayers().values() )
  {
    if ( layer->type() == QgsMapLayer::RasterLayer )
    {
      QgsRasterLayer *raster = qobject_cast<QgsRasterLayer*>( layer );
      if ( raster  && raster->providerType() == "grassraster" )
      {
        QgsGrassRasterProvider *provider = qobject_cast<QgsGrassRasterProvider *>( raster->dataProvider() );
        if ( provider )
        {
          providers << provider;
        }
      }
    }
  }
  return providers;
}

// freezeOutput/thawOutput is only necessary in Windows, where files cannot be overwritten
// when open by another app. It is enabled on all platforms, so that it gets tested.
void QgsGrassModuleStandardOptions::freezeOutput( bool freeze )
{
  QgsDebugMsg( "called." );

  for ( int i = 0; i < mParams.size(); i++ )
  {
    QgsGrassModuleOption *opt = dynamic_cast<QgsGrassModuleOption *>( mParams[i] );
    if ( !opt || !opt->isOutput() )
    {
      continue;
    }
    QgsDebugMsg( "opt->key() = " + opt->key() );

    if ( opt->outputType() == QgsGrassModuleOption::Vector )
    {
      QgsDebugMsg( "freeze vector layers" );

      QgsGrassObject outputObject = QgsGrass::getDefaultMapsetObject();
      outputObject.setName( opt->value() );
      outputObject.setType( QgsGrassObject::Vector );
      QgsDebugMsg( "outputObject = " + outputObject.toString() );

      Q_FOREACH ( QgsGrassProvider *provider, grassProviders() )
      {
        QgsGrassObject layerObject;
        layerObject.setFromUri( provider->dataSourceUri() );
        if ( layerObject == outputObject )
        {
          if ( freeze )
          {
            QgsDebugMsg( "freeze map " + provider->dataSourceUri() );
            provider->freeze();
          }
          else
          {
            QgsDebugMsg( "thaw map " + provider->dataSourceUri() );
            provider->thaw();
          }
        }
      }
    }
    else if ( opt->outputType() == QgsGrassModuleOption::Raster )
    {
      QgsDebugMsg( "freeze raster layers" );

      QgsGrassObject outputObject = QgsGrass::getDefaultMapsetObject();
      outputObject.setName( opt->value() );
      outputObject.setType( QgsGrassObject::Raster );
      QgsDebugMsg( "outputObject = " + outputObject.toString() );

      Q_FOREACH ( QgsGrassRasterProvider *provider, grassRasterProviders() )
      {
        QgsGrassObject layerObject;
        layerObject.setFromUri( provider->dataSourceUri() );
        if ( layerObject == outputObject )
        {
          if ( freeze )
          {
            QgsDebugMsg( "freeze map " + provider->dataSourceUri() );
            provider->freeze();
          }
          else
          {
            QgsDebugMsg( "thaw map " + provider->dataSourceUri() );
            provider->thaw();
          }
        }
      }
    }
  }
}

QStringList QgsGrassModuleStandardOptions::output( int type )
{
  QgsDebugMsg( "called." );
  QStringList list;

  for ( int i = 0; i < mParams.size(); i++ )
  {
    QgsGrassModuleOption *opt = dynamic_cast<QgsGrassModuleOption *>( mParams[i] );
    if ( !opt )
      continue;

    QgsDebugMsg( "opt->key() = " + opt->key() );

    if ( opt->isOutput() )
    {
      if ( opt->outputType() == type )
      {
        QString out = opt->value();
        if ( !out.isEmpty() )
        {
          list.append( out );
        }
      }
    }
  }

  return list;
}

bool QgsGrassModuleStandardOptions::hasOutput( int type )
{
  QgsDebugMsg( "called." );
  QStringList list;

  for ( int i = 0; i < mParams.size(); i++ )
  {
    QgsGrassModuleOption *opt = dynamic_cast<QgsGrassModuleOption *>( mParams[i] );
    if ( !opt )
      continue;

    QgsDebugMsg( "opt->key() = " + opt->key() );

    if ( opt->isOutput() )
    {
      if ( opt->outputType() == type )
        return true;
    }
  }

  return false;
}

QStringList QgsGrassModuleStandardOptions::ready()
{
  QStringList list;

  for ( int i = 0; i < mParams.size(); i++ )
  {
    QString err = mParams[i]->ready();
    if ( !err.isNull() )
    {
      list.append( err );
    }
  }

  return list;
}

QStringList QgsGrassModuleStandardOptions::checkRegion()
{
  QgsDebugMsg( "called." );
  QStringList list;

  struct Cell_head currentWindow;
  try
  {
    QgsGrass::region( &currentWindow );
  }
  catch ( QgsGrass::Exception &e )
  {
    QgsGrass::warning( e );
    return list;
  }

  for ( int i = 0; i < mParams.size(); i++ )
  {
    struct Cell_head window;

    QgsGrassModuleInput *item = dynamic_cast<QgsGrassModuleInput *>( mParams[i] );
    if ( !item )
      continue;

    QgsDebugMsg( "currentMap = " +  item->currentMap() );
    // The input may be empty, it means input is not used.

    if ( item->currentMap().isEmpty() )
    {
      continue;
    }
    if ( !getCurrentMapRegion( item, &window ) )
    {
      continue;
    }

    if ( G_window_overlap( &currentWindow,
                           window.north, window.south, window.east, window.west ) == 0 )
    {
      list.append( item->currentMap() );
    }
  }

  return list;
}

bool QgsGrassModuleStandardOptions::inputRegion( struct Cell_head *window, QgsCoordinateReferenceSystem &crs, bool all )
{
  QgsDebugMsg( "called." );

  RegionMode mode = ( QgsGrassModuleOptions::RegionMode ) mRegionModeComboBox->itemData( mRegionModeComboBox->currentIndex() ).toInt();
  if ( mDirect && mode == RegionCurrent )
  {
    // TODO: warn if outside region
    if ( mCanvas->hasCrsTransformEnabled() )
    {
      crs = mCanvas->mapSettings().destinationCrs();
    }
    else
    {
      crs = QgsCoordinateReferenceSystem();
    }
    QgsRectangle rect = mCanvas->extent();

    QgsGrass::initRegion( window );
    window->west = rect.xMinimum();
    window->south = rect.yMinimum();
    window->east = rect.xMaximum();
    window->north = rect.yMaximum();
    window->rows = ( int ) mCanvas->mapSettings().outputSize().height();
    window->cols = ( int ) mCanvas->mapSettings().outputSize().width();

    try
    {
      QgsGrass::adjustCellHead( window, 1, 1 );
    }
    catch ( QgsGrass::Exception &e )
    {
      QgsGrass::warning( e );
      return false;
    }
  }
  else
  {
    if ( mDirect )
    {
      QgsGrass::initRegion( window );
    }
    else
    {
      // Get current resolution
      try
      {
        QgsGrass::region( window );
      }
      catch ( QgsGrass::Exception &e )
      {
        QgsGrass::warning( e );
        return false;
      }
    }

    int rasterCount = 0;
    int vectorCount = 0;
    for ( int i = 0; i < mParams.size(); i++ )
    {
      struct Cell_head mapWindow;

      QgsGrassModuleInput *item = dynamic_cast<QgsGrassModuleInput *>( mParams[i] );
      if ( !item )
        continue;


      if ( !all && !item->useRegion() )
      {
        continue;
      }

      QgsDebugMsg( "currentMap = " +  item->currentMap() );
      // The input may be empty, it means input is not used.
      if ( item->currentMap().isEmpty() )
      {
        continue;
      }
      if ( !getCurrentMapRegion( item, &mapWindow ) )
      {
        return false;
      }

      // TODO: best way to set resolution ?
      if ( item->type() == QgsGrassObject::Raster
           && rasterCount == 0 )
      {
        QgsGrass::copyRegionResolution( &mapWindow, window );
      }
      if ( rasterCount + vectorCount == 0 )
      {
        QgsGrass::copyRegionExtent( &mapWindow, window );
      }
      else
      {
        QgsGrass::extendRegion( &mapWindow, window );
      }

      if ( item->type() == QgsGrassObject::Raster )
        rasterCount++;
      else if ( item->type() == QgsGrassObject::Vector )
        vectorCount++;
    }

    G_adjust_Cell_head3( window, 0, 0, 0 );
  }
  return true;
}

bool QgsGrassModuleStandardOptions::requestsRegion()
{
  QgsDebugMsg( "called." );

  if ( mDirect ) return true;

  for ( int i = 0; i < mParams.size(); i++ )
  {
    QgsGrassModuleInput *item = dynamic_cast<QgsGrassModuleInput *>( mParams[i] );
    if ( !item )
      continue;

    if ( item->useRegion() )
      return true;
  }
  return false;
}

bool QgsGrassModuleStandardOptions::usesRegion()
{
  QgsDebugMsg( "called." );

  for ( int i = 0; i < mParams.size(); i++ )
  {
    QgsGrassModuleInput *input = dynamic_cast<QgsGrassModuleInput *>( mParams[i] );
    if ( input && input->usesRegion() )
      return true;

    QgsGrassModuleOption *option = dynamic_cast<QgsGrassModuleOption *>( mParams[i] );
    if ( option && option->usesRegion() )
      return true;
  }

  QgsDebugMsg( "NO usesRegion()" );
  return false;
}

bool QgsGrassModuleStandardOptions::getCurrentMapRegion( QgsGrassModuleInput* input, struct Cell_head * window )
{
  if ( !input )
  {
    return false;
  }

  QgsDebugMsg( "currentMap = " +  input->currentMap() );
  if ( input->currentMap().isEmpty() )
  {
    // The input may be empty, it means input is not used.
    return false;
  }

  QStringList mm = input->currentMap().split( "@" );
  QString map = mm.value( 0 );
  QString mapset = QgsGrass::getDefaultMapset();
  if ( mm.size() > 1 )
  {
    mapset = mm.value( 1 );
  }
  if ( !QgsGrass::mapRegion( input->type(),
                             QgsGrass::getDefaultGisdbase(),
                             QgsGrass::getDefaultLocation(), mapset, map,
                             window ) )
  {
    QgsGrass::warning( tr( "Cannot get region of map %1" ).arg( input->currentMap() ) );
    return false;
  }
  return true;
}


QgsGrassModuleStandardOptions::~QgsGrassModuleStandardOptions()
{
}

QDomDocument QgsGrassModuleStandardOptions::readInterfaceDescription( const QString & xname, QStringList & errors )
{
  QDomDocument gDoc( "task" );

  // Attention!: sh.exe (MSYS) sets $0 in scripts to file name
  // without full path. Strange because when run from msys.bat
  // $0 is set to full path. GRASS scripts call
  // exec g.parser "$0" "$@"
  // and it fails -> find and run with full path
  QStringList arguments = QgsGrassModule::execArguments( xname );

  if ( arguments.size() == 0 )
  {
    errors << tr( "Cannot find module %1" ).arg( mXName );
    return gDoc;
  }

  QString cmd = arguments.takeFirst();

  arguments.append( "--interface-description" );

  QProcess process( this );

  QProcessEnvironment environment = QgsGrassModule::processEnvironment( mDirect );
  process.setProcessEnvironment( environment );
  process.start( cmd, arguments );

  // ? Does binary on Win need .exe extension ?
  // Return code 255 (-1) was correct in GRASS < 6.1.0
  // Return code 1 is the value (correct?) .py modules actually returns (see #4667)
  if ( !process.waitForStarted()
       || !process.waitForReadyRead()
       || !process.waitForFinished()
       || ( process.exitCode() != 0 && process.exitCode() != 255 &&
            ( !cmd.endsWith( ".py" ) || process.exitCode() != 1 ) ) )
  {
    QString pathVariable = QgsGrassModule::libraryPathVariable();
    QgsDebugMsg( "process.exitCode() = " + QString::number( process.exitCode() ) );
    QString msg = tr( "Cannot start module %1" ).arg( mXName )
                  + "<br><br>" + pathVariable + "=" + environment.value( pathVariable )
                  + "<br><br>PATH=" + environment.value( "PATH" )
                  + "<br><br>PYTHONPATH=" + environment.value( "PYTHONPATH" )
                  + "<br><br>" + tr( "command" ) + QString( ": %1 %2<br>%3<br>%4" )
                  .arg( cmd, arguments.join( " " ),
                        process.readAllStandardOutput().constData(),
                        process.readAllStandardError().constData() );
    QgsDebugMsg( msg );
    errors << msg;
    return gDoc;
  }

  QByteArray baDesc = process.readAllStandardOutput();

  // GRASS commands usually output text in system default encoding.
  // Let's use the System codec whether Qt doesn't recognize the encoding
  // of the interface description (see http://hub.qgis.org/issues/4547)
  QTextCodec *codec = 0;

  QgsDebugMsg( "trying to get encoding name from XML interface description..." );

  // XXX: getting the encoding using a regular expression works
  // until GRASS will use UTF-16 or UTF-32.
  // TODO: We should check the correct encoding by using the BOM (Byte
  // Order Mark) from the beginning of the data.
  QString xmlDeclaration = QString::fromUtf8( baDesc ).section( '>', 0, 0, QString::SectionIncludeTrailingSep );
  QRegExp reg( "<\\?xml\\s+.*encoding\\s*=\\s*(['\"])([A-Za-z][-a-zA-Z0-9_.]*)\\1\\s*\\?>" );
  if ( reg.indexIn( xmlDeclaration ) != -1 )
  {
    QByteArray enc = reg.cap( 2 ).toLocal8Bit();
    QgsDebugMsg( QString( "found encoding name '%1'" ).arg( QString::fromUtf8( enc ) ) );

    codec = QTextCodec::codecForName( enc );
    if ( !codec )
    {
      QgsDebugMsg( "unrecognized encoding name. Let's use 'System' codec" );
      codec = QTextCodec::codecForName( "System" );
      Q_ASSERT( codec );
    }
  }
  else
  {
    QgsDebugMsg( "unable to get encoding name from XML content. Will let Qt detects encoding!" );
  }

  bool ok = false;
  QString err;
  int line = -1, column = -1;

  if ( codec )
  {
    QgsDebugMsg( QString( "parsing XML interface description using '%1' codec..." ).arg( QString::fromUtf8( codec->name() ) ) );
    ok = gDoc.setContent( codec->toUnicode( baDesc ), false, &err, &line, &column );
    if ( !ok )
    {
      QgsDebugMsg( "parse FAILED. Will let Qt detects encoding" );
      codec = 0;
    }
  }

  if ( !codec )
  {
    ok = gDoc.setContent( baDesc, false, &err, &line, &column );
  }

  if ( !ok )
  {
    QString errmsg = tr( "Cannot read module description (%1):" ).arg( mXName )
                     + tr( "\n%1\nat line %2 column %3" ).arg( err ).arg( line ).arg( column );
    QgsDebugMsg( errmsg );
    errors << errmsg;
  }
  return gDoc;
}
