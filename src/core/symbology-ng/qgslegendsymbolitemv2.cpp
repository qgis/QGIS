#include "qgslegendsymbolitemv2.h"

#include "qgssymbolv2.h"

QgsLegendSymbolItemV2::QgsLegendSymbolItemV2()
  : symbol( 0 )
  , index( -1 )
  , scaleDenomMin( -1 )
  , scaleDenomMax( -1 )
{
}

QgsLegendSymbolItemV2::QgsLegendSymbolItemV2( QgsSymbolV2* s, const QString& lbl, int idx )
  : symbol( s )
  , label( lbl )
  , index( idx )
  , scaleDenomMin( -1 )
  , scaleDenomMax( -1 )
{
}

QgsLegendSymbolItemV2::QgsLegendSymbolItemV2( const QgsLegendSymbolItemV2& other )
  : symbol( 0 )
  , index( -1 )
  , scaleDenomMin( -1 )
  , scaleDenomMax( -1 )
{
  *this = other;
}

QgsLegendSymbolItemV2::~QgsLegendSymbolItemV2()
{
  delete symbol;
}

QgsLegendSymbolItemV2& QgsLegendSymbolItemV2::operator=( const QgsLegendSymbolItemV2& other )
{
  if ( this == &other )
    return *this;

  delete symbol;
  symbol = other.symbol ? other.symbol->clone() : 0;
  label = other.label;
  index = other.index;
  scaleDenomMin = other.scaleDenomMin;
  scaleDenomMax = other.scaleDenomMax;

  return *this;
}
