# ***************************************************************************
#     qgis_sip_project.py
#     ---------------------
#     Date                 : September 2021
#     Copyright            : (C) 2021 by Sandro Mani
#     Email                : manisandro at gmail dot com
# ***************************************************************************
# *                                                                         *
# *   This program is free software; you can redistribute it and/or modify  *
# *   it under the terms of the GNU General Public License as published by  *
# *   the Free Software Foundation; either version 2 of the License, or     *
# *   (at your option) any later version.                                   *
# *                                                                         *
# ***************************************************************************

import shutil
from pathlib import Path

from pyqtbuild import PyQtBindings, PyQtProject
from sipbuild import Bindings, Option


class QgisProject(PyQtProject):
    """
    A SIP project for building one part of the QGIS bindings.
    To be subclassed in a project.py file.
    """

    bindings_factories: list[type[Bindings]]  # To be set by subclass

    def __init__(self):
        """Initialize the project."""
        super().__init__()
        self.sip_files_dir = "."
        self.bindings_factories = type(self).bindings_factories

    def get_options(self):
        """Return the list of configurable options."""
        options = super().get_options()
        options.append(
            Option(
                "include_dirs",
                option_type=list,
                help="additional directory to search for .sip files",
                metavar="DIR",
            )
        )
        options.append(
            Option(
                "disable_features",
                option_type=list,
                help="features to disable project-wide",
                metavar="NAME",
            )
        )
        return options

    def apply_user_defaults(self, tool):
        """Set default values for user options that haven't been set yet."""
        super().apply_user_defaults(tool)
        if self.include_dirs is not None:
            self.sip_include_dirs += self.include_dirs

    def build(self):
        super().build()

        # Copy all files from the build directory to "out/", only overwriting
        # them if they've changed. This preserves the timestamp for unchanged
        # files, so ninja doesn't rebuild them.
        build_dir = Path(self.build_dir)
        out_dir = Path("out")
        out_dir.mkdir(exist_ok=True)
        changed = []
        for source in build_dir.glob("**/*"):
            if not source.is_file():
                continue
            rel = source.relative_to(build_dir)
            dest = out_dir / rel

            if dest.exists():
                # Check if the files match, otherwise copy
                source_data = source.read_bytes()
                if source_data != dest.read_bytes():
                    print("File changed:", rel)
                    dest.write_bytes(source_data)
                    changed.append(dest)
            else:
                # The file doesn't exist, create all directories and copy.
                dest.parent.mkdir(exist_ok=True, parents=True)
                shutil.copy(source, dest)
                changed.append(dest)

            # HACK: If a header changes, we need to rebuild all .cpp files that
            # include it as well. This is what deps files are for, and Ninja
            # should use them to find this out itself, but there's likely some
            # bug with restat=1. So to make sure we don't get broken builds, we
            # touch every .cpp file manually.
            if any(f.suffix == ".h" for f in changed):
                for f in out_dir.glob("**/*.cpp"):
                    f.touch()


class QgisBindings(PyQtBindings):
    """Bindings building one part of QGIS"""

    name: str  # To be set by subclass

    def __init__(self, project):
        """Initialize the bindings."""
        name = type(self).name
        super().__init__(project, name)
        self.sip_file = f"{name}.sip"
        self.exceptions = True
        self.release_gil = True

    def apply_user_defaults(self, tool):
        """Set default values for user options that haven't been set yet."""
        if self.project.py_platform.startswith("win32"):
            self.tags.append("WS_WIN")
        elif self.project.py_platform == "darwin":
            self.tags.append("WS_MACX")
        else:
            self.tags.append("WS_X11")

        if self.project.disable_features is not None:
            self.disabled_features = self.project.disable_features

        super().apply_user_defaults(tool)
