/***************************************************************************
    qgsgeometrycheckfactory.h
     --------------------------------------
    Date                 : September 2018
    Copyright            : (C) 2018 Matthias Kuhn
    Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRYCHECKFACTORY_H
#define QGSGEOMETRYCHECKFACTORY_H

#include <QString>
#include <QMap>

#include "qgis_sip.h"
#include "qgis_analysis.h"

class QgsGeometryCheck;

/**
 * \ingroup analysis
 */
class ANALYSIS_EXPORT QgsGeometryCheckFactory
{
  public:

    QgsGeometryCheckFactory();

    /**
     * Destructor
     *
     * Deletes all the registered checks
     */
    virtual ~QgsGeometryCheckFactory();

    virtual QgsGeometryCheck *createGeometryCheck() const = 0 SIP_FACTORY;

    virtual QString id() const = 0;
};

#endif // QGSGEOMETRYCHECKFACTORY_H
