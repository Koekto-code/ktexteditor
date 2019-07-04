/*  This file is part of the KDE libraries and the KTextEditor project.
 *
 *  Copyright (C) 2019 Nibaldo González S. <nibgonz@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

//BEGIN Includes
#include "katemodemenulist.h"

#include "katedocument.h"
#include "kateconfig.h"
#include "kateview.h"
#include "kateglobal.h"
#include "katesyntaxmanager.h"
#include "katepartdebug.h"

#include <QWidgetAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QAbstractItemView>
#include <QApplication>
//END Includes


namespace
{
/**
 * Detect words delimiters:
 *      ! " # $ % & ' ( ) * + , - . / : ;
 *      < = > ? [ \ ] ^ ` { | } ~ « »
 */
static bool isDelimiter(const ushort c)
{
    return (c <= 126 && c >= 33 && (c >= 123 || c <= 47 ||
           (c <= 96 && c >= 58 && c != 95 && (c >= 91 || c <= 63)))) ||
           c == 171 || c == 187;
}
}


void KateModeMenuList::init(const SearchBarPosition searchBarPos)
{
    m_list = new KateModeMenuListData::ListView(this);
    m_searchBar = new KateModeMenuListData::SearchLine(this);

    /*
     * Load list widget, scroll bar and items.
     */
    m_scroll = new QScrollBar(Qt::Vertical, this);
    m_list->setVerticalScrollBar(m_scroll);
    // The vertical scroll bar will be added in another layout
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setIconSize(QSize(m_iconSize, m_iconSize));
    m_list->setResizeMode(QListView::Adjust);
    // Initial size of the list widget, this can be modified later
    m_list->setSizeList(428);

    loadHighlightingModel(); // Data model (items)

    // Add scroll bar and set margin
    QHBoxLayout *layoutScrollBar = new QHBoxLayout();
    layoutScrollBar->addWidget(m_scroll);
    layoutScrollBar->setMargin(2);

    /*
     * Search bar widget.
     */
    m_searchBar->setPlaceholderText(i18nc("Placeholder in search bar", "Search..."));
    m_searchBar->setToolTip(i18nc("ToolTip of the search bar of modes of syntax highlighting", "Search for syntax highlighting modes by language name or file extension (for example, C++ or .cpp)"));
    m_searchBar->setMaxLength(200);

    m_list->setFocusProxy(m_searchBar);

    /*
     * Set layouts and widgets.
     */
    QWidget *container = new QWidget();
    QVBoxLayout *layoutContainer = new QVBoxLayout();
    m_layoutList = new QGridLayout();
    QHBoxLayout *layoutSearchBar = new QHBoxLayout();

    // Overlap scroll bar above the list widget
    m_layoutList->addWidget(m_list, 0, 0, Qt::AlignLeft);
    m_layoutList->addLayout(layoutScrollBar, 0, 0, Qt::AlignRight);

    layoutSearchBar->addWidget(m_searchBar);

    if (searchBarPos == Top) {
        layoutContainer->addLayout(layoutSearchBar);
    }
    layoutContainer->addLayout(m_layoutList);
    if (searchBarPos == Bottom) {
        layoutContainer->addLayout(layoutSearchBar);
    }
    container->setLayout(layoutContainer);

    QWidgetAction *widAct = new QWidgetAction(this);
    widAct->setDefaultWidget(container);
    addAction(widAct);

    // Detect selected item with one click.
    // This also applies to double-clicks.
    connect(m_list, &KateModeMenuListData::ListView::clicked,
            this, &KateModeMenuList::selectHighlighting);
}


