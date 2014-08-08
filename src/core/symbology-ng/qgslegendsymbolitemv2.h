#ifndef QGSLEGENDSYMBOLITEMV2_H
#define QGSLEGENDSYMBOLITEMV2_H

#include <QString>

class QgsSymbolV2;

/**
 * The class stores information about one class/rule of a vector layer renderer in a unified way
 * that can be used by legend model for rendering of legend.
 *
 * @note added in 2.6
 */
class CORE_EXPORT QgsLegendSymbolItemV2
{
  public:
    QgsLegendSymbolItemV2();
    //! construct item, does not take ownership of symbol (makes internal clone)
    QgsLegendSymbolItemV2( QgsSymbolV2* symbol, const QString& label, const QString& ruleKey, int scaleMinDenom = -1, int scaleMaxDenom = -1 );
    ~QgsLegendSymbolItemV2();
    QgsLegendSymbolItemV2( const QgsLegendSymbolItemV2& other );
    QgsLegendSymbolItemV2& operator=( const QgsLegendSymbolItemV2& other );

    QgsSymbolV2* symbol() const { return mSymbol; }
    QString label() const { return mLabel; }
    QString ruleKey() const { return mKey; }

    //! used for older code that identifies legend entries from symbol pointer within renderer
    QgsSymbolV2* legacyRuleKey() const { return mOriginalSymbolPointer; }

    bool isScaleOK( double scale ) const;
    int scaleMinDenom() const { return mScaleMinDenom; }
    int scaleMaxDenom() const { return mScaleMaxDenom; }

    //! takes ownership of symbol
    void setSymbol( QgsSymbolV2* s );
    void setLabel( const QString& label ) { mLabel = label; }

  private:
    //! symbol. owned by the struct. can be null.
    QgsSymbolV2* mSymbol;
    //! label of the item (may be empty or non-unique)
    QString mLabel;
    //! unique identifier of the symbol item (within renderer)
    QString mKey;

    QgsSymbolV2* mOriginalSymbolPointer;

    // additional data that may be used for filtering

    int mScaleMinDenom;
    int mScaleMaxDenom;
};


typedef QList< QgsLegendSymbolItemV2 > QgsLegendSymbolListV2;

#endif // QGSLEGENDSYMBOLITEMV2_H
