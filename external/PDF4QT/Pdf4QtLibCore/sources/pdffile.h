//    Copyright (C) 2019-2021 Jakub Melka
//
//    This file is part of PDF4QT.
//
//    PDF4QT is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    with the written consent of the copyright owner, any later version.
//
//    PDF4QT is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with PDF4QT.  If not, see <https://www.gnu.org/licenses/>.

#ifndef PDFFILE_H
#define PDFFILE_H

#include "pdfobject.h"

#include <QColor>
#include <QDateTime>

namespace pdf
{
class PDFObjectStorage;

/// File identifier according section 14.4 of PDF 2.0 specification.
/// Each identifier consists of two parts - permanent identifier, which
/// is unique identifier based on original document, and changing identifier,
/// which is updated when document is being modified.
class PDF4QTLIBCORESHARED_EXPORT PDFFileIdentifier
{
public:
    explicit inline PDFFileIdentifier() = default;

    const QByteArray& getPermanentIdentifier() const { return m_permanentIdentifier; }
    const QByteArray& getChangingIdentifier() const { return m_changingIdentifier; }

    static PDFFileIdentifier parse(const PDFObjectStorage* storage, PDFObject object);

private:
    QByteArray m_permanentIdentifier;
    QByteArray m_changingIdentifier;
};

/// Provides description of collection item property field. It describes it's
/// kind, data type, if content of the property should be presented to the user,
/// and ordering, visibility and editability.
class PDF4QTLIBCORESHARED_EXPORT PDFCollectionField
{
public:
    explicit inline PDFCollectionField() = default;

    enum class Kind
    {
        Invalid,
        TextField,
        DateField,
        NumberField,
        FileName,
        Description,
        ModifiedDate,
        CreationDate,
        Size,
        CompressedSize
    };

    enum class Value
    {
        Invalid,
        TextString,
        DateTime,
        Number
    };

    Kind getKind() const { return m_kind; }
    Value getValue() const { return m_value; }
    QString getFieldName() const { return m_fieldName; }
    PDFInteger getOrder() const { return m_order; }
    bool isVisible() const { return m_visible; }
    bool isEditable() const { return m_editable; }

    static PDFCollectionField parse(const PDFObjectStorage* storage, PDFObject object);

private:
    Kind m_kind = Kind::Invalid;
    Value m_value = Value::Invalid;
    QString m_fieldName;
    PDFInteger m_order = 0;
    bool m_visible = true;
    bool m_editable = false;
};

/// Collection schema. Contains a list of defined fields.
/// Schema can be queried for field definition.
class PDF4QTLIBCORESHARED_EXPORT PDFCollectionSchema
{
public:
    explicit inline PDFCollectionSchema() = default;

    /// Returns true, if schema is valid, i.e. some fields
    /// are defined.
    bool isValid() const { return !m_fields.empty(); }

    /// Returns collection field. This function always returns
    /// valid field definition. If field can't be found, then
    /// default field (invalid) is returned.
    /// \param key Key
    const PDFCollectionField* getField(const QByteArray& key) const;

    /// Returns true, if field definition with given key is valid.
    /// \param key Key
    bool isFieldValid(const QByteArray& key) const { return getField(key)->getKind() != PDFCollectionField::Kind::Invalid;}

    static PDFCollectionSchema parse(const PDFObjectStorage* storage, PDFObject object);

private:
    std::map<QByteArray, PDFCollectionField> m_fields;
};

/// Collection of file attachments. In the PDF file, attached files
/// can be grouped in collection (if they are related to each other).
class PDF4QTLIBCORESHARED_EXPORT PDFCollection
{
public:
    explicit inline PDFCollection() = default;

    enum class ViewMode
    {
        /// Collection items should be displayed in details mode,
        /// for example, multi-column table, which contains icon,
        /// file name, and all properties listed in the schema.
        Details,

        /// Collection should be presented in tile mode, each file
        /// should have icon, and some information from the schema
        /// dictionary.
        Tiles,

        /// Collection should be initially hidden and should be presented
        /// to the user if user performs some explicit action.
        Hidden,

        /// Collection should be presented via Navigation entry
        Navigation
    };

    enum class SplitMode
    {
        /// List of files nad preview should be splitted horizontally
        Horizontally,
        /// List of files and preview should be splitted vertically
        Vertically,
        /// No preview should be shown
        None
    };

    enum ColorRole
    {
        Background,
        CardBackground,
        CardBorder,
        PrimaryText,
        SecondaryText,
        LastColorRole
    };

    struct SortColumn
    {
        QByteArray field;
        bool ascending = true;
    };
    using SortColumns = std::vector<SortColumn>;

    /// Returns true, if collection schema is valid. If not,
    /// default file properties should be used.
    /// \returns true, if collection's schema is valid
    bool isSchemaValid() const { return m_schema.isValid(); }

    /// Returns collection field. This function always returns
    /// valid field definition. If field can't be found, then
    /// default field (invalid) is returned.
    /// \param key Key
    const PDFCollectionField* getField(const QByteArray& key) const { return m_schema.getField(key); }

