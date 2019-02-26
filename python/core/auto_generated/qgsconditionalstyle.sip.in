/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsconditionalstyle.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




typedef QList<QgsConditionalStyle> QgsConditionalStyles;


class QgsConditionalLayerStyles
{
%Docstring
The QgsConditionalLayerStyles class holds conditional style information
for a layer. This includes field styles and full row styles.
%End

%TypeHeaderCode
#include "qgsconditionalstyle.h"
%End
  public:
    QgsConditionalLayerStyles();

    QList<QgsConditionalStyle> rowStyles();

    void setRowStyles( const QList<QgsConditionalStyle> &styles );
%Docstring
Set the conditional styles that apply to full rows of data in the attribute table.
Each row will check be checked against each rule.

:param styles: The styles to assign to all the rows

.. versionadded:: 2.12
%End

    void setFieldStyles( const QString &fieldName, const QList<QgsConditionalStyle> &styles );
%Docstring
Set the conditional styles for the field UI properties.

:param fieldName: name of field
:param styles:
%End

    QList<QgsConditionalStyle> fieldStyles( const QString &fieldName );
%Docstring
Returns the conditional styles set for the field UI properties

:return: A list of conditional styles that have been set.
%End

    bool readXml( const QDomNode &node, const QgsReadWriteContext &context );
%Docstring
Reads field ui properties specific state from Dom node.
%End

    bool writeXml( QDomNode &node, QDomDocument &doc, const QgsReadWriteContext &context ) const;
%Docstring
Write field ui properties specific state from Dom node.
%End

};

class QgsConditionalStyle
{
%Docstring
Conditional styling for a rule.
%End

%TypeHeaderCode
#include "qgsconditionalstyle.h"
%End
  public:
    QgsConditionalStyle();
    QgsConditionalStyle( const QgsConditionalStyle &other );
    QgsConditionalStyle( const QString &rule );


    bool matches( const QVariant &value, QgsExpressionContext &context ) const;
%Docstring
Check if the rule matches using the given value and feature

:param value: The current value being checked. The "value" variable from the context is replaced with this value.
:param context: Expression context for evaluating rule expression

:return: ``True`` of the rule matches against the given feature
%End

    QPixmap renderPreview() const;
%Docstring
Render a preview icon of the rule.

:return: QPixmap preview of the style
%End

    void setName( const QString &value );
%Docstring
Set the name of the style.  Names are optional but handy for display

:param value: The name given to the style
%End

    void setRule( const QString &value );
%Docstring
Set the rule for the style.  Rules should be of QgsExpression syntax.
Special value of \@value is replaced at run time with the check value

:param value: The QgsExpression style rule to use for this style
%End

    void setBackgroundColor( const QColor &value );
%Docstring
Set the background color for the style

:param value: QColor for background color
%End

    void setTextColor( const QColor &value );
%Docstring
Set the text color for the style

:param value: QColor for text color
%End

    void setFont( const QFont &value );
%Docstring
Set the font for the style

:param value: QFont to be used for text
%End

    void setSymbol( QgsSymbol *value );
%Docstring
Set the icon for the style. Icons are generated from symbols

:param value: QgsSymbol to be used when generating the icon
%End

    QString displayText() const;
%Docstring
The name of the style.

:return: The name of the style. Names are optional so might be empty.
%End

    QString name() const;
%Docstring
The name of the style.

:return: The name of the style. Names are optional so might be empty.
%End

    QPixmap icon() const;
%Docstring
The icon set for style generated from the set symbol

:return: A QPixmap that was set for the icon using the symbol
%End

    QgsSymbol *symbol() const;
%Docstring
The symbol used to generate the icon for the style

:return: The QgsSymbol used for the icon
%End

    QColor textColor() const;
%Docstring
The text color set for style

:return: QColor for text color
%End

    bool validTextColor() const;
%Docstring
Check if the text color is valid for render.
Valid colors are non invalid QColors and a color with a > 0 alpha

:return: ``True`` of the color set for text is valid.
%End

    QColor backgroundColor() const;
%Docstring
The background color for style

:return: QColor for background color
%End

    bool validBackgroundColor() const;
%Docstring
Check if the background color is valid for render.
Valid colors are non invalid QColors and a color with a > 0 alpha

:return: ``True`` of the color set for background is valid.
%End

    QFont font() const;
%Docstring
The font for the style

:return: QFont for the style
%End

    QString rule() const;
%Docstring
The condition rule set for the style. Rule may contain variable \@value
to represent the current value

:return: QString of the current set rule
%End

    bool isValid() const;
%Docstring
isValid Check if this rule is valid.  A valid rule has one or more properties
set.

:return: ``True`` if the rule is valid.
%End

    static QList<QgsConditionalStyle> matchingConditionalStyles( const QList<QgsConditionalStyle> &styles, const QVariant &value, QgsExpressionContext &context );
%Docstring
Find and return the matching styles for the value and feature.
If no match is found a invalid QgsConditionalStyle is return.

:return: A conditional style that matches the value and feature.
         Check with :py:func:`QgsConditionalStyle.isValid()`
%End

    static QgsConditionalStyle matchingConditionalStyle( const QList<QgsConditionalStyle> &styles, const QVariant &value, QgsExpressionContext &context );
%Docstring
Find and return the matching style for the value and feature.
If no match is found a invalid QgsConditionalStyle is return.

:return: A conditional style that matches the value and feature.
         Check with :py:func:`QgsConditionalStyle.isValid()`
%End

    static QgsConditionalStyle compressStyles( const QList<QgsConditionalStyle> &styles );
%Docstring
Compress a list of styles into a single style.  This can be used to stack the elements of the
styles. The font of the last style is used in the output.

:param styles: The list of styles to compress down

:return: A single style generated from joining each style property.
%End

    bool readXml( const QDomNode &node, const QgsReadWriteContext &context );
%Docstring
Reads vector conditional style specific state from layer Dom node.
%End

    bool writeXml( QDomNode &node, QDomDocument &doc, const QgsReadWriteContext &context ) const;
%Docstring
Write vector conditional style specific state from layer Dom node.
%End


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsconditionalstyle.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
