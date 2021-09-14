/***************************************************************************
                              qgsmaptooloffsetcurve.h
    ------------------------------------------------------------
    begin                : February 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLOFFSETCURVE_H
#define QGSMAPTOOLOFFSETCURVE_H

#include "qgsmaptooledit.h"
#include "qgsgeometry.h"
#include "qgis_app.h"
#include "ui_qgsoffsetuserinputwidget.h"
#include "qgspointlocator.h"
#include "qgsfeature.h"

class QGridLayout;

class QgsSnapIndicator;
class QgsDoubleSpinBox;
class QGraphicsProxyWidget;
class QgsFeature;

class APP_EXPORT QgsOffsetUserWidget : public QWidget, private Ui::QgsOffsetUserInputBase
{
    Q_OBJECT

  public:

    explicit QgsOffsetUserWidget( QWidget *parent = nullptr );

    void setOffset( double offset );
    double offset();
    QDoubleSpinBox *editor() const {return mOffsetSpinBox;}

    void setPolygonMode( bool polygon );

  signals:
    void offsetChanged( double offset );
    void offsetEditingFinished( double offset, const Qt::KeyboardModifiers &modifiers );
    void offsetEditingCanceled();
    void offsetConfigChanged();

  protected:
    bool eventFilter( QObject *obj, QEvent *ev ) override;
};

class APP_EXPORT QgsMapToolOffsetCurve: public QgsMapToolEdit
{
    Q_OBJECT
  public:
    QgsMapToolOffsetCurve( QgsMapCanvas *canvas );
    ~QgsMapToolOffsetCurve() override;

    void keyPressEvent( QKeyEvent *e ) override;
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void canvasMoveEvent( QgsMapMouseEvent *e ) override;

  private slots:
    //! Places curve offset from the mouse position or from the value entered in the spin box
    void updateGeometryAndRubberBand( double offset );

    void applyOffsetFromWidget( double offset, Qt::KeyboardModifiers modifiers );

    //! Apply the offset either from the spin box or from the mouse event
    void applyOffset( double offset, Qt::KeyboardModifiers modifiers );

    void cancel();

  private:
    //! Rubberband that shows the position of the offset curve
    QgsRubberBand *mRubberBand = nullptr;
    //! Snapping indicators
    std::unique_ptr<QgsSnapIndicator> mSnapIndicator;

    //! The layer being manipulated
    QgsVectorLayer *mSourceLayer = nullptr;

    //! Geometry to manipulate
    QgsGeometry mOriginalGeometry;
    //! Geometry being manipulated
    QgsGeometry mManipulatedGeometry;
    //! Geometry after manipulation
    QgsGeometry mModifiedGeometry;
    //! ID of manipulated feature
    QgsFeatureId mModifiedFeature = -1;
    int mModifiedPart = -1;
    int mModifiedRing = -1;

    //! Internal flag to distinguish move from click
    bool mGeometryModified = false;

    //! Shows current distance value and allows numerical editing
    QgsOffsetUserWidget *mUserInputWidget = nullptr;

    //! Forces geometry copy (no modification of geometry in current layer)
    bool mCtrlHeldOnFirstClick = false;

    QgsFeature mSourceFeature;

    double calculateOffset( const QgsPointXY &mapPoint );

    void createUserInputWidget();
    void deleteUserInputWidget();

    void prepareGeometry( const QgsPointLocator::Match &match, QgsFeature &snappedFeature );

    void deleteRubberBandAndGeometry();


    //! Creates a linestring from the polygon ring containing the snapped vertex. Caller takes ownership of the created object
    QgsGeometry linestringFromPolygon( const QgsGeometry &featureGeom, int vertex );
    //! Returns a single line from a multiline (or does nothing if geometry is already a single line). Deletes the input geometry
    QgsGeometry convertToSingleLine( const QgsGeometry &geom, int vertex );
};

#endif // QGSMAPTOOLOFFSETCURVE_H
