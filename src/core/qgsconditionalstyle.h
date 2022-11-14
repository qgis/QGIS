/***************************************************************************
    qgsconditionalstyle.h
    ---------------------
    begin                : August 2015
    copyright            : (C) 2015 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCONDITIONALSTYLE_H
#define QGSCONDITIONALSTYLE_H

#include "qgis_core.h"
#include <QObject>
#include <QFont>
#include <QColor>
#include <QPixmap>
#include <QDomNode>
#include <QDomDocument>
#include <QHash>
#include <memory>

class QgsConditionalStyle;
class QgsReadWriteContext;
class QgsExpressionContext;
class QgsSymbol;

typedef QList<QgsConditionalStyle> QgsConditionalStyles;


/**
 * \ingroup core
 * \brief The QgsConditionalLayerStyles class holds conditional style information
 * for a layer. This includes field styles and full row styles.
 */
class CORE_EXPORT QgsConditionalLayerStyles : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsConditionalLayerStyles, with the specified \a parent object.
     */
    QgsConditionalLayerStyles( QObject *parent = nullptr );

    /**
     * Returns a list of row styles associated with the layer.
     *
     * \see setRowStyles()
     */
    QgsConditionalStyles rowStyles() const;

    /**
     * Sets the conditional \a styles that apply to full rows of data in the attribute table.
     * Each row will check be checked against each rule.
     *
     * \see rowStyles()
     * \since QGIS 2.12
     */
    void setRowStyles( const QgsConditionalStyles &styles );

    /**
     * Set the conditional \a styles for a field, with the specified \a fieldName.
     *
     * \see fieldStyles()
     */
    void setFieldStyles( const QString &fieldName, const QList<QgsConditionalStyle> &styles );

    /**
     * Returns the conditional styles set for the field with matching \a fieldName.
     *
     * \see setFieldStyles()
     */
    QList<QgsConditionalStyle> fieldStyles( const QString &fieldName ) const;

    /**
     * Reads the condition styles state from a DOM node.
     *
     * \see writeXml()
     */
    bool readXml( const QDomNode &node, const QgsReadWriteContext &context );

    /**
     * Writes the condition styles state to a DOM node.
     *
     * \see readXml()
     */
    bool writeXml( QDomNode &node, QDomDocument &doc, const QgsReadWriteContext &context ) const;

    /**
     * Returns TRUE if at least one rule needs geometry.
     * \since QGIS 3.26.3
     */
    bool rulesNeedGeometry() const;

  signals:

    /**
     * Emitted when the conditional styles are changed.
     *
     * \since QGIS 3.10
     */
    void changed();

  private:
    QHash<QString, QgsConditionalStyles> mFieldStyles;
    QgsConditionalStyles mRowStyles;
};

/**
 * \class QgsConditionalStyle
 *  \ingroup core
 * \brief Conditional styling for a rule.
 */
class CORE_EXPORT QgsConditionalStyle
{
  public:
    QgsConditionalStyle();
    QgsConditionalStyle( const QgsConditionalStyle &other );
    QgsConditionalStyle( const QString &rule );
    ~QgsConditionalStyle();

    QgsConditionalStyle &operator=( const QgsConditionalStyle &other );

    /**
     * \brief Check if the rule matches using the given value and feature
     * \param value The current value being checked. The "value" variable from the context is replaced with this value.
     * \param context Expression context for evaluating rule expression
     * \returns TRUE of the rule matches against the given feature
     */
    bool matches( const QVariant &value, QgsExpressionContext &context ) const;

    /**
     * \brief Render a preview icon of the rule, at the specified \a size.
     *
     * If \a size is not specified, a default size will be used.
     *
     * \returns QPixmap preview of the style
     */
    QPixmap renderPreview( const QSize &size = QSize() ) const;

    /**
     * \brief Set the name of the style.  Names are optional but handy for display
     * \param value The name given to the style
     */
    void setName( const QString &value ) { mName = value; mValid = true; }

    /**
     * \brief Set the rule for the style.  Rules should be of QgsExpression syntax.
     * Special value of \@value is replaced at run time with the check value
     * \param value The QgsExpression style rule to use for this style
     */
    void setRule( const QString &value ) { mRule = value; mValid = true; }

    /**
     * \brief Set the background color for the style
     * \param value QColor for background color
     */
    void setBackgroundColor( const QColor &value ) { mBackColor = value; mValid = true; }

