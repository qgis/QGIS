"""
QGIS algorithm utilities
"""

import inspect
from pathlib import Path

from qgis.core import Qgis


def mark_source(func):
    """
    Decorator that should be placed within each algorithm's implementation, decorating
    the function which best represents the source that reflects the internal
    logic of the algorithm.

    This is only needed if that function is not the processAlgorithm/processFeature
    function.
    """
    file_path = inspect.getsourcefile(func)
    line_no = func.__code__.co_firstlineno
    func._impl_source = (file_path, line_no)
    return func


def get_implementation_source_uri(self) -> str:
    """
    Automatically calculates the appropriate URI for an algorithm's
    implementationSourceUri function.
    """

    def _default_implementation_source() -> tuple[str, int]:
        nonlocal self

        # if explicitly marked source, use that
        cls = self.__class__
        # scan all attributes in the class for the mark_source decorator
        for attr in cls.__dict__.values():
            if hasattr(attr, "_impl_source"):
                return attr._impl_source

        # else just default to processFeature/processAlgorithm function, or class
        # itself if none
        target = cls.__dict__.get(
            "processFeature", cls.__dict__.get("processAlgorithm", cls)
        )
        return inspect.getsourcefile(target), inspect.getsourcelines(target)[1]

    file_path, line_no = _default_implementation_source()
    # strip local absolute paths down to relative QGIS repository path
    file_path = Path(file_path)
    for marker in ("python", "src"):
        if marker in file_path.parts:
            file_path = Path(*file_path.parts[file_path.parts.index(marker) :])
            break

    # determine git branch name from QGIS build version constants
    release_name = Qgis.QGIS_RELEASE_NAME
    if release_name.lower() == "master":
        branch = "master"
    else:
        version_int = getattr(Qgis, "QGIS_VERSION_INT", 0)
        major = version_int // 10000
        minor = (version_int % 10000) // 100
        branch = f"release-{major}_{minor}"

    return (
        f"https://github.com/qgis/QGIS/blob/{branch}/{file_path.as_posix()}#L{line_no}"
    )
