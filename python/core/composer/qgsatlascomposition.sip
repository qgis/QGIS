/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgsatlascomposition.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsAtlasComposition : QObject
{
%Docstring
 Class used to render an Atlas, iterating over geometry features.
 prepareForFeature() modifies the atlas map's extent to zoom on the given feature.
 This class is used for printing, exporting to PDF and images.
.. note::

   This class should not be created directly. For the atlas to function correctly
 the atlasComposition() property for QgsComposition should be used to retrieve a
 QgsAtlasComposition which is automatically created and attached to the composition.
%End

%TypeHeaderCode
#include "qgsatlascomposition.h"
%End
  public:
    QgsAtlasComposition( QgsComposition *composition );

    bool enabled() const;
%Docstring
 Returns whether the atlas generation is enabled
 :return: true if atlas is enabled
.. seealso:: setEnabled
 :rtype: bool
%End

    void setEnabled( bool enabled );
%Docstring
 Sets whether the atlas is enabled
 \param enabled set to true to enable to atlas
.. seealso:: enabled
%End

    bool hideCoverage() const;
%Docstring
 Returns true if the atlas is set to hide the coverage layer
 :return: true if coverage layer is hidden
.. seealso:: setHideCoverage
 :rtype: bool
%End

    void setHideCoverage( bool hide );
%Docstring
 Sets whether the coverage layer should be hidden in map items in the composition
 \param hide set to true to hide the coverage layer
.. seealso:: hideCoverage
%End

    QString filenamePattern() const;
%Docstring
 Returns the filename expression used for generating output filenames for each
 atlas page.
 :return: filename pattern
.. seealso:: setFilenamePattern
.. seealso:: filenamePatternErrorString
.. note::

   This property has no effect when exporting to PDF if singleFile() is true
 :rtype: str
%End

    bool setFilenamePattern( const QString &pattern );
%Docstring
 Sets the filename expression used for generating output filenames for each
 atlas page.
 :return: true if filename expression could be successful set, false if expression is invalid
 \param pattern expression to use for output filenames
.. seealso:: filenamePattern
.. seealso:: filenamePatternErrorString
.. note::

   This method has no effect when exporting to PDF if singleFile() is true
 :rtype: bool
%End

    QString filenamePatternErrorString() const;
%Docstring
 Returns an error string from parsing the filename expression.
 :return: filename pattern parser error
.. seealso:: setFilenamePattern
.. seealso:: filenamePattern
 :rtype: str
%End

    QgsVectorLayer *coverageLayer() const;
%Docstring
 Returns the coverage layer used for the atlas features
 :return: atlas coverage layer
.. seealso:: setCoverageLayer
 :rtype: QgsVectorLayer
%End

    void setCoverageLayer( QgsVectorLayer *layer );
%Docstring
 Sets the coverage layer to use for the atlas features
 \param layer vector coverage layer
.. seealso:: coverageLayer
%End

    QString pageNameExpression() const;
%Docstring
 Returns the expression used for calculating the page name.
 :return: expression string, or field name from coverage layer
.. seealso:: setPageNameExpression
.. seealso:: nameForPage
.. versionadded:: 2.12
 :rtype: str
%End

    void setPageNameExpression( const QString &pageNameExpression );
%Docstring
 Sets the expression used for calculating the page name.
 \param pageNameExpression expression string, or field name from coverage layer
.. seealso:: pageNameExpression
.. versionadded:: 2.12
%End

    QString nameForPage( int pageNumber ) const;
%Docstring
 Returns the calculated name for a specified atlas page number.
 \param pageNumber number of page, where 0 = first page
 :return: page name
.. seealso:: pageNameExpression
.. versionadded:: 2.12
 :rtype: str
%End

    bool singleFile() const;
%Docstring
 Returns whether the atlas will be exported to a single file. This is only
 applicable for PDF exports.
 :return: true if atlas will be exported to a single file
.. seealso:: setSingleFile
.. note::

   This property is only used for PDF exports.
 :rtype: bool
%End

    void setSingleFile( bool single );
%Docstring
 Sets whether the atlas should be exported to a single file. This is only
 applicable for PDF exports.
 \param single set to true to export atlas to a single file.
.. seealso:: singleFile
.. note::

   This method is only used for PDF exports.
%End

    QString fileFormat() const;
%Docstring
 Returns the  atlas file format used for image exports.
 :return: true if atlas will be exported to a single file
.. seealso:: setFileFormat
.. note::

   This property is only used for image exports.
.. versionadded:: 3.0
 :rtype: str
%End

    void setFileFormat( const QString &format );
%Docstring
 Sets the  atlas file format used for image exports.
 \param format set the file format extension
.. seealso:: fileFormat
.. note::

   This property is only used for image exports.
.. versionadded:: 3.0
%End

    bool sortFeatures() const;
%Docstring
 :rtype: bool
%End
    void setSortFeatures( bool doSort );

    bool sortAscending() const;
%Docstring
 :rtype: bool
%End
    void setSortAscending( bool ascending );

    bool filterFeatures() const;
%Docstring
 :rtype: bool
%End
    void setFilterFeatures( bool doFilter );

    QString featureFilter() const;
%Docstring
 :rtype: str
%End
    void setFeatureFilter( const QString &expression );

    QString featureFilterErrorString() const;
%Docstring
 Returns an error string from parsing the feature filter expression.
 :return: filename pattern parser error
.. seealso:: setFilenamePattern
.. seealso:: filenamePattern
 :rtype: str
%End

    QString sortKeyAttributeName() const;
%Docstring
 :rtype: str
%End
    void setSortKeyAttributeName( const QString &fieldName );

    QVector<qreal> predefinedScales() const;
%Docstring
 Returns the current list of predefined scales for the atlas. This is used
 for maps which are set to the predefined atlas scaling mode.
 :return: a vector of doubles representing predefined scales
.. seealso:: setPredefinedScales
.. seealso:: QgsComposerMap.atlasScalingMode
 :rtype: list of qreal
%End

    void setPredefinedScales( const QVector<qreal> &scales );
%Docstring
 Sets the list of predefined scales for the atlas. This is used
 for maps which are set to the predefined atlas scaling mode.
 \param scales a vector of doubles representing predefined scales
.. seealso:: predefinedScales
.. seealso:: QgsComposerMap.atlasScalingMode
%End

    bool beginRender();
%Docstring
 Begins the rendering. Returns true if successful, false if no matching atlas
features found.*
 :rtype: bool
%End
    void endRender();
%Docstring
Ends the rendering. Restores original extent
%End

    int numFeatures() const;
%Docstring
Returns the number of features in the coverage layer
 :rtype: int
%End

    bool prepareForFeature( const int i, const bool updateMaps = true );
%Docstring
 Prepare the atlas map for the given feature. Sets the extent and context variables
 \param i feature number
 \param updateMaps set to true to redraw maps and recalculate their extent
 :return: true if feature was successfully prepared
 :rtype: bool
%End

    bool prepareForFeature( const QgsFeature *feat );
%Docstring
 Prepare the atlas map for the given feature. Sets the extent and context variables
 :return: true if feature was successfully prepared
 :rtype: bool
%End

    QString currentFilename() const;
%Docstring
Returns the current filename. Must be called after prepareForFeature()
 :rtype: str
%End

    void writeXml( QDomElement &elem, QDomDocument &doc ) const;

    void readXml( const QDomElement &elem, const QDomDocument &doc );
%Docstring
 Reads general atlas settings from xml
 \param elem a QDomElement holding the atlas properties.
 \param doc QDomDocument for the source xml.
.. seealso:: readXMLMapSettings
.. note::

   This method should be called before restoring composer item properties
%End

    QgsComposition *composition();
%Docstring
 :rtype: QgsComposition
%End

    int updateFeatures();
%Docstring
 Requeries the current atlas coverage layer and applies filtering and sorting. Returns
 number of matching features. Must be called after prepareForFeature()
 :rtype: int
%End

    QgsFeature feature() const;
%Docstring
 Returns the current atlas feature. Must be called after prepareForFeature().
.. versionadded:: 2.12
 :rtype: QgsFeature
%End

    QString currentPageName() const;
%Docstring
 Returns the name of the page for the current atlas feature. Must be called after prepareForFeature().
.. versionadded:: 2.12
 :rtype: str
%End

    int currentFeatureNumber() const;
%Docstring
 Returns the current feature number, where a value of 0 corresponds to the first feature.
.. versionadded:: 2.12
 :rtype: int
%End

    void prepareMap( QgsComposerMap *map );
%Docstring
Recalculates the bounds of an atlas driven map
%End

    QgsGeometry currentGeometry( const QgsCoordinateReferenceSystem &projectedTo = QgsCoordinateReferenceSystem() ) const;
%Docstring
Returns the current atlas geometry in the given projection system (default to the coverage layer's CRS)
 :rtype: QgsGeometry
%End

  public slots:

    void refreshFeature();
%Docstring
 Refreshes the current atlas feature, by refetching its attributes from the vector layer provider
.. versionadded:: 2.5
%End

    void nextFeature();
    void prevFeature();
    void lastFeature();
    void firstFeature();

  signals:
    void parameterChanged();
%Docstring
Emitted when one of the parameters changes
%End

    void toggled( bool );
%Docstring
Emitted when atlas is enabled or disabled
%End

    void statusMsgChanged( const QString &message );
%Docstring
Is emitted when the atlas has an updated status bar message for the composer window
%End

    void coverageLayerChanged( QgsVectorLayer *layer );
%Docstring
Is emitted when the coverage layer for an atlas changes
%End

    void renderBegun();
%Docstring
Is emitted when atlas rendering has begun
%End

    void renderEnded();
%Docstring
Is emitted when atlas rendering has ended
%End

    void featureChanged( QgsFeature *feature );
%Docstring
Is emitted when the current atlas feature changes
%End

    void numberFeaturesChanged( int numFeatures );
%Docstring
 Is emitted when the number of features for the atlas changes.
.. versionadded:: 2.12
%End

  public:
    typedef QMap< QgsFeatureId, QVariant > SorterKeys;

};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgsatlascomposition.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
