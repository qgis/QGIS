//    Copyright (C) 2019-2022 Jakub Melka
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

#include "pdfmultimedia.h"
#include "pdfdocument.h"
#include "pdfnametreeloader.h"

#include <QtEndian>

#include "pdfdbgheap.h"

namespace pdf
{

constexpr const std::array<std::pair<const char*, RichMediaType>, 4> richMediaTypes = {
    std::pair<const char*, RichMediaType>{ "3D", RichMediaType::_3D },
    std::pair<const char*, RichMediaType>{ "Flash", RichMediaType::Flash },
    std::pair<const char*, RichMediaType>{ "Sound", RichMediaType::Sound },
    std::pair<const char*, RichMediaType>{ "Video", RichMediaType::Video }
};

class PDF3DAuxiliaryParser
{
public:
    PDF3DAuxiliaryParser() = delete;

    /// Parses 4x4 transformation matrix
    /// \param storage Storage
    /// \param object Object
    static QMatrix4x4 parseMatrix4x4(const PDFObjectStorage* storage, PDFObject object);

    /// Parses 3D annotation color
    static QColor parseColor(const PDFObjectStorage* storage, PDFObject object, QColor defaultColor);
};

QMatrix4x4 PDF3DAuxiliaryParser::parseMatrix4x4(const PDFObjectStorage* storage, PDFObject object)
{
    QMatrix4x4 matrix;

    PDFDocumentDataLoaderDecorator loader(storage);
    std::vector<PDFReal> elements = loader.readNumberArray(object);
    if (elements.size() == 12)
    {
        const PDFReal a = elements[ 0];
        const PDFReal b = elements[ 1];
        const PDFReal c = elements[ 2];
        const PDFReal d = elements[ 3];
        const PDFReal e = elements[ 4];
        const PDFReal f = elements[ 5];
        const PDFReal g = elements[ 6];
        const PDFReal h = elements[ 7];
        const PDFReal i = elements[ 8];
        const PDFReal tx = elements[ 9];
        const PDFReal ty = elements[10];
        const PDFReal tz = elements[11];

        matrix = QMatrix4x4(a, b, c, 0,
                            d, e, f, 0,
                            g, h, i, 0,
                            tx, ty, tz, 1.0);
    }

    return matrix;
}

QColor PDF3DAuxiliaryParser::parseColor(const PDFObjectStorage* storage, PDFObject object, QColor defaultColor)
{
    object = storage->getObject(object);

    // If color is invalid according to the specification, then return default value
    if (object.isArray())
    {
        const PDFArray* array = object.getArray();
        if (array->getCount() == 4)
        {
            PDFDocumentDataLoaderDecorator loader(storage);
            if (loader.readName(array->getItem(0)) == "DeviceRGB")
            {
                const PDFReal r = loader.readNumber(array->getItem(1), 0.0);
                const PDFReal g = loader.readNumber(array->getItem(2), 0.0);
                const PDFReal b = loader.readNumber(array->getItem(3), 0.0);
                return QColor::fromRgbF(r, g, b, 1.0);
            }
        }
    }

    return defaultColor;
}

PDFSound PDFSound::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFSound result;

    object = storage->getObject(object);
    if (object.isStream())
    {
        const PDFStream* stream = object.getStream();
        const PDFDictionary* dictionary = stream->getDictionary();

        constexpr const std::array<std::pair<const char*, Format>, 4> formats = {
            std::pair<const char*, Format>{ "Raw", Format::Raw },
            std::pair<const char*, Format>{ "Signed", Format::Signed },
            std::pair<const char*, Format>{ "muLaw", Format::muLaw },
            std::pair<const char*, Format>{ "ALaw", Format::ALaw }
        };

        // Jakub Melka: parse the sound without exceptions
        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_fileSpecification = PDFFileSpecification::parse(storage, dictionary->get("F"));
        result.m_samplingRate = loader.readNumberFromDictionary(dictionary, "R", 0.0);
        result.m_channels = loader.readIntegerFromDictionary(dictionary, "C", 1);
        result.m_bitsPerSample = loader.readIntegerFromDictionary(dictionary, "B", 8);
        result.m_format = loader.readEnumByName(dictionary->get("E"), formats.cbegin(), formats.cend(), Format::Raw);
        result.m_soundCompression = loader.readNameFromDictionary(dictionary, "CO");
        result.m_soundCompressionParameters = storage->getObject(dictionary->get("CP"));
        result.m_streamObject = object;
    }

    return result;
}

PDFRendition PDFRendition::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFRendition result;
    object = storage->getObject(object);
    const PDFDictionary* renditionDictionary = nullptr;
    if (object.isDictionary())
    {
        renditionDictionary = object.getDictionary();
    }
    else if (object.isStream())
    {
        renditionDictionary = object.getStream()->getDictionary();
    }

    if (renditionDictionary)
    {
        constexpr const std::array<std::pair<const char*, Type>, 2> types = {
            std::pair<const char*, Type>{ "MR", Type::Media },
            std::pair<const char*, Type>{ "SR", Type::Selector }
        };

        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_type = loader.readEnumByName(renditionDictionary->get("S"), types.cbegin(), types.cend(), Type::Invalid);
        result.m_name = loader.readTextStringFromDictionary(renditionDictionary, "N", QString());

        auto readMediaCriteria = [renditionDictionary, storage](const char* entry)
        {
            PDFObject dictionaryObject = storage->getObject(renditionDictionary->get(entry));
            if (dictionaryObject.isDictionary())
            {
                const PDFDictionary* dictionary = dictionaryObject.getDictionary();
                return PDFMediaCriteria::parse(storage, dictionary->get("C"));
            }

            return PDFMediaCriteria();
        };

        result.m_mustHonored = readMediaCriteria("MH");
        result.m_bestEffort = readMediaCriteria("BE");

        switch (result.m_type)
        {
            case Type::Media:
            {
                MediaRenditionData data;
                data.clip = PDFMediaClip::parse(storage, renditionDictionary->get("C"));
                data.playParameters = PDFMediaPlayParameters::parse(storage, renditionDictionary->get("P"));
                data.screenParameters = PDFMediaScreenParameters::parse(storage, renditionDictionary->get("SP"));

                result.m_data = qMove(data);
                break;
            }

            case Type::Selector:
            {
                result.m_data = SelectorRenditionData{ renditionDictionary->get("R") };
                break;
            }

            default:
                break;
        }
    }

    return result;
}

