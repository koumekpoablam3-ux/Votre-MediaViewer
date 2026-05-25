#include "AudioPlayer.h"
#include "PlaylistWidget.h"

#include <QApplication>
#include <QStyle>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QScreen>
#include <QDebug>

// ============================================================
// Constructor
// ============================================================

AudioPlayer::AudioPlayer(QWidget *parent)
    : QWidget(parent)
    , m_isPlaying(false)
    , m_shuffleEnabled(false)
    , m_repeatMode(RepeatMode::None)
    , m_currentTrackIndex(-1)
{
    // Create Qt Multimedia objects
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);

    // Default volume
    m_audioOutput->setVolume(0.7);

    // Create playlist widget
    m_playlistWidget = new PlaylistWidget(this);

    setupUi();
    setupConnections();
}

// ============================================================
// UI Setup
// ============================================================

void AudioPlayer::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(12);

    // ===================== Info Panel =====================
    m_infoPanel = new QWidget;
    QVBoxLayout *infoLayout = new QVBoxLayout(m_infoPanel);
    infoLayout->setContentsMargins(0, 10, 0, 10);
    infoLayout->setSpacing(8);
    infoLayout->setAlignment(Qt::AlignCenter);

    // Cover art placeholder
    m_coverLabel = new QLabel;
    m_coverLabel->setFixedSize(180, 180);
    m_coverLabel->setAlignment(Qt::AlignCenter);
    m_coverLabel->setStyleSheet(R"(
        QLabel {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #667eea, stop:1 #764ba2);
            border-radius: 12px;
            border: 2px solid #555;
        }
    )");
    m_coverLabel->setText("\u266B"); // Musical note character
    m_coverLabel->setStyleSheet(m_coverLabel->styleSheet() +
        " QLabel { color: white; font-size: 72px; }");

    // Track title
    m_trackTitleLabel = new QLabel("Aucune piste");
    m_trackTitleLabel->setAlignment(Qt::AlignCenter);
    m_trackTitleLabel->setWordWrap(true);
    m_trackTitleLabel->setStyleSheet(
        "font-size: 18px; font-weight: bold; color: #333; padding: 4px;");

    // Artist / Album
    m_trackArtistLabel = new QLabel("");
    m_trackArtistLabel->setAlignment(Qt::AlignCenter);
    m_trackArtistLabel->setStyleSheet(
        "font-size: 13px; color: #777; padding: 2px;");

    m_trackAlbumLabel = new QLabel("");
    m_trackAlbumLabel->setAlignment(Qt::AlignCenter);
    m_trackAlbumLabel->setStyleSheet(
        "font-size: 12px; color: #999; font-style: italic; padding: 2px;");

    infoLayout->addWidget(m_coverLabel, 0, Qt::AlignCenter);
    infoLayout->addWidget(m_trackTitleLabel);
    infoLayout->addWidget(m_trackArtistLabel);
    infoLayout->addWidget(m_trackAlbumLabel);

    mainLayout->addWidget(m_infoPanel, 0, Qt::AlignTop | Qt::AlignHCenter);

    // ===================== Seek Panel =====================
    m_seekPanel = new QWidget;
    QHBoxLayout *seekLayout = new QHBoxLayout(m_seekPanel);
    seekLayout->setContentsMargins(10, 5, 10, 5);
    seekLayout->setSpacing(8);

    m_currentTimeLabel = new QLabel("00:00");
    m_currentTimeLabel->setFixedWidth(45);
    m_currentTimeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_currentTimeLabel->setStyleSheet("font-size: 11px; color: #555; font-family: monospace;");

    m_seekSlider = new QSlider(Qt::Horizontal);
    m_seekSlider->setRange(0, 0);
    m_seekSlider->setPageStep(5000);
    m_seekSlider->setCursor(Qt::PointingHandCursor);
    m_seekSlider->setFixedHeight(22);
    m_seekSlider->setStyleSheet(R"(
        QSlider::groove:horizontal {
            background: #e0e0e0;
            height: 6px;
            border-radius: 3px;
        }
        QSlider::handle:horizontal {
            background: #667eea;
            width: 16px;
            height: 16px;
            margin: -5px 0;
            border-radius: 8px;
        }
        QSlider::handle:horizontal:hover {
            background: #5a6fd6;
        }
        QSlider::sub-page:horizontal {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #667eea, stop:1 #764ba2);
            border-radius: 3px;
        }
    )");

    m_totalTimeLabel = new QLabel("00:00");
    m_totalTimeLabel->setFixedWidth(45);
    m_totalTimeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_totalTimeLabel->setStyleSheet("font-size: 11px; color: #555; font-family: monospace;");

    seekLayout->addWidget(m_currentTimeLabel);
    seekLayout->addWidget(m_seekSlider, 1);
    seekLayout->addWidget(m_totalTimeLabel);

    mainLayout->addWidget(m_seekPanel);

    // ===================== Controls Panel =====================
    m_controlsPanel = new QWidget;
    QHBoxLayout *controlsLayout = new QHBoxLayout(m_controlsPanel);
    controlsLayout->setContentsMargins(20, 5, 20, 5);
    controlsLayout->setSpacing(15);

    // Shuffle button
    m_shuffleBtn = new QPushButton("\U0001F500"); // Shuffle icon
    m_shuffleBtn->setFixedSize(40, 40);
    m_shuffleBtn->setCursor(Qt::PointingHandCursor);
    m_shuffleBtn->setToolTip("Melanger (S)");
    m_shuffleBtn->setCheckable(true);
    m_shuffleBtn->setStyleSheet(R"(
        QPushButton {
            border: none;
            font-size: 18px;
            background: transparent;
            border-radius: 8px;
        }
        QPushButton:hover { background: #f0f0f0; }
        QPushButton:checked { background: #e0e0ff; color: #667eea; }
    )");

    // Previous button
    m_prevBtn = new QPushButton("\u23EE"); // Track previous
    m_prevBtn->setFixedSize(44, 44);
    m_prevBtn->setCursor(Qt::PointingHandCursor);
    m_prevBtn->setToolTip("Piste precedente");
    m_prevBtn->setStyleSheet(R"(
        QPushButton {
            border: none;
            font-size: 22px;
            background: transparent;
            border-radius: 22px;
        }
        QPushButton:hover { background: #f0f0f0; }
        QPushButton:pressed { background: #ddd; }
    )");

    // Play/Pause button
    m_playPauseBtn = new QPushButton("\u25B6"); // Play triangle
    m_playPauseBtn->setFixedSize(64, 64);
    m_playPauseBtn->setCursor(Qt::PointingHandCursor);
    m_playPauseBtn->setToolTip("Lecture/Pause (Espace)");
    m_playPauseBtn->setStyleSheet(R"(
        QPushButton {
            border: none;
            font-size: 28px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #667eea, stop:1 #764ba2);
            color: white;
            border-radius: 32px;
            padding: 0px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #5a6fd6, stop:1 #6a4196);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #4e63c2, stop:1 #5e379a);
        }
    )");

    // Next button
    m_nextBtn = new QPushButton("\u23ED"); // Track next
    m_nextBtn->setFixedSize(44, 44);
    m_nextBtn->setCursor(Qt::PointingHandCursor);
    m_nextBtn->setToolTip("Piste suivante");
    m_nextBtn->setStyleSheet(m_prevBtn->styleSheet());

    // Repeat button
    m_repeatBtn = new QPushButton("\U0001F501"); // Repeat icon
    m_repeatBtn->setFixedSize(40, 40);
    m_repeatBtn->setCursor(Qt::PointingHandCursor);
    m_repeatBtn->setToolTip("Repeter (R)");
    m_repeatBtn->setCheckable(true);
    m_repeatBtn->setStyleSheet(m_shuffleBtn->styleSheet());

    controlsLayout->addStretch();
    controlsLayout->addWidget(m_shuffleBtn);
    controlsLayout->addWidget(m_prevBtn);
    controlsLayout->addWidget(m_playPauseBtn);
    controlsLayout->addWidget(m_nextBtn);
    controlsLayout->addWidget(m_repeatBtn);
    controlsLayout->addStretch();

    mainLayout->addWidget(m_controlsPanel);

    // ===================== Volume Panel =====================
    m_volumePanel = new QWidget;
    QHBoxLayout *volumeLayout = new QHBoxLayout(m_volumePanel);
    volumeLayout->setContentsMargins(40, 5, 40, 10);
    volumeLayout->setSpacing(8);

    m_volumeBtn = new QPushButton("\U0001F50A"); // Speaker icon
    m_volumeBtn->setFixedSize(32, 32);
    m_volumeBtn->setCursor(Qt::PointingHandCursor);
    m_volumeBtn->setToolTip("Mute/Unmute (M)");
    m_volumeBtn->setStyleSheet(R"(
        QPushButton {
            border: none; font-size: 16px;
            background: transparent; border-radius: 6px;
        }
        QPushButton:hover { background: #f0f0f0; }
    )");

    m_volumeSlider = new QSlider(Qt::Horizontal);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(70);
    m_volumeSlider->setCursor(Qt::PointingHandCursor);
    m_volumeSlider->setFixedHeight(18);
    m_volumeSlider->setStyleSheet(R"(
        QSlider::groove:horizontal {
            background: #e0e0e0; height: 4px; border-radius: 2px;
        }
        QSlider::handle:horizontal {
            background: #667eea; width: 14px; height: 14px;
            margin: -5px 0; border-radius: 7px;
        }
        QSlider::handle:horizontal:hover { background: #5a6fd6; }
        QSlider::sub-page:horizontal {
            background: #667eea; border-radius: 2px;
        }
    )");

    m_volumeLabel = new QLabel("70%");
    m_volumeLabel->setFixedWidth(35);
    m_volumeLabel->setStyleSheet("font-size: 11px; color: #555;");

    volumeLayout->addWidget(m_volumeBtn);
    volumeLayout->addWidget(m_volumeSlider, 1);
    volumeLayout->addWidget(m_volumeLabel);

    mainLayout->addWidget(m_volumePanel);
    mainLayout->addStretch();
}

// ============================================================
// Connections
// ============================================================

void AudioPlayer::setupConnections()
{
    // Player signals
    connect(m_player, &QMediaPlayer::positionChanged,
            this, &AudioPlayer::onPositionChanged);
    connect(m_player, &QMediaPlayer::durationChanged,
            this, &AudioPlayer::onDurationChanged);
    connect(m_player, &QMediaPlayer::mediaStatusChanged,
            this, &AudioPlayer::onMediaStatusChanged);
    connect(m_player, &QMediaPlayer::errorOccurred,
            this, &AudioPlayer::onErrorOccurred);
    connect(m_player, &QMediaPlayer::playbackStateChanged,
            this, [this](QMediaPlayer::PlaybackState state) {
        bool playing = (state == QMediaPlayer::PlayingState);
        m_isPlaying = playing;
        m_playPauseBtn->setText(playing ? "\u23F8" : "\u25B6");
        emit playbackStateChanged(playing);
        emit statusMessage(playing ? "Lecture en cours..." : "En pause");
    });

    // Seek slider
    connect(m_seekSlider, &QSlider::sliderMoved,
            this, &AudioPlayer::onSeek);
    connect(m_seekSlider, &QSlider::sliderPressed, this, [this]() {
        m_player->setPosition(m_seekSlider->value());
    });

    // Volume
    connect(m_volumeSlider, &QSlider::valueChanged,
            this, &AudioPlayer::onVolumeChanged);
    connect(m_volumeBtn, &QPushButton::clicked, this, [this]() {
        if (m_audioOutput->volume() > 0) {
            m_audioOutput->setVolume(0);
            m_volumeBtn->setText("\U0001F507"); // Muted speaker
            m_volumeSlider->setValue(0);
        } else {
            m_audioOutput->setVolume(0.7);
            m_volumeBtn->setText("\U0001F50A");
            m_volumeSlider->setValue(70);
        }
    });

    // Control buttons
    connect(m_playPauseBtn, &QPushButton::clicked,
            this, &AudioPlayer::togglePlayPause);
    connect(m_prevBtn, &QPushButton::clicked,
            this, &AudioPlayer::previousTrack);
    connect(m_nextBtn, &QPushButton::clicked,
            this, &AudioPlayer::nextTrack);

    // Shuffle
    connect(m_shuffleBtn, &QPushButton::toggled, this, [this](bool checked) {
        m_shuffleEnabled = checked;
        emit statusMessage(checked ? "Melange active" : "Melange desactive");
    });

    // Repeat
    connect(m_repeatBtn, &QPushButton::clicked, this, [this]() {
        switch (m_repeatMode) {
        case RepeatMode::None:
            m_repeatMode = RepeatMode::All;
            m_repeatBtn->setChecked(true);
            m_repeatBtn->setText("\U0001F501"); // Repeat
            emit statusMessage("Repeter tout active");
            break;
        case RepeatMode::All:
            m_repeatMode = RepeatMode::One;
            m_repeatBtn->setText("\U0001F502"); // Repeat one
            emit statusMessage("Repeter un titre active");
            break;
        case RepeatMode::One:
            m_repeatMode = RepeatMode::None;
            m_repeatBtn->setChecked(false);
            m_repeatBtn->setText("\U0001F501");
            emit statusMessage("Repeter desactive");
            break;
        }
    });

    // Playlist widget
    connect(m_playlistWidget, &PlaylistWidget::trackSelected,
            this, &AudioPlayer::onPlaylistTrackSelected);
}

// ============================================================
// Public Methods
// ============================================================

void AudioPlayer::loadAndPlay(const QString &filePath)
{
    loadAudioFile(filePath);
}

void AudioPlayer::play()
{
    if (m_playlist.isEmpty() && m_currentTrackIndex < 0) {
        emit statusMessage("Aucune piste a lire");
        return;
    }
    m_player->play();
}

void AudioPlayer::pause()
{
    m_player->pause();
}

void AudioPlayer::stop()
{
    m_player->stop();
    m_isPlaying = false;
    m_playPauseBtn->setText("\u25B6");
    m_seekSlider->setValue(0);
    m_currentTimeLabel->setText("00:00");
}

void AudioPlayer::togglePlayPause()
{
    if (m_player->playbackState() == QMediaPlayer::PlayingState)
        m_player->pause();
    else
        m_player->play();
}

void AudioPlayer::nextTrack()
{
    if (m_playlist.isEmpty())
        return;

    if (m_currentTrackIndex < 0) {
        m_currentTrackIndex = 0;
    } else {
        if (m_shuffleEnabled) {
            // Random next track (different from current)
            if (m_playlist.size() > 1) {
                int next;
                do {
                    next = QRandomGenerator::global()->bounded(m_playlist.size());
                } while (next == m_currentTrackIndex);
                m_currentTrackIndex = next;
            }
        } else {
            m_currentTrackIndex++;
            if (m_currentTrackIndex >= m_playlist.size()) {
                if (m_repeatMode == RepeatMode::All) {
                    m_currentTrackIndex = 0;
                } else {
                    m_currentTrackIndex = m_playlist.size() - 1;
                    stop();
                    emit statusMessage("Fin de la playlist");
                    return;
                }
            }
        }
    }

    loadAudioFile(m_playlist.at(m_currentTrackIndex));
    m_playlistWidget->setCurrentIndex(m_currentTrackIndex);
}

void AudioPlayer::previousTrack()
{
    if (m_playlist.isEmpty())
        return;

    // If more than 3 seconds in, restart current track
    if (m_player->position() > 3000) {
        m_player->setPosition(0);
        return;
    }

    if (m_currentTrackIndex > 0) {
        m_currentTrackIndex--;
    } else if (m_repeatMode == RepeatMode::All) {
        m_currentTrackIndex = m_playlist.size() - 1;
    } else {
        m_currentTrackIndex = 0;
    }

    loadAudioFile(m_playlist.at(m_currentTrackIndex));
    m_playlistWidget->setCurrentIndex(m_currentTrackIndex);
}

void AudioPlayer::addToPlaylist(const QStringList &filePaths)
{
    for (const QString &path : filePaths) {
        if (!m_playlist.contains(path))
            m_playlist << path;
    }
    m_playlistWidget->setPlaylist(m_playlist);

    // Auto-play first if not already playing
    if (m_currentTrackIndex < 0 && !m_playlist.isEmpty()) {
        m_currentTrackIndex = 0;
        loadAudioFile(m_playlist.first());
        m_playlistWidget->setCurrentIndex(0);
    }

    emit statusMessage(QString("Playlist: %1 piste(s)").arg(m_playlist.size()));
}

void AudioPlayer::clearPlaylist()
{
    m_playlist.clear();
    m_currentTrackIndex = -1;
    m_playlistWidget->clear();
    stop();
    m_trackTitleLabel->setText("Aucune piste");
    m_trackArtistLabel->clear();
    m_trackAlbumLabel->clear();
}

PlaylistWidget *AudioPlayer::playlistWidget() const
{
    return m_playlistWidget;
}

void AudioPlayer::setVolume(int volume)
{
    m_volumeSlider->setValue(qBound(0, volume, 100));
}

int AudioPlayer::volume() const
{
    return m_volumeSlider->value();
}

bool AudioPlayer::isPlaying() const
{
    return m_isPlaying;
}

QString AudioPlayer::currentTrackName() const
{
    if (m_currentTrackIndex >= 0 && m_currentTrackIndex < m_playlist.size())
        return QFileInfo(m_playlist.at(m_currentTrackIndex)).fileName();
    return QString();
}

// ============================================================
// Private Slots
// ============================================================

void AudioPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    switch (status) {
    case QMediaPlayer::LoadedMedia:
        emit statusMessage("Piste chargee: " + currentTrackName());
        break;
    case QMediaPlayer::EndOfMedia:
        // Handle auto-advance based on repeat mode
        if (m_repeatMode == RepeatMode::One) {
            m_player->setPosition(0);
            m_player->play();
        } else {
            nextTrack();
        }
        break;
    case QMediaPlayer::InvalidMedia:
        emit statusMessage("Media invalide");
        break;
    default:
        break;
    }
}

void AudioPlayer::onPositionChanged(qint64 position)
{
    if (!m_seekSlider->isSliderDown()) {
        m_seekSlider->setValue(static_cast<int>(position));
    }
    m_currentTimeLabel->setText(formatTime(position));
}

void AudioPlayer::onDurationChanged(qint64 duration)
{
    m_seekSlider->setRange(0, static_cast<int>(duration));
    m_totalTimeLabel->setText(formatTime(duration));
}

void AudioPlayer::onErrorOccurred(QMediaPlayer::Error error, const QString &errorString)
{
    Q_UNUSED(error);
    QMessageBox::warning(this, "Erreur de lecture",
        "Impossible de lire le fichier audio :\n" + errorString);
    emit statusMessage("Erreur: " + errorString);
}

void AudioPlayer::onSeek(int position)
{
    m_player->setPosition(position);
}

void AudioPlayer::onVolumeChanged(int volume)
{
    m_audioOutput->setVolume(static_cast<float>(volume) / 100.0f);
    m_volumeLabel->setText(QString("%1%").arg(volume));

    if (volume == 0)
        m_volumeBtn->setText("\U0001F507");
    else if (volume < 50)
        m_volumeBtn->setText("\U0001F509");
    else
        m_volumeBtn->setText("\U0001F50A");
}

void AudioPlayer::onPlaylistTrackSelected(const QString &filePath)
{
    int index = m_playlist.indexOf(filePath);
    if (index >= 0) {
        m_currentTrackIndex = index;
        loadAudioFile(filePath);
    }
}

void AudioPlayer::updateTimeDisplay()
{
    if (m_player->playbackState() == QMediaPlayer::PlayingState) {
        m_currentTimeLabel->setText(formatTime(m_player->position()));
    }
}

// ============================================================
// Private Methods
// ============================================================

void AudioPlayer::loadAudioFile(const QString &filePath)
{
    m_player->setSource(QUrl::fromLocalFile(filePath));
    m_player->play();
    updateTrackInfo();
    emit trackChanged(filePath);
}

void AudioPlayer::updateTrackInfo()
{
    if (m_currentTrackIndex < 0 || m_currentTrackIndex >= m_playlist.size())
        return;

    const QFileInfo fi(m_playlist.at(m_currentTrackIndex));
    QString baseName = fi.completeBaseName();

    // Try to parse artist - title from filename
    // Common patterns: "Artist - Title", "Artist - Title (Year)"
    static QRegularExpression re("^(.+?)\\s*[-]\\s*(.+)$");
    QRegularExpressionMatch match = re.match(baseName);

    if (match.hasMatch()) {
        m_trackArtistLabel->setText(match.captured(1).trimmed());
        m_trackTitleLabel->setText(match.captured(2).trimmed());
    } else {
        m_trackTitleLabel->setText(baseName);
        m_trackArtistLabel->setText(fi.absolutePath().split("/").last());
    }

    m_trackAlbumLabel->setText(fi.suffix().toUpper());
}

QString AudioPlayer::formatTime(qint64 ms) const
{
    if (ms < 0) ms = 0;
    const int seconds = static_cast<int>(ms / 1000);
    const int minutes = seconds / 60;
    const int secs = seconds % 60;
    return QString("%1:%2")
        .arg(minutes)
        .arg(secs, 2, 10, QChar('0'));
}
