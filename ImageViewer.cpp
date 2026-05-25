#include "ImageViewer.h"

#include <QApplication>
#include <QToolBar>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPaintEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QStyle>
#include <QAction>
#include <QLabel>
#include <QMovie>
#include <QTimer>
#include <QScreen>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDebug>
#include <QFileInfo>
#include <QtMath>
#include <algorithm>

// ============================================================
// ImageLabel Implementation
// ============================================================

ImageLabel::ImageLabel(QWidget *parent)
    : QWidget(parent)
    , m_zoom(1.0)
    , m_zoomMode(ZoomMode::FitToWindow)
    , m_rotation(0)
    , m_flippedH(false)
    , m_flippedV(false)
    , m_panOffset(0, 0)
    , m_isPanning(false)
    , m_leftButtonPressed(false)
    , m_slideshowTimer(new QTimer(this))
{
    setMinimumSize(200, 150);
    setMouseTracking(true);
    setCursor(Qt::OpenHandCursor);
    setAcceptDrops(true);
    setStyleSheet("background-color: #2b2b2b;");

    m_slideshowTimer->setSingleShot(false);
    connect(m_slideshowTimer, &QTimer::timeout, this, [this]() {
        emit statusMessage("Diaporama - Image suivante...");
        // The parent ImageViewer will handle next image
        Q_EMIT static_cast<ImageViewer*>(parentWidget())->nextImage();
    });
}

void ImageLabel::setImage(const QPixmap &pixmap)
{
    m_originalPixmap = pixmap;
    m_rotation = 0;
    m_flippedH = false;
    m_flippedV = false;
    m_panOffset = QPointF(0, 0);

    if (m_zoomMode == ZoomMode::FitToWindow)
        fitToWindow();
    else
        updateTransform();

    update();
}

void ImageLabel::clearImage()
{
    m_originalPixmap = QPixmap();
    m_transformedPixmap = QPixmap();
    m_rotation = 0;
    m_zoom = 1.0;
    m_panOffset = QPointF(0, 0);
    update();
}

bool ImageLabel::hasImage() const
{
    return !m_originalPixmap.isNull();
}

void ImageLabel::zoomIn(double factor)
{
    setZoom(m_zoom * factor);
}

void ImageLabel::zoomOut(double factor)
{
    setZoom(m_zoom * factor);
}

void ImageLabel::setZoom(double scale)
{
    m_zoom = qBound(0.05, scale, 50.0);
    m_zoomMode = ZoomMode::FreeZoom;

    // Center the image on zoom
    if (!m_originalPixmap.isNull()) {
        QSize scaledSize = m_originalPixmap.size() * m_zoom;
        m_panOffset = QPointF(
            (width() - scaledSize.width()) / 2.0,
            (height() - scaledSize.height()) / 2.0
        );
    }

    updateTransform();
    update();
    emit zoomChanged(m_zoom);
}

void ImageLabel::fitToWindow()
{
    if (m_originalPixmap.isNull())
        return;

    m_zoomMode = ZoomMode::FitToWindow;
    m_panOffset = QPointF(0, 0);

    const double scaleX = static_cast<double>(width()) / m_originalPixmap.width();
    const double scaleY = static_cast<double>(height()) / m_originalPixmap.height();
    m_zoom = qMin(scaleX, scaleY) * 0.95; // 95% to leave some margin

    updateTransform();
    update();
    emit zoomChanged(m_zoom);
}

void ImageLabel::actualSize()
{
    m_zoom = 1.0;
    m_zoomMode = ZoomMode::ActualSize;
    m_panOffset = QPointF(
        (width() - m_originalPixmap.width()) / 2.0,
        (height() - m_originalPixmap.height()) / 2.0
    );

    updateTransform();
    update();
    emit zoomChanged(m_zoom);
}

void ImageLabel::rotateLeft()
{
    m_rotation = (m_rotation - 90) % 360;
    updateTransform();
    update();
    emit rotationChanged(m_rotation);
}

void ImageLabel::rotateRight()
{
    m_rotation = (m_rotation + 90) % 360;
    updateTransform();
    update();
    emit rotationChanged(m_rotation);
}

