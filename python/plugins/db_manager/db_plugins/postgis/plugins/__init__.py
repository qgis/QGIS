# -*- coding: utf-8 -*-

"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QGIS
Date                 : May 23, 2011
copyright            : (C) 2011 by Giuseppe Sucameli
email                : brush.tyler@gmail.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""

import os
current_dir = os.path.dirname(__file__)

def load(dbplugin, mainwindow):
	for name in os.listdir(current_dir):
		if not os.path.isdir( os.path.join( current_dir, name ) ):
			continue
		try:
			exec( u"from .%s import load" % name  )
		except ImportError, e:
			continue

		load(dbplugin, mainwindow)

