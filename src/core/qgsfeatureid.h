/***************************************************************************
                      qgsfeatureid.h
                     --------------------------------------
Date                 : 3.9.2018
Copyright            : (C) 2018 by Matthias Kuhn
email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFEATUREID_H
#define QGSFEATUREID_H

#include <QSet>
#include "qgis_sip.h"

// feature id (currently 64 bit)

// 64 bit feature ids
typedef qint64 QgsFeatureId SIP_SKIP;
#define FID_NULL            std::numeric_limits<QgsFeatureId>::min()
#define FID_IS_NULL(fid)    ( fid == std::numeric_limits<QgsFeatureId>::min() )
#define FID_IS_NEW(fid)     ( fid < 0 && fid != std::numeric_limits<QgsFeatureId>::min() )
#define FID_TO_NUMBER(fid)  static_cast<qint64>( fid )
#define FID_TO_STRING(fid)  ( fid != std::numeric_limits<QgsFeatureId>::min() ? QString::number( fid ) : QStringLiteral( "NULL" ) )
#define STRING_TO_FID(str)  ( (str).toLongLong() )

#ifndef SIP_RUN
typedef QSet<QgsFeatureId> QgsFeatureIds;
#else
typedef QSet<qint64> QgsFeatureIds;
#endif

#endif
