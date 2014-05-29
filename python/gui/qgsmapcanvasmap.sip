
%ModuleHeaderCode
// fix to allow compilation with sip 4.7 that for some reason
// doesn't add this include to the file where the code from
// ConvertToSubClassCode goes.
#include <qgsmapcanvasmap.h>
%End

class QgsMapCanvasMap : QgsMapCanvasItem
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
    QgsMapCanvasMap( QgsMapCanvas* canvas /TransferThis/ );

    //! @note added in 2.4
    void setContent( const QImage& image, const QgsRectangle& rect );

    //! @note added in 2.4
    QImage contentImage() const;

    virtual void paint( QPainter * painter );

    //! @deprecated in 2.4 - does nothing. Kept for API compatibility
    void refresh() /Deprecated/;

    //! @deprecated in 2.4 - does nothing. Kept for API compatibility
    void resize( QSize size ) /Deprecated/;

    //! @deprecated in 2.4 - does nothing. Kept for API compatibility
    void enableAntiAliasing( bool flag ) /Deprecated/;

    //! @deprecated in 2.4 - does nothing. Kept for API compatibility
    void render() /Deprecated/;

    //! @deprecated in 2.4 - does nothing. Kept for API compatibility
    void setBackgroundColor( const QColor& color ) /Deprecated/;

    //! @deprecated in 2.4 - not called by QgsMapCanvas anymore
    void setPanningOffset( const QPoint& point ) /Deprecated/;

    //! @deprecated in 2.4
    QPaintDevice& paintDevice() /Deprecated/;

    //! @deprecated in 2.4 - does nothing. Kept for API compatibility
    void updateContents() /Deprecated/;
};
