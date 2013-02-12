
/** Catalogue of point symbols */
class QgsMarkerCatalogue : QObject /NoDefaultCtors/
{
%TypeHeaderCode
#include <qgsmarkercatalogue.h>
%End

public:
    //! Destructor
    ~QgsMarkerCatalogue();

    //! Access to canonical QgsMarkerCatalogue instance
    static QgsMarkerCatalogue *instance();

    /**List of available markers*/
    QStringList list();

    /** Returns pixmap of the marker
     * \param fullName full name, e.g. hard:circle, svg:/home/usr1/marker1.svg
     * \param size size of image
     * \param pen pen to use
     * \param brush brush to use
     * \param opacity opacity to use
     */
    QImage imageMarker( QString fullName, double size, QPen pen, QBrush brush, double opacity = 1.0 );

    /** Returns qpicture of the marker
     * \param fullName full name, e.g. hard:circle, svg:/home/usr1/marker1.svg
     * \param size size of marker
     * \param pen pen to use
     * \param brush brush to use
     * \param opacity opacity to use
     */
    QPicture pictureMarker( QString fullName, double size, QPen pen, QBrush brush, double opacity = 1.0 );

    /** Returns a pixmap given a file name of a svg marker
     *  NOTE: this method needs to be public static for QgsMarkerDialog::visualizeMarkers */
    static bool svgMarker( QPainter *thepPainter, QString name, double size );

  public slots:
    void refreshList();

  signals:
    void markersRefreshed();
};

