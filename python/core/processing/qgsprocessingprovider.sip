/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingprovider.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsProcessingProvider
{
%Docstring
 Abstract base class for processing providers. An algorithm provider is a set of
 related algorithms, typically from the same external application or related
 to a common area of analysis.
 \since QGIS 3.0
%End

%TypeHeaderCode
#include "qgsprocessingprovider.h"
%End

  public:

    QgsProcessingProvider();
%Docstring
 Constructor for QgsProcessingProvider.
%End

    virtual ~QgsProcessingProvider();

//     QgsProcessingProvider &operator=( const QgsProcessingProvider &other ) = delete;

    virtual QIcon icon() const;
%Docstring
 Returns an icon for the provider.
 @see svgIconPath()
%End

    virtual QString svgIconPath() const;
%Docstring
 Returns a path to an SVG version of the provider's icon.
 @see icon()
%End

    virtual QString id() const = 0;
%Docstring
 Returns the unique provider id, used for identifying the provider. This string
 should be a unique, short, character only string, eg "qgis" or "gdal". This
 string should not be localised.
 @see name()
%End

    virtual QString name() const = 0;
%Docstring
 Returns the full provider name, which is used to describe the provider within the GUI.
 This string should be localised.
 @see id()
%End

    virtual bool canBeActivated() const;
%Docstring
 Returns true if the provider can be activated, or false if it cannot be activated (e.g. due to
 missing external dependencies).
%End

  private:
    QgsProcessingProvider( const QgsProcessingProvider &other );
};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingprovider.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
