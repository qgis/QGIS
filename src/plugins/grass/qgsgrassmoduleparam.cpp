/***************************************************************************
                         qgsgrassmoduleparam.cpp
                             -------------------
    begin                : August, 2015
    copyright            : (C) 2015 by Radim Blazek
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

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>

#include "qgis.h"
#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"

#include "qgsgrass.h"
#include "qgsgrassmodule.h"
#include "qgsgrassmoduleparam.h"
#include "qgsgrassplugin.h"
#include "qgsgrassprovider.h"

extern "C"
{
#if GRASS_VERSION_MAJOR < 7
#include <grass/Vect.h>
#else
#include <grass/vector.h>
#endif
}

/********************** QgsGrassModuleParam *************************/
QgsGrassModuleParam::QgsGrassModuleParam( QgsGrassModule *module, QString key,
                                        QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode, bool direct )
    : mModule( module )
    , mKey( key )
    , mHidden( false )
    , mRequired( false )
    , mDirect( direct )
{
  Q_UNUSED( gdesc );
  //mAnswer = qdesc.attribute("answer", "");

  if ( !qdesc.attribute( "answer" ).isNull() )
  {
    mAnswer = qdesc.attribute( "answer" ).trimmed();
  }
  else
  {
    QDomNode n = gnode.namedItem( "default" );
    if ( !n.isNull() )
    {
      QDomElement e = n.toElement();
      mAnswer = e.text().trimmed();
    }
  }

  if ( qdesc.attribute( "hidden" ) == "yes" )
  {
    mHidden = true;
  }

  QString label, description;
  if ( !qdesc.attribute( "label" ).isEmpty() )
  {
    label = QApplication::translate( "grasslabel", qdesc.attribute( "label" ).trimmed().toUtf8() );
  }
  if ( label.isEmpty() )
  {
    QDomNode n = gnode.namedItem( "label" );
    if ( !n.isNull() )
    {
      QDomElement e = n.toElement();
      label = module->translate( e.text() );
    }
  }
  QDomNode n = gnode.namedItem( "description" );
  if ( !n.isNull() )
  {
    QDomElement e = n.toElement();
    description = module->translate( e.text() );
  }

  if ( !label.isEmpty() )
  {
    mTitle = label;
    mToolTip = description;
  }
  else
  {
    mTitle = description;
  }

  if ( gnode.toElement().attribute( "required" ) == "yes" )
  {
    mRequired = true;
  }

  mId = qdesc.attribute( "id" );
}

QgsGrassModuleParam::~QgsGrassModuleParam() {}

bool QgsGrassModuleParam::hidden()
{
  return mHidden;
}

QStringList QgsGrassModuleParam::options()
{
  return QStringList();
}

QString QgsGrassModuleParam::getDescPrompt ( QDomElement descDomElement )
{
  QDomNode gispromptNode = descDomElement.namedItem( "gisprompt" );

  if ( !gispromptNode.isNull())
  {
    QDomElement gispromptElement = gispromptNode.toElement();
    if ( !gispromptElement.isNull() )
    {
      return gispromptElement.attribute( "prompt" );
    }
  }
  return QString();
}

QDomNode QgsGrassModuleParam::nodeByKey( QDomElement descDomElement, QString key )
{
  QgsDebugMsg( "called with key=" + key );
  QDomNode n = descDomElement.firstChild();

  while ( !n.isNull() )
  {
    QDomElement e = n.toElement();

    if ( !e.isNull() )
    {
      if ( e.tagName() == "parameter" || e.tagName() == "flag" )
      {
        if ( e.attribute( "name" ) == key )
        {
          return n;
        }
      }
    }
    n = n.nextSibling();
  }

  return QDomNode();
}

QList<QDomNode> QgsGrassModuleParam::nodesByType( QDomElement descDomElement, STD_OPT optionType )
{
  // TODO: never tested
  QList<QDomNode> nodes;

  // Not all options have prompt set, for example G_OPT_V_TYPE and G_OPT_V_FIELD, which would be useful, don't have prompt
  QMap<QString, STD_OPT> typeMap;
#if GRASS_VERSION_MAJOR < 7
  typeMap.insert( "dbtable", G_OPT_TABLE );
  typeMap.insert( "dbdriver", G_OPT_DRIVER );
  typeMap.insert( "dbname", G_OPT_DATABASE );
  typeMap.insert( "dbcolumn", G_OPT_COLUMN );
#else
  typeMap.insert( "dbtable", G_OPT_DB_TABLE );
  typeMap.insert( "dbdriver", G_OPT_DB_DRIVER );
  typeMap.insert( "dbname", G_OPT_DB_DATABASE );
  typeMap.insert( "dbcolumn", G_OPT_DB_COLUMN );
#endif

  QDomNode n = descDomElement.firstChild();

  while ( !n.isNull() )
  {
    QString prompt = getDescPrompt ( n.toElement() );
    if ( typeMap.value( prompt ) == optionType )
    {
      nodes << n;
    }

    n = n.nextSibling();
  }

  return nodes;
}

/********************** QgsGrassModuleOption *************************/
void QgsGrassModuleOption::addLineEdit()
{
  QgsDebugMsg( "called." );

  // TODO make the widget growing with new lines. HOW???!!!
  QLineEdit *lineEdit = new QLineEdit( this );
  mLineEdits.push_back( lineEdit );
  lineEdit->setText( mAnswer );

  if ( mValueType == Integer )
  {
    if ( mHaveLimits )
    {
      mValidator = new QIntValidator(( int )mMin, ( int )mMax, this );
    }
    else
    {
      mValidator = new QIntValidator( this );
    }
    lineEdit->setValidator( mValidator );
  }
  else if ( mValueType == Double )
  {
    if ( mHaveLimits )
    {
      mValidator = new QDoubleValidator( mMin, mMax, 10, this );
    }
    else
    {
      mValidator = new QDoubleValidator( this );
    }
    lineEdit->setValidator( mValidator );
  }
  else if ( mIsOutput )
  {
    QRegExp rx;
    if ( mOutputType == Vector )
    {
      rx.setPattern( "[A-Za-z_][A-Za-z0-9_]+" );
    }
    else
    {
      rx.setPattern( "[A-Za-z0-9_.]+" );
    }
    mValidator = new QRegExpValidator( rx, this );

    lineEdit->setValidator( mValidator );
  }

  if ( mIsOutput && mDirect )
  {
    QHBoxLayout *l = new QHBoxLayout();
    l->addWidget( lineEdit );
    lineEdit->setSizePolicy( QSizePolicy::Expanding, QSizePolicy:: Preferred );
    QPushButton *button = new QPushButton( tr( "Browse" ) );
    l->addWidget( button );
    mLayout->addItem( l );
    connect( button, SIGNAL( clicked( bool ) ), this, SLOT( browse( bool ) ) );
  }
  else
  {
    mLayout->addWidget( lineEdit );
  }
}

