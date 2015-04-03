/***************************************************************************
    qgsadvanceddigitizingdock.h  -  dock for CAD tools
    ----------------------
    begin                : October 2014
    copyright            : (C) Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSADVANCEDDIGITIZINGDOCK
#define QGSADVANCEDDIGITIZINGDOCK

#include <QDockWidget>

#include "qgsmapmouseevent.h"

#include <ui_qgsadvanceddigitizingdockwidgetbase.h>


class QgsAdvancedDigitizingCanvasItem;
class QgsMapCanvas;
class QgsMapTool;
class QgsMapToolAdvancedDigitizing;
class QgsMessageBarItem;
class QgsPoint;

// tolerances for soft constraints (last values, and common angles)
// for angles, both tolerance in pixels and degrees are used for better performance
static const double SoftConstraintTolerancePixel = 15;
static const double SoftConstraintToleranceDegrees = 10;

/**
 * @brief The QgsAdvancedDigitizingDock class is a dockable widget
 * used to handle the CAD tools on top of a selection of map tools.
 * It handles both the UI and the constraints. Constraints are applied
 * by implemeting filters called from QgsMapToolAdvancedDigitizing.
 */
class APP_EXPORT QgsAdvancedDigitizingDockWidget : public QDockWidget, private Ui::QgsAdvancedDigitizingDockWidgetBase
{
    Q_OBJECT
    Q_FLAGS( CadCapacities )

  public:
    /**
     * @brief The CadCapacity enum defines the possible constraints to be set
     *  depending on the number of points in the CAD point list (the list of points
     * currently digitized)
     */
    enum CadCapacity
    {
      AbsoluteAngle = 1, // = Azimuth
      RelativeAngle = 2, // also for parallel and perpendicular
      RelativeCoordinates = 4, // this corresponds to distance and relative coordinates
    };
    Q_DECLARE_FLAGS( CadCapacities, CadCapacity )

    enum AdditionalConstraint
    {
      NoConstraint,
      Perpendicular,
      Parallel
    };

    /**
     * @brief The CadConstraint is an abstract class for all basic constraints (angle/distance/x/y).
     * It contains all values (locked, value, relative) and pointers to corresponding widgets.
     * @note Relative is not mandatory since it is not used for distance.
     */
    class CadConstraint
    {
      public:
        enum LockMode
        {
          NoLock,
          SoftLock,
          HardLock
        };

        CadConstraint( QLineEdit* lineEdit, QToolButton* lockerButton, QToolButton* relativeButton = 0 )
            : mLineEdit( lineEdit )
            , mLockerButton( lockerButton )
            , mRelativeButton( relativeButton )
            , mLockMode( NoLock )
            , mRelative( false )
            , mValue( 0.0 )
        {}

        LockMode lockMode() const { return mLockMode;}
        bool isLocked() const {return mLockMode != NoLock;}
        bool relative() const {return mRelative;}
        double value() const {return mValue;}

        QLineEdit* lineEdit() const {return mLineEdit;}

        void setLockMode( LockMode mode );
        void setRelative( bool relative );
        void setValue( double value );

        void toggleLocked();
        void toggleRelative();

      private:
        QLineEdit* mLineEdit;
        QToolButton* mLockerButton;
        QToolButton* mRelativeButton;
        LockMode mLockMode;
        bool mRelative;
        double mValue;
    };

    //! performs the intersection of a circle and a line
    //! @note from the two solutions, the intersection will be set to the closest point
    static bool lineCircleIntersection( const QgsPoint& center, const double radius, const QList<QgsPoint>& segment, QgsPoint& intersection );

    explicit QgsAdvancedDigitizingDockWidget( QgsMapCanvas* canvas, QWidget *parent = 0 );

    ~QgsAdvancedDigitizingDockWidget();

    void hideEvent( QHideEvent* ) override;

    virtual bool canvasPressEventFilter( QgsMapMouseEvent* e );
    virtual bool canvasReleaseEventFilter( QgsMapMouseEvent* e );
    virtual bool canvasMoveEventFilter( QgsMapMouseEvent* e );
    virtual bool canvasKeyPressEventFilter( QKeyEvent *e );

    QgsMapMouseEvent::SnappingMode snappingMode() {return mSnappingMode;}

    //! key press event on the dock
    void keyPressEvent( QKeyEvent* e ) override;

    //! determines if CAD tools are enabled or if map tools behaves "nomally"
    bool cadEnabled() const { return mCadEnabled; }

    //! construction mode is used to draw intermediate points. These points won't be given any further (i.e. to the map tools)
    bool constructionMode() const {return mConstructionMode;}

    //! Additional constraints are used to place perpendicular/parallel segments to snapped segments on the canvas
    AdditionalConstraint additionalConstraint() const  {return mAdditionalConstraint;}
    const CadConstraint* constraintAngle()const  {return mAngleConstraint;}
    const CadConstraint* constraintDistance() const {return mDistanceConstraint;}
    const CadConstraint* constraintX() const {return mXConstraint;}
    const CadConstraint* constraintY() const {return mYConstraint;}
    bool commonAngleConstraint() const {return mCommonAngleConstraint;}