void KateModeMenuList::loadHighlightingModel()
{
    m_model = new QStandardItemModel(0, 0);
    m_list->setModel(m_model);

    QString *prevHlSection = nullptr;
    /*
     * The width of the text container in the item, in pixels. This is used to make
     * a custom word wrap and prevent the item's text from passing under the scroll bar.
     * NOTE: 12 = the edges
     */
    const int maxWidthText = m_list->sizeHint().width() - m_scroll->sizeHint().width() - m_iconSize - 12;

    /*
     * Get list of modes from KateModeManager::list().
     * We assume that the modes are arranged according to sections, alphabetically;
     * and the attribute "translatedSection" isn't empty if "section" has a value.
     */
    for (auto *hl : KTextEditor::EditorPrivate::self()->modeManager()->list()) {
        /*
         * Detects a new section.
         */
        if ( !hl->translatedSection.isEmpty() && (prevHlSection == nullptr || hl->translatedSection != *prevHlSection) ) {
            QPixmap transparent = QPixmap(m_iconSize / 2, m_iconSize / 2);
            transparent.fill(Qt::transparent);

            /*
             * Add a separator to the list.
             */
            KateModeMenuListData::ListItem *separator = new KateModeMenuListData::ListItem();
            separator->setFlags(Qt::NoItemFlags);
            separator->setSizeHint(QSize(separator->sizeHint().width() - 2, 4));
            separator->setBackground(QBrush(transparent));

            QFrame *line = new QFrame();
            line->setFrameStyle(QFrame::HLine);

            m_model->appendRow(separator);
            m_list->setIndexWidget( m_model->index(separator->row(), 0), line );

            /*
             * Add the section name to the list.
             */
            KateModeMenuListData::ListItem *section = new KateModeMenuListData::ListItem();
            section->setFlags(Qt::NoItemFlags);

            QLabel *label = new QLabel(hl->sectionTranslated());
            if (m_list->layoutDirection() == Qt::RightToLeft) {
                label->setAlignment(Qt::AlignRight);
            }
            label->setTextFormat(Qt::RichText);
            label->setIndent(6);

            // NOTE: Names of sections in bold. The font color
            // should change according to Kate's color theme.
            QFont font = label->font();
            font.setWeight(QFont::Bold);
            label->setFont(font);

            section->setBackground(QBrush(transparent));

            m_model->appendRow(section);
            m_list->setIndexWidget( m_model->index(section->row(), 0), label );
        }
        prevHlSection = hl->translatedSection.isNull() ? nullptr : &hl->translatedSection;

        /*
         * Create item in the list with the language name.
         */
        KateModeMenuListData::ListItem *item = new KateModeMenuListData::ListItem();
        item->setText(setWordWrap( hl->nameTranslated(), maxWidthText, m_list->fontMetrics() ));
        item->setMode(hl);
        // NOTE: Search names generated in: KateModeMenuListData::SearchLine::updateSearch()
        // item->generateSearchName( hl->translatedName.isEmpty() ? &hl->name : &hl->translatedName );

        // Set empty icon
        QPixmap emptyIcon(m_iconSize, m_iconSize);
        emptyIcon.fill(Qt::transparent);
        item->setIcon(QIcon(emptyIcon));
        item->setEditable(false);
        // Add item
        m_model->appendRow(item);
    }
}


void KateModeMenuList::setButton(QPushButton* button, const bool bAutoUpdateTextButton, AlignmentButton position)
{
    if (position == Inverse) {
        if (layoutDirection() == Qt::RightToLeft) {
            m_position = KateModeMenuList::Left;
        } else {
            m_position = KateModeMenuList::Right;
        }
    } else if (position == Left && layoutDirection() != Qt::RightToLeft) {
        m_position = KateModeMenuList::Default;
    } else {
        m_position = position;
    }

    m_pushButton = button;
    m_bAutoUpdateTextButton = bAutoUpdateTextButton;
}


void KateModeMenuList::setSizeList(const int height, const int width)
{
    m_list->setSizeList(height, width);
}


void KateModeMenuList::autoScroll()
{
    if (m_autoScroll == ScrollToSelectedItem) {
        m_list->setCurrentItem(m_selectedItem->row());
        m_list->scrollToItem(m_selectedItem->row(), QAbstractItemView::PositionAtCenter);
    } else {
        m_list->setCurrentItem(0);
        m_list->scrollToTop();
    }
}


