/***************************************************************************
                             main.cpp
                             --------
    begin                : 2026-07-14
    copyright            : (C) 2026 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsconfig.h"

#include "qgshtmltopdf.h"

#include <QCommandLineParser>
#include <QGuiApplication>
#include <QString>
#include <QSurfaceFormat>
#include <QTimer>

using namespace Qt::StringLiterals;

#ifdef Q_OS_WIN
#include <fcntl.h> /*  _O_BINARY */
#else
#include <getopt.h>
#endif

#ifdef Q_OS_MACOS
#include <ApplicationServices/ApplicationServices.h>
#if MAC_OS_X_VERSION_MAX_ALLOWED < 1050
typedef SInt32 SRefCon;
#endif
#endif

/////////////////////////////////////////////////////////////////
// Command line options 'behavior' flag setup
////////////////////////////////////////////////////////////////

int main( int argc, char *argv[] )
{
#ifdef Q_OS_WIN // Windows
#ifdef _MSC_VER
  _set_fmode( _O_BINARY );
#else  //MinGW
  _fmode = _O_BINARY;
#endif // _MSC_VER
#endif // Q_OS_WIN

  // Initialize the default surface format for all
  // QWindow and QWindow derived components
#if !defined( QT_NO_OPENGL )
  QSurfaceFormat format;
  format.setRenderableType( QSurfaceFormat::OpenGL );
#ifdef Q_OS_MACOS
  format.setVersion( 4, 1 ); //OpenGL is deprecated on MacOS, use last supported version
  format.setProfile( QSurfaceFormat::CoreProfile );
#else
  format.setVersion( 4, 3 );
  format.setProfile( QSurfaceFormat::CompatibilityProfile ); // Chromium only supports core profile on mac
#endif
  format.setDepthBufferSize( 24 );
  format.setSamples( 4 );
  format.setStencilBufferSize( 8 );
  QSurfaceFormat::setDefaultFormat( format );
#endif

#if !defined( QT_NO_OPENGL )
  QCoreApplication::setAttribute( Qt::AA_ShareOpenGLContexts, true );
#endif

  QGuiApplication app( argc, argv );
  QGuiApplication::setOrganizationName( u"QGIS"_s );
  QGuiApplication::setOrganizationDomain( u"qgis.org"_s );
  QGuiApplication::setApplicationName( u"QGIS4"_s );
  QGuiApplication::setApplicationVersion( u"%1 (%2)"_s.arg( VERSION ).arg( RELEASE_NAME ) );

  QCommandLineParser parser;
  parser.setApplicationDescription( "QGIS HTML to PDF converter utility.\n\nUsed to convert HTML web pages to a single-page PDF file, for rendering within the main QGIS application." );
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument( "url", "Source HTML URL." );
  parser.addPositionalArgument( "destination", "Destination PDF file." );

  parser.process( app );

  const QStringList args = parser.positionalArguments();
  if ( args.size() < 2 )
    return 1;

  const QString url = args.at( 0 );
  const QString destination = args.at( 1 );

  QgsHtmlToPdf converter;
  QObject::connect( &converter, &QgsHtmlToPdf::finished, &app, &QGuiApplication::exit );
  QTimer::singleShot( 0, &converter, [&converter, url, destination] { converter.run( url, destination ); } );

  return QCoreApplication::exec();
}
