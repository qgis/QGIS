
/**
 * The class stores information about one class/rule of a vector layer renderer in a unified way
 * that can be used by legend model for rendering of legend.
 *
 * @see QgsSymbolV2LegendNode
 * @note added in 2.6
 */
class QgsLegendSymbolItemV2
{
%TypeHeaderCode
#include <qgslegendsymbolitemv2.h>
%End

  public:
    QgsLegendSymbolItemV2();
    //! Construct item. Does not take ownership of symbol (makes internal clone)
    QgsLegendSymbolItemV2( QgsSymbolV2* symbol, const QString& label, const QString& ruleKey, bool checkable = false, int scaleMinDenom = -1, int scaleMaxDenom = -1, int level = 0 );
    ~QgsLegendSymbolItemV2();
    QgsLegendSymbolItemV2( const QgsLegendSymbolItemV2& other );
    //QgsLegendSymbolItemV2& operator=( const QgsLegendSymbolItemV2& other );

    //! Return associated symbol. May be null.
    QgsSymbolV2* symbol() const;
    //! Return text label
    QString label() const;
    //! Return unique identifier of the rule for identification of the item within renderer
    QString ruleKey() const;
    //! Return whether the item is user-checkable - whether renderer supports enabling/disabling it
    bool isCheckable() const;

    //! Used for older code that identifies legend entries from symbol pointer within renderer
    QgsSymbolV2* legacyRuleKey() const;

    //! Determine whether given scale is within the scale range. Returns true if scale or scale range is invalid (value <= 0)
    bool isScaleOK( double scale ) const;
    //! Min scale denominator of the scale range. For range 1:1000 to 1:2000 this will return 1000.
    //! Value <= 0 means the range is unbounded on this side
    int scaleMinDenom() const;
    //! Max scale denominator of the scale range. For range 1:1000 to 1:2000 this will return 2000.
    //! Value <= 0 means the range is unbounded on this side
    int scaleMaxDenom() const;

    //! Identation level that tells how deep the item is in a hierarchy of items. For flat lists level is 0
    int level() const;

};


typedef QList< QgsLegendSymbolItemV2 > QgsLegendSymbolListV2;
