# -*- coding: utf-8 -*-

"""
/***************************************************************************
Name                 : TopoViewer plugin for DB Manager 
Description          : Create a project to display topology schema on QGis
Date                 : Sep 23, 2011
copyright            : (C) 2011 by Giuseppe Sucameli
email                : brush.tyler@gmail.com

Based on qgis_pgis_topoview by Sandro Santilli <strk@keybit.net>
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

from PyQt4.QtCore import *
from PyQt4.QtGui import *

import os
current_path = os.path.dirname(__file__)


# The load function is called when the "db" database or either one of its 
# children db objects (table o schema) is selected by the user.
# @param db is the selected database
# @param mainwindow is the DBManager mainwindow
def load(db, mainwindow):
	# check whether the selected database has topology enabled
	# (search for topology.topology)
	sql = u"""SELECT count(*) 
		FROM pg_class AS cls JOIN pg_namespace AS nsp ON nsp.oid = cls.relnamespace
		WHERE cls.relname = 'topology' AND nsp.nspname = 'topology'"""
	c = db.connector._get_cursor()	
	db.connector._execute( c, sql )
	res = db.connector._fetchone( c )
	if res == None or int(res[0]) <= 0:
		return

	# add the action to the DBManager menu
	action = QAction( QIcon(), "&TopoViewer", db )
	mainwindow.registerAction( action, "&Schema", run )


# The run function is called once the user clicks on the action TopoViewer 
# (look above at the load function) from the DBManager menu/toolbar.
# @param item is the selected db item (either db, schema or table)
# @param action is the clicked action on the DBManager menu/toolbar
# @param mainwindow is the DBManager mainwindow
def run(item, action, mainwindow):
	db = item.database()
	uri = db.uri()
	conninfo = uri.connectionInfo()

	# check if the selected item is a topology schema
	isTopoSchema = False
	if hasattr(item, 'schema') and item.schema() != None:
		sql = u"SELECT count(*) FROM topology.topology WHERE name = '%s'" % item.schema().name
		c = db.connector._get_cursor()	
		db.connector._execute( c, sql )
		res = db.connector._fetchone( c )
		isTopoSchema = res != None and int(res[0]) > 0

	if not isTopoSchema:
		QMessageBox.critical(mainwindow, "Invalid topology", u'Schema "%s" is not registered in topology.topology.' % item.schema().name)
		return False

	# create the new project from the template one
	tpl_name = u'topoview_template.qgs'
	toponame = item.schema().name
	project_name = u'topoview_%s_%s.qgs' % (uri.database(), toponame)

	template_file = os.path.join(current_path, tpl_name)
	inf = QFile( template_file )
	if not inf.exists():
		QMessageBox.critical(mainwindow, "Error", u'Template "%s" not found!' % template_file)
		return False

	project_file = os.path.join(current_path, project_name)
	outf = QFile( project_file )
	if not outf.open( QIODevice.WriteOnly ):
		QMessageBox.critical(mainwindow, "Error", u'Unable to open "%s"' % project_file)
		return False

	if not inf.open( QIODevice.ReadOnly ):
		QMessageBox.critical(mainwindow, "Error", u'Unable to open "%s"' % template_file)
		return False

	while not inf.atEnd():
		l = inf.readLine()
		l = l.replace( u"dbname='@@DBNAME@@'", conninfo.toUtf8() )
		l = l.replace( u'@@TOPONAME@@', toponame )
		outf.write( l )

	inf.close()
	outf.close()

	# load the project on QGis canvas
	iface = mainwindow.iface
	iface.newProject( True )
	if iface.mapCanvas().layerCount() == 0:
		iface.addProject( project_file )
	return True

