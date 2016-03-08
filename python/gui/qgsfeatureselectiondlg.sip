/***************************************************************************
    qgsfeatureselectiondlg.sip
     --------------------------------------
    Date                 : 30.11.2015
    Copyright            : (C) 2015 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// This is required for the ConvertToSubClassCode to work properly
// so RTTI for casting is available in the whole module.
%ModuleCode
#include "qgsfeatureselectiondlg.h"
%End

class QgsFeatureSelectionDlg : QDialog
{
%TypeHeaderCode
#include "qgsfeatureselectiondlg.h"
%End

%ConvertToSubClassCode
  if ( qobject_cast<QgsFeatureSelectionDlg*>( sipCpp ) )
    sipType = sipType_QgsFeatureSelectionDlg;
  else
    sipType = 0;
%End

  public:
    explicit QgsFeatureSelectionDlg( QgsVectorLayer* vl, QgsAttributeEditorContext &context, QWidget *parent /TransferThis/ = 0 );

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
};