PDFMediaMinimumBitDepth PDFMediaMinimumBitDepth::parse(const PDFObjectStorage* storage, PDFObject object)
{
    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        return PDFMediaMinimumBitDepth(loader.readIntegerFromDictionary(dictionary, "V", -1), loader.readIntegerFromDictionary(dictionary, "M", 0));
    }

    return PDFMediaMinimumBitDepth(-1, -1);
}

PDFMediaMinimumScreenSize PDFMediaMinimumScreenSize::parse(const PDFObjectStorage* storage, PDFObject object)
{
    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        std::vector<PDFInteger> values = loader.readIntegerArrayFromDictionary(dictionary, "V");
        if (values.size() == 2)
        {
            return PDFMediaMinimumScreenSize(values[0], values[1], loader.readIntegerFromDictionary(dictionary, "M", 0));
        }
    }

    return  PDFMediaMinimumScreenSize(-1, -1, -1);
}

PDFMediaSoftwareIdentifier PDFMediaSoftwareIdentifier::parse(const PDFObjectStorage* storage, PDFObject object)
{
    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        return PDFMediaSoftwareIdentifier(loader.readTextStringFromDictionary(dictionary, "U", QString()).toLatin1(),
                                          loader.readIntegerArrayFromDictionary(dictionary, "L"),
                                          loader.readIntegerArrayFromDictionary(dictionary, "H"),
                                          loader.readBooleanFromDictionary(dictionary, "LI", true),
                                          loader.readBooleanFromDictionary(dictionary, "HI", true),
                                          loader.readStringArrayFromDictionary(dictionary, "OS"));
    }

    return PDFMediaSoftwareIdentifier(QByteArray(), { }, { }, true, true, { });
}

PDFMediaCriteria PDFMediaCriteria::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFMediaCriteria criteria;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        auto readBoolean = [&loader, dictionary](const char* name, std::optional<bool>& value)
        {
            if (dictionary->hasKey(name))
            {
                value = loader.readBooleanFromDictionary(dictionary, name, false);
            }
        };

        readBoolean("A", criteria.m_audioDescriptions);
        readBoolean("C", criteria.m_textCaptions);
        readBoolean("O", criteria.m_audioOverdubs);
        readBoolean("S", criteria.m_subtitles);
        if (dictionary->hasKey("R"))
        {
            criteria.m_bitrate = loader.readIntegerFromDictionary(dictionary, "R", 0);
        }
        if (dictionary->hasKey("D"))
        {
            criteria.m_minimumBitDepth = PDFMediaMinimumBitDepth::parse(storage, dictionary->get("D"));
        }
        if (dictionary->hasKey("Z"))
        {
            criteria.m_minimumScreenSize = PDFMediaMinimumScreenSize::parse(storage, dictionary->get("Z"));
        }
        if (dictionary->hasKey("V"))
        {
            const PDFObject& viewerObject = storage->getObject(dictionary->get("V"));
            if (viewerObject.isArray())
            {
                std::vector<PDFMediaSoftwareIdentifier> viewers;
                const PDFArray* viewersArray = viewerObject.getArray();
                viewers.reserve(viewersArray->getCount());
                for (size_t i = 0; i < viewersArray->getCount(); ++i)
                {
                    viewers.emplace_back(PDFMediaSoftwareIdentifier::parse(storage, viewersArray->getItem(i)));
                }
                criteria.m_viewers = qMove(viewers);
            }
        }
        std::vector<QByteArray> pdfVersions = loader.readNameArrayFromDictionary(dictionary, "P");
        if (pdfVersions.size() > 0)
        {
            criteria.m_minimumPdfVersion = qMove(pdfVersions[0]);
            if (pdfVersions.size() > 1)
            {
                criteria.m_maximumPdfVersion = qMove(pdfVersions[1]);
            }
        }
        if (dictionary->hasKey("L"))
        {
            criteria.m_languages = loader.readStringArrayFromDictionary(dictionary, "L");
        }
    }

    return criteria;
}

PDFMediaPermissions PDFMediaPermissions::parse(const PDFObjectStorage* storage, PDFObject object)
{
    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        constexpr const std::array<std::pair<const char*, Permission>, 4> types = {
            std::pair<const char*, Permission>{ "TEMPNEVER", Permission::Never },
            std::pair<const char*, Permission>{ "TEMPEXTRACT", Permission::Extract },
            std::pair<const char*, Permission>{ "TEMPACCESS", Permission::Access },
            std::pair<const char*, Permission>{ "TEMPALWAYS", Permission::Always }
        };

        return PDFMediaPermissions(loader.readEnumByName(dictionary->get("TF"), types.cbegin(), types.cend(), Permission::Never));
    }

    return PDFMediaPermissions(Permission::Never);
}

PDFMediaPlayers PDFMediaPlayers::parse(const PDFObjectStorage* storage, PDFObject object)
{
    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        auto readPlayers = [storage, dictionary](const char* key)
        {
            std::vector<PDFMediaPlayer> result;

            const PDFObject& playersArrayObject = storage->getObject(dictionary->get(key));
            if (playersArrayObject.isArray())
            {
                const PDFArray* playersArray = playersArrayObject.getArray();
                result.reserve(playersArray->getCount());

                for (size_t i = 0; i < playersArray->getCount(); ++i)
                {
                    result.emplace_back(PDFMediaPlayer::parse(storage, playersArray->getItem(i)));
                }
            }

            return result;
        };

        return PDFMediaPlayers(readPlayers("MU"), readPlayers("A"), readPlayers("NU"));
    }

    return PDFMediaPlayers({ }, { }, { });
}

PDFMediaPlayer PDFMediaPlayer::parse(const PDFObjectStorage* storage, PDFObject object)
{
    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        return PDFMediaPlayer(PDFMediaSoftwareIdentifier::parse(storage, dictionary->get("PID")));
    }
    return PDFMediaPlayer(PDFMediaSoftwareIdentifier(QByteArray(), { }, { }, true, true, { }));
}

