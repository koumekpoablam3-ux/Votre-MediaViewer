#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QWidget>
#include <QStringList>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>

class PlaylistWidget;

// ============================================================
// AudioPlayer - Complete audio player with controls,
//               seek bar, volume, and playlist integration.
// ============================================================
class AudioPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit AudioPlayer(QWidget *parent = nullptr);

    // Playback control
    void loadAndPlay(const QString &filePath);
    void play();
    void pause();
    void stop();
    void togglePlayPause();
    void nextTrack();
    void previousTrack();

    // Playlist
    void addToPlaylist(const QStringList &filePaths);
    void clearPlaylist();
    PlaylistWidget *playlistWidget() const;

    // Volume
    void setVolume(int volume);
    int volume() const;

    // State
    bool isPlaying() const;
    QString currentTrackName() const;

signals:
    void statusMessage(const QString &info);
    void trackChanged(const QString &filePath);
    void playbackStateChanged(bool playing);

private slots:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onErrorOccurred(QMediaPlayer::Error error, const QString &errorString);
    void onSeek(int position);
    void onVolumeChanged(int volume);
    void onPlaylistTrackSelected(const QString &filePath);
    void updateTimeDisplay();

private:
    void setupUi();
    void setupConnections();
    void loadAudioFile(const QString &filePath);
    QString formatTime(qint64 ms) const;
    void updateTrackInfo();

    // Qt Multimedia
    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;

    // UI - Info area
    QWidget *m_infoPanel;
    QLabel *m_coverLabel;
    QLabel *m_trackTitleLabel;
    QLabel *m_trackArtistLabel;
    QLabel *m_trackAlbumLabel;

    // UI - Seek bar
    QWidget *m_seekPanel;
    QSlider *m_seekSlider;
    QLabel *m_currentTimeLabel;
    QLabel *m_totalTimeLabel;

    // UI - Controls
    QWidget *m_controlsPanel;
    QPushButton *m_shuffleBtn;
    QPushButton *m_prevBtn;
    QPushButton *m_playPauseBtn;
    QPushButton *m_nextBtn;
    QPushButton *m_repeatBtn;

    // UI - Volume
    QWidget *m_volumePanel;
    QPushButton *m_volumeBtn;
    QSlider *m_volumeSlider;
    QLabel *m_volumeLabel;

    // Playlist
    PlaylistWidget *m_playlistWidget;

    // State
    bool m_isPlaying;
    bool m_shuffleEnabled;
    enum class RepeatMode { None, One, All } m_repeatMode;
    QStringList m_playlist;
    int m_currentTrackIndex;
};

#endif // AUDIOPLAYER_H
