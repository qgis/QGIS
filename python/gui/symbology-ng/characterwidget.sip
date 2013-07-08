class CharacterWidget : QWidget
{
%TypeHeaderCode
#include <characterwidget.h>
%End
  public:
    CharacterWidget( QWidget *parent = 0 );
    QSize sizeHint() const;

    int getColumns() const;
    int getSquareSize() const;

  public slots:
    void updateFont( const QFont &font );
    void updateSize( double fontSize );
    void updateStyle( const QString &fontStyle );
    void updateFontMerging( bool enable );
    void updateColumns( int cols );

  signals:
    void characterSelected( const QChar &character );

  protected:
    void mouseMoveEvent( QMouseEvent *event );
    void mousePressEvent( QMouseEvent *event );
    void paintEvent( QPaintEvent *event );
};

