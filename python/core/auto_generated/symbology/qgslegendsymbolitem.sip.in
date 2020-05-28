/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgslegendsymbolitem.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLegendSymbolItem
{
%Docstring
The class stores information about one class/rule of a vector layer renderer in a unified way
that can be used by legend model for rendering of legend.

.. seealso:: :py:class:`QgsSymbolLegendNode`

.. versionadded:: 2.6
%End

%TypeHeaderCode
#include "qgslegendsymbolitem.h"
%End
  public:

    QgsLegendSymbolItem();
%Docstring
Constructor for QgsLegendSymbolItem.
%End

    QgsLegendSymbolItem( QgsSymbol *symbol, const QString &label, const QString &ruleKey, bool checkable = false, int scaleMinDenom = -1, int scaleMaxDenom = -1, int level = 0, const QString &parentRuleKey = QString() );
%Docstring
Construct item. Does not take ownership of symbol (makes internal clone)

.. versionadded:: 2.8
%End
    ~QgsLegendSymbolItem();

    QgsLegendSymbolItem( const QgsLegendSymbolItem &other );

    QgsSymbol *symbol() const;
%Docstring
Returns associated symbol. May be ``None``.
%End
    QString label() const;
%Docstring
Returns text label
%End
    QString ruleKey() const;
%Docstring
Returns unique identifier of the rule for identification of the item within renderer
%End
    bool isCheckable() const;
%Docstring
Returns whether the item is user-checkable - whether renderer supports enabling/disabling it
%End

    QgsSymbol *legacyRuleKey() const;
%Docstring
Used for older code that identifies legend entries from symbol pointer within renderer
%End

    bool isScaleOK( double scale ) const;
%Docstring
Determine whether given scale is within the scale range. Returns ``True`` if scale or scale range is invalid (value <= 0)
%End

    int scaleMinDenom() const;
%Docstring
Min scale denominator of the scale range. For range 1:1000 to 1:2000 this will return 1000.
Value <= 0 means the range is unbounded on this side
%End

    int scaleMaxDenom() const;
%Docstring
Max scale denominator of the scale range. For range 1:1000 to 1:2000 this will return 2000.
Value <= 0 means the range is unbounded on this side
%End

    int level() const;
%Docstring
Indentation level that tells how deep the item is in a hierarchy of items. For flat lists level is 0
%End

    QString parentRuleKey() const;
%Docstring
Key of the parent legend node. For legends with tree hierarchy

.. note::

   Parameter parentRuleKey added in QGIS 2.8
%End

    void setSymbol( QgsSymbol *s );
%Docstring
Sets the symbol of the item.

Does not take ownership of symbol -- an internal clone is made of the symbol.

.. seealso:: :py:func:`symbol`
%End

    void setDataDefinedSizeLegendSettings( QgsDataDefinedSizeLegend *settings /Transfer/ );
%Docstring
Sets extra information about data-defined size. If set, this item should be converted to :py:class:`QgsDataDefinedSizeLegendNode`
rather than QgsSymbolLegendNode instance as usual. Passing ``None`` removes any data-defined size legend settings.

Takes ownership of the settings object.

.. versionadded:: 3.0
%End

    QgsDataDefinedSizeLegend *dataDefinedSizeLegendSettings() const;
%Docstring
Returns extra information for data-defined size legend rendering. Normally it returns ``None``.

.. versionadded:: 3.0
%End

};

typedef QList< QgsLegendSymbolItem > QgsLegendSymbolList;

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgslegendsymbolitem.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
