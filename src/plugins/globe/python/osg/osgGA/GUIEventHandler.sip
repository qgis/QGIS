/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/


namespace osgGA{

/**

GUIEventHandler provides a basic interface for any class which wants to handle
a GUI Events.

The GUIEvent is supplied by a GUIEventAdapter. Feedback resulting from the
handle method is supplied by a GUIActionAdapter, which allows the GUIEventHandler
to ask the GUI to take some action in response to an incoming event.

For example, consider a Trackball Viewer class which takes mouse events and
manipulates a scene camera in response. The Trackball Viewer is a GUIEventHandler,
and receives the events via the handle method. If the user 'throws' the model,
the Trackball Viewer class can detect this via the incoming events, and
request that the GUI set up a timer callback to continually redraw the view.
This request is made via the GUIActionAdapter class.

*/

class GUIEventHandler // : public osg::NodeCallback, public osg::Drawable::EventCallback
{
%TypeHeaderCode
#include <osgGA/GUIEventHandler>
%End
public:

        GUIEventHandler();
//        GUIEventHandler(const osgGA::GUIEventHandler& eh,const osg::CopyOp& copyop);

//        /** Event traversal node callback method.*/
//        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
//
//        /** Event traversal drawable callback method.*/
//        virtual void event(osg::NodeVisitor* nv, osg::Drawable* drawable);
//
//        /** Handle events, return true if handled, false otherwise. */
//        virtual bool handle(const GUIEventAdapter& ea,GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor*) { return handle(ea,aa); }
//
//        /** Convenience method that only passes on to the handle(,,,) method events that either haven't been
//          * handled yet, or have been handled but haven't be set to be ignored by the IgnoreHandledEventsMask.
//          * Note, this method is an inline method, and not appropriate for users to override, override the handle(,,,)
//          * method instead.*/
//        inline bool handleWithCheckAgainstIgnoreHandledEventsMask(const GUIEventAdapter& ea,GUIActionAdapter& aa, osg::Object* object, osg::NodeVisitor* nv)
//        {
//            if (!ea.getHandled() ||
//                (ea.getEventType() & _ignoreHandledEventsMask)==0)
//            {
//                bool handled = handle(ea,aa,object,nv);
//                if (handled) ea.setHandled(true);
//                return handled;
//            }
//            else
//            {
//                return false;
//            }
//        }
//
//        /** Deprecated, Handle events, return true if handled, false otherwise. */
//        virtual bool handle(const GUIEventAdapter&,GUIActionAdapter&) { return false; }
//
//        /** Convenience method that only passes on to the handle(,) method events that either haven't been
//          * handled yet, or have been handled but haven't be set to be ignored by the IgnoreHandledEventsMask.
//          * Note, this method is an inline method, and not appropriate for users to override, override the handle(,)
//          * method instead.*/
//        inline bool handleWithCheckAgainstIgnoreHandledEventsMask(const GUIEventAdapter& ea,GUIActionAdapter& aa)
//        {
//            if (!ea.getHandled() ||
//                (ea.getEventType() & _ignoreHandledEventsMask)==0)
//            {
//                bool handled = handle(ea,aa);
//                if (handled) ea.setHandled(true);
//                return handled;
//            }
//            else
//            {
//                return false;
//            }
//        }
//
//        /** Get the keyboard and mouse usage of this manipulator.*/
//        virtual void getUsage(osg::ApplicationUsage&) const {}
//
//        /** Set a mask of osgGA::GUIEeventAdapter::Event to be ignored if marked as handled */
//        void setIgnoreHandledEventsMask(unsigned int mask) { _ignoreHandledEventsMask = mask; }
//
//        /** Get the event mask of the osgGA::GUIEeventAdapter::Event to be ignored if marked as handled */
//        unsigned int getIgnoreHandledEventsMask() const { return _ignoreHandledEventsMask; };
private:
        osgGA::GUIEventHandler& operator=( const osgGA::GUIEventHandler& );
//
//protected:
//        unsigned int _ignoreHandledEventsMask;
//
};

};
