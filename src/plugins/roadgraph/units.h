/***************************************************************************
 *   Copyright (C) 2010 by Sergey Yakushev                                 *
 *   yakushevs@list.ru                                                     *
 *                                                                         *
 *   This is file define vrp plugins time, distance and speed units        *
 *   classes                                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef ROADGRAPH_UNITS_H
#define ROADGRAPH_UNITS_H

//#include <vrpguibase.h>
//  QT includes
#include <qstring.h>

// Qgis includes

// forward declaration Qgis-classes

/**
@author Sergey Yakushev
*/
/**
 * \class Unit
 * \brief This class provide interface to access unit name and multipler.
 * You can use it for convert units to metric system
 */
class Unit
{
  public:
    /**
     * default constructor
     */
    Unit();

    /**
     * constructor
     */
    Unit( const QString& name, double multipler );

    /**
     * return unit name
     */
    QString name() const;

    /**
     * return unit multipler. You can use multipler to conver unit to metric system
     */
    double multipler() const;

    /**
     * return unit by name
     */
    static Unit byName( const QString& name );
  private:

    /**
     * units name
     */
    QString mName;

    /**
     * units multipler
     */
    double mMultipler;
};

class SpeedUnit
{
  public:
    SpeedUnit();
    SpeedUnit( const Unit& distanceUnit, const Unit& timeUnit );

    QString name() const;
    double multipler() const;

    Unit timeUnit() const;
    Unit distanceUnit() const;

    static SpeedUnit byName( const QString& name );
  protected:
    Unit mTimeUnit;
    Unit mDistanceUnit;
};
#endif