void ImageLabel::flipHorizontal()
{
    m_flippedH = !m_flippedH;
    updateTransform();
    update();
}

void ImageLabel::flipVertical()
{
    m_flippedV = !m_flippedV;
    updateTransform();
    update();
}

void ImageLabel::resetTransform()
{
    m_rotation = 0;
    m_flippedH = false;
    m_flippedV = false;
    m_panOffset = QPointF(0, 0);
    fitToWindow();
}

void ImageLabel::updateTransform()
{
    if (m_originalPixmap.isNull())
        return;

    QPixmap px = m_originalPixmap;

    // Apply rotation
    if (m_rotation != 0) {
        QTransform rot;
        rot.rotate(m_rotation);
        px = px.transformed(rot, Qt::SmoothTransformation);
    }

    // Apply flips
    if (m_flippedH || m_flippedV) {
        QTransform flip;
        if (m_flippedH) flip.scale(-1, 1);
        if (m_flippedV) flip.scale(1, -1);
        px = px.transformed(flip);
    }

    m_transformedPixmap = px;
}

bool ImageLabel::isSlideshowRunning() const
{
    return m_slideshowTimer->isActive();
}

void ImageLabel::startSlideshow(int intervalMs)
{
    m_slideshowTimer->start(intervalMs);
    emit statusMessage("Diaporama demarre (intervalle: " +
        QString::number(intervalMs / 1000) + "s)");
}

void ImageLabel::stopSlideshow()
{
    m_slideshowTimer->stop();
    emit statusMessage("Diaporama arrete");
}

// === Events ===

void ImageLabel::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Background
    painter.fillRect(rect(), QColor(43, 43, 43));

    if (m_transformedPixmap.isNull()) {
        // Draw placeholder
        painter.setPen(QColor(150, 150, 150));
        QFont font = painter.font();
        font.setPointSize(14);
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter,
            "Deposez une image ici\nou utilisez Ouvrir (Ctrl+O)");
        return;
    }

    // Apply zoom and pan
    QSize drawnSize = m_transformedPixmap.size() * m_zoom;

    // Calculate position
    QPointF pos = m_panOffset;

    // If FitToWindow, center the image
    if (m_zoomMode == ZoomMode::FitToWindow) {
        pos = QPointF(
            (width() - drawnSize.width()) / 2.0,
            (height() - drawnSize.height()) / 2.0
        );
    }

    // Draw image
    painter.drawPixmap(QRectF(pos, drawnSize), m_transformedPixmap,
                        m_transformedPixmap.rect());

    // Draw checkerboard pattern for transparency (small indicator)
    if (!m_originalPixmap.isNull() && m_originalPixmap.hasAlphaChannel()) {
        // Small indicator in bottom-right corner
        painter.setPen(QColor(200, 200, 200, 150));
        QFont f = painter.font();
        f.setPointSize(8);
        painter.setFont(f);
        painter.drawText(rect().adjusted(5, 0, -5, -5), Qt::AlignBottom | Qt::AlignRight,
            "Alpha");
    }
}

void ImageLabel::wheelEvent(QWheelEvent *event)
{
    if (m_originalPixmap.isNull())
        return;

    // Zoom with mouse wheel
    const double delta = event->angleDelta().y();
    double factor = (delta > 0) ? 1.15 : 1.0 / 1.15;

    // Zoom towards mouse position
    QPointF mousePos = event->position();
    QPointF oldScenePos = (mousePos - m_panOffset) / m_zoom;

    m_zoom = qBound(0.05, m_zoom * factor, 50.0);
    m_zoomMode = ZoomMode::FreeZoom;

    // Adjust pan to keep the point under the mouse stationary
    m_panOffset = mousePos - oldScenePos * m_zoom;

    updateTransform();
    update();
    emit zoomChanged(m_zoom);
    event->accept();
}

void ImageLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_leftButtonPressed = true;
        m_lastMousePos = event->position();
        m_isPanning = true;
        setCursor(Qt::ClosedHandCursor);
    }
    event->accept();
}

