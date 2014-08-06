#include "qgslegendsymbolitemv2.h"

#include "qgssymbolv2.h"

QgsLegendSymbolItemV2::QgsLegendSymbolItemV2()
  : symbol( 0 )
  , index( -1 )
{
}

QgsLegendSymbolItemV2::~QgsLegendSymbolItemV2()
{
  delete symbol;
}

QgsLegendSymbolItemV2::QgsLegendSymbolItemV2( const QgsLegendSymbolItemV2& other )
  : symbol( 0 )
  , index( -1 )
{
  *this = other;
}

QgsLegendSymbolItemV2& QgsLegendSymbolItemV2::operator=( const QgsLegendSymbolItemV2& other )
{
  if ( this == &other )
    return *this;

  delete symbol;
  symbol = other.symbol ? other.symbol->clone() : 0;
  label = other.label;
  index = other.index;

  return *this;
}