    /// Returns true, if field definition with given key is valid.
    /// \param key Key
    bool isFieldValid(const QByteArray& key) const { return m_schema.isFieldValid(key); }

    /// Returns file field schema. Fields are properties of the files,
    /// such as size, description, date/time etc.
    const PDFCollectionSchema& getSchema() const { return m_schema; }

    /// Returns initial document, which shall be presented to the user
    /// in the ui (so interactive processor should display this file first).
    const QByteArray& getDocument() const { return m_document; }

    /// Returns view mode of the collection. It defines how collection
    /// should appear in user interface.
    ViewMode getViewMode() const { return m_viewMode; }

    /// Returns reference to a navigator (if it exists)
    PDFObjectReference getNavigator() const { return m_navigator; }

    /// Returns color for given role. Invalid color can be returned.
    /// If this occurs, default color should be used.
    /// \param role Color role
    QColor getColor(ColorRole role) const { return m_colors.at(role); }

    /// Returns sorting information (list of columns, defining
    /// sort key). If it is empty, default sorting should occur.
    const SortColumns& getSortColumns() const { return m_sortColumns; }

    /// If sort columns are empty, then is default sorting ascending?
    bool isSortAscending() const { return m_sortAscending; }

    /// Returns folder root (if collection has folders)
    PDFObjectReference getFolderRoot() const { return m_folderRoot; }

    /// Returns split mode of list of files and preview
    SplitMode getSplitMode() const { return m_splitMode; }

    /// Returns split proportion
    PDFReal getSplitPropertion() const { return m_splitProportion; }

    static PDFCollection parse(const PDFObjectStorage* storage, PDFObject object);

private:
    PDFCollectionSchema m_schema;
    QByteArray m_document;
    ViewMode m_viewMode = ViewMode::Details;
    PDFObjectReference m_navigator;
    std::array<QColor, LastColorRole> m_colors;
    SortColumns m_sortColumns;
    bool m_sortAscending = true;
    PDFObjectReference m_folderRoot;
    SplitMode m_splitMode = SplitMode::None;
    PDFReal m_splitProportion = 30;
};

/// Collection folder. Can contain subfolders and files.
class PDF4QTLIBCORESHARED_EXPORT PDFCollectionFolder
{
public:
    explicit inline PDFCollectionFolder() = default;

    PDFInteger getInteger() const { return m_ID; }
    const QString& getName() const { return m_name; }
    const QString& getDescription() const { return m_description; }
    PDFObjectReference getParent() const { return m_parent; }
    PDFObjectReference getChild() const { return m_child; }
    PDFObjectReference getNext() const { return m_next; }
    PDFObjectReference getCollection() const { return m_collection; }
    PDFObjectReference getThumbnail() const { return m_thumbnail; }
    const QDateTime& getCreatedDate() const { return m_created; }
    const QDateTime& getModifiedDate() const { return m_modified; }
    const std::vector<PDFInteger>& getFreeIds() const { return m_freeIds; }

    static PDFCollectionFolder parse(const PDFObjectStorage* storage, PDFObject object);

private:
    PDFInteger m_ID = 0;
    QString m_name;
    QString m_description;
    PDFObjectReference m_parent;
    PDFObjectReference m_child;
    PDFObjectReference m_next;
    PDFObjectReference m_collection;
    PDFObjectReference m_thumbnail;
    QDateTime m_created;
    QDateTime m_modified;
    std::vector<PDFInteger> m_freeIds;
};

/// Collection item. Contains properties of the collection item,
/// for example, embedded file.
class PDF4QTLIBCORESHARED_EXPORT PDFCollectionItem
{
public:
    explicit inline PDFCollectionItem() = default;
    explicit inline PDFCollectionItem(const PDFObject& object) : m_object(object) { }

    /// Returns true, if collection item entry is valid
    bool isValid() const { return m_object.isDictionary(); }

    /// Returns string from property key. If property is invalid, empty
    /// string is returned, no exception is thrown.
    /// \param key Key
    /// \param storage Storage
    QString getString(const QByteArray& key, const PDFObjectStorage* storage) const;

    /// Returns date/time from property key. If property is invalid, invalid
    /// QDateTime is returned, no exception is thrown.
    /// \param key Key
    /// \param storage Storage
    QDateTime getDateTime(const QByteArray& key, const PDFObjectStorage* storage) const;

    /// Returns integer from property key. If property is invalid, zero
    /// integer is returned, no exception is thrown.
    /// \param key Key
    /// \param storage Storage
    PDFInteger getNumber(const QByteArray& key, const PDFObjectStorage* storage) const;

    /// Returns prefix string from property key. If property is invalid, empty
    /// string is returned, no exception is thrown.
    /// \param key Key
    /// \param storage Storage
    QString getPrefixString(const QByteArray& key, const PDFObjectStorage* storage) const;

private:
    PDFObject m_object;
};

/// Collection navigator. It contains modes of display. Interactive
/// PDF processor should display first layout it is capable of.
class PDF4QTLIBCORESHARED_EXPORT PDFCollectionNavigator
{
public:
    explicit inline PDFCollectionNavigator() = default;

