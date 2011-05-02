
%ModuleHeaderCode
// fix to allow compilation with sip 4.7 that for some reason
// doesn't add this include to the file where the code from
// ConvertToSubClassCode goes.
#include <qgsmapcanvasmap.h>
%End

class QgsMapCanvasMap : QGraphicsRectItem
{
%TypeHeaderCode
#include <qgsmapcanvasmap.h>
%End

%ConvertToSubClassCode
	if (dynamic_cast<QgsMapCanvasMap*>(sipCpp) != NULL)
	{
		sipClass = sipClass_QgsMapCanvasMap;
	}
  else 
  {
    sipClass = NULL;
  }
%End

  public:
    
    //! constructor
    QgsMapCanvasMap(QgsMapCanvas* canvas /TransferThis/);
    
    //! resize canvas item and pixmap
    void resize(QSize size);
    
    void enableAntiAliasing(bool flag);
    
    void useImageToRender(bool flag);

    //! renders map using QgsMapRender to mPixmap
    void render();
    
    void setBackgroundColor(const QColor& color);
    
    void setPanningOffset(const QPoint& point);
    
    //! @deprecated Please use paintDevice() function which is also save in case QImage is used
    QPixmap& pixmap() /Deprecated/;
    
    void paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*);

    QRectF boundingRect() const;
  
    //! Update contents - can be called while drawing to show the status.
    //! Added in version 1.2
    void updateContents();

};

