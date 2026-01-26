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
#include <QVector>

union wkbPtr
{
    void *vPtr = nullptr;
    double *dPtr;
    int *iPtr;
    unsigned char *ucPtr;
    char *cPtr;
};

const int SDO_ARRAY_SIZE = 1024;

#define SDO_GTYPE_D( g ) ( g / 1000 % 10 )
#define SDO_GTYPE_L( g ) ( g / 100 % 10 )
#define SDO_GTYPE_TT( g ) ( g % 100 )
#define SDO_GTYPE( g, tt ) ( g * 1000 + tt )

enum SDO_GTYPE_TT
{
  GtUnknown = 0,
  GtPoint = 1,
  GtLine = 2,
  GtPolygon = 3,
  GtCollection = 4,
  GtMultiPoint = 5,
  GtMultiLine = 6,
  GtMultiPolygon = 7,
};


class QOCISpatialGeometry : public QSharedData
{
  public:
    QOCISpatialGeometry() = default;

    bool isNull = true;
    int gtype = -1;
    int srid = -1;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    QVector<int> eleminfo;
    QVector<double> ordinates;
};

Q_DECLARE_METATYPE( QOCISpatialGeometry );

#endif // WKBPTR_H
