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

#ifndef PDFMULTIMEDIA_H
#define PDFMULTIMEDIA_H

#include "pdfobject.h"
#include "pdffile.h"

#include <QColor>
#include <QMatrix4x4>

#include <map>
#include <array>
#include <optional>

namespace pdf
{
class PDFObjectStorage;

using PDFPoint3D = std::array<PDFReal, 3>;

struct PDFMediaMultiLanguageTexts
{
    static PDFMediaMultiLanguageTexts parse(const PDFObjectStorage* storage, PDFObject object);

    std::map<QByteArray, QString> texts;
};

class PDFMediaOffset
{
public:

    enum class Type
    {
        Invalid,
        Time,
        Frame,
        Marker
    };

    struct TimeData
    {
        PDFInteger seconds = 0;
    };

    struct FrameData
    {
        PDFInteger frame = 0;
    };

    struct MarkerData
    {
        QString namedOffset;
    };

    explicit inline PDFMediaOffset() :
        m_type(Type::Invalid)
    {

    }

    template<typename Data>
    explicit inline PDFMediaOffset(Type type, Data&& data) :
        m_type(type),
        m_data(qMove(data))
    {

    }

    static PDFMediaOffset parse(const PDFObjectStorage* storage, PDFObject object);

    Type getType() const { return m_type; }
    const TimeData* getTimeData() const { return std::holds_alternative<TimeData>(m_data) ? &std::get<TimeData>(m_data) : nullptr; }
    const FrameData* getFrameData() const { return std::holds_alternative<FrameData>(m_data) ? &std::get<FrameData>(m_data) : nullptr; }
    const MarkerData* getMarkerData() const { return std::holds_alternative<MarkerData>(m_data) ? &std::get<MarkerData>(m_data) : nullptr; }

private:
    Type m_type;
    std::variant<std::monostate, TimeData, FrameData, MarkerData> m_data;
};

class PDFMediaSoftwareIdentifier
{
public:
    explicit inline PDFMediaSoftwareIdentifier(QByteArray&& software, std::vector<PDFInteger>&& lowVersion, std::vector<PDFInteger>&& highVersion,
                                               bool lowVersionInclusive, bool highVersionInclusive, std::vector<QByteArray>&& languages) :
        m_software(qMove(software)),
        m_lowVersion(qMove(lowVersion)),
        m_highVersion(qMove(highVersion)),
        m_lowVersionInclusive(lowVersionInclusive),
        m_highVersionInclusive(highVersionInclusive),
        m_languages(qMove(languages))
    {

    }

    static PDFMediaSoftwareIdentifier parse(const PDFObjectStorage* storage, PDFObject object);

    const QByteArray& getSoftware() const { return m_software; }
    const std::vector<PDFInteger>& getLowVersion() const { return m_lowVersion; }
    const std::vector<PDFInteger>& getHighVersion() const { return m_highVersion; }
    bool isLowVersionInclusive() const { return m_lowVersionInclusive; }
    bool isHighVersionInclusive() const { return m_highVersionInclusive; }
    const std::vector<QByteArray>& getLanguages() const { return m_languages; }

private:
    QByteArray m_software;
    std::vector<PDFInteger> m_lowVersion;
    std::vector<PDFInteger> m_highVersion;
    bool m_lowVersionInclusive;
    bool m_highVersionInclusive;
    std::vector<QByteArray> m_languages;
};

class PDFMediaPlayer
{
public:
    explicit inline PDFMediaPlayer(PDFMediaSoftwareIdentifier&& softwareIdentifier) :
        m_softwareIdentifier(qMove(softwareIdentifier))
    {

    }

    static PDFMediaPlayer parse(const PDFObjectStorage* storage, PDFObject object);

    const PDFMediaSoftwareIdentifier* getSoftwareIdentifier() const { return &m_softwareIdentifier; }

private:
    PDFMediaSoftwareIdentifier m_softwareIdentifier;
};

class PDFMediaPlayers
{
public:
    explicit inline PDFMediaPlayers() = default;

    explicit inline PDFMediaPlayers(std::vector<PDFMediaPlayer>&& playersMustUsed,
                                    std::vector<PDFMediaPlayer>&& playersAlternate,
                                    std::vector<PDFMediaPlayer>&& playersNeverUsed) :
        m_playersMustUsed(qMove(playersMustUsed)),
        m_playersAlternate(qMove(playersAlternate)),
        m_playersNeverUsed(qMove(playersNeverUsed))
    {

    }

    static PDFMediaPlayers parse(const PDFObjectStorage* storage, PDFObject object);

    const std::vector<PDFMediaPlayer>& getPlayersMustUsed() const { return m_playersMustUsed; }
    const std::vector<PDFMediaPlayer>& getPlayersAlternate() const { return m_playersAlternate; }
    const std::vector<PDFMediaPlayer>& getPlayersNeverUsed() const { return m_playersNeverUsed; }

private:
    std::vector<PDFMediaPlayer> m_playersMustUsed;
    std::vector<PDFMediaPlayer> m_playersAlternate;
    std::vector<PDFMediaPlayer> m_playersNeverUsed;
};

class PDFMediaPermissions
{
public:

    /// Are we allowed to save temporary file to play rendition?
    enum class Permission
    {
        Never,
        Extract,
        Access,
        Always
    };

