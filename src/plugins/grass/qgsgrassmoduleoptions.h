/***************************************************************************
                          qgsgrassmoduleoptions.h
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
#ifndef QGSGRASSMODULEOPTIONS_H
#define QGSGRASSMODULEOPTIONS_H

//#include <QCheckBox>
//#include <QComboBox>
//#include <QGroupBox>
//#include <QLineEdit>
//#include <QPushButton>
//#include <QVBoxLayout>

#include "qgis.h"
#include "qgsfields.h"
#include "qgscoordinatereferencesystem.h"

#include "qgsgrassmoduleparam.h"
#include "qgsgrassprovider.h"
#include "qgsgrassrasterprovider.h"

class QgsGrassTools;
class QgsGrassModule;

class QgisInterface;
class QgsMapCanvas;

/**
 * \class QgsGrassModuleOptions
 *  \brief Widget with GRASS options.QgsGrassTools
 *
 */
class QgsGrassModuleOptions
{
  public:
    enum RegionMode
    {
      RegionInput = 1,  // intersection of input maps extent and highest input resolution
      RegionCurrent = 0 // current map canvas extent and resolution
    };

    //! Constructor
    QgsGrassModuleOptions(
      QgsGrassTools *tools, QgsGrassModule *module,
      QgisInterface *iface, bool direct );

    virtual ~QgsGrassModuleOptions() = default;

    //! Gets module options as list of arguments for QProcess
    virtual QStringList arguments();

    //! Check if output exists
    // return empty list
    // return list of existing output maps
    virtual QStringList checkOutput() { return QStringList() ; }

    //! Freeze output maps used in QGIS
    // freeze / thaw output layers
    virtual void freezeOutput( bool freeze = true ) { Q_UNUSED( freeze ) }

    //! Check if option is ready
    //  Returns empty string or error message
    virtual QStringList ready() { return QStringList() ; }

    //! Gets list of current output maps
    virtual QStringList output( int type )
    { Q_UNUSED( type ) return QStringList(); }

    //! Has any output
    virtual bool hasOutput( int type )
    { Q_UNUSED( type ) return true; }

    //! Has raster input or output
    virtual bool usesRegion() { return false; }

    //! One or more input maps were switched on to be used as region
    virtual bool requestsRegion() { return false; }

    //! Check region
    // return empty list
    // return list of input maps (both raster and vector) outside region
    virtual QStringList checkRegion() { return QStringList() ; }

    //! Gets region covering all input maps
    // \param all true all input maps
    // \param all false only the mas which were switched on
    virtual bool inputRegion( struct Cell_head *window, QgsCoordinateReferenceSystem &crs, bool all )
    { Q_UNUSED( window ) Q_UNUSED( crs ); Q_UNUSED( all ); return false; }

    //! Flag names
    virtual QStringList flagNames() { return QStringList() ; }

    QStringList errors() const { return mErrors; }

  protected:
    //! Pointer to the QGIS interface object
    QgisInterface *mIface = nullptr;

    //! Pointer to canvas
    QgsMapCanvas *mCanvas = nullptr;

    //! Pointer to GRASS Tools
    QgsGrassTools *mTools = nullptr;

    //! Pointer to GRASS module
    QgsGrassModule *mModule = nullptr;

    //! Parent widget
    //QWidget *mParent;

    //! QGIS directory
    QString mAppDir;

    //! Region mode select box
    QComboBox *mRegionModeComboBox = nullptr;

    //! Direct mode
    bool mDirect;

    //! Error messages
    QStringList mErrors;
};

/**
 * \class QgsGrassModuleStandardOptions
 *  \brief Widget with GRASS standard options.
 *
 */
class QgsGrassModuleStandardOptions: public QWidget, public QgsGrassModuleOptions
{
    Q_OBJECT

  public:
    //! Constructor
    QgsGrassModuleStandardOptions(
      QgsGrassTools *tools, QgsGrassModule *module,
      QgisInterface *iface,
      QString xname, QDomElement confDocElem,
      bool direct, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    //! Gets module options as list of arguments for QProcess
    QStringList arguments() override;

    //! Get item by ID
    QgsGrassModuleParam *item( QString id );

    //! Get item by key
    QgsGrassModuleParam *itemByKey( QString key );

    // Reimplemented methods from QgsGrassModuleOptions
    QStringList checkOutput() override;
    // freeze / thaw output layers
    void freezeOutput( bool freeze = true ) override;
    QStringList ready() override;
    QStringList output( int type ) override;
    bool hasOutput( int type ) override;
    QStringList checkRegion() override;
    bool usesRegion() override;
    bool requestsRegion() override;
    bool inputRegion( struct Cell_head *window, QgsCoordinateReferenceSystem &crs, bool all ) override;
    QStringList flagNames() override { return mFlagNames; }

  public slots:
    //! Show/hide advanced options
    void switchAdvanced();

  private:

    /**
     * Read and parse module options (--interface-description).
     * \param errors list to which possible errors are added
     */
    QDomDocument readInterfaceDescription( const QString &xname, QStringList &errors );

    /**
     * Gets region for currently selected map. It will show warning dialog if region could not be read.
     * \returns TRUE if region was successfully read
     */
    bool getCurrentMapRegion( QgsGrassModuleInput *param, struct Cell_head *window );

    // List of providers used by layers in QgsProject
    QList<QgsGrassProvider *> grassProviders();
    QList<QgsGrassRasterProvider *> grassRasterProviders();

    //! Name of module executable
    QString mXName;

    //! Path to module executable
    QString mXPath;

    //! Option items
    QList<QgsGrassModuleParam *> mParams;

    //! List of all flags. Necessary for scripts.
    QStringList mFlagNames;

    //! Advanced options switch button
    QPushButton mAdvancedPushButton;

    //! Advanced options frame
    QFrame mAdvancedFrame;
};

#endif // QGSGRASSMODULEOPTIONS_H