PDFMediaOffset PDFMediaOffset::parse(const PDFObjectStorage* storage, PDFObject object)
{
    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        QByteArray S = loader.readNameFromDictionary(dictionary, "S");
        if (S == "T")
        {
            if (const PDFDictionary* timespanDictionary = storage->getDictionaryFromObject(dictionary->get("T")))
            {
                return PDFMediaOffset(Type::Time, TimeData{ loader.readIntegerFromDictionary(timespanDictionary, "V", 0) });
            }
        }
        else if (S == "F")
        {
            return PDFMediaOffset(Type::Frame, FrameData{ loader.readIntegerFromDictionary(dictionary, "F", 0) });
        }
        else if (S == "M")
        {
            return PDFMediaOffset(Type::Marker, MarkerData{ loader.readTextStringFromDictionary(dictionary, "M", QString()) });
        }
    }

    return PDFMediaOffset(Type::Invalid, std::monostate());
}

PDFMediaClip PDFMediaClip::parse(const PDFObjectStorage* storage, PDFObject object)
{
    MediaClipData clipData;
    std::vector<MediaSectionData> sections;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        std::set<PDFObjectReference> usedReferences;
        while (dictionary)
        {
            QByteArray type = loader.readNameFromDictionary(dictionary, "S");
            if (type == "MCD")
            {
                clipData.name = loader.readTextStringFromDictionary(dictionary, "N", QString());

                PDFObject fileSpecificationOrStreamObject = storage->getObject(dictionary->get("D"));
                if (fileSpecificationOrStreamObject.isStream())
                {
                    clipData.dataStream = fileSpecificationOrStreamObject;
                }
                else
                {
                    clipData.fileSpecification = PDFFileSpecification::parse(storage, fileSpecificationOrStreamObject);
                }
                clipData.contentType = loader.readStringFromDictionary(dictionary, "CT");
                clipData.permissions = PDFMediaPermissions::parse(storage, dictionary->get("P"));
                clipData.alternateTextDescriptions = PDFMediaMultiLanguageTexts::parse(storage, dictionary->get("Alt"));
                clipData.players = PDFMediaPlayers::parse(storage, dictionary->get("PL"));

                auto getBaseUrl = [&loader, storage, dictionary](const char* name)
                {
                    if (const PDFDictionary* subdictionary = storage->getDictionaryFromObject(dictionary->get(name)))
                    {
                        return loader.readStringFromDictionary(subdictionary, "BU");
                    }

                    return QByteArray();
                };
                clipData.m_baseUrlMustHonored = getBaseUrl("MH");
                clipData.m_baseUrlBestEffort = getBaseUrl("BE");
                break;
            }
            else if (type == "MCS")
            {
                MediaSectionData sectionData;
                sectionData.name = loader.readTextStringFromDictionary(dictionary, "N", QString());
                sectionData.alternateTextDescriptions = PDFMediaMultiLanguageTexts::parse(storage, dictionary->get("Alt"));

                auto readMediaSectionBeginEnd = [storage, dictionary](const char* name)
                {
                    MediaSectionBeginEnd result;

                    if (const PDFDictionary* subdictionary = storage->getDictionaryFromObject(dictionary->get(name)))
                    {
                        result.offsetBegin = PDFMediaOffset::parse(storage, subdictionary->get("B"));
                        result.offsetEnd = PDFMediaOffset::parse(storage, subdictionary->get("E"));
                    }

                    return result;
                };
                sectionData.m_mustHonored = readMediaSectionBeginEnd("MH");
                sectionData.m_bestEffort = readMediaSectionBeginEnd("BE");

                // Jakub Melka: parse next item in linked list
                PDFObject next = dictionary->get("D");
                if (next.isReference())
                {
                    if (usedReferences.count(next.getReference()))
                    {
                        break;
                    }
                    usedReferences.insert(next.getReference());
                }
                dictionary = storage->getDictionaryFromObject(next);
                continue;
            }

            dictionary = nullptr;
        }
    }

    return PDFMediaClip(qMove(clipData), qMove(sections));
}

PDFMediaMultiLanguageTexts PDFMediaMultiLanguageTexts::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFMediaMultiLanguageTexts texts;

    object = storage->getObject(object);
    if (object.isArray())
    {
        const PDFArray* array = object.getArray();
        if (array->getCount() % 2 == 0)
        {
            PDFDocumentDataLoaderDecorator loader(storage);
            const size_t pairs = array->getCount() / 2;
            for (size_t i = 0; i < pairs; ++i)
            {
                const PDFObject& languageName = storage->getObject(array->getItem(2 * i));
                const PDFObject& text = array->getItem(2 * i + 1);

                if (languageName.isString())
                {
                    texts.texts[languageName.getString()] = loader.readTextString(text, QString());
                }
            }
        }
    }

    return texts;
}

PDFMediaPlayParameters PDFMediaPlayParameters::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFMediaPlayParameters result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        result.m_players = PDFMediaPlayers::parse(storage, dictionary->get("PL"));

        auto getPlayParameters = [storage, dictionary](const char* name)
        {
            PlayParameters parameters;

            if (const PDFDictionary* subdictionary = storage->getDictionaryFromObject(dictionary->get(name)))
            {
                PDFDocumentDataLoaderDecorator loader(storage);
                parameters.volume = loader.readIntegerFromDictionary(subdictionary, "V", 100);
                parameters.controllerUserInterface = loader.readBooleanFromDictionary(subdictionary, "C", false);
                parameters.fitMode = static_cast<FitMode>(loader.readIntegerFromDictionary(subdictionary, "F", 5));
                parameters.playAutomatically = loader.readBooleanFromDictionary(subdictionary, "A", true);
                parameters.repeat = loader.readNumberFromDictionary(dictionary, "RC", 1.0);

                if (const PDFDictionary* durationDictionary = storage->getDictionaryFromObject(subdictionary->get("D")))
                {
                    constexpr const std::array<std::pair<const char*, Duration>, 3> durations = {
                        std::pair<const char*, Duration>{ "I", Duration::Intrinsic },
                        std::pair<const char*, Duration>{ "F", Duration::Infinity },
                        std::pair<const char*, Duration>{ "T", Duration::Seconds }
                    };
                    parameters.duration = loader.readEnumByName(durationDictionary->get("S"), durations.cbegin(), durations.cend(), Duration::Intrinsic);

                    if (const PDFDictionary* timeDictionary = storage->getDictionaryFromObject(durationDictionary->get("T")))
                    {
                        parameters.durationSeconds = loader.readNumberFromDictionary(timeDictionary, "V", 0.0);
                    }
                }
            }

            return parameters;
        };
        result.m_mustHonored = getPlayParameters("MH");
        result.m_bestEffort = getPlayParameters("BE");
    }

    return result;
}

