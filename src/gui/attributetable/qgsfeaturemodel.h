/***************************************************************************
    qgsfeaturemodel.h
    ---------------------
    begin                : April 2013
    copyright            : (C) 2013 by Matthias Kuhn
    email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFEATUREMODEL_H
#define QGSFEATUREMODEL_H

#include "qgis_gui.h"
#include "qgsfeatureid.h"
#include <QModelIndex>

/**
 * \ingroup gui
 * \class QgsFeatureModel
 */
class GUI_EXPORT QgsFeatureModel
{
  public:
    virtual ~QgsFeatureModel() = default;

    virtual QModelIndex fidToIndex( QgsFeatureId fid ) = 0;
};

#endif // QGSFEATUREMODEL_H
