/***************************************************************************
    qgsfeatureselectiondlg.h
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

#ifndef QGSFEATURESELECTIONDLG_H
#define QGSFEATURESELECTIONDLG_H

class QgsGenericFeatureSelectionManager;

#include "ui_qgsfeatureselectiondlg.h"

/** \ingroup gui
 * \class QgsFeatureSelectionDlg
 */
class GUI_EXPORT QgsFeatureSelectionDlg : public QDialog, private Ui::QgsFeatureSelectionDlg
{
    Q_OBJECT

  public:
    explicit QgsFeatureSelectionDlg( QgsVectorLayer* vl, QgsAttributeEditorContext &context, QWidget *parent = nullptr );

    /**
     * Get the selected features
     *
     * @return The selected feature ids
     */
    const QgsFeatureIds& selectedFeatures();

    /**
     * Set the selected features
     * @param ids The feature ids to select
     */
    void setSelectedFeatures( const QgsFeatureIds& ids );

  private:
    QgsGenericFeatureSelectionManager* mFeatureSelection;
    QgsVectorLayer* mVectorLayer;
};

#endif // QGSFEATURESELECTIONDLG_H
