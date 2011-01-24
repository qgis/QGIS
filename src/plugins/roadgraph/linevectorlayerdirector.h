/***************************************************************************
  linevectorlayerdirector.h
  --------------------------------------
  Date                 : 2010-10-20
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
#ifndef ROADGRAPH_LINEVECTORLAYERDIRECTOR
#define ROADGRAPH_LINEVECTORLAYERDIRECTOR

//QT4 includes

//QGIS includes

// Road-graph plugin includes
#include "graphdirector.h"
#include "linevectorlayersettings.h"

//forward declarations
class RgGraphBuilder;

/**
* \class RgLineVectorLayerDirector
* \brief Determine making the graph from vector line layer
*/
class RgLineVectorLayerDirector : public RgGraphDirector
{
public:
  RgLineVectorLayerDirector();
  //! Destructor
  virtual ~RgLineVectorLayerDirector();
  /**
   * MANDATORY DIRECTOR PROPERTY DECLARATION
   */
  void makeGraph( RgGraphBuilder * ) const;
  
  RgSettings* settings();

  QString name() const;
private:
  /**
   * settings of this director
   */
  RgLineVectorLayerSettings mSettings;
};
#endif //GRAPHDIRECTOR
