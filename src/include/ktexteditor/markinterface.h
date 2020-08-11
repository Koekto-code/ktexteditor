/*
    SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>

    Documentation:
    SPDX-FileCopyrightText: 2005 Dominik Haumann <dhdev@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_MARKINTERFACE_H
#define KTEXTEDITOR_MARKINTERFACE_H

#include <ktexteditor_export.h>

#include <QHash>
#include <QObject>

class QIcon;
class QPixmap;
class QPoint;
class QMenu;

namespace KTextEditor
{
class Document;

/**
 * \class Message message.h <KTextEditor/Message>
 *
 * \brief Mark class containing line and mark types.
 *
 * \section mark_intro Introduction
 *
 * The class Mark represents a mark in a Document. It contains the \e line
 * and \e type. A line can have multiple marks, like a \e bookmark and a
 * \e breakpoint, i.e. the \e type contains all marks combined with a logical
 * \e OR (<tt>|</tt>). There are several predefined mark types, look into the
 * MarkInterface for further details.
 *
 * \see KTextEditor::MarkInterface, KTextEditor::Document
 */
class Mark
{
public:
    /** The line that contains the mark. */
    int line;

    /** The mark types in the line, combined with logical OR. */
    uint type;
};

/**
 * \class MarkInterface markinterface.h <KTextEditor/MarkInterface>
 *
 * \brief Mark extension interface for the Document.
 *
 * \ingroup kte_group_doc_extensions
 *
 * \section markext_intro Introduction
 *
 * The MarkInterface provides methods to enable and disable marks in a
 * Document, a marked line can be visualized for example with a shaded
 * background color and/or a pixmap in the iconborder of the Document's View.
 * There are a number of predefined mark types, specified in
 * reservedMarkersCount(). Additionally it is possible to add custom marks
 * and set custom pixmaps.
 *
 * \section markext_access Accessing the Interface
 *
 * The MarkInterface is supposed to be an extension interface for a Document,
 * i.e. the Document inherits the interface \e provided that the
 * KTextEditor library in use implements the interface. Use qobject_cast to access
 * the interface:
 * \code
 * // doc is of type KTextEditor::Document*
 * auto iface = qobject_cast<KTextEditor::MarkInterface*>(doc);
 *
 * if (iface) {
 *     // the implementation supports the interface
 *     // do stuff
 * } else {
 *     // the implementation does not support the interface
 * }
 * \endcode
 *
 * \section markext_handling Handling Marks
 *
 * Get all marks in the document by calling marks(). Use clearMarks() to
 * remove all marks in the entire document. A single mark can be retrieved
 * with mark(). To remove all marks from a line call clearMark(). To add
 * and remove marks from a given line use addMark() and removeMark(). It is
 * also possible to replace all marks with setMark(), i.e. setMark() is the
 * same as a call of clearMark() followed by addMark(). The signals
 * marksChanged() and markChanged() are emitted whenever a line's marks
 * changed.
 *
 * \attention A mark type is represented as an \e uint. An \e uint can have
 *     several mark types combined (see above: logical OR). That means for
 *     all functions/signals with an \e uint parameter, e.g. setMark(),
 *     removeMark(), etc, the \e uint may contain \e multiple marks, i.e.
 *     you can add and remove multiple marks \e simultaneously.
 *
 * \section markext_userdefined User Defined Marks
 *
 * All marks that should be editable by the user can be specified with a mark
 * mask via setEditableMarks(). To set a description and pixmap of a mark type
 * call setMarkDescription() and setMarkPixmap().
 *
 * \see KTextEditor::Document, KTextEditor::Mark
 * \author Christoph Cullmann \<cullmann@kde.org\>
 */
class KTEXTEDITOR_EXPORT MarkInterface
{
public:
    MarkInterface();

    /**
     * Virtual destructor.
     */
    virtual ~MarkInterface();

    //
    // slots !!!
    //
public:
    /**
     * Get all marks set on the \p line.
     * \param line requested line
     * \return a \e uint representing of the marks set in \p line concatenated
     *         by logical OR
     * \see addMark(), removeMark()
     */
    virtual uint mark(int line) = 0;

    /**
     * Set the \p line's mark types to \p markType.
     * If \p line already contains a mark of the given type it has no effect.
     * All other marks are deleted before the mark is set. You can achieve
     * the same by calling
     * \code
     * clearMark(line);
     * addMark(line, markType);
     * \endcode
     * \param line line to set the mark
     * \param markType mark type
     * \see clearMark(), addMark(), mark()
     */
    virtual void setMark(int line, uint markType) = 0;

    /**
     * Clear all marks set in the \p line.
     * \param line line to clear marks
     * \see clearMarks(), removeMark(), addMark()
     */
    virtual void clearMark(int line) = 0;

