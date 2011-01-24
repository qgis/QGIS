/***************************************************************************
 *   Copyright (C) 2010 by Sergey Yakushev                                 *
 *   yakushevs <at> list.ru                                                     *
 *                                                                         *
 *   This is file define Road graph plugins settings                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef ROADGRAPH_LINEVECTOR_SETTINGS
#define ROADGRAPH_LINEVECTOR_SETTINGS

#include "settings.h"

//  QT includes
#include <qstring.h>

// Qgis includes

// standart includes

// forward declaration Qgis-classes
class QWidget;

/**
@author Sergey Yakushev
*/
/**
 * \class RgSettings
 * \brief This class contained settings for RgLineVectorLayerDirector
 */

class RgLineVectorLayerSettings: public RgSettings
{
public:
  /**
   * \enum  DirectionType
   * \brief DirectionType enumeration discribe 
   */
  enum DirectionType {  FirstPointToLastPoint=1, LastPointToFirstPoint=2, Both=3 };
 
public:
  /**
   * default constructor.
   */
  RgLineVectorLayerSettings();
  
  /**
   * destructor
   */
  ~RgLineVectorLayerSettings();
public:
  /*
   * MANDATORY SETTINGS PROPERTY DECLARATIONS
   */
  void write( QgsProject * );
  void read( const QgsProject * );
  bool test();
  QWidget *getGui( QWidget* Parent );
  void    setFromGui( QWidget* );
public:

  /**
   * contained Layer name
   */
  QString mLayer;

  /**
   * contained direction field name
   */
  QString mDirection;
  
  /**
   * mDirection field value as first point to last point value
   */
  QString mFirstPointToLastPointDirectionVal;
  
  /**
   * mDirection field value as last point to first point value
   */
  QString mLastPointToFirstPointDirectionVal;
  
  /**
   * mDirection field value as both direction
   */
  QString mBothDirectionVal;

  /**
   * contained Default direction
   */
  DirectionType mDefaultDirection;

  /**
   * contained speed filed name
   */
  QString mSpeed;

  /**
   * сontained default speed value;
   */
  double mDefaultSpeed;
  
  /*
   * name of speed unit
   */
  QString mSpeedUnitName;
};
#endif
