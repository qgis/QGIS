# /***************************************************************************
#  *                                                                         *
#  *   This program is free software; you can redistribute it and/or modify  *
#  *   it under the terms of the GNU General Public License as published by  *
#  *   the Free Software Foundation; either version 2 of the License, or     *
#  *   (at your option) any later version.                                   *
#  *                                                                         *
#  ***************************************************************************/

"""
GUI Utilities
"""

import os
from typing import Optional, Dict

from qgis.PyQt.QtGui import (
    QIcon,
    QImage,
    QPixmap
)


class GuiUtils:
    """
    Utilities for GUI plugin components
    """

    ICON_CACHE: Dict[str, QIcon] = {}

    @staticmethod
    def get_icon(icon: str) -> QIcon:
        """
        Returns a plugin icon
        :param icon: icon name (base part of file name)
        :return: QIcon
        """
        if icon in GuiUtils.ICON_CACHE:
            return GuiUtils.ICON_CACHE[icon]

        # prefer SVG files if present
        path = GuiUtils.get_icon_svg(icon)
        if path:
            res = QIcon(path)
            GuiUtils.ICON_CACHE[icon] = res
            return res

        pixmap = GuiUtils.get_icon_as_pixmap(icon)
        if pixmap is not None:
            res = QIcon(pixmap)
            GuiUtils.ICON_CACHE[icon] = res
            return res

        # return an invalid icon
        GuiUtils.ICON_CACHE[icon] = QIcon()
        return QIcon()

    @staticmethod
    def get_icon_svg(icon: str) -> str:
        """
        Returns a plugin icon's SVG file path
        :param icon: icon name (base part of file name)
        :return: icon svg path
        """
        path = os.path.join(
            os.path.dirname(__file__),
            'icons',
            icon + '.svg')
        if not os.path.exists(path):
            return ''

        return path

    @staticmethod
    def get_pixmap_path(icon: str) -> Optional[str]:
        """
        Returns the path to a pixmap icon
        """
        for suffix in ('.png', '.gif', '.xpm'):
            path = os.path.join(
                os.path.dirname(__file__),
                'icons',
                icon + suffix)
            if os.path.exists(path):
                return path

        return None

    @staticmethod
    def get_icon_as_pixmap(icon: str) -> Optional[QPixmap]:
        """
        Returns a plugin icon's PNG file path
        :param icon: icon name (png file name)
        :return: icon png path
        """
        path = GuiUtils.get_pixmap_path(icon)
        if path is not None:
            im = QImage(path)
            return QPixmap.fromImage(im)

        return None

    @staticmethod
    def get_ui_file_path(file: str) -> str:
        """
        Returns a UI file's path
        :param file: file name (uifile name)
        :return: ui file path
        """
        path = os.path.join(
            os.path.dirname(__file__),
            'ui',
            file)
        if not os.path.exists(path):
            return ''

        return path
