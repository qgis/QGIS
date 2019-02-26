/***************************************************************************
    qgsifeatureselectionmanager.h
     --------------------------------------
    Date                 : 6.6.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSIFEATURESELECTIONMANAGER_H
#define QGSIFEATURESELECTIONMANAGER_H

#include <QObject>
#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgsfeatureid.h"

/**
 * \ingroup gui
 * Is an interface class to abstract feature selection handling.
 *
 * e.g. QgsVectorLayer implements this interface to manage its selections.
 */

class GUI_EXPORT QgsIFeatureSelectionManager : public QObject
{
    Q_OBJECT

  public:
    QgsIFeatureSelectionManager( QObject *parent SIP_TRANSFERTHIS )
      : QObject( parent ) {}

    /**
     * Returns the number of features that are selected in this layer.
     */
    virtual int selectedFeatureCount() = 0;

    /**
     * Select features by feature \a ids.
     */
    virtual void select( const QgsFeatureIds &ids ) = 0;

    /**
     * Deselect features by feature \a ids.
     */
    virtual void deselect( const QgsFeatureIds &ids ) = 0;

    /**
     * Change selection to the new set of features. Dismisses the current selection.
     * Will emit the selectionChanged( const QgsFeatureIds&, const QgsFeatureIds&, bool ) signal with the
     * clearAndSelect flag set.
     *
     * \param ids   The ids which will be the new selection
     * \see selectedFeatureIds()
     */
    virtual void setSelectedFeatures( const QgsFeatureIds &ids ) = 0;

    /**
     * Returns reference to identifiers of selected features
     *
     * \returns A list of QgsFeatureId's
     * \see setSelectedFeatures()
     */
    virtual const QgsFeatureIds &selectedFeatureIds() const = 0;

  signals:

    /**
     * Emitted when selection was changed.
     *
     * \param selected        Newly selected feature ids
     * \param deselected      Ids of all features which have previously been selected but are not any more
     * \param clearAndSelect  In case this is set to TRUE, the old selection was dismissed and the new selection corresponds to selected
     */
    void selectionChanged( const QgsFeatureIds &selected, const QgsFeatureIds &deselected, bool clearAndSelect );
};

#endif // QGSIFEATURESELECTIONMANAGER_H
