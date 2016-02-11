class CharacterWidget : QWidget
{
%TypeHeaderCode
#include <characterwidget.h>
%End
  public:
    CharacterWidget( QWidget *parent /TransferThis/ = 0 );
    QSize sizeHint() const;

    int getColumns() const;
    int getSquareSize() const;

  public slots:
    void updateFont( const QFont &font );
    void updateSize( double fontSize );
    void updateStyle( const QString &fontStyle );
    void updateFontMerging( bool enable );
    void updateColumns( int cols );
    void setCharacter( QChar character );

  signals:
    void characterSelected( QChar character );

  protected:
    void mouseMoveEvent( QMouseEvent *event );
    void mousePressEvent( QMouseEvent *event );
    void paintEvent( QPaintEvent *event );
};

