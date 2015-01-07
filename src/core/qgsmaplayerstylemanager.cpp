#include "qgsmaplayerstylemanager.h"

#include "qgsmaplayer.h"

#include <QDomElement>
#include <QTemporaryFile>
#include <QTextStream>

QgsMapLayerStyleManager::QgsMapLayerStyleManager( QgsMapLayer* layer )
    : mLayer( layer )
{
  QgsMapLayerStyle defaultStyle;
  defaultStyle.loadFromLayer( mLayer );
  mStyles.insert( QString(), defaultStyle );
}

void QgsMapLayerStyleManager::readXml( const QDomElement& mgrElement )
{
  mCurrentStyle = mgrElement.attribute( "current" );

  mStyles.clear();
  QDomElement ch = mgrElement.firstChildElement( "map-layer-style" );
  while ( !ch.isNull() )
  {
    QString name = ch.attribute( "name" );
    QgsMapLayerStyle style;
    style.readXml( ch );
    mStyles.insert( name, style );

    ch = ch.nextSiblingElement( "map-layer-style" );
  }
}

void QgsMapLayerStyleManager::writeXml( QDomElement& mgrElement ) const
{
  const_cast<QgsMapLayerStyleManager*>( this )->syncCurrentStyle();

  QDomDocument doc = mgrElement.ownerDocument();
  mgrElement.setAttribute( "current", mCurrentStyle );

  foreach ( const QString& name, styles() )
  {
    QDomElement ch = doc.createElement( "map-layer-style" );
    ch.setAttribute( "name", name );
    mStyles[name].writeXml( ch );
    mgrElement.appendChild( ch );
  }
}

QStringList QgsMapLayerStyleManager::styles() const
{
  return mStyles.keys();
}

QgsMapLayerStyle QgsMapLayerStyleManager::style( const QString& name ) const
{
  if ( name == mCurrentStyle ) // make sure it is sync'ed
    const_cast<QgsMapLayerStyleManager*>( this )->syncCurrentStyle();

  return mStyles.value( name );
}

bool QgsMapLayerStyleManager::addStyle( const QString& name, const QgsMapLayerStyle& style )
{
  if ( mStyles.contains( name ) )
    return false;
  if ( !style.isValid() )
    return false;

  mStyles.insert( name, style );
  return true;
}

bool QgsMapLayerStyleManager::addStyleFromLayer( const QString& name )
{
  QgsMapLayerStyle style;
  style.loadFromLayer( mLayer );
  return addStyle( name, style );
}

bool QgsMapLayerStyleManager::removeStyle( const QString& name )
{
  if ( !mStyles.contains( name ) )
    return false;
  if ( mStyles.count() == 1 )
    return false; // cannot remove the last one

  // change to a different style if this one is the current
  if ( mCurrentStyle == name )
  {
    QStringList keys = mStyles.keys();
    QString newCurrent = keys[0];
    if ( newCurrent == name )
      newCurrent = keys[1]; // there must be at least one more
    setCurrentStyle( newCurrent );
  }

  mStyles.remove( name );
  return true;
}

QString QgsMapLayerStyleManager::currentStyle() const
{
  return mCurrentStyle;
}

bool QgsMapLayerStyleManager::setCurrentStyle( const QString& name )
{
  if ( !mStyles.contains( name ) )
    return false;

  if ( mCurrentStyle == name )
    return true; // nothing to do

  syncCurrentStyle(); // sync before unloading it
  mCurrentStyle = name;
  mStyles[mCurrentStyle].applyToLayer( mLayer );
  return true;
}

void QgsMapLayerStyleManager::syncCurrentStyle()
{
  mStyles[mCurrentStyle].loadFromLayer( mLayer );
}

// -----

QgsMapLayerStyle::QgsMapLayerStyle()
{
}

bool QgsMapLayerStyle::isValid() const
{
  return !mXmlData.isEmpty();
}

QString QgsMapLayerStyle::dump() const
{
  return mXmlData;
}

void QgsMapLayerStyle::loadFromLayer( QgsMapLayer* layer )
{
  QString errorMsg;
  QDomDocument doc;
  layer->exportNamedStyle( doc, errorMsg );
  if ( !errorMsg.isEmpty() )
  {
    qDebug( " ERR: %s", errorMsg.toAscii().data() );
    return;
  }

  QTextStream stream( &mXmlData );
  doc.save( stream, 0 );
}

void QgsMapLayerStyle::applyToLayer( QgsMapLayer* layer ) const
{
  // QgsMapLayer does not have a importNamedStyle() method - working it around like this
  QTemporaryFile f;
  f.open();
  f.write( mXmlData );
  f.flush();

  bool res;
  QString status = layer->loadNamedStyle( f.fileName(), res );
  qDebug( "APPLY: %s", status.toAscii().data() );
  if ( !res )
    qDebug( " APPLY ERR!" );
}

void QgsMapLayerStyle::readXml( const QDomElement& styleElement )
{
  QTextStream stream( &mXmlData );
  styleElement.firstChildElement().save( stream, 0 );
}

void QgsMapLayerStyle::writeXml( QDomElement& styleElement ) const
{
  QDomDocument docX;
  docX.setContent( mXmlData );
  styleElement.appendChild( docX.documentElement() );
}
