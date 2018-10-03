/***************************************************************************
                         qgsanalysis.h
                         --------
    begin                : September 2018
    copyright            : (C) 2018 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSANALYSIS_H
#define QGSANALYSIS_H

#include "qgis_analysis.h"
#include "qgis_sip.h"

#include <memory>

class QgsGeometryCheckRegistry;

/**
 * \ingroup analysis
 * QgsAnalysis is a singleton class containing various registry and other global members
 * related to analysis classes.
 * \since QGIS 3.4
 */
class ANALYSIS_EXPORT QgsAnalysis
{
  public:

    //! QgsAnalysis cannot be copied
    QgsAnalysis( const QgsAnalysis &other ) = delete;

    //! QgsAnalysis cannot be copied
    QgsAnalysis &operator=( const QgsAnalysis &other ) = delete;

    /**
     * Returns a pointer to the singleton instance.
     */
    static QgsAnalysis *instance();

    /**
     * Returns the global geometry checker registry, used for managing all geometry check factories.
     */
    static QgsGeometryCheckRegistry *geometryCheckRegistry();

  private:

    QgsAnalysis();

    std::unique_ptr<QgsGeometryCheckRegistry> mGeometryCheckRegistry;

#ifdef SIP_RUN
    QgsAnalysis( const QgsAnalysis &other );
#endif

};

#endif // QGSANALYSIS_H
