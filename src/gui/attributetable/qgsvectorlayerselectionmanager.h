/***************************************************************************
    qgsvectorlayerselectionmanager.h
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

#ifndef QGSVECTORLAYERSELECTIONMANAGER_H
#define QGSVECTORLAYERSELECTIONMANAGER_H

#include "qgsifeatureselectionmanager.h"

class QgsVectorLayer;

/** \ingroup gui
 * \class QgsVectorLayerSelectionManager
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsVectorLayerSelectionManager : public QgsIFeatureSelectionManager
{
    Q_OBJECT

  public:
    explicit QgsVectorLayerSelectionManager( QgsVectorLayer* layer, QObject *parent = nullptr );
    /**
     * The number of features that are selected in this layer
     *
     * @return See description
     */
    virtual int selectedFeatureCount() override;

    /**
     * Select features
     *
     * @param ids            Feature ids to select
     */
    virtual void select( const QgsFeatureIds& ids ) override;

    /**
     * Deselect features
     *
     * @param ids            Feature ids to deselect
     */
    virtual void deselect( const QgsFeatureIds& ids ) override;

    /**
     * Change selection to the new set of features. Dismisses the current selection.
     * Will emit the { @link selectionChanged( const QgsFeatureIds&, const QgsFeatureIds&, bool ) } signal with the
     * clearAndSelect flag set.
     *
     * @param ids   The ids which will be the new selection
     */
    virtual void setSelectedFeatures( const QgsFeatureIds& ids ) override;

    /**
     * Return reference to identifiers of selected features
     *
     * @return A list of { @link QgsFeatureIds }
     * @see selectedFeatures()
     */
    virtual const QgsFeatureIds& selectedFeaturesIds() const override;

  private:
    QgsVectorLayer* mLayer;
};

#endif // QGSVECTORLAYERSELECTIONMANAGER_H