void QgsGrassModuleOption::browse( bool checked )
{
  Q_UNUSED( checked );
  QgsDebugMsg( "called." );

  QSettings settings;
  QString lastDir = settings.value( "/GRASS/lastDirectOutputDir", "" ).toString();
  QString fileName = QFileDialog::getSaveFileName( this, tr( "Output file" ), lastDir, tr( "GeoTIFF" ) + " (*.tif)" );
  if ( !fileName.isEmpty() )
  {
    if ( !fileName.endsWith( ".tif", Qt::CaseInsensitive ) && !fileName.endsWith( ".tiff", Qt::CaseInsensitive ) )
    {
      fileName = fileName + ".tif";
    }
    mLineEdits.at( 0 )->setText( fileName );
    settings.setValue( "/GRASS/lastDirectOutputDir",  QFileInfo( fileName ).absolutePath() );
  }
}

void QgsGrassModuleOption::removeLineEdit()
{
  QgsDebugMsg( "called." );

  if ( mLineEdits.size() < 2 )
    return;
  delete mLineEdits.at( mLineEdits.size() - 1 );
  mLineEdits.pop_back();
}

QString QgsGrassModuleOption::outputExists()
{
  QgsDebugMsg( "called." );

  if ( !mIsOutput )
    return QString();

  QLineEdit *lineEdit = mLineEdits.at( 0 );
  QString value = lineEdit->text().trimmed();
  QgsDebugMsg( "mKey = " + mKey );
  QgsDebugMsg( "value = " + value );
  QgsDebugMsg( "mOutputElement = " + mOutputElement );

  if ( value.length() == 0 )
    return QString();

  QString path = QgsGrass::getDefaultGisdbase() + "/"
                 + QgsGrass::getDefaultLocation() + "/"
                 + QgsGrass::getDefaultMapset() + "/"
                 + mOutputElement + "/" + value;

  QFileInfo fi( path );

  if ( fi.exists() )
  {
    return ( lineEdit->text() );
  }

  return QString();
}

QString QgsGrassModuleOption::value()
{
  QString value;

  if ( mControlType == LineEdit )
  {
    for ( int i = 0; i < mLineEdits.size(); i++ )
    {
      QLineEdit *lineEdit = mLineEdits.at( i );
      if ( lineEdit->text().trimmed().length() > 0 )
      {
        if ( value.length() > 0 )
          value.append( "," );
        value.append( lineEdit->text().trimmed() );
      }
    }
  }
  else if ( mControlType == ComboBox )
  {
    value = mValues[mComboBox->currentIndex()];
  }
  else if ( mControlType == CheckBoxes )
  {
    QStringList values;
    for ( int i = 0; i < mCheckBoxes.size(); ++i )
    {
      if ( mCheckBoxes[i]->isChecked() )
      {
        values.append( mValues[i] );
      }
    }
    value = values.join( "," );
  }
  return value;
}

bool QgsGrassModuleOption::checkVersion( const QString& version_min, const QString& version_max, QStringList& errors )
{
  QgsDebugMsg( "version_min = " + version_min );
  QgsDebugMsg( "version_max = " + version_max );

  bool minOk = true;
  bool maxOk = true;
  QRegExp rxVersion( "(\\d+)\\.(\\d+)" );
  if ( !version_min.isEmpty() )
  {
    if ( !rxVersion.exactMatch( version_min ) )
    {
      errors << tr( "Cannot parse version_min %1" ).arg( version_min );
    }
    else
    {
      int versionMajorMin = rxVersion.cap( 1 ).toInt();
      int versionMinorMin = rxVersion.cap( 2 ).toInt();
      if ( QgsGrass::versionMajor() < versionMajorMin || ( QgsGrass::versionMajor() == versionMajorMin && QgsGrass::versionMinor() < versionMinorMin ) )
      {
        minOk = false;
      }
    }
  }

  if ( !version_max.isEmpty() )
  {
    if ( !rxVersion.exactMatch( version_max ) )
    {
      errors << tr( "Cannot parse version_max %1" ).arg( version_max );
    }
    else
    {
      int versionMajorMax = rxVersion.cap( 1 ).toInt();
      int versionMinorMax = rxVersion.cap( 2 ).toInt();
      if ( QgsGrass::versionMajor() > versionMajorMax || ( QgsGrass::versionMajor() == versionMajorMax && QgsGrass::versionMinor() > versionMinorMax ) )
      {
        maxOk = false;
      }
    }
  }
  return errors.isEmpty() && minOk && maxOk;
}

QStringList QgsGrassModuleOption::options()
{
  QStringList list;

  if ( mHidden )
  {
    list.push_back( mKey + "=" + mAnswer );
  }
  else
  {
    QString val = value();
    if ( !val.isEmpty() )
    {
      list.push_back( mKey + "=" + val );
    }
  }
  return list;
}

QString QgsGrassModuleOption::ready()
{
  QgsDebugMsg( "called." );

  QString error;

  if ( mControlType == LineEdit )
  {
    if ( mLineEdits.at( 0 )->text().trimmed().length() == 0 && mRequired )
    {
      error.append( tr( "%1:&nbsp;missing value" ).arg( title() ) );
    }
  }
  return error;
}

QgsGrassModuleOption::~QgsGrassModuleOption()
{
}

/***************** QgsGrassModuleFlag *********************/
QgsGrassModuleFlag::QgsGrassModuleFlag( QgsGrassModule *module, QString key,
                                        QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
                                        bool direct, QWidget * parent )
    : QgsGrassModuleCheckBox( "", parent ), QgsGrassModuleParam( module, key, qdesc, gdesc, gnode, direct )
{
  QgsDebugMsg( "called." );

  if ( mHidden )
    hide();

  if ( mAnswer == "on" )
    setChecked( true );
  else
    setChecked( false );

  setText( mTitle );
  setToolTip( mToolTip );
}

QStringList QgsGrassModuleFlag::options()
{
  QStringList list;
  if ( isChecked() )
  {
    list.push_back( "-" + mKey );
  }
  return list;
}

QgsGrassModuleFlag::~QgsGrassModuleFlag()
{
}

/************************** QgsGrassModuleInput ***************************/

