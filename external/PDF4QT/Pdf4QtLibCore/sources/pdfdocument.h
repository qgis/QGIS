//    Copyright (C) 2018-2021 Jakub Melka
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

#ifndef PDFDOCUMENT_H
#define PDFDOCUMENT_H

#include "pdfglobal.h"
#include "pdfobject.h"
#include "pdfcatalog.h"
#include "pdfsecurityhandler.h"

#include <QColor>
#include <QTransform>
#include <QDateTime>

#include <optional>

namespace pdf
{
class PDFDocument;
class PDFDocumentBuilder;

/// Storage for objects. This class is not thread safe for writing (calling non-const functions). Caller must ensure
/// locking, if this object is used from multiple threads. Calling const functions should be thread safe.
class PDF4QTLIBCORESHARED_EXPORT PDFObjectStorage
{
public:
    inline PDFObjectStorage() = default;

    inline PDFObjectStorage(const PDFObjectStorage&) = default;
    inline PDFObjectStorage(PDFObjectStorage&&) = default;

    inline PDFObjectStorage& operator=(const PDFObjectStorage&) = default;
    inline PDFObjectStorage& operator=(PDFObjectStorage&&) = default;

    bool operator==(const PDFObjectStorage& other) const;
    bool operator!=(const PDFObjectStorage& other) const { return !(*this == other); }

    struct Entry
    {
        constexpr inline explicit Entry() = default;
        inline explicit Entry(PDFInteger generation, PDFObject object) : generation(generation), object(std::move(object)) { }

        inline bool operator==(const Entry& other) const { return generation == other.generation && object == other.object; }
        inline bool operator!=(const Entry& other) const { return !(*this == other); }

        PDFInteger generation = 0;
        PDFObject object;
    };

    using PDFObjects = std::vector<Entry>;

    explicit PDFObjectStorage(PDFObjects&& objects, PDFObject&& trailerDictionary, PDFSecurityHandlerPointer&& securityHandler) :
        m_objects(std::move(objects)),
        m_trailerDictionary(std::move(trailerDictionary)),
        m_securityHandler(std::move(securityHandler))
    {

    }

    /// Returns object from the object storage. If invalid reference is passed,
    /// then null object is returned (no exception is thrown).
    const PDFObject& getObject(PDFObjectReference reference) const;

    /// If object is reference, the dereference attempt is performed
    /// and object is returned. If it is not a reference, then self
    /// is returned. If dereference attempt fails, then null object
    /// is returned (no exception is thrown).
    const PDFObject& getObject(const PDFObject& object) const;

    /// Returns dictionary from an object. If object is not a dictionary,
    /// then nullptr is returned (no exception is thrown).
    const PDFDictionary* getDictionaryFromObject(const PDFObject& object) const;

    /// Returns object by reference. If dereference attempt fails, then null object
    /// is returned (no exception is thrown).
    const PDFObject& getObjectByReference(PDFObjectReference reference) const;

    /// Returns array of objects stored in this storage
    const PDFObjects& getObjects() const { return m_objects; }

    /// Returns array of objects stored in this storage
    PDFObjects& getObjects() { return m_objects; }

    /// Sets array of objects
    void setObjects(PDFObjects&& objects) { m_objects = qMove(objects); }

    /// Returns trailer dictionary
    const PDFObject& getTrailerDictionary() const { return m_trailerDictionary; }

    /// Returns security handler associated with these objects
    const PDFSecurityHandler* getSecurityHandler() const { return m_securityHandler.data(); }

    /// Sets security handler associated with these objects
    void setSecurityHandler(PDFSecurityHandlerPointer handler) { m_securityHandler = qMove(handler); }

    /// Adds a new object to the object list. This function
    /// is not thread safe, do not call it from multiple threads.
    /// \param object Object to be added
    /// \returns Reference to new object
    PDFObjectReference addObject(PDFObject object);

    /// Sets object to object storage. Reference must exist.
    /// \param reference Reference to object
    /// \param object New value of object
    void setObject(PDFObjectReference reference, PDFObject object);

