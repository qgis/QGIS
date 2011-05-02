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
#include <QBrush>
#include <QImage>
#include <QPen>
#include <QString>
#include <QXmlDefaultHandler>
#include <QXmlAttributes>


/**
 * Class to represent a paint rule.
 * Paint rule can be applied to all tags that will match it.
 */
class Rule
{
  public:
    /**
     * Construction.
     * @param pKey tag key for which this rule should be applied
     * @param pVal tag value for which this rule should be applied
     * @param pPen pen that should be used when applying this rule
     * @param pBrush brush that should be used when applying this rule
     * @param pImg image that should be used when applying this rule
     */
    Rule( QString pKey, QString pVal, QPen pPen, QBrush pBrush, QImage pImg )
        : key( pKey ), val( pVal ), pen( pPen ), brush( pBrush ), img( pImg ) {};

    //! tag key for which this rule should be applied
    QString key;

    //! tag value for which this rule should be applied
    QString val;

    //! pen that should be used when applying this rule
    QPen pen;

    //! brush that should be used when applying this rule
    QBrush brush;

    //! image that should be used when applying this rule
    QImage img;
};


/**
 * Class representing OSM stylesheet.
 */
class OsmStyle
{
  public:
    /**
     * Construction.
     * @param filename name of the file with stylesheet information
     */
    OsmStyle( QString filename );

    /**
     * Destruction.
     */
    ~OsmStyle();

    /**
     * Function to parse line of input file and create 'Line' rule in this way.
     * @param line line of input file
     */
    void parse_rule_line( QString line );

    /**
     * Function to parse line of input file and create 'Polygon' rule in this way.
     * @param line line of input file
     */
    void parse_rule_polygon( QString line );

    /**
     * Function to parse line of input file and create 'Point' rule in this way.
     * @param line line of input file
     */
    void parse_rule_point( QString line );

    /**
     * Function helps to display lines.
     * It gets list of all tags of some line, goes through all Line paint rules and returns pen,
     * that is most suitable for line with such tags.
     * @param line line of input file
     */
    QPen get_pen( QMap<QString, QString> tags );

    /**
     * Function helps to display polygons.
     * It gets list of all tags of some polygon, goes through all Polygon paint rules and returns pen and brush,
     * that are most suitable for polygon with such tags.
     * @param line line of input file
     */
    QPen get_pen_brush( QMap<QString, QString> tags, QBrush &brush );

    /**
     * Function helps to display points.
     * It gets list of all tags of some point, goes through all Point paint rules and returns image,
     * that is most suitable for point with such tags.
     * @param line line of input file
     */
    QImage get_image( QMap<QString, QString> tags );

  private:

    //! list of all rules for 'Lines'
    QList<Rule> rules_line;

    //! list of all rules for 'Polygon'
    QList<Rule> rules_polygon;

    //! list of all rules for 'Point'
    QList<Rule> rules_point;
};


