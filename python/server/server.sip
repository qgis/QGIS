%Module(name=qgis._server,
        version=0,
        keyword_arguments="Optional")


%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtXml/QtXmlmod.sip

%Import core/core.sip


%Include qgsserverfilter.sip
%Include qgsmapserviceexception.sip
%Include qgscapabilitiescache.sip
%Include qgsrequesthandler.sip
%Include qgsserverinterface.sip
%Include qgsserverprojectparser.sip
%Include qgswcsprojectparser.sip
%Include qgswmsconfigparser.sip
%Include qgswmsprojectparser.sip
%Include qgswfsprojectparser.sip
%Include qgsconfigcache.sip
