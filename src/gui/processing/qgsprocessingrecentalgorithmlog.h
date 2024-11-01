/***************************************************************************
                             qgsprocessingrecentalgorithmlog.h
                             ----------------------------------
    Date                 : July 2018
    Copyright            : (C) 2018 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGRECENTALGORITHMLOG_H
#define QGSPROCESSINGRECENTALGORITHMLOG_H

#include "qgis.h"
#include "qgis_gui.h"

///@cond NOT_STABLE

/**
 * \ingroup gui
 * \brief A log for tracking recently used processing algorithms.
 *
 * QgsProcessingRecentAlgorithmLog is not usually directly created, instead
 * use the instance accessible through QgsGui::processingRecentAlgorithmLog().
 *
 * The log contents are saved and restored via QgsSettings.
 *
 * \note Not stable API
 * \since QGIS 3.4
 */
class GUI_EXPORT QgsProcessingRecentAlgorithmLog : public QObject
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsProcessingRecentAlgorithmLog, with the specified
     * \a parent object.
     */
    QgsProcessingRecentAlgorithmLog( QObject *parent = nullptr );

    /**
     * Returns a list of the IDs of recently used processing algorithms, where the
     * first item in the list is the most recently used algorithm.
     */
    QStringList recentAlgorithmIds() const;

    /**
     * Pushes the algorithm with matching \a id to the top of the recently used
     * algorithm list.
     *
     * If this changes the list of recent algorithm IDs then the changed() signal
     * will be emitted.
     */
    void push( const QString &id );

  signals:

    /**
     * Emitted when the list of recently used algorithms is changed, e.g. when
     * a new algorithm ID is pushed to the list (see push()).
     */
    void changed();

  private:
    QStringList mRecentAlgorithmIds;
};

///@endcond

#endif // QGSPROCESSINGRECENTALGORITHMLOG_H