    explicit inline PDFMediaPermissions() :
        m_permission(Permission::Never)
    {

    }

    explicit inline PDFMediaPermissions(Permission permission) :
        m_permission(permission)
    {

    }

    static PDFMediaPermissions parse(const PDFObjectStorage* storage, PDFObject object);

    Permission getPermission() const { return m_permission; }

private:
    Permission m_permission;
};

class PDFMediaPlayParameters
{
public:
    explicit inline PDFMediaPlayParameters() = default;

    enum class FitMode
    {
        Meet,
        Slice,
        Fill,
        Scroll,
        Hidden,
        Default
    };

    enum class Duration
    {
        Intrinsic,
        Infinity,
        Seconds
    };

    struct PlayParameters
    {
        PDFInteger volume = 100;
        bool controllerUserInterface = false;
        FitMode fitMode = FitMode::Default;
        bool playAutomatically = true;
        PDFReal repeat = 1.0;
        Duration duration = Duration::Intrinsic;
        PDFReal durationSeconds = 0.0;
    };

    static PDFMediaPlayParameters parse(const PDFObjectStorage* storage, PDFObject object);

    const PDFMediaPlayers* getPlayers() const { return &m_players; }
    const PlayParameters* getPlayParametersMustHonored() const { return &m_mustHonored; }
    const PlayParameters* getPlayParametersBestEffort() const { return &m_bestEffort; }

private:
    PDFMediaPlayers m_players;
    PlayParameters m_mustHonored;
    PlayParameters m_bestEffort;
};

class PDFMediaScreenParameters
{
public:

    enum class WindowType
    {
        Floating,
        FullScreen,
        Hidden,
        ScreenAnnotation
    };

    enum class WindowRelativeTo
    {
        DocumentWindow,
        ApplicationWindow,
        VirtualDesktop,
        Monitor
    };

    enum class OffscreenMode
    {
        NoAction,
        MoveOnScreen,
        NonViable
    };

    enum class ResizeMode
    {
        Fixed,
        ResizableKeepAspectRatio,
        Resizeble
    };

    struct ScreenParameters
    {
        WindowType windowType = WindowType::ScreenAnnotation;
        QColor backgroundColor = QColor(Qt::white);
        PDFReal opacity = 1.0;
        PDFInteger monitorSpecification = 0;

        QSize floatingWindowSize;
        WindowRelativeTo floatingWindowReference = WindowRelativeTo::DocumentWindow;
        Qt::Alignment floatingWindowAlignment = Qt::AlignCenter;
        OffscreenMode floatingWindowOffscreenMode = OffscreenMode::MoveOnScreen;
        bool floatingWindowHasTitleBar = true;
        bool floatingWindowCloseable = true;
        ResizeMode floatingWindowResizeMode = ResizeMode::Fixed;
        PDFMediaMultiLanguageTexts floatingWindowTitle;
    };

    explicit inline PDFMediaScreenParameters() = default;
    explicit inline PDFMediaScreenParameters(ScreenParameters&& mustHonored, ScreenParameters&& bestEffort) :
        m_mustHonored(qMove(mustHonored)),
        m_bestEffort(qMove(bestEffort))
    {

    }

    static PDFMediaScreenParameters parse(const PDFObjectStorage* storage, PDFObject object);

    const ScreenParameters* getScreenParametersMustHonored() const { return &m_mustHonored; }
    const ScreenParameters* getScreenParametersBestEffort() const { return &m_bestEffort; }

private:
    ScreenParameters m_mustHonored;
    ScreenParameters m_bestEffort;
};

class PDFMediaClip
{
public:
    struct MediaClipData
    {
        QString name;
        PDFFileSpecification fileSpecification;
        PDFObject dataStream;
        QByteArray contentType;
        PDFMediaPermissions permissions;
        PDFMediaMultiLanguageTexts alternateTextDescriptions;
        PDFMediaPlayers players;
        QByteArray m_baseUrlMustHonored;
        QByteArray m_baseUrlBestEffort;
    };

    struct MediaSectionBeginEnd
    {
        PDFMediaOffset offsetBegin;
        PDFMediaOffset offsetEnd;
    };

    struct MediaSectionData
    {
        QString name;
        PDFMediaMultiLanguageTexts alternateTextDescriptions;
        MediaSectionBeginEnd m_mustHonored;
        MediaSectionBeginEnd m_bestEffort;
    };

    explicit inline PDFMediaClip() = default;

    explicit inline PDFMediaClip(MediaClipData&& mediaClipData, std::vector<MediaSectionData>&& sections) :
        m_mediaClipData(qMove(mediaClipData)),
        m_sections(qMove(sections))
    {

    }

    static PDFMediaClip parse(const PDFObjectStorage* storage, PDFObject object);

    const MediaClipData& getMediaClipData() const { return m_mediaClipData; }
    const std::vector<MediaSectionData>& getClipSections() const { return m_sections; }

private:
    MediaClipData m_mediaClipData;
    std::vector<MediaSectionData> m_sections;
};

class PDFMediaMinimumBitDepth
{
public:
    explicit inline PDFMediaMinimumBitDepth(PDFInteger screenMinimumBitDepth, PDFInteger monitorSpecifier) :
        m_screenMinimumBitDepth(screenMinimumBitDepth),
        m_monitorSpecifier(monitorSpecifier)
    {

    }

