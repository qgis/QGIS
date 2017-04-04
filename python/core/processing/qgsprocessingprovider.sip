/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingprovider.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsProcessingProvider : QObject
{
%Docstring
 Abstract base class for processing providers. An algorithm provider is a set of
 related algorithms, typically from the same external application or related
 to a common area of analysis.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingprovider.h"
%End

  public:

    QgsProcessingProvider( QObject* parent /TransferThis/ = 0 );
%Docstring
 Constructor for QgsProcessingProvider.
%End

    virtual ~QgsProcessingProvider();

//     QgsProcessingProvider &operator=( const QgsProcessingProvider &other ) = delete;

    virtual QIcon icon() const;
%Docstring
 Returns an icon for the provider.
 \see svgIconPath()
%End

    virtual QString svgIconPath() const;
%Docstring
 Returns a path to an SVG version of the provider's icon.
 \see icon()
%End

    virtual QString id() const = 0;
%Docstring
 Returns the unique provider id, used for identifying the provider. This string
 should be a unique, short, character only string, eg "qgis" or "gdal". This
 string should not be localised.
 \see name()
%End

    virtual QString name() const = 0;
%Docstring
 Returns the full provider name, which is used to describe the provider within the GUI.
 This string should be localised.
 \see id()
%End

    virtual bool canBeActivated() const;
%Docstring
 Returns true if the provider can be activated, or false if it cannot be activated (e.g. due to
 missing external dependencies).
%End

    virtual QStringList supportedOutputRasterLayerExtensions() const;
%Docstring
 Returns a list of the raster format file extensions supported by this provider.
 \see supportedOutputVectorLayerExtensions()
 \see supportedOutputTableExtensions()
%End

    virtual QStringList supportedOutputVectorLayerExtensions() const;
%Docstring
 Returns a list of the vector format file extensions supported by this provider.
 \see supportedOutputRasterLayerExtensions()
 \see supportedOutputTableExtensions()
 \see supportsNonFileBasedOutput()
%End

    virtual QStringList supportedOutputTableExtensions() const;
%Docstring
 Returns a list of the table format file extensions supported by this provider.
 \see supportedOutputRasterLayerExtensions()
 \see supportedOutputVectorLayerExtensions()
%End

    virtual bool supportsNonFileBasedOutput() const;
%Docstring
 Returns true if the provider supports non-file based outputs (such as memory layers
 or direct database outputs).
 \see supportedOutputVectorLayerExtensions()
%End

    virtual bool load();
%Docstring
 Loads the provider. This will be called when the plugin is being loaded, and any general
 setup actions should occur in an overridden version of this method.
 Subclasses should not load any algorithms in their load() implementations, as that must
 occur within the loadAlgorithms() method.
 \returns true if provider could be successfully loaded
 \see unload()
%End

    virtual void unload();
%Docstring
 Unloads the provider. Any tear-down steps required by the provider should be implemented here.
 \see load()
%End

    void refreshAlgorithms();
%Docstring
 Refreshes the algorithms available from the provider, causing it to re-populate with all associated algorithms.
%End

    QList< QgsProcessingAlgorithm * > algorithms() const;
%Docstring
 Returns a list of algorithms supplied by this provider.
 \see algorithm()
%End

    QgsProcessingAlgorithm *algorithm( const QString &name ) const;
%Docstring
 Returns the matching algorithm by name, or a nullptr if no matching
 algorithm is contained by this provider.
 \see algorithms()
%End

  signals:

    void algorithmsLoaded();
%Docstring
 Emitted when the provider has loaded (or refreshed) its list of available
 algorithms.
 \see refreshAlgorithms()
%End

  protected:

    virtual void loadAlgorithms() = 0;
%Docstring
 Loads all algorithms belonging to this provider. Subclasses should implement this, calling
 addAlgorithm() to register all their associated algorithms.
%End

    bool addAlgorithm( QgsProcessingAlgorithm *algorithm /Transfer/ );
%Docstring
 Adds an algorithm to the provider. Ownership of the algorithm is transferred to the provider.
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
