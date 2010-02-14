/***************************************************************************
     qgsgeorefconfigdialog.h
     --------------------------------------
    Date                 : 14-Feb-2010
    Copyright            : (C) 2010 by Jack R, Maxim Dubinin (GIS-Lab)
    Email                : sim@gis-lab.info
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgspoint.h"
#include "qgsgeorefdatapoint.h"

#include "qgsgcplist.h"

QgsGCPList::QgsGCPList()
  : QList<QgsGeorefDataPoint *>()
{
}

QgsGCPList::QgsGCPList(const QgsGCPList &list)
{
  clear();
  QgsGCPList::const_iterator it = list.constBegin();
  for (; it != list.constEnd(); ++it)
  {
    QgsGeorefDataPoint *pt = new QgsGeorefDataPoint(**it);
    append(pt);
  }
}

void QgsGCPList::createGCPVectors(std::vector<QgsPoint> &mapCoords,
                                  std::vector<QgsPoint> &pixelCoords)
{
  mapCoords   = std::vector<QgsPoint>(size());
  pixelCoords = std::vector<QgsPoint>(size());
  for (int i = 0, j = 0; i < sizeAll(); i++)
  {
    QgsGeorefDataPoint *pt = at(i);
    if (pt->isEnabled())
    {
      mapCoords[j] = pt->mapCoords();
      pixelCoords[j] = pt->pixelCoords();
      j++;
    }
  }
}

int QgsGCPList::size() const
{
  if (QList<QgsGeorefDataPoint *>::isEmpty())
    return 0;

  int s = 0;
  const_iterator it = begin();
  while (it != end())
  {
    if ((*it)->isEnabled()) s++;
    it++;
  }
  return s;
}

int QgsGCPList::sizeAll() const
{
  return QList<QgsGeorefDataPoint *>::size();
}

QgsGCPList &QgsGCPList::operator =(const QgsGCPList &list)
{
  clear();
  QgsGCPList::const_iterator it = list.constBegin();
  for (; it != list.constEnd(); ++it)
  {
    QgsGeorefDataPoint *pt = new QgsGeorefDataPoint(**it);
    append(pt);
  }
  return *this;
}