    static PDFMediaMinimumBitDepth parse(const PDFObjectStorage* storage, PDFObject object);

    PDFInteger getScreenMinimumBitDepth() const { return m_screenMinimumBitDepth; }
    PDFInteger getMonitorSpecifier() const { return m_monitorSpecifier; }

private:
    PDFInteger m_screenMinimumBitDepth;
    PDFInteger m_monitorSpecifier;
};

class PDFMediaMinimumScreenSize
{
public:
    explicit inline PDFMediaMinimumScreenSize(PDFInteger minimumWidth, PDFInteger minimumHeight, PDFInteger monitorSpecifier) :
        m_minimumWidth(minimumWidth),
        m_minimumHeight(minimumHeight),
        m_monitorSpecifier(monitorSpecifier)
    {

    }

    static PDFMediaMinimumScreenSize parse(const PDFObjectStorage* storage, PDFObject object);

    PDFInteger getMinimumWidth() const { return m_minimumWidth; }
    PDFInteger getMinimumHeight() const { return m_minimumHeight; }
    PDFInteger getMonitorSpecifier() const { return m_monitorSpecifier; }

private:
    PDFInteger m_minimumWidth;
    PDFInteger m_minimumHeight;
    PDFInteger m_monitorSpecifier;
};

/// Media critera object (see PDF 1.7 reference, chapter 9.1.2). Some values are optional,
/// so they are implemented using std::optional. Always call "has" functions before
/// accessing the getters.
class PDFMediaCriteria
{
public:
    explicit inline PDFMediaCriteria() = default;

    static PDFMediaCriteria parse(const PDFObjectStorage* storage, PDFObject object);

    bool hasAudioDescriptions() const { return m_audioDescriptions.has_value(); }
    bool hasTextCaptions() const { return m_textCaptions.has_value(); }
    bool hasAudioOverdubs() const { return m_audioOverdubs.has_value(); }
    bool hasSubtitles() const { return m_subtitles.has_value(); }
    bool hasBitrate() const { return m_bitrate.has_value(); }
    bool hasMinimumBitDepth() const { return m_minimumBitDepth.has_value(); }
    bool hasMinimumScreenSize() const { return m_minimumScreenSize.has_value(); }
    bool hasViewers() const { return m_viewers.has_value(); }
    bool hasMinimumPdfVersion() const { return m_minimumPdfVersion.has_value(); }
    bool hasMaximumPdfVersion() const { return m_maximumPdfVersion.has_value(); }
    bool hasLanguages() const { return m_languages.has_value(); }

    bool getAudioDescriptions() const { return m_audioDescriptions.value(); }
    bool getTextCaptions() const { return m_textCaptions.value(); }
    bool getAudioOverdubs() const { return m_audioOverdubs.value(); }
    bool getSubtitles() const { return m_subtitles.value(); }
    PDFInteger getBitrate() const { return m_bitrate.value(); }
    const PDFMediaMinimumBitDepth& getMinimumBitDepth() const { return m_minimumBitDepth.value(); }
    const PDFMediaMinimumScreenSize& getMinimumScreenSize() const { return m_minimumScreenSize.value(); }
    const std::vector<PDFMediaSoftwareIdentifier>& getViewers() const { return m_viewers.value(); }
    const QByteArray& getMinimumPdfVersion() const { return m_minimumPdfVersion.value(); }
    const QByteArray& getMaximumPdfVersion() const { return m_maximumPdfVersion.value(); }
    const std::vector<QByteArray>& getLanguages() const { return m_languages.value(); }

private:
    std::optional<bool> m_audioDescriptions;
    std::optional<bool> m_textCaptions;
    std::optional<bool> m_audioOverdubs;
    std::optional<bool> m_subtitles;
    std::optional<PDFInteger> m_bitrate;
    std::optional<PDFMediaMinimumBitDepth> m_minimumBitDepth;
    std::optional<PDFMediaMinimumScreenSize> m_minimumScreenSize;
    std::optional<std::vector<PDFMediaSoftwareIdentifier>> m_viewers;
    std::optional<QByteArray> m_minimumPdfVersion;
    std::optional<QByteArray> m_maximumPdfVersion;
    std::optional<std::vector<QByteArray>> m_languages;
};

/// Rendition object
class PDFRendition
{
public:

    enum class Type
    {
        Invalid,
        Media,
        Selector
    };

    struct MediaRenditionData
    {
        PDFMediaClip clip;
        PDFMediaPlayParameters playParameters;
        PDFMediaScreenParameters screenParameters;
    };

    struct SelectorRenditionData
    {
        PDFObject renditions;
    };

    static PDFRendition parse(const PDFObjectStorage* storage, PDFObject object);

    Type getType() const { return m_type; }
    const QString& getName() const { return m_name; }
    const PDFMediaCriteria* getMediaCriteriaMustHonored() const { return &m_mustHonored; }
    const PDFMediaCriteria* getMediaCriteriaBestEffort() const { return &m_bestEffort; }
    const MediaRenditionData* getMediaRenditionData() const { return std::holds_alternative<MediaRenditionData>(m_data) ? &std::get<MediaRenditionData>(m_data) : nullptr; }
    const SelectorRenditionData* getSelectorRenditionData() const { return std::holds_alternative<SelectorRenditionData>(m_data) ? &std::get<SelectorRenditionData>(m_data) : nullptr; }

private:
    Type m_type = Type::Invalid;
    QString m_name;