QgsGrassModuleInput::QgsGrassModuleInput( QgsGrassModule *module,
    QgsGrassModuleStandardOptions *options, QString key,
    QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
    bool direct, QWidget * parent )
    : QgsGrassModuleGroupBoxItem( module, key, qdesc, gdesc, gnode, direct, parent )
    , mType( QgsGrassModuleInput::Vector )
    , mModuleStandardOptions( options )
    , mGeometryTypeOption( "" )
    , mVectorLayerOption( "" )
    , mLayerComboBox( 0 )
    , mRegionButton( 0 )
    , mUpdate( false )
    , mUsesRegion( false )
    , mRequired( false )
{
  QgsDebugMsg( "called." );
  mGeometryTypeMask = GV_POINT | GV_LINE | GV_AREA;

  if ( mTitle.isEmpty() )
  {
    mTitle = tr( "Input" );
  }
  adjustTitle();

  // Check if this parameter is required
  mRequired = gnode.toElement().attribute( "required" ) == "yes";

  QDomNode promptNode = gnode.namedItem( "gisprompt" );
  QDomElement promptElem = promptNode.toElement();
  QString element = promptElem.attribute( "element" );

  if ( element == "vector" )
  {
    mType = Vector;

    // Read type mask if "typeoption" is defined
    QString opt = qdesc.attribute( "typeoption" );
    if ( ! opt.isNull() )
    {

      QDomNode optNode = nodeByKey( gdesc, opt );

      if ( optNode.isNull() )
      {
        mErrors << tr( "Cannot find typeoption %1" ).arg( opt );
      }
      else
      {
        mGeometryTypeOption = opt;

        QDomNode valuesNode = optNode.namedItem( "values" );
        if ( valuesNode.isNull() )
        {
          mErrors << tr( "Cannot find values for typeoption %1" ).arg( opt );
        }
        else
        {
          mGeometryTypeMask = 0; //GV_POINT | GV_LINE | GV_AREA;

          QDomElement valuesElem = valuesNode.toElement();
          QDomNode valueNode = valuesElem.firstChild();

          while ( !valueNode.isNull() )
          {
            QDomElement valueElem = valueNode.toElement();

            if ( !valueElem.isNull() && valueElem.tagName() == "value" )
            {
              QDomNode n = valueNode.namedItem( "name" );
              if ( !n.isNull() )
              {
                QDomElement e = n.toElement();
                QString val = e.text().trimmed();

                if ( val == "point" )
                {
                  mGeometryTypeMask |= GV_POINT;
                }
                else if ( val == "line" )
                {
                  mGeometryTypeMask |= GV_LINE;
                }
                else if ( val == "area" )
                {
                  mGeometryTypeMask |= GV_AREA;
                }
              }
            }

            valueNode = valueNode.nextSibling();
          }
        }
      }
    }

    // Read type mask defined in configuration
    opt = qdesc.attribute( "typemask" );
    if ( ! opt.isNull() )
    {
      int mask = 0;

      if ( opt.indexOf( "point" ) >= 0 )
      {
        mask |= GV_POINT;
      }
      if ( opt.indexOf( "line" ) >= 0 )
      {
        mask |= GV_LINE;
      }
      if ( opt.indexOf( "area" ) >= 0 )
      {
        mask |= GV_AREA;
      }

      mGeometryTypeMask &= mask;
    }

    // Read "layeroption" if defined
    opt = qdesc.attribute( "layeroption" );
    if ( ! opt.isNull() )
    {

      QDomNode optNode = nodeByKey( gdesc, opt );

      if ( optNode.isNull() )
      {
        mErrors << tr( "Cannot find layeroption %1" ).arg( opt );
      }
      else
      {
        mVectorLayerOption = opt;
      }
    }

    // Read "mapid"
    mMapId = qdesc.attribute( "mapid" );
  }
  else if ( element == "cell" )
  {
    mType = Raster;
  }
  else
  {
    mErrors << tr( "GRASS element %1 not supported" ).arg( element );
  }

  if ( qdesc.attribute( "update" ) == "yes" )
  {
    mUpdate = true;
  }

  QHBoxLayout *l = new QHBoxLayout( this );
  mLayerComboBox = new QComboBox();
  mLayerComboBox->setSizePolicy( QSizePolicy::Expanding, QSizePolicy:: Preferred );
  l->addWidget( mLayerComboBox );

  QString region = qdesc.attribute( "region" );
  if ( mType == Raster
       && QgsGrass::versionMajor() >= 6 && QgsGrass::versionMinor() >= 1
       && region != "no"
     )
  {

    mRegionButton = new QPushButton(
      QgsGrassPlugin::getThemeIcon( "grass_set_region.png" ), "" );

    mRegionButton->setToolTip( tr( "Use region of this map" ) );
    mRegionButton->setCheckable( true );
    mRegionButton->setSizePolicy( QSizePolicy::Minimum,
                                  QSizePolicy:: Preferred );

    if ( !mDirect )
    {
      l->addWidget( mRegionButton );
    }
  }

  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersAdded( QList<QgsMapLayer *> ) ),
           this, SLOT( updateQgisLayers() ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersRemoved( QStringList ) ),
           this, SLOT( updateQgisLayers() ) );

  connect( mLayerComboBox, SIGNAL( activated( int ) ), this, SLOT( changed( int ) ) );

  if ( !mMapId.isEmpty() )
  {
    QgsGrassModuleParam *item = mModuleStandardOptions->item( mMapId );
    if ( item )
    {
      QgsGrassModuleInput *mapInput = dynamic_cast<QgsGrassModuleInput *>( item );

      connect( mapInput, SIGNAL( valueChanged() ), this, SLOT( updateQgisLayers() ) );
    }
  }

  mUsesRegion = false;
  if ( region.length() > 0 )
  {
    if ( region == "yes" )
      mUsesRegion = true;
  }
  else
  {
    if ( type() == Raster )
      mUsesRegion = true;
  }

  // Fill in QGIS layers
  updateQgisLayers();
}

bool QgsGrassModuleInput::useRegion()
{
  QgsDebugMsg( "called." );

  return mUsesRegion && mType == Raster && mRegionButton && mRegionButton->isChecked();
}

