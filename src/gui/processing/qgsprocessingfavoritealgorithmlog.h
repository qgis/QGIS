/***************************************************************************
                             qgsprocessingfavoritealgorithmlog.h
                             ----------------------------------
    Date                 : February 2024
    Copyright            : (C) 2024 Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGFAVORITEALGORITHMLOG_H
#define QGSPROCESSINGFAVORITEALGORITHMLOG_H

#include "qgis.h"
#include "qgis_gui.h"

class QgsSettingsEntryStringList;

///@cond NOT_STABLE

/**
 * \ingroup gui
 * \brief A log for tracking favorite Processing algorithms.
 *
 * QgsProcessingFavoriteAlgorithmLog is not usually directly created, instead
 * use the instance accessible through QgsGui::processingFavoriteAlgorithmLog().
 *
 * The log contents are saved and restored via QgsSettings.
 *
 * \note Not stable API
 * \since QGIS 3.38
 */
class GUI_EXPORT QgsProcessingFavoriteAlgorithmLog : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProcessingFavoriteAlgorithmLog, with the specified
     * \a parent object.
     */
    QgsProcessingFavoriteAlgorithmLog( QObject *parent = nullptr );

    /**
     * Returns a list of the IDs of favorite Processing algorithms.
     */
    QStringList favoriteAlgorithmIds() const;

    /**
     * Adds the algorithm with matching \a id to the favorite algorithms list.
     *
     * If this changes the list of favorite algorithm IDs then the changed() signal
     * will be emitted.
     */
    void add( const QString &id );

    /**
     * Removes the algorithm with matching \a id from the favorite algorithms list.
     *
     * If this changes the list of favorite algorithm IDs then the changed() signal
     * will be emitted.
     */
    void remove( const QString &id );

    /**
     * Clears list of favorite Processing algorithms
     */
    void clear();

    /**
     * Returns TRUE if the algorithm with matching \a id is in a favorite list.
     */
    bool isFavorite( const QString &id );

#ifndef SIP_RUN
    //! Settings entry favorite algorithms
    static const QgsSettingsEntryStringList *settingsFavoriteAlgorithms;
#endif

  signals:

    /**
     * Emitted when the list of favorite algorithms is changed, e.g. when
     * a new algorithm ID is added to the list or an existing algorithm ID
     * is removed from the list.
     */
    void changed();

  private:

    QStringList mFavoriteAlgorithmIds;

};

///@endcond

#endif // QGSPROCESSINGFAVORITEALGORITHMLOG_H
