/* OSG Viewer widget. Based on osgviewerQT example.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include "qgsosgviewer.h"

#include <osgViewer/ViewerEventHandlers>

#include <QString>

QgsGLWidgetAdapter::QgsGLWidgetAdapter( QWidget * parent, const char * name, const QGLWidget * shareWidget, WindowFlags f):
    QGLWidget(parent, shareWidget, f)
{
    _gw = new osgViewer::GraphicsWindowEmbedded(0,0,width(),height());
    setStereoMode();
    setFocusPolicy(Qt::ClickFocus);
    setMouseTracking ( true );
}

void QgsGLWidgetAdapter::resizeGL( int width, int height )
{
    _gw->getEventQueue()->windowResize(0, 0, width, height );
    _gw->resized(0,0,width,height);
}

void QgsGLWidgetAdapter::keyPressEvent( QKeyEvent* event )
{
    _gw->getEventQueue()->keyPress( (osgGA::GUIEventAdapter::KeySymbol) *(event->text().toAscii().data() ) );
}

void QgsGLWidgetAdapter::keyReleaseEvent( QKeyEvent* event )
{
    _gw->getEventQueue()->keyRelease( (osgGA::GUIEventAdapter::KeySymbol) *(event->text().toAscii().data() ) );
}

void QgsGLWidgetAdapter::mousePressEvent( QMouseEvent* event )
{
    int button = 0;
    switch(event->button())
    {
        case(Qt::LeftButton): button = 1; break;
        case(Qt::MidButton): button = 2; break;
        case(Qt::RightButton): button = 3; break;
        case(Qt::NoButton): button = 0; break;
        default: button = 0; break;
    }
    _gw->getEventQueue()->mouseButtonPress(event->x(), event->y(), button);
}

void QgsGLWidgetAdapter::mouseReleaseEvent( QMouseEvent* event )
{
    int button = 0;
    switch(event->button())
    {
        case(Qt::LeftButton): button = 1; break;
        case(Qt::MidButton): button = 2; break;
        case(Qt::RightButton): button = 3; break;
        case(Qt::NoButton): button = 0; break;
        default: button = 0; break;
    }
    _gw->getEventQueue()->mouseButtonRelease(event->x(), event->y(), button);
}

void QgsGLWidgetAdapter::mouseMoveEvent( QMouseEvent* event )
{
    _gw->getEventQueue()->mouseMotion(event->x(), event->y());
}

void QgsGLWidgetAdapter::wheelEvent(QWheelEvent *event)
{
    _gw->getEventQueue()->mouseScroll((event->delta()>0) ? osgGA::GUIEventAdapter::SCROLL_DOWN : osgGA::GUIEventAdapter::SCROLL_UP);
}

void QgsGLWidgetAdapter::setStereoMode()
{
  //TODO: maybe consolidate this method somewhere else since it is repeated in globe_plugin_dialog.cpp
  
  //osg::DisplaySettings::instance()->setStereo( (bool) settings.value( "/Plugin-Globe/stereo", 0 ).toInt() );
  //osg::DisplaySettings::instance()->setStereoMode( (osg::DisplaySettings::StereoMode) settings.value( "/Plugin-Globe/stereoMode", 1 ).toInt());
  
  QString stereoMode;
  stereoMode = settings.value( "/Plugin-Globe/stereoMode", "OFF" ).toString();
  
  if("OFF" == stereoMode)
  {
    osg::DisplaySettings::instance()->setStereo( false );
    }
  else if("ADVANCED" == stereoMode)
  {
    //osg::DisplaySettings::instance()->set
    }
  else
  {
    osg::DisplaySettings::instance()->setStereo( true );
    
    if("ANAGLYPHIC" == stereoMode)
    {
      osg::DisplaySettings::instance()->setStereoMode( osg::DisplaySettings::ANAGLYPHIC );
    }
    else if("VERTICAL_SPLIT" == stereoMode)
    {
      osg::DisplaySettings::instance()->setStereoMode( osg::DisplaySettings::VERTICAL_SPLIT );
    }
    else
    {
      //should never get here
    }
  }
}
