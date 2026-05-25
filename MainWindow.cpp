#include "MainWindow.h"
#include "ImageViewer.h"
#include "AudioPlayer.h"
#include "VideoPlayer.h"
#include "PlaylistWidget.h"

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QSplitter>
#include <QVBoxLayout>
#include <QScreen>
#include <QStyle>
#include <QGuiApplication>

// ============================================================
// Constructor / Destructor
// ============================================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_tabWidget(new QTabWidget(this))
    , m_imageViewer(new ImageViewer(this))
    , m_audioPlayer(new AudioPlayer(this))
    , m_videoPlayer(new VideoPlayer(this))
    , m_isFullScreen(false)
{
    setupUi();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupConnections();

    setWindowTitle("MediaViewer - Images / Audio / Video");
    setMinimumSize(900, 600);

    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        const QRect geo = screen->availableGeometry();
        resize(geo.width() * 0.75, geo.height() * 0.75);
        move((geo.width() - width()) / 2, (geo.height() - height()) / 2);
    }
    setWindowTitle("MediaViewer par RODRIGUE - Images / Audio / Video");
}
MainWindow::~MainWindow()
{
}
// ============================================================
// UI Setup
// ============================================================

void MainWindow::setupUi()
{
    m_tabWidget->setDocumentMode(true);
    m_tabWidget->setTabsClosable(false);
    m_tabWidget->setMovable(false);
    m_tabWidget->setElideMode(Qt::ElideRight);

    // --- Image tab ---
    QWidget *imageTab = new QWidget;
    QVBoxLayout *imageLayout = new QVBoxLayout(imageTab);
    imageLayout->setContentsMargins(0, 0, 0, 0);
    imageLayout->addWidget(m_imageViewer);
    m_tabWidget->addTab(imageTab, "   Images");

    // --- Audio tab with playlist ---
    QWidget *audioTab = new QWidget;
    QSplitter *audioSplitter = new QSplitter(Qt::Horizontal, audioTab);

    QWidget *playerContainer = new QWidget;
    QVBoxLayout *playerLayout = new QVBoxLayout(playerContainer);
    playerLayout->setContentsMargins(4, 4, 4, 4);
    playerLayout->addWidget(m_audioPlayer);

    PlaylistWidget *playlistWidget = m_audioPlayer->playlistWidget();

    audioSplitter->addWidget(playerContainer);
    audioSplitter->addWidget(playlistWidget);
    audioSplitter->setStretchFactor(0, 3);
    audioSplitter->setStretchFactor(1, 1);
    audioSplitter->setHandleWidth(3);

    QVBoxLayout *audioLayout = new QVBoxLayout(audioTab);
    audioLayout->setContentsMargins(0, 0, 0, 0);
    audioLayout->addWidget(audioSplitter);

    m_tabWidget->addTab(audioTab, "   Audio");

    // --- Video tab ---
    m_tabWidget->addTab(m_videoPlayer, "   Video");

    // Style
    m_tabWidget->setStyleSheet(R"(
        QTabWidget::pane { border: 1px solid #c0c0c0; background: #ffffff; }
        QTabBar::tab {
            background: #f0f0f0; border: 1px solid #c0c0c0;
            padding: 8px 20px; margin-right: 2px;
            font-size: 13px; min-width: 80px;
        }
        QTabBar::tab:selected {
            background: #ffffff; border-bottom-color: #ffffff; font-weight: bold;
        }
        QTabBar::tab:hover { background: #e8e8e8; }
    )");

    setCentralWidget(m_tabWidget);
}

void MainWindow::setupMenuBar()
{
    // File
    m_fileMenu = menuBar()->addMenu("&Fichier");
    m_openFileAction = new QAction("&Ouvrir un fichier...", this);
    m_openFileAction->setShortcut(QKeySequence::Open);
    m_fileMenu->addAction(m_openFileAction);

    m_openFolderAction = new QAction("Ouvrir un &dossier...", this);
    m_openFolderAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O));
    m_fileMenu->addAction(m_openFolderAction);
    m_fileMenu->addSeparator();

    m_exitAction = new QAction("&Quitter", this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    m_fileMenu->addAction(m_exitAction);

    // View
    m_viewMenu = menuBar()->addMenu("&Affichage");
    m_fullScreenAction = new QAction("Mode plein ecran", this);
    m_fullScreenAction->setCheckable(true);
    m_fullScreenAction->setShortcut(QKeySequence::FullScreen);
    m_viewMenu->addAction(m_fullScreenAction);

    // Help
    m_helpMenu = menuBar()->addMenu("&Aide");
    m_aboutAction = new QAction("&A propos de MediaViewer", this);
    m_helpMenu->addAction(m_aboutAction);
    m_aboutQtAction = new QAction("A propos de &Qt", this);
    m_helpMenu->addAction(m_aboutQtAction);
}

void MainWindow::setupToolBar()
{
    m_toolBar = addToolBar("Principal");
    m_toolBar->setMovable(false);
    m_toolBar->setIconSize(QSize(20, 20));
    m_toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_toolBar->addAction(m_openFileAction);
    m_toolBar->addAction(m_openFolderAction);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_fullScreenAction);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel("Pret");
    m_statusLabel->setMinimumWidth(200);
    statusBar()->addWidget(m_statusLabel, 1);

    m_infoLabel = new QLabel("");
    m_infoLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_infoLabel->setMinimumWidth(250);
    statusBar()->addPermanentWidget(m_infoLabel);
}

