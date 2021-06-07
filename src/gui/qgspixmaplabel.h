/***************************************************************************

               ----------------------------------------------------
              date                 : 7.9.2015
              copyright            : (C) 2015 by Matthias Kuhn
              email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPIXMAPLABEL_H
#define QGSPIXMAPLABEL_H

#include <QLabel>
#include "qgis_sip.h"
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \brief The QgsPixmapLabel class shows a pixmap and adjusts its size to the space given
 * to the widget by the layout and keeping its aspect ratio.
 */
class GUI_EXPORT QgsPixmapLabel : public QLabel
{
    Q_OBJECT

  public:

    //! Constructor for QgsPixmapLabel
    explicit QgsPixmapLabel( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Calculates the height for the given width.
     *
     * \param width The width for the widget
     * \returns An appropriate height
     */
    int heightForWidth( int width ) const override;

    /**
     * An optimal size for the widget. Effectively using the height
     * determined from the width with the given aspect ratio.
     * \returns A size hint
     */
    QSize sizeHint() const override;

  public slots:
    void setPixmap( const QPixmap & );
    void resizeEvent( QResizeEvent * ) override;
  private:
    QPixmap mPixmap;
};

#endif // QGSPIXMAPLABEL_H
