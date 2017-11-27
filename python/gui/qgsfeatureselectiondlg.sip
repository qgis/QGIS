/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfeatureselectiondlg.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




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
    if ( qobject_cast<QgsFeatureSelectionDlg *>( sipCpp ) )
      sipType = sipType_QgsFeatureSelectionDlg;
    else
      sipType = 0;
%End
  public:
    explicit QgsFeatureSelectionDlg( QgsVectorLayer *vl, QgsAttributeEditorContext &context, QWidget *parent /TransferThis/ = 0 );

    const QgsFeatureIds &selectedFeatures();
%Docstring
 Get the selected features

 :return: The selected feature ids
 :rtype: QgsFeatureIds
%End

    void setSelectedFeatures( const QgsFeatureIds &ids );
%Docstring
 Set the selected features
 \param ids The feature ids to select
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfeatureselectiondlg.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
