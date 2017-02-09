
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

    void setRelation( const QgsRelation& relation, bool allowNullValue );

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
    //! Set the fields for which filter comboboxes will be created
    void setFilterFields( const QStringList& filterFields );

    //! determines the open form button is visible in the widget
    bool openFormButtonVisible();
    void setOpenFormButtonVisible( bool openFormButtonVisible );

    /**
     * Determines if the filters are chained
     *
     * @return True if filters are chained
     */
    bool chainFilters() const;

    /**
     * Set if filters are chained.
     * Chained filters restrict the option of subsequent filters based on the selection of a previous filter.
     *
     * @param chainFilters If chaining should be enabled
     */
    void setChainFilters( bool chainFilters );

    //! return the related feature (from the referenced layer)
    //! if no feature is related, it returns an invalid feature
    QgsFeature referencedFeature() const;

    /** Sets the widget to display in an indeterminate "mixed value" state.
     * @note added in QGIS 2.16
     */
    void showIndeterminateState();

    /**
     * Determines if a button for adding new features should be shown.
     *
     * @note added in QGIS 2.16
     */
    bool allowAddFeatures() const;

    /**
     * Determines if a button for adding new features should be shown.
     *
     * @note added in QGIS 2.16
     */
    void setAllowAddFeatures( bool allowAddFeatures );

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
    void foreignKeyChanged( const QVariant& );
};
