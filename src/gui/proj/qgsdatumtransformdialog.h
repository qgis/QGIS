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
#include "qgsguiutils.h"
#include "qgis_gui.h"

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

        //! Proj coordinate operation description, for Proj >= 6.0 builds only
        QString proj;

        //! TRUE if fallback transforms can be used
        bool allowFallback = true;
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
     * The optional \a mapCanvas argument can be used to refine the dialog's display based on the current
     * map canvas extent.
     *
     * \since QGIS 3.8
     */
    static bool run( const QgsCoordinateReferenceSystem &sourceCrs = QgsCoordinateReferenceSystem(), const QgsCoordinateReferenceSystem &destinationCrs = QgsCoordinateReferenceSystem(), QWidget *parent = nullptr, QgsMapCanvas *mapCanvas = nullptr, const QString &windowTitle = QString() );

    // TODO QGIS 4.0 - remove selectedDatumTransform, forceChoice

    /**
     * Constructor for QgsDatumTransformDialog.
     */
    QgsDatumTransformDialog( const QgsCoordinateReferenceSystem &sourceCrs = QgsCoordinateReferenceSystem(), const QgsCoordinateReferenceSystem &destinationCrs = QgsCoordinateReferenceSystem(), bool allowCrsChanges = false, bool showMakeDefault = true, bool forceChoice = true, QPair<int, int> selectedDatumTransforms = qMakePair( -1, -1 ), QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags(), const QString &selectedProj = QString(), QgsMapCanvas *mapCanvas = nullptr, bool allowFallback = true );

    void accept() override;
    void reject() override;

    /**
     * Returns the source and destination transforms, each being a pair of QgsCoordinateReferenceSystems and datum transform code
     */
    TransformInfo selectedDatumTransform();

  private slots:

    void operationChanged();
    void setSourceCrs( const QgsCoordinateReferenceSystem &sourceCrs );
    void setDestinationCrs( const QgsCoordinateReferenceSystem &destinationCrs );

  private:
    enum Roles
    {
      TransformIdRole = Qt::UserRole + 1,
      ProjRole,
      AvailableRole,
      BoundsRole
    };

    bool gridShiftTransformation( const QString &itemText ) const;

    void setOKButtonEnabled();

    /**
     * Returns TRUE if the dialog should be shown and the user prompted to make the transformation selection.
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

    std::unique_ptr<QgsTemporaryCursorRestoreOverride> mPreviousCursorOverride;

    friend class TestQgsDatumTransformDialog;
};

#endif // QGSDATUMTRANSFORMDIALOG_H
