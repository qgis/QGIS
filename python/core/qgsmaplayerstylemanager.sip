
class QgsMapLayerStyle
{
%TypeHeaderCode
#include <qgsmaplayerstylemanager.h>
%End
  public:
    //! construct invalid style
    QgsMapLayerStyle();

    //! construct style from QML definition (XML)
    explicit QgsMapLayerStyle( const QString& xmlData );

    //! Tell whether the style is valid (i.e. there is something stored in it)
    bool isValid() const;

    //! Remove any stored style data (will get invalid)
    void clear();

    //! Return XML content of the style
    QString xmlData() const;

    //! Store layer's active style information in the instance
    void readFromLayer( QgsMapLayer* layer );
    //! Apply stored layer's style information to the layer
    void writeToLayer( QgsMapLayer* layer ) const;

    //! Read style configuration (for project file reading)
    void readXml( const QDomElement& styleElement );
    //! Write style configuration (for project file writing)
    void writeXml( QDomElement& styleElement ) const;
};


class QgsMapLayerStyleManager : QObject
{
%TypeHeaderCode
#include <qgsmaplayerstylemanager.h>
%End
  public:
    //! Construct a style manager associated with a map layer (must not be null)
    QgsMapLayerStyleManager( QgsMapLayer* layer );

    //! Get pointer to the associated map layer
    QgsMapLayer* layer() const;

    //! Reset the style manager to a basic state - with one default style which is set as current
    void reset();

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
    //! Rename a stored style to a different name
    //! @return true on success (style exists and new name is unique)
    bool renameStyle( const QString& name, const QString& newName );

    //! Return name of the current style
    QString currentStyle() const;
    //! Set a different style as the current style - will apply it to the layer
    //! @return true on success
    bool setCurrentStyle( const QString& name );

    //! Temporarily apply a different style to the layer. The argument
    //! can be either a style name or a full QML style definition.
    //! Each call must be paired with restoreOverrideStyle()
    bool setOverrideStyle( const QString& styleDef );
    //! Restore the original store after a call to setOverrideStyle()
    bool restoreOverrideStyle();

  signals:
    //! Emitted when a new style has been added
    void styleAdded( const QString& name );
    //! Emitted when a style has been removed
    void styleRemoved( const QString& name );
    //! Emitted when a style has been renamed
    void styleRenamed( const QString& oldName, const QString& newName );
    //! Emitted when the current style has been changed
    void currentStyleChanged( const QString& currentName );
};
