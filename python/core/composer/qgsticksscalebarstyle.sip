class QgsTicksScaleBarStyle: QgsScaleBarStyle
{
%TypeHeaderCode
#include <qgsticksscalebarstyle.h>
%End
  public:
    enum TickPosition
    {
      TicksUp,
      TicksDown,
      TicksMiddle
    };

    QgsTicksScaleBarStyle( const QgsComposerScaleBar* bar );
    ~QgsTicksScaleBarStyle();

    QString name() const;

    /** Draw method
     @param p painter object
     @param xOffset offset
     */
    void draw( QPainter* p, double xOffset = 0 ) const;

    void setTickPosition( TickPosition p );
};
