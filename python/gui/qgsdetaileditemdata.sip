class QgsDetailedItemData
{
%TypeHeaderCode
#include <qgsdetaileditemdata.h>
%End

  public:
    QgsDetailedItemData();
    void setTitle( const QString& title );
    void setDetail( const QString& detail );
    void setCategory( const QString& category );
    void setIcon( const QPixmap& icon );
    void setCheckable( const bool flag );
    void setChecked( const bool flag );
    void setEnabled( bool flag );
    /** This is a hint to the delegate to render using
     * a widget rather than manually painting every
     * part of the list item.
     * @note the delegate may completely ignore this
     * depending on the delegate implementation.
     */
    void setRenderAsWidget( bool flag );

    QString title() const;
    QString detail() const;
    QString category() const;
    QPixmap icon() const;
    bool isCheckable() const;
    bool isChecked() const;
    bool isEnabled() const;
    bool isRenderedAsWidget() const;
};