    PDFMediaCriteria m_mustHonored;
    PDFMediaCriteria m_bestEffort;
    std::variant<std::monostate, MediaRenditionData, SelectorRenditionData> m_data;
};

/// Sound object, see chapter 9.2 in PDF 1.7 reference
class PDFSound
{
public:
    explicit inline PDFSound() = default;

    enum class Format
    {
        Raw,
        Signed,
        muLaw,
        ALaw
    };

    const PDFFileSpecification* getFileSpecification() const { return &m_fileSpecification; }
    PDFReal getSamplingRate() const { return m_samplingRate; }
    PDFInteger getChannels() const { return m_channels; }
    PDFInteger getBitsPerSample() const { return m_bitsPerSample; }
    Format getFormat() const { return m_format; }
    const QByteArray& getSoundCompression() { return m_soundCompression; }
    const PDFObject& getSoundCompressionParameters() const { return m_soundCompressionParameters; }
    const PDFStream* getStream() const { return m_streamObject.isStream() ? m_streamObject.getStream() : nullptr; }

    /// If this function returns true, sound is valid
    bool isValid() const { return getStream(); }

    /// Creates a new sound from the object. If data are invalid, then invalid object
    /// is returned, no exception is thrown.
    static PDFSound parse(const PDFObjectStorage* storage, PDFObject object);

private:
    PDFFileSpecification m_fileSpecification;
    PDFReal m_samplingRate = 0.0;
    PDFInteger m_channels = 0;
    PDFInteger m_bitsPerSample = 0;
    Format m_format = Format::Raw;
    QByteArray m_soundCompression;
    PDFObject m_soundCompressionParameters;
    PDFObject m_streamObject;
};

/// Movie object, see chapter 9.3 in PDF 1.7 reference
class PDFMovie
{
public:
    explicit inline PDFMovie() = default;

    const PDFFileSpecification* getMovieFileSpecification() const { return &m_movieFile; }
    QSize getWindowSize() const { return m_windowSize; }
    PDFInteger getRotationAngle() const { return m_rotationAngle; }
    bool isPosterVisible() const { return m_showPoster; }
    const PDFObject& getPosterObject() const { return m_poster; }

    /// Creates a new movie from the object. If data are invalid, then invalid object
    /// is returned, no exception is thrown.
    static PDFMovie parse(const PDFObjectStorage* storage, PDFObject object);

private:
    PDFFileSpecification m_movieFile;
    QSize m_windowSize;
    PDFInteger m_rotationAngle = 0;
    bool m_showPoster = false;
    PDFObject m_poster;
};

/// Movie activation object, see table 9.31 in PDF 1.7 reference
class PDFMovieActivation
{
public:
    explicit inline PDFMovieActivation() = default;

    struct MovieTime
    {
        PDFInteger value = 0;
        std::optional<PDFInteger> unitsPerSecond;
    };

    enum class Mode
    {
        Once,
        Open,
        Repeat,
        Palindrome
    };

    MovieTime getStartTime() const { return m_start; }
    MovieTime getDuration() const { return m_duration; }
    PDFReal getRate() const { return m_rate; }
    PDFReal getVolume() const { return m_volume; }
    bool isShowControls() const { return m_showControls; }
    bool isSynchronous() const { return m_synchronous; }
    Mode getMode() const { return m_mode; }
    bool hasScale() const { return m_scaleDenominator != 0; }
    PDFInteger getScaleNumerator() const { return m_scaleNumerator; }
    PDFInteger getScaleDenominator() const { return m_scaleDenominator; }
    QPointF getRelativeWindowPosition() const { return m_relativeWindowPosition; }

    /// Creates a new movie from the object. If data are invalid, then invalid object
    /// is returned, no exception is thrown.
    static PDFMovieActivation parse(const PDFObjectStorage* storage, PDFObject object);

private:
    static MovieTime parseMovieTime(const PDFObjectStorage* storage, PDFObject object);
    static PDFInteger parseMovieTimeFromString(const QByteArray& string);

    MovieTime m_start;
    MovieTime m_duration;
    PDFReal m_rate = 1.0;
    PDFReal m_volume = 1.0;
    bool m_showControls = false;
    bool m_synchronous = false;
    Mode m_mode = Mode::Once;
    PDFInteger m_scaleNumerator = 0;
    PDFInteger m_scaleDenominator = 0;
    QPointF m_relativeWindowPosition;
};

/// Rich media position of window
class PDFRichMediaWindowPosition
{
public:
    explicit inline PDFRichMediaWindowPosition() = default;

    enum class Alignment
    {
        Near,
        Center,
        Far
    };

    Alignment getHorizontalAlignment() const { return m_horizontalAlignment; }
    Alignment getVerticalAlignment() const { return m_verticalAlignment; }
    PDFReal getHorizontalOffset() const { return m_horizontalOffset; }
    PDFReal getVerticalOffset() const { return m_verticalOffset; }

