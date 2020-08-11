/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "registers.h"
#include "katepartdebug.h"

#include <KConfigGroup>

#include <QApplication>
#include <QClipboard>

using namespace KateVi;

Registers::Registers()
{
}

Registers::~Registers()
{
}

void Registers::readConfig(const KConfigGroup &config)
{
    const QStringList names = config.readEntry("ViRegisterNames", QStringList());
    const QStringList contents = config.readEntry("ViRegisterContents", QStringList());
    const QList<int> flags = config.readEntry("ViRegisterFlags", QList<int>());

    if (names.size() != contents.size() || contents.size() != flags.size()) {
        return;
    }

    for (int i = 0; i < names.size(); i++) {
        if (!names.at(i).isEmpty()) {
            set(names.at(i).at(0), contents.at(i), (OperationMode)(flags.at(i)));
        }
    }
}

void Registers::writeConfig(KConfigGroup &config) const
{
    if (m_registers.isEmpty()) {
        return;
    }

    QStringList names, contents;
    QList<int> flags;
    QMap<QChar, Register>::const_iterator i;
    for (i = m_registers.constBegin(); i != m_registers.constEnd(); ++i) {
        if (i.value().first.length() <= 1000) {
            names << i.key();
            contents << i.value().first;
            flags << int(i.value().second);
        } else {
            qCDebug(LOG_KTE) << "Did not save contents of register " << i.key() << ": contents too long (" << i.value().first.length() << " characters)";
        }
    }

    config.writeEntry("ViRegisterNames", names);
    config.writeEntry("ViRegisterContents", contents);
    config.writeEntry("ViRegisterFlags", flags);
}

void Registers::setInsertStopped(const QString &text)
{
    set(InsertStoppedRegister, text);
}

void Registers::set(const QChar &reg, const QString &text, OperationMode flag, bool append)
{
    if (reg == BlackHoleRegister) {
        return;
    }

    if (reg >= FirstNumberedRegister && reg <= LastNumberedRegister) { // "kill ring" registers
        setNumberedRegister(text);
    } else if (reg == SystemClipboardRegister) {
        QApplication::clipboard()->setText(text, QClipboard::Clipboard);
    } else if (reg == SystemSelectionRegister) {
        QApplication::clipboard()->setText(text, QClipboard::Selection);
    } else {
        if (append) {
            m_registers[reg].first.append(text);
        } else {
            m_registers.insert(reg, Register(text, flag));
        }
    }

    if (reg == ZeroRegister || reg == FirstNumberedRegister || reg == SmallDeleteRegister) {
        m_default = reg;
    }
}

QString Registers::getContent(const QChar &reg) const
{
    return getRegister(reg).first;
}

OperationMode Registers::getFlag(const QChar &reg) const
{
    return getRegister(reg).second;
}

Registers::Register Registers::getRegister(const QChar &reg) const
{
    Register regPair;
    QChar _reg = (reg != UnnamedRegister ? reg : m_default);

    if (_reg >= FirstNumberedRegister && _reg <= LastNumberedRegister) {
        int index = QString(_reg).toInt() - 1;
        if (m_numbered.size() > index) {
            regPair = m_numbered.at(index);
        }
    } else if (_reg == SystemClipboardRegister) {
        QString regContent = QApplication::clipboard()->text(QClipboard::Clipboard);
        regPair = Register(regContent, CharWise);
    } else if (_reg == SystemSelectionRegister) {
        QString regContent = QApplication::clipboard()->text(QClipboard::Selection);
        regPair = Register(regContent, CharWise);
    } else {
        if (m_registers.contains(_reg)) {
            regPair = m_registers.value(_reg);
        }
    }

    return regPair;
}

void Registers::setNumberedRegister(const QString &text, OperationMode flag)
{
    if (m_numbered.size() == 9) {
        m_numbered.removeLast();
    }

    // register 0 is used for the last yank command, so insert at position 1
    m_numbered.prepend(Register(text, flag));
}
