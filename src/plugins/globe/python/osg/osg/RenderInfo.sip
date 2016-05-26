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

namespace osg {

class RenderInfo
{
%TypeHeaderCode
#include <osg/RenderInfo>
%End
public:

    RenderInfo();

    RenderInfo(const RenderInfo& rhs);

    RenderInfo(State* state, View* view);

    RenderInfo& operator = (const RenderInfo& rhs);

    unsigned int getContextID() const;

    void setState(State* state);
    State* getState();
    const State* getState() const;

    void setView(View* view);
    View* getView();
    const View* getView() const;

    void pushCamera(Camera* camera);
    void popCamera();

    Camera* getCurrentCamera();

    void setUserData(Referenced* userData);
    Referenced* getUserData();
    const Referenced* getUserData() const;

protected:

    typedef std::vector<Camera*> Cameras;

    ref_ptr<State>          _state;
    View*                   _view;
    Cameras                 _cameras;
    ref_ptr<Referenced>     _userData;
};

};
