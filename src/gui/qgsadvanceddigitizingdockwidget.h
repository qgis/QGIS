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
#include <QQueue>

#include <memory>

#include "ui_qgsadvanceddigitizingdockwidgetbase.h"
#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsadvanceddigitizingtools.h"
#include "qgsdockwidget.h"
#include "qgsmessagebaritem.h"
#include "qgspointxy.h"
#include "qgspointlocator.h"
#include "qgssnapindicator.h"


class QgsAdvancedDigitizingCanvasItem;
class QgsAdvancedDigitizingFloater;
class QgsAdvancedDigitizingTool;
class QgsMapCanvas;
class QgsMapTool;
class QgsMapToolAdvancedDigitizing;
class QgsMapMouseEvent;
class QgsUserInputWidget;

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
    enum CadCapacity SIP_ENUM_BASETYPE( IntFlag )
    {
      AbsoluteAngle = 1, //!< Azimuth
      RelativeAngle = 2, //!< Also for parallel and perpendicular
      RelativeCoordinates = 4, //!< This corresponds to distance and relative coordinates
      Distance = 8, //!< Distance
    };
    Q_DECLARE_FLAGS( CadCapacities, CadCapacity )
    Q_FLAG( CadCapacities )

    /**
     * Type of interaction to simulate when editing values from external widget
     * \note unstable API (will likely change)
     * \since QGIS 3.8
     */
    enum WidgetSetMode
    {
      ReturnPressed, FocusOut, TextEdited
    };


    /**
     * \ingroup gui
     * \brief The CadConstraint is a class for all basic constraints (angle/distance/x/y).
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
         * Returns TRUE if a repeating lock is set for the constraint. Repeating locks are not
         * automatically cleared after a new point is added.
         * \see setRepeatingLock()
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
         * \param repeating set to TRUE to set the lock to repeat automatically
         * \see isRepeatingLock()
         */
        void setRepeatingLock( bool repeating );

        /**
         * Set if the constraint should be treated relative
         */
        void setRelative( bool relative );

        /**
         * Set the value of the constraint
         * \param value new value for constraint
         * \param updateWidget set to FALSE to prevent automatically updating the associated widget's value
         */
        void setValue( double value, bool updateWidget = true );

        /**
         * Returns a localized formatted string representation of the value.
         * \since QGIS 3.32
         */
        QString displayValue() const;

        /**
         * Toggle lock mode
         */
        void toggleLocked();

        /**
         * Toggle relative mode
         */
        void toggleRelative();

        /**
         * Returns the numeric precision (decimal places) to show in the associated widget.
         *
         * \see setPrecision()
         * \since QGIS 3.22
         */
        int precision() const { return mPrecision; }

        /**
         * Sets the numeric precision (decimal places) to show in the associated widget.
         *
         * \see precision()
         * \since QGIS 3.22
         */
        void setPrecision( int precision );

        /**
         * Returns the constraint type
         * \since QGIS 3.32
         */
        Qgis::CadConstraintType cadConstraintType() const;

        /**
         * Sets the constraint type to \a constraintType
         * \since QGIS 3.32
         */
        void setCadConstraintType( Qgis::CadConstraintType constraintType );

        /**
         * Sets the map canvas to \a mapCanvas
         * \since QGIS 3.32
         */
        void setMapCanvas( QgsMapCanvas *mapCanvas );

        /**
         * Removes unit suffix from the constraint text.
         * \since QGIS 3.34
         */
        static QString removeSuffix( const QString &text, Qgis::CadConstraintType constraintType );


      private:
        QLineEdit *mLineEdit = nullptr;
        QToolButton *mLockerButton = nullptr;
        QToolButton *mRelativeButton = nullptr;
        QToolButton *mRepeatingLockButton = nullptr;
        LockMode mLockMode;
        bool mRepeatingLock;
        bool mRelative;
        double mValue;
        int mPrecision = 6;
        Qgis::CadConstraintType mCadConstraintType = Qgis::CadConstraintType::Generic;
        QgsMapCanvas *mMapCanvas = nullptr;
    };

    /**
     * Create an advanced digitizing dock widget
     * \param canvas The map canvas on which the widget operates
     * \param parent The parent
     * \param userInputWidget The user input widget on which tools can add widget overlays on top of the map canvas (since QGIS 3.40)
     */
    explicit QgsAdvancedDigitizingDockWidget( QgsMapCanvas *canvas, QWidget *parent = nullptr, QgsUserInputWidget *userInputWidget = nullptr );
    ~QgsAdvancedDigitizingDockWidget();

    /**
     * Filter key events to e.g. toggle construction mode or adapt constraints
     *
     * \param e A mouse event (may be modified)
     * \returns  If the event is hidden (construction mode hides events from the maptool)
     */
    bool canvasKeyPressEventFilter( QKeyEvent *e );

    /**
     * apply the CAD constraints. The will modify the position of the map event in map coordinates by applying the CAD constraints.
     * \returns FALSE if no solution was found (invalid constraints)
     */
    bool applyConstraints( QgsMapMouseEvent *e );

    /**
     * align to segment for between line constraint.
     * If between line constraints are used, this will determine the angle to be locked depending on the snapped segment.
     */
    bool alignToSegment( QgsMapMouseEvent *e, QgsAdvancedDigitizingDockWidget::CadConstraint::LockMode lockMode = QgsAdvancedDigitizingDockWidget::CadConstraint::HardLock );

    /**
     * Processes the canvas press \a event.
     */
    void processCanvasPressEvent( QgsMapMouseEvent *event );

    /**
     * Processes the canvas move \a event.
     */
    void processCanvasMoveEvent( QgsMapMouseEvent *event );

    /**
     * Processes the canvas release \a event.
     */
    void processCanvasReleaseEvent( QgsMapMouseEvent *event );

    /**
     * Sets an advanced digitizing tool which will take over digitizing until the tool is close.
     * \since QGIS 3.40
     */
    void setTool( QgsAdvancedDigitizingTool *tool );

    /**
     * Returns the current advanced digitizing tool. Returns NULLPTR if not set.
     * \since QGIS 3.40
     */
    QgsAdvancedDigitizingTool *tool() const;

    /**
     * unlock all constraints
     * \param releaseRepeatingLocks set to FALSE to preserve the lock for any constraints set to repeating lock mode
     */
    void releaseLocks( bool releaseRepeatingLocks = true );

    /**
     * Clear any cached previous clicks and helper lines
     */
    void clear();

    void keyPressEvent( QKeyEvent *e ) override;

    //! determines if CAD tools are enabled or if map tools behaves "nomally"
    bool cadEnabled() const { return mCadEnabled; }

    /**
     * Determines if Z or M will be enabled.
     * \since QGIS 3.22
     */
    void switchZM( );

    /**
     * Sets whether Z is enabled
     * \since QGIS 3.22
     */
    void setEnabledZ( bool enable );

    /**
     * Sets whether M is enabled
     * \since QGIS 3.22
     */
    void setEnabledM( bool enable );

    /**
     * Returns whether the construction mode is activated. The construction mode is used to draw intermediate
     * points that will not be part of a geometry being digitized.
     */
    bool constructionMode() const { return mConstructionMode; }

    /**
     * Returns the vector layer within which construction guides are stored.
     * \since QGIS 3.40
     */
    QgsVectorLayer *constructionGuidesLayer() const { return mConstructionGuidesLayer.get(); }

    /**
     * Returns whether the construction guides are visible.
     * \since QGIS 3.40
     */
    bool showConstructionGuides() const;

    /**
     * Returns whether points should snap to construction guides.
     * \since QGIS 3.40
     */
    bool snapToConstructionGuides() const;

    /**
     * Returns whether construction guides are being recorded.
     * \since QGIS 3.40
     */
    bool recordConstructionGuides() const;

    /**
     * Returns the between line constraints which are used to place
     * perpendicular/parallel segments to snapped segments on the canvas
     */
    Qgis::BetweenLineConstraint betweenLineConstraint() const  { return mBetweenLineConstraint; }
    //! Returns the \a CadConstraint on the angle
    const CadConstraint *constraintAngle() const  { return mAngleConstraint.get(); }
    //! Returns the \a CadConstraint on the distance
    const CadConstraint *constraintDistance() const { return mDistanceConstraint.get(); }
    //! Returns the \a CadConstraint on the X coordinate
    const CadConstraint *constraintX() const { return mXConstraint.get(); }
    //! Returns the \a CadConstraint on the Y coordinate
    const CadConstraint *constraintY() const { return mYConstraint.get(); }

    /**
     * Returns the \a CadConstraint on the Z coordinate
     * \since QGIS 3.22
     */
    const CadConstraint *constraintZ() const { return mZConstraint.get(); }

    /**
     * Returns the \a CadConstraint on the M coordinate
     * \since QGIS 3.22
     */
    const CadConstraint *constraintM() const { return mMConstraint.get(); }
    //! Returns TRUE if a constraint on a common angle is active
    bool commonAngleConstraint() const { return !qgsDoubleNear( mCommonAngleConstraint, 0.0 ); }

    //! Returns the \a CadConstraint
    const CadConstraint *constraintLineExtension() const { return mLineExtensionConstraint.get(); }

    //! Returns on which side of the constraint line extension point, the line was created
    Qgis::LineExtensionSide lineExtensionSide() const { return mSoftLockLineExtension; }

    //! Returns the \a CadConstraint
    const CadConstraint *constraintXyVertex() const { return mXyVertexConstraint.get(); }

    //! Returns the X value of the X soft lock. The value is NaN is the constraint isn't magnetized to a line
    double softLockX() const { return mSoftLockX; }

    //! Returns the Y value of the Y soft lock. The value is NaN is the constraint isn't magnetized to a line
    double softLockY() const { return mSoftLockY; }

    /**
     * Toggles the distance constraint.
     * \since QGIS 3.40
     */
    void toggleConstraintDistance();

    /**
     * Returns the point locator match
     * \since QGIS 3.4
     */
    QgsPointLocator::Match mapPointMatch() const { return mSnapMatch; }

    /**
     * Returns the snap matches whose vertices have been locked
     * \since QGIS 3.26
     */
    QList< QgsPointLocator::Match > lockedSnapVertices() const { return mLockedSnapVertices; }

    /**
      * Removes all points from the locked snap vertex list
      * \param force Clears the list even if the constraints that use it are still locked.
      * \since QGIS 3.26
      */
    void clearLockedSnapVertices( bool force = true );

    /**
     * Removes all points from the CAD point list
     */
    void clearPoints();

    /**
     * Adds point to the CAD point list
     */
    void addPoint( const QgsPointXY &point );

    /**
     * Removes previous point in the CAD point list
     * \since QGIS 3.8
     */
    void removePreviousPoint();

    /**
     * Updates the current \a point in the CAD point list
     * \since QGIS 3.30.2
     */
    void updateCurrentPoint( const QgsPoint &point );

    /**
     * Configures list of current CAD points
     *
     * Some map tools may find it useful to override list of CAD points that is otherwise
     * automatically populated when user clicks with left mouse button on map canvas.
     */
    void setPoints( const QList<QgsPointXY> &points );

    /**
     * The last point.
     * Helper for the CAD point list. The CAD point list is the list of points
     * currently digitized. It contains both  "normal" points and intermediate points (construction mode).
     *
     * \since QGIS 3.22
     */
    QgsPoint currentPointV2( bool *exists  = nullptr ) const;

    /**
     * Returns the last CAD point, in a map \a layer's coordinates.
     *
     * \since QGIS 3.22
     */
    QgsPoint currentPointLayerCoordinates( QgsMapLayer *layer ) const;

    /**
     * The last point.
     * Helper for the CAD point list. The CAD point list is the list of points
     * currently digitized. It contains both  "normal" points and intermediate points (construction mode).
     * \deprecated QGIS 3.22. Use currentPointV2() instead.
     */
    Q_DECL_DEPRECATED QgsPointXY currentPoint( bool *exists  = nullptr ) const SIP_DEPRECATED { return currentPointV2( exists ); };

    /**
     * The previous point.
     * Helper for the CAD point list. The CAD point list is the list of points
     * currently digitized. It contains both  "normal" points and intermediate points (construction mode).
     */
    QgsPoint previousPointV2( bool *exists = nullptr ) const;

    /**
     * The previous point.
     * Helper for the CAD point list. The CAD point list is the list of points
     * currently digitized. It contains both  "normal" points and intermediate points (construction mode).
     * \deprecated QGIS 3.22. Use previousPointV2() instead.
     */
    Q_DECL_DEPRECATED QgsPointXY previousPoint( bool *exists = nullptr ) const SIP_DEPRECATED { return previousPointV2( exists ); };

    /**
     * The penultimate point.
     * Helper for the CAD point list. The CAD point list is the list of points
     * currently digitized. It contains both  "normal" points and intermediate points (construction mode).
     */
    QgsPoint penultimatePointV2( bool *exists = nullptr ) const;

    /**
     * The penultimate point.
     * Helper for the CAD point list. The CAD point list is the list of points
     * currently digitized. It contains both  "normal" points and intermediate points (construction mode).
     * \deprecated QGIS 3.22. Use penultimatePointV2() instead.
     */
    Q_DECL_DEPRECATED QgsPointXY penultimatePoint( bool *exists = nullptr ) const SIP_DEPRECATED { return penultimatePointV2( exists ); };

    /**
     * The number of points in the CAD point helper list
     */
    inline int pointsCount() const { return mCadPointList.count(); }

    /**
     * Is it snapped to a vertex
     */
    inline bool snappedToVertex() const { return ( mSnapMatch.isValid() && ( mSnapMatch.hasVertex() || mSnapMatch.hasLineEndpoint() ) ); }

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
     */
    void updateCadPaintItem();

    /**
    * Set the X value on the widget.
    * Can be used to set constraints by external widgets.
    * \param mode What type of interaction to emulate
    * \param value The value (as a QString, as it could be an expression)
    * \note unstable API (will likely change)
    * \since QGIS 3.8
    */
    void setX( const QString &value, WidgetSetMode mode );

    /**
    * Set the Y value on the widget.
    * Can be used to set constraints by external widgets.
    * \param mode What type of interaction to emulate
    * \param value The value (as a QString, as it could be an expression)
    * \note unstable API (will likely change)
    * \since QGIS 3.8
    */
    void setY( const QString &value, WidgetSetMode mode );

    /**
    * Set the Z value on the widget.
    * Can be used to set constraints by external widgets.
    * \param mode What type of interaction to emulate
    * \param value The value (as a QString, as it could be an expression)
    * \note unstable API (will likely change)
    * \since QGIS 3.22
    */
    void setZ( const QString &value, WidgetSetMode mode );

    /**
    * Set the M value on the widget.
    * Can be used to set constraints by external widgets.
    * \param mode What type of interaction to emulate
    * \param value The value (as a QString, as it could be an expression)
    * \note unstable API (will likely change)
    * \since QGIS 3.22
    */
    void setM( const QString &value, WidgetSetMode mode );

    /**
    * Set the angle value on the widget.
    * Can be used to set constraints by external widgets.
    * \param mode What type of interaction to emulate
    * \param value The value (as a QString, as it could be an expression)
    * \note unstable API (will likely change)
    * \since QGIS 3.8
    */
    void setAngle( const QString &value, WidgetSetMode mode );

    /**
    * Set the distance value on the widget.
    * Can be used to set constraints by external widgets.
    * \param mode What type of interaction to emulate
    * \param value The value (as a QString, as it could be an expression)
    * \note unstable API (will likely change)
    * \since QGIS 3.8
    */
    void setDistance( const QString &value, WidgetSetMode mode );

    /**
     * Convenient method to get the Z value from the line edit wiget
     * \since QGIS 3.22
     */
    double getLineZ( ) const;

    /**
     * Convenient method to get the M value from the line edit wiget
     * \since QGIS 3.22
     */
    double getLineM( ) const;

    /**
     * Returns the capacities
     * \since QGIS 3.26
     */
    CadCapacities capacities() const { return mCapacities; };

    /**
     * Returns the formatted label for common angle snapping option.
     * \since QGIS 3.32
     */
    QString formatCommonAngleSnapping( double angle );

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
     * \since QGIS 3.22
     */
    void pointChangedV2( const QgsPoint &point );

    /**
     * Sometimes a constraint may change the current point out of a mouse event. This happens normally
     * when a constraint is toggled.
     *
     * \param point The last known digitizing point. Can be used to emulate a mouse event.
     * \deprecated QGIS 3.22. No longer used, will be removed in QGIS 4.0. Use pointChangedV2 instead.
     */
    Q_DECL_DEPRECATED void pointChanged( const QgsPointXY &point ) SIP_DEPRECATED;

    //! Signals for external widgets that need to update according to current values

    /**
    * Emitted whenever CAD is enabled or disabled
    *
    * \param enabled Whether CAD is enabled or not
    * \note unstable API (will likely change).
    * \since QGIS 3.8
    */
    void cadEnabledChanged( bool enabled );

    /**
    * Emitted whenever the X \a value changes (either the mouse moved, or the user changed the input).
    * Could be used by widgets that must reflect the current advanced digitizing state.
    * \note unstable API (will likely change)
    * \since QGIS 3.8
    */
    void valueXChanged( const QString &value );

    /**
    * Emitted whenever the Y \a value changes (either the mouse moved, or the user changed the input).
    * Could be used by widgets that must reflect the current advanced digitizing state.
    * \note unstable API (will likely change)
    * \since QGIS 3.8
    */
    void valueYChanged( const QString &value );

    /**
    * Emitted whenever the Z \a value changes (either the mouse moved, or the user changed the input).
    * Could be used by widgets that must reflect the current advanced digitizing state.
    * \note unstable API (will likely change)
    * \since QGIS 3.22
    */
    void valueZChanged( const QString &value );

    /**
    * Emitted whenever the M \a value changes (either the mouse moved, or the user changed the input).
    * Could be used by widgets that must reflect the current advanced digitizing state.
    * \note unstable API (will likely change)
    * \since QGIS 3.22
    */
    void valueMChanged( const QString &value );

    /**
    * Emitted whenever the angle \a value changes (either the mouse moved, or the user changed the input).
    * Could be used by widgets that must reflect the current advanced digitizing state.
    * \note unstable API (will likely change)
    * \since QGIS 3.8
    */
    void valueAngleChanged( const QString &value );

    /**
    * Emitted whenever the distance \a value changes (either the mouse moved, or the user changed the input).
    * Could be used by widgets that must reflect the current advanced digitizing state.
    * \note unstable API (will likely change)
    * \since QGIS 3.8
    */
    void valueDistanceChanged( const QString &value );

    /**
    * Emitted whenever the bearing \a value changes.
    * Could be used by widgets that must reflect the current advanced digitizing state.
    * \note unstable API (will likely change)
    * \since QGIS 3.32
    */
    void valueBearingChanged( const QString &value );

    /**
    * Emitted whenever the X parameter is \a locked.
    * Could be used by widgets that must reflect the current advanced digitizing state.
    * \note unstable API (will likely change)
    * \since QGIS 3.8
    */
    void lockXChanged( bool locked );

    /**
    * Emitted whenever the Y parameter is \a locked.
    * Could be used by widgets that must reflect the current advanced digitizing state.
    * \note unstable API (will likely change)
    * \since QGIS 3.8
    */
    void lockYChanged( bool locked );

    /**
    * Emitted whenever the Z parameter is \a locked.
    * Could be used by widgets that must reflect the current advanced digitizing state.
    * \note unstable API (will likely change)
    * \since QGIS 3.22
    */
    void lockZChanged( bool locked );

    /**
    * Emitted whenever the M parameter is \a locked.
    * Could be used by widgets that must reflect the current advanced digitizing state.
    * \note unstable API (will likely change)
    * \since QGIS 3.22
    */
    void lockMChanged( bool locked );

    /**
    * Emitted whenever the angle parameter is \a locked.
    * Could be used by widgets that must reflect the current advanced digitizing state.
    * \note unstable API (will likely change)
    * \since QGIS 3.8
    */
    void lockAngleChanged( bool locked );

    /**
    * Emitted whenever the distance parameter is \a locked.
    * Could be used by widgets that must reflect the current advanced digitizing state.
    * \note unstable API (will likely change)
    * \since QGIS 3.8
    */
    void lockDistanceChanged( bool locked );

    /**
    * Emitted whenever the X parameter is toggled between absolute and relative.
    * Could be used by widgets that must reflect the current advanced digitizing state.
    *
    * \param relative Whether the X parameter is relative or not.
    * \note unstable API (will likely change)
    * \since QGIS 3.8
    */
    void relativeXChanged( bool relative );

    /**
    * Emitted whenever the Y parameter is toggled between absolute and relative.
    * Could be used by widgets that must reflect the current advanced digitizing state.
    *
    * \param relative Whether the Y parameter is relative or not.
    * \note unstable API (will likely change)
    * \since QGIS 3.8
    */
    void relativeYChanged( bool relative );

    /**
    * Emitted whenever the Z parameter is toggled between absolute and relative.
    * Could be used by widgets that must reflect the current advanced digitizing state.
    *
    * \param relative Whether the Z parameter is relative or not.
    * \note unstable API (will likely change)
    * \since QGIS 3.22
    */
    void relativeZChanged( bool relative );

    /**
    * Emitted whenever the M parameter is toggled between absolute and relative.
    * Could be used by widgets that must reflect the current advanced digitizing state.
    *
    * \param relative Whether the M parameter is relative or not.
    * \note unstable API (will likely change)
    * \since QGIS 3.22
    */
    void relativeMChanged( bool relative );

    /**
    * Emitted whenever the angleX parameter is toggled between absolute and relative.
    * Could be used by widgets that must reflect the current advanced digitizing state.
    *
    * \param relative Whether the angle parameter is relative or not.
    * \note unstable API (will likely change)
    * \since QGIS 3.8
    */
    void relativeAngleChanged( bool relative );

    /**
    * Emitted whenever the soft line extension parameter is \a locked.
    * Could be used by widgets that must reflect the current advanced digitizing state.
    * \note unstable API (will likely change)
    * \since QGIS 3.26
    */
    void softLockLineExtensionChanged( bool locked );

    /**
    * Emitted whenever the soft x/y extension parameter is \a locked.
    * Could be used by widgets that must reflect the current advanced digitizing state.
    * \note unstable API (will likely change)
    * \since QGIS 3.26
    */
    void softLockXyChanged( bool locked );

    // relativeDistanceChanged doesn't exist as distance is always relative

    /**
    * Emitted whenever the X field is enabled or disabled. Depending on the context, some parameters
    * do not make sense (e.g. you need a previous point to define a distance).
    * Could be used by widgets that must reflect the current advanced digitizing state.
    *
    * \param enabled Whether the X parameter is enabled or not.
    * \note unstable API (will likely change)
    * \since QGIS 3.8
    */
    void enabledChangedX( bool enabled );

    /**
    * Emitted whenever the Y field is enabled or disabled. Depending on the context, some parameters
    * do not make sense (e.g. you need a previous point to define a distance).
    * Could be used by widgets that must reflect the current advanced digitizing state.
    *
    * \param enabled Whether the Y parameter is enabled or not.
    * \note unstable API (will likely change)
    * \since QGIS 3.8
    */
    void enabledChangedY( bool enabled );

    /**
    * Emitted whenever the Z field is enabled or disabled. Depending on the context, some parameters
    * do not make sense (e.g. you need a previous point to define a distance).
    * Could be used by widgets that must reflect the current advanced digitizing state.
    *
    * \param enabled Whether the Z parameter is enabled or not.
    * \note unstable API (will likely change)
    * \since QGIS 3.22
    */
    void enabledChangedZ( bool enabled );

    /**
    * Emitted whenever the M field is enabled or disabled. Depending on the context, some parameters
    * do not make sense (e.g. you need a previous point to define a distance).
    * Could be used by widgets that must reflect the current advanced digitizing state.
    *
    * \param enabled Whether the M parameter is enabled or not.
    * \note unstable API (will likely change)
    * \since QGIS 3.22
    */
    void enabledChangedM( bool enabled );

    /**
    * Emitted whenever the angle field is enabled or disabled. Depending on the context, some parameters
    * do not make sense (e.g. you need a previous point to define a distance).
    * Could be used by widgets that must reflect the current advanced digitizing state.
    *
    * \param enabled Whether the angle parameter is enabled or not.
    * \note unstable API (will likely change)
    * \since QGIS 3.8
    */
    void enabledChangedAngle( bool enabled );

    /**
    * Emitted whenever the distance field is enabled or disabled. Depending on the context, some parameters
    * do not make sense (e.g. you need a previous point to define a distance).
    * Could be used by widgets that must reflect the current advanced digitizing state.
    *
    * \param enabled Whether the distance parameter is enabled or not.
    * \note unstable API (will likely change)
    * \since QGIS 3.8
    */
    void enabledChangedDistance( bool enabled );

    /**
    * Emitted whenever the X field should get the focus using the shortcuts (X).
    * Could be used by widgets to capture the focus when a field is being edited.
    * \note unstable API (will likely change)
    * \since QGIS 3.8
    */
    void focusOnXRequested();

    /**
    * Emitted whenever the Y field should get the focus using the shortcuts (Y).
    * Could be used by widgets to capture the focus when a field is being edited.
    * \note unstable API (will likely change)
    * \since QGIS 3.8
    */
    void focusOnYRequested();

    /**
    * Emitted whenever the Z field should get the focus using the shortcuts (Z).
    * Could be used by widgets to capture the focus when a field is being edited.
    * \note unstable API (will likely change)
    * \since QGIS 3.22
    */
    void focusOnZRequested();

    /**
    * Emitted whenever the M field should get the focus using the shortcuts (M).
    * Could be used by widgets to capture the focus when a field is being edited.
    * \note unstable API (will likely change)
    * \since QGIS 3.22
    */
    void focusOnMRequested();

    /**
    * Emitted whenever the angle field should get the focus using the shortcuts (A).
    * Could be used by widgets to capture the focus when a field is being edited.
    * \note unstable API (will likely change)
    * \since QGIS 3.8
    */
    void focusOnAngleRequested();

    /**
    * Emitted whenever the distance field should get the focus using the shortcuts (D).
    * Could be used by widgets to capture the focus when a field is being edited.
    * \note unstable API (will likely change)
    * \since QGIS 3.8
    */
    void focusOnDistanceRequested();

    /**
    * Emitted whenever the snapping to common angle option changes, angle = 0 means that the functionality is disabled.
    * \since QGIS 3.32
    */
    void valueCommonAngleSnappingChanged( double angle );

    /**
     * Emitted whenever the option to show common angle snapping in the floater changes.
     * \note Not available in Python bindings
     */
    void commonAngleSnappingShowInFloaterChanged( bool enabled ) SIP_SKIP;

  private slots:
    //! Sets the between line constraint by clicking on the perpendicular/parallel buttons
    void betweenLineConstraintClicked( bool activated );

    //! lock/unlock a constraint and set its value
    void lockConstraint( bool activate = true );

    //! lock/unlock a parameterless constraint
    void lockParameterlessConstraint( bool activate = true );

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

    /**
     * Returns the layer currently associated with the map tool using the dock widget.
     */
    QgsMapLayer *targetLayer() const;

    //! updates the UI depending on activation of the tools and clear points / release locks.
    void setCadEnabled( bool enabled );

    /**
     * \brief updateCapacity updates the cad capacities depending on the point list and update the UI according to the capabilities.
     * \param updateUIwithoutChange if TRUE, it will update the UI even if new capacities are not different from previous ones.
     */
    void updateCapacity( bool updateUIwithoutChange = false );

    //! defines the between line constraint to be used (no/parallel/perpendicular)
    void lockBetweenLineConstraint( Qgis::BetweenLineConstraint constraint );

    /**
     * Returns the first snapped segment. Will try to snap a segment using all layers
     * \param originalMapPoint point to be snapped (in map coordinates)
     * \param snapped if given, determines if a segment has been snapped
     */
    QList<QgsPointXY> snapSegmentToAllLayers( const QgsPointXY &originalMapPoint, bool *snapped = nullptr ) const;

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
    double parseUserInput( const QString &inputValue, const Qgis::CadConstraintType type, bool &ok ) const;

    /**
     * Updates a constraint value based on a text input.
     * \param constraint constraint to update
     * \param textValue user entered text value, may be an expression
     * \param convertExpression set to TRUE to update widget contents to calculated expression value
     */
    void updateConstraintValue( CadConstraint *constraint, const QString &textValue, bool convertExpression = false );

    //! Updates values of constraints that are not locked based on the current point
    void updateUnlockedConstraintValues( const QgsPoint &point );

    /**
     * Adds or removes the snap match if it is already in the locked snap queue or not.
     * \param snapMatch the snap match to add or remove.
     * \param previouslySnap the previous snap match to avoid toggling the same match.
     */
    void toggleLockedSnapVertex( const QgsPointLocator::Match &snapMatch, QgsPointLocator::Match previouslySnap );

    /**
     * Resets the vector layer and point locator objects handling construction guides.
     */
    void resetConstructionGuides();

    /**
     * Updates the construction guides layer CRS to match the map canvas' destination CRS
     * and reproject pre-existing construction guides.
     */
    void updateConstructionGuidesCrs();

    QgsMapCanvas *mMapCanvas = nullptr;
    QgsUserInputWidget *mUserInputWidget = nullptr;

    QgsAdvancedDigitizingCanvasItem *mCadPaintItem = nullptr;
    //! Snapping indicator
    std::unique_ptr<QgsSnapIndicator> mSnapIndicator;

    CadCapacities mCapacities = CadCapacities();

    bool mCurrentMapToolSupportsCad = false;

    // Pointer to the floater
    QgsAdvancedDigitizingFloater *mFloater = nullptr;

    // CAD properties
    //! is CAD currently enabled for current map tool
    bool mCadEnabled = false;
    bool mConstructionMode = false;

    // constraints
    std::unique_ptr< CadConstraint > mAngleConstraint;
    std::unique_ptr< CadConstraint > mDistanceConstraint;
    std::unique_ptr< CadConstraint > mXConstraint;
    std::unique_ptr< CadConstraint > mYConstraint;
    std::unique_ptr< CadConstraint > mZConstraint;
    std::unique_ptr< CadConstraint > mMConstraint;
    std::unique_ptr< CadConstraint > mLineExtensionConstraint;
    std::unique_ptr< CadConstraint > mXyVertexConstraint;
    Qgis::BetweenLineConstraint mBetweenLineConstraint;
    double mCommonAngleConstraint; // if 0: do not snap to common angles

    //! Flag that controls whether snapping to features has priority over common angle
    bool mSnappingPrioritizeFeatures = false;

    // Point list and current snap point / segment
    QList<QgsPoint> mCadPointList;
    QList<QgsPointXY> mSnappedSegment;

    bool mSessionActive = false;

    // Construction path history
    std::unique_ptr<QgsVectorLayer> mConstructionGuidesLayer;
    QgsFeatureId mConstructionGuideId;
    QgsLineString mConstructionGuideLine;
    bool mDeferredUpdateConstructionGuidesCrs = false;

    // Error message
    std::unique_ptr<QgsMessageBarItem> mErrorMessage;

    // UI
    QMap< double, QAction *> mCommonAngleActions; // map the common angle actions with their angle values
    QAction *mLineExtensionAction = nullptr;
    QAction *mXyVertexAction = nullptr;
    QAction *mRecordConstructionGuides = nullptr;
    QAction *mShowConstructionGuides = nullptr;
    QAction *mSnapToConstructionGuides = nullptr;
    QAction *mClearConstructionGuides = nullptr;

    // Snap indicator
    QgsPointLocator::Match mSnapMatch;
    QgsPointLocator::Match mLastSnapMatch;

    // Extra constraint context
    Qgis::LineExtensionSide mSoftLockLineExtension;
    double mSoftLockX;
    double mSoftLockY;
    QQueue< QgsPointLocator::Match > mLockedSnapVertices;

    // Advanced digitizing tool
    QPointer<QgsAdvancedDigitizingTool> mCurrentTool;

#ifdef SIP_RUN
    //! event filter for line edits in the dock UI (angle/distance/x/y line edits)
    bool eventFilter( QObject *obj, QEvent *event );
#endif
    //! Convenient method to convert a 2D Point to a QgsPoint
    QgsPoint pointXYToPoint( const QgsPointXY &point ) const;

    QMenu *mCommonAngleActionsMenu = nullptr;
    QMenu *mFloaterActionsMenu = nullptr;

    static const QgsSettingsEntryBool *settingsCadSnappingPriorityPrioritizeFeature;
    static const QgsSettingsEntryBool *settingsCadRecordConstructionGuides;
    static const QgsSettingsEntryBool *settingsCadShowConstructionGuides;
    static const QgsSettingsEntryBool *settingsCadSnapToConstructionGuides;

    friend class TestQgsAdvancedDigitizing;
    friend class TestQgsAdvancedDigitizingDockWidget;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsAdvancedDigitizingDockWidget::CadCapacities )

#endif // QGSADVANCEDDIGITIZINGDOCK_H