    /// Creates a new rich media position from the object. If data are invalid, then invalid object
    /// is returned, no exception is thrown.
    static PDFRichMediaWindowPosition parse(const PDFObjectStorage* storage, PDFObject object);

private:
    Alignment m_horizontalAlignment = Alignment::Far;
    Alignment m_verticalAlignment = Alignment::Near;
    PDFReal m_horizontalOffset = 18.0;
    PDFReal m_verticalOffset = 18.0;
};

/// Rich media window
class PDFRichMediaWindow
{
public:
    explicit inline PDFRichMediaWindow() = default;

    enum SizeHintType
    {
        Default,
        Max,
        Min,
        End
    };

    PDFReal getWidth(SizeHintType type) const { return m_width.at(type); }
    PDFReal getHeight(SizeHintType type) const { return m_height.at(type); }
    const PDFRichMediaWindowPosition& getRichMediaWindowPosition() const { return m_richMediaPosition; }

    using WindowSize = std::array<PDFReal, End>;

    /// Creates a new rich media window from the object. If data are invalid, then invalid object
    /// is returned, no exception is thrown.
    static PDFRichMediaWindow parse(const PDFObjectStorage* storage, PDFObject object);

private:
    WindowSize m_width = { 288.0, 576.0, 72.0 };
    WindowSize m_height = { 288.0, 576.0, 72.0 };
    PDFRichMediaWindowPosition m_richMediaPosition;
};

/// Rich media presentation defines, how rich media will appear on the page,
/// or if it should be launched in a new window.
class PDFRichMediaPresentation
{
public:
    explicit inline PDFRichMediaPresentation() = default;

    enum class Style
    {
        Embedded,
        Windowed
    };

    Style getStyle() const { return m_style; }
    const PDFRichMediaWindow& getWindow() const { return m_window; }
    bool isTransparent() const { return m_isTransparent; }
    bool isNavigationPaneEnabled() const { return m_isNavigationPaneEnabled; }
    bool isToolbarEnabled() const { return m_isToolbarEnabled; }
    bool isContentClickPassed() const { return m_isContentClickPassed; }

    /// Creates a new rich media presentation from the object. If data are invalid, then invalid object
    /// is returned, no exception is thrown.
    static PDFRichMediaPresentation parse(const PDFObjectStorage* storage, PDFObject object);

private:
    Style m_style = Style::Embedded;
    PDFRichMediaWindow m_window; ///< Meaningful only when style is windowed
    bool m_isTransparent = false;
    bool m_isNavigationPaneEnabled = false;
    bool m_isToolbarEnabled = false;
    bool m_isContentClickPassed = false;
};

/// Rich media animation
class PDFRichMediaAnimation
{
public:
    explicit inline PDFRichMediaAnimation() = default;

    enum class Animation
    {
        None,
        Linear,
        Oscillating
    };

    Animation getAnimation() const { return m_animation; }
    PDFInteger getPlayCount() const { return m_playCount; }
    PDFReal getSpeed() const { return m_speed; }

    /// Creates a new rich media animation from the object. If data are invalid, then invalid object
    /// is returned, no exception is thrown.
    static PDFRichMediaAnimation parse(const PDFObjectStorage* storage, PDFObject object);

private:
    Animation m_animation = Animation::None;
    PDFInteger m_playCount = -1;
    PDFReal m_speed = 1;
};

/// Rich media deactivation dictionary
class PDFRichMediaDeactivation
{
public:
    explicit inline PDFRichMediaDeactivation() = default;

    enum class DeactivationMode
    {
        ExplicitlyByUserAction,
        PageLoseFocus,
        PageHide
    };

    DeactivationMode getMode() const { return m_mode; }

    /// Creates a new rich media deactivation from the object. If data are invalid, then invalid object
    /// is returned, no exception is thrown.
    static PDFRichMediaDeactivation parse(const PDFObjectStorage* storage, PDFObject object);

private:
    DeactivationMode m_mode = DeactivationMode::ExplicitlyByUserAction;
};

/// Rich media activation dictionary
class PDFRichMediaActivation
{
public:
    explicit inline PDFRichMediaActivation() = default;

    enum class ActivationMode
    {
        ExplicitlyByUserAction,
        PageEnterFocus,
        PageShow
    };

    ActivationMode getActivationMode() const { return m_mode; }
    const PDFRichMediaAnimation& getAnimation() const { return m_animation; }
    PDFObjectReference getView() const { return m_view; }
    PDFObjectReference getConfiguration() const { return m_configuration; }
    const PDFRichMediaPresentation& getPresentation() const { return m_presentation; }

    /// Creates a new rich media activation from the object. If data are invalid, then invalid object
    /// is returned, no exception is thrown.
    static PDFRichMediaActivation parse(const PDFObjectStorage* storage, PDFObject object);

private:
    ActivationMode m_mode = ActivationMode::ExplicitlyByUserAction;
    PDFRichMediaAnimation m_animation;
    PDFObjectReference m_view;
    PDFObjectReference m_configuration;
    PDFRichMediaPresentation m_presentation;
};

/// Rich media settings
class PDFRichMediaSettings
{
public:
    explicit inline PDFRichMediaSettings() = default;

