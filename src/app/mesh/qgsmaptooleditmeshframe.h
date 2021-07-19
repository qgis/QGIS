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

    QWidget *keyboardEntryWidget() const;

  private:
    QgsDoubleSpinBox *mZValueSpinBox = nullptr;
};

class APP_EXPORT QgsMapToolEditMeshFrame : public QgsMapToolAdvancedDigitizing
{
    Q_OBJECT
  public:

    //! Constructor
    QgsMapToolEditMeshFrame( QgsMapCanvas *canvas );
    ~QgsMapToolEditMeshFrame();

    QList<QAction *> actions() const;
    QList<QAction *> mapToolActions();

    void deactivate() override;
    void activate() override;
    bool populateContextMenuWithEvent( QMenu *menu, QgsMapMouseEvent *event ) override;
    Flags flags() const override;

  protected:
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
    void onEditingStarted();
    void onEditingStopped();
    void removeSelectedVerticesFromMesh( bool fillHole );
    void removeFacesFromMesh();
    void splitSelectedFaces();

  private:

    enum State
    {
      Digitizing, //!< Digitizing action can be start (add/remove vertices, selection, add/remove faces, move vertices)
      AddingNewFace, //!< Adding a face has been start and the user have to choose or digitize vertices
      Selecting, //!< Selection is in process
      MovingVertex //!< Moving vertex or vertices is processing
    };

    typedef QPair<int, int> Edge; //first face index, second the vertex index corresponding to the end extremity (ccw)

    // methods
    void initialize();
    void activateWithState( State state );
    const QgsMeshVertex mapVertex( int index ) const;
    const QgsPointXY mapVertexXY( int index ) const;
    const QgsMeshFace nativeFace( int index ) const;

    void highLight( const QgsPointXY &mapPoint );
    void highlightCurrentHoveredFace( const QgsPointXY &mapPoint );
    void highlightCloseVertex( const QgsPointXY &mapPoint );
    void highlightCloseEdge( const QgsPointXY &mapPoint );
    bool edgeCanBeInteractive( int vertexIndex1, int vertexIndex2 ) const;
    bool faceCanBeInteractive( int faceIndex ) const;

    void createZValueWidget();
    void deleteZvalueWidget();

    void clearSelection();
    void clearCanvasHelpers();
    void clearEdgeHelpers();

    void clearAll();

    void addVertex( const QgsPointXY &mapPoint, const QgsPointLocator::Match &mapPointMatch, Qt::KeyboardModifiers modifiers );
    void updateFreeVertices();

    //! Checks if we are closed to a vertex, if yes return the index of the vertex;
    int closeVertex( const QgsPointXY &point ) const;

    QgsPointSequence nativeFaceGeometry( int faceIndex ) const;
    QVector<QgsPointXY> edgeGeometry( const Edge &edge ) const;
    QVector<int> edgeVertices( const Edge &edge ) const;

    QgsPointXY newFaceMarkerPosition( int vertexIndex );

    void addVertexToFaceCanditate( int vertexIndex );
    bool testNewVertexInFaceCanditate( int vertexIndex );

    // selection methods
    void select( const QgsPointXY &mapPoint, Qt::KeyboardModifiers modifiers, double tolerance );
    void setSelectedVertices( const QList<int> newSelectedVertex,  Qt::KeyboardModifiers modifiers );
    void clearSelectedvertex();
    void selectInGeometry( const QgsGeometry &geometry,  Qt::KeyboardModifiers modifiers );
    void applyZValueOnSelectedVertices();
    void prepareSelection();
    void updateSelectecVerticesMarker();

    bool testBorderMovingFace( const QgsMeshFace &borderMovingfaces, const QgsVector &translation ) const;

    // members
    struct SelectedVertexData
    {
      //Here edges are the indexes of the face where the following vertices (ccw) is the other extremity of the edge
      QList<Edge> selectedEdges;
      QList<Edge> meshFixedEdges;
    };

    bool mIsInitialized = false;
    State mCurrentState = Digitizing;
    State mPreviousState = Digitizing; //used to store a state and restore it after a particular action as selecting
    bool mLeftButtonPressed = false;
    bool mKeepSelectionOnEdit = false;


