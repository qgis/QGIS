%Module(name=qgis._server,
        keyword_arguments="All")

%If (SIP_4_20 - )
%DefaultDocstringSignature "prepended"
%End

%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtXml/QtXmlmod.sip

%Import core/core.sip

%Feature HAVE_SERVER_PYTHON_PLUGINS

%Include server_auto.sip
