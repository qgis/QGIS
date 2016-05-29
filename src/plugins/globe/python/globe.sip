%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip

%Import gui/gui.sip
%Import core/core.sip

//%Import osgEarth/Symbology/Symbology.sip
//%Import osgEarth/Features/Features.sip

%Import osg/osg/osg.sip
%Import osg/osgViewer/osgViewer.sip

%Module(name=qgis.globe,
        version=0,
        keyword_arguments="Optional")

%If(WS_WIN)
typedef void XEvent;
%End

//%Include qgsglobefeatureutils.sip
%Include qgsglobeinterface.sip
