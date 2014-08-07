#ifndef QGSLEGENDSYMBOLITEMV2_H
#define QGSLEGENDSYMBOLITEMV2_H

#include <QString>

class QgsSymbolV2;

class CORE_EXPORT QgsLegendSymbolItemV2
{
public:
  QgsLegendSymbolItemV2();
  //! construct item, takes ownership of symbol
  QgsLegendSymbolItemV2( QgsSymbolV2* s, const QString& lbl, const QString& k );
  ~QgsLegendSymbolItemV2();
  QgsLegendSymbolItemV2( const QgsLegendSymbolItemV2& other );
  QgsLegendSymbolItemV2& operator=( const QgsLegendSymbolItemV2& other );

  QgsSymbolV2* symbol; //!< owned by the struct
  QString label;
  QString key;           //!< identifier of the symbol item (within renderer)

  // additional data that may be used for filtering

  int scaleDenomMin;
  int scaleDenomMax;
};


typedef QList< QgsLegendSymbolItemV2 > QgsLegendSymbolListV2;

#endif // QGSLEGENDSYMBOLITEMV2_H
