/***************************************************************************
    wkbptr.h
    ---------------------
    begin                : Dezember 2012
    copyright            : (C) 2012 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************
 *                                                                         *
 * This file may be used under the terms of the GNU Lesser                 *
 * General Public License version 2.1 as published by the Free Software    *
 * Foundation and appearing in the file LICENSE.LGPL included in the       *
 * packaging of this file.  Please review the following information to     *
 * ensure the GNU Lesser General Public License version 2.1 requirements   *
 * will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.    *
 *                                                                         *
 ***************************************************************************/
#ifndef WKBPTR_H
#define WKBPTR_H

#include <QSharedData>

union wkbPtr
{
  void *vPtr;
  double *dPtr;
  int *iPtr;
  unsigned char *ucPtr;
  char *cPtr;

};

const int SDO_ARRAY_SIZE = 1024;

#define SDO_GTYPE_D(g)  (g/1000%10)
#define SDO_GTYPE_L(g)  (g/100%10)
#define SDO_GTYPE_TT(g) (g%100)
#define SDO_GTYPE(g,tt) (g*1000+tt)

enum SDO_GTYPE_TT
{
  gtUnknown = 0,
  gtPoint = 1,
  gtLine = 2,
  gtPolygon = 3,
  gtCollection = 4,
  gtMultiPoint = 5,
  gtMultiLine = 6,
  gtMultiPolygon = 7,
};


class QOCISpatialGeometry : public QSharedData
{
  public:
    bool isNull;
    int gtype;
    int srid;
    double x, y, z;

    QVector<int> eleminfo;
    QVector<double> ordinates;
};

Q_DECLARE_METATYPE( QOCISpatialGeometry );

#endif // WKBPTR_H
