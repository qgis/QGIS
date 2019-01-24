/***************************************************************************
    qgsextentgroupbox.h
    ---------------------
    begin                : March 2014
    copyright            : (C) 2014 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXTENTGROUPBOX_H
#define QGSEXTENTGROUPBOX_H

#include "qgscollapsiblegroupbox.h"
#include "qgsmaptool.h"
#include "qgsmaptoolextent.h"
#include "qgis_sip.h"

#include "ui_qgsextentgroupboxwidget.h"

#include "qgscoordinatereferencesystem.h"
#include "qgsrectangle.h"
#include "qgis_gui.h"

#include <memory>

class QgsCoordinateReferenceSystem;
class QgsMapLayerModel;
class QgsMapLayer;

/**
 * \ingroup gui
 * Collapsible group box for configuration of extent, typically for a save operation.
 *
 * Besides allowing the user to enter the extent manually, it comes with options to use
 * original extent or extent defined by the current view in map canvas.
 *
 * When using the widget, make sure to call setOriginalExtent(), setCurrentExtent() and setOutputCrs() during initialization.
 *
 * \since QGIS 2.4
 */
class GUI_EXPORT QgsExtentGroupBox : public QgsCollapsibleGroupBox, private Ui::QgsExtentGroupBoxWidget
{
    Q_OBJECT
    Q_PROPERTY( QString titleBase READ titleBase WRITE setTitleBase )

  public:

    //! Available states for the current extent selection in the widget
    enum ExtentState
    {
      OriginalExtent,  //!< Layer's extent
      CurrentExtent,   //!< Map canvas extent
      UserExtent,      //!< Extent manually entered/modified by the user
      ProjectLayerExtent, //!< Extent taken from a layer within the project
      DrawOnCanvas, //!< Extent taken from a rectangled drawn onto the map canvas
    };

