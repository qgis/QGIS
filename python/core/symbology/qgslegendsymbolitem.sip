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

.. seealso:: QgsSymbolLegendNode
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
Return associated symbol. May be null.
 :rtype: QgsSymbol
%End
    QString label() const;
%Docstring
Return text label
 :rtype: str
%End
    QString ruleKey() const;
%Docstring
Return unique identifier of the rule for identification of the item within renderer
 :rtype: str
%End
    bool isCheckable() const;
%Docstring
Return whether the item is user-checkable - whether renderer supports enabling/disabling it
 :rtype: bool
%End

    QgsSymbol *legacyRuleKey() const;
%Docstring
Used for older code that identifies legend entries from symbol pointer within renderer
 :rtype: QgsSymbol
%End

    bool isScaleOK( double scale ) const;
%Docstring
Determine whether given scale is within the scale range. Returns true if scale or scale range is invalid (value <= 0)
 :rtype: bool
%End

    int scaleMinDenom() const;
%Docstring
 Min scale denominator of the scale range. For range 1:1000 to 1:2000 this will return 1000.
 Value <= 0 means the range is unbounded on this side
 :rtype: int
%End

    int scaleMaxDenom() const;
%Docstring
 Max scale denominator of the scale range. For range 1:1000 to 1:2000 this will return 2000.
 Value <= 0 means the range is unbounded on this side
 :rtype: int
%End

    int level() const;
%Docstring
Identation level that tells how deep the item is in a hierarchy of items. For flat lists level is 0
 :rtype: int
%End

    QString parentRuleKey() const;
%Docstring
 Key of the parent legend node. For legends with tree hierarchy
.. note::

   Parameter parentRuleKey added in QGIS 2.8
 :rtype: str
%End

    void setSymbol( QgsSymbol *s /Transfer/ );
%Docstring
Set symbol of the item. Takes ownership of symbol.
%End

    void setDataDefinedSizeLegendSettings( QgsDataDefinedSizeLegend *settings /Transfer/ );
%Docstring
 Sets extra information about data-defined size. If set, this item should be converted to QgsDataDefinedSizeLegendNode
 rather than QgsSymbolLegendNode instance as usual. Passing null removes any data-defined size legend settings.

 Takes ownership of the settings object.
.. versionadded:: 3.0
%End

    QgsDataDefinedSizeLegend *dataDefinedSizeLegendSettings() const;
%Docstring
 Returns extra information for data-defined size legend rendering. Normally it returns null.
.. versionadded:: 3.0
 :rtype: QgsDataDefinedSizeLegend
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
