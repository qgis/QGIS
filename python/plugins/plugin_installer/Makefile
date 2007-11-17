INST_DIR = /usr/local/share/qgis/python/plugins/plugin_installer

PYRCC = /usr/bin/pyrcc4
PYUIC = /usr/bin/pyuic4

RC_PY_FILE = resources.py
UI_PY_FILE = gui.py

all: $(RC_PY_FILE) $(UI_PY_FILE)

install: all
	mkdir -p $(INST_DIR)
	cp *.py $(INST_DIR)/

clean:
	rm -f $(RC_PY_FILE) $(UI_PY_FILE)
	rm -f *.pyc

zip:
	cd /usr/local/share/qgis/python/plugins/ && zip -r ~/Desktop/plugin_installer.zip plugin_installer

$(RC_PY_FILE): resources.qrc
	$(PYRCC) -o $(RC_PY_FILE) resources.qrc

$(UI_PY_FILE): gui.ui
	$(PYUIC) -o $(UI_PY_FILE) gui.ui