void QgsGrassModuleInput::updateQgisLayers()
{
  QgsDebugMsg( "called." );

  QString current = mLayerComboBox->currentText();
  mLayerComboBox->clear();
  mMaps.clear();
  mGeometryTypes.clear();
  mVectorLayerNames.clear();
  mMapLayers.clear();
  mBands.clear();
  mVectorFields.clear();

  // If not required, add an empty item to combobox and a padding item into
  // layer containers.
  if ( !mRequired )
  {
    mMaps.push_back( QString( "" ) );
    mVectorLayerNames.push_back( QString( "" ) );
    mMapLayers.push_back( NULL );
    mBands.append( 0 );
    mLayerComboBox->addItem( tr( "Select a layer" ), QVariant() );
  }

  // Find map option
  QString sourceMap;
  if ( !mMapId.isEmpty() )
  {
    QgsGrassModuleParam *item = mModuleStandardOptions->item( mMapId );
    if ( item )
    {
      QgsGrassModuleInput *mapInput = dynamic_cast<QgsGrassModuleInput *>( item );
      if ( mapInput )
        sourceMap = mapInput->currentMap();
    }
  }

  // Note: QDir::cleanPath is using '/' also on Windows
  //QChar sep = QDir::separator();
  QChar sep = '/';

  //QgsMapCanvas *canvas = mModule->qgisIface()->mapCanvas();
  //int nlayers = canvas->layerCount();
  foreach ( QString layerId, QgsMapLayerRegistry::instance()->mapLayers().keys() )
  {
    //QgsMapLayer *layer = canvas->layer( i );
    QgsMapLayer *layer =  QgsMapLayerRegistry::instance()->mapLayers().value( layerId );

    QgsDebugMsg( "layer->type() = " + QString::number( layer->type() ) );

    if ( mType == Vector && layer->type() == QgsMapLayer::VectorLayer )
    {
      QgsVectorLayer *vector = ( QgsVectorLayer* )layer;
      QgsDebugMsg( "vector->providerType() = " + vector->providerType() );
      if ( vector->providerType() != "grass" )
        continue;

      //TODO dynamic_cast ?
      QgsGrassProvider *provider = ( QgsGrassProvider * ) vector->dataProvider();

      // Check type mask
      int geomType = provider->geometryType();

      if (( geomType == QGis::WKBPoint && !( mGeometryTypeMask & GV_POINT ) ) ||
          ( geomType == QGis::WKBLineString && !( mGeometryTypeMask & GV_LINE ) ) ||
          ( geomType == QGis::WKBPolygon && !( mGeometryTypeMask & GV_AREA ) )
         )
      {
        continue;
      }

      // TODO add map() mapset() location() gisbase() to grass provider
      QString source = QDir::cleanPath( provider->dataSourceUri() );

      QgsDebugMsg( "source = " + source );

      // Check GISDBASE and LOCATION
      QStringList split = source.split( sep, QString::SkipEmptyParts );

      if ( split.size() < 4 )
        continue;
      split.pop_back(); // layer

      QString map = split.last();
      split.pop_back(); // map

      QString mapset = split.last();
      split.pop_back(); // mapset

      //QDir locDir ( sep + split.join ( QString(sep) ) );
      //QString loc = locDir.canonicalPath();
      QString loc =  source.remove( QRegExp( "/[^/]+/[^/]+/[^/]+$" ) );
      loc = QDir( loc ).canonicalPath();

      QDir curlocDir( QgsGrass::getDefaultGisdbase() + sep + QgsGrass::getDefaultLocation() );
      QString curloc = curlocDir.canonicalPath();

      QgsDebugMsg( "loc = " + loc );
      QgsDebugMsg( "curloc = " + curloc );
      QgsDebugMsg( "mapset = " + mapset );
      QgsDebugMsg( "QgsGrass::getDefaultMapset() = " + QgsGrass::getDefaultMapset() );

      if ( loc != curloc )
        continue;

      if ( mUpdate && mapset != QgsGrass::getDefaultMapset() )
        continue;

      // Check if it comes from source map if necessary
      if ( !mMapId.isEmpty() )
      {
        QString cm = map + "@" + mapset;
        if ( sourceMap != cm )
          continue;
      }

      mMaps.push_back( map + "@" + mapset );

      QString type;
      if ( geomType == QGis::WKBPoint )
      {
        type = "point";
      }
      else if ( geomType == QGis::WKBLineString )
      {
        type = "line";
      }
      else if ( geomType == QGis::WKBPolygon )
      {
        type = "area";
      }
      else
      {
        type = "unknown";
      }

      mGeometryTypes.push_back( type );

      QString grassLayer = QString::number( provider->grassLayer() );

      QString label = layer->name() + " ( " + map + "@" + mapset
                      + " " + grassLayer + " " + type + " )";

      mLayerComboBox->addItem( label );
      if ( label == current )
        mLayerComboBox->setCurrentIndex( mLayerComboBox->count() - 1 );

      mMapLayers.push_back( vector );
      mVectorLayerNames.push_back( grassLayer );

      // convert from QgsFields to std::vector<QgsField>
      mVectorFields.push_back( vector->dataProvider()->fields() );
    }
    else if ( mType == Raster && layer->type() == QgsMapLayer::RasterLayer )
    {
      if ( mDirect )
      {
        // Add item for each numeric band
        QgsRasterLayer* rasterLayer = qobject_cast<QgsRasterLayer *>( layer );
        if ( rasterLayer && rasterLayer->dataProvider() )
        {
          QString providerKey = rasterLayer->dataProvider()->name();
          // TODO: GRASS itself is not supported for now because module is run
          // with fake GRASS gis lib and the provider needs true gis lib
          if ( providerKey == "grassraster" ) continue;
          // Cannot use WCS until the problem with missing QThread is solved
          if ( providerKey == "wcs" ) continue;
          for ( int i = 1; i <= rasterLayer->dataProvider()->bandCount(); i++ )
          {
            if ( QgsRasterBlock::typeIsNumeric( rasterLayer->dataProvider()->dataType( i ) ) )
            {
              QString uri = rasterLayer->dataProvider()->dataSourceUri();
              mMaps.push_back( uri );

              QString label = tr( "%1 (band %2)" ).arg( rasterLayer->name() ).arg( i );
              mLayerComboBox->addItem( label );
              mMapLayers.push_back( layer );
              mBands.append( i );

              if ( label == current )
                mLayerComboBox->setCurrentIndex( mLayerComboBox->count() - 1 );
            }
          }
        }
      }
      else
      {
        // Check if it is GRASS raster
        QString source = QDir::cleanPath( layer->source() );

        if ( source.contains( "cellhd" ) == 0 )
          continue;

        // Most probably GRASS layer, check GISDBASE and LOCATION
        QStringList split = source.split( sep, QString::SkipEmptyParts );

        if ( split.size() < 4 )
          continue;

        QString map = split.last();
        split.pop_back(); // map
        if ( split.last() != "cellhd" )
          continue;
        split.pop_back(); // cellhd

        QString mapset = split.last();
        split.pop_back(); // mapset

        //QDir locDir ( sep + split.join ( QString(sep) ) );
        //QString loc = locDir.canonicalPath();
        QString loc =  source.remove( QRegExp( "/[^/]+/[^/]+/[^/]+$" ) );
        loc = QDir( loc ).canonicalPath();

        QDir curlocDir( QgsGrass::getDefaultGisdbase() + sep + QgsGrass::getDefaultLocation() );
        QString curloc = curlocDir.canonicalPath();

        if ( loc != curloc )
          continue;

        if ( mUpdate && mapset != QgsGrass::getDefaultMapset() )
          continue;

        mMaps.push_back( map + "@" + mapset );
        mMapLayers.push_back( layer );

        QString label = layer->name() + " ( " + map + "@" + mapset + " )";

        mLayerComboBox->addItem( label );
        if ( label == current )
          mLayerComboBox->setCurrentIndex( mLayerComboBox->count() - 1 );
      }
    }
  }
}

