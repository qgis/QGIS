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
#include "qgis_gui.h"


SIP_NO_FILE

class QgsVectorLayer;

/**
 * \ingroup gui
 * \class QgsVectorLayerSelectionManager
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsVectorLayerSelectionManager : public QgsIFeatureSelectionManager
{
    Q_OBJECT

  public:
    explicit QgsVectorLayerSelectionManager( QgsVectorLayer *layer, QObject *parent = nullptr );

    int selectedFeatureCount() override;
    void select( const QgsFeatureIds &ids ) override;
    void deselect( const QgsFeatureIds &ids ) override;
    void setSelectedFeatures( const QgsFeatureIds &ids ) override;
    const QgsFeatureIds &selectedFeatureIds() const override;

    /**
     * Returns the vector layer
     */
    QgsVectorLayer *layer() const;

  private slots:

    /**
     * Called whenever layer selection was changed
     *
     * \param selected        Newly selected feature ids
     * \param deselected      Ids of all features which have previously been selected but are not any more
     * \param clearAndSelect  In case this is set to TRUE, the old selection was dismissed and the new selection corresponds to selected
     */
    virtual void onSelectionChanged( const QgsFeatureIds &selected, const QgsFeatureIds &deselected, bool clearAndSelect );

  private:
    QgsVectorLayer *mLayer = nullptr;
};

#endif // QGSVECTORLAYERSELECTIONMANAGER_H