PDFMediaScreenParameters PDFMediaScreenParameters::parse(const PDFObjectStorage* storage, PDFObject object)
{
    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        auto getScreenParameters = [storage, dictionary](const char* name)
        {
            ScreenParameters result;
            if (const PDFDictionary* screenDictionary = storage->getDictionaryFromObject(dictionary->get(name)))
            {
                PDFDocumentDataLoaderDecorator loader(storage);
                result.windowType = static_cast<WindowType>(loader.readIntegerFromDictionary(screenDictionary, "W", 3));
                result.opacity = loader.readNumberFromDictionary(screenDictionary, "O", 1.0);
                result.monitorSpecification = loader.readIntegerFromDictionary(screenDictionary, "M", 0);
                std::vector<PDFReal> rgb = loader.readNumberArrayFromDictionary(screenDictionary, "B", { 1.0, 1.0, 1.0 });
                rgb.resize(3, 1.0);
                result.backgroundColor.setRgbF(rgb[0], rgb[1], rgb[2]);

                if (const PDFDictionary* floatWindowDictionary = storage->getDictionaryFromObject(screenDictionary->get("F")))
                {
                    std::vector<PDFInteger> sizeArray = loader.readIntegerArrayFromDictionary(floatWindowDictionary, "D");
                    sizeArray.resize(2, 0);
                    result.floatingWindowSize = QSize(sizeArray[0], sizeArray[1]);
                    result.floatingWindowReference = static_cast<WindowRelativeTo>(loader.readIntegerFromDictionary(floatWindowDictionary, "RT", 0));
                    result.floatingWindowOffscreenMode = static_cast<OffscreenMode>(loader.readIntegerFromDictionary(floatWindowDictionary, "O", 1));
                    result.floatingWindowHasTitleBar = loader.readBooleanFromDictionary(floatWindowDictionary, "T", true);
                    result.floatingWindowCloseable = loader.readBooleanFromDictionary(floatWindowDictionary, "UC", true);
                    result.floatingWindowResizeMode = static_cast<ResizeMode>(loader.readIntegerFromDictionary(floatWindowDictionary, "R", 0));
                    result.floatingWindowTitle = PDFMediaMultiLanguageTexts::parse(storage, floatWindowDictionary->get("TT"));
                    switch (loader.readIntegerFromDictionary(floatWindowDictionary, "P", 4))
                    {
                        case 0:
                            result.floatingWindowAlignment = Qt::AlignTop | Qt::AlignLeft;
                            break;
                        case 1:
                            result.floatingWindowAlignment = Qt::AlignTop | Qt::AlignHCenter;
                            break;
                        case 2:
                            result.floatingWindowAlignment = Qt::AlignTop | Qt::AlignRight;
                            break;
                        case 3:
                            result.floatingWindowAlignment = Qt::AlignVCenter | Qt::AlignLeft;
                            break;
                        case 4:
                        default:
                            result.floatingWindowAlignment = Qt::AlignCenter;
                            break;
                        case 5:
                            result.floatingWindowAlignment =  Qt::AlignVCenter | Qt::AlignRight;
                            break;
                        case 6:
                            result.floatingWindowAlignment = Qt::AlignBottom | Qt::AlignLeft;
                            break;
                        case 7:
                            result.floatingWindowAlignment = Qt::AlignBottom | Qt::AlignHCenter;
                            break;
                        case 8:
                            result.floatingWindowAlignment = Qt::AlignBottom | Qt::AlignRight;
                            break;
                    }
                }
            }

            return result;
        };
        return PDFMediaScreenParameters(getScreenParameters("MH"), getScreenParameters("BE"));
    }

    return PDFMediaScreenParameters();
}

PDFMovie PDFMovie::parse(const PDFObjectStorage* storage, PDFObject object)
{
    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFMovie result;

        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_movieFile = PDFFileSpecification::parse(storage, dictionary->get("F"));
        std::vector<PDFInteger> windowSizeArray = loader.readIntegerArrayFromDictionary(dictionary, "Aspect");
        if (windowSizeArray.size() == 2)
        {
            result.m_windowSize = QSize(windowSizeArray[0], windowSizeArray[1]);
        }
        result.m_rotationAngle = loader.readIntegerFromDictionary(dictionary, "Rotate", 0);

        PDFObject posterObject = storage->getObject(dictionary->get("Poster"));
        if (posterObject.isBool())
        {
            result.m_showPoster = posterObject.getBool();
        }
        else if (posterObject.isStream())
        {
            result.m_showPoster = true;
            result.m_poster = posterObject;
        }

        return result;
    }

    return PDFMovie();
}

PDFMovieActivation PDFMovieActivation::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFMovieActivation result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);

        constexpr const std::array<std::pair<const char*, Mode>, 4> modes = {
            std::pair<const char*, Mode>{ "Once", Mode::Once },
            std::pair<const char*, Mode>{ "Open", Mode::Open },
            std::pair<const char*, Mode>{ "Repeat", Mode::Repeat },
            std::pair<const char*, Mode>{ "Palindrome", Mode::Palindrome }
        };

        std::vector<PDFInteger> scale = loader.readIntegerArrayFromDictionary(dictionary, "FWScale");
        if (scale.size() != 2)
        {
            scale.resize(2, 0);
        }
        std::vector<PDFReal> relativePosition = loader.readNumberArrayFromDictionary(dictionary, "FWPosition");
        if (relativePosition.size() != 2)
        {
            relativePosition.resize(2, 0.5);
        }

        result.m_start = parseMovieTime(storage, dictionary->get("Start"));
        result.m_duration = parseMovieTime(storage, dictionary->get("Duration"));
        result.m_rate = loader.readNumberFromDictionary(dictionary, "Rate", 1.0);
        result.m_volume = loader.readNumberFromDictionary(dictionary, "Volume", 1.0);
        result.m_showControls = loader.readBooleanFromDictionary(dictionary, "ShowControls", false);
        result.m_synchronous = loader.readBooleanFromDictionary(dictionary, "Synchronous", false);
        result.m_mode = loader.readEnumByName(dictionary->get("Mode"), modes.cbegin(), modes.cend(), Mode::Once);
        result.m_scaleNumerator = scale[0];
        result.m_scaleDenominator = scale[1];
        result.m_relativeWindowPosition = QPointF(relativePosition[0], relativePosition[1]);
    }

    return result;
}