    const PDFRichMediaActivation& getActivation() const { return m_activation; }
    const PDFRichMediaDeactivation& getDeactivation() const { return m_deactivation; }

    /// Creates a new rich media settings from the object. If data are invalid, then invalid object
    /// is returned, no exception is thrown.
    static PDFRichMediaSettings parse(const PDFObjectStorage* storage, PDFObject object);

private:
    PDFRichMediaActivation m_activation;
    PDFRichMediaDeactivation m_deactivation;
};

/// Rich media content dictionary
class PDFRichMediaContent
{
public:
    explicit inline PDFRichMediaContent() = default;

    using Assets = std::map<QByteArray, PDFFileSpecification>;
    using Configurations = std::vector<PDFObjectReference>;
    using Views = std::vector<PDFObjectReference>;

    const Assets& getAssets() const { return m_assets; }
    const Configurations& getConfigurations() const { return m_configurations; }
    const Views& getViews() const { return m_views; }

    /// Creates a new rich media content from the object. If data are invalid, then invalid object
    /// is returned, no exception is thrown.
    static PDFRichMediaContent parse(const PDFObjectStorage* storage, PDFObject object);

private:
    Assets m_assets;
    Configurations m_configurations;
    Views m_views;
};

enum class RichMediaType
{
    Unspecified,
    _3D,
    Sound,
    Flash,
    Video
};

/// Rich media configuration dictionary
class PDFRichMediaConfiguration
{
public:
    explicit inline PDFRichMediaConfiguration() = default;

    using Instances = std::vector<PDFObjectReference>;

    RichMediaType getType() const { return m_type; }
    const Instances& getInstances() const { return m_instances; }

    /// Creates a new rich media configuration from the object. If data are invalid, then invalid object
    /// is returned, no exception is thrown.
    static PDFRichMediaConfiguration parse(const PDFObjectStorage* storage, PDFObject object);

private:
    RichMediaType m_type = RichMediaType::Unspecified;
    Instances m_instances;
};

/// Rich media instance dictionary
class PDFRichMediaInstance
{
public:
    explicit inline PDFRichMediaInstance() = default;

    RichMediaType getType() const { return m_type; }
    PDFObjectReference getAsset() const { return m_asset; }

    /// Creates a new rich media instance from the object. If data are invalid, then invalid object
    /// is returned, no exception is thrown.
    static PDFRichMediaInstance parse(const PDFObjectStorage* storage, PDFObject object);

private:
    RichMediaType m_type = RichMediaType::Unspecified;
    PDFObjectReference m_asset;
};

/// 3D PDF activation dictionary
class PDF3DActivation
{
public:

    /// Activation/deactivation mode
    enum class TriggerMode
    {
        ExplicitlyByUserAction, ///< Explicitly turned on/off by user
        PageFocus,              ///< Turned on/off by gain/lose focus
        PageVisibility          ///< Turned on/off by page shown/hidden
    };

    /// Activation mode
    enum class ActivationMode
    {
        Uninstantiated,
        Instantiated,
        Live
    };

    /// Style
    enum class Style
    {
        Embedded,
        Windowed
    };

    TriggerMode getActivationTriggerMode() const { return m_activationTriggerMode; }
    TriggerMode getDeactivationTriggerMode() const { return m_deactivationTriggerMode; }
    ActivationMode getActivationMode() const { return m_activationMode; }
    ActivationMode getDeactivationMode() const { return m_deactivationMode; }
    bool hasToolbar() const { return m_toolbar; }
    bool hasNavigator() const { return m_navigator; }
    Style getStyle() const { return m_style; }
    const PDFRichMediaWindow& getWindow() const { return m_window; }
    bool isTransparent() const { return m_transparent; }

    /// Creates a new 3D annotation activation from the object. If data are invalid, then invalid object
    /// is returned, no exception is thrown.
    static PDF3DActivation parse(const PDFObjectStorage* storage, PDFObject object);

private:
    TriggerMode m_activationTriggerMode = TriggerMode::ExplicitlyByUserAction;
    TriggerMode m_deactivationTriggerMode = TriggerMode::PageVisibility;
    ActivationMode m_activationMode = ActivationMode::Live;
    ActivationMode m_deactivationMode = ActivationMode::Uninstantiated;
    bool m_toolbar = true;
    bool m_navigator = false;
    Style m_style = Style::Embedded;
    PDFRichMediaWindow m_window;
    bool m_transparent = false;
};

/// 3D PDF render mode
class PDF3DRenderMode
{
public:
    explicit inline PDF3DRenderMode() = default;

    enum class RenderMode
    {
        Solid,
        SolidWireframe,
        Transparent,
        TransparentWireframe,
        BoundingBox,
        TransparentBoundingBox,
        TransparentBoundingBoxOutline,
        Wireframe,
        ShadedWireframe,
        HiddenWireframe,
        Vertices,
        ShadedVertices,
        Illustration,
        SolidOutline,
        ShadedIllustration
    };

    enum class FaceColorMode
    {
        BG,     ///< Current background color
        Color   ///< Color specified
    };

