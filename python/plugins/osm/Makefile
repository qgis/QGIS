
GEN_FILES = OsmLoadDlg_ui.py OsmSaveDlg_ui.py OsmDownloadDlg_ui.py OsmUploadDlg_ui.py OsmFeatureDW_ui.py OsmAddRelationDlg_ui.py OsmUndoRedoDW_ui.py OsmImportDlg_ui.py resources.py

all: $(GEN_FILES)

OsmLoadDlg_ui.py: ui_files/OsmLoadDlg.ui
	pyuic4 -o OsmLoadDlg_ui.py ui_files/OsmLoadDlg.ui

OsmSaveDlg_ui.py: ui_files/OsmSaveDlg.ui
	pyuic4 -o OsmSaveDlg_ui.py ui_files/OsmSaveDlg.ui

OsmDownloadDlg_ui.py: ui_files/OsmDownloadDlg.ui
	pyuic4 -o OsmDownloadDlg_ui.py ui_files/OsmDownloadDlg.ui

OsmUploadDlg_ui.py: ui_files/OsmUploadDlg.ui
	pyuic4 -o OsmUploadDlg_ui.py ui_files/OsmUploadDlg.ui

OsmFeatureDW_ui.py: ui_files/OsmFeatureDW.ui
	pyuic4 -o OsmFeatureDW_ui.py ui_files/OsmFeatureDW.ui

OsmAddRelationDlg_ui.py: ui_files/OsmAddRelationDlg.ui
	pyuic4 -o OsmAddRelationDlg_ui.py ui_files/OsmAddRelationDlg.ui

OsmUndoRedoDW_ui.py: ui_files/OsmUndoRedoDW.ui
	pyuic4 -o OsmUndoRedoDW_ui.py ui_files/OsmUndoRedoDW.ui

OsmImportDlg_ui.py: ui_files/OsmImportDlg.ui
	pyuic4 -o OsmImportDlg_ui.py ui_files/OsmImportDlg.ui

resources.py: resources.qrc
	pyrcc4 -o resources.py resources.qrc

clean:
	rm -f $(GEN_FILES) *.pyc

package:
	cd .. && rm -f osm_plugin.zip && zip -r osm_plugin.zip osm_plugin -x \*.svn-base -x \*.pyc
