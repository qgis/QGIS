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


/**
 * @brief The QgsAdvancedDigitizingDock class is a dockable widget
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

        CadConstraint( QLineEdit* lineEdit, QToolButton* lockerButton, QToolButton* relativeButton = 0 );

        LockMode lockMode() const;
        bool isLocked() const;
        bool relative() const;
        double value() const;

        QLineEdit* lineEdit() const;

        void setLockMode( LockMode mode );
        void setRelative( bool relative );
        void setValue( double value );

        void toggleLocked();
        void toggleRelative();
    };

    //! performs the intersection of a circle and a line
    //! @note from the two solutions, the intersection will be set to the closest point
    static bool lineCircleIntersection( const QgsPoint& center, const double radius, const QList<QgsPoint>& segment, QgsPoint& intersection );

    explicit QgsAdvancedDigitizingDockWidget( QgsMapCanvas* canvas, QWidget *parent = 0 );

    void hideEvent( QHideEvent* );

    bool canvasPressEvent( QgsMapMouseEvent* e );
    bool canvasReleaseEvent( QgsMapMouseEvent* e , bool captureSegment );
    bool canvasMoveEvent( QgsMapMouseEvent* e );
    bool canvasKeyPressEventFilter( QKeyEvent *e );

    //! apply the CAD constraints. The will modify the position of the map event in map coordinates by applying the CAD constraints.
    //! @return false if no solution was found (invalid constraints)
    virtual bool applyConstraints( QgsMapMouseEvent* e );

    void clear();

    QgsMapMouseEvent::SnappingMode snappingMode();

    //! key press event on the dock
    void keyPressEvent( QKeyEvent* e );

    //! determines if CAD tools are enabled or if map tools behaves "nomally"
    bool cadEnabled() const;

    //! construction mode is used to draw intermediate points. These points won't be given any further (i.e. to the map tools)
    bool constructionMode() const;

    //! Additional constraints are used to place perpendicular/parallel segments to snapped segments on the canvas
    AdditionalConstraint additionalConstraint() const;
    const CadConstraint* constraintAngle()const;
    const CadConstraint* constraintDistance() const;
    const CadConstraint* constraintX() const;
    const CadConstraint* constraintY() const;
    bool commonAngleConstraint() const;

    /** Helpers for the CAD point list. The CAD point list is the list of points
     * currently digitized. It contains both  "normal" points and intermediate points (construction mode).
     */
    QgsPoint currentPoint( bool* exists  = 0 ) const;
    QgsPoint previousPoint( bool* exists = 0 ) const;
    QgsPoint penultimatePoint( bool* exists = 0 ) const;
    int pointsCount() const;
    bool snappedToVertex() const;
    const QList<QgsPoint>& snappedSegment() const;

    //! return the action used to enable/disable the tools
    QAction* enableAction();

    void enable();

    void disable();

  signals:
    void pushWarning( const QString& message );

    void popWarning();

    void pointChanged( const QgsPoint& point );

  private slots:
    //! set the additiona constraint by clicking on the perpendicular/parallel buttons
    void addtionalConstraintClicked( bool activated );

    //! lock/unlock a constraint and set its value
    void lockConstraint( bool activate = true );

    //! unlock all constraints
    void releaseLocks();

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
