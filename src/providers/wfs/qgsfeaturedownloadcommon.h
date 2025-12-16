/***************************************************************************
    qgsfeaturedownloadcommon.h
    --------------------------
    begin                : October 2019
    copyright            : (C) 2016-2019 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFEATUREDOWNLOADCOMMON_H
#define QGSFEATUREDOWNLOADCOMMON_H

#include "qgsfeature.h"

#include <QPair>
#include <QString>

//! Type that associate a QgsFeature to a (hopefully) unique id across requests
typedef QPair<QgsFeature, QString> QgsFeatureUniqueIdPair;

#endif // QGSFEATUREDOWNLOADCOMMON_H
