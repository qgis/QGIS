/***************************************************************************
    qgsfontcombobox.cpp
     --------------------------------------
    Date                 : July 2026
    Copyright            : (C) 2026 Nyall Dawson
    Email                : nyall.dawson@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfontcombobox.h"

#include <QString>

#include "moc_qgsfontcombobox.cpp"

using namespace Qt::StringLiterals;

QgsFontComboBox::QgsFontComboBox( QWidget *parent )
  : QFontComboBox( parent )
{
  QFont f = font();
  // set font size to match QFontComboBox default:
  // https://github.com/openwebos/qt/blob/master/src/gui/widgets/qfontcombobox.cpp#L134C1-L135C1
  // yep, ¯\(°_o)/¯
  f.setPointSize( static_cast< int >( 3 / 2.0 * f.pointSize() ) );

  // these fonts have broken metadata -- they aren't correctly tagged as symbol fonts,
  // so QFontComboBox happily renders the font name in undecipherable symbol characters.
  // Yes, it's the fonts themselves which are broken, but given that these particular fonts
  // are used heavily in spatial contexts, its worth working around this!
  for ( const QString &family : {
          u"D050000L [urw]"_s,
          u"D050000L [URW ]"_s,
          u"D050000L"_s,
          u"ESRI AMFM Electric"_s,
          u"ESRI Cartography"_s,
          u"ESRI Caves 1"_s,
          u"ESRI Caves 2"_s,
          u"ESRI Caves 3"_s,
          u"ESRI Environmental & Icons"_s,
          u"ESRI Geology"_s,
          u"ESRI Geology USGS 95-525"_s,
          u"ESRI Geometric Symbols"_s,
          u"ESRI Hazardous Materials"_s,
          u"ESRI MilMod 01"_s,
          u"ESRI MilSym 04"_s,
          u"ESRI NIMA DNC PT"_s,
          u"ESRI Oil, Gas, & Water"_s,
          u"ESRI Pipeline US 1"_s,
          u"ESRI Public1"_s,
          u"ESRI SDS 1.95 1"_s,
          u"ESRI SDS 2.00 1"_s,
          u"ESRI Transportation & Civic"_s,
          u"ESRI US Forestry 1"_s,
          u"ESRI US MUTCD 1"_s,
          u"ESRI US MUTCD 2"_s,
          u"ESRI US MUTCD 3"_s,
          u"ESRI Weather"_s,
        } )
  {
    setDisplayFont( family, f );
  }
}
