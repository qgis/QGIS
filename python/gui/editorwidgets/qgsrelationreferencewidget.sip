
class QgsRelationReferenceWidget : QWidget
{
%TypeHeaderCode
#include <qgsrelationreferencewidget.h>
%End

  public:
    enum CanvasExtent
    {
      Fixed,
      Pan,
      Scale
    };

    explicit QgsRelationReferenceWidget( QWidget* parent /TransferThis/ );

    ~QgsRelationReferenceWidget();

    void setRelation( QgsRelation relation, bool allowNullValue );

    void setRelationEditable( bool editable );

    //! this sets the related feature using from the foreign key
    void setForeignKey( const QVariant &value );

    //! returns the related feature foreign key
    QVariant foreignKey();

    void setEditorContext( const QgsAttributeEditorContext& context, QgsMapCanvas* canvas, QgsMessageBar* messageBar );

    //! determines if the form of the related feature will be shown
    bool embedForm();
    void setEmbedForm( bool display );

    //! determines if the foreign key is shown in a combox box or a read-only line edit
    bool readOnlySelector();
    void setReadOnlySelector( bool readOnly );

    //! determines if the widge offers the possibility to select the related feature on the map (using a dedicated map tool)
    bool allowMapIdentification();
    void setAllowMapIdentification( bool allowMapIdentification );

    //! If the widget will order the combobox entries by value
    bool orderByValue();
    //! Set if the widget will order the combobox entries by value
    void setOrderByValue( bool orderByValue );

    //! determines the open form button is visible in the widget
    bool openFormButtonVisible();
    void setOpenFormButtonVisible( bool openFormButtonVisible );

    //! return the related feature (from the referenced layer)
    //! if no feature is related, it returns an invalid feature
    QgsFeature referencedFeature();

  public slots:
    //! open the form of the related feature in a new dialog
    void openForm();

    //! activate the map tool to select a new related feature on the map
    void mapIdentification();

    //! unset the currently related feature
    void deleteForeignKey();

  protected:
    virtual void showEvent( QShowEvent* e );

    void init();

  signals:
    void foreignKeyChanged( QVariant );
};
