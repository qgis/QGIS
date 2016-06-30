class QgsSingleBandPseudoColorRendererWidget : QgsRasterRendererWidget
{
%TypeHeaderCode
#include <qgssinglebandpseudocolorrendererwidget.h>
%End

  public:
    enum Mode
    {
      Continuous,    // Using breaks from color palette
      EqualInterval,
      Quantile,
    };

    QgsSingleBandPseudoColorRendererWidget( QgsRasterLayer* layer, const QgsRectangle &extent = QgsRectangle() );
    ~QgsSingleBandPseudoColorRendererWidget();

    static QgsRasterRendererWidget* create( QgsRasterLayer* layer, const QgsRectangle &theExtent ) /Factory/;
    QgsRasterRenderer* renderer();
    void setMapCanvas( QgsMapCanvas* canvas );

    void setFromRenderer( const QgsRasterRenderer* r );

  public slots:
    void loadMinMax( int theBandNo, double theMin, double theMax, int theOrigin );

};

/**
 * Custom QTreeWidgetItem with extra signal when item is edited and numeric sorting.
 */
class QgsTreeWidgetItem: QObject, QTreeWidgetItem
{
%TypeHeaderCode
#include <qgssinglebandpseudocolorrendererwidget.h>
%End
  public:
    /** Constructs a tree widget item of the specified type and appends it to the items in the given parent. */
    explicit QgsTreeWidgetItem( QTreeWidget * parent, int type = Type );

    /** Sets the value for the item's column and role to the given value. */
    virtual void setData( int column, int role, const QVariant & value );
    virtual bool operator< ( const QTreeWidgetItem & other ) const;

  signals:
    /** This signal is emitted when the contents of the column in the specified item has been edited by the user. */
    void itemEdited( QTreeWidgetItem* item, int column );
};
