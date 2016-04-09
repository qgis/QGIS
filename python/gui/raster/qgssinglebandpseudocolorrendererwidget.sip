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

    void setFromRenderer( const QgsRasterRenderer* r );

  public slots:
    void loadMinMax( int theBandNo, double theMin, double theMax, int theOrigin );

};

class QgsTreeWidgetItem: QObject, QTreeWidgetItem
{
%TypeHeaderCode
#include <qgssinglebandpseudocolorrendererwidget.h>
%End
  public:
    explicit QgsTreeWidgetItem( QTreeWidget * parent, int type = Type );
    virtual void setData( int column, int role, const QVariant & value );
    virtual bool operator< ( const QTreeWidgetItem & other ) const;

  signals:
    void itemEdited( QTreeWidgetItem* item, int column );
};