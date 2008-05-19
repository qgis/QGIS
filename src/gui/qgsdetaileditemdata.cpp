/***************************************************************************
     qgsdetailedlistdata.cpp  -  A data represenation for a rich QItemData subclass
                             -------------------
    begin                : Sat May 17 2008
    copyright            : (C) 2008 Tim Sutton
    email                : tim@linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id:$ */

#include "qgsdetaileditemdata.h"
QgsDetailedItemData::QgsDetailedItemData() 
{
}

QgsDetailedItemData::~QgsDetailedItemData()
{
}

void QgsDetailedItemData::setTitle(QString theTitle)
{
  mTitle=theTitle;
}

void QgsDetailedItemData::setDetail(QString theDetail)
{
  mDetail=theDetail;
}

QString QgsDetailedItemData::title()
{
  return mTitle;
}

QString QgsDetailedItemData::detail()
{
  return mDetail;
}