void ImageLabel::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPanning && m_leftButtonPressed) {
        QPointF delta = event->position() - m_lastMousePos;
        m_panOffset += delta;
        m_lastMousePos = event->position();

        if (m_zoomMode == ZoomMode::FitToWindow)
            m_zoomMode = ZoomMode::FreeZoom;

        update();
    }
    event->accept();
}

void ImageLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_leftButtonPressed = false;
        m_isPanning = false;
        setCursor(Qt::OpenHandCursor);
    }
    event->accept();
}

void ImageLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!m_originalPixmap.isNull()) {
        if (m_zoomMode == ZoomMode::FitToWindow)
            actualSize();
        else
            fitToWindow();
    }
    event->accept();
}

void ImageLabel::resizeEvent(QResizeEvent *event)
{
    if (m_zoomMode == ZoomMode::FitToWindow && !m_originalPixmap.isNull())
        fitToWindow();
    QWidget::resizeEvent(event);
}

// ============================================================
// ImageViewer Implementation
// ============================================================

ImageViewer::ImageViewer(QWidget *parent)
    : QWidget(parent)
    , m_currentIndex(-1)
{
    setupUi();
    setupToolbar();
    setupConnections();
}

void ImageViewer::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Scroll area containing the image label
    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setAlignment(Qt::AlignCenter);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setStyleSheet("QScrollArea { background: #2b2b2b; border: none; }");

    m_imageLabel = new ImageLabel(m_scrollArea);
    m_scrollArea->setWidget(m_imageLabel);

    mainLayout->addWidget(m_scrollArea);
}

void ImageViewer::setupToolbar()
{
    m_toolBar = new QToolBar("Outils Image");
    m_toolBar->setMovable(false);
    m_toolBar->setIconSize(QSize(18, 18));
    m_toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_toolBar->setStyleSheet(R"(
        QToolBar {
            background: #f5f5f5;
            border-bottom: 1px solid #ddd;
            spacing: 2px;
            padding: 2px;
        }
        QToolButton {
            padding: 4px;
            border: 1px solid transparent;
            border-radius: 3px;
        }
        QToolButton:hover {
            background: #e0e0e0;
            border-color: #ccc;
        }
        QToolButton:pressed {
            background: #d0d0d0;
        }
        QToolButton:disabled {
            color: #aaa;
        }
    )");

    // File operations
    m_openAction = m_toolBar->addAction(
        style()->standardIcon(QStyle::SP_DialogOpenButton), "Ouvrir");

    m_toolBar->addSeparator();

    // Navigation
    m_prevAction = m_toolBar->addAction(
        style()->standardIcon(QStyle::SP_ArrowBack), "Precedent");
    m_nextAction = m_toolBar->addAction(
        style()->standardIcon(QStyle::SP_ArrowForward), "Suivant");

    m_toolBar->addSeparator();

    // Zoom
    m_zoomInAction = m_toolBar->addAction(
        style()->standardIcon(QStyle::SP_ArrowUp), "Zoom +");
    m_zoomOutAction = m_toolBar->addAction(
        style()->standardIcon(QStyle::SP_ArrowDown), "Zoom -");
    m_fitAction = m_toolBar->addAction(
        style()->standardIcon(QStyle::SP_DialogYesButton), "Ajuster");
    m_actualSizeAction = m_toolBar->addAction(
        style()->standardIcon(QStyle::SP_ComputerIcon), "Taille reelle");

    m_toolBar->addSeparator();

    // Transform
    m_rotateLeftAction = m_toolBar->addAction(
        style()->standardIcon(QStyle::SP_FileDialogBack), "Rot. -90");
    m_rotateRightAction = m_toolBar->addAction(
        style()->standardIcon(QStyle::SP_FileDialogContentsView), "Rot. +90");
    m_flipHAction = m_toolBar->addAction(
        style()->standardIcon(QStyle::SP_BrowserReload), "Miroir H");
    m_flipVAction = m_toolBar->addAction(
        style()->standardIcon(QStyle::SP_BrowserStop), "Miroir V");
    m_resetAction = m_toolBar->addAction(
        style()->standardIcon(QStyle::SP_DialogResetButton), "Reinitialiser");

    m_toolBar->addSeparator();

    // Slideshow
    m_slideshowAction = m_toolBar->addAction(
        style()->standardIcon(QStyle::SP_MediaPlay), "Diaporama");
    m_slideshowAction->setCheckable(true);

    // Initially disable navigation and transform actions
    m_prevAction->setEnabled(false);
    m_nextAction->setEnabled(false);
    m_zoomInAction->setEnabled(false);
    m_zoomOutAction->setEnabled(false);
    m_fitAction->setEnabled(false);
    m_actualSizeAction->setEnabled(false);
    m_rotateLeftAction->setEnabled(false);
    m_rotateRightAction->setEnabled(false);
    m_flipHAction->setEnabled(false);
    m_flipVAction->setEnabled(false);
    m_resetAction->setEnabled(false);
    m_slideshowAction->setEnabled(false);

    // Set tooltips
    m_openAction->setToolTip("Ouvrir une image (Ctrl+O)");
    m_prevAction->setToolTip("Image precedente (Gauche)");
    m_nextAction->setToolTip("Image suivante (Droite)");
    m_zoomInAction->setToolTip("Zoom avant (Ctrl+Plus)");
    m_zoomOutAction->setToolTip("Zoom arriere (Ctrl+Moins)");
    m_fitAction->setToolTip("Ajuster a la fenetre (Ctrl+F)");
    m_actualSizeAction->setToolTip("Taille reelle (Ctrl+1)");
    m_rotateLeftAction->setToolTip("Rotation -90 degres (Ctrl+L)");
    m_rotateRightAction->setToolTip("Rotation +90 degres (Ctrl+R)");
    m_flipHAction->setToolTip("Miroir horizontal");
    m_flipVAction->setToolTip("Miroir vertical");
    m_resetAction->setToolTip("Reinitialiser les transformations");
    m_slideshowAction->setToolTip("Demarrer/Arreter le diaporama");

    ((QVBoxLayout*)layout())->insertWidget(0, m_toolBar);
}

