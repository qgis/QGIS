/** \ingroup gui
 * \class QgsFilterLegendButton
 * A tool button that allows enabling or disabling legend filter by contents of the map.
 * An additional pop down menu allows defining a boolean expression to refine the filtering.
 * @note added in 2.14
 */

class QgsLegendFilterButton: public QToolButton
{
%TypeHeaderCode
#include <qgslegendfilterbutton.h>
%End

  public:
    /**
     * Construct a new filter legend button
     *
     * @param parent The parent QWidget
     */
    QgsLegendFilterButton( QWidget* parent = 0 );
    ~QgsLegendFilterButton();

    /**
     * Returns the current text used as filter expression
     */
    QString expressionText() const;

    /**
     * Sets the current text used as filter expression.
     * This will update the menu
     */
    void setExpressionText( const QString& expression );

    /**
     * Returns the current associated vectorLayer
     * May be null
     */
    QgsVectorLayer* vectorLayer() const;
    /**
     * Sets the associated vectorLayer
     * May be null
     */
    void setVectorLayer( QgsVectorLayer* layer );

  signals:
    /**
     * Emitted when the expression text changes
     */
    void expressionTextChanged();
};