QStringList QgsGrassModuleInput::options()
{
  QStringList list;
  QString opt;

  int current = mLayerComboBox->currentIndex();
  if ( current < 0 ) // not found
    return list;

  if ( mDirect )
  {
    QgsMapLayer *layer = mMapLayers[current];

    if ( layer->type() == QgsMapLayer::RasterLayer )
    {
      QgsRasterLayer* rasterLayer = qobject_cast<QgsRasterLayer *>( layer );
      if ( !rasterLayer || !rasterLayer->dataProvider() )
      {
        QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot get provider" ) );
        return list;
      }
      QString grassUri;
      QString providerUri = rasterLayer->dataProvider()->dataSourceUri();
      QString providerKey = rasterLayer->dataProvider()->name();
      int band = mBands.value( current );
      if ( providerKey == "gdal" && band == 1 )
      {
        // GDAL provider and band 1 are defaults, thus we can use simply GDAL path
        grassUri = providerUri;
      }
      else
      {
        // Need to encode more info into uri
        QgsDataSourceURI uri;
        if ( providerKey == "gdal" )
        {
          // providerUri is simple file path
          // encoded uri is not currently supported by GDAL provider, it is only used here and decoded in fake gis lib
          uri.setParam( "path", providerUri );
        }
        else // WCS
        {
          // providerUri is encoded QgsDataSourceURI
          uri.setEncodedUri( providerUri );
        }
        uri.setParam( "provider", providerKey );
        uri.setParam( "band", QString::number( band ) );
        grassUri = uri.encodedUri();
      }
      opt = mKey + "=" + grassUri;
      list.push_back( opt );
    }
    else if ( layer->type() == QgsMapLayer::VectorLayer )
    {
      QgsVectorLayer* vectorLayer = qobject_cast<QgsVectorLayer *>( layer );
      if ( !vectorLayer || !vectorLayer->dataProvider() )
      {
        QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot get provider" ) );
        return list;
      }
      opt = mKey + "=" + vectorLayer->dataProvider()->dataSourceUri();
      list.push_back( opt );
    }
  }
  else
  {
    // TODO: this is hack for network nodes, do it somehow better
    if ( mMapId.isEmpty() )
    {
      if ( current <  mMaps.size() )
      {
        if ( ! mMaps[current].isEmpty() )
        {
          list.push_back( mKey + "=" + mMaps[current] );
        }
      }
    }

    if ( !mGeometryTypeOption.isEmpty() && current < mGeometryTypes.size() )
    {
      opt = mGeometryTypeOption + "=" + mGeometryTypes[current];
      list.push_back( opt );
    }

    if ( !mVectorLayerOption.isEmpty() && current < mVectorLayerNames.size() )
    {
      opt = mVectorLayerOption + "=" + mVectorLayerNames[current];
      list.push_back( opt );
    }
  }

  return list;
}

QgsFields QgsGrassModuleInput::currentFields()
{
  QgsDebugMsg( "called." );

  int limit = 0;
  if ( !mRequired )
    limit = 1;

  QgsFields fields;

  int current = mLayerComboBox->currentIndex();
  if ( current < limit )
    return fields;

  if ( current >= limit && current <  mVectorFields.size() )
  {
    fields = mVectorFields[current];
  }

  return fields;
}

QgsMapLayer * QgsGrassModuleInput::currentLayer()
{
  QgsDebugMsg( "called." );

  int limit = 0;
  if ( !mRequired )
    limit = 1;

  int current = mLayerComboBox->currentIndex();
  if ( current < limit )
    return 0;

  if ( current >= limit && current <  mMapLayers.size() )
  {
    return mMapLayers[current];
  }

  return 0;
}

QString QgsGrassModuleInput::currentMap()
{
  QgsDebugMsg( "called." );

  int limit = 0;
  if ( !mRequired )
    limit = 1;

  int current = mLayerComboBox->currentIndex();
  if ( current < limit )
    return QString();

  if ( current >= limit && current < mMaps.size() )
  {
    return mMaps[current];
  }

  return QString();
}

void QgsGrassModuleInput::changed( int i )
{
  Q_UNUSED( i );
  emit valueChanged();
}

QString QgsGrassModuleInput::ready()
{
  QgsDebugMsg( "called." );

  QString error;

  QgsDebugMsg( QString( "count = %1" ).arg( mLayerComboBox->count() ) );
  if ( mLayerComboBox->count() == 0 )
  {
    error.append( tr( "%1:&nbsp;no input" ).arg( title() ) );
  }
  return error;
}

QgsGrassModuleInput::~QgsGrassModuleInput()
{
}

/***************** QgsGrassModuleGroupBoxItem *********************/

QgsGrassModuleGroupBoxItem::QgsGrassModuleGroupBoxItem( QgsGrassModule *module, QString key,
    QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
    bool direct, QWidget * parent )
    : QGroupBox( parent )
    , QgsGrassModuleParam( module, key, qdesc, gdesc, gnode, direct )
{
  adjustTitle();

  setToolTip( mToolTip );
}

QgsGrassModuleGroupBoxItem::~QgsGrassModuleGroupBoxItem() {}

void QgsGrassModuleGroupBoxItem::resizeEvent( QResizeEvent * event )
{
  Q_UNUSED( event );
  adjustTitle();
  setToolTip( mToolTip );
}

void QgsGrassModuleGroupBoxItem::adjustTitle()
{
  QString tit = fontMetrics().elidedText( mTitle, Qt::ElideRight, width() - 20 );

  setTitle( tit );
}

/***************** QgsGrassModuleGdalInput *********************/

