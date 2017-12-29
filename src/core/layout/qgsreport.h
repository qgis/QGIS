/***************************************************************************
                             qgsreport.h
                             ---------------------------
    begin                : December 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSREPORT_H
#define QGSREPORT_H

#include "qgis_core.h"
#include "qgsabstractreportsection.h"


///@cond NOT_STABLE

// This is not considered stable API - it is exposed to python bindings only for unit testing!

/**
 * \ingroup core
 * \class QgsReport
 * \brief Represents a report for use with the QgsLayout engine.
 *
 * Reports consist of multiple sections, represented by QgsAbstractReportSection
 * subclasses.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings for unit testing purposes only.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsReport : public QgsAbstractReportSection
{

  public:

    /**
     * Constructor for QgsReport, associated with the specified
     * \a project.
     *
     * Note that ownership is not transferred to \a project.
     */
    QgsReport( QgsProject *project );

    /**
     * Returns the associated project.
     */
    QgsProject *project() { return mProject; }

    QgsReport *clone() const override SIP_FACTORY;

  private:

    QgsProject *mProject = nullptr;

};

///@endcond

#endif //QGSREPORT_H
