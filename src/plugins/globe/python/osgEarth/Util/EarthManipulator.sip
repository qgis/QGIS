/* -*-c++-*- */
/* osgEarth - Dynamic map generation toolkit for OpenSceneGraph
 * Copyright 2008-2013 Pelican Mapping
 * http://osgearth.org
 *
 * osgEarth is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

namespace osgEarth {
namespace Util {

    /**
     * A programmable manipulator suitable for use with geospatial terrains.
     *
     * You can use the "Settings" class to define custom input device bindings
     * and navigation parameters. Create one or more of these and call
     * applySettings() to "program" the manipulator at runtime.
     */
    class EarthManipulator // : public osgGA::CameraManipulator
    {
%TypeHeaderCode
    #include <osgEarthUtil/EarthManipulator>
    using namespace osgEarth;
%End
    public:

//        /** Bindable manipulator actions. */
//        enum ActionType {
//            ACTION_NULL,
//            ACTION_HOME,
//            ACTION_GOTO,
//            ACTION_PAN,
//            ACTION_PAN_LEFT,
//            ACTION_PAN_RIGHT,
//            ACTION_PAN_UP,
//            ACTION_PAN_DOWN,
//            ACTION_ROTATE,
//            ACTION_ROTATE_LEFT,
//            ACTION_ROTATE_RIGHT,
//            ACTION_ROTATE_UP,
//            ACTION_ROTATE_DOWN,
//            ACTION_ZOOM,
//            ACTION_ZOOM_IN,
//            ACTION_ZOOM_OUT,
//            ACTION_EARTH_DRAG
//        };
//
//        /** Vector of action types */
//        typedef std::vector<ActionType> ActionTypeVector;
//
//        /** Bindable event types. */
//        enum EventType {
//            EVENT_MOUSE_DOUBLE_CLICK = osgGA::GUIEventAdapter::DOUBLECLICK,
//            EVENT_MOUSE_DRAG         = osgGA::GUIEventAdapter::DRAG,
//            EVENT_KEY_DOWN           = osgGA::GUIEventAdapter::KEYDOWN,
//            EVENT_SCROLL             = osgGA::GUIEventAdapter::SCROLL,
//            EVENT_MOUSE_CLICK        = osgGA::GUIEventAdapter::USER << 1,
//            EVENT_MULTI_DRAG         = osgGA::GUIEventAdapter::USER << 2,   // drag with 2 fingers
//            EVENT_MULTI_PINCH        = osgGA::GUIEventAdapter::USER << 3,   // pinch with 2 fingers
//            EVENT_MULTI_TWIST        = osgGA::GUIEventAdapter::USER << 4    // drag 2 fingers in different directions
//        };
//
//        /** Bindable mouse buttons. */
//        enum MouseEvent {
//            MOUSE_LEFT_BUTTON   = osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON,
//            MOUSE_MIDDLE_BUTTON = osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON,
//            MOUSE_RIGHT_BUTTON  = osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON
//        };
//
//        /** Action options. Certain options are only meaningful to certain Actions.
//            See the bind* documentation for information. */
//        enum ActionOptionType {
//            OPTION_SCALE_X,             // Sensitivity multiplier for horizontal input movements
//            OPTION_SCALE_Y,             // Sensitivity multiplier for vertical input movements
//            OPTION_CONTINUOUS,          // Whether to act as long as the button or key is depressed
//            OPTION_SINGLE_AXIS,         // If true, only operate on one axis at a time (the one with the larger value)
//            OPTION_GOTO_RANGE_FACTOR,   // for ACTION_GOTO, multiply the Range by this factor (to zoom in/out)
//            OPTION_DURATION             // Time it takes to complete the action (in seconds)
//        };
//
//        /** Tethering options **/
//        enum TetherMode
//        {
//            TETHER_CENTER,              // The camera will follow the center of the node.
//            TETHER_CENTER_AND_ROTATION, // The camera will follow the node and all rotations made by the node
//            TETHER_CENTER_AND_HEADING   // The camera will follow the node and only follow heading rotation
//        };
//
//        /** Camera projection matrix type **/
//        enum CameraProjection
//        {
//            PROJ_PERSPECTIVE,
//            PROJ_ORTHOGRAPHIC
//        };
//
//        struct OSGEARTHUTIL_EXPORT ActionOption {
//            ActionOption() { }
//            ActionOption( int option, bool value ) : _option(option), _bool_value(value) { }
//            ActionOption( int option, int value ) : _option(option), _int_value(value) { }
//            ActionOption( int option, double value ) : _option(option), _dbl_value(value) { }
//
//            int option() const { return _option; }
//            bool boolValue() const { return _bool_value; }
//            int intValue() const { return _int_value; }
//            double doubleValue() const { return _dbl_value; }
//
//        private:
//            int _option;
//            union {
//                bool _bool_value;
//                int _int_value;
//                double _dbl_value;
//            };
//        };
//
//        struct OSGEARTHUTIL_EXPORT ActionOptions : public std::vector<ActionOption> {
//            void add( int option, bool value ) { push_back( ActionOption(option,value) ); }
//            void add( int option, int value )  { push_back( ActionOption(option,value) ); }
//            void add( int option, double value) { push_back( ActionOption(option,value) ); }
//        };
//
//    protected:
//        struct InputSpec
//        {
//            InputSpec( int event_type, int input_mask, int modkey_mask )
//                : _event_type(event_type), _input_mask(input_mask), _modkey_mask( modkey_mask ) { }
//            InputSpec( const InputSpec& rhs )
//                : _event_type(rhs._event_type), _input_mask(rhs._input_mask), _modkey_mask(rhs._modkey_mask) { }
//
//            bool operator == ( const InputSpec& rhs ) const {
//                return _event_type == rhs._event_type &&
//                       _input_mask == rhs._input_mask &&
//                       ((_modkey_mask|osgGA::GUIEventAdapter::MODKEY_NUM_LOCK) == (rhs._modkey_mask|osgGA::GUIEventAdapter::MODKEY_NUM_LOCK));
//            }
//
//            inline bool operator < (const InputSpec& rhs) const {
//                if ( _event_type < rhs._event_type) return true;
//                else if ( _event_type > rhs._event_type ) return false;
//                else if ( _input_mask < rhs._input_mask ) return true;
//                else if ( _input_mask > rhs._input_mask ) return false;
//                else return ( _modkey_mask < rhs._modkey_mask );
//            }
//
//            int _event_type;
//            int _input_mask;
//            int _modkey_mask;
//        };
//        typedef std::list<InputSpec> InputSpecs;
//
//        enum Direction {
//            DIR_NA,
//            DIR_LEFT,
//            DIR_RIGHT,
//            DIR_UP,
//            DIR_DOWN
//        };
//
//        struct Action
//        {
//            Action( ActionType type =ACTION_NULL );
//            Action( ActionType type, const ActionOptions& options );
//            Action( const Action& rhs );
//            ActionType _type;
//            Direction _dir;
//            ActionOptions _options;
//            bool getBoolOption( int option, bool defaultValue ) const;
//            int getIntOption( int option, int defaultValue ) const;
//            double getDoubleOption( int option, double defaultValue ) const;
//        private:
//            void init();
//        };
//
//        void dumpActionInfo( const Action& action, osg::NotifySeverity level ) const;
//
//        static Action NullAction;
//
//    public:
//
//        class OSGEARTHUTIL_EXPORT Settings : public osg::Referenced, public Revisioned
//        {
//        public:
//            // construct with default settings
//            Settings();
//
//            // copy ctor
//            Settings( const Settings& rhs );
//
//            /** dtor */
//            virtual ~Settings() { }
//
//            /**
//             * Assigns behavior to the action of dragging the mouse while depressing one or
//             * more mouse buttons and modifier keys.
//             *
//             * @param action
//             *      The EarthManipulator::ActionType value to which to bind this mouse
//             *      input specification.
//             *
//             * @param button_mask
//             *      Mask of osgGA::GUIEventAdapter::MouseButtonMask values
//             *
//             * @param modkey_mask (default = 0L)
//             *      A mask of osgGA::GUIEventAdapter::ModKeyMask values defining a modifier key
//             *      combination to associate with the action.
//             *
//             * @param options
//             *      Action options. Valid options are:
//             *      OPTION_CONTINUOUS, OPTION_SCALE_X, OPTION_SCALE_Y
//             */
//            void bindMouse(
//                ActionType action, int button_mask,
//                int modkey_mask = 0L,
//                const ActionOptions& options =ActionOptions() );
//
//            /**
//             * Assigns a bevahior to the action of clicking one or more mouse buttons.
//             *
//             * @param action
//             *      The EarthManipulator::ActionType value to which to bind this mouse click
//             *      input specification.
//             *
//             * @param button_mask
//             *      Mask of osgGA::GUIEventAdapter::MouseButtonMask values
//             *
//             * @param modkey_mask (default = 0L)
//             *      A mask of osgGA::GUIEventAdapter::ModKeyMask values defining a modifier key
//             *      combination to associate with the action.
//             *
//             * @param options
//             *      Action options. Valid options are:
//             *      OPTION_GOTO_RANGE_FACTOR, OPTION_DURATION
//             */
//            void bindMouseClick(
//                ActionType action, int button_mask,
//                int modkey_mask =0L,
//                const ActionOptions& options =ActionOptions() );
//
//            /**
//             * Assigns a bevahior to the action of double-clicking one or more mouse buttons.
//             *
//             * @param action
//             *      The EarthManipulator::ActionType value to which to bind this double-click
//             *      input specification.
//             *
//             * @param button_mask
//             *      Mask of osgGA::GUIEventAdapter::MouseButtonMask values
//             *
//             * @param modkey_mask (default = 0L)
//             *      A mask of osgGA::GUIEventAdapter::ModKeyMask values defining a modifier key
//             *      combination to associate with the action.
//             *
//             * @param options
//             *      Action options. Valid options are:
//             *      OPTION_GOTO_RANGE_FACTOR, OPTION_DURATION
//             */
//            void bindMouseDoubleClick(
//                ActionType action, int button_mask,
//                int modkey_mask =0L,
//                const ActionOptions& options =ActionOptions() );
//
//            /**
//             * Assigns a bevahior to the action of depressing a key.
//             *
//             * @param action
//             *      The EarthManipulator::ActionType value to which to bind this key
//             *      input specification.
//             *
//             * @param key
//             *      A osgGA::GUIEventAdapter::KeySymbol value
//             *
//             * @param modkey_mask (default = 0L)
//             *      A mask of osgGA::GUIEventAdapter::ModKeyMask values defining a modifier key
//             *      combination to associate with the action.
//             *
//             * @param options
//             *      Action options. Valid options are:
//             *      OPTION_CONTINUOUS
//             */
//            void bindKey(
//                ActionType action, int key,
//                int modkey_mask =0L,
//                const ActionOptions& options =ActionOptions() );
//
//            /**
//             * Assigns a bevahior to operation of the mouse's scroll wheel.
//             *
//             * @param action
//             *      The EarthManipulator::ActionType value to which to bind this scroll
//             *      input specification.
//             *
//             * @param scrolling_motion
//             *      A osgGA::GUIEventAdapter::ScrollingMotion value
//             *
//             * @param modkey_mask (default = 0L)
//             *      A mask of osgGA::GUIEventAdapter::ModKeyMask values defining a modifier key
//             *      combination to associate with the action.
//             *
//             * @param options
//             *      Action options. Valid options are:
//             *      OPTION_SCALE_Y
//             */
//            void bindScroll(
//                ActionType action, int scrolling_motion,
//                int modkey_mask =0L,
//                const ActionOptions& options =ActionOptions() );
//
//
//            void bindPinch(
//                ActionType action, const ActionOptions& =ActionOptions() );
//
//            void bindMultiDrag(
//                ActionType action, const ActionOptions& =ActionOptions() );
//
//            /**
//             * Sets an overall mouse sensitivity factor.
//             *
//             * @param value
//             *      A scale factor to apply to mouse readings.
//             *      1.0 = default; < 1.0 = less sensitive; > 1.0 = more sensitive.
//             */
//            void setMouseSensitivity( double value ) { _mouse_sens = value; }
//
//            /**
//             * Gets the overall mouse sensitivity scale factor. Default = 1.0.
//             */
//            double getMouseSensitivity() const { return _mouse_sens; }
//
//            /**
//             * Sets the keyboard action sensitivity factor. This applies to navigation actions
//             * that are bound to keyboard events. For example, you may bind the LEFT arrow to
//             * the ACTION_PAN_LEFT action; this factor adjusts how much panning will occur during
//             * each frame that the key is depressed.
//             *
//             * @param value
//             *      A scale factor to apply to keyboard-controller navigation.
//             *      1.0 = default; < 1.0 = less sensitive; > 1.0 = more sensitive.
//             */
//            void setKeyboardSensitivity( double value ) { _keyboard_sens = value; }
//
//            /**
//             * Gets the keyboard action sensitivity scale factor. Default = 1.0.
//             */
//            double getKeyboardSensitivity() const { return _keyboard_sens; }
//
//            /**
//             * Sets the scroll-wheel sensitivity factor. This applies to navigation actions
//             * that are bound to scrolling events. For example, you may bind the scroll wheel to
//             * the ACTION_ZOOM_IN action; this factor adjusts how much zooming will occur each time
//             * you click the scroll wheel.
//             *
//             * @param value
//             *      A scale factor to apply to scroll-wheel-controlled navigation.
//             *      1.0 = default; < 1.0 = less sensitive; > 1.0 = more sensitive.
//             */
//            void setScrollSensitivity( double value ) { _scroll_sens = value; }
//
//            /**
//             * Gets the scroll wheel sensetivity scale factor. Default = 1.0.
//             */
//            double getScrollSensitivity() const { return _scroll_sens; }
//
//            /**
//             * When set to true, prevents simultaneous control of pitch and azimuth.
//             *
//             * Usually you can alter pitch and azimuth at the same time. When this flag
//             * is set, you can only control one at a time - if you start slewing the azimuth of the camera,
//             * the pitch stays locked until you stop moving and then start slewing the pitch.
//             *
//             * Default = false.
//             */
//            void setSingleAxisRotation( bool value ) { _single_axis_rotation = value; }
//
//            /**
//             * Gets whether simultaneous control over pitch and azimuth is disabled.
//             * Default = false.
//             */
//            bool getSingleAxisRotation() const { return _single_axis_rotation; }
//
//            /**
//             * Sets whether to lock in a camera heading when performing panning operations (i.e.,
//             * changing the focal point).
//             */
//            void setLockAzimuthWhilePanning( bool value ) { _lock_azim_while_panning = value; }
//
//            /**
//             * Gets true if the manipulator should lock in a camera heading when performing panning
//             * operations (i.e. changing the focal point.)
//             */
//            bool getLockAzimuthWhilePanning() const { return _lock_azim_while_panning; }
//
//            /**
//             * Sets the minimum and maximum allowable local camera pitch, in degrees.
//             *
//             * By "local" we mean relative to the tangent plane passing through the focal point on
//             * the surface of the terrain.
//             *
//             * Defaults are: Min = -90, Max = -10.
//             */
//            void setMinMaxPitch( double min_pitch, double max_pitch );
//
//            /** Gets the minimum allowable local pitch, in degrees. */
//            double getMinPitch() const { return _min_pitch; }
//
//            /** Gets the maximum allowable local pitch, in degrees. */
//            double getMaxPitch() const { return _max_pitch; }
//
//            /** Gets the max x offset in world coordates */
//            double getMaxXOffset() const { return _max_x_offset; }
//
//            /** Gets the max y offset in world coordates */
//            double getMaxYOffset() const { return _max_y_offset; }
//
//            /** Gets the minimum distance from the focal point in world coordinates */
//            double getMinDistance() const {return _min_distance; }
//
//            /** Gets the maximum distance from the focal point in world coordinates */
//            double getMaxDistance() const {return _max_distance; }
//
//            /** Sets the min and max distance from the focal point in world coordiantes */
//            void setMinMaxDistance( double min_distance, double max_distance);
//
//            /**
//            * Sets the maximum allowable offsets for the x and y camera offsets in world coordinates
//            */
//            void setMaxOffset(double max_x_offset, double max_y_offset);
//
//            /**
//            * Gets the TetherMode
//            */
//            TetherMode getTetherMode() const { return _tether_mode; }
//
//            /**
//            * Sets the TetherMode
//            */
//            void setTetherMode( TetherMode tether_mode ) { _tether_mode = tether_mode; }
//
//            /** Access to the list of Actions that will automatically break a tether */
//            ActionTypeVector& getBreakTetherActions() { return _breakTetherActions; }
//            const ActionTypeVector& getBreakTetherActions() const { return _breakTetherActions; }
//
//            /** Whether a setViewpoint transition whould "arc" */
//            void setArcViewpointTransitions( bool value );
//            bool getArcViewpointTransitions() const { return _arc_viewpoints; }
//
//            /** Activates auto-duration for transitioned viewpoints. */
//            void setAutoViewpointDurationEnabled( bool value );
//            bool getAutoViewpointDurationEnabled() const { return _auto_vp_duration; }
//
//            void setAutoViewpointDurationLimits( double minSeconds, double maxSeconds );
//            void getAutoViewpointDurationLimits( double& out_minSeconds, double& out_maxSeconds ) const {
//                out_minSeconds = _min_vp_duration_s;
//                out_maxSeconds = _max_vp_duration_s;
//            }
//
//            /** The camera projection matrix type */
//            void setCameraProjection( const CameraProjection& value );
//            const CameraProjection& getCameraProjection() const { return _camProjType; }
//
//            /** Frustum offset (in pixels) */
//            void setCameraFrustumOffsets( const osg::Vec2s& offsets );
//            const osg::Vec2s& getCameraFrustumOffsets() const { return _camFrustOffsets; }
//
//        private:
//
//            friend class EarthManipulator;
//
//            typedef std::pair<InputSpec,Action> ActionBinding;
//            typedef std::map<InputSpec,Action> ActionBindings;
//
//            // Gets the action bound to the provided input specification, or NullAction if there is
//            // to matching binding.
//            const Action& getAction( int event_type, int input_mask, int modkey_mask ) const;
//
//            void expandSpec( const InputSpec& input, InputSpecs& output ) const;
//            void bind( const InputSpec& spec, const Action& action );
//
//        private:
//
//            ActionBindings _bindings;
//            bool _single_axis_rotation;
//            bool _lock_azim_while_panning;
//            double _mouse_sens;
//            double _keyboard_sens;
//            double _scroll_sens;
//            double _min_pitch;
//            double _max_pitch;
//
//            double _max_x_offset;
//            double _max_y_offset;
//
//            double _min_distance;
//            double _max_distance;
//
//            TetherMode _tether_mode;
//            ActionTypeVector _breakTetherActions;
//            bool _arc_viewpoints;
//            bool _auto_vp_duration;
//            double _min_vp_duration_s, _max_vp_duration_s;
//
//            CameraProjection _camProjType;
//            osg::Vec2s _camFrustOffsets;
//        };
//
//    public:
//        EarthManipulator();
//        EarthManipulator( const EarthManipulator& rhs );
//
//        /**
//         * Applies a new settings object to the manipulator, which takes effect immediately.
//         */
//        void applySettings( Settings* settings );
//
//        /**
//         * Gets a handle on the current manipulator settings object.
//         */
//        Settings* getSettings() const;
//
//        /**
//         * Gets the current camera position.
//         */
//        Viewpoint getViewpoint() const;
//
//        /**
//         * Sets the camera position, optionally moving it there over time.
//         */
//        virtual void setViewpoint( const Viewpoint& vp, double duration_s =0.0 );
//
//        /**
//         * Cancels a viewpoint transition if one is in progress
//         */
//        void cancelViewpointTransition() { _setting_viewpoint = false; }
//
//        /**
//         * Sets the viewpoint to activate when performing the ACTION_HOME action.
//         */
//        void setHomeViewpoint( const Viewpoint& vp, double duration_s = 0.0 );
//
//        /**
//         * Locks the camera's focal point on the center of a node's bounding sphere.
//         * While tethered, you can still call navigate or call setViewpoint() to move the
//         * camera relative to the tether object. Pass NULL to deactivate the tether.
//         *
//         * @param node
//         *      Node to which to tether the viewpoint.
//         */
//        void setTetherNode( osg::Node* node );
//
//        /**
//         * Gets the node to which the camera is tethered, or NULL if tethering is
//         * disabled.
//         */
//        osg::Node* getTetherNode() const;
//
//        /**
//         * Gets the spatial reference system of the terrain map to which this
//         * manipulator is currently attached.
//         */
//        const osgEarth::SpatialReference* getSRS() const;
//
//        /**
//         * Move the focal point of the camera using deltas (normalized screen coords).
//         */
//        virtual void pan( double dx, double dy );
//
//        /**
//         * Rotate the camera (dx = azimuth, dy = pitch) using deltas (radians).
//         */
//        virtual void rotate( double dx, double dy );
//
//        /**
//         * Zoom the camera using deltas (dy only)
//         */
//        virtual void zoom( double dx, double dy );
//
//        /**
//         * Drag the earth using deltas
//         */
//        virtual void drag( double dx, double dy, osg::View* view);
//
//        /**
//         * Converts screen coordinates (relative to the view's viewpoint) to world
//         * coordinates. Note, this method will use the mask set by setTraversalMask().
//         *
//         * @param x, y
//         *      Viewport coordinates
//         * @param view
//         *      View for which to calculate world coordinates
//         * @param out_coords
//         *      Output world coordinates (only valid if the method returns true)
//         */
//        bool screenToWorld(float x, float y, osg::View* view, osg::Vec3d& out_coords ) const;
//
//        /**
//         * Gets the distance from the focal point in world coordiantes
//         */
//        double getDistance() const { return _distance; }
//
//        /**
//         * Sets the distance from the focal point in world coordinates.
//         *
//         * The incoming distance value will be clamped within the valid range specified by the settings.
//         */
//        void   setDistance( double distance);
//
//        /**
//         * Gets the rotation of the manipulator.  Note:  This rotation is in addition to the rotation needed to center the view on the focal point.
//         */
//        const osg::Quat& getRotation() { return _rotation; }
//
//        /**
//         * Sets the rotation of the manipulator.  Note:  This rotation is in addition to the rotation needed to center the view on the focal point.
//         */
//        void  setRotation( const osg::Quat& rotation) { _rotation = rotation; }
//
//
//
//    public: // osgGA::MatrixManipulator
//
//        virtual const char* className() const { return "EarthManipulator"; }
//
//        /** set the position of the matrix manipulator using a 4x4 Matrix.*/
//        virtual void setByMatrix(const osg::Matrixd& matrix);
//
//        /** set the position of the matrix manipulator using a 4x4 Matrix.*/
//        virtual void setByInverseMatrix(const osg::Matrixd& matrix) { setByMatrix(osg::Matrixd::inverse(matrix)); }
//
//        /** get the position of the manipulator as 4x4 Matrix.*/
//        virtual osg::Matrixd getMatrix() const;
//
//        /** get the position of the manipulator as a inverse matrix of the manipulator, typically used as a model view matrix.*/
//        virtual osg::Matrixd getInverseMatrix() const;
//
//        // Gets the stereo convergance mode.
//        virtual osgUtil::SceneView::FusionDistanceMode getFusionDistanceMode() const { return osgUtil::SceneView::USE_FUSION_DISTANCE_VALUE; }
//
//        // Gets the stereo convergance distance.
//        virtual float getFusionDistanceValue() const { return _distance; }
//
//        // Attach a node to the manipulator.
//        virtual void setNode(osg::Node*);
//
//        // Gets the node to which this manipulator is attached.
//        virtual osg::Node* getNode();
//
//        // Move the camera to the default position.
//        virtual void home(double /*unused*/);
//        virtual void home(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);
//
//        // Start/restart the manipulator.
//        virtual void init(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);
//
//        // handle events, return true if handled, false otherwise.
//        virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);
//
//        // Get the keyboard and mouse usage of this manipulator.
//        virtual void getUsage(osg::ApplicationUsage& usage) const;
//
//        virtual void computeHomePosition();
//
//
//    protected:
//
//        virtual ~EarthManipulator();
//
//        bool intersect(const osg::Vec3d& start, const osg::Vec3d& end, osg::Vec3d& intersection) const;
//
//        // resets the mouse event stack and pushes the provided event.
//        void resetMouse( osgGA::GUIActionAdapter& );
//
//        // Reset the internal event stack.
//        void flushMouseEventStack();
//
//        // Add the current mouse osgGA::GUIEvent to internal stack.
//        void addMouseEvent(const osgGA::GUIEventAdapter& ea);
//
//        // sets the camera position by doing a "look at" calculation. This is only used for
//        // the "home" function at the moment -- look into depcrecation -GW
//        void setByLookAt(const osg::Vec3d& eye, const osg::Vec3d& lv, const osg::Vec3d& up);
//
//        // checks to see whether the mouse is "moving".
//        bool isMouseMoving();
//
//        // This sets the camera's roll based on your location on the globe.
//        void recalculateRoll();
//
//    protected:
//
//        enum TaskType
//        {
//            TASK_NONE,
//            TASK_PAN,
//            TASK_ROTATE,
//            TASK_ZOOM
//        };
//
//        struct Task : public osg::Referenced
//        {
//            Task() : _type(TASK_NONE) { }
//            void set( TaskType type, double dx, double dy, double duration, double now ) {
//                _type = type; _dx = dx; _dy = dy; _duration_s = duration; _time_last_service = now;
//            }
//            TaskType _type;
//            double   _dx, _dy;
//            double   _duration_s;
//            double   _time_last_service;
//        };
//
//        // "ticks" the resident Task, which allows for multi-frame animation of navigation
//        // movements.
//        bool serviceTask();
//
//        //void recalculateLocalPitchAndAzimuth();
//        void getLocalEulerAngles( double* out_azim, double* out_pitch =0L ) const;
//
//
//        void recalculateCenter( const osg::CoordinateFrame& frame );
//
//        osg::Matrixd getRotation(const osg::Vec3d& center) const;
//        osg::Quat makeCenterRotation(const osg::Vec3d& center) const
//        {
//            return getRotation(center).getRotate().inverse();
//        }
//
//        void updateTether();
//
//        void updateSetViewpoint();
//
//        void updateHandCam( const osg::Timer_t& now );
//
//        bool isMouseClick( const osgGA::GUIEventAdapter* mouse_up_event ) const;
//
//        void applyOptionsToDeltas( const Action& action, double& dx, double& dy );
//
//        void configureDefaultSettings();
//
//        void reinitialize();
//
//        bool established();
//
//        // sets the new center (focal) point and recalculates it's L2W matrix.
//        void setCenter( const osg::Vec3d& center );
//
//        // creates a "local-to-world" transform relative to the input point.
//        bool createLocalCoordFrame( const osg::Vec3d& worldPos, osg::CoordinateFrame& out_frame ) const;
//
//        // returns an ActionType that would be initiated by the OSG UI event
//        ActionType getActionTypeForEvent( const osgGA::GUIEventAdapter& ea ) const;
//
//    public:
//
//        void recalculateCenter() { recalculateCenter(_centerLocalToWorld); }
//
//        const GeoPoint& centerMap() const { return _centerMap; }
//
//    protected:
//        typedef osgGA::GUIEventAdapter::TouchData::TouchPoint TouchPoint;
//        typedef std::vector<TouchPoint> MultiTouchPoint; // one per ID (finger/touchpoint)
//        typedef std::deque<MultiTouchPoint> MultiTouchPointQueue;
//        MultiTouchPointQueue _touchPointQueue;
//        //struct TouchEvent {
//        //    EventType _eventType;     // derived touch/other event
//        //    float     _deltaDistance; // change in distance between 2 touches
//        //    float     _dx[2], _dy[2]; // movement of primary and secondary touch points
//        //    float     _dot;
//        //};
//        struct TouchEvent {
//            TouchEvent() : _mbmask(0) { }
//            EventType _eventType;
//            unsigned  _mbmask;
//            float     _dx, _dy;
//        };
//        typedef std::vector<TouchEvent> TouchEvents;
//        void addTouchEvents( const osgGA::GUIEventAdapter& ea );
//        bool parseTouchEvents( TouchEvents& ev );
//
//
//        // Applies an action using the raw input parameters.
//        bool handleAction( const Action& action, double dx, double dy, double duration );
//
//        virtual bool handleMouseAction( const Action& action, osg::View* view );
//        virtual bool handleMouseClickAction( const Action& action );
//        virtual bool handleKeyboardAction( const Action& action, double duration_s = DBL_MAX );
//        virtual bool handleScrollAction( const Action& action, double duration_s = DBL_MAX );
//        virtual bool handlePointAction( const Action& type, float mx, float my, osg::View* view );
//        virtual void handleContinuousAction( const Action& action, osg::View* view );
//        virtual void handleMovementAction( const ActionType& type, double dx, double dy, osg::View* view );
//        //virtual bool handleMultiTouchAction( const Action& action, const TouchEvent& te, osg::View* view );
//
//    protected:
//
//        // makeshift "stack" of the last 2 incoming events.
//        osg::ref_ptr<const osgGA::GUIEventAdapter> _ga_t1;
//        osg::ref_ptr<const osgGA::GUIEventAdapter> _ga_t0;
//
//        osg::ref_ptr<const osgGA::GUIEventAdapter> _mouse_down_event;
//
//        osg::observer_ptr<osg::Node> _node;
//        osg::observer_ptr<osg::CoordinateSystemNode> _csn;
//#ifdef USE_OBSERVER_NODE_PATH
//        osg::ObserverNodePath _csnObserverPath;
//#endif
//        osg::NodePath _csnPath;
//
//        osg::ref_ptr<const osgEarth::SpatialReference> _cached_srs;
//        bool _is_geocentric;
//        bool _srs_lookup_failed;
//
//        osg::observer_ptr<osg::Node> _tether_node;
//        osg::Transform*              _tether_xform;
//        osg::Vec3d                   _tether_local_center;
//        Viewpoint                    _pre_tether_vp;
//
//        double                  _time_s_last_frame;
//        double                  _time_s_now;
//        osg::Timer_t            _now;
//        double                  _delta_t;
//        double                  _t_factor;
//        bool                    _thrown;
//
//        // The world coordinate of the Viewpoint focal point.
//        osg::Vec3d              _center;
//        GeoPoint                _centerMap;
//
//        // local2world matrix for the center point.
//        osg::CoordinateFrame    _centerLocalToWorld;
//
//        // The rotation (heading and pitch) of the camera in the
//        // earth-local frame.
//        osg::Quat               _rotation;
//
//        // The rotation that makes the camera look down on the focal
//        // point on the earth. This is equivalent to a rotation by
//        // latitude, longitude.
//        osg::Quat               _centerRotation;
//        double                  _distance;
//        double                  _offset_x;
//        double                  _offset_y;
//        osg::Vec3d              _previousUp;
//        osg::ref_ptr<Task>      _task;
//        osg::Timer_t            _time_last_frame;
//        //double                  _local_pitch;
//        //double                  _local_azim;
//
//        bool                    _continuous;
//        double                  _continuous_dx;
//        double                  _continuous_dy;
//        double                  _last_continuous_action_time;
//
//        double                  _single_axis_x;
//        double                  _single_axis_y;
//
//        // the "pending" viewpoint is only used to enable setting the
//        // viewpoint before the frame loop starts
//        bool                    _has_pending_viewpoint;
//        Viewpoint               _pending_viewpoint;
//        double                  _pending_viewpoint_duration_s;
//
//        bool                    _setting_viewpoint;
//        Viewpoint               _start_viewpoint;
//        double                  _delta_heading, _delta_pitch, _delta_range, _arc_height;
//        osg::Vec3d              _delta_focal_point;
//        double                  _time_s_set_viewpoint;
//        double                  _set_viewpoint_duration_s;
//        double                  _set_viewpoint_accel;
//        double                  _set_viewpoint_accel_2;
//
//        unsigned                _frame_count;
//
//        osg::ref_ptr<Settings> _settings;
//
//        osgEarth::optional<Viewpoint> _homeViewpoint;
//        double _homeViewpointDuration;
//
//        Action _last_action;
//
//        // to support updating the camera after the update traversal (e.g., for tethering)
//        osg::observer_ptr<osg::Camera> _viewCamera;
//        double _vfov;
//        double _tanHalfVFOV;
//        optional<osg::CullSettings::ComputeNearFarMode> _savedCNFMode;
//        Revision _viewCameraSettingsMonitor;
//
//        struct CameraPostUpdateCallback : public osg::NodeCallback {
//            CameraPostUpdateCallback(EarthManipulator* m) : _m(m) { }
//            virtual void operator()(osg::Node* node, osg::NodeVisitor* nv) { _m->postUpdate(); traverse(node,nv); }
//            EarthManipulator* _m;
//        };
//        void postUpdate();
//        friend struct CameraUpdateCallback;
//
//        osg::observer_ptr<CameraPostUpdateCallback> _cameraUpdateCB;
//
//        void updateCamera( osg::Camera* eventCamera );
//        // Support snappy transition when the pointer leaves and
//        // returns to earth during a drag
//        osg::Vec3d               _lastPointOnEarth;
//
//
//        osg::ref_ptr< TerrainCallback > _terrainCallback;
//
       private:
         ~EarthManipulator();
    };

}; // namespace Util
}; // namespace osgEarth
