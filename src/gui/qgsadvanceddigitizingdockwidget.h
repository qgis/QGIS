/***************************************************************************
    qgsadvanceddigitizingdockwidget.h  -  dock for CAD tools
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

#include <QList>

#include <memory>

#include "ui_qgsadvanceddigitizingdockwidgetbase.h"
#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsdockwidget.h"
#include "qgsmessagebaritem.h"
#include "qgspointxy.h"
#include "qgspointlocator.h"


class QgsAdvancedDigitizingCanvasItem;
class QgsMapCanvas;
class QgsMapTool;
class QgsMapToolAdvancedDigitizing;
class QgsMapMouseEvent;

/**
 * \ingroup gui
 * \brief The QgsAdvancedDigitizingDockWidget class is a dockable widget
 * used to handle the CAD tools on top of a selection of map tools.
 * It handles both the UI and the constraints. Constraints are applied
 * by implementing filters called from QgsMapToolAdvancedDigitizing.
 */
class GUI_EXPORT QgsAdvancedDigitizingDockWidget : public QgsDockWidget, private Ui::QgsAdvancedDigitizingDockWidgetBase
{
    Q_OBJECT

  public:

    /**
     * The CadCapacity enum defines the possible constraints to be set
     * depending on the number of points in the CAD point list (the list of points
     * currently digitized)
     */
    enum CadCapacity
    {
      AbsoluteAngle = 1, //!< Azimuth
      RelativeAngle = 2, //!< Also for parallel and perpendicular
      RelativeCoordinates = 4, //!< This corresponds to distance and relative coordinates
    };
    Q_DECLARE_FLAGS( CadCapacities, CadCapacity )
    Q_FLAG( CadCapacities )

    /**
     * Additional constraints which can be enabled
     */
    enum AdditionalConstraint
    {
      NoConstraint,  //!< No additional constraint
      Perpendicular, //!< Perpendicular
      Parallel       //!< Parallel
    };

    /**
     * \ingroup gui
     * \brief The CadConstraint is an abstract class for all basic constraints (angle/distance/x/y).
     * It contains all values (locked, value, relative) and pointers to corresponding widgets.
     * \note Relative is not mandatory since it is not used for distance.
     */
    class GUI_EXPORT CadConstraint
    {
      public:

        /**
         * The lock mode
         */
        enum LockMode
        {
          NoLock,
          SoftLock,
          HardLock
        };

        /**
         * Constructor for CadConstraint.
         * \param lineEdit associated line edit for constraint value
         * \param lockerButton associated button for locking constraint
         * \param relativeButton optional button for toggling relative constraint mode
         * \param repeatingLockButton optional button for toggling repeating lock mode
         */
        CadConstraint( QLineEdit *lineEdit, QToolButton *lockerButton, QToolButton *relativeButton = nullptr, QToolButton *repeatingLockButton = nullptr )
          : mLineEdit( lineEdit )
          , mLockerButton( lockerButton )
          , mRelativeButton( relativeButton )
          , mRepeatingLockButton( repeatingLockButton )
          , mLockMode( NoLock )
          , mRepeatingLock( false )
          , mRelative( false )
          , mValue( 0.0 )
        {}

        /**
         * The current lock mode of this constraint
         * \returns Lock mode
         */
        LockMode lockMode() const { return mLockMode; }

        /**
         * Is any kind of lock mode enabled
         */
        bool isLocked() const { return mLockMode != NoLock; }

        /**
         * Returns true if a repeating lock is set for the constraint. Repeating locks are not
         * automatically cleared after a new point is added.
         * \see setRepeatingLock()
         * \since QGIS 2.16
         */
        bool isRepeatingLock() const { return mRepeatingLock; }

        /**
         * Is the constraint in relative mode
         */
        bool relative() const { return mRelative; }

        /**
         * The value of the constraint
         */
        double value() const { return mValue; }

        /**
         * The line edit that manages the value of the constraint
         */
        QLineEdit *lineEdit() const { return mLineEdit; }

