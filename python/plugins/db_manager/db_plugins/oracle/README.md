# Oracle implementation of QGis DBManager plugin

## Introduction

This Python code try to implement the Oracle part of the QGis DBManager plugin. DBManager plugin is a good tool from QGis with which you can easily manage your databases and create your own queries which can be dynamically added to QGis maps.

For the moment, DBManager plugin is only able to connect to PostGIS and Spatialite databases. If you want to manage your Oracle Spatial repository, you can (try) to do this with this code implementation.

The code base of this implementation was the Postgis one. I tried to make every features of the PostGIS work under Oracle but there are some limitations. Read TODO.md to have more details about what is working and what needs to be done.

Expect bugs !


## Installation

The code does not need [cx_Oracle](http://cx-oracle.sourceforge.net/) anymore ! Thanks to [Jürgen Fischer](https://github.com/jef-n), all Oracle connections uses PyQt QtSql module which is included in QGIS.

To install DBManager oracle plugin, you just have to clone the git repository in a directory named `oracle` in the `db_plugins` directory of the db_manager installation.

For MS-Windows users:

* If you have git for MS-Windows:
  $ cd "C:\Program Files\QGis Wien\apps\qgis\python\plugins\db_manager\db_plugins"
  $ git clone https://github.com/medspx/dbmanager-oracle.git oracle

* Or:
  * Just create a directory named `oracle` in "C:\Program Files\QGis Wien\apps\qgis\python\plugins\db_manager\db_plugins"
  * unzip `https://github.com/medspx/dbmanager-oracle/archive/master.zip` into "C:\Program Files\QGis Wien\apps\qgis\python\plugins\db_manager\db_plugins\oracle"

For GNU/Linux users:

  # cd /usr/share/qgis/python/plugins/db_manager/db_plugins
  # git clone https://github.com/medspx/dbmanager-oracle.git oracle


## Limitations

* You have to define Oracle connections directly in QGis for the plugin to work (same thing than PostGIS and Spatialite).
* Oracle Spatial Rasters are not supported (as I've don't have a way to test them).
* The code try to use the maximum of your Oracle connections parameters. If you have a huge geographic database with a lot of layers, listing tables can take time. So be careful about your connections parameters (try to restrict to user tables to reduce internal queries duration).
* Tests have been done with QGis 2.4, 2.6 and 2.8.2. You probably should use the latest version because before 2.4 the Oracle provider of QGis was not able to load dynamic queries.
* Some things could not have been well tested, particularly everything that requires administrative rights on DB like schema creation/deletion.
* Tests have been done against an Oracle 10g database. I tried to incorporate the official Oracle 12c "dictionary" of commands and the internal queries should also work with 11g and 12c versions of Oracle Database server.
* Some tasks cannot been done under Oracle Database like moving a table from a schema to another. There is also no PostgreSQL Rules features under Oracle.
* Code has been tested only under MS-Windows (bad) but as it is Python code, I hope it will also works under other OS.


## Bug reports

For the moment, use the ["issues" tool of GitHub](https://github.com/medspx/dbmanager-oracle/issues) to report bugs. 


## Main goal

My main goal is that this code can be incorporated in the official QGis source code repository. Once this has been done, the code upgrades will take place there.


## License

This code is released under the GNU GPLv2 license. Read headers code for more information.