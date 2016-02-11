
class QgsPluginLayerType
{
%TypeHeaderCode
#include "qgspluginlayerregistry.h"
%End
  public:

    QgsPluginLayerType( const QString& name );
    virtual ~QgsPluginLayerType();

    QString name();

    /** Return new layer of this type. Return NULL on error */
    virtual QgsPluginLayer* createLayer() /Factory/;

    /** Return new layer of this type, using layer URI (specific to this plugin layer type). Return NULL on error.
     * @note added in 2.10
     */
    virtual QgsPluginLayer* createLayer( const QString& uri ) /Factory/;

    /** Show plugin layer properties dialog. Return false if the dialog cannot be shown. */
    virtual bool showLayerProperties( QgsPluginLayer* layer );

};

//=============================================================================

/** \ingroup core
    a registry of plugin layers types
*/
class QgsPluginLayerRegistry
{
%TypeHeaderCode
#include "qgspluginlayerregistry.h"
%End

  public:

    /** Means of accessing canonical single instance  */
    static QgsPluginLayerRegistry* instance();

    ~QgsPluginLayerRegistry();

    /** List all known layer types
     *  \note added in v2.1 */
    QStringList pluginLayerTypes();

    /** Add plugin layer type (take ownership) and return true on success */
    bool addPluginLayerType( QgsPluginLayerType* pluginLayerType /Transfer/ );

    /** Remove plugin layer type and return true on success */
    bool removePluginLayerType( const QString& typeName );

    /** Return plugin layer type metadata or NULL if doesn't exist */
    QgsPluginLayerType* pluginLayerType( const QString& typeName );

    /** Return new layer if corresponding plugin has been found, else return NULL.
     * @note optional param uri added in 2.10
     */
    QgsPluginLayer* createLayer( const QString& typeName, const QString& uri = QString() ) /Factory/;

  private:

    /** Private since instance() creates it */
    QgsPluginLayerRegistry();
    QgsPluginLayerRegistry( const QgsPluginLayerRegistry& rh );

};
