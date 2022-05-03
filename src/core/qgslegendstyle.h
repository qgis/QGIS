/***************************************************************************
                         qgslegendstyle.h
                         -------------------
    begin                : March 2013
    copyright            : (C) 2013 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLEGENDSTYLE_H
#define QGSLEGENDSTYLE_H

#include <QFont>
#include <QMap>
#include <QString>
#include <QDomElement>
#include <QDomDocument>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsreadwritecontext.h"

/**
 * \ingroup core
 * \brief Contains detailed styling information relating to how a layout legend should be rendered.
 */
class CORE_EXPORT QgsLegendStyle
{
  public:

    //! Component of legends which can be styled
    enum Style
    {
      Undefined, //!< Should not happen, only if corrupted project file
      Hidden, //!< Special style, item is hidden including margins around
      Title, //!< Legend title
      Group, //!< Legend group title
      Subgroup, //!< Legend subgroup title
      Symbol, //!< Symbol icon (excluding label)
      SymbolLabel, //!< Symbol label (excluding icon)
    };

    // TODO QGIS 4.0 - use Qt enum instead

    //! Margin sides
    enum Side
    {
      Top = 0, //!< Top side
      Bottom = 1, //!< Bottom side
      Left = 2, //!< Left side
      Right = 3, //!< Right side
    };

    QgsLegendStyle();

    /**
     * Returns the font used for rendering this legend component.
     * \see setFont()
     */
    QFont font() const { return mFont; }

    /**
     * Sets the \a font used for rendering this legend component.
     * \see font()
     */
    void setFont( const QFont &font ) { mFont = font; }

    /**
     * Returns a modifiable reference to the component's font.
     *
     * \see setFont()
     * \note Not available in Python bindings
     */
    SIP_SKIP QFont &rfont() { return mFont; }

    /**
     * Returns the margin (in mm) for the specified \a side of the component.
     *
     * \note Not all legend components respect all margin side settings!
     *
     * \see setMargin()
     */
    double margin( Side side ) { return mMarginMap.value( side ); }

    /**
     * Sets the \a margin (in mm) for the specified \a side of the component.
     *
     * \note Not all legend components respect all margin side settings!
     *
     * \see margin()
     */
    void setMargin( Side side, double margin ) { mMarginMap[side] = margin; }

    /**
     * Sets all margin sides to the same \a margin size (in mm).
     * \see margin()
     */
    void setMargin( double margin );

    /**
     * Returns the alignment for the legend component.
     *
     * \see setAlignment()
     * \since QGIS 3.10
     */
    Qt::Alignment alignment() const { return mAlignment; }

    /**
     * Sets the alignment for the legend component.
     *
     * \see alignment()
     * \since QGIS 3.10
     */
    void setAlignment( Qt::Alignment alignment ) { mAlignment = alignment; }

    /**
    * Returns the indent (in mm) of a group or subgroup.
    *
    * \see indent()
    * \since QGIS 3.22
    */
    double indent() const { return mIndent; }

    /**
     * Sets the indent (in mm) of a group or subgroup.
     *
     * \see indent()
     * \since QGIS 3.22
     */
    void setIndent( double indent ) { mIndent = indent; }

    /**
     * Writes the component's style definition to an XML element.
     * \see readXml()
     */
    void writeXml( const QString &name, QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context = QgsReadWriteContext() ) const;

    /**
     * Reads the component's style definition from an XML element.
     * \see writeXml()
     */
    void readXml( const QDomElement &elem, const QDomDocument &doc, const QgsReadWriteContext &context = QgsReadWriteContext() );

    /**
     * Returns the name for a style component as a string.
     *
     * This is a non-localised version, for internal use.
     *
     * \see styleFromName()
     * \see styleLabel()
     */
    static QString styleName( Style s );

    /**
     * Returns the style from name string.
     * \see styleName()
     */
    static Style styleFromName( const QString &styleName );

    /**
     * Returns a translated string representing a style component, for use in UI.
     * \see styleName()
     */
    static QString styleLabel( Style s );

  private:
    QFont mFont;
    QMap<Side, double> mMarginMap;
    Qt::Alignment mAlignment = Qt::AlignLeft;
    double mIndent = 0;
};

#endif