QgsGrassModuleGdalInput::QgsGrassModuleGdalInput(
  QgsGrassModule *module, int type, QString key, QDomElement &qdesc,
  QDomElement &gdesc, QDomNode &gnode, bool direct, QWidget * parent )
    : QgsGrassModuleGroupBoxItem( module, key, qdesc, gdesc, gnode, direct, parent )
    , mType( type )
    , mOgrLayerOption( "" )
    , mOgrWhereOption( "" )
{
  if ( mTitle.isEmpty() )
  {
    mTitle = tr( "OGR/PostGIS/GDAL Input" );
  }
  adjustTitle();

  // Check if this parameter is required
  mRequired = gnode.toElement().attribute( "required" ) == "yes";

  QDomNode promptNode = gnode.namedItem( "gisprompt" );
  QDomElement promptElem = promptNode.toElement();
  QString element = promptElem.attribute( "element" );

  // Read "layeroption" is defined
  QString opt = qdesc.attribute( "layeroption" );
  if ( ! opt.isNull() )
  {

    QDomNode optNode = nodeByKey( gdesc, opt );

    if ( optNode.isNull() )
    {
      mErrors << tr( "Cannot find layeroption %1" ).arg( opt );
    }
    else
    {
      mOgrLayerOption = opt;
    }
  }

  // Read "whereoption" if defined
  opt = qdesc.attribute( "whereoption" );
  if ( !opt.isNull() )
  {
    QDomNode optNode = nodeByKey( gdesc, opt );
    if ( optNode.isNull() )
    {
      mErrors << tr( "Cannot find whereoption %1" ).arg( opt );
    }
    else
    {
      mOgrWhereOption = opt;
    }
  }

  QVBoxLayout *l = new QVBoxLayout( this );
  mLayerComboBox = new QComboBox();
  mLayerComboBox->setSizePolicy( QSizePolicy::Expanding, QSizePolicy:: Preferred );
  l->addWidget( mLayerComboBox );

  QLabel *lbl = new QLabel( tr( "Password" ) );
  l->addWidget( lbl );

  mLayerPassword = new QLineEdit();
  mLayerPassword->setEchoMode( QLineEdit::Password );
  mLayerPassword->setEnabled( false );
  l->addWidget( mLayerPassword );

  lbl->setBuddy( mLayerPassword );

  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersAdded( QList<QgsMapLayer *> ) ),
           this, SLOT( updateQgisLayers() ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersRemoved( QStringList ) ),
           this, SLOT( updateQgisLayers() ) );

  // Fill in QGIS layers
  updateQgisLayers();
}

void QgsGrassModuleGdalInput::updateQgisLayers()
{
  QgsDebugMsg( "called." );

  QString current = mLayerComboBox->currentText();
  mLayerComboBox->clear();
  mUri.clear();
  mOgrLayers.clear();

  // If not required, add an empty item to combobox and a padding item into
  // layer containers.
  if ( !mRequired )
  {
    mUri.push_back( QString() );
    mOgrLayers.push_back( QString() );
    mOgrWheres.push_back( QString() );
    mLayerComboBox->addItem( tr( "Select a layer" ), QVariant() );
  }

  foreach ( QgsMapLayer *layer, QgsMapLayerRegistry::instance()->mapLayers().values() )
  {
    if ( !layer ) continue;

    if ( mType == Ogr && layer->type() == QgsMapLayer::VectorLayer )
    {
      QgsVectorLayer *vector = qobject_cast<QgsVectorLayer *>( layer );
      if ( !vector ||
           ( vector->providerType() != "ogr" && vector->providerType() != "postgres" )
         )
        continue;

      QgsDataProvider *provider = vector->dataProvider();

      QString uri;
      QString ogrLayer;
      QString ogrWhere;
      if ( vector->providerType() == "postgres" )
      {
        // Construct OGR DSN
        QgsDataSourceURI dsUri( provider->dataSourceUri() );
        uri = "PG:" + dsUri.connectionInfo();

        // Starting with GDAL 1.7.0, it is possible to restrict the schemas
        // layer names are then listed without schema if only one schema is specified
        if ( !dsUri.schema().isEmpty() )
        {
          uri += " schemas=" + dsUri.schema();
        }

        ogrLayer += dsUri.table();
        ogrWhere = dsUri.sql();
      }
      else if ( vector->providerType() == "ogr" )
      {
        QStringList items = provider->dataSourceUri().split( "|" );

        if ( items.size() > 1 )
        {
          uri = items[0];

          ogrLayer = "";
          ogrWhere = "";

          for ( int i = 1; i < items.size(); i++ )
          {
            QStringList args = items[i].split( "=" );

            if ( args.size() != 2 )
              continue;

            if ( args[0] == "layername" && args[0] == "layerid" )
            {
              ogrLayer = args[1];
            }
            else if ( args[0] == "subset" )
            {
              ogrWhere = args[1];
            }
          }

          if ( uri.endsWith( ".shp", Qt::CaseInsensitive ) )
          {
            ogrLayer = "";
          }
        }
        else
        {
          uri = items[0];
          ogrLayer = "";
          ogrWhere = "";
        }
      }

      QgsDebugMsg( "uri = " + uri );
      QgsDebugMsg( "ogrLayer = " + ogrLayer );

      mLayerComboBox->addItem( layer->name() );
      if ( layer->name() == current )
        mLayerComboBox->setItemText( mLayerComboBox->currentIndex(), current );

      mUri.push_back( uri );
      mOgrLayers.push_back( ogrLayer );
      mOgrWheres.push_back( ogrWhere );
    }
    else if ( mType == Gdal && layer->type() == QgsMapLayer::RasterLayer )
    {
      QString uri = layer->source();
      mLayerComboBox->addItem( layer->name() );
      if ( layer->name() == current )
        mLayerComboBox->setItemText( mLayerComboBox->currentIndex(), current );
      mUri.push_back( uri );
      mOgrLayers.push_back( "" );
      mOgrWheres.push_back( "" );
    }
  }
}