    /// Updates trailer dictionary. Preserves items which are not in a new
    /// dictionary \p trailerDictionary. It merges new dictionary to the
    /// old one.
    /// \param trailerDictionary New trailer dictionary
    void updateTrailerDictionary(PDFObject trailerDictionary);

    /// Returns the decoded stream. If stream data cannot be decoded,
    /// then empty byte array is returned.
    /// \param stream Stream to be decoded
    QByteArray getDecodedStream(const PDFStream* stream) const;

    /// Set trailer dictionary
    /// \param object Object defining trailer dictionary
    void setTrailerDictionary(const PDFObject& object) { m_trailerDictionary = object; }

private:
    PDFObjects m_objects;
    PDFObject m_trailerDictionary;
    PDFSecurityHandlerPointer m_securityHandler;
};

/// Loads data from the object contained in the PDF document, such as integers,
/// bools, ... This object has two sets of functions - first one with default values,
/// then if object with valid data is not found, default value is used, and second one,
/// without default value, if valid data are not found, then exception is thrown.
/// This class uses Decorator design pattern.
class PDF4QTLIBCORESHARED_EXPORT PDFDocumentDataLoaderDecorator
{
public:
    explicit PDFDocumentDataLoaderDecorator(const PDFDocument* document);
    inline explicit PDFDocumentDataLoaderDecorator(const PDFObjectStorage* storage) : m_storage(storage) { }
    inline ~PDFDocumentDataLoaderDecorator() = default;

    /// Reads a name from the object, if it is possible. If object is not a name,
    /// then empty byte array is returned.
    /// \param object Object, can be an indirect reference to object (it is dereferenced)
    QByteArray readName(const PDFObject& object) const;

    /// Reads a string from the object, if it is possible. If object is not a string,
    /// then empty byte array is returned.
    /// \param object Object, can be an indirect reference to object (it is dereferenced)
    QByteArray readString(const PDFObject& object) const;

    /// Reads an integer from the object, if it is possible.
    /// \param object Object, can be an indirect reference to object (it is dereferenced)
    /// \param defaultValue Default value
    PDFInteger readInteger(const PDFObject& object, PDFInteger defaultValue) const;

    /// Reads a real number from the object, if it is possible. If integer appears as object,
    /// then it is converted to real number.
    /// \param object Object, can be an indirect reference to object (it is dereferenced)
    /// \param defaultValue Default value
    PDFReal readNumber(const PDFObject& object, PDFReal defaultValue) const;

    /// Reads a boolean from the object, if it is possible.
    /// \param object Object, can be an indirect reference to object (it is dereferenced)
    /// \param defaultValue Default value
    bool readBoolean(const PDFObject& object, bool defaultValue) const;

    /// Reads a text string from the object, if it is possible.
    /// \param object Object, can be an indirect reference to object (it is dereferenced)
    /// \param defaultValue Default value
    QString readTextString(const PDFObject& object, const QString& defaultValue) const;

    /// Reads a rectangle from the object, if it is possible.
    /// \param object Object, can be an indirect reference to object (it is dereferenced)
    /// \param defaultValue Default value
    QRectF readRectangle(const PDFObject& object, const QRectF& defaultValue) const;

    /// Reads enum from name object, if it is possible.
    /// \param object Object, can be an indirect reference to object (it is dereferenced)
    /// \param begin Begin of the enum search array
    /// \param end End of the enum search array
    /// \param default value Default value
    template<typename Enum, typename Iterator>
    Enum readEnumByName(const PDFObject& object, Iterator begin, Iterator end, Enum defaultValue) const
    {
        const PDFObject& dereferencedObject = m_storage->getObject(object);
        if (dereferencedObject.isName() || dereferencedObject.isString())
        {
            QByteArray name = dereferencedObject.getString();

            for (Iterator it = begin; it != end; ++it)
            {
                if (name == (*it).first)
                {
                    return (*it).second;
                }
            }
        }

        return defaultValue;
    }

