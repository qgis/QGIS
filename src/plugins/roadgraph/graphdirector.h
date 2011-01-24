/***************************************************************************
  graphdirector.h
  --------------------------------------
  Date                 : 2010-10-18
  Copyright            : (C) 2010 by Yakushev Sergey
  Email                : YakushevS <at> list.ru
****************************************************************************
*                                                                          *
*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published by   *
*   the Free Software Foundation; either version 2 of the License, or      *
*   (at your option) any later version.                                    *
*                                                                          *
***************************************************************************/
#ifndef ROADGRAPH_GRAPHDIRECTOR
#define ROADGRAPH_GRAPHDIRECTOR

//QT4 includes

//QGIS includes
#include <qgsrectangle.h>

//forward declarations
class RgSettings;
class RgGraphBuilder;

/**
* \class RgGraphDirector
* \brief Determine making the graph
* contained the settings
*/
class RgGraphDirector
{
public:
  //! Destructor
  virtual ~RgGraphDirector() { };
  
  /**
   * get adjacency matrix
   */
  virtual void makeGraph( RgGraphBuilder * ) const = 0;
  
  /**
   * return pointer to my Settings
   */
  virtual RgSettings* settings() = 0;
  
  /**
   * return Director name
   */
  virtual QString name() const = 0;
};
#endif //GRAPHDIRECTOR