QStringList QgsGrassModuleGdalInput::options()
{
  QStringList list;

  int current = mLayerComboBox->currentIndex();
  if ( current < 0 )
    return list;

  QString opt( mKey + "=" );

  if ( current >= 0 && current < mUri.size() )
  {
    QString uri = mUri[current];

    if ( uri.startsWith( "PG:" ) && uri.contains( "password=" ) && !mLayerPassword->text().isEmpty() )
    {
      uri += " password=" + mLayerPassword->text();
    }

    opt.append( uri );
  }

  list.push_back( opt );

  if ( !mOgrLayerOption.isEmpty() && mOgrLayers[current].size() > 0 )
  {
    opt = mOgrLayerOption + "=";
    // GDAL 1.4.0 supports schemas (r9998)
#if GDAL_VERSION_NUM >= 1400
    opt += mOgrLayers[current];
#else
    // Handle older versions of gdal gracefully
    // OGR does not support schemas !!!
    if ( current >= 0 && current <  mUri.size() )
    {
      QStringList l = mOgrLayers[current].split( "." );
      opt += l.at( 1 );

      // Currently only PostGIS is using layer
      //  -> layer -> PostGIS -> warning
      if ( mOgrLayers[current].length() > 0 )
      {
        QMessageBox::warning( 0, tr( "Warning" ),
                              tr( "PostGIS driver in OGR does not support schemas!<br>"
                                  "Only the table name will be used.<br>"
                                  "It can result in wrong input if more tables of the same name<br>"
                                  "are present in the database." ) );
      }
    }
#endif //GDAL_VERSION_NUM
    list.push_back( opt );
  }

  if ( !mOgrWhereOption.isEmpty() && mOgrWheres[current].length() > 0 )
  {
    list.push_back( mOgrWhereOption + "=" + mOgrWheres[current] );
  }

  return list;
}

QString QgsGrassModuleGdalInput::ready()
{
  QgsDebugMsg( "called." );

  QString error;

  QgsDebugMsg( QString( "count = %1" ).arg( mLayerComboBox->count() ) );
  if ( mLayerComboBox->count() == 0 )
  {
    error.append( tr( "%1:&nbsp;no input" ).arg( title() ) );
  }
  return error;
}

void QgsGrassModuleGdalInput::changed( int i )
{
  mLayerPassword->setEnabled( i < mUri.size() && mUri[i].startsWith( "PG:" ) && !mUri[i].contains( "password=" ) );
}

QgsGrassModuleGdalInput::~QgsGrassModuleGdalInput()
{
}

/***************** QgsGrassModuleField *********************/

QgsGrassModuleField::QgsGrassModuleField(
  QgsGrassModule *module, QgsGrassModuleStandardOptions *options,
  QString key, QDomElement &qdesc,
  QDomElement &gdesc, QDomNode &gnode, bool direct, QWidget * parent )
    : QgsGrassModuleGroupBoxItem( module, key, qdesc, gdesc, gnode, direct, parent )
    , mModuleStandardOptions( options ), mLayerInput( 0 )
{
  if ( mTitle.isEmpty() )
  {
    mTitle = tr( "Attribute field" );
  }
  adjustTitle();

  QDomNode promptNode = gnode.namedItem( "gisprompt" );
  QDomElement promptElem = promptNode.toElement();
  QString element = promptElem.attribute( "element" );

  mType = qdesc.attribute( "type" );

  mLayerKey = qdesc.attribute( "layer" );
  if ( mLayerKey.isNull() || mLayerKey.length() == 0 )
  {
    mErrors << tr( "'layer' attribute in field tag with key= %1 is missing." ).arg( mKey );
  }
  else
  {
    QgsGrassModuleParam *item = mModuleStandardOptions->itemByKey( mLayerKey );
    // TODO check type
    if ( item )
    {
      mLayerInput = dynamic_cast<QgsGrassModuleInput *>( item );
      connect( mLayerInput, SIGNAL( valueChanged() ), this, SLOT( updateFields() ) );
    }
  }

  QHBoxLayout *l = new QHBoxLayout( this );
  mFieldComboBox = new QComboBox();
  l->addWidget( mFieldComboBox );

  // Fill in layer current fields
  updateFields();
}

void QgsGrassModuleField::updateFields()
{
  QgsDebugMsg( "called." );

  QString current = mFieldComboBox->currentText();
  mFieldComboBox->clear();

  //QgsMapCanvas *canvas = mModule->qgisIface()->mapCanvas();

  if ( mLayerInput == 0 )
    return;

  QgsFields fields = mLayerInput->currentFields();

  for ( int i = 0; i < fields.size(); i++ )
  {
    if ( mType.contains( fields[i].typeName() ) )
    {
      mFieldComboBox->addItem( fields[i].name() );
      if ( fields[i].name() == current )
      {
        mFieldComboBox->setItemText( mFieldComboBox->currentIndex(), current );
      }
    }
  }
}

QStringList QgsGrassModuleField::options()
{
  QStringList list;

  if ( !mFieldComboBox->currentText().isEmpty() )
  {
    QString opt( mKey + "=" + mFieldComboBox->currentText() );
    list.push_back( opt );
  }

  return list;
}

QgsGrassModuleField::~QgsGrassModuleField()
{
}

/***************** QgsGrassModuleSelection *********************/

QgsGrassModuleSelection::QgsGrassModuleSelection(
  QgsGrassModule *module, QgsGrassModuleStandardOptions *options,
  QString key, QDomElement &qdesc,
  QDomElement &gdesc, QDomNode &gnode, bool direct, QWidget * parent )
    : QgsGrassModuleGroupBoxItem( module, key, qdesc, gdesc, gnode, direct, parent )
    , mModuleStandardOptions( options )
    , mLayerInput( 0 )
    , mVectorLayer( 0 )
{
  if ( mTitle.isEmpty() )
  {
    mTitle = tr( "Selected categories" );
  }
  adjustTitle();

  QDomNode promptNode = gnode.namedItem( "gisprompt" );
  QDomElement promptElem = promptNode.toElement();
  QString element = promptElem.attribute( "element" );

  mLayerId = qdesc.attribute( "layerid" );

  mType = qdesc.attribute( "type" );

  QgsGrassModuleParam *item = mModuleStandardOptions->item( mLayerId );
  // TODO check type
  if ( item )
  {
    mLayerInput = dynamic_cast<QgsGrassModuleInput *>( item );
    connect( mLayerInput, SIGNAL( valueChanged() ), this, SLOT( updateSelection() ) );
  }

  QHBoxLayout *l = new QHBoxLayout( this );
  mLineEdit = new QLineEdit( this );
  l->addWidget( mLineEdit );

  // Fill in layer current fields
  updateSelection();
}