PDFMovieActivation::MovieTime PDFMovieActivation::parseMovieTime(const PDFObjectStorage* storage, PDFObject object)
{
    MovieTime result;

    object = storage->getObject(object);
    if (object.isInt())
    {
        result.value = object.getInteger();
    }
    else if (object.isString())
    {
        result.value = parseMovieTimeFromString(object.getString());
    }
    else if (object.isArray())
    {
        const PDFArray* objectArray = object.getArray();
        if (objectArray->getCount() == 2)
        {
            PDFDocumentDataLoaderDecorator loader(storage);
            result.unitsPerSecond = loader.readInteger(objectArray->getItem(1), 0);

            object = storage->getObject(objectArray->getItem(0));
            if (object.isInt())
            {
                result.value = object.getInteger();
            }
            else if (object.isString())
            {
                result.value = parseMovieTimeFromString(object.getString());
            }
        }
    }

    return result;
}

PDFInteger PDFMovieActivation::parseMovieTimeFromString(const QByteArray& string)
{
    // According to the specification, the string contains 64-bit signed integer,
    // in big-endian format.
    if (string.size() == sizeof(quint64))
    {
        quint64 result = reinterpret_cast<quint64>(string.data());
        qFromBigEndian<decltype(result)>(&result, qsizetype(sizeof(decltype(result))), &result);
        return static_cast<PDFInteger>(result);
    }

    return 0;
}

PDFRichMediaWindowPosition PDFRichMediaWindowPosition::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFRichMediaWindowPosition result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        constexpr const std::array<std::pair<const char*, Alignment>, 3> alignments = {
            std::pair<const char*, Alignment>{ "Near", Alignment::Near },
            std::pair<const char*, Alignment>{ "Center", Alignment::Center },
            std::pair<const char*, Alignment>{ "Far", Alignment::Far }
        };

        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_horizontalAlignment = loader.readEnumByName(dictionary->get("HAlign"), alignments.cbegin(), alignments.cend(), Alignment::Far);
        result.m_verticalAlignment = loader.readEnumByName(dictionary->get("VAlign"), alignments.cbegin(), alignments.cend(), Alignment::Near);
        result.m_horizontalOffset = loader.readNumberFromDictionary(dictionary, "HOffset", 18.0);
        result.m_verticalOffset = loader.readNumberFromDictionary(dictionary, "VOffset", 18.0);
    }

    return result;
}

PDFRichMediaWindow PDFRichMediaWindow::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFRichMediaWindow result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);

        // Parse width
        if (const PDFDictionary* widthDictionary = storage->getDictionaryFromObject(dictionary->get("Width")))
        {
            result.m_width[Default] = loader.readNumberFromDictionary(widthDictionary, "Default", 288.0);
            result.m_width[Max] = loader.readNumberFromDictionary(widthDictionary, "Max", 576.0);
            result.m_width[Min] = loader.readNumberFromDictionary(widthDictionary, "Min", 72.0);
        }

        // Parse height
        if (const PDFDictionary* heightDictionary = storage->getDictionaryFromObject(dictionary->get("Height")))
        {
            result.m_height[Default] = loader.readNumberFromDictionary(heightDictionary, "Default", 288.0);
            result.m_height[Max] = loader.readNumberFromDictionary(heightDictionary, "Max", 576.0);
            result.m_height[Min] = loader.readNumberFromDictionary(heightDictionary, "Min", 72.0);
        }

        result.m_richMediaPosition = PDFRichMediaWindowPosition::parse(storage, dictionary->get("Position"));
    }

    return result;
}

PDFRichMediaPresentation PDFRichMediaPresentation::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFRichMediaPresentation result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        constexpr const std::array<std::pair<const char*, Style>, 2> styles = {
            std::pair<const char*, Style>{ "Embedded", Style::Embedded },
            std::pair<const char*, Style>{ "Windowed", Style::Windowed }
        };

        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_style = loader.readEnumByName(dictionary->get("Style"), styles.cbegin(), styles.cend(), Style::Embedded);
        result.m_window = PDFRichMediaWindow::parse(storage, dictionary->get("Window"));
        result.m_isTransparent = loader.readBooleanFromDictionary(dictionary, "Transparent", false);
        result.m_isNavigationPaneEnabled = loader.readBooleanFromDictionary(dictionary, "NavigationPane", false);
        result.m_isToolbarEnabled = loader.readBooleanFromDictionary(dictionary, "Toolbar", false);
        result.m_isContentClickPassed = loader.readBooleanFromDictionary(dictionary, "PassContextClick", false);
    }

    return result;
}

PDFRichMediaAnimation PDFRichMediaAnimation::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFRichMediaAnimation result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        constexpr const std::array<std::pair<const char*, Animation>, 3> animations = {
            std::pair<const char*, Animation>{ "None", Animation::None },
            std::pair<const char*, Animation>{ "Linear", Animation::Linear },
            std::pair<const char*, Animation>{ "Oscillating", Animation::Oscillating }
        };

        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_animation = loader.readEnumByName(dictionary->get("Subtype"), animations.cbegin(), animations.cend(), Animation::None);
        result.m_playCount = loader.readIntegerFromDictionary(dictionary, "PlayCount", -1);
        result.m_speed = loader.readNumberFromDictionary(dictionary, "Speed", 1.0);
    }

    return result;
}

PDFRichMediaDeactivation PDFRichMediaDeactivation::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFRichMediaDeactivation result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        constexpr const std::array<std::pair<const char*, DeactivationMode>, 3> deactivationModes = {
            std::pair<const char*, DeactivationMode>{ "XD", DeactivationMode::ExplicitlyByUserAction },
            std::pair<const char*, DeactivationMode>{ "PC", DeactivationMode::PageLoseFocus },
            std::pair<const char*, DeactivationMode>{ "PI", DeactivationMode::PageHide }
        };

        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_mode = loader.readEnumByName(dictionary->get("Condition"), deactivationModes.cbegin(), deactivationModes.cend(), DeactivationMode::ExplicitlyByUserAction);
    }

    return result;
}

