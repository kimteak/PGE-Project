/*
 * Platformer Game Engine by Wohlstand, a free platform for game making
 * Copyright (c) 2014-2019 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common_features/main_window_ptr.h>
#include <common_features/file_keeper.h>
#include <main_window/global_settings.h>
#include <PGE_File_Formats/file_formats.h>
#include <PGE_File_Formats/pge_x.h>

#include "npcedit.h"
#include <ui_npcedit.h>

static int FileName_to_npcID(QString filename)
{
    QStringList tmp = filename.split(QChar('-'));

    if((tmp.size()==2) && (PGEFile::IsIntU(tmp[1])))
        return tmp[1].toInt();

    return 0;
}

void NpcEdit::newFile(unsigned long npcID)
{
    npc_id = npcID;
    static int sequenceNumber = 1;
    ui->CurrentNPCID->setText( QString::number(npcID) );

    m_isUntitled = true;

    curFile = QString("npc-%1.txt").arg(npcID);

    setWindowTitle(curFile + QString(" [*] (%1)").arg(sequenceNumber++));

    setDefaultData(npcID);

    NpcData = DefaultNPCData; // create data template
    StartNPCData = DefaultNPCData;
    setDataBoxes();

    documentWasModified();

    loadPreview();
    /*connect(document(), SIGNAL(contentsChanged()),
            this, SLOT(documentWasModified()));*/
}

bool NpcEdit::loadFile(const QString &fileName, NPCConfigFile FileData)
{
    QFile file(fileName);
    NpcData = FileData;
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Load file error"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QFileInfo fileI(fileName);

    //Get NPC-ID from FileName
    npc_id = FileName_to_npcID(fileI.baseName());
    setDefaultData(npc_id);
    ui->CurrentNPCID->setText( QString::number(npc_id) );

    StartNPCData = NpcData; //Save current history for made reset
    setDataBoxes();

    setCurrentFile(fileName);
    documentNotModified();

    loadPreview();
    on_DirectLeft_clicked();

    return true;
}

bool NpcEdit::save(bool savOptionsDialog)
{
    if (m_isUntitled) {
        return saveAs(savOptionsDialog);
    } else {
        return saveFile(curFile);
    }
}

bool NpcEdit::saveAs(bool savOptionsDialog)
{
    Q_UNUSED(savOptionsDialog);
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
      (m_isUntitled)?GlobalSettings::savePath_npctxt+QString("/")+curFile:curFile, tr("SMBX custom NPC config file (npc-*.txt)"));
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

bool NpcEdit::trySave()
{
    return maybeSave();
}

bool NpcEdit::saveFile(const QString &fileName, const bool addToRecent)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if(!FileFormats::WriteNPCTxtFileF(fileName, NpcData))
    {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(this, tr("File save error"),
                             tr("Cannot save file %1:\n%2.")
                             .arg(fileName)
                             .arg(NpcData.errorString));
        return false;
    }

    FileKeeper fileKeeper = FileKeeper(fileName);
    if(!fileKeeper.isValid())
    {
        QMessageBox::warning(this, tr("File save error"),
                             tr("Cannot save file %1:\n%2.")
                                     .arg(fileName)
                                     .arg(tr("Can't create a temporary backup file")));
        return false;
    }

    GlobalSettings::savePath_npctxt = QFileInfo(fileName).path();

    QFileInfo fileI(fileName);
    unsigned long old_npc_id = npc_id;
    npc_id = static_cast<unsigned long>(FileName_to_npcID(fileI.baseName()));
    setDefaultData(npc_id);
    ui->CurrentNPCID->setText( QString::number(npc_id) );

    QApplication::restoreOverrideCursor();
    setCurrentFile(fileName);

    documentNotModified();

    if(old_npc_id == npc_id)
    {
        refreshImageFile();
        updatePreview();
    }
    else
    {
        loadPreview();
    }

    if(addToRecent)
    {
        MainWinConnect::pMainWin->AddToRecentFiles(fileName);
        MainWinConnect::pMainWin->SyncRecentFiles();
    }

    // Remove temporary backup file
    fileKeeper.remove();

    return true;
}



bool NpcEdit::maybeSave()
{
    if (m_isModyfied) {
    QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, userFriendlyCurrentFile()+tr(" not saved"),
                     tr("'%1' has been modified.\n"
                        "Do you want to save your changes?")
                     .arg(userFriendlyCurrentFile()),
                     QMessageBox::Save | QMessageBox::Discard
             | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }

    return true;
}


void NpcEdit::documentWasModified()
{
    QFont font;
    font.setWeight( QFont::Bold );
    m_isModyfied = true;
    setWindowTitle(userFriendlyCurrentFile() + "[*]");
    ui->isModyfiedL->setText("Yes");
    ui->isModyfiedL->setFont( font );
}

void NpcEdit::documentNotModified()
{
    QFont font;
    font.setWeight( QFont::Normal );

    m_isModyfied = false;
    setWindowTitle(userFriendlyCurrentFile());
    ui->isModyfiedL->setText("No");
    ui->isModyfiedL->setFont( font );
}

void NpcEdit::setCurrentFile(const QString &fileName)
{
    curFile = QFileInfo(fileName).canonicalFilePath();
    m_isUntitled = false;
    //document()->setModified(false);
    setWindowModified(false);
    documentWasModified();
    setWindowTitle(userFriendlyCurrentFile() + "[*]");
}



QString NpcEdit::userFriendlyCurrentFile()
{
    return strippedName(curFile);
}

void NpcEdit::makeCrashState()
{
    this->m_isUntitled = true;
    this->m_isModyfied = true; //just in case
}

bool NpcEdit::isUntitled()
{
    return m_isUntitled;
}

bool NpcEdit::isModified()
{
    return m_isModyfied;
}

void NpcEdit::markForForceClose()
{
    m_isUntitled = false;
    m_isModyfied = false;
}

QString NpcEdit::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}
