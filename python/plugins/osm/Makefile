
GEN_FILES = DlgLoadOSM_ui.py DlgSaveOSM_ui.py DlgDownloadOSM_ui.py DlgUploadOSM_ui.py DockWidget_ui.py DlgAddRelation_ui.py DockUndoRedo_ui.py DlgImport_ui.py resources.py

all: $(GEN_FILES)

DlgLoadOSM_ui.py: ui_files/DlgLoadOSM.ui
	pyuic4 -o DlgLoadOSM_ui.py ui_files/DlgLoadOSM.ui

DlgSaveOSM_ui.py: ui_files/DlgSaveOSM.ui
	pyuic4 -o DlgSaveOSM_ui.py ui_files/DlgSaveOSM.ui

DlgDownloadOSM_ui.py: ui_files/DlgDownloadOSM.ui
	pyuic4 -o DlgDownloadOSM_ui.py ui_files/DlgDownloadOSM.ui

DlgUploadOSM_ui.py: ui_files/DlgUploadOSM.ui
	pyuic4 -o DlgUploadOSM_ui.py ui_files/DlgUploadOSM.ui

DockWidget_ui.py: ui_files/DockWidget.ui
	pyuic4 -o DockWidget_ui.py ui_files/DockWidget.ui

DlgAddRelation_ui.py: ui_files/DlgAddRelation.ui
	pyuic4 -o DlgAddRelation_ui.py ui_files/DlgAddRelation.ui

DockUndoRedo_ui.py: ui_files/DockUndoRedo.ui
	pyuic4 -o DockUndoRedo_ui.py ui_files/DockUndoRedo.ui

DlgImport_ui.py: ui_files/DlgImport.ui
	pyuic4 -o DlgImport_ui.py ui_files/DlgImport.ui

resources.py: resources.qrc
	pyrcc4 -o resources.py resources.qrc

clean:
	rm -f $(GEN_FILES) *.pyc

package:
	cd .. && rm -f osm_plugin.zip && zip -r osm_plugin.zip osm_plugin -x \*.svn-base -x \*.pyc