PDFRichMediaActivation PDFRichMediaActivation::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFRichMediaActivation result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        constexpr const std::array<std::pair<const char*, ActivationMode>, 3> activationModes = {
            std::pair<const char*, ActivationMode>{ "XA", ActivationMode::ExplicitlyByUserAction },
            std::pair<const char*, ActivationMode>{ "PO", ActivationMode::PageEnterFocus },
            std::pair<const char*, ActivationMode>{ "PV", ActivationMode::PageShow }
        };

        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_mode = loader.readEnumByName(dictionary->get("Condition"), activationModes.cbegin(), activationModes.cend(), ActivationMode::ExplicitlyByUserAction);
        result.m_animation = PDFRichMediaAnimation::parse(storage, dictionary->get("Animation"));
        result.m_view = loader.readReferenceFromDictionary(dictionary, "View");
        result.m_configuration = loader.readReferenceFromDictionary(dictionary, "Configuration");
        result.m_presentation = PDFRichMediaPresentation::parse(storage, dictionary->get("Presentation"));
    }

    return result;
}

PDFRichMediaSettings PDFRichMediaSettings::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFRichMediaSettings result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        result.m_activation = PDFRichMediaActivation::parse(storage, dictionary->get("Activation"));
        result.m_deactivation = PDFRichMediaDeactivation::parse(storage, dictionary->get("Deactivation"));
    }

    return result;
}

PDFRichMediaContent PDFRichMediaContent::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFRichMediaContent result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_assets = PDFNameTreeLoader<PDFFileSpecification>::parse(storage, dictionary->get("Assets"), &PDFFileSpecification::parse);
        result.m_configurations = loader.readReferenceArrayFromDictionary(dictionary, "Configurations");
        result.m_views = loader.readReferenceArrayFromDictionary(dictionary, "Views");
    }

    return result;
}

PDFRichMediaConfiguration PDFRichMediaConfiguration::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFRichMediaConfiguration result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_type = loader.readEnumByName(dictionary->get("Subtype"), richMediaTypes.cbegin(), richMediaTypes.cend(), RichMediaType::Unspecified);
        result.m_instances = loader.readReferenceArrayFromDictionary(dictionary, "Instances");
    }

    return result;
}

PDFRichMediaInstance PDFRichMediaInstance::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFRichMediaInstance result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_type = loader.readEnumByName(dictionary->get("Subtype"), richMediaTypes.cbegin(), richMediaTypes.cend(), RichMediaType::Unspecified);
        result.m_asset = loader.readReferenceFromDictionary(dictionary, "Asset");
    }

    return result;
}

PDF3DActivation PDF3DActivation::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDF3DActivation result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        constexpr const std::array<std::pair<const char*, TriggerMode>, 3> activationModes = {
            std::pair<const char*, TriggerMode>{ "XA", TriggerMode::ExplicitlyByUserAction },
            std::pair<const char*, TriggerMode>{ "PO", TriggerMode::PageFocus },
            std::pair<const char*, TriggerMode>{ "PV", TriggerMode::PageVisibility }
        };

        constexpr const std::array<std::pair<const char*, TriggerMode>, 3> deactivationModes = {
            std::pair<const char*, TriggerMode>{ "XD", TriggerMode::ExplicitlyByUserAction },
            std::pair<const char*, TriggerMode>{ "PC", TriggerMode::PageFocus },
            std::pair<const char*, TriggerMode>{ "PI", TriggerMode::PageVisibility }
        };

        constexpr const std::array<std::pair<const char*, ActivationMode>, 3> stateModes = {
            std::pair<const char*, ActivationMode>{ "I", ActivationMode::Instantiated },
            std::pair<const char*, ActivationMode>{ "L", ActivationMode::Live },
            std::pair<const char*, ActivationMode>{ "U", ActivationMode::Uninstantiated }
        };

        constexpr const std::array<std::pair<const char*, Style>, 2> styles = {
            std::pair<const char*, Style>{ "Embedded", Style::Embedded },
            std::pair<const char*, Style>{ "Windowed", Style::Windowed }
        };

        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_activationTriggerMode = loader.readEnumByName(dictionary->get("A"), activationModes.cbegin(), activationModes.cend(), TriggerMode::ExplicitlyByUserAction);
        result.m_deactivationTriggerMode = loader.readEnumByName(dictionary->get("D"), deactivationModes.cbegin(), deactivationModes.cend(), TriggerMode::PageVisibility);
        result.m_activationMode = loader.readEnumByName(dictionary->get("AIS"), stateModes.cbegin(), stateModes.cend(), ActivationMode::Live);
        result.m_deactivationMode = loader.readEnumByName(dictionary->get("DIS"), stateModes.cbegin(), stateModes.cend(), ActivationMode::Uninstantiated);
        result.m_toolbar = loader.readBooleanFromDictionary(dictionary, "TB", true);
        result.m_navigator = loader.readBooleanFromDictionary(dictionary, "NP", false);
        result.m_style = loader.readEnumByName(dictionary->get("Style"), styles.cbegin(), styles.cend(), Style::Embedded);
        result.m_window = PDFRichMediaWindow::parse(storage, dictionary->get("Window"));
        result.m_transparent = loader.readBooleanFromDictionary(dictionary, "Transparent", false);
    }

    return result;
}

PDF3DRenderMode PDF3DRenderMode::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDF3DRenderMode result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        constexpr const std::array<std::pair<const char*, RenderMode>, 15> renderModes = {
            std::pair<const char*, RenderMode>{ "Solid",                          RenderMode::Solid },
            std::pair<const char*, RenderMode>{ "SolidWireframe",                 RenderMode::SolidWireframe },
            std::pair<const char*, RenderMode>{ "Transparent",                    RenderMode::Transparent },
            std::pair<const char*, RenderMode>{ "TransparentWireframe",           RenderMode::TransparentWireframe },
            std::pair<const char*, RenderMode>{ "BoundingBox",                    RenderMode::BoundingBox },
            std::pair<const char*, RenderMode>{ "TransparentBoundingBox",         RenderMode::TransparentBoundingBox },
            std::pair<const char*, RenderMode>{ "TransparentBoundingBoxOutline",  RenderMode::TransparentBoundingBoxOutline },
            std::pair<const char*, RenderMode>{ "Wireframe",                      RenderMode::Wireframe },
            std::pair<const char*, RenderMode>{ "ShadedWireframe",                RenderMode::ShadedWireframe },
            std::pair<const char*, RenderMode>{ "HiddenWireframe",                RenderMode::HiddenWireframe },
            std::pair<const char*, RenderMode>{ "Vertices",                       RenderMode::Vertices },
            std::pair<const char*, RenderMode>{ "ShadedVertices",                 RenderMode::ShadedVertices },
            std::pair<const char*, RenderMode>{ "Illustration",                   RenderMode::Illustration },
            std::pair<const char*, RenderMode>{ "SolidOutline",                   RenderMode::SolidOutline },
            std::pair<const char*, RenderMode>{ "ShadedIllustration",             RenderMode::ShadedIllustration }
        };

        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_renderMode = loader.readEnumByName(dictionary->get("Subtype"), renderModes.cbegin(), renderModes.cend(), RenderMode::Solid);
        result.m_auxiliaryColor = PDF3DAuxiliaryParser::parseColor(storage, dictionary->get("AC"), Qt::black);
        result.m_faceColor = PDF3DAuxiliaryParser::parseColor(storage, dictionary->get("FC"), Qt::black);
        result.m_faceColorMode = (loader.readName(dictionary->get("FC")) != "BG") ? FaceColorMode::Color : FaceColorMode::BG;
        result.m_opacity = loader.readNumberFromDictionary(dictionary, "O", 0.5);
        result.m_creaseAngle = loader.readNumberFromDictionary(dictionary, "CV", 45.0);
    }

    return result;
}