        /**
         * Set the lock mode
         */
        void setLockMode( LockMode mode );

        /**
         * Sets whether a repeating lock is set for the constraint. Repeating locks are not
         * automatically cleared after a new point is added.
         * \param repeating set to true to set the lock to repeat automatically
         * \see isRepeatingLock()
         * \since QGIS 2.16
         */
        void setRepeatingLock( bool repeating );

        /**
         * Set if the constraint should be treated relative
         */
        void setRelative( bool relative );

        /**
         * Set the value of the constraint
         * \param value new value for constraint
         * \param updateWidget set to false to prevent automatically updating the associated widget's value
         */
        void setValue( double value, bool updateWidget = true );

        /**
         * Toggle lock mode
         */
        void toggleLocked();

        /**
         * Toggle relative mode
         */
        void toggleRelative();

      private:
        QLineEdit *mLineEdit = nullptr;
        QToolButton *mLockerButton = nullptr;
        QToolButton *mRelativeButton = nullptr;
        QToolButton *mRepeatingLockButton = nullptr;
        LockMode mLockMode;
        bool mRepeatingLock;
        bool mRelative;
        double mValue;
    };

    /**
     * Create an advanced digitizing dock widget
     * \param canvas The map canvas on which the widget operates
     * \param parent The parent
     */
    explicit QgsAdvancedDigitizingDockWidget( QgsMapCanvas *canvas, QWidget *parent = nullptr );

    /**
     * Disables the CAD tools when hiding the dock
     */
    void hideEvent( QHideEvent * ) override;

    /**
     * Filter key events to e.g. toggle construction mode or adapt constraints
     *
     * \param e A mouse event (may be modified)
     * \returns  If the event is hidden (construction mode hides events from the maptool)
     */
    bool canvasKeyPressEventFilter( QKeyEvent *e );

    /**
     * apply the CAD constraints. The will modify the position of the map event in map coordinates by applying the CAD constraints.
     * \returns false if no solution was found (invalid constraints)
     */
    bool applyConstraints( QgsMapMouseEvent *e );

    /**
     * align to segment for additional constraint.
     * If additional constraints are used, this will determine the angle to be locked depending on the snapped segment.
     * \since QGIS 3.0
     */
    bool alignToSegment( QgsMapMouseEvent *e, QgsAdvancedDigitizingDockWidget::CadConstraint::LockMode lockMode = QgsAdvancedDigitizingDockWidget::CadConstraint::HardLock );

    /**
     * unlock all constraints
     * \param releaseRepeatingLocks set to false to preserve the lock for any constraints set to repeating lock mode
     * \since QGIS 3.0
     */
    void releaseLocks( bool releaseRepeatingLocks = true );

    /**
     * Clear any cached previous clicks and helper lines
     */
    void clear();

    void keyPressEvent( QKeyEvent *e ) override;

    //! determines if CAD tools are enabled or if map tools behaves "nomally"
    bool cadEnabled() const { return mCadEnabled; }

    //! construction mode is used to draw intermediate points. These points won't be given any further (i.e. to the map tools)
    bool constructionMode() const { return mConstructionMode; }

    /**
     * Returns the additional constraints which are used to place
     * perpendicular/parallel segments to snapped segments on the canvas
     */
    AdditionalConstraint additionalConstraint() const  { return mAdditionalConstraint; }
    //! Returns the \a CadConstraint on the angle
    const CadConstraint *constraintAngle() const  { return mAngleConstraint.get(); }
    //! Returns the \a CadConstraint on the distance
    const CadConstraint *constraintDistance() const { return mDistanceConstraint.get(); }
    //! Returns the \a CadConstraint on the X coordinate
    const CadConstraint *constraintX() const { return mXConstraint.get(); }
    //! Returns the \a CadConstraint on the Y coordinate
    const CadConstraint *constraintY() const { return mYConstraint.get(); }
    //! Returns true if a constraint on a common angle is active
    bool commonAngleConstraint() const { return !qgsDoubleNear( mCommonAngleConstraint, 0.0 ); }