void KateModeMenuList::showEvent(QShowEvent* event)
{
    Q_UNUSED(event);

    // TODO: Put the menu in the center of the window if the status bar is hidden.
    // Set the menu position
    if (m_pushButton && m_pushButton->isVisible()) {
        if (m_position == Right) {
            // New menu position
            int newMenu_x = pos().x() - geometry().width() + m_pushButton->geometry().width();
            // Get position of the right edge of the toggle button
            const int buttonPositionRight = m_pushButton->mapToGlobal(QPoint(0, 0)).x()
                                            + m_pushButton->geometry().width();
            if (newMenu_x < 0) {
                newMenu_x = 0;
            } else if ( newMenu_x + geometry().width() < buttonPositionRight ) {
                newMenu_x = buttonPositionRight - geometry().width();
            }
            move(newMenu_x, pos().y());
        }
        else if (m_position == Left) {
            move(m_pushButton->mapToGlobal(QPoint(0, 0)).x(), pos().y());
        }
    }

    // Select text from the search bar
    if (!m_searchBar->text().isEmpty()) {
        if (m_searchBar->text().simplified().isEmpty()) {
            m_searchBar->clear();
        } else {
            m_searchBar->selectAll();
        }
    }

    // Set focus on the list. The list widget uses focus proxy to the search bar.
    m_list->setFocus(Qt::ActiveWindowFocusReason);

    KTextEditor::DocumentPrivate *doc = m_doc;
    if (!doc) {
        return;
    }

    // First show or if an external changed the current syntax highlighting.
    if (!m_selectedItem || ( m_selectedItem->hasMode() && m_selectedItem->getMode()->name != doc->fileType() )) {
        selectHighlightingFromExternal(doc->fileType());
    }
}


void KateModeMenuList::updateSelectedItem(KateModeMenuListData::ListItem *item)
{
    // Change the previously selected item to empty icon
    if (m_selectedItem) {
        QPixmap emptyIcon(m_iconSize, m_iconSize);
        emptyIcon.fill(Qt::transparent);
        m_selectedItem->setIcon(QIcon(emptyIcon));
    }

    // Update the selected item
    item->setIcon(m_checkIcon);
    m_selectedItem = item;
    m_list->setCurrentItem(item->row());

    // Change text of the trigger button
    if (m_bAutoUpdateTextButton && m_pushButton && item->hasMode()) {
        m_pushButton->setText(item->getMode()->nameTranslated());
    }
}


void KateModeMenuList::selectHighlightingSetVisibility(QStandardItem *pItem, const bool bHideMenu)
{
    KateModeMenuListData::ListItem *item = static_cast<KateModeMenuListData::ListItem *>(pItem);

    updateSelectedItem(item);

    if (bHideMenu) {
        hide();
    }

    // Apply syntax highlighting
    KTextEditor::DocumentPrivate *doc = m_doc;
    if (doc && item->hasMode()) {
        doc->updateFileType(item->getMode()->name, true);
    }
}


void KateModeMenuList::selectHighlighting(const QModelIndex &index)
{
    selectHighlightingSetVisibility(m_model->item(index.row(), 0), true);
}


void KateModeMenuList::selectHighlightingFromExternal(const QString &nameMode)
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
        KateModeMenuListData::ListItem *item = static_cast<KateModeMenuListData::ListItem *>( m_model->item(i, 0) );

        if (!item->hasMode() || m_model->item(i, 0)->text().isEmpty()) {
            continue;
        }
        if (item->getMode()->name == nameMode || ( nameMode.isEmpty() && item->getMode()->name == QLatin1String("Normal") )) {
            updateSelectedItem(item);

            // Clear search
            if (!m_searchBar->text().isEmpty()) {
                // Prevent the empty list message from being seen over the items for a short time
                if (m_emptyListMsg) {
                    m_emptyListMsg->hide();
                }

                // NOTE: This calls updateSearch(), it's scrolled to the selected item or the first item.
                m_searchBar->clear();
            } else if (m_autoScroll == ScrollToSelectedItem) {
                m_list->scrollToItem(i);
            } else {
                // autoScroll()
                m_list->setCurrentItem(0);
                m_list->scrollToTop();
            }
            return;
        }
    }
}


void KateModeMenuList::selectHighlightingFromExternal()
{
    KTextEditor::DocumentPrivate *doc = m_doc;
    if (doc) {
        selectHighlightingFromExternal(doc->fileType());
    }
}