    RenderMode getRenderMode() const { return m_renderMode; }
    QColor getAuxiliaryColor() const { return m_auxiliaryColor; }
    QColor getFaceColor() const { return m_faceColor; }
    FaceColorMode getFaceColorMode() const { return m_faceColorMode; }
    PDFReal getOpacity() const { return m_opacity; }
    PDFReal getCreaseAngle() const { return m_creaseAngle; }

    /// Creates a new 3D render mode from the object. If data are invalid, then invalid object
    /// is returned, no exception is thrown.
    static PDF3DRenderMode parse(const PDFObjectStorage* storage, PDFObject object);

private:
    RenderMode m_renderMode = RenderMode::Solid;
    QColor m_auxiliaryColor = Qt::black;
    QColor m_faceColor = Qt::black;
    FaceColorMode m_faceColorMode = FaceColorMode::BG;
    PDFReal m_opacity = 0.5;
    PDFReal m_creaseAngle = 45.0;
};

/// 3D PDF node (part of 3D structure)
class PDF3DNode
{
public:
    explicit inline PDF3DNode() = default;

    const QString& getName() const { return m_name; }
    const std::optional<PDFReal>& getOpacity() const { return m_opacity; }
    const std::optional<bool>& getVisibility() const { return m_visibility; }
    const std::optional<QMatrix4x4>& getMatrix() const { return m_matrix; }
    PDFObjectReference getInstance() const { return m_instance; }
    const QString& getData() const { return m_data; }
    const std::optional<PDF3DRenderMode>& getRenderMode() const { return m_renderMode; }

    /// Creates a new 3D node from the object. If data are invalid, then invalid object
    /// is returned, no exception is thrown.
    static PDF3DNode parse(const PDFObjectStorage* storage, PDFObject object);

private:
    QString m_name;
    std::optional<PDFReal> m_opacity;
    std::optional<bool> m_visibility;
    std::optional<QMatrix4x4> m_matrix;
    PDFObjectReference m_instance;
    QString m_data;
    std::optional<PDF3DRenderMode> m_renderMode;
};

/// 3D PDF Cross section
class PDF3DCrossSection
{
public:
    explicit inline PDF3DCrossSection() = default;

    enum class Direction
    {
        X,
        Y,
        Z
    };

    PDFPoint3D getCenterOfRotation() const { return m_centerOfRotation; }
    PDFPoint3D getRotationAngles() const { return m_rotationAngles; }
    Direction getPerpendicularDirection() const { return m_perpendicularDirection; }
    PDFReal getCutPlaneOpacity() const { return m_cutPlaneOpacity; }
    QColor getCutPlaneColor() const { return m_cutPlaneColor; }
    bool getIntersectionVisibility() const { return m_intersectionVisibility; }
    QColor getIntersectionColor() const { return m_intersectionColor; }
    bool getShowTransparent() const { return m_showTransparent; }
    bool getSectionCapping() const { return m_sectionCapping; }

    /// Creates a new 3D cross section from the object. If data are invalid, then invalid object
    /// is returned, no exception is thrown.
    static PDF3DCrossSection parse(const PDFObjectStorage* storage, PDFObject object);

private:
    PDFPoint3D m_centerOfRotation{};
    PDFPoint3D m_rotationAngles{};
    Direction m_perpendicularDirection = Direction::X;
    PDFReal m_cutPlaneOpacity = 0.5;
    QColor m_cutPlaneColor = Qt::white;
    bool m_intersectionVisibility = false;
    QColor m_intersectionColor = Qt::green;
    bool m_showTransparent = false;
    bool m_sectionCapping = false;
};

/// 3D PDF lighting scheme
class PDF3DLightingScheme
{
public:
    explicit inline PDF3DLightingScheme() = default;

    enum class LightingScheme
    {
        Artwork,
        None,
        White,
        Day,
        Night,
        Hard,
        Primary,
        Blue,
        Red,
        Cube,
        CAD,
        Headlamp
    };

    LightingScheme getLightingScheme() const { return m_scheme; }

    /// Creates a new 3D lighting scheme from the object. If data are invalid, then invalid object
    /// is returned, no exception is thrown.
    static PDF3DLightingScheme parse(const PDFObjectStorage* storage, PDFObject object);

private:
    LightingScheme m_scheme = LightingScheme::Artwork;
};

/// 3D PDF background
class PDF3DBackground
{
public:
    explicit inline PDF3DBackground() = default;

    QColor getColor() const { return m_color; }
    bool getEntireAnnotation() const { return m_entireAnnotation; }

    /// Creates a new 3D background from the object. If data are invalid, then invalid object
    /// is returned, no exception is thrown.
    static PDF3DBackground parse(const PDFObjectStorage* storage, PDFObject object);

private:
    QColor m_color;
    bool m_entireAnnotation = false;
};

/// 3D PDF projection
class PDF3DProjection
{
public:
    explicit inline PDF3DProjection() = default;

    enum class Projection
    {
        Orthographic,
        Perspective
    };

    enum class ClippingStyle
    {
        Explicit,
        Automatic
    };

    enum class ScaleMode
    {
        W,
        H,
        Min,
        Max,
        Absolute,
        Value       ///< This means projection scaling diameter is defined
    };

