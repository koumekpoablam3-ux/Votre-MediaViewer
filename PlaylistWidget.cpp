#include "PlaylistWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QFileInfo>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFont>
#include <QBrush>
#include <QColor>
#include <QDebug>

// ============================================================
// Constructor
// ============================================================

PlaylistWidget::PlaylistWidget(QWidget *parent)
    : QWidget(parent)
    , m_currentIndex(-1)
{
    setupUi();
}

// ============================================================
// UI Setup
// ============================================================

void PlaylistWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    // Header
    QLabel *headerLabel = new QLabel("\U0001F3B5 Playlist");
    headerLabel->setStyleSheet(
        "font-size: 14px; font-weight: bold; color: #333; "
        "padding: 6px; border-bottom: 2px solid #667eea;");
    mainLayout->addWidget(headerLabel);

    // List widget
    m_listWidget = new QListWidget;
    m_listWidget->setAlternatingRowColors(true);
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listWidget->setDragDropMode(QAbstractItemView::InternalMove);
    m_listWidget->setDefaultDropAction(Qt::MoveAction);
    m_listWidget->setAcceptDrops(true);
    m_listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Style the list
    m_listWidget->setStyleSheet(R"(
        QListWidget {
            background: #fafafa;
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            outline: none;
            font-size: 12px;
        }
        QListWidget::item {
            padding: 6px 8px;
            border-bottom: 1px solid #f0f0f0;
        }
        QListWidget::item:hover {
            background: #f0f4ff;
        }
        QListWidget::item:selected {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #667eea, stop:1 #764ba2);
            color: white;
        }
        QListWidget::item:selected:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #5a6fd6, stop:1 #6a4196);
            color: white;
        }
        QListWidget::item:alternate {
            background: #f5f5f5;
        }
        QScrollBar:vertical {
            width: 8px;
            background: transparent;
        }
        QScrollBar::handle:vertical {
            background: #ccc;
            border-radius: 4px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background: #aaa;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )");

    mainLayout->addWidget(m_listWidget);

    // Bottom info
    QLabel *infoLabel = new QLabel("Double-cliquez pour jouer | "
                                     "Glissez pour reordonner");
    infoLabel->setStyleSheet("font-size: 10px; color: #999; padding: 4px;");
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    // Connections
    connect(m_listWidget, &QListWidget::itemDoubleClicked,
            this, &PlaylistWidget::onItemDoubleClicked);
    connect(m_listWidget, &QListWidget::customContextMenuRequested,
            this, &PlaylistWidget::onCustomContextMenu);

    setMaximumWidth(280);
    setMinimumWidth(180);
}

// ============================================================
// Public Methods
// ============================================================

void PlaylistWidget::setPlaylist(const QStringList &filePaths)
{
    m_playlist = filePaths;
    m_listWidget->clear();

    for (int i = 0; i < m_playlist.size(); ++i) {
        QListWidgetItem *item = new QListWidgetItem(m_listWidget);
        item->setText(formatTrackName(m_playlist.at(i)));
        item->setData(Qt::UserRole, m_playlist.at(i));
        item->setData(Qt::UserRole + 1, i); // Track number
    }

    updateRowNumbers();
}

void PlaylistWidget::clear()
{
    m_playlist.clear();
    m_currentIndex = -1;
    m_listWidget->clear();
}

void PlaylistWidget::addTrack(const QString &filePath)
{
    if (m_playlist.contains(filePath))
        return;

    m_playlist << filePath;

    QListWidgetItem *item = new QListWidgetItem(m_listWidget);
    item->setText(formatTrackName(filePath));
    item->setData(Qt::UserRole, filePath);
    item->setData(Qt::UserRole + 1, m_playlist.size() - 1);

    updateRowNumbers();
}

void PlaylistWidget::removeTrack(int index)
{
    if (index < 0 || index >= m_playlist.size())
        return;

    m_playlist.removeAt(index);
    delete m_listWidget->takeItem(index);

    // Update current index
    if (m_currentIndex == index) {
        m_currentIndex = -1;
    } else if (m_currentIndex > index) {
        m_currentIndex--;
    }

    updateRowNumbers();
    emit trackRemoved(index);
}

void PlaylistWidget::setCurrentIndex(int index)
{
    // Clear previous selection highlight
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem *item = m_listWidget->item(i);
        QFont font = item->font();
        font.setBold(false);
        item->setFont(font);
    }

    m_currentIndex = index;

    if (index >= 0 && index < m_listWidget->count()) {
        m_listWidget->setCurrentRow(index);
        QListWidgetItem *item = m_listWidget->item(index);
        QFont font = item->font();
        font.setBold(true);
        item->setFont(font);

        // Ensure visible
        m_listWidget->scrollToItem(item, QAbstractItemView::EnsureVisible);
    }
}