void KateModeMenuList::loadEmptyMsg()
{
    m_emptyListMsg = new QLabel(i18nc("A search yielded no results", "No items matching your search"));
    m_emptyListMsg->setMargin(15);
    m_emptyListMsg->setWordWrap(true);

    QColor color = m_emptyListMsg->palette().color(QPalette::Text);
    m_emptyListMsg->setStyleSheet( QLatin1String("font-size: 14pt; color: rgba(") + QString::number(color.red()) + QLatin1Char(',') + QString::number(color.green()) + QLatin1Char(',') + QString::number(color.blue()) + QLatin1String(", 0.3);") );

    m_emptyListMsg->setAlignment(Qt::AlignCenter);
    m_layoutList->addWidget(m_emptyListMsg, 0, 0, Qt::AlignCenter);
}


QString KateModeMenuList::setWordWrap(const QString &text, const int maxWidth, const QFontMetrics &fontMetrics) const
{
    // Get the length of the text, in pixels, and compare it with the container
    if (fontMetrics.boundingRect(text).width() <= maxWidth) {
        return text;
    }

    // Add line breaks in the text to fit in the container
    QStringList words = text.split(QLatin1Char(' '));
    if (words.count() < 1) {
        return text;
    }
    QString newText = QString();
    QString tmpLineText = QString();

    for (int i = 0; i < words.count() - 1; ++i) {
        tmpLineText += words[i];

        // This prevents the last line of text from having only one word with 1 or 2 chars
        if ( i == words.count() - 3 && words[i + 2].length() <= 2 &&
            fontMetrics.boundingRect( tmpLineText + QLatin1Char(' ') + words[i + 1] + QLatin1Char(' ') + words[i + 2] ).width() > maxWidth ) {
            newText += tmpLineText + QLatin1Char('\n');
            tmpLineText.clear();
        }
        // Add line break if the maxWidth is exceeded with the next word
        else if ( fontMetrics.boundingRect( tmpLineText + QLatin1Char(' ') + words[i + 1] ).width() > maxWidth ) {
            newText += tmpLineText + QLatin1Char('\n');
            tmpLineText.clear();
        }
        else {
            tmpLineText.append(QLatin1Char(' '));
        }
    }

    // Add line breaks in delimiters, if the last word is greater than the container
    if (fontMetrics.boundingRect( words[words.count() - 1] ).width() > maxWidth) {
        const int lastw = words.count() - 1;
        for (int c = words[lastw].length() - 1; c >= 0; --c) {
            if (isDelimiter(words[lastw][c].unicode()) && fontMetrics.boundingRect( words[lastw].mid(0, c + 1) ).width() <= maxWidth) {
                words[lastw].insert(c + 1, QLatin1Char('\n'));
                break;
            }
        }
    }

    if (!tmpLineText.isEmpty()) {
        newText += tmpLineText;
    }
    newText += words[words.count() - 1];
    return newText;
}


void KateModeMenuListData::ListView::setSizeList(const int height, const int width)
{
    setMinimumWidth(width);
    setMaximumWidth(width);
    setMinimumHeight(height);
    setMaximumHeight(height);
}


bool KateModeMenuListData::ListItem::generateSearchName(const QString *itemName)
{
    QString searchName = QString(*itemName);
    bool bNewName = false;

    // Replace word delimiters with spaces
    for (int i = searchName.length() - 1; i >= 0; --i) {
        if (isDelimiter( searchName[i].unicode() )) {
            searchName.replace(i, 1, QLatin1Char(' '));
            if (!bNewName) {
                bNewName = true;
            }
        }
        // Avoid duplicate delimiters/spaces
        if (bNewName && i < searchName.length() - 1 && searchName[i].isSpace() && searchName[i + 1].isSpace()) {
            searchName.remove(i + 1, 1);
        }
    }

    if (bNewName) {
        if (searchName[searchName.length() - 1].isSpace()) {
            searchName.remove(searchName.length() - 1, 1);
        }
        if (searchName[0].isSpace()) {
            searchName.remove(0, 1);
        }
        m_searchName = new QString(searchName);
        return true;
    } else {
        m_searchName = itemName;
    }
    return false;
}