    enum Layout
    {
        None        = 0x0000,
        Invalid     = 0x0001,
        Details     = 0x0002,
        Tile        = 0x0004,
        Hidden      = 0x0008,
        FilmStrip   = 0x0010,
        FreeForm    = 0x0020,
        Linear      = 0x0040,
        Tree        = 0x0080
    };
    Q_DECLARE_FLAGS(Layouts, Layout)

    /// Returns true, if navigator has valid layout
    bool hasLayout() const { return (m_layouts != None) && !m_layouts.testFlag(Invalid); }

    /// Returns current layouts
    Layouts getLayout() const { return m_layouts; }

    static PDFCollectionNavigator parse(const PDFObjectStorage* storage, PDFObject object);

private:
    Layouts m_layouts = None;
};

class PDF4QTLIBCORESHARED_EXPORT PDFEmbeddedFile
{
public:
    explicit PDFEmbeddedFile() = default;

    bool isValid() const { return m_stream.isStream(); }
    const QByteArray& getSubtype() const { return m_subtype; }
    PDFInteger getSize() const { return m_size; }
    const QDateTime& getCreationDate() const { return m_creationDate; }
    const QDateTime& getModifiedDate() const { return m_modifiedDate; }
    const QByteArray& getChecksum() const { return m_checksum; }
    const PDFStream* getStream() const { return m_stream.getStream(); }

    static PDFEmbeddedFile parse(const PDFObjectStorage* storage, PDFObject object);

private:
    PDFObject m_stream;
    QByteArray m_subtype;
    PDFInteger m_size = -1;
    QDateTime m_creationDate;
    QDateTime m_modifiedDate;
    QByteArray m_checksum;
};

/// File specification
class PDF4QTLIBCORESHARED_EXPORT PDFFileSpecification
{
public:
    explicit PDFFileSpecification() = default;

    struct RelatedFile
    {
        QByteArray name;
        PDFObjectReference fileReference;
    };

    using RelatedFiles = std::vector<RelatedFile>;

    enum class AssociatedFileRelationship
    {
        Unspecified,
        Source,
        Data,
        Alternative,
        Supplement,
        EncryptedPayload,
        FormData,
        Schema
    };

    /// Returns platform file name as string. It looks into the UF, F,
    /// and platform names and selects the appropriate one. If error
    /// occurs. then empty string is returned.
    QString getPlatformFileName() const;

    /// Returns platform file.
    const PDFEmbeddedFile* getPlatformFile() const;

    const QByteArray& getFileSystem() const { return m_fileSystem; }
    const QByteArray& getF() const { return m_F; }
    const QString& getUF() const { return m_UF; }
    const QByteArray& getDOS() const { return m_DOS; }
    const QByteArray& getMac() const { return m_Mac; }
    const QByteArray& getUnix() const { return m_Unix; }
    const PDFFileIdentifier& getFileIdentifier() const { return m_id; }
    bool isVolatile() const { return m_volatile; }
    const QString& getDescription() const { return m_description; }
    PDFObjectReference getSelfReference() const { return m_selfReference; }
    PDFObjectReference getCollection() const { return m_collection; }
    PDFObjectReference getThumbnail() const { return m_thumbnailReference; }
    const std::map<QByteArray, PDFEmbeddedFile>& getEmbeddedFiles() const { return m_embeddedFiles; }
    const std::map<QByteArray, RelatedFiles>& getRelatedFiles() const { return m_relatedFiles; }
    const PDFObject& getEncryptedPayloadDictionary() const { return m_encryptedPayload; }
    AssociatedFileRelationship getAssociatedFileRelationship() const { return m_associatedFileRelationship; }

    static PDFFileSpecification parse(const PDFObjectStorage* storage, PDFObject object);

private:
    /// Name of the file system used to interpret this file specification,
    /// usually, it is URL (this is only file system defined in PDF specification 1.7).
    QByteArray m_fileSystem;

    /// File specification string (for backward compatibility). If file system is URL,
    /// it contains unified resource locator.
    QByteArray m_F;

    /// File specification string as unicode.
    QString m_UF;

    QByteArray m_DOS;
    QByteArray m_Mac;
    QByteArray m_Unix;

    PDFFileIdentifier m_id;

    /// Is file volatile? I.e it is, for example, link to a video file from online camera?
    /// If this boolean is true, then file should never be cached.
    bool m_volatile = false;

    /// Description of the file (for example, if file is embedded file stream)
    QString m_description;

    /// Self reference
    PDFObjectReference m_selfReference;

    /// Collection item dictionary reference
    PDFObjectReference m_collection;

    /// Thumbnail reference
    PDFObjectReference m_thumbnailReference;

    /// Embedded files
    std::map<QByteArray, PDFEmbeddedFile> m_embeddedFiles;

    /// Related files for embedded files
    std::map<QByteArray, RelatedFiles> m_relatedFiles;

    /// Encrypted payload dictionary (used in document wrapper)
    PDFObject m_encryptedPayload;

    AssociatedFileRelationship m_associatedFileRelationship = AssociatedFileRelationship::Unspecified;
};

}   // namespace pdf

#endif // PDFFILE_H
