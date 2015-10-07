class QgsRendererV2Widget /External/;

class QgsRendererV2AbstractMetadata
{
%TypeHeaderCode
#include <qgsrendererv2registry.h>
%End

  public:
    QgsRendererV2AbstractMetadata( const QString& name, const QString& visibleName, const QIcon& icon = QIcon() );
    virtual ~QgsRendererV2AbstractMetadata();

    QString name() const;
    QString visibleName() const;

    QIcon icon() const;
    void setIcon( const QIcon& icon );

    /** Return new instance of the renderer given the DOM element. Returns NULL on error.
     * Pure virtual function: must be implemented in derived classes.  */
    virtual QgsFeatureRendererV2* createRenderer( QDomElement& elem ) = 0 /Factory/;
    /** Return new instance of settings widget for the renderer. Returns NULL on error. */
    virtual QgsRendererV2Widget* createRendererWidget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer ) /Factory/;

    virtual QgsFeatureRendererV2* createRendererFromSld( QDomElement& elem, QGis::GeometryType geomType ) /Factory/;
};

/**
 Convenience metadata class that uses static functions to create renderer and its widget.
 */
class QgsRendererV2Metadata : QgsRendererV2AbstractMetadata
{
%TypeHeaderCode
#include <qgsrendererv2registry.h>
%End
  public:
    virtual QgsFeatureRendererV2* createRenderer( QDomElement& elem ) /Factory/;
    virtual QgsRendererV2Widget* createRendererWidget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer ) /Factory/;
    virtual QgsFeatureRendererV2* createRendererFromSld( QDomElement& elem, QGis::GeometryType geomType ) /Factory/;

    // QgsRendererV2CreateFunc createFunction() const;
    // QgsRendererV2WidgetFunc widgetFunction() const;
    // QgsFeatureRendererV2* (*)( QDomElement&, QGis::GeometryType geomType ) createFromSldFunction() const;
    // void setWidgetFunction( QgsRendererV2WidgetFunc f );

  private:
    QgsRendererV2Metadata(); // pretend this is private
};

/**
  Registry of renderers.

  This is a singleton, renderers can be added / removed at any time
 */
class QgsRendererV2Registry
{
%TypeHeaderCode
#include <qgsrendererv2registry.h>
%End

  public:

    static QgsRendererV2Registry* instance();

    //! add a renderer to registry. Takes ownership of the metadata object.
    bool addRenderer( QgsRendererV2AbstractMetadata* metadata /Transfer/ );

    //! remove renderer from registry
    bool removeRenderer( const QString& rendererName );

    //! get metadata for particular renderer. Returns NULL if not found in registry.
    QgsRendererV2AbstractMetadata* rendererMetadata( const QString& rendererName );

    //! return a list of available renderers
    QStringList renderersList();

  protected:
    //! protected constructor
    QgsRendererV2Registry();
    ~QgsRendererV2Registry();
};