    /**
     * \brief Set the text color for the style
     * \param value QColor for text color
     */
    void setTextColor( const QColor &value ) { mTextColor = value; mValid = true; }

    /**
     * \brief Set the font for the style
     * \param value QFont to be used for text
     */
    void setFont( const QFont &value ) { mFont = value; mValid = true; }

    /**
     * \brief Set the icon for the style. Icons are generated from symbols
     * \param value QgsSymbol to be used when generating the icon
     */
    void setSymbol( QgsSymbol *value );

    /**
     * \brief The name of the style.
     * \returns The name of the style. Names are optional so might be empty.
     */
    QString displayText() const;

    /**
     * \brief The name of the style.
     * \returns The name of the style. Names are optional so might be empty.
     */
    QString name() const { return mName; }

    /**
     * \brief The icon set for style generated from the set symbol
     * \returns A QPixmap that was set for the icon using the symbol
     */
    QPixmap icon() const { return mIcon; }

    /**
     * \brief The symbol used to generate the icon for the style
     * \returns The QgsSymbol used for the icon
     */
    QgsSymbol *symbol() const { return mSymbol.get(); }

    /**
     * \brief The text color set for style
     * \returns QColor for text color
     */
    QColor textColor() const { return mTextColor; }

    /**
     * \brief Check if the text color is valid for render.
     * Valid colors are non invalid QColors and a color with a > 0 alpha
     * \returns TRUE of the color set for text is valid.
     */
    bool validTextColor() const;

    /**
     * \brief The background color for style
     * \returns QColor for background color
     */
    QColor backgroundColor() const { return mBackColor; }

    /**
     * \brief Check if the background color is valid for render.
     * Valid colors are non invalid QColors and a color with a > 0 alpha
     * \returns TRUE of the color set for background is valid.
     */
    bool validBackgroundColor() const;

    /**
     * \brief The font for the style
     * \returns QFont for the style
     */
    QFont font() const { return mFont; }

    /**
     * \brief The condition rule set for the style. Rule may contain variable \@value
     * to represent the current value
     * \returns QString of the current set rule
     */
    QString rule() const { return mRule; }

    /**
     * \brief isValid Check if this rule is valid.  A valid rule has one or more properties
     * set.
     * \returns TRUE if the rule is valid.
     */
    bool isValid() const { return mValid; }

    /**
     * \brief Find and return the matching styles for the value and feature.
     * If no match is found a invalid QgsConditionalStyle is return.
     *
     * \returns A conditional style that matches the value and feature.
     * Check with QgsConditionalStyle::isValid()
     */
    static QList<QgsConditionalStyle> matchingConditionalStyles( const QList<QgsConditionalStyle> &styles, const QVariant &value, QgsExpressionContext &context );

    /**
     * \brief Find and return the matching style for the value and feature.
     * If no match is found a invalid QgsConditionalStyle is return.
     *
     * \returns A conditional style that matches the value and feature.
     * Check with QgsConditionalStyle::isValid()
     */
    static QgsConditionalStyle matchingConditionalStyle( const QList<QgsConditionalStyle> &styles, const QVariant &value, QgsExpressionContext &context );

    /**
     * \brief Compress a list of styles into a single style.  This can be used to stack the elements of the
     * styles. The font of the last style is used in the output.
     * \param styles The list of styles to compress down
     * \returns A single style generated from joining each style property.
     */
    static QgsConditionalStyle compressStyles( const QList<QgsConditionalStyle> &styles );

    /**
     * Reads vector conditional style specific state from layer Dom node.
     */
    bool readXml( const QDomNode &node, const QgsReadWriteContext &context );

    /**
     * Write vector conditional style specific state from layer Dom node.
     */
    bool writeXml( QDomNode &node, QDomDocument &doc, const QgsReadWriteContext &context ) const;

    bool operator==( const QgsConditionalStyle &other ) const;
    bool operator!=( const QgsConditionalStyle &other ) const;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str;
    if ( !sipCpp->name().isEmpty() )
      str = QStringLiteral( "<QgsConditionalStyle: '%1' (%2)>" ).arg( sipCpp->name(), sipCpp->rule() );
    else
      str = QStringLiteral( "<QgsConditionalStyle: %2>" ).arg( sipCpp->rule() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  private:

    bool mValid = false;
    QString mName;
    QString mRule;
    std::unique_ptr<QgsSymbol> mSymbol;
    QFont mFont;
    QColor mBackColor;
    QColor mTextColor;
    QPixmap mIcon;
};

#endif // QGSCONDITIONALSTYLE_H
