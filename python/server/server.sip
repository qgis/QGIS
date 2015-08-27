%Module(name=qgis._server,
        version=0,
        keyword_arguments="Optional")


%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtXml/QtXmlmod.sip

%Import core/core.sip

%Feature HAVE_SERVER_PYTHON_PLUGINS

%If (HAVE_SERVER_PYTHON_PLUGINS)
%Include qgsserverfilter.sip
%Include qgsserverinterface.sip
%End

%Include qgsmapserviceexception.sip
%Include qgscapabilitiescache.sip
%Include qgsrequesthandler.sip
%Include qgsserverprojectparser.sip
%Include qgswcsprojectparser.sip
%Include qgswmsconfigparser.sip
%Include qgswmsprojectparser.sip
%Include qgswfsprojectparser.sip
%Include qgsconfigcache.sip
%Include qgsserver.sip
