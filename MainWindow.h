#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QCloseEvent>

class ImageViewer;
class AudioPlayer;
class VideoPlayer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onTabChanged(int index);
    void onOpenFile();
    void onOpenFolder();
    void onExit();
    void onToggleFullScreen(bool checked);
    void onAbout();
    void showStatusMessage(const QString &message, int timeout = 3000);
    void showImageStatus(const QString &info);
    void showAudioStatus(const QString &info);
    void showVideoStatus(const QString &info);

private:
    void setupUi();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupConnections();

    QTabWidget *m_tabWidget;
    ImageViewer  *m_imageViewer;
    AudioPlayer  *m_audioPlayer;
    VideoPlayer  *m_videoPlayer;

    QMenu *m_fileMenu;
    QMenu *m_viewMenu;
    QMenu *m_helpMenu;
    QToolBar *m_toolBar;

    QAction *m_openFileAction;
    QAction *m_openFolderAction;
    QAction *m_exitAction;
    QAction *m_fullScreenAction;
    QAction *m_aboutAction;
    QAction *m_aboutQtAction;

    QLabel *m_statusLabel;
    QLabel *m_infoLabel;
    bool m_isFullScreen;
};

#endif // MAINWINDOW_H
