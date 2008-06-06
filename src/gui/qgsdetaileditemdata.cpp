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
  mRenderAsWidgetFlag=false;
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

void QgsDetailedItemData::setIcon(QPixmap theIcon)
{
  mPixmap = theIcon;
}
void QgsDetailedItemData::setCheckable(bool theFlag)
{
  mCheckableFlag = theFlag;
}
void QgsDetailedItemData::setChecked(bool theFlag)
{
  mCheckedFlag = theFlag;
}
void QgsDetailedItemData::setRenderAsWidget(bool theFlag)
{
  mRenderAsWidgetFlag = theFlag;
}

QString QgsDetailedItemData::title()
{
  return mTitle;
}

QString QgsDetailedItemData::detail()
{
  return mDetail;
}

QPixmap QgsDetailedItemData::icon()
{
  return mPixmap;
}

bool QgsDetailedItemData::isCheckable()
{
  return mCheckableFlag;
}

bool QgsDetailedItemData::isChecked()
{
  return mCheckedFlag;
}

bool QgsDetailedItemData::isRenderedAsWidget()
{
  return mRenderAsWidgetFlag;
}

