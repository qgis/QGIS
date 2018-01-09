/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutatlas.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayoutAtlas : QObject, QgsAbstractLayoutIterator, QgsLayoutSerializableObject
{
%Docstring
Class used to render an Atlas, iterating over geometry features.
prepareForFeature() modifies the atlas map's extent to zoom on the given feature.
This class is used for printing, exporting to PDF and images.

.. note::

   This class should not be created directly. For the atlas to function correctly
the atlasComposition() property for QgsComposition should be used to retrieve a
QgsLayoutAtlas which is automatically created and attached to the composition.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutatlas.h"
%End
  public:

    QgsLayoutAtlas( QgsLayout *layout /TransferThis/ );
%Docstring
Constructor for new QgsLayoutAtlas.
%End

    virtual QString stringType() const;

    virtual QgsLayout *layout();



    virtual bool writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const;

    virtual bool readXml( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context );


    bool enabled() const;
%Docstring
Returns whether the atlas generation is enabled

.. seealso:: :py:func:`setEnabled`
%End

    void setEnabled( bool enabled );
%Docstring
Sets whether the atlas is ``enabled``.

.. seealso:: :py:func:`enabled`
%End

    bool hideCoverage() const;
%Docstring
Returns true if the atlas is set to hide the coverage layer.

.. seealso:: :py:func:`setHideCoverage`
%End

    void setHideCoverage( bool hide );
%Docstring
Sets whether the coverage layer should be hidden in map items in the layouts.

.. seealso:: :py:func:`hideCoverage`
%End

    QString filenameExpression() const;
%Docstring
Returns the filename expression used for generating output filenames for each
atlas page.

.. seealso:: :py:func:`setFilenameExpression`

.. seealso:: :py:func:`currentFilename`
%End

    bool setFilenameExpression( const QString &expression, QString &errorString /Out/ );
%Docstring
Sets the filename ``expression`` used for generating output filenames for each
atlas page.
If an invalid expression is passed, false will be returned and ``errorString``
will be set to the expression error.

.. seealso:: :py:func:`filenameExpression`

.. seealso:: :py:func:`currentFilename`
%End

    QString currentFilename() const;
%Docstring
Returns the current feature filename.

.. seealso:: :py:func:`filenameExpression`

.. seealso:: :py:func:`setFilenameExpression`
%End

    QgsVectorLayer *coverageLayer() const;
%Docstring
Returns the coverage layer used for the atlas features.

.. seealso:: :py:func:`setCoverageLayer`
%End

    void setCoverageLayer( QgsVectorLayer *layer );
%Docstring
Sets the coverage ``layer`` to use for the atlas features.

.. seealso:: :py:func:`coverageLayer`
%End

    QString pageNameExpression() const;
%Docstring
Returns the expression (or field name) used for calculating the page name.

.. seealso:: :py:func:`setPageNameExpression`

.. seealso:: :py:func:`nameForPage`
%End

    void setPageNameExpression( const QString &expression );
%Docstring
Sets the ``expression`` (or field name) used for calculating the page name.

.. seealso:: :py:func:`pageNameExpression`
%End

    QString nameForPage( int page ) const;
%Docstring
Returns the calculated name for a specified atlas ``page`` number. Page numbers start at 0.

.. seealso:: :py:func:`pageNameExpression`
%End

    bool sortFeatures() const;
%Docstring
Returns true if features should be sorted in the atlas.

.. seealso:: :py:func:`setSortFeatures`

.. seealso:: :py:func:`sortAscending`

.. seealso:: :py:func:`sortExpression`
%End

    void setSortFeatures( bool enabled );
%Docstring
Sets whether features should be sorted in the atlas.

.. seealso:: :py:func:`sortFeatures`

.. seealso:: :py:func:`setSortAscending`

.. seealso:: :py:func:`setSortExpression`
%End

    bool sortAscending() const;
%Docstring
Returns true if features should be sorted in an ascending order.

This property has no effect is sortFeatures() is false.

.. seealso:: :py:func:`sortFeatures`

.. seealso:: :py:func:`setSortAscending`

.. seealso:: :py:func:`sortExpression`
%End

    void setSortAscending( bool ascending );
%Docstring
Sets whether features should be sorted in an ascending order.

This property has no effect is sortFeatures() is false.

.. seealso:: :py:func:`setSortFeatures`

.. seealso:: :py:func:`sortAscending`

.. seealso:: :py:func:`setSortExpression`
%End

    QString sortExpression() const;
%Docstring
Returns the expression (or field name) to use for sorting features.

This property has no effect is sortFeatures() is false.

.. seealso:: :py:func:`sortFeatures`

.. seealso:: :py:func:`sortAscending`

.. seealso:: :py:func:`setSortExpression`
%End

    void setSortExpression( const QString &expression );
%Docstring
Sets the ``expression`` (or field name) to use for sorting features.

This property has no effect is sortFeatures() is false.

.. seealso:: :py:func:`setSortFeatures`

.. seealso:: :py:func:`setSortAscending`

.. seealso:: :py:func:`sortExpression`
%End

    bool filterFeatures() const;
%Docstring
Returns true if features should be filtered in the coverage layer.

.. seealso:: :py:func:`filterExpression`

.. seealso:: :py:func:`setFilterExpression`
%End

    void setFilterFeatures( bool filtered );
%Docstring
Sets whether features should be ``filtered`` in the coverage layer.

.. seealso:: :py:func:`filterFeatures`

.. seealso:: :py:func:`setFilterExpression`
%End

    QString filterExpression() const;
%Docstring
Returns the expression used for filtering features in the coverage layer.

This property has no effect is filterFeatures() is false.

.. seealso:: :py:func:`setFilterExpression`

.. seealso:: :py:func:`filterFeatures`
%End

    bool setFilterExpression( const QString &expression, QString &errorString /Out/ );
%Docstring
Sets the ``expression`` used for filtering features in the coverage layer.

This property has no effect is filterFeatures() is false.

If an invalid expression is passed, false will be returned and ``errorString``
will be set to the expression error.

.. seealso:: :py:func:`filterExpression`

.. seealso:: :py:func:`setFilterFeatures`
%End

    int updateFeatures();
%Docstring
Requeries the current atlas coverage layer and applies filtering and sorting. Returns
number of matching features.
%End

    virtual bool beginRender();

    virtual bool endRender();

    virtual int count();

    virtual QString filePath( const QString &baseFilePath, const QString &extension );


    int currentFeatureNumber() const;
%Docstring
Returns the current feature number, where a value of 0 corresponds to the first feature.
%End

  public slots:

    virtual bool next();


    bool previous();
%Docstring
Iterates to the previous feature, returning false if no previous feature exists.

.. seealso:: :py:func:`next`

.. seealso:: :py:func:`last`

.. seealso:: :py:func:`first`

.. seealso:: :py:func:`seekTo`
%End

    bool last();
%Docstring
Seeks to the last feature, returning false if no feature was found.

.. seealso:: :py:func:`next`

.. seealso:: :py:func:`previous`

.. seealso:: :py:func:`first`

.. seealso:: :py:func:`seekTo`
%End

    bool first();
%Docstring
Seeks to the first feature, returning false if no feature was found.

.. seealso:: :py:func:`next`

.. seealso:: :py:func:`previous`

.. seealso:: :py:func:`last`

.. seealso:: :py:func:`seekTo`
%End

    bool seekTo( int feature );
%Docstring
Seeks to the specified ``feature`` number.

.. seealso:: :py:func:`first`

.. seealso:: :py:func:`previous`

.. seealso:: :py:func:`next`

.. seealso:: :py:func:`last`
%End

    bool seekTo( const QgsFeature &feature );
%Docstring
Seeks to the specified ``feature``.

.. seealso:: :py:func:`first`

.. seealso:: :py:func:`previous`

.. seealso:: :py:func:`next`

.. seealso:: :py:func:`last`
%End

    void refreshCurrentFeature();
%Docstring
Refreshes the current atlas feature, by refetching its attributes from the vector layer provider
%End

  signals:

    void changed();
%Docstring
Emitted when one of the atlas parameters changes.
%End

    void toggled( bool );
%Docstring
Emitted when atlas is enabled or disabled.
%End

    void coverageLayerChanged( QgsVectorLayer *layer );
%Docstring
Emitted when the coverage layer for the atlas changes.
%End

    void messagePushed( const QString &message );
%Docstring
Is emitted when the atlas has an updated status bar ``message``.
%End

    void numberFeaturesChanged( int numFeatures );
%Docstring
Emitted when the number of features for the atlas changes.
%End

    void featureChanged( const QgsFeature &feature );
%Docstring
Is emitted when the current atlas ``feature`` changes.
%End

    void renderBegun();
%Docstring
Emitted when atlas rendering has begun.
%End

    void renderEnded();
%Docstring
Emitted when atlas rendering has ended.
%End

};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutatlas.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