void QgsGrassModuleSelection::updateSelection()
{
  QgsDebugMsg( "called." );

  mLineEdit->setText( "" );
  //QgsMapCanvas *canvas = mModule->qgisIface()->mapCanvas();
  if ( mLayerInput == 0 )
    return;

  QgsMapLayer *layer = mLayerInput->currentLayer();
  if ( !layer )
    return;
  QgsVectorLayer *vector = qobject_cast<QgsVectorLayer *>( layer );

  QgsGrassProvider *provider = ( QgsGrassProvider * ) vector->dataProvider();
  QgsAttributeList allAttributes = provider->attributeIndexes();
  const QgsFeatureIds& selected = vector->selectedFeaturesIds();
  int keyField = provider->keyField();

  if ( keyField < 0 )
    return;

  QString cats;
  QgsFeatureIterator fi = provider->getFeatures( QgsFeatureRequest() );
  QgsFeature feature;

  int i = 0;
  while ( fi.nextFeature( feature ) )
  {
    if ( !selected.contains( feature.id() ) )
      continue;

    QgsAttributes attr = feature.attributes();
    if ( attr.size() > keyField )
    {
      if ( i > 0 )
        cats.append( "," );
      cats.append( attr[keyField].toString() );
      i++;
    }
  }
  if ( mVectorLayer != vector )
  {
    if ( mVectorLayer )
    {
      disconnect( mVectorLayer, SIGNAL( selectionChanged() ), this, SLOT( updateSelection() ) );
    }

    connect( vector, SIGNAL( selectionChanged() ), this, SLOT( updateSelection() ) );
    mVectorLayer = vector;
  }

  mLineEdit->setText( cats );
}

QStringList QgsGrassModuleSelection::options()
{
  QStringList list;

  if ( !mLineEdit->text().isEmpty() )
  {
    QString opt( mKey + "=" + mLineEdit->text() );
    list.push_back( opt );
  }

  return list;
}

QgsGrassModuleSelection::~QgsGrassModuleSelection()
{
}

/***************** QgsGrassModuleFile *********************/

QgsGrassModuleFile::QgsGrassModuleFile(
  QgsGrassModule *module,
  QString key, QDomElement &qdesc,
  QDomElement &gdesc, QDomNode &gnode, bool direct, QWidget * parent )
    : QgsGrassModuleGroupBoxItem( module, key, qdesc, gdesc, gnode, direct, parent )
    , mType( Old )
{
  if ( mTitle.isEmpty() )
  {
    mTitle = tr( "File" );
  }
  adjustTitle();

  QDomNode promptNode = gnode.namedItem( "gisprompt" );
  QDomElement promptElem = promptNode.toElement();
  QString element = promptElem.attribute( "element" );

  if ( qdesc.attribute( "type" ).toLower() == "new" )
  {
    mType = New;
  }
  if ( qdesc.attribute( "type" ).toLower() == "multiple" )
  {
    mType = Multiple;
  }

  if ( qdesc.attribute( "type" ).toLower() == "directory" )
  {
    mType = Directory;
  }

  if ( !qdesc.attribute( "filters" ).isNull() )
  {
    mFilters = qdesc.attribute( "filters" ).split( ";;" );

    if ( mFilters.size() > 0 )
    {
      QRegExp rx( ".*\\( *..([^ )]*).*" );
      QString ext;
      if ( rx.indexIn( mFilters.at( 0 ) ) == 0 )
      {
        mSuffix = rx.cap( 1 );
      }
    }
  }

  mFileOption = qdesc.attribute( "fileoption" );

  QHBoxLayout *l = new QHBoxLayout( this );
  mLineEdit = new QLineEdit();
  mBrowseButton = new QPushButton( "..." );
  l->addWidget( mLineEdit );
  l->addWidget( mBrowseButton );

  connect( mBrowseButton, SIGNAL( clicked() ),
           this, SLOT( browse() ) );
}

QStringList QgsGrassModuleFile::options()
{
  QStringList list;
  QString path = mLineEdit->text().trimmed();

  if ( mFileOption.isNull() )
  {
    QString opt( mKey + "=" + path );
    list.push_back( opt );
  }
  else
  {
    QFileInfo fi( path );

    QString opt( mKey + "=" + fi.path() );
    list.push_back( opt );

    opt = mFileOption + "=" + fi.baseName();
    list.push_back( opt );
  }

  return list;
}

void QgsGrassModuleFile::browse()
{
  static QString lastDir = QDir::currentPath();

  if ( mType == Multiple )
  {
    QString path = mLineEdit->text().split( "," ).first();
    if ( path.isEmpty() )
      path = lastDir;
    else
      path = QFileInfo( path ).absolutePath();

    QStringList files = QFileDialog::getOpenFileNames( this, 0, path );
    if ( files.isEmpty() )
      return;

    lastDir = QFileInfo( files[0] ).absolutePath();

    mLineEdit->setText( files.join( "," ) );
  }
  else
  {
    QString selectedFile = mLineEdit->text();
    if ( selectedFile.isEmpty() )
      selectedFile = lastDir;

    if ( mType == New )
      selectedFile = QFileDialog::getSaveFileName( this, 0, selectedFile );
    else if ( mType == Directory )
      selectedFile = QFileDialog::getExistingDirectory( this, 0, selectedFile );
    else
      selectedFile = QFileDialog::getOpenFileName( this, 0, selectedFile );

    lastDir = QFileInfo( selectedFile ).absolutePath();

    mLineEdit->setText( selectedFile );
  }
}

QString QgsGrassModuleFile::ready()
{
  QgsDebugMsg( "called." );

  QString error;
  QString path = mLineEdit->text().trimmed();


  if ( path.length() == 0 && mRequired )
  {
    error.append( tr( "%1:&nbsp;missing value" ).arg( title() ) );
    return error;
  }

  QFileInfo fi( path );
  if ( !fi.dir().exists() )
  {
    error.append( tr( "%1:&nbsp;directory does not exist" ).arg( title() ) );
  }

  return error;
}

QgsGrassModuleFile::~QgsGrassModuleFile()
{
}

/***************************** QgsGrassModuleCheckBox *********************************/

QgsGrassModuleCheckBox::QgsGrassModuleCheckBox( const QString & text, QWidget * parent )
    : QCheckBox( text, parent ), mText( text )
{
  QgsDebugMsg( "called." );
  adjustText();
}

QgsGrassModuleCheckBox::~QgsGrassModuleCheckBox()
{
}

void QgsGrassModuleCheckBox::resizeEvent( QResizeEvent * event )
{
  Q_UNUSED( event );
  adjustText();
}
void QgsGrassModuleCheckBox::setText( const QString & text )
{
  mText = text;
  adjustText();
}
void QgsGrassModuleCheckBox::setToolTip( const QString & text )
{
  mTip = text;
  QWidget::setToolTip( text );
}
void QgsGrassModuleCheckBox::adjustText()
{
  QString t = fontMetrics().elidedText( mText, Qt::ElideRight, width() - iconSize().width() - 20 );
  QCheckBox::setText( t );

  if ( mTip.isEmpty() )
  {
    QString tt;
    if ( t != mText )
    {
      tt = mText;
    }
    QWidget::setToolTip( tt );
  }
}

