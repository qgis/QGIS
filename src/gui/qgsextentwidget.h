/***************************************************************************
    qgsextentwidget.h
    ---------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXTENTWIDGET_H
#define QGSEXTENTWIDGET_H

#include "qgscollapsiblegroupbox.h"
#include "qgsmaptool.h"
#include "qgsmaptoolextent.h"
#include "qgis_sip.h"

#include "ui_qgsextentgroupboxwidget.h"

#include "qgscoordinatereferencesystem.h"
#include "qgsrectangle.h"
#include "qgis_gui.h"

#include <memory>
#include <QRegularExpression>

class QgsBookmarkManagerProxyModel;
class QgsCoordinateReferenceSystem;
class QgsMapLayerProxyModel;
class QgsMapLayer;

/**
 * \ingroup gui
 * \brief A widget for configuration of a map extent.
 *
 * Besides allowing the user to enter the extent manually, it comes with options to use
 * original extent or extent defined by the current view in map canvas.
 *
 * When using the widget, make sure to call setOriginalExtent(), setCurrentExtent() and setOutputCrs() during initialization.
 *
 * \see QgsExtentGroupBox
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsExtentWidget : public QWidget, private Ui::QgsExtentGroupBoxWidget
{
    Q_OBJECT

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

    //! Widget styles
    enum WidgetStyle
    {
      CondensedStyle, //!< Shows a compressed widget, for use when available space is minimal
      ExpandedStyle, //!< Shows an expanded widget, for use when space is not constrained
    };

    /**
     * Constructor for QgsExtentWidget.
     */
    explicit QgsExtentWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr, WidgetStyle style = CondensedStyle );

    ~QgsExtentWidget() override;

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
     */
    QgsCoordinateReferenceSystem outputCrs() const { return mOutputCrs; }

    /**
     * Returns the currently selected state for the widget's extent.
     */
    QgsExtentWidget::ExtentState extentState() const { return mExtentState; }

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
    QSize ratio() const { return mRatio; }

    /**
     * Returns the name of the extent layer.
     */
    QString extentLayerName() const;

    /**
     * Returns TRUE if the widget is in a valid state, i.e. has an extent set.
     */
    bool isValid() const;

    /**
     * Sets whether the widget can be set to a "not set" (null) state.
     *
     * The specified \a notSetText will be used for showing null values.
     *
     * \note This mode only applies to widgets in the condensed state!
     */
    void setNullValueAllowed( bool allowed, const QString &notSetText = QString() );

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
    void setRatio( QSize ratio ) { mRatio = ratio; }

    /**
     * Clears the widget, setting it to a null value.
     */
    void clear();

  signals:

    /**
     * Emitted when the widget's extent is changed.
     */
    void extentChanged( const QgsRectangle &r );

    /**
     * Emitted when the widget's validation state changes.
     */
    void validationChanged( bool valid );

    /**
     * Emitted when the parent dialog visibility must be changed (e.g.
     * to permit access to the map canvas)
     */
    void toggleDialogVisibility( bool visible );

  protected:

    void dragEnterEvent( QDragEnterEvent *event ) override;
    void dragLeaveEvent( QDragLeaveEvent *event ) override;
    void dropEvent( QDropEvent *event ) override;

  private slots:

    void layerMenuAboutToShow();
    void layoutMenuAboutToShow();
    void bookmarkMenuAboutToShow();

    void extentDrawn( const QgsRectangle &extent );
    void mapToolDeactivated();

  private:
    void setOutputExtent( const QgsRectangle &r, const QgsCoordinateReferenceSystem &srcCrs, QgsExtentWidget::ExtentState state );
    void setOutputExtentFromLineEdit();
    void setOutputExtentFromCondensedLineEdit();

    ExtentState mExtentState = OriginalExtent;

    QgsCoordinateReferenceSystem mOutputCrs;

    QgsRectangle mCurrentExtent;
    QgsCoordinateReferenceSystem mCurrentCrs;

    QgsRectangle mOriginalExtent;
    QgsCoordinateReferenceSystem mOriginalCrs;

    QMenu *mMenu = nullptr;

    QMenu *mLayerMenu = nullptr;
    QMenu *mLayoutMenu = nullptr;
    QMenu *mBookmarkMenu = nullptr;

    QgsMapLayerProxyModel *mMapLayerModel = nullptr;
    QgsBookmarkManagerProxyModel *mBookmarkModel = nullptr;

    QList< QAction * > mLayerMenuActions;
    QAction *mUseCanvasExtentAction = nullptr;
    QAction *mUseCurrentExtentAction = nullptr;
    QAction *mDrawOnCanvasAction = nullptr;

    QPointer< const QgsMapLayer > mExtentLayer;
    QString mExtentLayerName;

    std::unique_ptr< QgsMapToolExtent > mMapToolExtent;
    QPointer< QgsMapTool > mMapToolPrevious = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QSize mRatio;

    bool mIsValid = false;
    bool mHasFixedOutputCrs = false;

    QRegularExpression mCondensedRe;
    void setValid( bool valid );

    void setExtentToLayerExtent( const QString &layerId );

    QgsMapLayer *mapLayerFromMimeData( const QMimeData *data ) const;

    friend class TestProcessingGui;


};

#endif // QGSEXTENTWIDGET_H
