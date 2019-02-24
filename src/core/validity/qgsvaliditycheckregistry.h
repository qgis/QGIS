/***************************************************************************
    qgsvaliditycheckregistry.h
    --------------------------
    begin                : November 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSVALIDITYCHECKREGISTRY_H
#define QGSVALIDITYCHECKREGISTRY_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsabstractvaliditycheck.h"
#include <QList>
#include <memory>
#include <vector>

/**
 * \class QgsValidityCheckRegistry
 * \ingroup gui
 * This class keeps a list of QgsAbstractValidityCheck checks which can be used when
 * performing validity checks.
 *
 * QgsValidityCheckRegistry is not usually directly created, but rather accessed through
 * QgsApplication::validityCheckRegistry().
 *
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsValidityCheckRegistry
{

  public:

    QgsValidityCheckRegistry();

    ~QgsValidityCheckRegistry();

    //! QgsValidityCheckRegistry cannot be copied.
    QgsValidityCheckRegistry( const QgsValidityCheckRegistry &rh ) = delete;
    //! QgsValidityCheckRegistry cannot be copied.
    QgsValidityCheckRegistry &operator=( const QgsValidityCheckRegistry &rh ) = delete;

    /**
     * Returns the list of available checks.
     */
    QList<const QgsAbstractValidityCheck *> checks() const;

    /**
     * Returns the list of all available checks of the matching \a type.
     */
    QList<const QgsAbstractValidityCheck *> checks( int type ) const;

    /**
     * Adds a \a check to the registry. Ownership of the check
     * is transferred to the registry.
     */
    void addCheck( QgsAbstractValidityCheck *check SIP_TRANSFER );

    /**
     * Removes a \a check from the registry.
     * The check object is automatically deleted.
     */
    void removeCheck( QgsAbstractValidityCheck *check );

    /**
     * Runs all checks of the specified \a type and returns a list of results.
     *
     * If all checks are "passed" and no warnings or errors are generated, then
     * an empty list will be returned.
     *
     * The \a context argument gives the wider in which the check is being run.
     *
     * The \a feedback argument is used to give progress reports and to support
     * cancellation of long-running checks.
     *
     * This is a blocking call, which will run all matching checks in the main
     * thread and only return when they have all completed.
     */
    QList< QgsValidityCheckResult > runChecks( int type, const QgsValidityCheckContext *context, QgsFeedback *feedback ) const;

  private:

#ifdef SIP_RUN
    QgsValidityCheckRegistry( const QgsValidityCheckRegistry &rh );
#endif

    /**
     * Returns a list containing new copies of all available checks of the matching \a type.
     */
    std::vector<std::unique_ptr< QgsAbstractValidityCheck > > createChecks( int type ) const SIP_FACTORY;

    //! Available checks, owned by this class
    QList< QgsAbstractValidityCheck * > mChecks;

};

#endif // QGSVALIDITYCHECKREGISTRY_H