    QPointer<QgsMeshLayer> mCurrentLayer = nullptr; //not own
    QPointer<QgsMeshEditor> mCurrentEditor = nullptr; // own by mesh layer
    std::unique_ptr<QgsSnapIndicator> mSnapIndicator;
    int mCurrentFaceIndex = -1;
    Edge mCurrentEdge = {-1, -1};
    int mCurrentVertexIndex = -1;
    QList<int> mNewFaceCandidate;
    bool mDoubleClicks = false;
    QgsPointXY mLastClickPoint;
    double mOrdinaryZValue = 0;
    bool mIsSelectedZValue = false;
    double mSelectedZValue = 0;

    //! Rubber band used to highlight a face that is on mouse over and not dragging anything, own by map canvas
    QgsRubberBand *mFaceRubberBand = nullptr;
    //! Rubber band used to highlight vertex of the face that is on mouse over and not dragging anything, own by map canvas
    QgsRubberBand *mFaceVerticesBand = nullptr;
    //! Rubber band used to highlight the vertex that is in mouse over and not dragging anything, own by map canvas
    QgsRubberBand *mVertexBand = nullptr;
    //! Rubber band used to highlight the edge that is in mouse over, own by map canvas
    QgsRubberBand *mEdgeBand = nullptr;
    //! Marker used to propose to add a new face when a boundary vertex is highlight, own by map canvas
    QgsVertexMarker *mNewFaceMarker = nullptr;
    //! Rubber band used when adding a new face, own by map canvas
    QgsRubberBand *mNewFaceBand = nullptr;
    QColor mInvalidFaceColor;
    QColor mValidFaceColor;

    //! Markers that makes visible free vertices, own by map canvas
    QList<QgsVertexMarker *> mFreeVertexMarker;

    //! members for selection of vertices/faces
    QMap<int, SelectedVertexData> mSelectedVertices;
    QSet<int> mSelectedFaces;
    QSet<int> mConcernedFaceBySelection;
    QgsVertexMarker *mSelectFaceMarker = nullptr; //own by map canvas
    QgsVertexMarker *mSelectEdgeMarker = nullptr; //own by map canvas
    QgsRubberBand *mSelectionBand = nullptr; //own by map canvas
    QPoint mStartSelectionPos;
    QColor mSelectionBandPartiallyFillColor = QColor( 0, 215, 120, 63 );
    QColor mSelectionBandPartiallyStrokeColor = QColor( 0, 204, 102, 100 );
    QColor mSelectionBandTotalFillColor = QColor( 0, 120, 215, 63 );
    QColor mSelectionBandTotalStrokeColor = QColor( 0, 102, 204, 100 );
    QgsRubberBand *mSelectedFacesRubberband = nullptr; //own by map canvas
    QMap< int, QgsVertexMarker * > mSelectedVerticesMarker;
    bool mSelectPartiallyContainedFace = false;

    //! members for moving vertices
    QgsPointXY mStartMovingPoint;
    bool mCanMovingStart = false;
    QgsRubberBand *mMovingEdgesRubberband = nullptr; //own by map canvas
    QgsRubberBand *mMovingFacesRubberband = nullptr; //own by map canvas
    QgsRubberBand *mMovingVerticesRubberband = nullptr; //own by map canvas
    bool mIsMovingAllowed = false;

    //! members for edge flip
    QgsVertexMarker *mFlipEdgeMarker = nullptr; //own by map canvas

    //! members for merge face
    QgsVertexMarker *mMergeFaceMarker = nullptr; //own by map canvas

    //! members for split face
    int mSplittableFaceCount = 0;

    QgsZValueWidget *mZValueWidget = nullptr; //own by QgsUserInputWidget instance

    QAction *mActionRemoveVerticesFillingHole = nullptr;
    QAction *mActionRemoveVerticesWithoutFillingHole = nullptr;
    QAction *mActionRemoveFaces = nullptr;
    QAction *mActionSplitFaces = nullptr;

    QAction *mActionDelaunayTriangulation = nullptr;
    QAction *mActionFacesRefinement = nullptr;

    QAction *mActionDigitizing = nullptr;

    friend class TestQgsMapToolEditMesh;
};

#endif // QGSMAPTOOLEDITMESHFRAME_H