    /**
     * Returns the point locator match
     * \since QGIS 3.4
     */
    QgsPointLocator::Match mapPointMatch() const { return mSnapMatch; }

    /**
     * Removes all points from the CAD point list
     * \since QGIS 3.0
     */
    void clearPoints();

    /**
     * Adds point to the CAD point list
     * \since QGIS 3.0
     */
    void addPoint( const QgsPointXY &point );

    /**
     * Configures list of current CAD points
     *
     * Some map tools may find it useful to override list of CAD points that is otherwise
     * automatically populated when user clicks with left mouse button on map canvas.
     * \since QGIS 3.0
     */
    void setPoints( const QList<QgsPointXY> &points );

    /**
     * The last point.
     * Helper for the CAD point list. The CAD point list is the list of points
     * currently digitized. It contains both  "normal" points and intermediate points (construction mode).
     */
    QgsPointXY currentPoint( bool *exists  = nullptr ) const;

    /**
     * The previous point.
     * Helper for the CAD point list. The CAD point list is the list of points
     * currently digitized. It contains both  "normal" points and intermediate points (construction mode).
     */
    QgsPointXY previousPoint( bool *exists = nullptr ) const;

    /**
     * The penultimate point.
     * Helper for the CAD point list. The CAD point list is the list of points
     * currently digitized. It contains both  "normal" points and intermediate points (construction mode).
     */
    QgsPointXY penultimatePoint( bool *exists = nullptr ) const;

    /**
     * The number of points in the CAD point helper list
     */
    inline int pointsCount() const { return mCadPointList.count(); }

    /**
     * Is it snapped to a vertex
     */
    inline bool snappedToVertex() const { return mSnappedToVertex; }

    /**
     * Snapped to a segment
     */
    QList<QgsPointXY> snappedSegment() const { return mSnappedSegment; }

    //! Returns the action used to enable/disable the tools
    QAction *enableAction() { return mEnableAction; }

    /**
     * Enables the tool (call this when an appropriate map tool is set and in the condition to make use of
     * cad digitizing)
     * Normally done automatically from QgsMapToolAdvancedDigitizing::activate() but may need to be fine tuned
     * if the map tool depends on preconditions like a feature selection.
     */
    void enable();

    /**
     * Disable the widget. Normally done automatically from QgsMapToolAdvancedDigitizing::deactivate().
     */
    void disable();

    /**
     * Updates canvas item that displays constraints on the ma
     * \since QGIS 3.0
     */
    void updateCadPaintItem();

  signals:

    /**
     * Push a warning
     *
     * \param message An informative message
     */
    void pushWarning( const QString &message );

    /**
     * Remove any previously emitted warnings (if any)
     */
    void popWarning();

    /**
     * Sometimes a constraint may change the current point out of a mouse event. This happens normally
     * when a constraint is toggled.
     *
     * \param point The last known digitizing point. Can be used to emulate a mouse event.
     */
    void pointChanged( const QgsPointXY &point );

  private slots:
    //! Sets the additional constraint by clicking on the perpendicular/parallel buttons
    void additionalConstraintClicked( bool activated );

    //! lock/unlock a constraint and set its value
    void lockConstraint( bool activate = true );

    /**
     * Called when user has manually altered a constraint value. Any entered expressions will
     * be left intact
     */
    void constraintTextEdited( const QString &textValue );

    /**
     * Called when a constraint input widget has lost focus. Any entered expressions
     * will be converted to their calculated value
     */
    void constraintFocusOut();

    //! Sets the relative properties of constraints
    void setConstraintRelative( bool activate );

    //! Sets the repeating lock property of constraints
    void setConstraintRepeatingLock( bool activate );

    /**
     * activate/deactivate tools. It is called when tools are activated manually (from the GUI)
     * it will call setCadEnabled to properly update the UI.
     */
    void activateCad( bool enabled );

    //! enable/disable construction mode (events are not forwarded to the map tool)
    void setConstructionMode( bool enabled );

