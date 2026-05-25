#include "VideoPlayer.h"

#include <QApplication>
#include <QStyle>
#include <QMessageBox>
#include <QFileInfo>
#include <QKeyEvent>
#include <QDebug>

// ============================================================
// Constructor
// ============================================================

VideoPlayer::VideoPlayer(QWidget *parent)
    : QWidget(parent)
    , m_isPlaying(false)
    , m_isFullscreen(false)
{
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(0.7);

    setupUi();
    setupConnections();
}

// ============================================================
// UI Setup
// ============================================================

void VideoPlayer::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(8);

    // Video output
    m_videoWidget = new QVideoWidget;
    m_videoWidget->setMinimumSize(400, 300);
    m_videoWidget->setStyleSheet("background: #1a1a1a; border-radius: 6px;");
    mainLayout->addWidget(m_videoWidget, 1);

    m_player->setVideoOutput(m_videoWidget);

    // Seek bar
    QHBoxLayout *seekLayout = new QHBoxLayout;
    seekLayout->setSpacing(8);

    m_currentTimeLabel = new QLabel("00:00");
    m_currentTimeLabel->setFixedWidth(50);
    m_currentTimeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_currentTimeLabel->setStyleSheet("font-size: 11px; color: #555; font-family: monospace;");

    m_seekSlider = new QSlider(Qt::Horizontal);
    m_seekSlider->setRange(0, 0);
    m_seekSlider->setPageStep(5000);
    m_seekSlider->setCursor(Qt::PointingHandCursor);
    m_seekSlider->setFixedHeight(22);
    m_seekSlider->setStyleSheet(R"(
        QSlider::groove:horizontal {
            background: #e0e0e0; height: 6px; border-radius: 3px;
        }
        QSlider::handle:horizontal {
            background: #667eea; width: 16px; height: 16px;
            margin: -5px 0; border-radius: 8px;
        }
        QSlider::handle:horizontal:hover { background: #5a6fd6; }
        QSlider::sub-page:horizontal {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #667eea, stop:1 #764ba2);
            border-radius: 3px;
        }
    )");

    m_totalTimeLabel = new QLabel("00:00");
    m_totalTimeLabel->setFixedWidth(50);
    m_totalTimeLabel->setStyleSheet("font-size: 11px; color: #555; font-family: monospace;");

    seekLayout->addWidget(m_currentTimeLabel);
    seekLayout->addWidget(m_seekSlider, 1);
    seekLayout->addWidget(m_totalTimeLabel);
    mainLayout->addLayout(seekLayout);

    // Controls
    QHBoxLayout *controlsLayout = new QHBoxLayout;
    controlsLayout->setSpacing(10);

    m_playPauseBtn = new QPushButton("\u25B6");
    m_playPauseBtn->setFixedSize(44, 44);
    m_playPauseBtn->setCursor(Qt::PointingHandCursor);
    m_playPauseBtn->setToolTip("Lecture/Pause (Espace)");
    m_playPauseBtn->setStyleSheet(R"(
        QPushButton {
            border: none; font-size: 22px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #667eea, stop:1 #764ba2);
            color: white; border-radius: 22px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #5a6fd6, stop:1 #6a4196);
        }
    )");
    controlsLayout->addWidget(m_playPauseBtn);

    m_stopBtn = new QPushButton("\u23F9");
    m_stopBtn->setFixedSize(38, 38);
    m_stopBtn->setCursor(Qt::PointingHandCursor);
    m_stopBtn->setToolTip("Stop");
    m_stopBtn->setStyleSheet(R"(
        QPushButton {
            border: none; font-size: 18px;
            background: #f0f0f0; border-radius: 19px;
        }
        QPushButton:hover { background: #e0e0e0; }
        QPushButton:pressed { background: #d0d0d0; }
    )");
    controlsLayout->addWidget(m_stopBtn);

    controlsLayout->addStretch();

    m_volumeBtn = new QPushButton("\U0001F50A");
    m_volumeBtn->setFixedSize(30, 30);
    m_volumeBtn->setCursor(Qt::PointingHandCursor);
    m_volumeBtn->setStyleSheet("border: none; font-size: 14px; background: transparent;");
    controlsLayout->addWidget(m_volumeBtn);

    m_volumeSlider = new QSlider(Qt::Horizontal);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(70);
    m_volumeSlider->setCursor(Qt::PointingHandCursor);
    m_volumeSlider->setFixedWidth(100);
    m_volumeSlider->setFixedHeight(16);
    m_volumeSlider->setStyleSheet(R"(
        QSlider::groove:horizontal { background: #e0e0e0; height: 4px; border-radius: 2px; }
        QSlider::handle:horizontal { background: #667eea; width: 12px; height: 12px; margin: -4px 0; border-radius: 6px; }
        QSlider::sub-page:horizontal { background: #667eea; border-radius: 2px; }
    )");
    controlsLayout->addWidget(m_volumeSlider);

    m_volumeLabel = new QLabel("70%");
    m_volumeLabel->setFixedWidth(32);
    m_volumeLabel->setStyleSheet("font-size: 10px; color: #777;");
    controlsLayout->addWidget(m_volumeLabel);

    controlsLayout->addStretch();

    m_fullscreenBtn = new QPushButton("\u26F6");
    m_fullscreenBtn->setFixedSize(36, 36);
    m_fullscreenBtn->setCursor(Qt::PointingHandCursor);
    m_fullscreenBtn->setToolTip("Plein ecran (F)");
    m_fullscreenBtn->setStyleSheet(R"(
        QPushButton {
            border: none; font-size: 16px;
            background: #f0f0f0; border-radius: 18px;
        }
        QPushButton:hover { background: #e0e0e0; }
    )");
    controlsLayout->addWidget(m_fullscreenBtn);

    mainLayout->addLayout(controlsLayout);

    // Status bar
    m_statusLabel = new QLabel("Aucune video chargee");
    m_statusLabel->setStyleSheet("font-size: 11px; color: #999; padding: 4px;");
    m_statusLabel->setWordWrap(true);
    mainLayout->addWidget(m_statusLabel);
}

// ============================================================
// Connections
// ============================================================

void VideoPlayer::setupConnections()
{
    connect(m_player, &QMediaPlayer::positionChanged,
            this, &VideoPlayer::onPositionChanged);
    connect(m_player, &QMediaPlayer::durationChanged,
            this, &VideoPlayer::onDurationChanged);
    connect(m_player, &QMediaPlayer::mediaStatusChanged,
            this, &VideoPlayer::onMediaStatusChanged);
    connect(m_player, &QMediaPlayer::errorOccurred,
            this, &VideoPlayer::onErrorOccurred);
    connect(m_player, &QMediaPlayer::playbackStateChanged,
            this, [this](QMediaPlayer::PlaybackState state) {
        m_isPlaying = (state == QMediaPlayer::PlayingState);
        m_playPauseBtn->setText(m_isPlaying ? "\u23F8" : "\u25B6");
        emit statusMessage(m_isPlaying ? "Lecture video..." : "En pause");
    });

    connect(m_seekSlider, &QSlider::sliderMoved,
            this, &VideoPlayer::onSeek);
    connect(m_seekSlider, &QSlider::sliderPressed, this, [this]() {
        m_player->setPosition(m_seekSlider->value());
    });

    connect(m_playPauseBtn, &QPushButton::clicked,
            this, &VideoPlayer::togglePlayPause);
    connect(m_stopBtn, &QPushButton::clicked, this, [this]() {
        stop();
    });

    connect(m_volumeSlider, &QSlider::valueChanged,
            this, &VideoPlayer::onVolumeChanged);
    connect(m_volumeBtn, &QPushButton::clicked, this, [this]() {
        if (m_audioOutput->volume() > 0) {
            m_audioOutput->setVolume(0);
            m_volumeBtn->setText("\U0001F507");
            m_volumeSlider->setValue(0);
        } else {
            m_audioOutput->setVolume(0.7);
            m_volumeBtn->setText("\U0001F50A");
            m_volumeSlider->setValue(70);
        }
    });

    connect(m_fullscreenBtn, &QPushButton::clicked,
            this, &VideoPlayer::toggleFullscreen);
}

// ============================================================
// Public Methods
// ============================================================

void VideoPlayer::loadAndPlay(const QString &filePath)
{
    m_player->setSource(QUrl::fromLocalFile(filePath));
    m_player->play();

    QFileInfo fi(filePath);
    m_statusLabel->setText("Lecture : " + fi.fileName());
    emit statusMessage("Video chargee : " + fi.fileName());
}

void VideoPlayer::play()
{
    m_player->play();
}

void VideoPlayer::pause()
{
    m_player->pause();
}

void VideoPlayer::stop()
{
    m_player->stop();
    m_isPlaying = false;
    m_playPauseBtn->setText("\u25B6");
    m_seekSlider->setValue(0);
    m_currentTimeLabel->setText("00:00");
    m_statusLabel->setText("Arrete");
}

void VideoPlayer::togglePlayPause()
{
    if (m_player->playbackState() == QMediaPlayer::PlayingState)
        m_player->pause();
    else
        m_player->play();
}

void VideoPlayer::setVolume(int volume)
{
    m_volumeSlider->setValue(qBound(0, volume, 100));
}

int VideoPlayer::volume() const
{
    return m_volumeSlider->value();
}

bool VideoPlayer::isPlaying() const
{
    return m_isPlaying;
}

QString VideoPlayer::currentTrackName() const
{
    return QString();
}

void VideoPlayer::toggleFullscreen()
{
    if (m_isFullscreen) {
        m_videoWidget->setFullScreen(false);
        m_isFullscreen = false;
        m_fullscreenBtn->setText("\u26F6");
    } else {
        m_videoWidget->setFullScreen(true);
        m_isFullscreen = true;
        m_fullscreenBtn->setText("\u26F6");
    }
}

// ============================================================
// Private Slots
// ============================================================

void VideoPlayer::onPositionChanged(qint64 position)
{
    if (!m_seekSlider->isSliderDown())
        m_seekSlider->setValue(static_cast<int>(position));
    m_currentTimeLabel->setText(formatTime(position));
}

void VideoPlayer::onDurationChanged(qint64 duration)
{
    m_seekSlider->setRange(0, static_cast<int>(duration));
    m_totalTimeLabel->setText(formatTime(duration));
}

void VideoPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::EndOfMedia) {
        m_statusLabel->setText("Video terminee");
        emit statusMessage("Video terminee");
    }
}

void VideoPlayer::onErrorOccurred(QMediaPlayer::Error error, const QString &errorString)
{
    Q_UNUSED(error);
    m_statusLabel->setText("Erreur : " + errorString);
    QMessageBox::warning(this, "Erreur video",
        "Impossible de lire la video :\n" + errorString);
}

void VideoPlayer::onSeek(int position)
{
    m_player->setPosition(position);
}

void VideoPlayer::onVolumeChanged(int volume)
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

QString VideoPlayer::formatTime(qint64 ms) const
{
    if (ms < 0) ms = 0;
    const int totalSeconds = static_cast<int>(ms / 1000);
    const int hours = totalSeconds / 3600;
    const int minutes = (totalSeconds % 3600) / 60;
    const int secs = totalSeconds % 60;

    if (hours > 0)
        return QString("%1:%2:%3")
            .arg(hours)
            .arg(minutes, 2, 10, QChar('0'))
            .arg(secs, 2, 10, QChar('0'));
    return QString("%1:%2")
        .arg(minutes)
        .arg(secs, 2, 10, QChar('0'));
}
