/***************************************************************************
    heatmap.h
    -------------------
    begin                : Jan 21, 2004
    copyright            : (C) 2004 by Tim Sutton
    email                : tim@linfiniti.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/***************************************************************************
 *   QGIS Programming conventions:
 *
 *   mVariableName - a class level member variable
 *   sVariableName - a static class level member variable
 *   variableName() - accessor for a class member (no 'get' in front of name)
 *   setVariableName() - mutator for a class member (prefix with 'set')
 *
 *   Additional useful conventions:
 *
 *   theVariableName - a method parameter (prefix with 'the')
 *   myVariableName - a locally declared variable within a method ('my' prefix)
 *
 *   DO: Use mixed case variable names - myVariableName
 *   DON'T: separate variable names using underscores: my_variable_name (NO!)
 *
 * **************************************************************************/
#ifndef Heatmap_H
#define Heatmap_H

//QT4 includes
#include <QObject>

//QGIS includes
#include "../qgisplugin.h"
#include "qgsvectorlayer.h"
#include "qgscoordinatereferencesystem.h"

//forward declarations
class QAction;
class QToolBar;

class QgisInterface;

/**
* \class Plugin
* \brief heatmap plugin for QGIS
* \description generates a heatmap raster for the input point vector
*/
class Heatmap: public QObject, public QgisPlugin
{
    Q_OBJECT
  public:

    //                MANDATORY PLUGIN METHODS FOLLOW

    /**
    * Constructor for a plugin. The QgisInterface pointer is passed by
    * QGIS when it attempts to instantiate the plugin.
    * @param theInterface Pointer to the QgisInterface object.
     */
    Heatmap( QgisInterface * theInterface );
    //! Destructor
    virtual ~Heatmap();

    // Kernel shape type
    enum KernelShape
    {
      Quartic,
      Triangular,
      Uniform,
      Triweight,
      Epanechnikov
    };

    // Output values type
    enum OutputValues
    {
      Raw,
      Scaled
    };

    QMap<QString, QVariant> mSessionSettings;

  public slots:
    //! init the gui
    virtual void initGui() override;
    //! Show the dialog box
    void run();
    //! unload the plugin
    void unload() override;
    //! show the help document
    void help();

  private:
    double mDecay;

    //! Worker to convert meters to map units
    double mapUnitsOf( double meters, QgsCoordinateReferenceSystem layerCrs );
    //! Worker to calculate buffer size in pixels
    int bufferSize( double radius, double cellsize );
    //! Calculate the value given to a point width a given distance for a specified kernel shape
    double calculateKernelValue( const double distance, const int bandwidth, const KernelShape shape, const OutputValues outputType );
    //! Uniform kernel function
    double uniformKernel( const double distance, const int bandwidth, const OutputValues outputType ) const;
    //! Quartic kernel function
    double quarticKernel( const double distance, const int bandwidth, const OutputValues outputType ) const;
    //! Triweight kernel function
    double triweightKernel( const double distance, const int bandwidth, const OutputValues outputType ) const;
    //! Epanechnikov kernel function
    double epanechnikovKernel( const double distance, const int bandwidth, const OutputValues outputType ) const;
    //! Triangular kernel function
    double triangularKernel( const double distance, const int bandwidth, const OutputValues outputType ) const;

    // MANDATORY PLUGIN PROPERTY DECLARATIONS  .....

    //! Pointer to the QGIS interface object
    QgisInterface *mQGisIface;
    //!pointer to the qaction for this plugin
    QAction * mQActionPointer;

};

#endif //Heatmap_H
