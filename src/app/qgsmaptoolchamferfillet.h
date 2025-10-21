/***************************************************************************
                              qgsmaptoolchamferfillet.h
    ------------------------------------------------------------
    begin                : September 2025
    copyright            : (C) 2025 by Oslandia
    email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLCHAMFERFILLET_H
#define QGSMAPTOOLCHAMFERFILLET_H

#include "qgsmaptooledit.h"
#include "qgsgeometry.h"
#include "qgis_app.h"
#include "ui_qgschamferfilletuserinputwidget.h"
#include "qgspointlocator.h"
#include "qgsfeature.h"

class QGridLayout;

class QgsSnapIndicator;
class QgsDoubleSpinBox;
class QGraphicsProxyWidget;
class QgsFeature;

class QgsSettingsEntryBool;
class QgsSettingsEntryDouble;
class QgsSettingsEntryInteger;
template<class T> class QgsSettingsEntryEnumFlag;

class APP_EXPORT QgsChamferFilletUserWidget : public QWidget, private Ui::QgsChamferFilletUserInputBase
{
    Q_OBJECT

  public:
    explicit QgsChamferFilletUserWidget( QWidget *parent = nullptr );

    void setValue1( double value1 );
    double value1() const;
    void setValue2( double value2 );
    double value2() const;
    void setMaximumValue1( double maximum );
    QDoubleSpinBox *editor() const { return mValue1SpinBox; }
    QgsGeometry::ChamferFilletOperationType operation() const;

  signals:
    void distanceEditingFinished( const Qt::KeyboardModifiers &modifiers );
    void distanceEditingCanceled();
    void distanceConfigChanged();

  protected:
    bool eventFilter( QObject *obj, QEvent *ev ) override;
};

class APP_EXPORT QgsMapToolChamferFillet : public QgsMapToolEdit
{
    Q_OBJECT
  public:
    QgsMapToolChamferFillet( QgsMapCanvas *canvas );
    ~QgsMapToolChamferFillet() override;

    void keyPressEvent( QKeyEvent *e ) override;
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void canvasMoveEvent( QgsMapMouseEvent *e ) override;

    //! Settings entry digitizing chamfer/fillet: operation name
    static const QgsSettingsEntryEnumFlag<QgsGeometry::ChamferFilletOperationType> *settingsOperation;

    //! Settings entry digitizing chamfer/fillet: nb fillet segment
    static const QgsSettingsEntryInteger *settingsFilletSegment;

    //! Settings entry digitizing chamfer/fillet: value1
    static const QgsSettingsEntryDouble *settingsValue1;

    //! Settings entry digitizing chamfer/fillet: value2
    static const QgsSettingsEntryDouble *settingsValue2;

    //! Settings entry digitizing chamfer/fillet: state for locker 1
    static const QgsSettingsEntryBool *settingsLock1;

    //! Settings entry digitizing chamfer/fillet: state for locker 2
    static const QgsSettingsEntryBool *settingsLock2;

  private slots:
    //! Places curve chamfer from the mouse position or from the value entered in the spin box
    void updateGeometryAndRubberBand( double value1, double value2 );

    void applyOperationFromWidget( Qt::KeyboardModifiers modifiers );

    //! Apply the chamfer either from the spin box or from the mouse event
    void applyOperation( double value1, double value2, Qt::KeyboardModifiers modifiers );

    void cancel();

    //! Only to wrap call to updateGeometryAndRubberBand
    void configChanged();

  private:
    //! Rubberband that shows the position of the chamfer curve
    QgsRubberBand *mRubberBand = nullptr;
    //! Snapping indicators
    std::unique_ptr<QgsSnapIndicator> mSnapIndicator;

    //! The layer being manipulated
    QPointer<QgsVectorLayer> mSourceLayer = nullptr;

    //! Geometry to manipulate
    QgsGeometry mOriginalGeometryInSourceLayerCrs;
    //! Geometry being manipulated
    QgsGeometry mManipulatedGeometryInSourceLayerCrs;
    //! Geometry after manipulation
    QgsGeometry mModifiedGeometry;
    //! ID of manipulated feature
    QgsFeatureId mModifiedFeature = -1;
    int mVertexIndex = -1;
    QgsPoint mVertexPointInSourceLayerCrs;

    //! Internal flag to distinguish move from click
    bool mGeometryModified = false;

    //! Shows current distance value and allows numerical editing
    QgsChamferFilletUserWidget *mUserInputWidget = nullptr;

    QgsFeature mSourceFeature;

    //! limits number of call to updateGeometryAndRubberBand
    QElapsedTimer mLastMouseMove;

    void calculateDistances( const QgsPointXY &mapPoint, double &value1, double &value2 );

    void createUserInputWidget();
    void deleteUserInputWidget();

    bool prepareGeometry( const QgsPointLocator::Match &match, QgsFeature &snappedFeature );

    void deleteRubberBandAndGeometry();


    //! Creates a linestring from the polygon ring containing the snapped vertex. Caller takes ownership of the created object
    QgsGeometry linestringFromPolygon( const QgsGeometry &featureGeom, int vertex );
    //! Returns a single line from a multiline (or does nothing if geometry is already a single line). Deletes the input geometry
    QgsGeometry convertToSingleLine( const QgsGeometry &geom, int vertex );
};

#endif // QGSMAPTOOLCHAMFERFILLET_H