    /**
     * Add marks of type \p markType to \p line. Existing marks on this line
     * are preserved. If the mark \p markType already is set, nothing
     * happens.
     * \param line line to set the mark
     * \param markType mark type
     * \see removeMark(), setMark()
     */
    virtual void addMark(int line, uint markType) = 0;

    /**
     * Remove the mark mask of type \p markType from \p line.
     * \param line line to remove the mark
     * \param markType mark type to be removed
     * \see clearMark()
     */
    virtual void removeMark(int line, uint markType) = 0;

    /**
     * Get a hash holding all marks in the document.
     * The hash key for a mark is its line.
     * \return a hash holding all marks in the document
     */
    virtual const QHash<int, KTextEditor::Mark *> &marks() = 0;

    /**
     * Clear all marks in the entire document.
     * \see clearMark(), removeMark()
     */
    /// TODO: dominik: add argument unit mask = 0
    virtual void clearMarks() = 0;

    /**
     * Get the number of predefined mark types we have so far.
     * \note FIXME: If you change this you have to make sure katepart
     *              supports the new size!
     * \return number of reserved marker types
     */
    static int reservedMarkersCount()
    {
        return 7;
    }

    /**
     * Predefined mark types.
     *
     * To add a new standard mark type, edit this interface and document
     * the type.
     */
    enum MarkTypes {
        /** Bookmark */
        markType01 = 0x1,
        /** Breakpoint active */
        markType02 = 0x2,
        /** Breakpoint reached */
        markType03 = 0x4,
        /** Breakpoint disabled */
        markType04 = 0x8,
        /** Execution mark */
        markType05 = 0x10,
        /** Warning */
        markType06 = 0x20,
        /** Error */
        markType07 = 0x40,

        markType08 = 0x80,
        markType09 = 0x100,
        markType10 = 0x200,
        markType11 = 0x400,
        markType12 = 0x800,
        markType13 = 0x1000,
        markType14 = 0x2000,
        markType15 = 0x4000,
        markType16 = 0x8000,
        markType17 = 0x10000,
        markType18 = 0x20000,
        markType19 = 0x40000,
        markType20 = 0x80000,
        markType21 = 0x100000,
        markType22 = 0x200000,
        markType23 = 0x400000,
        markType24 = 0x800000,
        markType25 = 0x1000000,
        markType26 = 0x2000000,
        markType27 = 0x4000000,
        markType28 = 0x8000000,
        markType29 = 0x10000000,
        markType30 = 0x20000000,
        markType31 = 0x40000000,
        markType32 = 0x80000000,
        /* reserved marks */
        Bookmark = markType01,
        BreakpointActive = markType02,
        BreakpointReached = markType03,
        BreakpointDisabled = markType04,
        Execution = markType05,
        Warning = markType06,
        Error = markType07,
        SearchMatch = markType32,
    };

    //
    // signals !!!
    //
public:
    /**
     * The \p document emits this signal whenever a mark mask changed.
     * \param document document which emitted this signal
     * \see markChanged()
     */
    virtual void marksChanged(KTextEditor::Document *document) = 0;

    /*
     * Methods to modify mark properties.
     */
public:
    /**
     * Set the \p mark's pixmap to \p pixmap.
     * \param mark mark to which the pixmap will be attached
     * \param pixmap new pixmap
     * \see setMarkDescription()
     */
    virtual void setMarkPixmap(MarkTypes mark, const QPixmap &pixmap) = 0;

    /**
     * Get the \p mark's pixmap.
     * \param mark mark type. If the pixmap does not exist the resulting is null
     *        (check with QPixmap::isNull()).
     * \see setMarkDescription()
     */
    virtual QPixmap markPixmap(MarkTypes mark) const = 0;

    /**
     * Set the \p mark's description to \p text.
     * \param mark mark to set the description
     * \param text new descriptive text
     * \see markDescription(), setMarkPixmap()
     */
    virtual void setMarkDescription(MarkTypes mark, const QString &text) = 0;

    /**
     * Get the \p mark's description to text.
     * \param mark mark to set the description
     * \return text of the given \p mark or QString(), if the entry does not
     *         exist
     * \see setMarkDescription(), setMarkPixmap()
     */
    virtual QString markDescription(MarkTypes mark) const = 0;

    /**
     * Set the mark mask the user is allowed to toggle to \p markMask.
     * I.e. concatenate all editable marks with a logical OR. If the user should
     * be able to add a bookmark and set a breakpoint with the context menu in
     * the icon pane, you have to call
     * \code
     * // iface is of Type KTextEditor::MarkInterface*
     * // only make bookmark and breakpoint editable
     * iface->setEditableMarks( MarkInterface::Bookmark |
     *                          MarkInterface::BreakpointActive );
     *
     * // or preserve last settings, and add bookmark and breakpoint
     * iface->setEditableMarks( iface->editableMarks() |
     *                          MarkInterface::Bookmark |
     *                          MarkInterface::BreakpointActive );
     * \endcode
     * \param markMask bitmap pattern
     * \see editableMarks(), setMarkPixmap(), setMarkDescription()
     */
    virtual void setEditableMarks(uint markMask) = 0;