    /** helpers for the CAD point list. The CAD point list is the list of points
     * currently digitized. It contains both  "normal" points and intermediate points (construction mode).
     */
    QgsPoint currentPoint( bool *exist = 0 ) const;
    QgsPoint previousPoint( bool *exist = 0 ) const;
    QgsPoint penultimatePoint( bool *exist = 0 ) const;
    int pointsCount() const {return mCadPointList.count();}
    bool snappedToVertex() const {return mSnappedToVertex;}
    const QList<QgsPoint>& snappedSegment() const {return mSnappedSegment;}

    //! return the action used to enable/disable the tools
    QAction* enableAction() { return mEnableAction; }

  public slots:
    //! whenever a map tool changes, determines if the dock shall be activated or not
    void mapToolChanged( QgsMapTool* tool );

  private slots:
    //! set the additiona constraint by clicking on the perpendicular/parallel buttons
    void addtionalConstraintClicked( bool activated );

    //! lock/unlock a constraint and set its value
    void lockConstraint( bool activate = true );

    //! unlock all constraints
    void releaseLocks();

    //! set the relative properties of constraints
    void setConstraintRelative( bool activate );

    //! activate/deactuvate tools. It is called when tools are activated manually (from the GUI)
    //! it will call setCadEnabled to properly update the UI.
    void activateCad( bool enabled );

    //! enable/disable construction mode (events are not forwarded to the map tool)
    void setConstructionMode( bool enabled );

    //! settings button triggered
    void settingsButtonTriggered( QAction* action );

  private:
    //! updates the UI depending on activation of the tools and clear points / release locks.
    void setCadEnabled( bool enabled );

    /**
     * @brief updateCapacity updates the cad capacities depending on the point list and update the UI according to the capabilities.
     * @param updateUIwithoutChange if true, it will update the UI even if new capacities are not different from previous ones.
     */
    void updateCapacity( bool updateUIwithoutChange = false );

    //! defines the additional constraint to be used (no/parallel/perpendicular)
    void lockAdditionalConstraint( AdditionalConstraint constraint );

    //! apply the CAD constraints. The will modify the position of the map event in map coordinates by applying the CAD constraints.
    //! @return false if no solution was found (invalid constraints)
    virtual bool applyConstraints( QgsMapMouseEvent* e );

    //! align to segment for additional constraint.
    //! If additional constraints are used, this will determine the angle to be locked depending on the snapped segment.
    bool alignToSegment( QgsMapMouseEvent* e, CadConstraint::LockMode lockMode = CadConstraint::HardLock );

    //! add point to the CAD point list
    void addPoint( QgsPoint point );
    //! update the current point in the CAD point list
    void updateCurrentPoint( QgsPoint point );
    //! remove previous point in the CAD point list
    void removePreviousPoint();
    //! remove all points from the CAD point list
    void clearPoints();

    //! filters key press
    //! @note called by eventFilter (fitler on line edits), canvasKeyPressEvent (filter on map tool) and keyPressEvent (filter on dock)
    bool filterKeyPress( QKeyEvent* e );

    //! event filter for line edits in the dock UI (angle/distance/x/y line edits)
    bool eventFilter( QObject *obj, QEvent *event ) override;

    //! trigger fake mouse move event to update map tool rubber band and/or show new constraints
    void triggerMouseMoveEvent();

    QgsMapCanvas* mMapCanvas;
    QgsAdvancedDigitizingCanvasItem* mCadPaintItem;

    QList<QgsMapToolAdvancedDigitizing*> mMapToolList;
    QgsMapToolAdvancedDigitizing* mCurrentMapTool;

    CadCapacities mCapacities;

    // CAD properties
    //! is CAD currently enabled for current map tool
    bool mCadEnabled;
    bool mConstructionMode;
    QgsMapMouseEvent::SnappingMode mSnappingMode;

    // constraints
    CadConstraint* mAngleConstraint;
    CadConstraint* mDistanceConstraint;
    CadConstraint* mXConstraint;
    CadConstraint* mYConstraint;
    AdditionalConstraint mAdditionalConstraint;
    int mCommonAngleConstraint; // if 0: do not snap to common angles

    // point list and current snap point / segment
    QList<QgsPoint> mCadPointList;
    bool mSnappedToVertex;
    QList<QgsPoint> mSnappedSegment;

    // error message
    QgsMessageBarItem* mErrorMessage;
    bool mErrorMessageDisplayed;

    // UI
    QAction* mEnableAction;
    QMap< QAction*, int > mCommonAngleActions; // map the common angle actions with their angle values
    QMap< QAction*, QgsMapMouseEvent::SnappingMode > mSnappingActions; // map the snapping mode actions with their values
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsAdvancedDigitizingDockWidget::CadCapacities )

#endif // QGSADVANCEDDIGITIZINGDOCK_H
