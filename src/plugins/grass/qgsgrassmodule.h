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

/**
 * \class QgsGrassModule
 *  \brief Interface to GRASS modules.
 *
 */
class QgsGrassModule : public QWidget, private  Ui::QgsGrassModuleBase
{
    Q_OBJECT

  public:
    class Description
    {
      public:
        QString label;
        // supported by GRASS Direct
        bool direct = true;
        Description() = default;
        Description( QString lab, bool dir = false ): label( lab ), direct( dir ) { }
        Description( const Description &desc ) { label = desc.label; direct = desc.direct; }
    };

    //! Constructor
    QgsGrassModule( QgsGrassTools *tools, QString moduleName, QgisInterface *iface,
                    bool direct, QWidget *parent = nullptr, Qt::WindowFlags f = nullptr );


    ~QgsGrassModule() override;

    QString translate( QString string );

    //! Returns module description (info from .qgs file) for module description path
    static Description description( QString path );

    //! Returns module label for module description path
    static QString label( QString path );

    /**
     * \brief Returns pixmap representing the module
     * \param path module path without .qgm extension
     */
    static QPixmap pixmap( QString path, int height );

    //! Returns pointer to QGIS interface
    QgisInterface *qgisIface();

    // ! Options widget
    QgsGrassModuleOptions *options() { return mOptions; }

    // ! Get executable + arguments. Executable is returned as first string.
    // On Window if the module is script the executable will be path to shell
    // Returns empty list if not found.
    static QStringList execArguments( QString module );

    //! Gets environment for process to start GRASS modules (set PATH)
    static QProcessEnvironment processEnvironment( bool direct );

    //! Returns true if module is direct
    bool isDirect() { return mDirect; }

    //! Gets name of library path environment variable
    static QString libraryPathVariable();

    //! Sets LD_LIBRARY_PATH or equivalent to GRASS Direct library
    static void setDirectLibraryPath( QProcessEnvironment &environment );

    QStringList errors() { return mErrors; }

  signals:
    // ! emitted when the module started
    void moduleStarted();

    // ! emitted when the module finished
    void moduleFinished();

  public slots:
    //! Run the module with current options
    void mRunButton_clicked() { run(); }
    void run();

    //! Close the module tab
    void mCloseButton_clicked() { close(); }
    void close();

    //! Show output in map view
    void mViewButton_clicked() { viewOutput(); }
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

    /**
     * Set progress bar or busy indicator if percent is 100
     * \param percent progress to show in %
     * \param force to set progress for 100% */
    void setProgress( int percent, bool force = false );

    //! Pointer to the QGIS interface object
    QgisInterface *mIface = nullptr;

    //! Pointer to canvas
    QgsMapCanvas *mCanvas = nullptr;

    //! Pointer to GRASS Tools
    QgsGrassTools *mTools = nullptr;

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
    QgsGrassModuleOptions *mOptions = nullptr;

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