int PlaylistWidget::currentIndex() const
{
    return m_currentIndex;
}

int PlaylistWidget::trackCount() const
{
    return m_playlist.size();
}

// ============================================================
// Private Slots
// ============================================================

void PlaylistWidget::onItemDoubleClicked(QListWidgetItem *item)
{
    if (!item)
        return;

    const QString filePath = item->data(Qt::UserRole).toString();
    emit trackSelected(filePath);
}

void PlaylistWidget::onCustomContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = m_listWidget->itemAt(pos);
    int index = (item) ? m_listWidget->row(item) : -1;

    QMenu contextMenu(this);
    contextMenu.setStyleSheet(R"(
        QMenu {
            background: white;
            border: 1px solid #ddd;
            border-radius: 4px;
            padding: 4px;
        }
        QMenu::item {
            padding: 6px 20px;
        }
        QMenu::item:hover {
            background: #f0f4ff;
        }
        QMenu::separator {
            height: 1px;
            background: #eee;
            margin: 4px 0;
        }
    )");

    QAction *playAction = contextMenu.addAction("\u25B6  Lire");
    contextMenu.addSeparator();

    QAction *moveUpAction = contextMenu.addAction("\u2191  Monter");
    QAction *moveDownAction = contextMenu.addAction("\u2193  Descendre");
    contextMenu.addSeparator();

    QAction *removeAction = contextMenu.addAction("\u2716  Supprimer");
    QAction *clearAction = contextMenu.addAction("\U0001F5D1  Tout effacer");

    // Enable/disable based on context
    playAction->setEnabled(item != nullptr);
    moveUpAction->setEnabled(index > 0);
    moveDownAction->setEnabled(item != nullptr && index < m_listWidget->count() - 1);
    removeAction->setEnabled(item != nullptr);
    clearAction->setEnabled(m_listWidget->count() > 0);

    QAction *selectedAction = contextMenu.exec(m_listWidget->mapToGlobal(pos));
    if (!selectedAction)
        return;

    if (selectedAction == playAction && item) {
        emit trackSelected(item->data(Qt::UserRole).toString());
    } else if (selectedAction == moveUpAction && index > 0) {
        onMoveUp();
    } else if (selectedAction == moveDownAction && index < m_listWidget->count() - 1) {
        onMoveDown();
    } else if (selectedAction == removeAction) {
        onRemoveSelected();
    } else if (selectedAction == clearAction) {
        onClearAll();
    }
}

void PlaylistWidget::onRemoveSelected()
{
    int row = m_listWidget->currentRow();
    if (row >= 0)
        removeTrack(row);
}

void PlaylistWidget::onClearAll()
{
    clear();
    emit trackRemoved(-1);
}

void PlaylistWidget::onMoveUp()
{
    int row = m_listWidget->currentRow();
    if (row <= 0)
        return;

    // Swap in data
    m_playlist.swapItemsAt(row, row - 1);

    // Swap in list widget
    QListWidgetItem *item = m_listWidget->takeItem(row);
    m_listWidget->insertItem(row - 1, item);

    // Update current index
    if (m_currentIndex == row)
        m_currentIndex = row - 1;
    else if (m_currentIndex == row - 1)
        m_currentIndex = row;

    m_listWidget->setCurrentRow(row - 1);
    updateRowNumbers();
}

void PlaylistWidget::onMoveDown()
{
    int row = m_listWidget->currentRow();
    if (row < 0 || row >= m_listWidget->count() - 1)
        return;

    // Swap in data
    m_playlist.swapItemsAt(row, row + 1);

    // Swap in list widget
    QListWidgetItem *item = m_listWidget->takeItem(row);
    m_listWidget->insertItem(row + 1, item);

    // Update current index
    if (m_currentIndex == row)
        m_currentIndex = row + 1;
    else if (m_currentIndex == row + 1)
        m_currentIndex = row;

    m_listWidget->setCurrentRow(row + 1);
    updateRowNumbers();
}

// ============================================================
// Private Methods
// ============================================================

void PlaylistWidget::updateRowNumbers()
{
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem *item = m_listWidget->item(i);
        QString displayText = formatTrackName(item->data(Qt::UserRole).toString());
        item->setText(QString("%1. %2").arg(i + 1).arg(displayText));
    }
}

QString PlaylistWidget::formatTrackName(const QString &filePath) const
{
    QFileInfo fi(filePath);
    QString baseName = fi.completeBaseName();

    // Try to extract artist - title pattern
    static QRegularExpression re("^(.+?)\\s*[-]\\s*(.+)$");
    QRegularExpressionMatch match = re.match(baseName);

    if (match.hasMatch()) {
        return match.captured(2).trimmed(); // Show just the title
    }

    return baseName;
}
