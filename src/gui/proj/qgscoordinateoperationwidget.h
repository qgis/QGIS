/***************************************************************************
                         qgscoordinateoperationwidget.h
                         ------------------------------
    begin                : December 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOORDINATEOPERATIONWIDGET_H
#define QGSCOORDINATEOPERATIONWIDGET_H

#include "ui_qgscoordinateoperationwidgetbase.h"
#include "qgscoordinatereferencesystem.h"
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \class QgsCoordinateOperationWidget
 * \brief A widget for selecting the coordinate operation to use when transforming between
 * a source and destination coordinate reference system.
 *
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsCoordinateOperationWidget : public QWidget, private Ui::QgsCoordinateOperationWidgetBase
{
    Q_OBJECT
  public:
    //! Coordinate operation details
    struct OperationDetails
    {
        //! Source transform ID
        int sourceTransformId = -1;

        //! Destination transform ID
        int destinationTransformId = -1;

        //! Proj coordinate operation description, for Proj >= 6.0 builds only
        QString proj;

        //! TRUE if operation is available
        bool isAvailable = true;

        //! TRUE if fallback transforms can be used
        bool allowFallback = true;
    };

    /**
     * Constructor for QgsCoordinateOperationWidget.
     */
    QgsCoordinateOperationWidget( QWidget *parent = nullptr );

    ~QgsCoordinateOperationWidget() override;

    /**
     * Returns the source CRS for the operations shown in the widget.
     *
     * \see setSourceCrs()
     * \see destinationCrs()
     */
    QgsCoordinateReferenceSystem sourceCrs() const { return mSourceCrs; }

    /**
     * Returns the destination CRS for the operations shown in the widget.
     *
     * \see setDestinationCrs()
     * \see sourceCrs()
     */
    QgsCoordinateReferenceSystem destinationCrs() const { return mDestinationCrs; }

    /**
     * Sets the source \a crs for the operations shown in the widget.
     *
     * \see sourceCrs()
     * \see setDestinationCrs()
     */
    void setSourceCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Sets the destination \a crs for the operations shown in the widget.
     *
     * \see destinationCrs()
     * \see setSourceCrs()
     */
    void setDestinationCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Sets a map \a canvas to link to the widget, which allows the widget's choices to reflect
     * the current canvas state.
     */
    void setMapCanvas( QgsMapCanvas *canvas );

    /**
     * Sets whether the "make default" checkbox should be shown.
     */
    void setShowMakeDefault( bool show );

    /**
     * Returns TRUE if the "make default" option is selected.
     */
    bool makeDefaultSelected() const;

    /**
     * Returns TRUE if there is a valid selection in the widget.
     */
    bool hasSelection() const;

    /**
     * Returns a list of the available operations shown in the widget.
     */
    QList<QgsCoordinateOperationWidget::OperationDetails> availableOperations() const;

    /**
     * Returns the details of the default operation suggested by the widget.
     */
    QgsCoordinateOperationWidget::OperationDetails defaultOperation() const;

    /**
     * Returns the details of the operation currently selected within the widget.
     * \see setSelectedOperation()
     */
    QgsCoordinateOperationWidget::OperationDetails selectedOperation() const;

    /**
     * Sets the details of the \a operation currently selected within the widget.
     * \see selectedOperation()
     */
    void setSelectedOperation( const QgsCoordinateOperationWidget::OperationDetails &operation );

    /**
     * Automatically sets the selected operation using the settings encapsulated in a transform \a context.
     *
     * If no matching operations are found within the context then the defaultOperation() will be
     * selected.
     */
    void setSelectedOperationUsingContext( const QgsCoordinateTransformContext &context );

    /**
     * Sets whether the "allow fallback" operations option is visible.
     *
     * \since QGIS 3.12
     */
    void setShowFallbackOption( bool visible );

  signals:

    /**
     * Emitted when the operation selected in the dialog is changed.
     */
    void operationChanged();

    /**
     * Emitted when an operation is double-clicked in the widget.
     */
    void operationDoubleClicked();

  private slots:

    void tableCurrentItemChanged( QTableWidgetItem *, QTableWidgetItem * );

    void showSupersededToggled( bool toggled );

    void installGrid();

  private:
    enum Roles
    {
      TransformIdRole = Qt::UserRole + 1,
      ProjRole,
      AvailableRole,
      BoundsRole,
      MissingGridsRole,
      MissingGridPackageNamesRole,
      MissingGridUrlsRole
    };

    bool gridShiftTransformation( const QString &itemText ) const;
    //! Returns FALSE if the location of the grid shift files is known (PROJ_LIB) and the shift file is not there
    bool testGridShiftFileAvailability( QTableWidgetItem *item ) const;
    void loadAvailableOperations();

    /**
     * Cleans up a PROJ scope string, adding friendly acronym descriptions.
     */
    QString formatScope( const QString &scope );

    QList<QgsDatumTransform::TransformDetails> mDatumTransforms;

    QgsCoordinateReferenceSystem mSourceCrs;
    QgsCoordinateReferenceSystem mDestinationCrs;
    OperationDetails mPreviousOp;
    int mBlockSignals = 0;
};

#endif // QGSCOORDINATEOPERATIONWIDGET_H