PDF3DNode PDF3DNode::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDF3DNode result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_name = loader.readTextStringFromDictionary(dictionary, "N", QString());
        if (dictionary->hasKey("O"))
        {
            result.m_opacity = loader.readNumberFromDictionary(dictionary, "O", 1.0);
        }
        if (dictionary->hasKey("V"))
        {
            result.m_visibility = loader.readBooleanFromDictionary(dictionary, "V", true);
        }
        if (dictionary->hasKey("M"))
        {
            result.m_matrix = PDF3DAuxiliaryParser::parseMatrix4x4(storage, dictionary->get("M"));
        }
        result.m_instance = loader.readReferenceFromDictionary(dictionary, "Instance");
        result.m_data = loader.readTextStringFromDictionary(dictionary, "Data", QString());
        if (dictionary->hasKey("RM"))
        {
            result.m_renderMode = PDF3DRenderMode::parse(storage, dictionary->get("RM"));
        }
    }

    return result;
}

PDF3DCrossSection PDF3DCrossSection::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDF3DCrossSection result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        loader.readNumberArray(dictionary->get("C"), result.m_centerOfRotation.begin(), result.m_centerOfRotation.end());

        PDFObject axesRotation = storage->getObject(dictionary->get("O"));
        if (const PDFArray* rotations = axesRotation.getArray())
        {
            if (rotations->getCount() == 3)
            {
                PDFObject xObject = storage->getObject(rotations->getItem(0));
                PDFObject yObject = storage->getObject(rotations->getItem(1));
                PDFObject zObject = storage->getObject(rotations->getItem(2));

                auto readValue = [&result, &loader](const PDFObject& object, int index, Direction direction)
                {
                    if (object.isNull())
                    {
                        result.m_perpendicularDirection = direction;
                        result.m_rotationAngles[index] = 0.0;
                    }
                    else
                    {
                        result.m_rotationAngles[index] = loader.readNumber(object, 0.0);
                    }
                };

                readValue(xObject, 0, Direction::X);
                readValue(yObject, 1, Direction::Y);
                readValue(zObject, 2, Direction::Z);
            }
        }

        result.m_cutPlaneOpacity = loader.readNumberFromDictionary(dictionary, "PO", 0.5);
        result.m_cutPlaneColor = PDF3DAuxiliaryParser::parseColor(storage, dictionary->get("PC"), Qt::white);
        result.m_intersectionVisibility = loader.readBooleanFromDictionary(dictionary, "IV", false);
        result.m_intersectionColor = PDF3DAuxiliaryParser::parseColor(storage, dictionary->get("IC"), Qt::green);
        result.m_showTransparent = loader.readBooleanFromDictionary(dictionary, "ST", false);
        result.m_sectionCapping = loader.readBooleanFromDictionary(dictionary, "SC", false);
    }

    return result;
}

PDF3DLightingScheme PDF3DLightingScheme::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDF3DLightingScheme result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        constexpr const std::array<std::pair<const char*, LightingScheme>, 12> lightingSchemes = {
            std::pair<const char*, LightingScheme>{ "Artwork", LightingScheme::Artwork },
            std::pair<const char*, LightingScheme>{ "None", LightingScheme::None },
            std::pair<const char*, LightingScheme>{ "White", LightingScheme::White },
            std::pair<const char*, LightingScheme>{ "Day", LightingScheme::Day },
            std::pair<const char*, LightingScheme>{ "Night", LightingScheme::Night },
            std::pair<const char*, LightingScheme>{ "Hard", LightingScheme::Hard },
            std::pair<const char*, LightingScheme>{ "Primary", LightingScheme::Primary },
            std::pair<const char*, LightingScheme>{ "Blue", LightingScheme::Blue },
            std::pair<const char*, LightingScheme>{ "Red", LightingScheme::Red },
            std::pair<const char*, LightingScheme>{ "Cube", LightingScheme::Cube },
            std::pair<const char*, LightingScheme>{ "CAD", LightingScheme::CAD },
            std::pair<const char*, LightingScheme>{ "Headlamp", LightingScheme::Headlamp }
        };

        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_scheme = loader.readEnumByName(dictionary->get("Subtype"), lightingSchemes.cbegin(), lightingSchemes.cend(), LightingScheme::Artwork);
    }

    return result;
}

PDF3DBackground PDF3DBackground::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDF3DBackground result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);

        // This is a hack to parse color in same way as in another objects
        PDFObject colorSpace = dictionary->get("CS");
        if (colorSpace.isNull())
        {
            colorSpace = PDFObject::createName("DeviceRGB");
        }
        std::vector<PDFReal> color = loader.readNumberArrayFromDictionary(dictionary, "C", { 1.0, 1.0, 1.0});
        PDFArray array;
        array.appendItem(colorSpace);
        for (PDFReal colorComponent : color)
        {
            array.appendItem(PDFObject::createReal(colorComponent));
        }
        PDFObject colorObject = PDFObject::createArray(std::make_shared<PDFArray>(qMove(array)));

        result.m_color = PDF3DAuxiliaryParser::parseColor(storage, colorObject, Qt::white);
        result.m_entireAnnotation = loader.readBooleanFromDictionary(dictionary, "EA", false);
    }

    return result;
}

