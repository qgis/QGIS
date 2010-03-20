
#include "qgssinglesymbolrendererv2.h"

#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"

#include <QDomDocument>
#include <QDomElement>

QgsSingleSymbolRendererV2::QgsSingleSymbolRendererV2( QgsSymbolV2* symbol )
    : QgsFeatureRendererV2( "singleSymbol" )
{
  mSymbol = symbol;
}

QgsSingleSymbolRendererV2::~QgsSingleSymbolRendererV2()
{
  delete mSymbol;
}

QgsSymbolV2* QgsSingleSymbolRendererV2::symbolForFeature( QgsFeature& feature )
{
  return mSymbol;
}

void QgsSingleSymbolRendererV2::startRender( QgsRenderContext& context, const QgsVectorLayer *vlayer )
{
  mSymbol->startRender( context );
}

void QgsSingleSymbolRendererV2::stopRender( QgsRenderContext& context )
{
  mSymbol->stopRender( context );
}

QList<QString> QgsSingleSymbolRendererV2::usedAttributes()
{
  return QList<QString>();
}

QgsSymbolV2* QgsSingleSymbolRendererV2::symbol() const
{
  return mSymbol;
}

void QgsSingleSymbolRendererV2::setSymbol( QgsSymbolV2* s )
{
  delete mSymbol;
  mSymbol = s;
}

QString QgsSingleSymbolRendererV2::dump()
{
  return QString( "SINGLE: %1" ).arg( mSymbol->dump() );
}

QgsFeatureRendererV2* QgsSingleSymbolRendererV2::clone()
{
  QgsSingleSymbolRendererV2* r = new QgsSingleSymbolRendererV2( mSymbol->clone() );
  r->setUsingSymbolLevels( usingSymbolLevels() );
  return r;
}

QgsSymbolV2List QgsSingleSymbolRendererV2::symbols()
{
  QgsSymbolV2List lst;
  lst.append( mSymbol );
  return lst;
}

QgsFeatureRendererV2* QgsSingleSymbolRendererV2::create( QDomElement& element )
{
  QDomElement symbolsElem = element.firstChildElement( "symbols" );
  if ( symbolsElem.isNull() )
    return NULL;

  QgsSymbolV2Map symbolMap = QgsSymbolLayerV2Utils::loadSymbols( symbolsElem );

  if ( !symbolMap.contains( "0" ) )
    return NULL;

  QgsSingleSymbolRendererV2* r = new QgsSingleSymbolRendererV2( symbolMap.take( "0" ) );

  // delete symbols if there are any more
  QgsSymbolLayerV2Utils::clearSymbolMap( symbolMap );

  // TODO: symbol levels
  return r;
}

QDomElement QgsSingleSymbolRendererV2::save( QDomDocument& doc )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( "type", "singleSymbol" );
  rendererElem.setAttribute( "symbollevels", ( mUsingSymbolLevels ? "1" : "0" ) );

  QgsSymbolV2Map symbols;
  symbols["0"] = mSymbol;
  QDomElement symbolsElem = QgsSymbolLayerV2Utils::saveSymbols( symbols, "symbols", doc );
  rendererElem.appendChild( symbolsElem );

  return rendererElem;
}

QgsLegendSymbologyList QgsSingleSymbolRendererV2::legendSymbologyItems( QSize iconSize )
{
  QPixmap pix = QgsSymbolLayerV2Utils::symbolPreviewPixmap( mSymbol, iconSize );

  QgsLegendSymbologyList lst;
  lst << qMakePair( QString(), pix );
  return lst;
}

QgsLegendSymbolList QgsSingleSymbolRendererV2::legendSymbolItems()
{
  QgsLegendSymbolList lst;
  lst << qMakePair( QString(), mSymbol );
  return lst;
}
