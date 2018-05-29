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
#include "qgis.h"

/**
 * \ingroup core
 * Composer legend components style
 */
class CORE_EXPORT QgsLegendStyle
{
  public:
    enum Style
    {
      Undefined, //!< Should not happen, only if corrupted project file
      Hidden, //!< Special style, item is hidden including margins around
      Title,
      Group,
      Subgroup, //!< Layer
      Symbol, //!< Symbol without label
      SymbolLabel
    };

    //! Margin side
    enum Side
    {
      Top = 0,
      Bottom = 1,
      Left = 2,
      Right = 3
    };

    QgsLegendStyle();

    /**
     * The font for this style.
     */
    QFont font() const { return mFont; }

    /**
     * The font for this style.
     */
    void setFont( const QFont &font ) { mFont = font; }

    /**
     * Modifiable reference to font.
     *
     * \see setFont()
     * \note Not available in Python bindings
     */
    SIP_SKIP QFont &rfont() { return mFont; }

    double margin( Side side ) { return mMarginMap.value( side ); }
    void setMargin( Side side, double margin ) { mMarginMap[side] = margin; }

    //! Sets all margins
    void setMargin( double margin );

    void writeXml( const QString &name, QDomElement &elem, QDomDocument &doc ) const;

    void readXml( const QDomElement &elem, const QDomDocument &doc );

    //! Gets name for style, used in project file
    static QString styleName( Style s );

    //! Gets style from name, used in project file
    static Style styleFromName( const QString &styleName );

    //! Gets style label, translated, used in UI
    static QString styleLabel( Style s );

  private:
    QFont mFont;
    //! Space around element
    QMap<Side, double> mMarginMap;
};

#endif
