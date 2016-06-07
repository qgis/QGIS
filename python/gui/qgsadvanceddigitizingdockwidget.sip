/***************************************************************************
    qgsadvanceddigitizingdockwidget.sip  -  dock for CAD tools
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


/**
 * @brief The QgsAdvancedDigitizingDockWidget class is a dockable widget
 * used to handle the CAD tools on top of a selection of map tools.
 * It handles both the UI and the constraints. Constraints are applied
 * by implemeting filters called from QgsMapToolAdvancedDigitizing.
 */
class QgsAdvancedDigitizingDockWidget : QDockWidget
{
%TypeHeaderCode
#include <qgsadvanceddigitizingdockwidget.h>
%End
  public:

    /**
     * The CadCapacity enum defines the possible constraints to be set
     * depending on the number of points in the CAD point list (the list of points
     * currently digitized)
     */
    enum CadCapacity
    {
      AbsoluteAngle, //!< Azimuth
      RelativeAngle, //!< also for parallel and perpendicular
      RelativeCoordinates, //!< this corresponds to distance and relative coordinates
    };

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
     * @brief The CadConstraint is an abstract class for all basic constraints (angle/distance/x/y).
     * It contains all values (locked, value, relative) and pointers to corresponding widgets.
     * @note Relative is not mandatory since it is not used for distance.
     */
    class CadConstraint
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

        /** Constructor for CadConstraint.
         * @param lineEdit associated line edit for constraint value
         * @param lockerButton associated button for locking constraint
         * @param relativeButton optional button for toggling relative constraint mode
         * @param repeatingLockButton optional button for toggling repeating lock mode
         */
        CadConstraint( QLineEdit* lineEdit, QToolButton* lockerButton, QToolButton* relativeButton = nullptr, QToolButton* repeatingLockButton = nullptr );

        /**
         * The current lock mode of this constraint
         * @return Lock mode
         */
        LockMode lockMode() const;
        /**
         * Is any kind of lock mode enabled
         */
        bool isLocked() const;

        /** Returns true if a repeating lock is set for the constraint. Repeating locks are not
         * automatically cleared after a new point is added.
         * @note added in QGIS 2.16
         * @see setRepeatingLock()
         */
        bool isRepeatingLock() const;

        /**
         * Is the constraint in relative mode
         */
        bool relative() const;
        /**
         * The value of the constraint
         */
        double value() const;

        /**
         * The line edit that manages the value of the constraint
         */
        QLineEdit* lineEdit() const;

        /**
         * Set the lock mode
         */
        void setLockMode( LockMode mode );

        /** Sets whether a repeating lock is set for the constraint. Repeating locks are not
         * automatically cleared after a new point is added.
         * @param repeating set to true to set the lock to repeat automatically
         * @note added in QGIS 2.16
         * @see isRepeatingLock()
         */
        void setRepeatingLock( bool repeating );

        /**
         * Set if the constraint should be treated relative
         */
        void setRelative( bool relative );

        /**
         * Set the value of the constraint
         * @param value new value for constraint
         * @param updateWidget set to false to prevent automatically updating the associated widget's value
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
    };

    //! performs the intersection of a circle and a line
    //! @note from the two solutions, the intersection will be set to the closest point
    static bool lineCircleIntersection( const QgsPoint& center, const double radius, const QList<QgsPoint>& segment, QgsPoint& intersection );

    /**
     * Create an advanced digitizing dock widget
     * @param canvas The map canvas on which the widget operates
     * @param parent The parent
     */
    explicit QgsAdvancedDigitizingDockWidget( QgsMapCanvas* canvas, QWidget *parent = 0 );

    /**
     * Disables the CAD tools when hiding the dock
     */
    void hideEvent( QHideEvent* );

    /**
     * Will react on a canvas press event
     *
     * @param e A mouse event (may be modified)
     * @return  If the event is hidden (construction mode hides events from the maptool)
     */
    bool canvasPressEvent( QgsMapMouseEvent* e );
    /**
     * Will react on a canvas release event
     *
     * @param e A mouse event (may be modified)
     * @param captureSegment Capture segments?
     * @return  If the event is hidden (construction mode hides events from the maptool)
     */
    bool canvasReleaseEvent( QgsMapMouseEvent* e , bool captureSegment );
    /**
     * Will react on a canvas move event
     *
     * @param e A mouse event (may be modified)
     * @return  If the event is hidden (construction mode hides events from the maptool)
     */
    bool canvasMoveEvent( QgsMapMouseEvent* e );
    /**
     * Filter key events to e.g. toggle construction mode or adapt constraints
     *
     * @param e A mouse event (may be modified)
     * @return  If the event is hidden (construction mode hides events from the maptool)
     */
    bool canvasKeyPressEventFilter( QKeyEvent *e );