PDF3DProjection PDF3DProjection::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDF3DProjection result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        constexpr const std::array<std::pair<const char*, Projection>, 2> projections = {
            std::pair<const char*, Projection>{ "O", Projection::Orthographic },
            std::pair<const char*, Projection>{ "P", Projection::Perspective }
        };

        constexpr const std::array<std::pair<const char*, ClippingStyle>, 2> clippingStyles = {
            std::pair<const char*, ClippingStyle>{ "XNF", ClippingStyle::Explicit },
            std::pair<const char*, ClippingStyle>{ "ANF", ClippingStyle::Automatic }
        };

        constexpr const std::array<std::pair<const char*, ScaleMode>, 5> scaleModes = {
            std::pair<const char*, ScaleMode>{ "W", ScaleMode::W },
            std::pair<const char*, ScaleMode>{ "H", ScaleMode::H },
            std::pair<const char*, ScaleMode>{ "Min", ScaleMode::Min },
            std::pair<const char*, ScaleMode>{ "Max", ScaleMode::Max },
            std::pair<const char*, ScaleMode>{ "Absolute", ScaleMode::Absolute }
        };

        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_projection = loader.readEnumByName(dictionary->get("Subtype"), projections.cbegin(), projections.cend(), Projection::Perspective);
        result.m_clippingStyle = loader.readEnumByName(dictionary->get("CS"), clippingStyles.cbegin(), clippingStyles.cend(), ClippingStyle::Automatic);
        result.m_near = loader.readNumberFromDictionary(dictionary, "N", 0.0);
        result.m_far = loader.readNumberFromDictionary(dictionary, "F", qInf());
        result.m_fieldOfViewAngle = loader.readNumberFromDictionary(dictionary, "FOV", 90.0);
        if (dictionary->hasKey("PS"))
        {
            result.m_projectionScalingDiameter = loader.readNumberFromDictionary(dictionary, "PS", 0.0);
            result.m_projectionScaleMode = loader.readEnumByName(dictionary->get("PS"), scaleModes.cbegin(), scaleModes.cend(), ScaleMode::Value);
        }
        result.m_scaleFactor = loader.readNumberFromDictionary(dictionary, "OS", 1.0);
        result.m_scaleMode = result.m_projectionScaleMode = loader.readEnumByName(dictionary->get("OB"), scaleModes.cbegin(), scaleModes.cend(), ScaleMode::Absolute);
    }

    return result;
}

PDF3DView PDF3DView::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDF3DView result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        constexpr const std::array<std::pair<const char*, MatrixSelection>, 2> matrixSelection = {
            std::pair<const char*, MatrixSelection>{ "M", MatrixSelection::M },
            std::pair<const char*, MatrixSelection>{ "U3D", MatrixSelection::U3D }
        };

        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_externalName = loader.readTextStringFromDictionary(dictionary, "XN", QString());
        result.m_internalName = loader.readTextStringFromDictionary(dictionary, "IN", QString());
        result.m_matrixSelection = loader.readEnumByName(dictionary->get("MS"), matrixSelection.cbegin(), matrixSelection.cend(), MatrixSelection::M);
        result.m_cameraToWorld = PDF3DAuxiliaryParser::parseMatrix4x4(storage, dictionary->get("C2W"));
        result.m_U3Dpath = loader.readTextStringList(dictionary->get("U3DPath"));
        result.m_cameraDistance = loader.readNumberFromDictionary(dictionary, "CO", 0.0);
        result.m_projection = PDF3DProjection::parse(storage, dictionary->get("P"));
        result.m_overlay = dictionary->get("O");
        result.m_background = PDF3DBackground::parse(storage, dictionary->get("BG"));
        result.m_renderMode = PDF3DRenderMode::parse(storage, dictionary->get("RM"));
        result.m_lightingScheme = PDF3DLightingScheme::parse(storage, dictionary->get("LS"));
        result.m_crossSections = loader.readObjectList<PDF3DCrossSection>(dictionary->get("SA"));
        result.m_nodes = loader.readObjectList<PDF3DNode>(dictionary->get("NA"));
        result.m_nodesRestore = loader.readBooleanFromDictionary(dictionary, "NR", false);
    }

    return result;
}

PDF3DAnimation PDF3DAnimation::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDF3DAnimation result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        constexpr const std::array<std::pair<const char*, Animation>, 3> animations = {
            std::pair<const char*, Animation>{ "None", Animation::None },
            std::pair<const char*, Animation>{ "Linear", Animation::Linear },
            std::pair<const char*, Animation>{ "Oscillating", Animation::Oscillating }
        };

        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_animation = loader.readEnumByName(dictionary->get("Subtype"), animations.cbegin(), animations.cend(), Animation::None);
        result.m_playCount = loader.readIntegerFromDictionary(dictionary, "PC", -1);
        result.m_speed = loader.readNumberFromDictionary(dictionary, "TM", 1.0);
    }

    return result;
}

PDF3DStream PDF3DStream::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDF3DStream result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        constexpr const std::array<std::pair<const char*, Type>, 2> types = {
            std::pair<const char*, Type>{ "U3D", Type::U3D },
            std::pair<const char*, Type>{ "PRC", Type::PRC }
        };

        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_stream = object;
        result.m_type = loader.readEnumByName(dictionary->get("Subtype"), types.cbegin(), types.cend(), Type::Invalid);
        result.m_views = loader.readObjectList<PDF3DView>(dictionary->get("VA"));

        PDFObject defaultViewObject = storage->getObject(dictionary->get("DV"));
        if (defaultViewObject.isDictionary())
        {
            result.m_defaultView = PDF3DView::parse(storage, defaultViewObject);
        }
        else if (defaultViewObject.isInt())
        {
            PDFInteger index = defaultViewObject.getInteger();
            if (index >= 0 && index < PDFInteger(result.m_views.size()))
            {
                result.m_defaultView = result.m_views[index];
            }
        }
        else if (defaultViewObject.isName() && !result.m_views.empty())
        {
            QByteArray name = defaultViewObject.getString();
            if (name == "F")
            {
                result.m_defaultView = result.m_views.front();
            }
            else if (name == "L")
            {
                result.m_defaultView = result.m_views.back();
            }
        }

        result.m_resources = dictionary->get("Resources");
        result.m_onInstantiateJavascript = dictionary->get("OnInstantiate");
        result.m_animation = PDF3DAnimation::parse(storage, dictionary->get("AN"));
        result.m_colorSpace = dictionary->get("ColorSpace");
    }

    return result;
}

}   // namespace pdf
