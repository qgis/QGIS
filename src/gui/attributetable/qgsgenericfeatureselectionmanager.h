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

#include "qgsifeatureselectionmanager.h"
#include "qgis_gui.h"
#include "qgsfeatureid.h"

SIP_NO_FILE

/**
 * \ingroup gui
 * This selection manager synchronizes a local set of selected features with an attribute table.
 * If you want to synchronize the attribute table selection with the map canvas selection, you
 * should use QgsVectorLayerSelectionManager instead.
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsGenericFeatureSelectionManager : public QgsIFeatureSelectionManager
{
    Q_OBJECT

  public:
    explicit QgsGenericFeatureSelectionManager( QObject *parent = nullptr );
    QgsGenericFeatureSelectionManager( const QgsFeatureIds &initialSelection, QObject *parent = nullptr );

    // QgsIFeatureSelection interface
    int selectedFeatureCount() override;
    void select( const QgsFeatureIds &ids ) override;
    void deselect( const QgsFeatureIds &ids ) override;
    void setSelectedFeatures( const QgsFeatureIds &ids ) override;
    const QgsFeatureIds &selectedFeatureIds() const override;

  private:
    QgsFeatureIds mSelectedFeatures;
};

#endif // QGSGENERICFEATURESELECTIONMANAGER_H