    //! settings button triggered
    void settingsButtonTriggered( QAction *action );

  private:
    //! updates the UI depending on activation of the tools and clear points / release locks.
    void setCadEnabled( bool enabled );

    /**
     * \brief updateCapacity updates the cad capacities depending on the point list and update the UI according to the capabilities.
     * \param updateUIwithoutChange if true, it will update the UI even if new capacities are not different from previous ones.
     */
    void updateCapacity( bool updateUIwithoutChange = false );

    //! defines the additional constraint to be used (no/parallel/perpendicular)
    void lockAdditionalConstraint( AdditionalConstraint constraint );

    /**
     * Returns the first snapped segment. Will try to snap a segment using all layers
     * \param originalMapPoint point to be snapped (in map coordinates)
     * \param snapped if given, determines if a segment has been snapped
     */
    QList<QgsPointXY> snapSegmentToAllLayers( const QgsPointXY &originalMapPoint, bool *snapped = nullptr ) const;

    //! update the current point in the CAD point list
    void updateCurrentPoint( const QgsPointXY &point );
    //! remove previous point in the CAD point list
    void removePreviousPoint();

    /**
     * filters key press
     * \note called by eventFilter (filter on line edits), canvasKeyPressEvent (filter on map tool) and keyPressEvent (filter on dock)
     */
    bool filterKeyPress( QKeyEvent *e );

    /**
     * event filter for line edits in the dock UI (angle/distance/x/y line edits)
     * \note defined as private in Python bindings
     */
    bool eventFilter( QObject *obj, QEvent *event ) override SIP_SKIP;

    //! trigger fake mouse move event to update map tool rubber band and/or show new constraints
    void triggerMouseMoveEvent();

    //! Returns the constraint associated with an object
    CadConstraint *objectToConstraint( const QObject *obj ) const;

    //! Attempts to convert a user input value to double, either directly or via expression
    double parseUserInput( const QString &inputValue, bool &ok ) const;

    /**
     * Updates a constraint value based on a text input.
     * \param constraint constraint to update
     * \param textValue user entered text value, may be an expression
     * \param convertExpression set to true to update widget contents to calculated expression value
     */
    void updateConstraintValue( CadConstraint *constraint, const QString &textValue, bool convertExpression = false );

    //! Updates values of constraints that are not locked based on the current point
    void updateUnlockedConstraintValues( const QgsPointXY &point );

    QgsMapCanvas *mMapCanvas = nullptr;
    QgsAdvancedDigitizingCanvasItem *mCadPaintItem = nullptr;

    CadCapacities mCapacities = nullptr;

    bool mCurrentMapToolSupportsCad = false;

    // CAD properties
    //! is CAD currently enabled for current map tool
    bool mCadEnabled = false;
    bool mConstructionMode = false;

    // constraints
    std::unique_ptr< CadConstraint > mAngleConstraint;
    std::unique_ptr< CadConstraint > mDistanceConstraint;
    std::unique_ptr< CadConstraint > mXConstraint;
    std::unique_ptr< CadConstraint > mYConstraint;
    AdditionalConstraint mAdditionalConstraint;
    double mCommonAngleConstraint; // if 0: do not snap to common angles

    // point list and current snap point / segment
    QList<QgsPointXY> mCadPointList;
    QList<QgsPointXY> mSnappedSegment;
    bool mSnappedToVertex = false;

    bool mSessionActive = false;

    // error message
    std::unique_ptr<QgsMessageBarItem> mErrorMessage;

    // UI
    QAction *mEnableAction = nullptr;
    QMap< QAction *, double > mCommonAngleActions; // map the common angle actions with their angle values

    // Snap indicator

    QgsPointLocator::Match mSnapMatch;
  private:
#ifdef SIP_RUN
    //! event filter for line edits in the dock UI (angle/distance/x/y line edits)
    bool eventFilter( QObject *obj, QEvent *event );
#endif
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsAdvancedDigitizingDockWidget::CadCapacities )

#endif // QGSADVANCEDDIGITIZINGDOCK_H