    /// Tries to read array of real values. Reads as much values as possible.
    /// If array size differs, then nothing happens.
    /// \param object Array of integers
    /// \param first First iterator
    /// \param second Second iterator
    template<typename T>
    void readNumberArray(const PDFObject& object, T first, T last)
    {
        const PDFObject& dereferencedObject = m_storage->getObject(object);
        if (dereferencedObject.isArray())
        {
            const PDFArray* array = dereferencedObject.getArray();

            size_t distance = std::distance(first, last);
            if (array->getCount() == distance)
            {
                T it = first;
                for (size_t i = 0; i < distance; ++i)
                {
                    *it = readNumber(array->getItem(i), *it);
                    ++it;
                }
            }
        }
    }

    /// Tries to read array of real values from dictionary. Reads as much values as possible.
    /// If array size differs, or entry dictionary doesn't exist, then nothing happens.
    /// \param dictionary Dictionary with desired values
    /// \param key Entry key
    /// \param first First iterator
    /// \param second Second iterator
    template<typename T>
    void readNumberArrayFromDictionary(const PDFDictionary* dictionary, const char* key, T first, T last)
    {
        if (dictionary->hasKey(key))
        {
            readNumberArray(dictionary->get(key), first, last);
        }
    }

    /// Tries to read matrix from the dictionary. If matrix entry is not present, default value is returned.
    /// If it is present and invalid, exception is thrown.
    QTransform readMatrixFromDictionary(const PDFDictionary* dictionary, const char* key, QTransform defaultValue) const;

    /// Tries to read array of real values from dictionary. If entry dictionary doesn't exist,
    /// or error occurs, default value is returned.
    std::vector<PDFReal> readNumberArrayFromDictionary(const PDFDictionary* dictionary, const char* key, std::vector<PDFReal> defaultValue = std::vector<PDFReal>()) const;

    /// Tries to read array of integer values from dictionary. If entry dictionary doesn't exist,
    /// or error occurs, empty array is returned.
    std::vector<PDFInteger> readIntegerArrayFromDictionary(const PDFDictionary* dictionary, const char* key) const;

    /// Reads number from dictionary. If dictionary entry doesn't exist, or error occurs, default value is returned.
    /// \param dictionary Dictionary containing desired data
    /// \param key Entry key
    /// \param defaultValue Default value
    PDFReal readNumberFromDictionary(const PDFDictionary* dictionary, const char* key, PDFReal defaultValue) const;

    /// Reads number from dictionary. If dictionary entry doesn't exist, or error occurs, default value is returned.
    /// \param dictionary Dictionary containing desired data
    /// \param key Entry key
    /// \param defaultValue Default value
    PDFReal readNumberFromDictionary(const PDFDictionary* dictionary, const QByteArray& key, PDFReal defaultValue) const;

    /// Reads integer from dictionary. If dictionary entry doesn't exist, or error occurs, default value is returned.
    /// \param dictionary Dictionary containing desired data
    /// \param key Entry key
    /// \param defaultValue Default value
    PDFInteger readIntegerFromDictionary(const PDFDictionary* dictionary, const char* key, PDFInteger defaultValue) const;

    /// Reads a text string from the dictionary, if it is possible.
    /// \param dictionary Dictionary containing desired data
    /// \param key Entry key
    /// \param defaultValue Default value
    QString readTextStringFromDictionary(const PDFDictionary* dictionary, const char* key, const QString& defaultValue) const;

    /// Tries to read array of references from dictionary. If entry dictionary doesn't exist,
    /// or error occurs, empty array is returned.
    std::vector<PDFObjectReference> readReferenceArrayFromDictionary(const PDFDictionary* dictionary, const char* key) const;

    /// Reads number array from dictionary. Reads all values. If some value is not
    /// real number (or integer number), default value is returned. Default value is also returned,
    /// if \p object is invalid.
    /// \param object Object containing array of numbers
    std::vector<PDFReal> readNumberArray(const PDFObject& object, std::vector<PDFReal> defaultValue = std::vector<PDFReal>()) const;

    /// Reads integer array from dictionary. Reads all values. If some value is not
    /// integer number, empty array is returned. Empty array is also returned,
    /// if \p object is invalid.
    /// \param object Object containing array of numbers
    std::vector<PDFInteger> readIntegerArray(const PDFObject& object) const;

    /// Reads reference. If error occurs, then invalid reference is returned.
    /// \param object Object containing reference
    PDFObjectReference readReference(const PDFObject& object) const;

