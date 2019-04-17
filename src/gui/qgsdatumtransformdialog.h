/***************************************************************************
                         qgsdatumtransformdialog.h
                         -------------------------
    begin                : November 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco.hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDATUMTRANSFORMDIALOG_H
#define QGSDATUMTRANSFORMDIALOG_H

#include "ui_qgsdatumtransformdialogbase.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgis_gui.h"

class QgsTemporaryCursorRestoreOverride;

#define SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsDatumTransformDialog
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsDatumTransformDialog : public QDialog, private Ui::QgsDatumTransformDialogBase
{
    Q_OBJECT
  public:

    //! Dialog transformation entry info
    struct TransformInfo
    {
      //! Source coordinate reference system
      QgsCoordinateReferenceSystem sourceCrs;

      //! Source transform ID
      int sourceTransformId = -1;

      //! Destination coordinate reference system
      QgsCoordinateReferenceSystem destinationCrs;

      //! Destination transform ID
      int destinationTransformId = -1;
    };

    /**
     * Runs the dialog (if required) prompting for the desired transform to use from \a sourceCrs to
     * \a destinationCrs, updating the current project transform context as required
     * based on the results of the run.
     *
     * This handles EVERYTHING, including only showing the dialog if multiple choices exist
     * and the user has asked to be prompted, not re-adding transforms already in the current project
     * context, etc.
     *
     * \since QGIS 3.8
     */
    static bool run( const QgsCoordinateReferenceSystem &sourceCrs = QgsCoordinateReferenceSystem(),
                     const QgsCoordinateReferenceSystem &destinationCrs = QgsCoordinateReferenceSystem(),
                     QWidget *parent = nullptr );

    /**
     * Constructor for QgsDatumTransformDialog.
     */
    QgsDatumTransformDialog( const QgsCoordinateReferenceSystem &sourceCrs = QgsCoordinateReferenceSystem(),
                             const QgsCoordinateReferenceSystem &destinationCrs = QgsCoordinateReferenceSystem(),
                             bool allowCrsChanges = false,
                             bool showMakeDefault = true,
                             bool forceChoice = true,
                             QPair<int, int> selectedDatumTransforms = qMakePair( -1, -1 ),
                             QWidget *parent = nullptr,
                             Qt::WindowFlags f = nullptr );
    ~QgsDatumTransformDialog() override;

    void accept() override;
    void reject() override;

    /**
     * Returns the source and destination transforms, each being a pair of QgsCoordinateReferenceSystems and datum transform code
     * \since 3.0
     */
    TransformInfo selectedDatumTransform();

  private slots:

    void tableCurrentItemChanged( QTableWidgetItem *, QTableWidgetItem * );
    void setSourceCrs( const QgsCoordinateReferenceSystem &sourceCrs );
    void setDestinationCrs( const QgsCoordinateReferenceSystem &destinationCrs );

  private:
    bool gridShiftTransformation( const QString &itemText ) const;
    //! Returns FALSE if the location of the grid shift files is known (PROJ_LIB) and the shift file is not there
    bool testGridShiftFileAvailability( QTableWidgetItem *item ) const;
    void load( QPair<int, int> selectedDatumTransforms = qMakePair( -1, -1 ) );
    void setOKButtonEnabled();

    /**
     * Returns true if the dialog should be shown and the user prompted to make the transformation selection.
     *
     * \see defaultDatumTransform()
     */
    bool shouldAskUserForSelection() const;

    /**
     * Returns the default transform (or only available transform). This represents the transform which
     * should be used if the user is not being prompted to make this selection for themselves.
     *
     * \see shouldAskUserForSelection()
     * \see applyDefaultTransform()
     */
    TransformInfo defaultDatumTransform() const;

    /**
     * Applies the defaultDatumTransform(), adding it to the current QgsProject instance.
     */
    void applyDefaultTransform();

    QList< QgsDatumTransform::TransformPair > mDatumTransforms;
    QgsCoordinateReferenceSystem mSourceCrs;
    QgsCoordinateReferenceSystem mDestinationCrs;
    std::unique_ptr< QgsTemporaryCursorRestoreOverride > mPreviousCursorOverride;

    friend class TestQgsDatumTransformDialog;
};

#endif // QGSDATUMTRANSFORMDIALOG_H
