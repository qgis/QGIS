/***************************************************************************
    osmstyle.h - Class representing OSM stylesheet.
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

#include <QFile>
#include <QMap>
#include <QProgressDialog>
#include <QString>
#include <QPen>
#include <QXmlDefaultHandler>
#include <QXmlAttributes>
#include <iostream>

using namespace std;


/**
 * Class to represent a paint rule.
 */
class Rule
{
  public:
    // construction, destruction
    Rule( QString pKey, QString pVal, QPen pPen, QBrush pBrush, QImage pImg )
        : key( pKey ), val( pVal ), pen( pPen ), brush( pBrush ), img( pImg ) {};

    // class members
    QString key;
    QString val;
    QPen pen;
    QBrush brush;
    QImage img;
};


/**
 * Class representing OSM stylesheet.
 */
class OsmStyle
{
  public:
    OsmStyle( QString filename );
    ~OsmStyle();

    QList<Rule> rules_line;
    QList<Rule> rules_polygon;
    QList<Rule> rules_point;


    void parse_rule_line( QString line );

    void parse_rule_polygon( QString line );

    void parse_rule_point( QString line );

    QPen get_pen( QMap<QString, QString> tags );

    QPen get_pen_brush( QMap<QString, QString> tags, QBrush &brush ); // todo: return both pen and brush

    QImage get_image( QMap<QString, QString> tags );
};


