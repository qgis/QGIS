import shutil
import subprocess
import sys
from pathlib import Path
import tempfile


def test_processing_enums_are_rewritten():
    repo_root = Path(__file__).resolve().parents[3]  # .../QGIS
    script = repo_root / "scripts" / "pyqt5_to_pyqt6" / "pyqt5_to_pyqt6.py"
    fixture_dir = (
        repo_root
        / "scripts"
        / "pyqt5_to_pyqt6"
        / "tests"
        / "fixtures"
        / "repro"
    )

    assert script.exists(), f"Missing script: {script}"
    assert fixture_dir.exists(), f"Missing fixture dir: {fixture_dir}"

    with tempfile.TemporaryDirectory() as tmp:
        tmp_dir = Path(tmp)
        work_dir = tmp_dir / "repro"
        shutil.copytree(fixture_dir, work_dir)

        # run converter
        res = subprocess.run(
            [sys.executable, str(script), str(work_dir)],
            capture_output=True,
            text=True,
        )

        # the script should exit with 0 (all fixed) or 1 (some fixed, some unfixed)
        assert res.returncode in (0, 1), (
            f"Unexpected return code {res.returncode}\n"
            f"STDOUT:\n{res.stdout}\n"
            f"STDERR:\n{res.stderr}"
        )

        # check updated file
        updated = (work_dir / "repro.py").read_text(encoding="utf-8")

        # correct fixes applied
        assert "QgsProcessing.SourceType.TypeVectorLine" in updated
        assert "QgsProcessingParameterNumber.Type.Double" in updated
