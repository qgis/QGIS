class QgsDetailedItemData
{
%TypeHeaderCode
#include <qgsdetaileditemdata.h>
%End

  public:
    QgsDetailedItemData();
    void setTitle( const QString& theTitle );
    void setDetail( const QString& theDetail );
    void setCategory( const QString& theCategory );
    void setIcon( const QPixmap& theIcon );
    void setCheckable( const bool theFlag );
    void setChecked( const bool theFlag );
    void setEnabled( bool theFlag );
    /** This is a hint to the delegate to render using
     * a widget rather than manually painting every
     * part of the list item.
     * @note the delegate may completely ignore this
     * depending on the delegate implementation.
     */
    void setRenderAsWidget( bool theFlag );

    QString title() const;
    QString detail() const;
    QString category() const;
    QPixmap icon() const;
    bool isCheckable() const;
    bool isChecked() const;
    bool isEnabled() const;
    bool isRenderedAsWidget() const;
};