    /**
     * Constructor for QgsExtentGroupBox.
     */
    explicit QgsExtentGroupBox( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the original extent and coordinate reference system for the widget. This should be called as part of initialization.
     * \see originalExtent()
     * \see originalCrs()
     */
    void setOriginalExtent( const QgsRectangle &originalExtent, const QgsCoordinateReferenceSystem &originalCrs );

    /**
     * Returns the original extent set for the widget.
     * \see setOriginalExtent()
     * \see originalCrs()
     */
    QgsRectangle originalExtent() const { return mOriginalExtent; }

    /**
     * Returns the original coordinate reference system set for the widget.
     * \see originalExtent()
     * \see setOriginalExtent()
     */
    QgsCoordinateReferenceSystem originalCrs() const { return mOriginalCrs; }

    /**
     * Sets the current extent to show in the widget - should be called as part of initialization (or whenever current extent changes).
     * The current extent is usually set to match the current map canvas extent.
     * \see currentExtent()
     * \see currentCrs()
     */
    void setCurrentExtent( const QgsRectangle &currentExtent, const QgsCoordinateReferenceSystem &currentCrs );

    /**
     * Returns the current extent set for the widget. The current extent is usually set to match the
     * current map canvas extent.
     * \see setCurrentExtent()
     * \see currentCrs()
     */
    QgsRectangle currentExtent() const { return mCurrentExtent; }

    /**
     * Returns the coordinate reference system for the current extent set for the widget. The current
     * extent and CRS usually reflects the map canvas extent and CRS.
     * \see setCurrentExtent()
     * \see currentExtent()
     */
    QgsCoordinateReferenceSystem currentCrs() const { return mCurrentCrs; }

    /**
     * Sets the output CRS - may need to be used for transformation from original/current extent.
     * Should be called as part of initialization and whenever the the output CRS is changed.
     * The current extent will be reprojected into the new output CRS.
     */
    void setOutputCrs( const QgsCoordinateReferenceSystem &outputCrs );

    /**
     * Returns the extent shown in the widget - in output CRS coordinates.
     * \see outputCrs
     */
    QgsRectangle outputExtent() const;

    /**
     * Returns the current output CRS, used in the display.
     * \see outputExtent
     * \since QGIS 3.0
     */
    QgsCoordinateReferenceSystem outputCrs() const { return mOutputCrs; }

    /**
     * Returns the currently selected state for the widget's extent.
     */
    QgsExtentGroupBox::ExtentState extentState() const { return mExtentState; }

    /**
     * Sets the base part of \a title of the group box (will be appended with extent state)
     * \see titleBase()
     * \since QGIS 2.12
     */
    void setTitleBase( const QString &title );

    /**
     * Returns the base part of title of the group box (will be appended with extent state).
     * \see setTitleBase()
     * \since QGIS 2.12
     */
    QString titleBase() const;

    /**
     * Sets the map canvas to enable dragging of extent on a canvas.
     * \param canvas the map canvas
     * \since QGIS 3.0
     */
    void setMapCanvas( QgsMapCanvas *canvas );

    /**
     * Returns the current fixed aspect ratio to be used when dragging extent onto the canvas.
     * If the aspect ratio isn't fixed, the width and height will be set to zero.
     * \since QGIS 3.0
     */
    QSize ratio() const { return mRatio; }

  public slots:

    /**
     * Sets the output extent to be the same as original extent (may be transformed to output CRS).
     */
    void setOutputExtentFromOriginal();

    /**
     * Sets the output extent to be the same as current extent (may be transformed to output CRS).
     */
    void setOutputExtentFromCurrent();

    /**
     * Sets the output extent to a custom extent (may be transformed to output CRS).
     */
    void setOutputExtentFromUser( const QgsRectangle &extent, const QgsCoordinateReferenceSystem &crs );

    /**
     * Sets the output extent to match a \a layer's extent (may be transformed to output CRS).
     * \since QGIS 3.0
     */
    void setOutputExtentFromLayer( const QgsMapLayer *layer );

    /**
     * Sets the output extent by dragging on the canvas.
     * \since QGIS 3.0
     */
    void setOutputExtentFromDrawOnCanvas();

    /**
     * Sets a fixed aspect ratio to be used when dragging extent onto the canvas.
     * To unset a fixed aspect ratio, set the width and height to zero.
     * \param ratio aspect ratio's width and height
     * \since QGIS 3.0
     */
    void setRatio( QSize ratio ) { mRatio = ratio; }

  signals:

    /**
     * Emitted when the widget's extent is changed.
     */
    void extentChanged( const QgsRectangle &r );

  private slots:

    void groupBoxClicked();
    void layerMenuAboutToShow();

    void extentDrawn( const QgsRectangle &extent );

  private:
    void setOutputExtent( const QgsRectangle &r, const QgsCoordinateReferenceSystem &srcCrs, QgsExtentGroupBox::ExtentState state );
    void setOutputExtentFromLineEdit();
    void updateTitle();

    //! Base part of the title used for the extent
    QString mTitleBase;

    ExtentState mExtentState = OriginalExtent;

    QgsCoordinateReferenceSystem mOutputCrs;

    QgsRectangle mCurrentExtent;
    QgsCoordinateReferenceSystem mCurrentCrs;

    QgsRectangle mOriginalExtent;
    QgsCoordinateReferenceSystem mOriginalCrs;

    QMenu *mLayerMenu = nullptr;
    QgsMapLayerModel *mMapLayerModel = nullptr;
    QList< QAction * > mMenuActions;
    QPointer< const QgsMapLayer > mExtentLayer;
    QString mExtentLayerName;

    std::unique_ptr< QgsMapToolExtent > mMapToolExtent;
    QPointer< QgsMapTool > mMapToolPrevious = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QSize mRatio;

    void setExtentToLayerExtent( const QString &layerId );

};

#endif // QGSEXTENTGROUPBOX_H