    //! apply the CAD constraints. The will modify the position of the map event in map coordinates by applying the CAD constraints.
    //! @return false if no solution was found (invalid constraints)
    virtual bool applyConstraints( QgsMapMouseEvent* e );

    /**
     * Clear any cached previous clicks and helper lines
     */
    void clear();

    /**
     * The snapping mode
     * @return Snapping mode
     */
    QgsMapMouseEvent::SnappingMode snappingMode();

    //! key press event on the dock
    void keyPressEvent( QKeyEvent* e );

    //! determines if CAD tools are enabled or if map tools behaves "nomally"
    bool cadEnabled() const;

    //! construction mode is used to draw intermediate points. These points won't be given any further (i.e. to the map tools)
    bool constructionMode() const;

    //! Additional constraints are used to place perpendicular/parallel segments to snapped segments on the canvas
    AdditionalConstraint additionalConstraint() const;
    //! Constraint on the angle
    const CadConstraint* constraintAngle()const;
    //! Constraint on the distance
    const CadConstraint* constraintDistance() const;
    //! Constraint on the X coordinate
    const CadConstraint* constraintX() const;
    //! Constraint on the Y coordinate
    const CadConstraint* constraintY() const;
    //! Constraint on a common angle
    bool commonAngleConstraint() const;

    /**
     * The last point.
     * Helper for the CAD point list. The CAD point list is the list of points
     * currently digitized. It contains both  "normal" points and intermediate points (construction mode).
     */
    QgsPoint currentPoint( bool* exists  = 0 ) const;

    /**
     * The previous point.
     * Helper for the CAD point list. The CAD point list is the list of points
     * currently digitized. It contains both  "normal" points and intermediate points (construction mode).
     */
    QgsPoint previousPoint( bool* exists = 0 ) const;

    /**
     * The penultimate point.
     * Helper for the CAD point list. The CAD point list is the list of points
     * currently digitized. It contains both  "normal" points and intermediate points (construction mode).
     */
    QgsPoint penultimatePoint( bool* exists = 0 ) const;

    /**
     * The number of points in the CAD point helper list
     */
    int pointsCount() const;

    /**
     * Is it snapped to a vertex
     */
    bool snappedToVertex() const;

    /**
     * Snapped to a segment
     */
    const QList<QgsPoint>& snappedSegment() const;

    //! return the action used to enable/disable the tools
    QAction* enableAction();

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

  signals:
    /**
     * Push a warning
     *
     * @param message An informative message
     */
    void pushWarning( const QString& message );

    /**
     * Remove any previously emitted warnings (if any)
     */
    void popWarning();

    /**
     * Sometimes a constraint may change the current point out of a mouse event. This happens normally
     * when a constraint is toggled.
     *
     * @param point The last known digitizing point. Can be used to emulate a mouse event.
     */
    void pointChanged( const QgsPoint& point );

  private slots:
    //! set the additiona constraint by clicking on the perpendicular/parallel buttons
    void addtionalConstraintClicked( bool activated );

    //! lock/unlock a constraint and set its value
    void lockConstraint( bool activate = true );

    //! unlock all constraints
    //! @param releaseRepeatingLocks set to false to preserve the lock for any constraints set to repeating lock mode
    void releaseLocks( bool releaseRepeatingLocks = true );

    //! set the relative properties of constraints
    void setConstraintRelative( bool activate );

    //! activate/deactivate tools. It is called when tools are activated manually (from the GUI)
    //! it will call setCadEnabled to properly update the UI.
    void activateCad( bool enabled );

    //! enable/disable construction mode (events are not forwarded to the map tool)
    void setConstructionMode( bool enabled );

    //! settings button triggered
    void settingsButtonTriggered( QAction* action );

  private:
    bool eventFilter( QObject *obj, QEvent *event );
};
