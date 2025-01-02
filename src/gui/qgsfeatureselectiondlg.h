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

class QgsVectorLayerSelectionManager;

#include "ui_qgsfeatureselectiondlg.h"
#include "qgis_sip.h"
#include "qgis_gui.h"

#ifdef SIP_RUN
// This is required for the ConvertToSubClassCode to work properly
// so RTTI for casting is available in the whole module.
//%ModuleCode
#include "qgsfeatureselectiondlg.h"
//%End
#endif

/**
 * \ingroup gui
 * \class QgsFeatureSelectionDlg
 */
class GUI_EXPORT QgsFeatureSelectionDlg : public QDialog, private Ui::QgsFeatureSelectionDlg
{
#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsFeatureSelectionDlg *>( sipCpp ) )
      sipType = sipType_QgsFeatureSelectionDlg;
    else
      sipType = 0;
    SIP_END
#endif

    Q_OBJECT

  public:
    //! Constructor for QgsFeatureSelectionDlg
    explicit QgsFeatureSelectionDlg( QgsVectorLayer *vl, const QgsAttributeEditorContext &context, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Gets the selected features
     *
     * \returns The selected feature ids
     */
    const QgsFeatureIds &selectedFeatures();

    /**
     * Set the selected features
     * \param ids The feature ids to select
     */
    void setSelectedFeatures( const QgsFeatureIds &ids );

    /**
     * Set form filter expression
     */
    void setFilterExpression( const QString &filter, QgsAttributeForm::FilterType type );

  protected:
    void keyPressEvent( QKeyEvent *evt ) override;

    //! Make sure the dialog does not grow too much
    void showEvent( QShowEvent *event ) override;

  private slots:

    /**
     * Inverts selection
     */
    void mActionInvertSelection_triggered();

    /**
     * Clears selection
     */
    void mActionRemoveSelection_triggered();

    /**
     * Select all
     */
    void mActionSelectAll_triggered();

    /**
     * Zooms to selected features
     */
    void mActionZoomMapToSelectedRows_triggered();

    /**
     * Pans to selected features
     */
    void mActionPanMapToSelectedRows_triggered();

    /**
     * Select feature using an expression
     */
    void mActionExpressionSelect_triggered();

    /**
     * View mode has changed
     */
    void viewModeChanged( QgsAttributeEditorContext::Mode mode );

  private:
    QgsVectorLayerSelectionManager *mFeatureSelection = nullptr;
    QgsVectorLayer *mVectorLayer = nullptr;
    QgsAttributeEditorContext mContext;
};

#endif // QGSFEATURESELECTIONDLG_H
