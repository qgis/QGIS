%Module(name=qgis._gui,
        keyword_arguments="All")

%DefaultDocstringSignature "prepended"

%Feature HAVE_QSCI_SIP

%Import core/core.sip

%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtXml/QtXmlmod.sip

%If (HAVE_QSCI_SIP)
  %Import Qsci/qscimod5.sip
%End

%Include gui_auto.sip
