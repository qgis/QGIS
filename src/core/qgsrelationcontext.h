/***************************************************************************
    qgsrelationcontext.h
     --------------------------------------
    Date                 : 20.12.2019
    Copyright            : (C) 2019 David Marteau
    Email                : dmarteau at 3liz dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRELATIONCONTEXT_H
#define QGSRELATIONCONTEXT_H

#include "qgis_core.h"
#include "qgis_sip.h"

class QgsProject;


/**
 * \ingroup core
 * \class QgsRelationContext
 * \brief Context for relations. Used to resolve layers from projects.
 * \since QGIS 3.11
 */
class CORE_EXPORT QgsRelationContext
{
  public:

    /**
     * Constructor for QgsRelationContext.
     */
    QgsRelationContext( QgsProject *project = nullptr );

    /**
     * Gets the associated project
     *
     * If there is no associated project, then it will return the global
     * project instance
     */
    const QgsProject *project() const;

    ~QgsRelationContext();

    QgsRelationContext( const QgsRelationContext &other );
    QgsRelationContext &operator=( const QgsRelationContext &other );

  private:
    QgsProject *mProject = nullptr;

};

#endif // QGSRELATIONCONTEXT_H

