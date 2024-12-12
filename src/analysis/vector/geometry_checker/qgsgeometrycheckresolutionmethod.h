/***************************************************************************
    qgsgeometrycheckresolutionmethod.h
     --------------------------------------
    Date                 : January 2020
    Copyright            : (C) 2020 Matthias Kuhn
    Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRYCHECKRESOLUTIONMETHOD_H
#define QGSGEOMETRYCHECKRESOLUTIONMETHOD_H

#include <QString>
#include "qgis_analysis.h"

/**
 * \ingroup analysis
 * \brief This class implements a resolution for problems detected in geometry checks.
 *
 * \since QGIS 3.12
 */
class ANALYSIS_EXPORT QgsGeometryCheckResolutionMethod
{
  public:
    /**
     * Creates a new method with the specified parameters.
     */
    QgsGeometryCheckResolutionMethod( int id, const QString &name, const QString &description, bool isStable = true );

    /**
     * An id that is unique per check. This will be used to trigger resolutions.
     */
    int id() const;

    /**
     * If this fix is stable enough to be listed by default.
     */
    bool isStable() const;

    /**
     * A human readable and translated name for this fix.
     */
    QString name() const;

    /**
     * A human readable and translated description for this fix.
     */
    QString description() const;

  private:
    int mId = -1;
    bool mIsStable = false;
    QString mName;
    QString mDescription;
};

#endif // QGSGEOMETRYCHECKRESOLUTIONMETHOD_H
