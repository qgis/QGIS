#ifndef QGSLEGENDSYMBOLITEMV2_H
#define QGSLEGENDSYMBOLITEMV2_H

#include <QString>

class QgsSymbolV2;

class QgsLegendSymbolItemV2
{
public:
  QgsLegendSymbolItemV2();
  ~QgsLegendSymbolItemV2();
  QgsLegendSymbolItemV2( const QgsLegendSymbolItemV2& other );
  QgsLegendSymbolItemV2& operator=( const QgsLegendSymbolItemV2& other );

  QgsSymbolV2* symbol; //!< owned by the struct
  QString label;
  int index;           //!< identifier of the symbol item (within renderer)
  // TODO: scale range, rule
};


typedef QList< QgsLegendSymbolItemV2 > QgsLegendSymbolListV2;

#endif // QGSLEGENDSYMBOLITEMV2_H
