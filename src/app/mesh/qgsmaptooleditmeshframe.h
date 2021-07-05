/***************************************************************************
  qgsmaptooleditmeshframe.h - QgsMapToolEditMeshFrame

 ---------------------
 begin                : 24.6.2021
 copyright            : (C) 2021 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMAPTOOLEDITMESHFRAME_H
#define QGSMAPTOOLEDITMESHFRAME_H

#include <QWidget>
#include <QPointer>

#include "qgis_app.h"
#include "qgsmaptooladvanceddigitizing.h"
#include "qgsmeshdataprovider.h"
#include "qgsmesheditor.h"
#include "qgsmeshlayer.h"
#include "qgspointlocator.h"

class QgsRubberBand;
class QgsVertexMarker;
class QgsDoubleSpinBox;
class QgsSnapIndicator;


class APP_EXPORT QgsZValueWidget : public QWidget
{
    Q_OBJECT
  public:

    //! Constructor
    QgsZValueWidget( const QString &label, QWidget *parent = nullptr );

    //! Returns the current \a z value
    double zValue() const;

    //! Sets the current value \a z of the widget
    void setZValue( double z );

    /**
     *  Sets the current value of the widget and set it as the default one ,
     *  that is the value that is retrieve if the z value spin box is cleared
     */
    void setDefaultValue( double z );

    /**
     *  Installs an event filter (see QObject::eventFilter()) on the Z value spin box.
     *  This \a filter can be used to control keyboard entry when the focus is on the Z value spin box.
     */
    void setEventFilterOnValueSpinbox( QObject *filter );

  private:
    QgsDoubleSpinBox *mZValueSpinBox = nullptr;
    double mValue = 0;
};

class APP_EXPORT QgsMapToolEditMeshFrame : public QgsMapToolAdvancedDigitizing
{
    Q_OBJECT
  public:

    //! Constructor
    QgsMapToolEditMeshFrame( QgsMapCanvas *canvas );
    ~QgsMapToolEditMeshFrame();

    void deactivate() override;
    void activate() override;
    bool populateContextMenuWithEvent( QMenu *menu, QgsMapMouseEvent *event ) override;
    Flags flags() const override;

  protected:
    bool eventFilter( QObject *obj, QEvent *ev ) override;
    void cadCanvasPressEvent( QgsMapMouseEvent *e ) override;
    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e ) override;
    void keyPressEvent( QKeyEvent *e ) override;
    void keyReleaseEvent( QKeyEvent *e ) override;

    //! Start addition of a new vertex on double-click
    void canvasDoubleClickEvent( QgsMapMouseEvent *e ) override;

  private slots:
    void setCurrentLayer( QgsMapLayer *layer );
    void onEdit();
    void onEdidingStarted();
    void onEditingStopped();

  private:
    // methods
    void initialize();
    const QgsMeshVertex mapVertex( int index ) const;
    const QgsMeshFace nativeFace( int index ) const;

    void highlightCurrentHoveredFace( const QgsPointXY &mapPoint );
    void highlightCloseVertex( const QgsPointXY &mapPoint );

    void createZValueWidget();
    void deleteZvalueWidget();

    //! Clear all markers and rubber bands
    void clear();
    //! Delete all markers and rubber bands
    void clearAll();

    void addVertex( const QgsPointXY &mapPoint, const QgsPointLocator::Match &mapPointMatch );
    void updateFreeVertices();

    //! Checks if we are closed to a vertex, if yes return the index of the vertex;
    int closeVertex( const QgsPointXY &point ) const;

    QgsPointSequence nativeFaceGeometry( int faceIndex ) const;

    QgsPointXY newFaceMarkerPosition( int vertexIndex );

    void addVertexToFaceCanditate( int vertexIndex );
    bool testNewVertexInFaceCanditate( int vertexIndex );

    // selection private method
    void setSelectedVertices( const QList<int> newSelectedVertex,  Qt::KeyboardModifiers modifiers );
    void clearSelectedvertex();
    void removeSelectedVerticesFromMesh( bool fillHole );
    void selectInGeometry( const QgsGeometry &geometry,  Qt::KeyboardModifiers modifiers );
    void applyZValueOnSelectedVertices();
    void prepareSelection();

    // members
    enum State
    {
      Default,
      AddingNewFace,
      Selecting,
    };

    struct SelectedVertexData
    {
      //here edges are the indexes of the face where the following vertices (ccw) is the other extremity of the edge
      QList<std::array<int, 2>> selectedEdges;
      QList<std::array<int, 2>> meshFixedEdges;
    };

    State mCurrentState = Default;

    QPointer<QgsMeshLayer> mCurrentLayer = nullptr;
    QPointer<QgsMeshEditor> mCurrentEditor = nullptr;
    std::unique_ptr<QgsSnapIndicator> mSnapIndicator;
    int mCurrentFaceIndex = -1;
    int mCurrentVertexIndex = -1;
    QList<int> mNewFaceCandidate;
    bool mDoubleClicks = false;
    QgsPointXY mLastClickPoint;
    double mCurrentZValue = 0;

    //! Rubber band used to highlight a face that is on mouse over and not dragging anything
    QgsRubberBand *mFaceRubberBand = nullptr;
    //! Rubber band used to highlight vertex of the face that is on mouse over and not dragging anything
    QgsRubberBand *mFaceVerticesBand = nullptr;
    //! Rubber band used to highlight the vertex that is in mouse over and not dragging anything
    QgsRubberBand *mVertexBand = nullptr;
    //! Marker used to propose to add a new face when a boundary vertex is highlight
    QgsVertexMarker *mNewFaceMarker = nullptr;
    //! Rubber band used when adding a new face
    QgsRubberBand *mNewFaceBand = nullptr;
    QColor mInvalidFaceColor;
    QColor mValidFaceColor;

    //! members for selection of vertices/faces
    QMap<int, SelectedVertexData> mSelectedVertices;
    QList<int> mSelectedFaces;
    QgsVertexMarker *mSelectFaceMarker = nullptr;
    QgsRubberBand *mSelectionBand = nullptr;
    QPoint mStartSelectionPos;
    QColor mSelectionBandPartiallyFillColor = QColor( 0, 215, 120, 63 );
    QColor mSelectionBandPartiallyStrokeColor = QColor( 0, 204, 102, 100 );
    QColor mSelectionBandTotalFillColor = QColor( 0, 120, 215, 63 );
    QColor mSelectionBandTotalStrokeColor = QColor( 0, 102, 204, 100 );
    QgsRubberBand *mSelectedFacesRubberband = nullptr;
    QMap< int, QgsVertexMarker * > mSelectedVerticesMarker;
    bool mSelectPartiallyContainedFace = false;

    //! Markers that makes visible free vertices
    QList<QgsVertexMarker *> mFreeVertexMarker;

    QgsZValueWidget *mZValueWidget = nullptr;

    QAction *mActionRemoveVerticesFillingHole = nullptr;
    QAction *mActionRemoveVerticesWithoutFillingHole = nullptr;

    friend class TestQgsMapToolEditMesh;
};

#endif // QGSMAPTOOLEDITMESHFRAME_H
