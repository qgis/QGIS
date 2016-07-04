/***************************************************************************
                         qgscomposerlegendstyle.h
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

#ifndef QGSCOMPOSERLEGENDSTYLE_H
#define QGSCOMPOSERLEGENDSTYLE_H

#include <QFont>
#include <QMap>
#include <QString>
#include <QDomElement>
#include <QDomDocument>

/** \ingroup core
 * Composer legend components style
 */
class CORE_EXPORT QgsComposerLegendStyle
{
  public:
    enum Style
    {
      Undefined, // should not happen, only if corrupted project file
      Hidden, // special style, item is hidden includeing margins around
      Title,
      Group,
      Subgroup, // layer
      Symbol, // symbol without label
      SymbolLabel
    };
    enum Side // margin side
    {
      Top = 0,
      Bottom = 1,
      Left = 2,
      Right = 3
    };
    QgsComposerLegendStyle();

    QFont font() const { return mFont; }
    QFont & rfont() { return mFont; }
    void setFont( const QFont & font ) { mFont = font; }

    double margin( Side side ) { return mMarginMap.value( side ); }
    void setMargin( Side side, double margin ) { mMarginMap[side] = margin; }

    // set all margins
    void setMargin( double margin );

    void writeXML( const QString& name, QDomElement& elem, QDomDocument & doc ) const;

    void readXML( const QDomElement& elem, const QDomDocument& doc );

    /** Get name for style, used in project file */
    static QString styleName( Style s );

    /** Get style from name, used in project file */
    static Style styleFromName( const QString& styleName );

    /** Get style label, translated, used in UI */
    static QString styleLabel( Style s );

  private:
    QFont mFont;
    // Space around element
    QMap<Side, double> mMarginMap;
};

#endif
