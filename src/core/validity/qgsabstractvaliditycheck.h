/***************************************************************************
    qgsabstractvaliditycheck.h
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
#ifndef QGSABSTRACTVALIDITYCHECK_H
#define QGSABSTRACTVALIDITYCHECK_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QString>
#include <QObject>

class QgsValidityCheckContext;
class QgsFeedback;

/**
 * \class QgsValidityCheckResult
 * \ingroup core
 * \brief Represents an individual result from a validity check run by a QgsAbstractValidityCheck subclass.
 *
 * Results can either be warnings or critical errors, as dictated by the type member. Critical error
 * are errors which are serious enough to prevent an operation from proceeding, while a warning
 * result will be communicated to users, but not prevent them from proceeding.
 *
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsValidityCheckResult
{
  public:

    //! Result types
    enum Type
    {
      Warning, //!< Warning only, allow operation to proceed but notify user of result
      Critical, //!< Critical error - notify user of result and prevent operation from proceeding
    };

    //! Result type
    Type type;

    /**
     * A short, translated string summarising the result. Ideally a single sentence.
     */
    QString title;

    /**
     * Detailed description of the result (translated), giving users enough detail for them to resolve
     * the error.
     */
    QString detailedDescription;

    /**
     * ID of the check which generated the result. This is usually automatically populated.
     */
    QString checkId;

};

/**
 * \class QgsAbstractValidityCheck
 * \ingroup core
 * \brief Abstract base class for individual validity checks.
 *
 * Validity checks represent objects which can run a test using a QgsValidityCheckContext, and return
 * the results of the check via QgsValidityCheckResult objects.
 *
 * Checks can be used for many different use cases, e.g. validating a layout's contents before allowing
 * an export to occur, or validating the contents of a Processing model (and warning if required plugin based
 * providers are not available or if compulsory algorithm parameters have not been populated).
 *
 * Subclasses must indicate the type of check they represent via the checkType() method. When checks are performed,
 * all the registered checks with a matching check type are performed sequentially. The check type also
 * dictates the subclass of the QgsValidityCheckContext which is given to the subclass' runCheck method.
 *
 * Checks must be registered in the application's QgsValidityCheckRegistry, which is accessible via
 * QgsApplication::validityCheckRegistry().
 *
 * \see QgsValidityCheckRegistry
 *
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsAbstractValidityCheck
{

  public:

    //! Check types
    enum Type
    {
      TypeLayoutCheck = 0, //!< Print layout validity check, triggered on exporting a print layout
      TypeUserCheck = 10000, //!< Starting point for custom user types
    };

    virtual ~QgsAbstractValidityCheck() = default;

    /**
     * Creates a new instance of the check and returns it.
     */
    virtual QgsAbstractValidityCheck *create() const = 0 SIP_FACTORY;

    /**
     * Returns the unique ID of the check.
     *
     * This is a non-translated, non-user visible string identifying the check.
     */
    virtual QString id() const = 0;

    /**
     * Returns the type of the check.
     */
    virtual int checkType() const = 0;

    /**
     * Prepares the check for execution, and returns TRUE if the check can be run.
     *
     * This method is always called from the main thread, and subclasses can implement
     * it to do preparatory steps which are not thread safe (e.g. obtaining feature
     * sources from vector layers). It is followed by a call to runCheck(), which
     * may be performed in a background thread.
     *
     * Individual calls to prepareCheck()/runCheck() are run on a new instance of the
     * check (see create()), so subclasses can safely store state from the prepareCheck() method
     * ready for the subsequent runCheck() method.
     *
     * The \a context argument gives the wider in which the check is being run.
     */
    virtual bool prepareCheck( const QgsValidityCheckContext *context, QgsFeedback *feedback )
    {
      Q_UNUSED( context )
      Q_UNUSED( feedback )
      return true;
    }

    /**
     * Runs the check and returns a list of results. If the check is "passed" and no warnings or errors are generated,
     * then an empty list should be returned.
     *
     * This method may be called in a background thread, so subclasses should take care to ensure that
     * only thread-safe methods are used. It is always preceded by a call to prepareCheck().
     *
     * If a check needs to perform non-thread-safe tests, these should be implemented within prepareCheck()
     * and stored in the subclass instance to be returned by this method.
     *
     * The \a context argument gives the wider in which the check is being run.
     */
    virtual QList< QgsValidityCheckResult > runCheck( const QgsValidityCheckContext *context, QgsFeedback *feedback ) = 0;

};

#endif // QGSABSTRACTVALIDITYCHECK_H
