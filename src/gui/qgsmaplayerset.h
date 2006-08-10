/***************************************************************************
    qgsmaplayerset.h  -  holds a set of layers
    ----------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */


#ifndef QGSMAPLAYERSET_H
#define QGSMAPLAYERSET_H

#include "qgsrect.h"
#include <deque>
#include <QString>


class QgsMapLayerSet
{
  public:
    //! returns current layer set
    std::deque<QString>& layerSet() { return mLayerSet; }
    
    //! returns number of layers in layer set
    int layerCount() { return mLayerSet.size(); }
    
    //! returns current extent of layer set
    QgsRect fullExtent() { updateFullExtent(); return mFullExtent; }
    
    //! change current layer set
    void setLayerSet(const std::deque<QString>& layers);
  
    //! mainly for internal use, called directly only when projection changes
    void updateFullExtent();
    
  private:
    
    //! stores array of layers (identified by string)
    std::deque<QString> mLayerSet;
    
    //! full extent of the layer set
    QgsRect mFullExtent;
};

#endif
