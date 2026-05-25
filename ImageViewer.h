#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QWidget>
#include <QPixmap>
#include <QPointF>
#include <QSize>
#include <QStringList>
#include <QTimer>
#include <QLabel>
#include <QScrollArea>
#include <QStack>
#include <QToolBar>
#include <QAction>
#include <QStyle>

// ============================================================
// ImageLabel - Custom widget for rendering and manipulating
//               images with zoom, pan, and rotation support.
// ============================================================
class ImageLabel : public QWidget
{
    Q_OBJECT

public:
    enum class ZoomMode {
        FitToWindow,
        ActualSize,
        FreeZoom
    };

    explicit ImageLabel(QWidget *parent = nullptr);

    // Image loading
    void setImage(const QPixmap &pixmap);
    void clearImage();
    bool hasImage() const;

    // Transformations
    void zoomIn(double factor = 1.25);
    void zoomOut(double factor = 0.8);
    void setZoom(double scale);
    void fitToWindow();
    void actualSize();
    void rotateLeft();
    void rotateRight();
    void flipHorizontal();
    void flipVertical();
    void resetTransform();

    // Getters
    double zoomLevel() const { return m_zoom; }
    ZoomMode zoomMode() const { return m_zoomMode; }
    int rotationAngle() const { return m_rotation; }

    // Slideshow
    bool isSlideshowRunning() const;

signals:
    void zoomChanged(double zoom);
    void rotationChanged(int angle);
    void statusMessage(const QString &info);

public slots:
    void startSlideshow(int intervalMs = 3000);
    void stopSlideshow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateTransform();

    QPixmap m_originalPixmap;
    QPixmap m_transformedPixmap;

    double m_zoom;
    ZoomMode m_zoomMode;
    int m_rotation;
    bool m_flippedH;
    bool m_flippedV;

    // Panning
    QPointF m_panOffset;
    QPointF m_lastMousePos;
    bool m_isPanning;
    bool m_leftButtonPressed;

    // Slideshow
    QTimer *m_slideshowTimer;
};

// ============================================================
// ImageViewer - Complete image viewer panel with toolbar,
//               scroll area, and ImageLabel.
// ============================================================
class ImageViewer : public QWidget
{
    Q_OBJECT

public:
    explicit ImageViewer(QWidget *parent = nullptr);

    // Load single or multiple images
    void loadImage(const QString &filePath);
    void loadImages(const QStringList &filePaths);

    // Navigation
    void previousImage();
    void nextImage();
    void goToImage(int index);
    int currentImageIndex() const;
    int imageCount() const;

    QString currentFileName() const;

signals:
    void statusMessage(const QString &info);
    void imageChanged(const QString &filePath);

private slots:
    void onZoomChanged(double zoom);
    void onFileDropped(const QString &filePath);

private:
    void setupUi();
    void setupToolbar();
    void setupConnections();
    void updateImageInfo();
    void updateNavButtons();

    // Widgets
    QToolBar *m_toolBar;
    QScrollArea *m_scrollArea;
    ImageLabel *m_imageLabel;
    QLabel *m_emptyLabel;

    // Toolbar actions
    QAction *m_openAction;
    QAction *m_prevAction;
    QAction *m_nextAction;
    QAction *m_zoomInAction;
    QAction *m_zoomOutAction;
    QAction *m_fitAction;
    QAction *m_actualSizeAction;
    QAction *m_rotateLeftAction;
    QAction *m_rotateRightAction;
    QAction *m_flipHAction;
    QAction *m_flipVAction;
    QAction *m_slideshowAction;
    QAction *m_resetAction;

    // Image list
    QStringList m_imageFiles;
    int m_currentIndex;
};

#endif // IMAGEVIEWER_H