void MainWindow::setupConnections()
{
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    connect(m_openFileAction, &QAction::triggered, this, &MainWindow::onOpenFile);
    connect(m_openFolderAction, &QAction::triggered, this, &MainWindow::onOpenFolder);
    connect(m_exitAction, &QAction::triggered, this, &MainWindow::onExit);
    connect(m_fullScreenAction, &QAction::toggled, this, &MainWindow::onToggleFullScreen);
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
    connect(m_aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);

    connect(m_imageViewer, &ImageViewer::statusMessage, this, &MainWindow::showImageStatus);
    connect(m_audioPlayer, &AudioPlayer::statusMessage, this, &MainWindow::showAudioStatus);
    connect(m_videoPlayer, &VideoPlayer::statusMessage, this, &MainWindow::showVideoStatus);
}

// ============================================================
// Slots
// ============================================================

void MainWindow::onTabChanged(int index)
{
    if (index == 0) showStatusMessage("Mode Visionneuse d'Images");
    else if (index == 1) showStatusMessage("Mode Lecteur Audio");
    else if (index == 2) showStatusMessage("Mode Lecteur Video");
}

void MainWindow::onOpenFile()
{
    static const QString imageFilter = "Images (*.png *.jpg *.jpeg *.bmp *.gif *.tiff *.tif *.webp *.svg *.ico *.cur)";
    static const QString audioFilter = "Audio (*.mp3 *.wav *.flac *.ogg *.wma *.aac *.m4a *.opus *.aiff)";
    static const QString videoFilter = "Video (*.mp4 *.avi *.mkv *.wmv *.mov *.flv *.webm)";

    QString filter = imageFilter + ";;" + audioFilter + ";;" + videoFilter + ";;Tous les fichiers (*)";

    const QString filePath = QFileDialog::getOpenFileName(this, "Ouvrir un fichier", QString(), filter);
    if (filePath.isEmpty()) return;

    const QFileInfo fi(filePath);
    const QString suffix = fi.suffix().toLower();

    static const QStringList imageExts = {"png","jpg","jpeg","bmp","gif","tiff","tif","webp","svg","ico","cur"};
    static const QStringList audioExts = {"mp3","wav","flac","ogg","wma","aac","m4a","opus","aiff"};
    static const QStringList videoExts = {"mp4","avi","mkv","wmv","mov","flv","webm"};

    if (imageExts.contains(suffix)) {
        m_imageViewer->loadImage(filePath);
        m_tabWidget->setCurrentIndex(0);
    } else if (audioExts.contains(suffix)) {
        m_audioPlayer->loadAndPlay(filePath);
        m_tabWidget->setCurrentIndex(1);
    } else if (videoExts.contains(suffix)) {
        m_videoPlayer->loadAndPlay(filePath);
        m_tabWidget->setCurrentIndex(2);
    } else {
        QMessageBox::warning(this, "Format non supporte",
            "Le format '" + suffix + "' n'est pas reconnu.");
    }
}

