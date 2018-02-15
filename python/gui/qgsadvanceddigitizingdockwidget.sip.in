/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsadvanceddigitizingdockwidget.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsAdvancedDigitizingDockWidget : QgsDockWidget
{
%Docstring
 The QgsAdvancedDigitizingDockWidget class is a dockable widget
used to handle the CAD tools on top of a selection of map tools.
It handles both the UI and the constraints. Constraints are applied
by implementing filters called from QgsMapToolAdvancedDigitizing.
%End

%TypeHeaderCode
#include "qgsadvanceddigitizingdockwidget.h"
%End
  public:

    enum CadCapacity
    {
      AbsoluteAngle,
      RelativeAngle,
      RelativeCoordinates,
    };
    typedef QFlags<QgsAdvancedDigitizingDockWidget::CadCapacity> CadCapacities;


    enum AdditionalConstraint
    {
      NoConstraint,
      Perpendicular,
      Parallel
    };

    class CadConstraint
{
%Docstring
 The CadConstraint is an abstract class for all basic constraints (angle/distance/x/y).
It contains all values (locked, value, relative) and pointers to corresponding widgets.

.. note::

   Relative is not mandatory since it is not used for distance.
%End

%TypeHeaderCode
#include "qgsadvanceddigitizingdockwidget.h"
%End
      public:

        enum LockMode
        {
          NoLock,
          SoftLock,
          HardLock
        };

        CadConstraint( QLineEdit *lineEdit, QToolButton *lockerButton, QToolButton *relativeButton = 0, QToolButton *repeatingLockButton = 0 );
%Docstring
Constructor for CadConstraint.

:param lineEdit: associated line edit for constraint value
:param lockerButton: associated button for locking constraint
:param relativeButton: optional button for toggling relative constraint mode
:param repeatingLockButton: optional button for toggling repeating lock mode
%End

        LockMode lockMode() const;
%Docstring
The current lock mode of this constraint

:return: Lock mode
%End

        bool isLocked() const;
%Docstring
Is any kind of lock mode enabled
%End

        bool isRepeatingLock() const;
%Docstring
Returns true if a repeating lock is set for the constraint. Repeating locks are not
automatically cleared after a new point is added.

.. versionadded:: 2.16

.. seealso:: :py:func:`setRepeatingLock`
%End

        bool relative() const;
%Docstring
Is the constraint in relative mode
%End

        double value() const;
%Docstring
The value of the constraint
%End

        QLineEdit *lineEdit() const;
%Docstring
The line edit that manages the value of the constraint
%End

        void setLockMode( LockMode mode );
%Docstring
Set the lock mode
%End

        void setRepeatingLock( bool repeating );
%Docstring
Sets whether a repeating lock is set for the constraint. Repeating locks are not
automatically cleared after a new point is added.

:param repeating: set to true to set the lock to repeat automatically

.. versionadded:: 2.16

.. seealso:: :py:func:`isRepeatingLock`
%End

        void setRelative( bool relative );
%Docstring
Set if the constraint should be treated relative
%End

        void setValue( double value, bool updateWidget = true );
%Docstring
Set the value of the constraint

:param value: new value for constraint
:param updateWidget: set to false to prevent automatically updating the associated widget's value
%End

        void toggleLocked();
%Docstring
Toggle lock mode
%End

        void toggleRelative();
%Docstring
Toggle relative mode
%End

    };

    explicit QgsAdvancedDigitizingDockWidget( QgsMapCanvas *canvas, QWidget *parent = 0 );
%Docstring
Create an advanced digitizing dock widget

:param canvas: The map canvas on which the widget operates
:param parent: The parent
%End

    virtual void hideEvent( QHideEvent * );

%Docstring
Disables the CAD tools when hiding the dock
%End

    bool canvasKeyPressEventFilter( QKeyEvent *e );
%Docstring
Filter key events to e.g. toggle construction mode or adapt constraints

:param e: A mouse event (may be modified)

:return:  If the event is hidden (construction mode hides events from the maptool)
%End

    bool applyConstraints( QgsMapMouseEvent *e );
%Docstring
apply the CAD constraints. The will modify the position of the map event in map coordinates by applying the CAD constraints.

:return: false if no solution was found (invalid constraints)
%End

    bool alignToSegment( QgsMapMouseEvent *e, QgsAdvancedDigitizingDockWidget::CadConstraint::LockMode lockMode = QgsAdvancedDigitizingDockWidget::CadConstraint::HardLock );
%Docstring
align to segment for additional constraint.
If additional constraints are used, this will determine the angle to be locked depending on the snapped segment.

.. versionadded:: 3.0
%End

    void releaseLocks( bool releaseRepeatingLocks = true );
%Docstring
unlock all constraints

:param releaseRepeatingLocks: set to false to preserve the lock for any constraints set to repeating lock mode

.. versionadded:: 3.0
%End

    void clear();
%Docstring
Clear any cached previous clicks and helper lines
%End

    virtual void keyPressEvent( QKeyEvent *e );


    bool cadEnabled() const;
%Docstring
determines if CAD tools are enabled or if map tools behaves "nomally"
%End

    bool constructionMode() const;
%Docstring
construction mode is used to draw intermediate points. These points won't be given any further (i.e. to the map tools)
%End

    AdditionalConstraint additionalConstraint() const;
%Docstring
Additional constraints are used to place perpendicular/parallel segments to snapped segments on the canvas
%End
    const CadConstraint *constraintAngle() const;
%Docstring
Constraint on the angle
%End
    const CadConstraint *constraintDistance() const;
%Docstring
Constraint on the distance
%End
    const CadConstraint *constraintX() const;
%Docstring
Constraint on the X coordinate
%End
    const CadConstraint *constraintY() const;
%Docstring
Constraint on the Y coordinate
%End
    bool commonAngleConstraint() const;
%Docstring
Constraint on a common angle
%End

    void clearPoints();
%Docstring
Removes all points from the CAD point list

.. versionadded:: 3.0
%End

    void addPoint( const QgsPointXY &point );
%Docstring
Adds point to the CAD point list

.. versionadded:: 3.0
%End

    void setPoints( const QList<QgsPointXY> &points );
%Docstring
Configures list of current CAD points

Some map tools may find it useful to override list of CAD points that is otherwise
automatically populated when user clicks with left mouse button on map canvas.

.. versionadded:: 3.0
%End

    QgsPointXY currentPoint( bool *exists  = 0 ) const;
%Docstring
The last point.
Helper for the CAD point list. The CAD point list is the list of points
currently digitized. It contains both  "normal" points and intermediate points (construction mode).
%End

    QgsPointXY previousPoint( bool *exists = 0 ) const;
%Docstring
The previous point.
Helper for the CAD point list. The CAD point list is the list of points
currently digitized. It contains both  "normal" points and intermediate points (construction mode).
%End

    QgsPointXY penultimatePoint( bool *exists = 0 ) const;
%Docstring
The penultimate point.
Helper for the CAD point list. The CAD point list is the list of points
currently digitized. It contains both  "normal" points and intermediate points (construction mode).
%End

    int pointsCount() const;
%Docstring
The number of points in the CAD point helper list
%End

    bool snappedToVertex() const;
%Docstring
Is it snapped to a vertex
%End

    QList<QgsPointXY> snappedSegment() const;
%Docstring
Snapped to a segment
%End

    QAction *enableAction();
%Docstring
return the action used to enable/disable the tools
%End

    void enable();
%Docstring
Enables the tool (call this when an appropriate map tool is set and in the condition to make use of
cad digitizing)
Normally done automatically from QgsMapToolAdvancedDigitizing.activate() but may need to be fine tuned
if the map tool depends on preconditions like a feature selection.
%End

    void disable();
%Docstring
Disable the widget. Normally done automatically from :py:func:`QgsMapToolAdvancedDigitizing.deactivate()`
%End

    void updateCadPaintItem();
%Docstring
Updates canvas item that displays constraints on the ma

.. versionadded:: 3.0
%End

  signals:

    void pushWarning( const QString &message );
%Docstring
Push a warning

:param message: An informative message
%End

    void popWarning();
%Docstring
Remove any previously emitted warnings (if any)
%End

    void pointChanged( const QgsPointXY &point );
%Docstring
Sometimes a constraint may change the current point out of a mouse event. This happens normally
when a constraint is toggled.

:param point: The last known digitizing point. Can be used to emulate a mouse event.
%End

  private:
    //! event filter for line edits in the dock UI (angle/distance/x/y line edits)
    bool eventFilter( QObject *obj, QEvent *event );
};

QFlags<QgsAdvancedDigitizingDockWidget::CadCapacity> operator|(QgsAdvancedDigitizingDockWidget::CadCapacity f1, QFlags<QgsAdvancedDigitizingDockWidget::CadCapacity> f2);


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsadvanceddigitizingdockwidget.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
