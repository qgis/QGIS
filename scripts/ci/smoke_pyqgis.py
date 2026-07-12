import json
import os
import sys

from qgis.PyQt.QtCore import QTimer
from qgis.PyQt.QtWidgets import QApplication

import qgis.core
import qgis.gui
from qgis.PyQt import Qsci
import console


out_path = os.environ.get("STRATA_PYQGIS_SMOKE_OUT")
if out_path:
    with open(out_path, "w", encoding="utf-8") as out_file:
        json.dump(
            {
                "ok": True,
                "python": sys.version.split()[0],
                "qgis_core": qgis.core.Qgis.version(),
                "qsci": bool(Qsci),
                "console": bool(console),
            },
            out_file,
            sort_keys=True,
        )

app = QApplication.instance()
if app is not None:
    QTimer.singleShot(0, app.quit)
