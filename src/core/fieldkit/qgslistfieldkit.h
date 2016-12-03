/***************************************************************************
  qgslistfieldkit.h - QgsListFieldKit

 ---------------------
 begin                : 3.12.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLISTFIELDKIT_H
#define QGSLISTFIELDKIT_H

#include "qgsfieldkit.h"

class QgsListFieldKit : public QgsFieldKit
{
  public:
    QgsListFieldKit();

    QString representValue( QgsVectorLayer* vl, int fieldIdx, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const override;
};

#endif // QGSLISTFIELDKIT_H
