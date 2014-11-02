/***************************************************************************
                             main.cpp
                             Browser main method
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QLocale>
#include <QSettings>
#include <QTranslator>
#include <QMainWindow>
#include <QLabel>
#include <QDialog>
#include <QApplication>
#include "qgsbrowser.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsconfig.h"
#include <qmainwindow.h>

#include "qgseditorwidgetregistry.h"
#include "qgsclassificationwidgetwrapperfactory.h"
#include "qgsrangewidgetfactory.h"
#include "qgsuniquevaluewidgetfactory.h"
#include "qgsfilenamewidgetfactory.h"
#include "qgsvaluemapwidgetfactory.h"
#include "qgsenumerationwidgetfactory.h"
#include "qgshiddenwidgetfactory.h"
#include "qgscheckboxwidgetfactory.h"
#include "qgstexteditwidgetfactory.h"
#include "qgsvaluerelationwidgetfactory.h"
#include "qgsuuidwidgetfactory.h"
#include "qgsphotowidgetfactory.h"
#include "qgswebviewwidgetfactory.h"
#include "qgscolorwidgetfactory.h"
#include "qgsrelationreferencefactory.h"
#include "qgsdatetimeeditfactory.h"

int main( int argc, char ** argv )
{
  QSettings settings;

  QgsApplication a( argc, argv, true );
  // update any saved setting for older themes to new default 'gis' theme (2013-04-15)
  QString theme = settings.value( "/Themes", "default" ).toString();
  if ( theme == QString( "gis" )
       || theme == QString( "classic" )
       || theme == QString( "nkids" ) )
  {
    theme = QString( "default" );
  }
  a.setThemeName( theme );
  a.initQgis();

  // Set up the QSettings environment must be done after qapp is created
  QCoreApplication::setOrganizationName( "QGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QGIS2" );

  QgsBrowser w;

  a.connect( &a, SIGNAL( aboutToQuit() ), &w, SLOT( saveWindowState() ) );
  w.restoreWindowState();

  w.show();

  a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );

  QgsEditorWidgetRegistry* editorWidgetRegistry = QgsEditorWidgetRegistry::instance();
  editorWidgetRegistry->registerWidget( "Classification", new QgsClassificationWidgetWrapperFactory( QObject::tr( "Classification" ) ) );
  editorWidgetRegistry->registerWidget( "Range", new QgsRangeWidgetFactory( QObject::tr( "Range" ) ) );
  editorWidgetRegistry->registerWidget( "UniqueValues", new QgsUniqueValueWidgetFactory( QObject::tr( "Unique Values" ) ) );
  editorWidgetRegistry->registerWidget( "FileName", new QgsFileNameWidgetFactory( QObject::tr( "File Name" ) ) );
  editorWidgetRegistry->registerWidget( "ValueMap", new QgsValueMapWidgetFactory( QObject::tr( "Value Map" ) ) );
  editorWidgetRegistry->registerWidget( "Enumeration", new QgsEnumerationWidgetFactory( QObject::tr( "Enumeration" ) ) );
  editorWidgetRegistry->registerWidget( "Hidden", new QgsHiddenWidgetFactory( QObject::tr( "Hidden" ) ) );
  editorWidgetRegistry->registerWidget( "CheckBox", new QgsCheckboxWidgetFactory( QObject::tr( "Check Box" ) ) );
  editorWidgetRegistry->registerWidget( "TextEdit", new QgsTextEditWidgetFactory( QObject::tr( "Text Edit" ) ) );
  editorWidgetRegistry->registerWidget( "ValueRelation", new QgsValueRelationWidgetFactory( QObject::tr( "Value Relation" ) ) );
  editorWidgetRegistry->registerWidget( "UuidGenerator", new QgsUuidWidgetFactory( QObject::tr( "Uuid Generator" ) ) );
  editorWidgetRegistry->registerWidget( "Photo", new QgsPhotoWidgetFactory( QObject::tr( "Photo" ) ) );
  editorWidgetRegistry->registerWidget( "WebView", new QgsWebViewWidgetFactory( QObject::tr( "Web View" ) ) );
  editorWidgetRegistry->registerWidget( "Color", new QgsColorWidgetFactory( QObject::tr( "Color" ) ) );
  editorWidgetRegistry->registerWidget( "RelationReference", new QgsRelationReferenceFactory( QObject::tr( "Relation Reference" ), 0, 0 ) );
  editorWidgetRegistry->registerWidget( "DateTime", new QgsDateTimeEditFactory( QObject::tr( "Date/Time" ) ) );

  return a.exec();
}
