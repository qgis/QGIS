%Module(name=qgis._analysis,
        keyword_arguments="All")

%If (SIP_4_20 - )
%DefaultDocstringSignature "prepended"
%End

%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtXml/QtXmlmod.sip

%Import core/core.sip

%Include analysis_auto.sip
