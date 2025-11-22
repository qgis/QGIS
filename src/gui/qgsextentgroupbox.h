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
#include "qgis_sip.h"

#include "qgscoordinatereferencesystem.h"
#include "qgsrectangle.h"
#include "qgis_gui.h"

class QgsMapLayer;
class QgsExtentWidget;
class QgsMapCanvas;

/**
 * \ingroup gui
 * \brief Collapsible group box for configuration of extent, typically for a save operation.
 *
 * Besides allowing the user to enter the extent manually, it comes with options to use
 * original extent or extent defined by the current view in map canvas.
 *
 * When using the group box, make sure to call setOriginalExtent(), setCurrentExtent() and setOutputCrs() during initialization.
 *
 *
 * \see QgsExtentWidget
 *
 */
class GUI_EXPORT QgsExtentGroupBox : public QgsCollapsibleGroupBox
{
    Q_OBJECT
    Q_PROPERTY( QString titleBase READ titleBase WRITE setTitleBase )

  public:
    // TODO QGIS 4.0 -- use QgsExtentWidget enum instead

    //! Available states for the current extent selection in the widget
    enum ExtentState
    {
      OriginalExtent,     //!< Layer's extent
      CurrentExtent,      //!< Map canvas extent
      UserExtent,         //!< Extent manually entered/modified by the user
      ProjectLayerExtent, //!< Extent taken from a layer within the project
      DrawOnCanvas,       //!< Extent taken from a rectangled drawn onto the map canvas
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
    QgsRectangle originalExtent() const;

    /**
     * Returns the original coordinate reference system set for the widget.
     * \see originalExtent()
     * \see setOriginalExtent()
     */
    QgsCoordinateReferenceSystem originalCrs() const;

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
    QgsRectangle currentExtent() const;

    /**
     * Returns the coordinate reference system for the current extent set for the widget. The current
     * extent and CRS usually reflects the map canvas extent and CRS.
     * \see setCurrentExtent()
     * \see currentExtent()
     */
    QgsCoordinateReferenceSystem currentCrs() const;

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
     */
    QgsCoordinateReferenceSystem outputCrs() const;

    /**
     * Returns the currently selected state for the widget's extent.
     */
    QgsExtentGroupBox::ExtentState extentState() const;

    /**
     * Sets the base part of \a title of the group box (will be appended with extent state)
     * \see titleBase()
     */
    void setTitleBase( const QString &title );

    /**
     * Returns the base part of title of the group box (will be appended with extent state).
     * \see setTitleBase()
     */
    QString titleBase() const;

    /**
     * Sets the map canvas to enable dragging of extent on a canvas.
     * \param canvas the map canvas
     * \param drawOnCanvasOption set to false to disable to draw on canvas option
     */
    void setMapCanvas( QgsMapCanvas *canvas, bool drawOnCanvasOption = true );

    /**
     * Returns the current fixed aspect ratio to be used when dragging extent onto the canvas.
     * If the aspect ratio isn't fixed, the width and height will be set to zero.
     */
    QSize ratio() const;

    /**
     * Sets whether the extent should be snapped to the grid of a reference raster layer.
     * \param snapToGrid Whether the extent should be snapped to the grid
     * \param rasterXRes The X resolution of the reference raster
     * \param rasterYRes The Y resolution of the reference raster
     * \param rasterMinX The minimum X coordinate of the reference raster
     * \param rasterMinY The minimum Y coordinate of the reference raster
     * \see snapToGrid()
     * \since QGIS 3.46
     */
    void setSnapToGrid( bool snapToGrid, double rasterXRes, double rasterYRes, double rasterMinX, double rasterMinY );

    /**
     * Returns whether the extent should be snapped to the grid of a reference raster layer.
     * \returns TRUE if the extent should be snapped to the grid
     * \see setSnapToGrid()
     * \since QGIS 3.46
     */
    bool snapToGrid() const { return mSnapToGrid; }

    /**
     * Returns the X resolution of the reference raster used for snapping to grid.
     * \see setSnapToGrid()
     * \since QGIS 3.46
     */
    double rasterXRes() const { return mRasterXRes; }

    /**
     * Returns the Y resolution of the reference raster used for snapping to grid.
     * \see setSnapToGrid()
     * \since QGIS 3.46
     */
    double rasterYRes() const { return mRasterYRes; }

    /**
     * Returns the minimum X coordinate of the reference raster used for snapping to grid.
     * \see setSnapToGrid()
     * \since QGIS 3.46
     */
    double rasterMinX() const { return mRasterMinX; }

    /**
     * Returns the minimum Y coordinate of the reference raster used for snapping to grid.
     * \see setSnapToGrid()
     * \since QGIS 3.46
     */
    double rasterMinY() const { return mRasterMinY; }

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
     */
    void setOutputExtentFromLayer( const QgsMapLayer *layer );

    /**
     * Sets the output extent by dragging on the canvas.
     */
    void setOutputExtentFromDrawOnCanvas();

    /**
     * Sets a fixed aspect ratio to be used when dragging extent onto the canvas.
     * To unset a fixed aspect ratio, set the width and height to zero.
     * \param ratio aspect ratio's width and height
     */
    void setRatio( QSize ratio );

  signals:

    /**
     * Emitted when the widget's extent is changed.
     */
    void extentChanged( const QgsRectangle &r );

    /**
     * Emitted when the extent layer is changed.
     * \since QGIS 3.44
     */
    void extentLayerChanged( QgsMapLayer *layer );

    /**
     * Emitted when the snap-to-grid state is changed.
     * \param enabled whether snap-to-grid is enabled
     * \since QGIS 3.46
     */
    void snapToGridChanged( bool enabled );

  private slots:

    void groupBoxClicked();

    void widgetExtentChanged();

    void validationChanged( bool valid );

  private:
    void updateTitle();

    QgsExtentWidget *mWidget = nullptr;

    //! Base part of the title used for the extent
    QString mTitleBase;

    //! Whether to snap the extent to the grid of a reference raster
    bool mSnapToGrid = false;

    //! X resolution of the reference raster for snapping to grid
    double mRasterXRes = 1.0;

    //! Y resolution of the reference raster for snapping to grid
    double mRasterYRes = 1.0;

    //! Minimum X coordinate of the reference raster for snapping to grid
    double mRasterMinX = 0.0;

    //! Minimum Y coordinate of the reference raster for snapping to grid
    double mRasterMinY = 0.0;
};

#endif // QGSEXTENTGROUPBOX_H