    /// Reads reference from dictionary. If error occurs, then invalid reference is returned.
    /// \param dictionary Dictionary containing desired data
    /// \param key Entry key
    PDFObjectReference readReferenceFromDictionary(const PDFDictionary* dictionary, const char* key) const;

    /// Reads reference array. Reads all values. If error occurs,
    /// then empty array is returned.
    /// \param object Object containing array of references
    std::vector<PDFObjectReference> readReferenceArray(const PDFObject& object) const;

    /// Reads name array. Reads all values. If error occurs,
    /// then empty array is returned.
    /// \param object Object containing array of references
    std::vector<QByteArray> readNameArray(const PDFObject& object) const;

    /// Reads string array. Reads all values. If error occurs,
    /// then empty array is returned.
    /// \param object Object containing array of references
    std::vector<QByteArray> readStringArray(const PDFObject& object) const;

    /// Reads name array from dictionary. Reads all values. If error occurs,
    /// then empty array is returned.
    /// \param dictionary Dictionary containing desired data
    /// \param key Entry key
    std::vector<QByteArray> readNameArrayFromDictionary(const PDFDictionary* dictionary, const char* key) const;

    /// Reads boolean from dictionary. If dictionary entry doesn't exist, or error occurs, default value is returned.
    /// \param dictionary Dictionary containing desired data
    /// \param key Entry key
    /// \param defaultValue Default value
    bool readBooleanFromDictionary(const PDFDictionary* dictionary, const char* key, bool defaultValue) const;

    /// Reads a name from dictionary. If dictionary entry doesn't exist, or error occurs, empty byte array is returned.
    /// \param dictionary Dictionary containing desired data
    /// \param key Entry key
    QByteArray readNameFromDictionary(const PDFDictionary* dictionary, const char* key) const;

    /// Reads a string from dictionary. If dictionary entry doesn't exist, or error occurs, empty byte array is returned.
    /// \param dictionary Dictionary containing desired data
    /// \param key Entry key
    QByteArray readStringFromDictionary(const PDFDictionary* dictionary, const char* key) const;

    /// Reads string array from dictionary. Reads all values. If error occurs,
    /// then empty array is returned.
    /// \param dictionary Dictionary containing desired data
    /// \param key Entry key
    std::vector<QByteArray> readStringArrayFromDictionary(const PDFDictionary* dictionary, const char* key) const;

    /// Reads string list. If error occurs, empty list is returned.
    QStringList readTextStringList(const PDFObject& object);

    /// Reads RGB color from dictionary
    QColor readRGBColorFromDictionary(const PDFDictionary* dictionary, const char* key, QColor defaultColor);

    /// Reads list of object, using parse function defined in object
    template<typename Object>
    std::vector<Object> readObjectList(PDFObject object)
    {
        std::vector<Object> result;
        object = m_storage->getObject(object);
        if (object.isArray())
        {
            const PDFArray* array = object.getArray();
            const size_t count = array->getCount();
            result.reserve(count);

            for (size_t i = 0; i < count; ++i)
            {
                result.emplace_back(Object::parse(m_storage, array->getItem(i)));
            }
        }

        return result;
    }

    /// Reads optional string from dictionary. If key is not in dictionary,
    /// then empty optional is returned.
    /// \param dictionary Dictionary containing desired data
    /// \param key Entry key
    std::optional<QByteArray> readOptionalStringFromDictionary(const PDFDictionary* dictionary, const char* key) const;

    /// Reads optionalinteger from dictionary. If dictionary entry doesn't exist, or error occurs, empty optional is returned.
    /// \param dictionary Dictionary containing desired data
    /// \param key Entry key
    std::optional<PDFInteger> readOptionalIntegerFromDictionary(const PDFDictionary* dictionary, const char* key) const;

private:
    const PDFObjectStorage* m_storage;
};

/// PDF document main class.
class PDF4QTLIBCORESHARED_EXPORT PDFDocument
{
    Q_DECLARE_TR_FUNCTIONS(pdf::PDFDocument)

public:
    explicit PDFDocument() = default;
    ~PDFDocument();

