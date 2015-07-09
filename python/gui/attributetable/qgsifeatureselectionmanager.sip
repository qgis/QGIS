/***************************************************************************
    qgsifeatureselectionmanager.sip
     --------------------------------------
    Date                 : 6.6.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
 * Is an interface class to abstract feature selection handling.
 *
 * e.g. @link{QgsVectorLayer} implements this interface to manage its selections.
 */

class QgsIFeatureSelectionManager : QObject
{
%TypeHeaderCode
#include <qgsifeatureselectionmanager.h>
%End
  public:
    QgsIFeatureSelectionManager( QObject* parent /TransferThis/ );

    /**
     * The number of features that are selected in this layer
     *
     * @return See description
     */
    virtual int selectedFeatureCount() = 0;

    /**
     * Select features
     *
     * @param ids            Feature ids to select
     */
    virtual void select( const QgsFeatureIds& ids ) = 0;

    /**
     * Deselect features
     *
     * @param ids            Feature ids to deselect
     */
    virtual void deselect( const QgsFeatureIds& ids ) = 0;

    /**
     * Change selection to the new set of features. Dismisses the current selection.
     * Will emit the { @link selectionChanged( QgsFeatureIds, QgsFeatureIds, bool ) } signal with the
     * clearAndSelect flag set.
     *
     * @param ids   The ids which will be the new selection
     */
    virtual void setSelectedFeatures( const QgsFeatureIds& ids ) = 0;

    /**
     * Return reference to identifiers of selected features
     *
     * @return A list of { @link QgsFeatureId } 's
     * @see selectedFeatures()
     */
    virtual const QgsFeatureIds &selectedFeaturesIds() const = 0;

  signals:
    /**
     * This signal is emitted when selection was changed
     *
     * @param selected        Newly selected feature ids
     * @param deselected      Ids of all features which have previously been selected but are not any more
     * @param clearAndSelect  In case this is set to true, the old selection was dismissed and the new selection corresponds to selected
     */
    void selectionChanged( const QgsFeatureIds selected, const QgsFeatureIds deselected, const bool clearAndSelect );
};
