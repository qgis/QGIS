/***************************************************************************
    osmstyle.cpp - Class representing OSM stylesheet.
    ------------------
    begin                : April 2009
    copyright            : (C) 2009 by Lukas Berka
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "osmstyle.h"
#include "symbology/qgssymbol.h"
#include <qgsapplication.h>

#include <qgslogger.h>

OsmStyle::OsmStyle( QString filename )
{
  rules_line.clear();
  rules_polygon.clear();
  rules_point.clear();

  QString rule_type = "unknown";
  QFile file( filename );

  if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    QgsDebugMsg( "failed to open style file: " + filename );
    return;
  }

  while ( !file.atEnd() )
  {
    QByteArray line_bytes = file.readLine();
    QString line = line_bytes.data();
    QgsDebugMsg( "line: " + line );
    if ( line.at( 0 ) == '#' )
      // change of rule type
      rule_type = line.mid( 1 ).trimmed();
    else
    {
      if ( rule_type == "LINE" )
        parse_rule_line( line );
      if ( rule_type == "POLYGON" )
        parse_rule_polygon( line );
      if ( rule_type == "POINT" )
        parse_rule_point( line );
    }
  }

  QgsDebugMsg( QString( "OSM style parsing done: %1 / %2 / %3" )
               .arg( rules_line.count() ).arg( rules_polygon.count() ).arg( rules_point.count() ) );
}


OsmStyle::~OsmStyle()
{
}


void OsmStyle::parse_rule_line( QString line )
{
  // split line into "key","val","width","color" parts
  QStringList line_parts = line.split( " " );
  QString key = line_parts[0];
  QString val = line_parts[1];
  QString width = line_parts[2];
  QString penStyle = line_parts[3];
  QString color = line_parts[4];

  // split color into its typical parts
  QStringList color_parts = color.split( "," );
  QString r = color_parts[0];
  QString g = color_parts[1];
  QString b = color_parts[2];

  // create pen
  QPen pen = QPen( QColor( r.toInt(), g.toInt(), b.toInt() ) );
  pen.setWidth( width.toFloat() );
  pen.setStyle(( Qt::PenStyle ) penStyle.toInt() );

  // add rule
  rules_line.append( Rule( key, val, pen, QBrush(), QImage() ) );
}


void OsmStyle::parse_rule_polygon( QString line )
{
  // split line into "key","val","width","color","fill" parts
  QStringList line_parts = line.split( " " );
  QString key = line_parts[0];
  QString val = line_parts[1];
  QString width = line_parts[2];
  QString penStyle = line_parts[3];
  QString color = line_parts[4];
  QString fill = line_parts[5];

  // split color into red, green and blue parts
  QStringList color_parts = color.split( "," );
  QString r = color_parts[0];
  QString g = color_parts[1];
  QString b = color_parts[2];

  // create pen
  QPen pen = QPen( QColor( r.toInt(), g.toInt(), b.toInt() ) );
  pen.setWidth( width.toFloat() );
  pen.setStyle(( Qt::PenStyle ) penStyle.toInt() );

  // split fill into red, green and blue parts
  color_parts = fill.split( "," );
  r = color_parts[0];
  g = color_parts[1];
  b = color_parts[2];
  QColor col( r.toInt(), g.toInt(), b.toInt(), 120 );

  QBrush brush = QBrush( col );
  brush.setStyle( Qt::SolidPattern );

  // add rule
  rules_polygon.append( Rule( key, val, pen, brush, QImage() ) );
}


void OsmStyle::parse_rule_point( QString line )
{
  // split line into "key","val","width","color" parts
  QStringList line_parts = line.split( " " );
  QString key = line_parts[0];
  QString val = line_parts[1];
  QString name = line_parts[2];
  QString size = line_parts[3];

  double widthScale = 1.0;
  bool selected = false;
  QColor mSelectionColor( 255, 255, 0 );

  QgsSymbol sym( QGis::Point );
  sym.setNamedPointSymbol( QString( "svg:%1%2" ).arg( QgsApplication::svgPath() ).arg( name ) );
  sym.setPointSize( size.toFloat() );

  QImage img = sym.getPointSymbolAsImage( widthScale, selected, mSelectionColor );

  // add rule
  rules_point.append( Rule( key, val, QPen(), QBrush(), img ) );
}


QPen OsmStyle::get_pen( QMap<QString, QString> tags )
{
  // go through rules one by one. the valid rule is applied
  for ( int i = 0; i < rules_line.size(); ++i )
  {
    const Rule& rule = rules_line.at( i );
    QString key = rule.key.trimmed();
    QString val = rule.val.trimmed();

    // todo: tmp comm, from python: if rule[0] == '*' or (tags.has_key(rule[0]) and (tags[rule[0]] == rule[1] or rule[1] == '*'))
    if (( key == "*" ) || (( tags.find( key ) != tags.end() ) && (( tags.value( key ) == rule.val ) || ( val == "*" ) ) ) )
    {
      return rule.pen;
    }
  }
  QgsDebugMsg( "not drawing." );
  return QPen( Qt::NoPen );
}


QPen OsmStyle::get_pen_brush( QMap<QString, QString> tags, QBrush &brush ) // todo: return both pen and brush
{
  // go through rules one by one. the valid rule is applied
  for ( int i = 0; i < rules_polygon.size(); ++i )
  {
    const Rule& rule = rules_polygon.at( i );
    QString key = rule.key.trimmed();
    QString val = rule.val.trimmed();

    // todo: tmp comm, from python: if rule[0] == '*' or (tags.has_key(rule[0]) and (tags[rule[0]] == rule[1] or rule[1] == '*'))
    if (( key == "*" ) || (( tags.find( key ) != tags.end() ) && (( tags.value( key ) == val ) || ( val == "*" ) ) ) )
    {
      brush = rule.brush;
      return rule.pen; // todo: and brush?
    }
  }
  brush = Qt::NoBrush;
  return QPen( Qt::NoPen ); // todo: and brush?
}


QImage OsmStyle::get_image( QMap<QString, QString> tags )
{
  // go through rules one by one. the valid rule is applied
  for ( int i = 0; i < rules_point.size(); ++i )
  {
    const Rule& rule = rules_point.at( i );
    // todo: tmp comm, from python: if rule[0] == '*' or (tags.has_key(rule[0]) and (tags[rule[0]] == rule[1] or rule[1] == '*'))
    if (( rule.key == "*" ) || (( tags.find( rule.key ) != tags.end() ) && (( tags.value( rule.key ) == rule.val ) || ( rule.val == "*" ) ) ) )
      return rule.img;
  }
  return QImage();
}


