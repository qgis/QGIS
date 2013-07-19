#include <QPainter>
#include <QPaintEvent>
#include <QSize>
#include <QSlider>

class QgsSlider : public QSlider
{
  Q_OBJECT
public:
  QgsSlider( QWidget *parent = 0 );
  QgsSlider( Qt::Orientation orientation, QWidget * parent = 0 );
protected:
  virtual void paintEvent( QPaintEvent * event );
};
