%Module(name=qgis._server,
        keyword_arguments="Optional")


%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtXml/QtXmlmod.sip

%Import core/core.sip

%Feature HAVE_SERVER_PYTHON_PLUGINS

%If (HAVE_SERVER_PYTHON_PLUGINS)
%Include qgsserverfilter.sip
%Include qgsserverinterface.sip
%Include qgsaccesscontrolfilter.sip
%Include qgsaccesscontrol.sip
%End

%Include qgsmapserviceexception.sip
%Include qgscapabilitiescache.sip
%Include qgsrequesthandler.sip
%Include qgsserverprojectparser.sip
%Include qgswmsconfigparser.sip
%Include qgswmsprojectparser.sip
%Include qgsconfigcache.sip
%Include qgsserversettings.sip
%Include qgsserverprojectutils.sip
%Include qgsserver.sip

%Include qgsserverrequest.sip
%Include qgsserverresponse.sip
%Include qgsbufferserverresponse.sip
%Include qgsbufferserverrequest.sip
%Include qgsserverexception.sip
%Include qgsservice.sip
%Include qgsservicemodule.sip
%Include qgsserviceregistry.sip

