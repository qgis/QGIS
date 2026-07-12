import json
import os
import sys

import console
import qgis.core
import qgis.gui
from qgis.PyQt import Qsci

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

# The AppImage smoke test only needs to prove that the bundled PyQGIS runtime
# imports successfully. Avoid Qt/QGIS teardown in CI, which can abort after the
# success marker has already been written.
os._exit(0)