    bool operator==(const PDFDocument& other) const;
    bool operator!=(const PDFDocument& other) const { return !(*this == other); }

    const PDFObjectStorage& getStorage() const { return m_pdfObjectStorage; }

    /// Returns info about the document (title, author, etc.)
    const PDFDocumentInfo* getInfo() const { return &m_info; }

    /// Returns document id part with given index. If index is invalid,
    /// then empty id is returned.
    QByteArray getIdPart(size_t index) const;

    /// If object is reference, the dereference attempt is performed
    /// and object is returned. If it is not a reference, then self
    /// is returned. If dereference attempt fails, then null object
    /// is returned (no exception is thrown).
    const PDFObject& getObject(const PDFObject& object) const;

    /// Returns dictionary from an object. If object is not a dictionary,
    /// then nullptr is returned (no exception is thrown).
    const PDFDictionary* getDictionaryFromObject(const PDFObject& object) const;

    /// Returns object by reference. If dereference attempt fails, then null object
    /// is returned (no exception is thrown).
    const PDFObject& getObjectByReference(PDFObjectReference reference) const;

    /// Returns the document catalog
    const PDFCatalog* getCatalog() const { return &m_catalog; }

    /// Returns the decoded stream. If stream data cannot be decoded,
    /// then empty byte array is returned.
    /// \param stream Stream to be decoded
    QByteArray getDecodedStream(const PDFStream* stream) const;

    /// Returns the trailer dictionary
    const PDFDictionary* getTrailerDictionary() const;

    /// Returns version of the PDF document. Version can be taken from catalog,
    /// or from PDF file header. Version from catalog has precedence over version from
    /// header.
    QByteArray getVersion() const;

    explicit PDFDocument(PDFObjectStorage&& storage, PDFVersion version, QByteArray sourceDataHash) :
        m_pdfObjectStorage(std::move(storage)),
        m_sourceDataHash(std::move(sourceDataHash))
    {
        init();

        m_info.version = version;
    }

    /**
     * @brief Retrieves the hash of the source data.
     *
     * This function returns the hash derived from the source data
     * from which the document was originally read.
     *
     * @return Hash value of the source data.
     */
    const QByteArray& getSourceDataHash() const { return m_sourceDataHash; }

private:
    friend class PDFDocumentReader;
    friend class PDFDocumentBuilder;
    friend class PDFOptimizer;

    /// Initialize data based on object in the storage.
    /// Can throw exception if error is detected.
    void init();

    /// Initialize the document info from the trailer dictionary.
    /// If document info is not present, then default document
    /// info is used. If error is detected, exception is thrown.
    void initInfo();

    /// Storage of objects
    PDFObjectStorage m_pdfObjectStorage;

    /// Info about the PDF document
    PDFDocumentInfo m_info;

    /// Catalog object
    PDFCatalog m_catalog;

    /// Hash of the source byte array's data,
    /// from which the document was created.
    QByteArray m_sourceDataHash;
};

using PDFDocumentPointer = QSharedPointer<PDFDocument>;

/// Helper class for document updates (for example, add/delete annotations,
/// fill form fields, do some other minor operations) and also for major
/// updates (document reset). It also contains modification flags, which are
/// hints for update operations (for example, if only annotations are changed,
/// then rebuilding outline tree is not needed). At least one of the flags
/// must be set. Reset flag is conservative, so it should be set, when document
/// changes are not known.
class PDFModifiedDocument
{
public:

    enum ModificationFlag
    {
        None                = 0x0000,   ///< No flag
        Reset               = 0x0001,   ///< Whole document content is changed (for example, new document is being set)
        PageContents        = 0x0002,   ///< Page contents changed (page graphics, not annotations)
        Annotation          = 0x0004,   ///< Annotations changed
        FormField           = 0x0008,   ///< Form field content changed
        Authorization       = 0x0010,   ///< Authorization has changed (for example, old document is granted user access, but for new, owner access)
        XFA_Pagination      = 0x0020,   ///< XFA pagination has been performed (this flag can be set only when Reset flag has been set and not any other flag)
        PreserveUndoRedo    = 0x0040,   ///< Preserve undo/red even when Reset flag is being set
        PreserveView        = 0x0080,   ///< Try to preserve view
    };