    Projection getProjection() const { return m_projection; }
    ClippingStyle getClippingStyle() const { return m_clippingStyle; }
    PDFReal getNearPlane() const { return m_near; }
    PDFReal getFarPlane() const { return m_far; }
    PDFReal getFieldOfViewAngle() const { return m_fieldOfViewAngle; }
    PDFReal getProjectionScalingDiameter() const { return m_projectionScalingDiameter; }
    ScaleMode getProjectionScaleMode() const { return m_projectionScaleMode; }
    PDFReal getScaleFactor() const { return m_scaleFactor; }
    ScaleMode getScaleMode() const { return m_scaleMode; }

    /// Creates a new 3D projection from the object. If data are invalid, then invalid object
    /// is returned, no exception is thrown.
    static PDF3DProjection parse(const PDFObjectStorage* storage, PDFObject object);

private:
    Projection m_projection = Projection::Perspective;
    ClippingStyle m_clippingStyle = ClippingStyle::Automatic;
    PDFReal m_near = 0.0;
    PDFReal m_far = qInf();
    PDFReal m_fieldOfViewAngle = 90.0;
    PDFReal m_projectionScalingDiameter = 0.0;
    ScaleMode m_projectionScaleMode = ScaleMode::W;
    PDFReal m_scaleFactor = 1.0;
    ScaleMode m_scaleMode = ScaleMode::Absolute;
};

/// 3D PDF view
class PDF3DView
{
public:
    explicit inline PDF3DView() = default;

    enum class MatrixSelection
    {
        M,
        U3D
    };

    const QString& getExternalName() const { return m_externalName; }
    const QString& getInternalName() const { return m_internalName; }
    MatrixSelection getMatrixSelection() const{ return m_matrixSelection; }
    const QMatrix4x4& getCameraToWorld() const { return m_cameraToWorld; }
    const QStringList& getU3DPath() const { return m_U3Dpath; }
    PDFReal getCameraDistance() const { return m_cameraDistance; }
    const PDF3DProjection& getProjection() const { return m_projection; }
    const PDFObject& getOverlay() const { return m_overlay; }
    const PDF3DBackground& getBackground() const { return m_background; }
    const PDF3DRenderMode& getRenderMode() const { return m_renderMode; }
    const PDF3DLightingScheme& getLightingScheme() const { return m_lightingScheme; }
    const std::vector<PDF3DCrossSection>& getCrossSections() const { return m_crossSections; }
    const std::vector<PDF3DNode>& getNodes() const { return m_nodes; }
    bool getNodesRestore() const { return m_nodesRestore; }

    /// Creates a new 3D view from the object. If data are invalid, then invalid object
    /// is returned, no exception is thrown.
    static PDF3DView parse(const PDFObjectStorage* storage, PDFObject object);

private:
    QString m_externalName;
    QString m_internalName;
    MatrixSelection m_matrixSelection = MatrixSelection::M;
    QMatrix4x4 m_cameraToWorld;
    QStringList m_U3Dpath;
    PDFReal m_cameraDistance = 0.0;
    PDF3DProjection m_projection;
    PDFObject m_overlay;
    PDF3DBackground m_background;
    PDF3DRenderMode m_renderMode;
    PDF3DLightingScheme m_lightingScheme;
    std::vector<PDF3DCrossSection> m_crossSections;
    std::vector<PDF3DNode> m_nodes;
    bool m_nodesRestore = false;
};

/// 3D PDF animation
class PDF3DAnimation
{
public:
    explicit inline PDF3DAnimation() = default;

    enum class Animation
    {
        None,
        Linear,
        Oscillating
    };

    Animation getAnimation() const { return m_animation; }
    PDFInteger getPlayCount() const { return m_playCount; }
    PDFReal getSpeed() const { return m_speed; }

    /// Creates a new 3D animation from the object. If data are invalid, then invalid object
    /// is returned, no exception is thrown.
    static PDF3DAnimation parse(const PDFObjectStorage* storage, PDFObject object);

private:
    Animation m_animation = Animation::None;
    PDFInteger m_playCount = -1;
    PDFReal m_speed = 1;
};

/// 3D PDF stream
class PDF3DStream
{
public:
    explicit inline PDF3DStream() = default;

    enum class Type
    {
        Invalid,
        U3D,
        PRC
    };

    PDFObject getStream() const { return m_stream; }
    Type getType() const { return m_type; }
    const std::vector<PDF3DView>& getViews() const { return m_views; }
    const std::optional<PDF3DView>& getDefaultView() const { return m_defaultView; }
    PDFObject getResources() const { return m_resources; }
    PDFObject getOnInstantiateJavascript() const { return m_onInstantiateJavascript; }
    PDF3DAnimation getAnimation() const { return m_animation; }
    PDFObject getColorSpace() const { return m_colorSpace; }

    /// Creates a new 3D stream from the object. If data are invalid, then invalid object
    /// is returned, no exception is thrown.
    static PDF3DStream parse(const PDFObjectStorage* storage, PDFObject object);

private:
    PDFObject m_stream;
    Type m_type = Type::Invalid;
    std::vector<PDF3DView> m_views;
    std::optional<PDF3DView> m_defaultView;
    PDFObject m_resources;
    PDFObject m_onInstantiateJavascript;
    PDF3DAnimation m_animation;
    PDFObject m_colorSpace;
};

}   // namespace pdf

#endif // PDFMULTIMEDIA_H