    /**
     * Get, which marks can be toggled by the user.
     * The returned value is a mark mask containing all editable marks combined
     * with a logical OR.
     * \return mark mask containing all editable marks
     * \see setEditableMarks()
     */
    virtual uint editableMarks() const = 0;

    /**
     * Possible actions on a mark.
     * \see markChanged()
     */
    enum MarkChangeAction {
        MarkAdded = 0,  /**< action: a mark was added.  */
        MarkRemoved = 1 /**< action: a mark was removed. */
    };

    //
    // signals !!!
    //
public:
    /**
     * The \p document emits this signal whenever the \p mark changes.
     * \param document the document which emitted the signal
     * \param mark changed mark
     * \param action action, either removed or added
     * \see marksChanged()
     */
    virtual void markChanged(KTextEditor::Document *document, KTextEditor::Mark mark, KTextEditor::MarkInterface::MarkChangeAction action) = 0;

Q_SIGNALS:

    /**
     * The \p document emits this signal whenever the \p mark is hovered using the mouse,
     * and the receiver may show a tooltip.
     * \param document the document which emitted the signal
     * \param mark mark that was hovered
     * \param position mouse position during the hovering
     * \param handled set this to 'true' if this event was handled externally
     */
    void markToolTipRequested(KTextEditor::Document *document, KTextEditor::Mark mark, QPoint position, bool &handled);

    /**
     * The \p document emits this signal whenever the \p mark is right-clicked to show a context menu.
     * The receiver may show an own context menu instead of the kate internal one.
     * \param document the document which emitted the signal
     * \param mark mark that was right-clicked
     * \param pos position where the menu should be started
     * \param handled set this to 'true' if this event was handled externally, and kate should not create an own context menu.
     */
    void markContextMenuRequested(KTextEditor::Document *document, KTextEditor::Mark mark, QPoint pos, bool &handled);

    /**
     * The \p document emits this signal whenever the \p mark is left-clicked.
     * \param document the document which emitted the signal
     * \param mark mark that was right-clicked
     * \param handled set this to 'true' if this event was handled externally, and kate should not do own handling of the left click.
     */
    void markClicked(KTextEditor::Document *document, KTextEditor::Mark mark, bool &handled);

private:
    class MarkInterfacePrivate *const d = nullptr;
};

/**
 * \brief Mark extension interface for the Document, version 2
 *
 * \ingroup kte_group_doc_extensions
 *
 * \section markextv2_intro Introduction
 *
 * The MarkInterfaceV2 allows to do the same as MarkInterface
 * and additionally
 * - (1) set an icon for a mark type instead of just a pixmap
 *
 * \section markextv2_access Accessing the Interface
 *
 * The MarkInterfaceV2 is supposed to be an extension interface for a Document,
 * i.e. the Document inherits the interface \e provided that the
 * KTextEditor library in use implements the interface. Use qobject_cast to access
 * the interface:
 * \code
 * // doc is of type KTextEditor::Document*
 * auto iface = qobject_cast<KTextEditor::MarkInterfaceV2*>(doc);
 *
 * if (iface) {
 *     // the implementation supports the interface
 *     // do stuff
 * } else {
 *     // the implementation does not support the interface
 * }
 * \endcode
 *
 * \since 5.69
 */
class KTEXTEDITOR_EXPORT MarkInterfaceV2 : public MarkInterface
{
    // KF6: Merge KTextEditor::MarkInterfaceV2 into KTextEditor::MarkInterface, drop QPixmap API (kossebau)
public:
    virtual ~MarkInterfaceV2()
    {
    }

    /**
     * Set the \p mark's icon to \p icon.
     * \param markType mark type to which the icon will be attached
     * \param icon new icon
     * \see setMarkDescription()
     */
    virtual void setMarkIcon(MarkTypes markType, const QIcon &icon) = 0;

    /**
     * Get the \p mark's icon.
     * \param markType mark type. If the icon does not exist the resulting is null
     *        (check with QIcon::isNull()).
     * \see setMarkDescription()
     */
    virtual QIcon markIcon(MarkTypes markType) const = 0;
};

}

Q_DECLARE_INTERFACE(KTextEditor::MarkInterface, "org.kde.KTextEditor.MarkInterface")
Q_DECLARE_INTERFACE(KTextEditor::MarkInterfaceV2, "org.kde.KTextEditor.MarkInterfaceV2")

#endif
