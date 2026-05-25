#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Application metadata
    QApplication::setApplicationName("MediaViewer");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("MediaViewer");

    // High DPI support
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    // Set application-wide stylesheet for a modern look
    app.setStyleSheet(R"(
        /* Global styles */
        QMainWindow {
            background: #ffffff;
        }
        QMenuBar {
            background: #f8f8f8;
            border-bottom: 1px solid #e0e0e0;
            padding: 2px;
        }
        QMenuBar::item {
            padding: 6px 12px;
            background: transparent;
            border-radius: 4px;
        }
        QMenuBar::item:selected {
            background: #e8e8e8;
        }
        QMenu {
            background: white;
            border: 1px solid #ddd;
            border-radius: 6px;
            padding: 6px;
        }
        QMenu::item {
            padding: 6px 30px 6px 20px;
            border-radius: 4px;
        }
        QMenu::item:selected {
            background: #667eea;
            color: white;
        }
        QMenu::separator {
            height: 1px;
            background: #eee;
            margin: 4px 10px;
        }
        QStatusBar {
            background: #f5f5f5;
            border-top: 1px solid #e0e0e0;
            font-size: 11px;
            color: #666;
        }
        QToolBar {
            background: #f8f8f8;
            border-bottom: 1px solid #e0e0e0;
            spacing: 4px;
            padding: 3px;
        }
        QToolButton {
            padding: 5px;
            border: 1px solid transparent;
            border-radius: 4px;
        }
        QToolButton:hover {
            background: #e8e8e8;
            border-color: #ddd;
        }
        QToolButton:pressed {
            background: #ddd;
        }

        /* Scrollbar */
        QScrollBar:vertical {
            width: 10px;
            background: transparent;
            margin: 0;
        }
        QScrollBar::handle:vertical {
            background: #ccc;
            border-radius: 5px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background: #aaa;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: transparent;
        }
        QScrollBar:horizontal {
            height: 10px;
            background: transparent;
        }
        QScrollBar::handle:horizontal {
            background: #ccc;
            border-radius: 5px;
            min-width: 30px;
        }
        QScrollBar::handle:horizontal:hover {
            background: #aaa;
        }

        /* Tab widget */
        QTabWidget::pane {
            border: none;
        }
    )");

    MainWindow mainWindow;
    mainWindow.show();

    // Handle command-line file arguments
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            // The MainWindow will handle opening files
            QString filePath = QString::fromLocal8Bit(argv[i]);
            QMetaObject::invokeMethod(&mainWindow, "onOpenFile",
                Qt::QueuedConnection);
        }
    }

    return app.exec();
}
