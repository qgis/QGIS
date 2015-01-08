
class QgsMapLayerStyle
{
%TypeHeaderCode
#include <qgsmaplayerstylemanager.h>
%End
  public:
    //! construct invalid style
    QgsMapLayerStyle();

    //! Tell whether the style is valid (i.e. there is something stored in it)
    bool isValid() const;

    //! Return information about the style - for debugging purposes only
    QString dump() const;

    //! Store layer's active style information in the instance
    void readFromLayer( QgsMapLayer* layer );
    //! Apply stored layer's style information to the layer
    void writeToLayer( QgsMapLayer* layer ) const;

    //! Read style configuration (for project file reading)
    void readXml( const QDomElement& styleElement );
    //! Write style configuration (for project file writing)
    void writeXml( QDomElement& styleElement ) const;
};


class QgsMapLayerStyleManager
{
%TypeHeaderCode
#include <qgsmaplayerstylemanager.h>
%End
  public:
    //! Construct a style manager associated with a map layer (must not be null)
    QgsMapLayerStyleManager( QgsMapLayer* layer );

    //! Read configuration (for project loading)
    void readXml( const QDomElement& mgrElement );
    //! Write configuration (for project saving)
    void writeXml( QDomElement& mgrElement ) const;

    //! Return list of all defined style names
    QStringList styles() const;
    //! Return data of a stored style - accessed by its unique name
    QgsMapLayerStyle style( const QString& name ) const;

    //! Add a style with given name and data
    //! @return true on success (name is unique and style is valid)
    bool addStyle( const QString& name, const QgsMapLayerStyle& style );
    //! Add style by cloning the current one
    //! @return true on success
    bool addStyleFromLayer( const QString& name );
    //! Remove a stored style
    //! @return true on success (style exists and it is not the last one)
    bool removeStyle( const QString& name );

    //! Return name of the current style
    QString currentStyle() const;
    //! Set a different style as the current style - will apply it to the layer
    //! @return true on success
    bool setCurrentStyle( const QString& name );
};
