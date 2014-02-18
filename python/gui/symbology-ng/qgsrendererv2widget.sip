class QgsRendererV2Widget : QWidget
{
%TypeHeaderCode
#include <qgsrendererv2widget.h>
%End

  public:
    QgsRendererV2Widget( QgsVectorLayer* layer, QgsStyleV2* style );

    virtual ~QgsRendererV2Widget();

    //! return pointer to the renderer (no transfer of ownership)
    virtual QgsFeatureRendererV2* renderer() = 0;

    //! show a dialog with renderer's symbol level settings
    void showSymbolLevelsDialog( QgsFeatureRendererV2* r );

  protected:
    /**Subclasses may provide the capability of changing multiple symbols at once by implementing the following two methods
      and by connecting the slot contextMenuViewCategories(const QPoint&)*/
    virtual QList<QgsSymbolV2*> selectedSymbols();
    virtual void refreshSymbolView();

  protected slots:
    void  contextMenuViewCategories( const QPoint& p );
    /**Change color of selected symbols*/
    void changeSymbolColor();
    /**Change opacity of selected symbols*/
    void changeSymbolTransparency();
    /**Change units mm/map units of selected symbols*/
    void changeSymbolUnit();
    /**Change line widths of selected symbols*/
    void changeSymbolWidth();
    /**Change marker sizes of selected symbols*/
    void changeSymbolSize();

    virtual void copy();
    virtual void paste();

};
