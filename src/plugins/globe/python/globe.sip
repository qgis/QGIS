%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip

%Import gui/gui.sip
%Import core/core.sip

%Module(name=qgis.globe,
        version=0,
        keyword_arguments="Optional")

%If(WS_WIN)
typedef void XEvent;
%End

//%Include qgsglobefeatureutils.sip
%Include qgsglobeinterface.sip
