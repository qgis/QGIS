class QgsRasterRendererWidget: QWidget
{
%TypeHeaderCode
#include <qgsrasterrendererwidget.h>
%End
  public:
    QgsRasterRendererWidget( QgsRasterLayer* layer, const QgsRectangle &extent );
    virtual ~QgsRasterRendererWidget();

    enum LoadMinMaxAlgo
    {
      Estimate,
      Actual,
      CurrentExtent,
      CumulativeCut   // 2 - 98% cumulative cut
    };

    virtual QgsRasterRenderer* renderer() = 0 /Factory/;

    void setRasterLayer( QgsRasterLayer* layer );
    const QgsRasterLayer* rasterLayer() const;

    virtual QString min( int index = 0 );
    virtual QString max( int index = 0 );
    virtual void setMin( QString value, int index = 0 );
    virtual void setMax( QString value, int index = 0 );
    virtual QString stdDev();
    virtual void setStdDev( QString value );
    virtual int selectedBand( int index = 0 );

  protected:
    /**Returns a band name for display. First choice is color name, otherwise band number*/
    QString displayBandName( int band ) const;
};