void ImageViewer::setupConnections()
{
    // Open
    connect(m_openAction, &QAction::triggered, this, [this]() {
        const QString path = QFileDialog::getOpenFileName(this, "Ouvrir une image",
            QString(),
            "Images (*.png *.jpg *.jpeg *.bmp *.gif *.tiff *.tif *.webp *.svg);;"
            "Tous les fichiers (*)");
        if (!path.isEmpty())
            loadImage(path);
    });

    // Navigation
    connect(m_prevAction, &QAction::triggered, this, &ImageViewer::previousImage);
    connect(m_nextAction, &QAction::triggered, this, &ImageViewer::nextImage);

    // Zoom
    connect(m_zoomInAction, &QAction::triggered, m_imageLabel, [this]() {
        m_imageLabel->zoomIn(1.25);
    });
    connect(m_zoomOutAction, &QAction::triggered, m_imageLabel, [this]() {
        m_imageLabel->zoomOut(0.8);
    });
    connect(m_fitAction, &QAction::triggered, m_imageLabel, &ImageLabel::fitToWindow);
    connect(m_actualSizeAction, &QAction::triggered, m_imageLabel, &ImageLabel::actualSize);

    // Transform
    connect(m_rotateLeftAction, &QAction::triggered, m_imageLabel, &ImageLabel::rotateLeft);
    connect(m_rotateRightAction, &QAction::triggered, m_imageLabel, &ImageLabel::rotateRight);
    connect(m_flipHAction, &QAction::triggered, m_imageLabel, &ImageLabel::flipHorizontal);
    connect(m_flipVAction, &QAction::triggered, m_imageLabel, &ImageLabel::flipVertical);
    connect(m_resetAction, &QAction::triggered, m_imageLabel, &ImageLabel::resetTransform);

    // Slideshow
    connect(m_slideshowAction, &QAction::toggled, this, [this](bool checked) {
        if (checked) {
            m_slideshowAction->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
            m_imageLabel->startSlideshow(3000);
        } else {
            m_slideshowAction->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            m_imageLabel->stopSlideshow();
        }
    });

    // Status from image label
    connect(m_imageLabel, &ImageLabel::statusMessage,
            this, &ImageViewer::statusMessage);
    connect(m_imageLabel, &ImageLabel::zoomChanged, this, [this](double zoom) {
        emit statusMessage(QString("Zoom: %1%").arg(qRound(zoom * 100)));
    });
}

