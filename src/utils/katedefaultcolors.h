/*
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_DEFAULTCOLORS_H
#define KATE_DEFAULTCOLORS_H

#include <KColorScheme>

namespace Kate
{
enum ColorRole {
    // editor backgrounds
    Background,
    SelectionBackground,
    HighlightedLineBackground,
    SearchHighlight,
    ReplaceHighlight,
    // text decorations
    HighlightedBracket,
    TabMarker,
    IndentationLine,
    SpellingMistakeLine,
    // icon border
    WordWrapMarker,
    IconBar,
    CodeFolding,
    LineNumber,
    CurrentLineNumber,
    Separator,
    ModifiedLine,
    SavedLine,
    // templates
    TemplateBackground,
    TemplateFocusedEditablePlaceholder,
    TemplateEditablePlaceholder,
    TemplateNotEditablePlaceholder
};

enum Mark {
    Bookmark = 0,
    ActiveBreakpoint,
    ReachedBreakpoint,
    DisabledBreakpoint,
    Execution,
    Warning,
    Error,

    FIRST_MARK = Bookmark,
    LAST_MARK = Error
};

}

class KateDefaultColors
{
public:
    KateDefaultColors();

    const KColorScheme &view() const
    {
        return m_view;
    }

    const KColorScheme &window() const
    {
        return m_window;
    }

    const KColorScheme &selection() const
    {
        return m_selection;
    }

    const KColorScheme &inactiveSelection() const
    {
        return m_inactiveSelection;
    }

    QColor color(Kate::ColorRole color) const;
    QColor mark(Kate::Mark mark) const;
    QColor mark(int mark) const;

    enum ColorType { ForegroundColor, BackgroundColor };
    QColor adaptToScheme(const QColor &color, ColorType type) const;

private:
    const KColorScheme m_view;
    const KColorScheme m_window;
    const KColorScheme m_selection;
    const KColorScheme m_inactiveSelection;
    const QColor m_background;
    const QColor m_foreground;
    const qreal m_backgroundLuma;
    const qreal m_foregroundLuma;
};

#endif // KATE_DEFAULTCOLORS_H