void MainWindow::onOpenFolder()
{
    const QString dirPath = QFileDialog::getExistingDirectory(this, "Ouvrir un dossier", QString());
    if (dirPath.isEmpty()) return;

    static const QStringList imageExts = {"png","jpg","jpeg","bmp","gif","tiff","tif","webp","svg"};
    static const QStringList audioExts = {"mp3","wav","flac","ogg","wma","aac","m4a","opus","aiff"};
    static const QStringList videoExts = {"mp4","avi","mkv","wmv","mov","flv","webm"};

    QStringList imageFiles, audioFiles, videoFiles;

    const QDir dir(dirPath);
    const QFileInfoList entries = dir.entryInfoList(QDir::Files, QDir::Name);

    for (const QFileInfo &f : entries) {
        const QString ext = f.suffix().toLower();
        if (imageExts.contains(ext)) imageFiles << f.absoluteFilePath();
        else if (audioExts.contains(ext)) audioFiles << f.absoluteFilePath();
        else if (videoExts.contains(ext)) videoFiles << f.absoluteFilePath();
    }

    if (imageFiles.isEmpty() && audioFiles.isEmpty() && videoFiles.isEmpty()) {
        QMessageBox::information(this, "Dossier vide", "Aucun fichier media trouve.");
        return;
    }

    if (!imageFiles.isEmpty()) { m_imageViewer->loadImages(imageFiles); }
    if (!audioFiles.isEmpty()) { m_audioPlayer->addToPlaylist(audioFiles); }
    if (!videoFiles.isEmpty()) {
        if (imageFiles.isEmpty() && audioFiles.isEmpty()) {
            m_videoPlayer->loadAndPlay(videoFiles.first());
        }
    }

    int total = imageFiles.size() + audioFiles.size() + videoFiles.size();
    showStatusMessage(QString("Charge : %1 fichier(s)").arg(total));
}

void MainWindow::onExit() { close(); }

void MainWindow::onToggleFullScreen(bool checked)
{
    m_isFullScreen = checked;
    if (checked) showFullScreen(); else showNormal();
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "A propos de MediaViewer",
        "<h2>MediaViewer v1.1.0</h2>"
        "<p>Visionneuse d'images, lecteur audio et video</p>"
        "<p>Developpe avec C++ et Qt6</p>"
        "<p><b>Formats images :</b> PNG, JPEG, BMP, GIF, TIFF, WebP, SVG</p>"
        "<p><b>Formats audio :</b> MP3, WAV, FLAC, OGG, AAC, M4A</p>"
        "<p><b>Formats video :</b> MP4, AVI, MKV, WMV, MOV, WebM</p>");
}

void MainWindow::showStatusMessage(const QString &message, int timeout)
{
    m_statusLabel->setText(message);
    if (timeout > 0) statusBar()->showMessage(message, timeout);
}

void MainWindow::showImageStatus(const QString &info) { m_infoLabel->setText(info); }
void MainWindow::showAudioStatus(const QString &info) { m_infoLabel->setText(info); }
void MainWindow::showVideoStatus(const QString &info) { m_infoLabel->setText(info); }

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_audioPlayer->stop();
    m_videoPlayer->stop();
    event->accept();
}
