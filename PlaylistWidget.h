#ifndef PLAYLISTWIDGET_H
#define PLAYLISTWIDGET_H

#include <QWidget>
#include <QStringList>
#include <QListWidget>

// ============================================================
// PlaylistWidget - Custom list widget for audio playlist
//                with drag-and-drop, numbering, and styling.
// ============================================================
class PlaylistWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlaylistWidget(QWidget *parent = nullptr);

    void setPlaylist(const QStringList &filePaths);
    void clear();
    void addTrack(const QString &filePath);
    void removeTrack(int index);
    void setCurrentIndex(int index);
    int currentIndex() const;
    int trackCount() const;

signals:
    void trackSelected(const QString &filePath);
    void trackRemoved(int index);

private slots:
    void onItemDoubleClicked(QListWidgetItem *item);
    void onCustomContextMenu(const QPoint &pos);
    void onRemoveSelected();
    void onClearAll();
    void onMoveUp();
    void onMoveDown();

private:
    void setupUi();
    void updateRowNumbers();
    QString formatTrackName(const QString &filePath) const;

    QListWidget *m_listWidget;
    QStringList m_playlist;
    int m_currentIndex;
};

#endif // PLAYLISTWIDGET_H
