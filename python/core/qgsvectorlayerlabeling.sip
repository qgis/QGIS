/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayerlabeling.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsAbstractVectorLayerLabeling
{
%Docstring
 Abstract base class - its implementations define different approaches to the labeling of a vector layer.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsvectorlayerlabeling.h"
%End
  public:
    QgsAbstractVectorLayerLabeling();
%Docstring
Default constructor
%End
    virtual ~QgsAbstractVectorLayerLabeling();

    virtual QString type() const = 0;
%Docstring
Unique type string of the labeling configuration implementation
 :rtype: str
%End

    virtual QgsAbstractVectorLayerLabeling *clone() const = 0 /Factory/;
%Docstring
Return a new copy of the object
 :rtype: QgsAbstractVectorLayerLabeling
%End


    virtual QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const = 0;
%Docstring
Return labeling configuration as XML element
 :rtype: QDomElement
%End

    virtual QStringList subProviders() const;
%Docstring
Get list of sub-providers within the layer's labeling.
 :rtype: list of str
%End

    virtual QgsPalLayerSettings settings( const QString &providerId = QString() ) const = 0;
%Docstring
 Get associated label settings. In case of multiple sub-providers with different settings,
 they are identified by their ID (e.g. in case of rule-based labeling, provider ID == rule key)
 :rtype: QgsPalLayerSettings
%End

    virtual void setSettings( QgsPalLayerSettings *settings /Transfer/, const QString &providerId = QString() ) = 0;
%Docstring
 Set pal settings for a specific provider (takes ownership).

 \param settings Pal layer settings
 \param providerId The id of the provider

.. versionadded:: 3.0
%End

    virtual bool requiresAdvancedEffects() const = 0;
%Docstring
 Returns true if drawing labels requires advanced effects like composition
 modes, which could prevent it being used as an isolated cached image
 or exported to a vector format.
.. versionadded:: 3.0
 :rtype: bool
%End


    static QgsAbstractVectorLayerLabeling *create( const QDomElement &element, const QgsReadWriteContext &context ) /Factory/;
%Docstring
Try to create instance of an implementation based on the XML data
 :rtype: QgsAbstractVectorLayerLabeling
%End

    virtual void toSld( QDomNode &parent, const QgsStringMap &props ) const;
%Docstring
 Writes the SE 1.1 TextSymbolizer element based on the current layer labeling settings
%End

  protected:

    virtual void writeTextSymbolizer( QDomNode &parent, QgsPalLayerSettings &settings, const QgsStringMap &props ) const;
%Docstring
 Writes a TextSymbolizer element contents based on the provided labeling settings
 writeTextSymbolizer
 @param parent the node that will have the text symbolizer element added to it
 @param settings the settings getting translated to a TextSymbolizer
 @param props a open ended set of properties that can drive/inform the SLD encoding
%End

  private:
    QgsAbstractVectorLayerLabeling( const QgsAbstractVectorLayerLabeling &rhs );
};

class QgsVectorLayerSimpleLabeling : QgsAbstractVectorLayerLabeling
{
%Docstring
 Basic implementation of the labeling interface.

 The configuration is kept in layer's custom properties for backward compatibility.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsvectorlayerlabeling.h"
%End
  public:
    explicit QgsVectorLayerSimpleLabeling( const QgsPalLayerSettings &settings );
%Docstring
Constructs simple labeling configuration with given initial settings
%End

    virtual QString type() const;
    virtual QgsAbstractVectorLayerLabeling *clone() const /Factory/;
    virtual QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const;
    virtual QgsPalLayerSettings settings( const QString &providerId = QString() ) const;

    virtual void setSettings( QgsPalLayerSettings *settings /Transfer/, const QString &providerId = QString() );
%Docstring
 Set pal settings (takes ownership).

 \param settings Pal layer settings
 \param providerId Unused parameter

.. versionadded:: 3.0
%End

    virtual bool requiresAdvancedEffects() const;

    virtual void toSld( QDomNode &parent, const QgsStringMap &props ) const;

    static QgsVectorLayerSimpleLabeling *create( const QDomElement &element, const QgsReadWriteContext &context );
%Docstring
Create the instance from a DOM element with saved configuration
 :rtype: QgsVectorLayerSimpleLabeling
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayerlabeling.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