    Q_DECLARE_FLAGS(ModificationFlags, ModificationFlag)

    explicit inline PDFModifiedDocument() = default;
    explicit inline PDFModifiedDocument(PDFDocument* document, PDFOptionalContentActivity* optionalContentActivity) :
        m_document(document),
        m_optionalContentActivity(optionalContentActivity),
        m_flags(Reset)
    {
        Q_ASSERT(m_document);
    }

    explicit inline PDFModifiedDocument(PDFDocumentPointer document, PDFOptionalContentActivity* optionalContentActivity) :
        m_documentPointer(qMove(document)),
        m_document(m_documentPointer.data()),
        m_optionalContentActivity(optionalContentActivity),
        m_flags(Reset)
    {
        Q_ASSERT(m_document);
    }

    explicit inline PDFModifiedDocument(PDFDocument* document, PDFOptionalContentActivity* optionalContentActivity, ModificationFlags flags) :
        m_document(document),
        m_optionalContentActivity(optionalContentActivity),
        m_flags(flags)
    {
        Q_ASSERT(m_document);
    }

    explicit inline PDFModifiedDocument(PDFDocumentPointer document, PDFOptionalContentActivity* optionalContentActivity, ModificationFlags flags) :
        m_documentPointer(qMove(document)),
        m_document(m_documentPointer.data()),
        m_optionalContentActivity(optionalContentActivity),
        m_flags(flags)
    {
        Q_ASSERT(m_document);
    }

    PDFDocument* getDocument() const { return m_document; }
    PDFOptionalContentActivity* getOptionalContentActivity() const { return m_optionalContentActivity; }
    void setOptionalContentActivity(PDFOptionalContentActivity* optionalContentActivity) { m_optionalContentActivity = optionalContentActivity; }
    ModificationFlags getFlags() const { return m_flags; }

    bool hasReset() const { return m_flags.testFlag(Reset); }
    bool hasPageContentsChanged() const { return m_flags.testFlag(PageContents); }
    bool hasPreserveUndoRedo() const { return m_flags.testFlag(PreserveUndoRedo); }
    bool hasFlag(ModificationFlag flag) const { return m_flags.testFlag(flag); }
    bool hasPreserveView() const { return m_flags.testFlag(PreserveView); }

    operator PDFDocument*() const { return m_document; }
    operator PDFDocumentPointer() const { return m_documentPointer; }

private:
    PDFDocumentPointer m_documentPointer;
    PDFDocument* m_document = nullptr;
    PDFOptionalContentActivity* m_optionalContentActivity = nullptr;
    ModificationFlags m_flags = Reset;
};

// Implementation

inline
const PDFObject& PDFDocument::getObject(const PDFObject& object) const
{
    if (object.isReference())
    {
        // Try to dereference the object
        return m_pdfObjectStorage.getObject(object.getReference());
    }

    return object;
}

inline
const PDFDictionary* PDFDocument::getDictionaryFromObject(const PDFObject& object) const
{
    const PDFObject& dereferencedObject = getObject(object);
    if (dereferencedObject.isDictionary())
    {
        return dereferencedObject.getDictionary();
    }
    else if (dereferencedObject.isStream())
    {
        return dereferencedObject.getStream()->getDictionary();
    }

    return nullptr;
}

inline
const PDFObject& PDFDocument::getObjectByReference(PDFObjectReference reference) const
{
    return m_pdfObjectStorage.getObject(reference);
}

inline
const PDFDictionary* PDFObjectStorage::getDictionaryFromObject(const PDFObject& object) const
{
    const PDFObject& dereferencedObject = getObject(object);
    if (dereferencedObject.isDictionary())
    {
        return dereferencedObject.getDictionary();
    }
    else if (dereferencedObject.isStream())
    {
        return dereferencedObject.getStream()->getDictionary();
    }

    return nullptr;
}

inline
const PDFObject& PDFObjectStorage::getObjectByReference(PDFObjectReference reference) const
{
    return getObject(reference);
}

}   // namespace pdf

#endif // PDFDOCUMENT_H
