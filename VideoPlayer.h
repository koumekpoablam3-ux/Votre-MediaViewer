#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QWidget>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoWidget>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStringList>

// ============================================================
// VideoPlayer - Complete video player with controls,
//               seek bar, volume, and fullscreen support.
// ============================================================
class VideoPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit VideoPlayer(QWidget *parent = nullptr);

    // Playback
    void loadAndPlay(const QString &filePath);
    void play();
    void pause();
    void stop();
    void togglePlayPause();

    // Volume
    void setVolume(int volume);
    int volume() const;

    // State
    bool isPlaying() const;
    QString currentTrackName() const;

    // Fullscreen
    void toggleFullscreen();

signals:
    void statusMessage(const QString &info);

private slots:
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onErrorOccurred(QMediaPlayer::Error error, const QString &errorString);
    void onSeek(int position);
    void onVolumeChanged(int volume);

private:
    void setupUi();
    void setupConnections();
    QString formatTime(qint64 ms) const;

    // Qt Multimedia
    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    QVideoWidget *m_videoWidget;

    // Controls
    QSlider *m_seekSlider;
    QLabel *m_currentTimeLabel;
    QLabel *m_totalTimeLabel;
    QPushButton *m_playPauseBtn;
    QPushButton *m_stopBtn;
    QPushButton *m_fullscreenBtn;
    QPushButton *m_volumeBtn;
    QSlider *m_volumeSlider;
    QLabel *m_volumeLabel;
    QLabel *m_statusLabel;

    // State
    bool m_isPlaying;
    bool m_isFullscreen;
};

#endif // VIDEOPLAYER_H