void ImageViewer::loadImage(const QString &filePath)
{
    QPixmap pixmap;
    if (!pixmap.load(filePath)) {
        QMessageBox::warning(this, "Erreur de chargement",
            "Impossible de charger l'image :\n" + filePath);
        return;
    }

    m_imageLabel->setImage(pixmap);

    // Update file list if not already present
    if (!m_imageFiles.contains(filePath)) {
        m_imageFiles.clear();
        m_imageFiles << filePath;
        m_currentIndex = 0;
    } else {
        m_currentIndex = m_imageFiles.indexOf(filePath);
    }

    // Enable toolbar actions
    bool hasImage = true;
    m_prevAction->setEnabled(hasImage && m_imageFiles.size() > 1);
    m_nextAction->setEnabled(hasImage && m_imageFiles.size() > 1);
    m_zoomInAction->setEnabled(hasImage);
    m_zoomOutAction->setEnabled(hasImage);
    m_fitAction->setEnabled(hasImage);
    m_actualSizeAction->setEnabled(hasImage);
    m_rotateLeftAction->setEnabled(hasImage);
    m_rotateRightAction->setEnabled(hasImage);
    m_flipHAction->setEnabled(hasImage);
    m_flipVAction->setEnabled(hasImage);
    m_resetAction->setEnabled(hasImage);
    m_slideshowAction->setEnabled(hasImage);

    updateImageInfo();
    emit imageChanged(filePath);
}

void ImageViewer::loadImages(const QStringList &filePaths)
{
    if (filePaths.isEmpty())
        return;

    m_imageFiles = filePaths;
    m_currentIndex = 0;
    loadImage(m_imageFiles.first());
}

void ImageViewer::previousImage()
{
    if (m_imageFiles.isEmpty() || m_currentIndex <= 0)
        return;
    m_currentIndex--;
    loadImage(m_imageFiles.at(m_currentIndex));
}

void ImageViewer::nextImage()
{
    if (m_imageFiles.isEmpty() || m_currentIndex >= m_imageFiles.size() - 1)
        return;
    m_currentIndex++;
    loadImage(m_imageFiles.at(m_currentIndex));
}

void ImageViewer::goToImage(int index)
{
    if (index < 0 || index >= m_imageFiles.size())
        return;
    m_currentIndex = index;
    loadImage(m_imageFiles.at(index));
}

int ImageViewer::currentImageIndex() const
{
    return m_currentIndex;
}

int ImageViewer::imageCount() const
{
    return m_imageFiles.size();
}

QString ImageViewer::currentFileName() const
{
    if (m_currentIndex >= 0 && m_currentIndex < m_imageFiles.size())
        return QFileInfo(m_imageFiles.at(m_currentIndex)).fileName();
    return QString();
}

void ImageViewer::onZoomChanged(double zoom)
{
    Q_UNUSED(zoom);
}

void ImageViewer::onFileDropped(const QString &filePath)
{
    loadImage(filePath);
}

void ImageViewer::updateImageInfo()
{
    if (m_currentIndex < 0 || m_currentIndex >= m_imageFiles.size()) {
        emit statusMessage("");
        return;
    }

    const QFileInfo fi(m_imageFiles.at(m_currentIndex));
    QPixmap px;
    px.load(m_imageFiles.at(m_currentIndex));

    QString info = QString("%1 (%2 x %3) | Image %4/%5")
        .arg(fi.fileName())
        .arg(px.width())
        .arg(px.height())
        .arg(m_currentIndex + 1)
        .arg(m_imageFiles.size());

    emit statusMessage(info);
    updateNavButtons();
}

void ImageViewer::updateNavButtons()
{
    m_prevAction->setEnabled(m_currentIndex > 0);
    m_nextAction->setEnabled(m_currentIndex < m_imageFiles.size() - 1);
}
