/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "[pluginlcasename]gui.h"
#include "qgscontexthelp.h"

//qt includes

//standard includes

[pluginname]Gui::[pluginname]Gui( QWidget* parent, Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );

  // Below is an example of how to make the translators job
  // much easier. Please follow this general guideline for LARGE
  // pieces of text. One-liners can be added in the .ui file

  // Note: Format does not relate to translation.
  QString format( "<html><body><h3>%1</h3>%2<h3>%3</h3>%4<p><a href=http://qgis.org/api>"
                  "http://qgis.org/api</a><p>"
                  "%5<p>%6<p>%7<p>%8<p><h3>%9</h3>"
                  "<h4>CMakeLists.txt</h4>%11"
                  "<h4>[pluginlcasename].h, [pluginlcasename].cpp</h4>%12"
                  "<h4>[pluginlcasename]gui.ui</h4>%13"
                  "<h4>[pluginlcasename]gui.cpp, [pluginlcasename]gui.h</h4>%14"
                  "<h4>[pluginlcasename].qrc</h4>%15"
                  "<h4>[pluginlcasename].png</h4>%16"
                  "<h4>README</h4>%17"
                  "<h3>%18</h3>%19<ul>%20</ul>%21<p>%22"
                  "<p><b>The QGIS Team</b>"
                  "</body></html>" );

  // Note: Table does not translate
  QString table( "<table><tr><td>QGisInterface<td><a href=http://qgis.org/api/classQgisInterface.html>"
                 "http://qgis.org/api/classQgisInterface.html</a>"
                 "<tr><td>QgsMapCanvas<td><a href=http://qgis.org/api/classQgsMapCanvas.html>"
                 "http://qgis.org/api/classQgsMapCanvas.html</a>"
                 "<tr><td>QgsMapTool<td><a href=http://qgis.org/api/classQgsMapTool.html>"
                 "http://qgis.org/api/classQgsMapTool.html</a>"
                 "<tr><td>QgsPlugin<td><a href=http://qgis.org/api/classQgisPlugin.html>"
                 "http://qgis.org/api/classQgisPlugin.html</a></table>" );

  // Note: Translatable strings below
  QString text = format
                 .arg( tr( "Welcome to your automatically generated plugin!" ) )
                 .arg( tr( "This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started." ) )
                 .arg( tr( "Documentation:" ) )
                 .arg( tr( "You really need to read the QGIS API Documentation now at:" ) )
                 .arg( tr( "In particular look at the following classes:" ) )
                 .arg( table )
                 .arg( "QGisInterface is an abstract base class (ABC) that specifies what publicly available features of QGIS are exposed to third party code and plugins. An instance of the QgisInterface is passed to the plugin when it loads. Please consult the QGIS development team if there is functionality required in the QGisInterface that is not available." )
                 .arg( tr( "QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details." ) )
                 .arg( tr( "What are all the files in my generated plugin directory for?" ) )
                 .arg( tr( "This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file." ) )
                 .arg( tr( "This is the class that provides the 'glue' between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice." ) )
                 .arg( tr( "This is a Qt designer 'ui' file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools)." ) )
                 .arg( tr( "This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really...." ) )
                 .arg( tr( "This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (':/Homann/'). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too." ) )
                 .arg( tr( "This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest." ) )
                 .arg( tr( "This file contains the documentation you are reading now!" ) )
                 .arg( tr( "Getting developer help:" ) )
                 .arg( tr( "For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:" ) )
                 .arg( tr( "<li> the QGIS developers mailing list, or </li><li> IRC (#qgis on freenode.net)</li>" ) )
                 .arg( tr( "QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community." ) )
                 .arg( tr( "Have fun and thank you for choosing QGIS." ) );

  textBrowser->setHtml( text );
}

[pluginname]Gui::~[pluginname]Gui()
{
}

void [pluginname]Gui::on_buttonBox_accepted()
{
  //close the dialog
  accept();
}

void [pluginname]Gui::on_buttonBox_rejected()
{
  reject();
}

void [pluginname]Gui::on_buttonBox_helpRequested()
{
  QgsContextHelp::run( context_id );
}