bool KateModeMenuListData::ListItem::matchExtension(const QString &text) const
{
    if (!hasMode() || m_type->wildcards.count() == 0) {
        return false;
    }

    /*
     * Only file extensions and full names are matched. Files like "Kconfig*"
     * aren't considered. It's also assumed that "text" doesn't contain '*'.
     */
    for (const auto &ext : m_type->wildcards) {
        // File extension
        if (ext.startsWith(QLatin1String("*."))) {
            if (text.length() == ext.length() - 2 && text.compare(ext.mid(2), Qt::CaseInsensitive) == 0) {
                return true;
            }
        } else if (text.length() != ext.length() || ext.endsWith(QLatin1Char('*'))) {
            continue;
        // Full name
        } else if (text.compare(&ext, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}


void KateModeMenuListData::ListView::keyPressEvent(QKeyEvent *event)
{
    // Ctrl/Alt/Shift/Meta + Return/Enter selects an item, but without hiding the menu
    if (( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return ) &&
        ( event->modifiers().testFlag(Qt::ControlModifier) || event->modifiers().testFlag(Qt::AltModifier) ||
          event->modifiers().testFlag(Qt::ShiftModifier) || event->modifiers().testFlag(Qt::MetaModifier) )) {
        m_parentMenu->selectHighlightingSetVisibility(m_parentMenu->m_list->currentItem(), false);
    }
    // Return/Enter selects an item and hide the menu
    else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        m_parentMenu->selectHighlightingSetVisibility(m_parentMenu->m_list->currentItem(), true);
    } else {
        QListView::keyPressEvent(event);
    }
}


void KateModeMenuListData::SearchLine::keyPressEvent(QKeyEvent *event)
{
    if (m_parentMenu->m_list &&
        ( event->matches(QKeySequence::MoveToNextLine) || event->matches(QKeySequence::SelectNextLine) ||
          event->matches(QKeySequence::MoveToPreviousLine) || event->matches(QKeySequence::SelectPreviousLine) ||
          event->matches(QKeySequence::MoveToNextPage) || event->matches(QKeySequence::SelectNextPage) ||
          event->matches(QKeySequence::MoveToPreviousPage) || event->matches(QKeySequence::SelectPreviousPage) ||
          event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter )) {
        QApplication::sendEvent(m_parentMenu->m_list, event);
    } else {
        QLineEdit::keyPressEvent(event);
    }
}


void KateModeMenuListData::SearchLine::init()
{
    connect(this, &KateModeMenuListData::SearchLine::textChanged,
            this, &KateModeMenuListData::SearchLine::_k_queueSearch);

    setEnabled(true);
    setClearButtonEnabled(true);
}


void KateModeMenuListData::SearchLine::clear()
{
    m_queuedSearches = 0;
    m_bSearchStateAutoScroll = (text().trimmed().isEmpty()) ? false : true;
    // NOTE: This calls "SearchLine::_k_queueSearch()" with an empty string.
    // The search clearing should be done without delays.
    QLineEdit::clear();
}


void KateModeMenuListData::SearchLine::_k_queueSearch(const QString &s)
{
    m_queuedSearches++;
    m_search = s;

    if (m_search.isEmpty()) {
        _k_activateSearch(); // Clear search without delay
    } else {
        QTimer::singleShot(m_searchDelay, this, &KateModeMenuListData::SearchLine::_k_activateSearch);
    }
}


void KateModeMenuListData::SearchLine::_k_activateSearch()
{
    m_queuedSearches--;

    if (m_queuedSearches <= 0) {
        updateSearch(m_search);
        m_queuedSearches = 0;
    }
}


void KateModeMenuListData::SearchLine::updateSearch(const QString &s)
{
    if (m_parentMenu->m_emptyListMsg) {
        m_parentMenu->m_emptyListMsg->hide();
    }
    if (m_parentMenu->m_scroll->isHidden()) {
        m_parentMenu->m_scroll->show();
    }

    KateModeMenuListData::ListView *listView = m_parentMenu->m_list;
    QStandardItemModel *listModel = m_parentMenu->m_model;

    const QString searchText = (s.isNull() ? text() : s).simplified();

    /*
     * Empty search bar.
     * Show all items and scroll to the selected item or to the first item.
     */
    if ( searchText.isEmpty() || (searchText.size() == 1 && searchText[0].isSpace()) ) {
        for (int i = 0; i < listModel->rowCount(); ++i) {
            if (listView->isRowHidden(i)) {
                listView->setRowHidden(i, false);
            }
        }

        // Don't auto-scroll if the search is already clear
        if (m_bSearchStateAutoScroll) {
            m_parentMenu->autoScroll();
        }
        m_bSearchStateAutoScroll = false;
        return;
    }

    /*
     * Prepare item filter.
     */
    int lastItem = -1;
    int lastSection = -1;
    bool bEmptySection = true;
    bool bSectionSeparator = false;
    bool bSectionName = false;
    bool bSearchExtensions = true;
    bool bExactMatch = false; // If the search name will not be used
    /*
     * It's used for two purposes, it's true if searchText is a
     * single alphanumeric character or if it starts with a point.
     * Both cases don't conflict, so a single bool is used.
     */
    bool bIsAlphaOrPointExt = false;

    /*
     * Don't search for extensions if the search text has only one character,
     * to avoid unwanted results. In this case, the items that start with
     * that character are displayed.
     */
    if (searchText.length() < 2) {
        bSearchExtensions = false;
        if (searchText[0].isLetterOrNumber()) {
            bIsAlphaOrPointExt = true;
        }
    }
    // If the search text has a point at the beginning, match extensions
    else if (searchText.length() > 1 && searchText[0].toLatin1() == 46) {
        bIsAlphaOrPointExt = true;
        bSearchExtensions = true;
        bExactMatch = true;
    }
    // Two characters: search using the normal name of the items
    else if (searchText.length() == 2) {
        bExactMatch = true;
        // if it contains the '*' character, don't match extensions
        if (searchText[1].toLatin1() == 42 || searchText[0].toLatin1() == 42) {
            bSearchExtensions = false;
        }
    }
    /*
     * Don't use the search name if the search text has delimiters.
     * Don't search in extensions if it contains the '*' character.
     */
    else {
        QString::const_iterator srcText = searchText.constBegin();
        QString::const_iterator endText = searchText.constEnd();

        for (int it = 0; it < searchText.length() / 2 + searchText.length() % 2; ++it) {
            --endText;
            const ushort ucsrc = srcText->unicode();
            const ushort ucend = endText->unicode();

            // If searchText contains "*"
            if (ucsrc == 42 || ucend == 42) {
                bSearchExtensions = false;
                bExactMatch = true;
                break;
            }
            if (!bExactMatch && ( isDelimiter(ucsrc) || (ucsrc != ucend && isDelimiter(ucend)) )) {
                bExactMatch = true;
            }
            ++srcText;
        }
    }

    /*
     * Filter items.
     */
    for (int i = 0; i < listModel->rowCount(); ++i) {
        QString itemName = listModel->item(i, 0)->text();

        /*
         * Hide/show the name of the section. If the text of the item
         * is empty, then it corresponds to the name of the section.
         */
        if (itemName.isEmpty()) {
            listView->setRowHidden(i, false);

            if (bSectionSeparator) {
                bSectionName = true;
            } else {
                bSectionSeparator = true;
            }

            /*
             * This hides the name of the previous section
             * (and the separator) if this section has no items.
             */
            if (bSectionName && bEmptySection && lastSection > 0) {
                listView->setRowHidden(lastSection, true);
                listView->setRowHidden(lastSection - 1, true);
            }

            // Find the section name
            if (bSectionName) {
                bSectionName = false;
                bSectionSeparator = false;
                bEmptySection = true;
                lastSection = i;
            }
            continue;
        }

        /*
         * Start filtering items.
         */
        KateModeMenuListData::ListItem *item = static_cast<KateModeMenuListData::ListItem *>( listModel->item(i, 0) );

        if (!item->hasMode()) {
            listView->setRowHidden(i, true);
            continue;
        }
        if (!item->getSearchName()) {
            item->generateSearchName( item->getMode()->translatedName.isEmpty() ? &item->getMode()->name : &item->getMode()->translatedName );
        }

        // Only a character is written in the search bar
        if (searchText.length() == 1) {
            if (bIsAlphaOrPointExt) {
                // CASE 1: All the items that start with that character will be displayed.
                if (item->getSearchName()->startsWith(searchText, m_caseSensitivity) ) {
                    setSearchResult(i, bEmptySection, lastSection, lastItem);
                    continue;
                }

                // CASE 2: Matches considering delimiters. For example, when writing "c",
                //         "Objective-C" will be displayed in the results, but not "Yacc/Bison".
                if (QString( QLatin1Char(' ') + *(item->getSearchName()) + QLatin1Char(' ') ).contains( QLatin1Char(' ') + searchText + QLatin1Char(' '), m_caseSensitivity )) {
                    setSearchResult(i, bEmptySection, lastSection, lastItem);
                    continue;
                }
            }
            // CASE 3: The character isn't a letter or number, do an exact search.
            else if ( item->getMode()->nameTranslated().contains(searchText[0], m_caseSensitivity) ) {
                setSearchResult(i, bEmptySection, lastSection, lastItem);
                continue;
            }
        }
        // CASE 4: Search text, using the search name or the normal name.
        else if (!bExactMatch && item->getSearchName()->contains(searchText, m_caseSensitivity)) {
            setSearchResult(i, bEmptySection, lastSection, lastItem);
            continue;
        }
        else if (bExactMatch && item->getMode()->nameTranslated().contains(searchText, m_caseSensitivity)) {
            setSearchResult(i, bEmptySection, lastSection, lastItem);
            continue;
        }

        // CASE 5: Exact matches in extensions.
        if (bSearchExtensions) {
            if (bIsAlphaOrPointExt && item->matchExtension(searchText.mid(1))) {
                setSearchResult(i, bEmptySection, lastSection, lastItem);
                continue;
            }
            else if (item->matchExtension(searchText)) {
                setSearchResult(i, bEmptySection, lastSection, lastItem);
                continue;
            }
        }

        // Item not found, hide
        listView->setRowHidden(i, true);
    }

    // Remove last section name, if it's empty.
    if ( bEmptySection && lastSection > 0 && !listModel->item( listModel->rowCount() - 1, 0 )->text().isEmpty() ) {
        listView->setRowHidden(lastSection, true);
        listView->setRowHidden(lastSection - 1, true);
    }

    listView->scrollToTop();

    // Show message of empty list
    if (lastItem == -1) {
        if (m_parentMenu->m_emptyListMsg == nullptr) {
            m_parentMenu->loadEmptyMsg();
        }
        m_parentMenu->m_scroll->hide();
        m_parentMenu->m_emptyListMsg->show();
    }
    // Hide scroll bar if it isn't necessary
    else if ( listView->visualRect( listModel->index(lastItem, 0) ).bottom() <= listView->geometry().height() ) {
        m_parentMenu->m_scroll->hide();
    }

    m_bSearchStateAutoScroll = true;
}


void KateModeMenuListData::SearchLine::setSearchResult(const int rowItem, bool &bEmptySection, int &lastSection, int &lastItem)
{
    if (lastItem == -1) {
        /*
         * Detect the first result of the search and "select" it.
         * This allows you to scroll through the list using
         * the Up/Down keys after entering a search.
         */
        m_parentMenu->m_list->setCurrentItem(rowItem);
        /*
         * This avoids showing the separator line in the name
         * of the first section, in the search results.
         */
        if (lastSection > 0) {
            m_parentMenu->m_list->setRowHidden(lastSection - 1, true);
        }
    }
    if (bEmptySection) {
        bEmptySection = false;
    }

    lastItem = rowItem;
    if ( m_parentMenu->m_list->isRowHidden(rowItem) ) {
        m_parentMenu->m_list->setRowHidden(rowItem, false);
    }
}


void KateModeMenuList::updateMenu(KTextEditor::Document *doc)
{
    m_doc = static_cast<KTextEditor::DocumentPrivate *>(doc);
}