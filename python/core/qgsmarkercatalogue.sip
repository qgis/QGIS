
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
     */
    QImage imageMarker (QString fullName, int size, QPen pen, QBrush brush, double opacity );

    /** Returns qpicture of the marker
     * \param fullName full name, e.g. hard:circle, svg:/home/usr1/marker1.svg
     */
    QPicture pictureMarker (QString fullName, int size, QPen pen, QBrush brush, double opacity );
    
    /** Returns a pixmap given a file name of a svg marker
     *  NOTE: this method needs to be public static for QgsMarkerDialog::visualizeMarkers */
    static void svgMarker (QPainter * thepPainter, QString name, int size );

  public slots:
    // reload the symbols
    // added in 1.2
    void refreshList();

  signals:
    // symbols were reloaded
    // added in 1.2
    void markersRefreshed();
};

