#include <QDial>
#include <QPainter>
#include <QPaintEvent>
#include <QSize>

class EQDial : public QDial
{
    Q_OBJECT
public:
    EQDial( QWidget *parent = NULL );
protected:
    virtual void paintEvent( QPaintEvent * event );
};
