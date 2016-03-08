/**
 * @brief The QgsPixmapLabel class shows a pixmap and adjusts its size to the space given
 * to the widget by the layout and keeping its aspect ratio.
 */
class QgsPixmapLabel : QLabel
{
%TypeHeaderCode
#include <qgspixmaplabel.h>
%End
  public:
    explicit QgsPixmapLabel( QWidget *parent /TransferThis/ = nullptr );
    /**
     * Calculates the height for the given width.
     *
     * @param width The width for the widget
     * @return An appropriate height
     */
    virtual int heightForWidth( int width ) const;

    /**
     * An optimal size for the widget. Effectively using the height
     * determined from the width with the given aspect ratio.
     * @return A size hint
     */
    virtual QSize sizeHint() const;

  public slots:
    void setPixmap( const QPixmap & );
    void resizeEvent( QResizeEvent * );
};
