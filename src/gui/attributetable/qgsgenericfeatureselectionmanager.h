/***************************************************************************
    qgsgenericfeatureselectionmgr.h
     --------------------------------------
    Date                 : 11.6.2013
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

#ifndef QGSGENERICFEATURESELECTIONMANAGER_H
#define QGSGENERICFEATURESELECTIONMANAGER_H

#include "qgsfeature.h"
#include "qgsifeatureselectionmanager.h"

/**
 * This selection manager synchronizes a local set of selected features with an attribute table.
 * If you want to synchronize the attribute table selection with the map canvas selection, you
 * should use { @link QgsVectorLayerSelectionManager } instead.
 */
class GUI_EXPORT QgsGenericFeatureSelectionManager : public QgsIFeatureSelectionManager
{
    Q_OBJECT

  public:
    explicit QgsGenericFeatureSelectionManager( QObject *parent = NULL );
    QgsGenericFeatureSelectionManager( const QgsFeatureIds& initialSelection, QObject *parent = NULL );

    // QgsIFeatureSelection interface
    virtual int selectedFeatureCount() override;
    virtual void select( const QgsFeatureIds& ids ) override;
    virtual void deselect( const QgsFeatureIds& ids ) override;
    virtual void setSelectedFeatures( const QgsFeatureIds& ids ) override;
    virtual const QgsFeatureIds& selectedFeaturesIds() const override;

  private:
    QgsFeatureIds mSelectedFeatures;
};

#endif // QGSGENERICFEATURESELECTIONMANAGER_H
