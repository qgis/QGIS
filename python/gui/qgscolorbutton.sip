

class QgsColorButton : QToolButton
{
%TypeHeaderCode
#include <qgscolorbutton.h>
%End

  public:
    /**
     * Construct a new color button.
     *
     * @param parent The parent QWidget for the dialog
     * @param cdt The title to show in the color chooser dialog
     * @param cdo Options for the color chooser dialog
     * @note changed in 1.9
     */
    QgsColorButton( QWidget *parent = 0, QString cdt = "Select Color", QColorDialog::ColorDialogOptions cdo = 0 );
    ~QgsColorButton();

    /**
     * Specify the current color. Will emit a colorChanged signal if the color is different to the previous.
     *
     * @param color the new color
     * @note added in 1.9
     */
    void setColor( const QColor &color );
    /**
     * Return the currently selected color.
     *
     * @return the currently selected color
     * @note added in 1.9
     */
    QColor color() const;

    /**
     * Specify the options for the color chooser dialog (e.g. alpha).
     *
     * @param cdo Options for the color chooser dialog
     * @note added in 1.9
     */
    void setColorDialogOptions( QColorDialog::ColorDialogOptions cdo );

    /**
     * Returns the options for the color chooser dialog.
     *
     * @return Options for the color chooser dialog
     * @note added in 1.9
     */
    QColorDialog::ColorDialogOptions colorDialogOptions();

    /**
     * Set the title, which the color chooser dialog will show.
     *
     * @param cdt Title for the color chooser dialog
     * @note added in 1.9
     */
    void setColorDialogTitle( QString cdt );

    /**
     * Returns the title, which the color chooser dialog shows.
     *
     * @return Title for the color chooser dialog
     * @note added in 1.9
     */
    QString colorDialogTitle();

  protected:
    void paintEvent( QPaintEvent *e );

  public slots:
    void onButtonClicked();

  signals:
    /**
     * Is emitted, whenever a new color is accepted. The color is always valid.
     * In case the new color is the same, no signal is emitted, to avoid infinite loops.
     *
     * @param color New color
     * @note added in 1.9
     */
    void colorChanged( const QColor &color );
};


class QgsColorButtonV2 : QPushButton
{
%TypeHeaderCode
#include <qgscolorbutton.h>
%End

  public:
    QgsColorButtonV2( QWidget* parent = 0 );
    QgsColorButtonV2( QString text, QWidget* parent = 0 );

    void setColor( const QColor &color );
    QColor color() const;
};
