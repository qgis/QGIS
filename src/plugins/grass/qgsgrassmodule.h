/***************************************************************************
                              qgsgrassmodule.h
                             -------------------
    begin                : March, 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGRASSMODULE_H
#define QGSGRASSMODULE_H

#include "ui_qgsgrassmodulebase.h"

#include "qgis.h"

#include <QProcess>

#include "qgsgrassmoduleoptions.h"

class QDomNode;
class QDomElement;

/** \class QgsGrassModule
 *  \brief Interface to GRASS modules.
 *
 */
class QgsGrassModule : public QDialog, private  Ui::QgsGrassModuleBase
{
    Q_OBJECT

  public:
    class Description
    {
      public:
        QString label;
        // supported by GRASS Direct
        bool direct;
        Description(): direct( true ) {}
        Description( QString lab, bool dir = false ): label( lab ), direct( dir ) { }
        Description( const Description & desc ) { label = desc.label; direct =  desc.direct; }
    };

    //! Constructor
    QgsGrassModule( QgsGrassTools *tools, QString moduleName, QgisInterface *iface,
                    bool direct, QWidget *parent = 0, Qt::WindowFlags f = 0 );

    //! Destructor
    ~QgsGrassModule();

    QString translate( QString string );

    //! Returns module description (info from .qgs file) for module description path
    static Description description( QString path );

    //! Returns module label for module description path
    static QString label( QString path );

    /** \brief Returns pixmap representing the module
     * \param path module path without .qgm extension
     */
    static QPixmap pixmap( QString path, int height );

    //! Find element in GRASS module description by key, if not found, returned element is null
    static QDomNode nodeByKey( QDomElement gDocElem, QString key );

    //! Returns pointer to QGIS interface
    QgisInterface *qgisIface();

    // ! Options widget
    QgsGrassModuleOptions *options() { return mOptions; }

    // ! List of directories in PATH variable + current directory on Windows
    //static QStringList mExecPath;
    //static bool mExecPathInited;

    // ! Find in exec path
    //   returns full path or null string
    //   appends automaticaly .exe on Windows
    static QString findExec( QString file );

    // ! Check if file is in mExecPath
    static bool inExecPath( QString file );

    // ! Get executable + arguments. Executable is returned as first string.
    // On Window if the module is script the executable will be path to shell
    // Returns empty list if not found.
    static QStringList execArguments( QString module );

    //! Get environment for process to start GRASS modules (set PATH)
    static QProcessEnvironment processEnvironment( bool direct );

    //! Returns true if module is direct
    bool isDirect() { return mDirect; }

    //! Get name of library path environment variable
    static QString libraryPathVariable();

    //! Set LD_LIBRARY_PATH or equivalent to GRASS Direct library
    static void setDirectLibraryPath( QProcessEnvironment & environment );

    QStringList errors() { return mErrors; }

  signals:
    // ! emitted when the module started
    void moduleStarted();

    // ! emitted when the module finished
    void moduleFinished();

  public slots:
    //! Run the module with current options
    void on_mRunButton_clicked() { run(); }
    void run();

    //! Close the module tab
    void on_mCloseButton_clicked() { close(); }
    void close();

    //! Show output in map view
    void on_mViewButton_clicked() { viewOutput(); }
    void viewOutput();

    //! Running process finished
    void finished( int exitCode, QProcess::ExitStatus exitStatus );

    //! Read module's standard output
    void readStdout();

    //! Read module's standard error
    void readStderr();

    //! Call on mapset change, i.e. also possible direct/indirect mode change
    //void mapsetChanged();

  private:
    //! Pointer to the QGIS interface object
    QgisInterface *mIface;

    //! Pointer to canvas
    QgsMapCanvas *mCanvas;

    //! Pointer to GRASS Tools
    QgsGrassTools *mTools;

    //! Module definition file path (.qgm file)
    //QString mPath;

    //! Name of module executable
    QString mXName;

    //! Path to module executable
    QString mXPath;

    //! Parent widget
    //QWidget *mParent;

    //! Running GRASS module
    QProcess mProcess;

    //! QGIS directory
    QString mAppDir;

    //! Pointer to options widget
    QgsGrassModuleOptions *mOptions;

    //! Last raster output
    QStringList mOutputRaster;

    //! Last vector output
    QStringList mOutputVector;

    //! True if the module successfully finished
    bool mSuccess;

    //! Direct mode
    bool mDirect;

    //! Error message
    QStringList mErrors;

    //! Debug message, detailed error
    //QStringList mDebugOutput;
};

#endif // QGSGRASSMODULE_H
