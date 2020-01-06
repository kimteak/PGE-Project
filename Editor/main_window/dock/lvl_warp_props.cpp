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

#include <QInputDialog>
#include <QStandardItemModel>

#include <editing/_scenes/level/lvl_history_manager.h>
#include <editing/_scenes/level/lvl_item_placing.h>
#include <editing/_dialogs/levelfilelist.h>
#include <editing/_dialogs/wld_setpoint.h>
#include <PGE_File_Formats/file_formats.h>
#include <main_window/dock/lvl_item_properties.h>
#include <common_features/direction_switch_widget.h>

#include <ui_mainwindow.h>
#include <mainwindow.h>

#include "lvl_warp_props.h"
#include "ui_lvl_warp_props.h"

void MainWindow::on_actionWarpsAndDoors_triggered(bool checked)
{
    dock_LvlWarpProps->setVisible(checked);
    if(checked) dock_LvlWarpProps->raise();
}


LvlWarpBox::LvlWarpBox(QWidget *parent) :
    QDockWidget(parent),
    MWDock_Base(parent),
    ui(new Ui::LvlWarpBox)
{
    setVisible(false);
    setAttribute(Qt::WA_ShowWithoutActivating);
    ui->setupUi(this);
    lockWarpSetSettings = false;

    ui->entrance->setPixmap(DirectionSwitchWidget::S_CENTER, QPixmap(":/arrows/arrows/box.png"));
    ui->entrance->setPixmap(DirectionSwitchWidget::S_LEFT, QPixmap(":/arrows/arrows/black_right.png"));
    ui->entrance->setPixmap(DirectionSwitchWidget::S_RIGHT, QPixmap(":/arrows/arrows/black_left.png"));
    ui->entrance->setPixmap(DirectionSwitchWidget::S_TOP, QPixmap(":/arrows/arrows/black_down.png"));
    ui->entrance->setPixmap(DirectionSwitchWidget::S_BOTTOM, QPixmap(":/arrows/arrows/black_up.png"));
    ui->entrance->setPixmapOn(DirectionSwitchWidget::S_LEFT, QPixmap(":/arrows/arrows/red_right.png"));
    ui->entrance->setPixmapOn(DirectionSwitchWidget::S_RIGHT, QPixmap(":/arrows/arrows/red_left.png"));
    ui->entrance->setPixmapOn(DirectionSwitchWidget::S_TOP, QPixmap(":/arrows/arrows/red_down.png"));
    ui->entrance->setPixmapOn(DirectionSwitchWidget::S_BOTTOM, QPixmap(":/arrows/arrows/red_up.png"));
    ui->entrance->mapValue(DirectionSwitchWidget::S_LEFT, LevelDoor::ENTRANCE_RIGHT);
    ui->entrance->mapValue(DirectionSwitchWidget::S_RIGHT, LevelDoor::ENTRANCE_LEFT);
    ui->entrance->mapValue(DirectionSwitchWidget::S_TOP, LevelDoor::ENTRANCE_DOWN);
    ui->entrance->mapValue(DirectionSwitchWidget::S_BOTTOM, LevelDoor::ENTRANCE_UP);

    ui->exit->setPixmap(DirectionSwitchWidget::S_CENTER, QPixmap(":/arrows/arrows/box.png"));
    ui->exit->setPixmap(DirectionSwitchWidget::S_LEFT, QPixmap(":/arrows/arrows/black_left.png"));
    ui->exit->setPixmap(DirectionSwitchWidget::S_RIGHT, QPixmap(":/arrows/arrows/black_right.png"));
    ui->exit->setPixmap(DirectionSwitchWidget::S_TOP, QPixmap(":/arrows/arrows/black_up.png"));
    ui->exit->setPixmap(DirectionSwitchWidget::S_BOTTOM, QPixmap(":/arrows/arrows/black_down.png"));
    ui->exit->setPixmapOn(DirectionSwitchWidget::S_LEFT, QPixmap(":/arrows/arrows/green_left.png"));
    ui->exit->setPixmapOn(DirectionSwitchWidget::S_RIGHT, QPixmap(":/arrows/arrows/green_right.png"));
    ui->exit->setPixmapOn(DirectionSwitchWidget::S_TOP, QPixmap(":/arrows/arrows/green_up.png"));
    ui->exit->setPixmapOn(DirectionSwitchWidget::S_BOTTOM, QPixmap(":/arrows/arrows/green_down.png"));
    ui->exit->mapValue(DirectionSwitchWidget::S_LEFT, LevelDoor::EXIT_LEFT);
    ui->exit->mapValue(DirectionSwitchWidget::S_RIGHT, LevelDoor::EXIT_RIGHT);
    ui->exit->mapValue(DirectionSwitchWidget::S_TOP, LevelDoor::EXIT_UP);
    ui->exit->mapValue(DirectionSwitchWidget::S_BOTTOM, LevelDoor::EXIT_DOWN);

    re_translate_widgets();

    QRect mwg = mw()->geometry();
    int GOffset = 10;
    mw()->addDockWidget(Qt::RightDockWidgetArea, this);
    connect(mw(), SIGNAL(languageSwitched()), this, SLOT(re_translate()));
    connect(this, SIGNAL(visibilityChanged(bool)), mw()->ui->actionWarpsAndDoors, SLOT(setChecked(bool)));
    setFloating(true);
    setGeometry(
        mwg.right() - width() - GOffset,
        mwg.y() + 120,
        width(),
        height()
    );

    connect(ui->entrance, &DirectionSwitchWidget::clicked, this, &LvlWarpBox::entrance_clicked);
    connect(ui->exit, &DirectionSwitchWidget::clicked, this, &LvlWarpBox::exit_clicked);

    connect(mw(), SIGNAL(setSMBX64Strict(bool)),
            this, SLOT(setSMBX64Strict(bool)));

    m_lastVisibilityState = isVisible();
    mw()->docks_level.
    addState(this, &m_lastVisibilityState);
}

LvlWarpBox::~LvlWarpBox()
{
    delete ui;
}

void LvlWarpBox::re_translate_widgets()
{
    ui->entrance->mapToolTip(DirectionSwitchWidget::S_LEFT, tr("Right"));
    ui->entrance->mapToolTip(DirectionSwitchWidget::S_RIGHT, tr("Left"));
    ui->entrance->mapToolTip(DirectionSwitchWidget::S_TOP, tr("Down"));
    ui->entrance->mapToolTip(DirectionSwitchWidget::S_BOTTOM, tr("Up"));
    ui->exit->mapToolTip(DirectionSwitchWidget::S_LEFT, tr("Left"));
    ui->exit->mapToolTip(DirectionSwitchWidget::S_RIGHT, tr("Right"));
    ui->exit->mapToolTip(DirectionSwitchWidget::S_TOP, tr("Up"));
    ui->exit->mapToolTip(DirectionSwitchWidget::S_BOTTOM, tr("Down"));
}

QComboBox *LvlWarpBox::cbox_layer()
{
    return ui->WarpLayer;
}

QComboBox *LvlWarpBox::cbox_event_enter()
{
    return ui->WarpEnterEvent;
}

void LvlWarpBox::setSMBX64Strict(bool en)
{
    dataconfigs &c = mw()->configs;
    bool shown = !en;

    {
        auto* model = qobject_cast<QStandardItemModel*>(ui->WarpType->model());
        QStandardItem *item = model->item(3);//Portal warp item
        if(c.editor.supported_features.level_warp_portal == EditorSetup::FeaturesSupport::F_ENABLED)
            item->setFlags(item->flags() | Qt::ItemIsEnabled);
        else
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
    }

    // Disable/Hide all non-supported in SMBX 1.3 parameters
    ui->WarpTwoWay->setEnabled(shown && (c.editor.supported_features.level_warp_two_way == EditorSetup::FeaturesSupport::F_ENABLED));
    ui->WarpTwoWay->setHidden(c.editor.supported_features.level_warp_two_way == EditorSetup::FeaturesSupport::F_HIDDEN);
    ui->WarpNeedAStarsMsg->setEnabled(shown && (c.editor.supported_features.level_warp_needstars_message == EditorSetup::FeaturesSupport::F_ENABLED));
    ui->WarpNeedAStarsMsg->setHidden(c.editor.supported_features.level_warp_needstars_message == EditorSetup::FeaturesSupport::F_HIDDEN);
    ui->WarpNeedAStarsMsg_label->setEnabled(shown && (c.editor.supported_features.level_warp_needstars_message == EditorSetup::FeaturesSupport::F_ENABLED));
    ui->WarpNeedAStarsMsg_label->setHidden(c.editor.supported_features.level_warp_needstars_message == EditorSetup::FeaturesSupport::F_HIDDEN);
    ui->WarpHideStars->setEnabled(shown && (c.editor.supported_features.level_warp_hide_stars == EditorSetup::FeaturesSupport::F_ENABLED));
    ui->WarpHideStars->setHidden(c.editor.supported_features.level_warp_hide_stars == EditorSetup::FeaturesSupport::F_HIDDEN);
    ui->WarpBombNeed->setEnabled(shown && (c.editor.supported_features.level_warp_bomb_exit == EditorSetup::FeaturesSupport::F_ENABLED));
    ui->WarpBombNeed->setHidden(c.editor.supported_features.level_warp_bomb_exit == EditorSetup::FeaturesSupport::F_HIDDEN);
    ui->WarpHideLevelEnterScreen->setEnabled(shown && (c.editor.supported_features.level_warp_hide_interlevel_scene == EditorSetup::FeaturesSupport::F_ENABLED));
    ui->WarpHideLevelEnterScreen->setHidden(c.editor.supported_features.level_warp_hide_interlevel_scene == EditorSetup::FeaturesSupport::F_HIDDEN);

    ui->WarpAllowNPC_IL->setEnabled(shown && (c.editor.supported_features.level_warp_allow_interlevel_npc == EditorSetup::FeaturesSupport::F_ENABLED));
    ui->WarpAllowNPC_IL->setHidden(c.editor.supported_features.level_warp_allow_interlevel_npc == EditorSetup::FeaturesSupport::F_HIDDEN);
    ui->WarpSpecialStateOnly->setEnabled(shown && (c.editor.supported_features.level_warp_allow_sp_state_only == EditorSetup::FeaturesSupport::F_ENABLED));
    ui->WarpSpecialStateOnly->setHidden(c.editor.supported_features.level_warp_allow_sp_state_only == EditorSetup::FeaturesSupport::F_HIDDEN);

    ui->warpBoxCannon->setEnabled(shown && (c.editor.supported_features.level_warp_cannon_exit == EditorSetup::FeaturesSupport::F_ENABLED));
    ui->warpBoxCannon->setHidden(c.editor.supported_features.level_warp_cannon_exit == EditorSetup::FeaturesSupport::F_HIDDEN);
    ui->WarpEnableCannon->setDisabled(en);
    ui->WarpCannonSpeed->setDisabled(en);

    ui->warpBoxEnterEvent->setEnabled(shown && (c.editor.supported_features.level_warp_on_enter_event == EditorSetup::FeaturesSupport::F_ENABLED));
    ui->warpBoxEnterEvent->setHidden(c.editor.supported_features.level_warp_on_enter_event == EditorSetup::FeaturesSupport::F_HIDDEN);
    ui->WarpEnterEvent->setDisabled(en);
    ui->WarpEnterEvent_label->setDisabled(en);
}

void LvlWarpBox::re_translate()
{
    lockWarpSetSettings = true;
    int doorType = ui->WarpType->currentIndex(); //backup combobox's index
    ui->retranslateUi(this);
    re_translate_widgets();
    ui->WarpType->setCurrentIndex(doorType); //restore combobox's index
    lockWarpSetSettings = false;
}

void LvlWarpBox::init()
{
    int winType = mw()->activeChildWindow();
    if(winType == MainWindow::WND_Level)
    {
        ui->WarpList->clear();
        for(LevelDoor &door : mw()->activeLvlEditWin()->LvlData.doors)
            ui->WarpList->addItem(doorTitle(door), door.meta.array_id);

        if(ui->WarpList->count() <= 0)
            setDoorData(-1);
        else
            setDoorData(ui->WarpList->currentIndex());
    }
}

QString LvlWarpBox::doorTitle(LevelDoor &door)
{
    return QString("%1: x%2y%3 <=> x%4y%5")
           .arg(door.meta.array_id).arg(door.ix).arg(door.iy).arg(door.ox).arg(door.oy);
}


void LvlWarpBox::SwitchToDoor(long arrayID)
{
    int WinType = mw()->activeChildWindow();
    if(WinType == MainWindow::WND_Level)
    {
        show();
        raise();
        ui->WarpList->setCurrentIndex(ui->WarpList->findData(QString::number(arrayID)));
    }

}


void LvlWarpBox::setDoorData(long index)
{
    long cIndex;
    if(index == -2)
        cIndex = ui->WarpList->currentIndex();
    else
        cIndex = index;

    lockWarpSetSettings = true;

    int WinType = mw()->activeChildWindow();

    qDebug() << "Current warp indes is " << ui->WarpList->currentIndex();
    qDebug() << "Activated windows type is " << WinType;

    if(WinType == MainWindow::WND_Level)
    {
        LevelEdit *le = mw()->activeLvlEditWin();
        int warpId = ui->WarpList->currentData().toInt();
        if((le->LvlData.doors.size() > 0) && (cIndex < le->LvlData.doors.size()))
        {
            for(LevelDoor &door : le->LvlData.doors)
            {
                if(door.meta.array_id == static_cast<unsigned long>(warpId))
                {
                    ui->WarpList->setItemText(static_cast<int>(cIndex), doorTitle(door));
                    ui->WarpRemove->setEnabled(true);

                    ui->warpBoxDirect->setEnabled(true);
                    ui->warpBoxMain->setEnabled(true);
                    ui->warpBoxWD->setEnabled(true);
                    ui->warpBoxWTL->setEnabled(true);
                    ui->warpBoxWTW->setEnabled(true);
                    ui->warpBoxCannon->setEnabled(true);

                    ui->WarpTwoWay->setChecked(door.two_way);

                    ui->WarpAllowNPC->setChecked(door.allownpc);
                    ui->WarpLock->setChecked(door.locked);
                    ui->WarpNoVehicles->setChecked(door.novehicles);
                    ui->WarpType->setCurrentIndex(door.type);

                    ui->WarpEntrancePlaced->setChecked(door.isSetIn);
                    ui->WarpExitPlaced->setChecked(door.isSetOut);

                    ui->WarpNeedAStars->setValue(door.stars);
                    ui->WarpNeedAStarsMsg->setText(door.stars_msg);
                    ui->WarpHideStars->setChecked(door.star_num_hide);
                    ui->WarpBombNeed->setChecked(door.need_a_bomb);
                    ui->WarpSpecialStateOnly->setChecked(door.special_state_required);

                    ui->WarpEntranceGrp->setEnabled(door.type == 1);
                    ui->WarpExitGrp->setEnabled(door.type == 1);

                    ui->WarpLayer->setCurrentText(door.layer);
                    if(ui->WarpLayer->currentIndex() < 0)
                        ui->WarpLayer->setCurrentIndex(0);

                    if(door.event_enter.isEmpty())
                    {
                        ui->WarpEnterEvent->setCurrentIndex(0);
                    }
                    else
                    {
                        ui->WarpEnterEvent->setCurrentText(door.event_enter);
                        if(ui->WarpEnterEvent->currentIndex() < 0)
                            ui->WarpEnterEvent->setCurrentIndex(0);
                    }

                    ui->entrance->setDirection(door.idirect);
                    ui->exit->setDirection(door.odirect);

                    ui->WarpEnableCannon->setChecked(door.cannon_exit);
                    ui->WarpCannonSpeed->setEnabled(door.cannon_exit && !le->LvlData.meta.smbx64strict);
                    ui->WarpCannonSpeed->setValue(door.cannon_exit_speed);

                    ui->WarpToMapX->setText((door.world_x != -1) ? QString::number(door.world_x) : "");
                    ui->WarpToMapY->setText((door.world_y != -1) ? QString::number(door.world_y) : "");

                    ui->WarpLevelExit->setChecked(door.lvl_o);
                    ui->WarpLevelEntrance->setChecked(door.lvl_i);

                    ui->WarpSetEntrance->setEnabled(((!door.lvl_o) && (!door.lvl_i)) || ((door.lvl_o) && (!door.lvl_i)));
                    ui->WarpSetExit->setEnabled(((!door.lvl_o) && (!door.lvl_i)) || ((door.lvl_i)));

                    ui->WarpLevelFile->setText(door.lname);
                    ui->WarpToExitNu->setValue(static_cast<int>(door.warpto));

                    ui->WarpHideLevelEnterScreen->setChecked(door.hide_entering_scene);
                    ui->WarpAllowNPC_IL->setChecked(door.allownpc_interlevel);
                    break;
                }
            }
        }
        else
        {
            ui->WarpRemove->setEnabled(false);

            ui->warpBoxDirect->setEnabled(false);
            ui->warpBoxMain->setEnabled(false);
            ui->warpBoxWD->setEnabled(false);
            ui->warpBoxWTL->setEnabled(false);
            ui->warpBoxWTW->setEnabled(false);
            ui->warpBoxCannon->setEnabled(false);
        }
    }
    lockWarpSetSettings = false;
}



///////////////////////////////////////////////////////////////
///////////////////// Door Control/////////////////////////////
///////////////////////////////////////////////////////////////
void LvlWarpBox::on_WarpList_currentIndexChanged(int index)
{
    setDoorData(index);
}

void LvlWarpBox::on_WarpAdd_clicked()
{
    int WinType = mw()->activeChildWindow();

    if(WinType == MainWindow::WND_Level)
    {
        LevelEdit *edit = mw()->activeLvlEditWin();
        LevelDoor newDoor = FileFormats::CreateLvlWarp();
        newDoor.meta.array_id = edit->LvlData.doors_array_id++;
        newDoor.meta.index = static_cast<unsigned int>(edit->LvlData.doors.size());
        newDoor.type = GlobalSettings::LvlItemDefaults.warp_type;//Apply initial warp type value
        edit->LvlData.doors.push_back(newDoor);
        edit->LvlData.meta.modified = true;

        edit->scene->m_history->addAddWarp(static_cast<int>(newDoor.meta.array_id),
                                           ui->WarpList->count(),
                                           static_cast<int>(newDoor.meta.index));

        ui->WarpList->addItem(doorTitle(newDoor), newDoor.meta.array_id);
        ui->WarpList->setCurrentIndex(ui->WarpList->count() - 1);
        ui->WarpRemove->setEnabled(true);
    }

}
void LvlWarpBox::on_WarpRemove_clicked()
{
    int WinType = mw()->activeChildWindow();
    if(WinType == MainWindow::WND_Level)
    {
        LevelEdit *edit = mw()->activeLvlEditWin();

        for(int i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                edit->scene->m_history->addRemoveWarp(edit->LvlData.doors[i]);
                break;
            }
        }
        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt(), true);

        for(int i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                edit->LvlData.doors.removeAt(i);
                break;
            }
        }

        ui->WarpList->removeItem(ui->WarpList->currentIndex());

        if(ui->WarpList->count() <= 0)
            setWarpRemoveButtonEnabled(false);

        edit->LvlData.meta.modified = true;
    }

}

void LvlWarpBox::on_WarpSetEntrance_clicked()
{
    //placeDoorEntrance
    if(mw()->activeChildWindow() == MainWindow::WND_Level)
    {
        LevelEdit *edit = mw()->activeLvlEditWin();
        bool placed = false;
        int i = 0;
        int array_id = 0;
        for(i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                placed = edit->LvlData.doors[i].isSetIn;
                array_id = edit->LvlData.doors[i].meta.array_id;
                break;
            }
        }

        if(placed)
        {
            edit->goTo(edit->LvlData.doors[i].ix, edit->LvlData.doors[i].iy, true, QPoint(0, 0), true);
            //deselect all and select placed one
            foreach(QGraphicsItem *i, edit->scene->selectedItems())
            {
                i->setSelected(false);
            }
            foreach(QGraphicsItem *item, edit->scene->items())
            {
                if(item->data(ITEM_TYPE).toString() == "Door_enter")
                {
                    if(item->data(ITEM_ARRAY_ID).toInt() == array_id)
                    {
                        item->setSelected(true);
                        break;
                    }
                }
            }

            return;
        }

        mw()->resetEditmodeButtons();

        edit->scene->clearSelection();
        edit->changeCursor(LevelEdit::MODE_PlaceItem);
        edit->scene->SwitchEditingMode(LvlScene::MODE_PlacingNew);
        edit->scene->setItemPlacer(4, ui->WarpList->currentData().toInt(), LvlPlacingItems::DOOR_Entrance);

        mw()->dock_LvlItemProps->hide();

        edit->setFocus();
    }
}

void LvlWarpBox::on_WarpSetExit_clicked()
{
    //placeDoorEntrance
    if(mw()->activeChildWindow() == MainWindow::WND_Level)
    {
        LevelEdit *edit = mw()->activeLvlEditWin();
        bool placed = false;
        int i = 0;
        int array_id = 0;
        for(i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                placed = edit->LvlData.doors[i].isSetOut;
                array_id = edit->LvlData.doors[i].meta.array_id;
                break;
            }
        }

        if(placed)
        {
            edit->goTo(edit->LvlData.doors[i].ox, edit->LvlData.doors[i].oy, true, QPoint(0, 0), true);
            //deselect all and select placed one
            foreach(QGraphicsItem *i, edit->scene->selectedItems())
            {
                i->setSelected(false);
            }
            foreach(QGraphicsItem *item, edit->scene->items())
            {
                if(item->data(ITEM_TYPE).toString() == "Door_exit")
                {
                    if(item->data(ITEM_ARRAY_ID).toInt() == array_id)
                    {
                        item->setSelected(true);
                        break;
                    }
                }
            }
            return;
        }

        mw()->resetEditmodeButtons();

        edit->scene->clearSelection();
        edit->changeCursor(LevelEdit::MODE_PlaceItem);
        edit->scene->SwitchEditingMode(LvlScene::MODE_PlacingNew);
        edit->scene->setItemPlacer(4, ui->WarpList->currentData().toInt(), LvlPlacingItems::DOOR_Exit);

        mw()->dock_LvlItemProps->hide();

        mw()->activeLvlEditWin()->setFocus();
    }
}

void LvlWarpBox::on_WarpLayer_currentIndexChanged(const QString &arg1)
{
    if(lockWarpSetSettings) return;

    int WinType = mw()->activeChildWindow();
    if(WinType == MainWindow::WND_Level)
    {
        LevelEdit *edit = mw()->activeLvlEditWin();
        QList<QVariant> dirData;
        for(int i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                dirData.push_back(edit->LvlData.doors[i].layer);
                dirData.push_back(arg1);
                edit->LvlData.doors[i].layer = arg1;
                break;
            }
        }
        edit->scene->m_history->addChangeWarpSettings(ui->WarpList->currentData().toInt(), HistorySettings::SETTING_LAYER, QVariant(dirData));
        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());
        edit->scene->applyLayersVisible();
        edit->LvlData.meta.modified = true;
    }
}

void LvlWarpBox::on_WarpEnterEvent_currentIndexChanged(const QString &arg1)
{
    if(lockWarpSetSettings) return;

    int WinType = mw()->activeChildWindow();
    if(WinType == MainWindow::WND_Level)
    {
        LevelEdit *edit = mw()->activeLvlEditWin();
        QList<QVariant> dirData;
        for(int i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                dirData.push_back(edit->LvlData.doors[i].event_enter);
                dirData.push_back(arg1);
                edit->LvlData.doors[i].event_enter = arg1;
                break;
            }
        }
        edit->scene->m_history->addChangeWarpSettings(ui->WarpList->currentData().toInt(), HistorySettings::SETTING_EV_WARP_ENTER, QVariant(dirData));
        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());
        edit->scene->applyLayersVisible();
        edit->LvlData.meta.modified = true;
    }
}


// ////////// Flags///////////
void LvlWarpBox::on_WarpTwoWay_clicked(bool checked)
{
    int WinType = mw()->activeChildWindow();
    if(WinType == MainWindow::WND_Level)
    {
        LevelEdit *edit = mw()->activeLvlEditWin();

        for(int i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                edit->LvlData.doors[i].two_way = checked;
                break;
            }
        }
        edit->scene->m_history->addChangeWarpSettings(ui->WarpList->currentData().toInt(), HistorySettings::SETTING_TWOWAY, QVariant(checked));
        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());
        edit->LvlData.meta.modified = true;
    }
}
void LvlWarpBox::on_WarpNoVehicles_clicked(bool checked)
{
    int WinType = mw()->activeChildWindow();
    if(WinType == MainWindow::WND_Level)
    {
        LevelEdit *edit = mw()->activeLvlEditWin();

        for(int i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                edit->LvlData.doors[i].novehicles = checked;
                break;
            }
        }
        edit->scene->m_history->addChangeWarpSettings(ui->WarpList->currentData().toInt(), HistorySettings::SETTING_NOVEHICLE, QVariant(checked));
        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());
        edit->LvlData.meta.modified = true;
    }
}
void LvlWarpBox::on_WarpAllowNPC_clicked(bool checked)
{
    int WinType = mw()->activeChildWindow();
    if(WinType == MainWindow::WND_Level)
    {
        LevelEdit *edit = mw()->activeLvlEditWin();

        for(int i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                edit->LvlData.doors[i].allownpc = checked;
                break;
            }
        }
        edit->scene->m_history->addChangeWarpSettings(ui->WarpList->currentData().toInt(), HistorySettings::SETTING_ALLOWNPC, QVariant(checked));
        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());
        edit->LvlData.meta.modified = true;
    }
}

void LvlWarpBox::on_WarpLock_clicked(bool checked)
{
    int WinType = mw()->activeChildWindow();
    if(WinType == MainWindow::WND_Level)
    {
        LevelEdit *edit = mw()->activeLvlEditWin();

        for(int i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                edit->LvlData.doors[i].locked = checked;
                break;
            }
        }
        edit->scene->m_history->addChangeWarpSettings(ui->WarpList->currentData().toInt(), HistorySettings::SETTING_LOCKED, QVariant(checked));
        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());
        edit->LvlData.meta.modified = true;
    }
}

void LvlWarpBox::on_WarpBombNeed_clicked(bool checked)
{
    int WinType = mw()->activeChildWindow();
    if(WinType == MainWindow::WND_Level)
    {
        LevelEdit *edit = mw()->activeLvlEditWin();

        for(int i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                edit->LvlData.doors[i].need_a_bomb = checked;
                break;
            }
        }
        edit->scene->m_history->addChangeWarpSettings(ui->WarpList->currentData().toInt(), HistorySettings::SETTING_NEED_A_BOMB, QVariant(checked));
        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());
        edit->LvlData.meta.modified = true;
    }
}

void LvlWarpBox::on_WarpSpecialStateOnly_clicked(bool checked)
{
    int WinType = mw()->activeChildWindow();
    if(WinType == MainWindow::WND_Level)
    {
        LevelEdit *edit = mw()->activeLvlEditWin();

        for(int i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                edit->LvlData.doors[i].special_state_required = checked;
                break;
            }
        }
        edit->scene->m_history->addChangeWarpSettings(ui->WarpList->currentData().toInt(), HistorySettings::SETTING_W_SPECIAL_STATE_REQUIRED, QVariant(checked));
        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());
        edit->LvlData.meta.modified = true;
    }
}



/////Door props
void LvlWarpBox::on_WarpType_currentIndexChanged(int index)
{
    if(lockWarpSetSettings) return;

    int WinType = mw()->activeChildWindow();
    if(WinType == MainWindow::WND_Level)
    {
        QList<QVariant> warpTypeData;
        LevelEdit *edit = mw()->activeLvlEditWin();
        unsigned int warpId = static_cast<unsigned>(ui->WarpList->currentData().toInt());

        for(int i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == warpId)
            {
                warpTypeData.push_back(edit->LvlData.doors[i].type);
                warpTypeData.push_back(index);
                edit->LvlData.doors[i].type = index;
                break;
            }
        }

        ui->WarpEntranceGrp->setEnabled(index == LevelDoor::WARP_PIPE);
        ui->WarpExitGrp->setEnabled(index == LevelDoor::WARP_PIPE);

        edit->scene->m_history->addChangeWarpSettings(static_cast<int>(warpId), HistorySettings::SETTING_WARPTYPE, QVariant(warpTypeData));
        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());
        edit->LvlData.meta.modified = true;
    }
}
void LvlWarpBox::on_WarpNeedAStars_valueChanged(int arg1)
{
    if(lockWarpSetSettings) return;

    int WinType = mw()->activeChildWindow();
    if(WinType == MainWindow::WND_Level)
    {
        QList<QVariant> starData;
        LevelEdit *edit = mw()->activeLvlEditWin();
        unsigned int warpId = static_cast<unsigned>(ui->WarpList->currentData().toInt());

        for(int i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == warpId)
            {
                starData.push_back(edit->LvlData.doors[i].stars);
                starData.push_back(arg1);
                edit->LvlData.doors[i].stars = arg1;
                break;
            }
        }
        edit->scene->m_history->addChangeWarpSettings(static_cast<int>(warpId), HistorySettings::SETTING_NEEDASTAR, QVariant(starData));
        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());
        edit->LvlData.meta.modified = true;
    }

}
void LvlWarpBox::on_WarpNeedAStarsMsg_editingFinished()
{
    if(lockWarpSetSettings) return;

    int WinType = mw()->activeChildWindow();
    if(WinType == MainWindow::WND_Level)
    {
        QList<QVariant> starData;
        LevelEdit *edit = mw()->activeLvlEditWin();

        for(int i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                QString msg = ui->WarpNeedAStarsMsg->text();
                starData.push_back(edit->LvlData.doors[i].stars_msg);
                starData.push_back(msg);
                edit->LvlData.doors[i].stars_msg = msg;
                break;
            }
        }
        edit->scene->m_history->addChangeWarpSettings(ui->WarpList->currentData().toInt(),
                                                      HistorySettings::SETTING_NEEDASTAR_MSG, QVariant(starData));
        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());
        edit->LvlData.meta.modified = true;
    }
}

void LvlWarpBox::on_WarpHideStars_clicked(bool checked)
{
    int WinType = mw()->activeChildWindow();
    if(WinType == MainWindow::WND_Level)
    {
        LevelEdit *edit = mw()->activeLvlEditWin();

        for(int i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                edit->LvlData.doors[i].star_num_hide = checked;
                break;
            }
        }
        edit->scene->m_history->addChangeWarpSettings(ui->WarpList->currentData().toInt(),
                                                      HistorySettings::SETTING_HIDE_STAR_NUMBER, QVariant(checked));
        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());
        edit->LvlData.meta.modified = true;
    }
}

void LvlWarpBox::entrance_clicked(int direction)
{
    if(lockWarpSetSettings) return;

    int winType = mw()->activeChildWindow();
    if(winType == MainWindow::WND_Level)
    {
        QList<QVariant> dirData;
        LevelEdit *edit = mw()->activeLvlEditWin();

        for(int i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                dirData.push_back(edit->LvlData.doors[i].idirect);
                dirData.push_back(direction);
                edit->LvlData.doors[i].idirect = direction;
                break;
            }
        }
        edit->scene->m_history->addChangeWarpSettings(ui->WarpList->currentData().toInt(),
                                                      HistorySettings::SETTING_ENTRDIR, QVariant(dirData));
        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());
        edit->LvlData.meta.modified = true;
    }
}

void LvlWarpBox::exit_clicked(int direction)
{
    if(lockWarpSetSettings) return;

    int winType = mw()->activeChildWindow();
    if(winType == MainWindow::WND_Level)
    {
        QList<QVariant> dirData;
        LevelEdit *edit = mw()->activeLvlEditWin();

        for(int i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                dirData.push_back(edit->LvlData.doors[i].odirect);
                dirData.push_back(direction);
                edit->LvlData.doors[i].odirect = direction;
                break;
            }
        }
        edit->scene->m_history->addChangeWarpSettings(ui->WarpList->currentData().toInt(),
                                                      HistorySettings::SETTING_EXITDIR, QVariant(dirData));
        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());
        edit->LvlData.meta.modified = true;
    }
}


void LvlWarpBox::on_WarpEnableCannon_clicked(bool checked)
{
    int winType = mw()->activeChildWindow();
    if(winType == MainWindow::WND_Level)
    {
        LevelEdit *edit = mw()->activeLvlEditWin();

        for(int i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                edit->LvlData.doors[i].cannon_exit = checked;
                break;
            }
        }
        edit->scene->m_history->addChangeWarpSettings(ui->WarpList->currentData().toInt(),
                                                      HistorySettings::SETTING_ENABLE_CANNON,
                                                      QVariant(checked));
        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());
        edit->LvlData.meta.modified = true;
    }
}

void LvlWarpBox::on_WarpCannonSpeed_valueChanged(double arg1)
{
    if(lockWarpSetSettings) return;

    int WinType = mw()->activeChildWindow();
    if(WinType == MainWindow::WND_Level)
    {
        QList<QVariant> starData;
        LevelEdit *edit = mw()->activeLvlEditWin();

        for(int i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                starData.push_back(edit->LvlData.doors[i].cannon_exit_speed);
                starData.push_back((float)arg1);
                edit->LvlData.doors[i].cannon_exit_speed = arg1;
                break;
            }
        }
        edit->scene->m_history->addChangeWarpSettings((unsigned int)ui->WarpList->currentData().toInt(), HistorySettings::SETTING_CANNON_SPEED, QVariant(starData));
        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());
        edit->LvlData.meta.modified = true;
    }
}


void LvlWarpBox::on_WarpToMapX_editingFinished()//_textEdited(const QString &arg1)
{
    if(lockWarpSetSettings) return;

    if(!ui->WarpToMapX->isModified()) return;
    ui->WarpToMapX->setModified(false);

    int WinType = mw()->activeChildWindow();
    if(WinType == MainWindow::WND_Level)
    {
        LevelEdit *edit = mw()->activeLvlEditWin();

        QString arg1 = ui->WarpToMapX->text();

        for(int i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                if(arg1.isEmpty())
                    edit->LvlData.doors[i].world_x = -1;
                else
                    edit->LvlData.doors[i].world_x = arg1.toInt();

                edit->LvlData.meta.modified = true;
                break;
            }
        }
        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());
        edit->LvlData.meta.modified = true;
    }
}

void LvlWarpBox::on_WarpToMapY_editingFinished()//_textEdited(const QString &arg1)
{
    if(lockWarpSetSettings) return;

    if(!ui->WarpToMapY->isModified()) return;
    ui->WarpToMapY->setModified(false);

    int WinType = mw()->activeChildWindow();
    if(WinType == MainWindow::WND_Level)
    {
        LevelEdit *edit = mw()->activeLvlEditWin();

        QString arg1 = ui->WarpToMapY->text();

        for(int i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                if(arg1.isEmpty())
                    edit->LvlData.doors[i].world_y = -1;
                else
                    edit->LvlData.doors[i].world_y = arg1.toInt();

                edit->LvlData.meta.modified = true;
                break;
            }
        }
        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());
        edit->LvlData.meta.modified = true;
    }
}

void LvlWarpBox::on_WarpGetXYFromWorldMap_clicked()
{
    if(lockWarpSetSettings)
        return;

    int windowType = mw()->activeChildWindow();
    if(windowType == MainWindow::WND_Level)
    {
        QString woldMapsPath = mw()->activeLvlEditWin()->LvlData.meta.path;
        QString woldMapsFile;
        QStringList filters;
        QStringList files;
        QDir levelDir(woldMapsPath);

        filters << "*.wld" << "*.wldx";
        levelDir.setSorting(QDir::Name);
        levelDir.setNameFilters(filters);

        files = levelDir.entryList(filters);
        if(files.isEmpty())
        {
            QMessageBox::warning(this,
                                 tr("World map files not found"),
                                 tr("You haven't available world map files with this level file.\n"
                                    "Please, put this level file with a world map, "
                                    "or create new world map in the same fomder with this level file.\n"
                                    "File path: %1").arg(woldMapsPath), QMessageBox::Ok);
            return;
        }

        bool ok = true;
        if(files.count() == 1)
            woldMapsFile = files.first();
        else
        {
            woldMapsFile = QInputDialog::getItem(this,
                                                  tr("Select world map file"),
                                                  tr("Found more than one world map files.\n"
                                                     "Please, select necessary world map in a list:"),
                                                  files, 0, false, &ok);
        }

        if(woldMapsFile.isEmpty() || !ok)
            return;

        QString wldPath = QString("%1/%2")
                .arg(woldMapsPath)
                .arg(woldMapsFile);


        QFile file(wldPath);
        if(!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::critical(this,
                                  tr("File open error"),
                                  tr("Can't open the file!"),
                                  QMessageBox::Ok);
            return;
        }
        WorldData FileData;
        if(!FileFormats::OpenWorldFile(wldPath, FileData))
        {
            mw()->formatErrorMsgBox(wldPath,
                                    FileData.meta.ERROR_info,
                                    FileData.meta.ERROR_linenum,
                                    FileData.meta.ERROR_linedata);
            return;
        }

        std::unique_ptr<WLD_SetPoint> pointDialog;
        pointDialog.reset(new WLD_SetPoint(mw()));

        pointDialog->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
        pointDialog->setGeometry(util::alignToScreenCenter(pointDialog->size()));

        if(pointDialog->loadFile(FileData, mw()->configs, GlobalSettings::LvlOpts))
        {
            pointDialog->resetPosition();
            if(ui->WarpToMapX->text().isEmpty() || ui->WarpToMapY->text().isEmpty())
            {
                pointDialog->m_mapPointIsNull = true;
            }
            else
            {
                pointDialog->pointSelected(
                    QPoint(ui->WarpToMapX->text().toInt(),
                           ui->WarpToMapY->text().toInt())
                );
                pointDialog->goTo(ui->WarpToMapX->text().toInt() + 16,
                                  ui->WarpToMapY->text().toInt() + 16,
                                  QPoint(-qRound(qreal(pointDialog->gViewPort()->width()) / 2),
                                         -qRound(qreal(pointDialog->gViewPort()->height()) / 2))
                );
                pointDialog->m_scene->m_pointSelector.setPoint(pointDialog->m_mapPoint);
            }

            if(pointDialog->exec() == QDialog::Accepted)
            {
                ui->WarpToMapX->setText(QString::number(pointDialog->m_mapPoint.x()));
                ui->WarpToMapY->setText(QString::number(pointDialog->m_mapPoint.y()));
                ui->WarpToMapX->setModified(true);
                ui->WarpToMapY->setModified(true);
                on_WarpToMapX_editingFinished();
                on_WarpToMapY_editingFinished();
            }
        }
    }
}


/////Door mode (Level Entrance / Level Exit)
void LvlWarpBox::on_WarpLevelExit_clicked(bool checked)
{
    if(lockWarpSetSettings) return;

    int WinType = mw()->activeChildWindow();
    if(WinType == MainWindow::WND_Level)
    {
        QList<QVariant> extraData;
        LevelEdit *edit = mw()->activeLvlEditWin();
        bool exists = false;
        int i = 0;
        for(i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                exists = true;
                extraData.push_back(checked);
                if(checked)
                {
                    extraData.push_back((int)edit->LvlData.doors[i].ox);
                    extraData.push_back((int)edit->LvlData.doors[i].oy);
                }
                edit->LvlData.doors[i].lvl_o = checked;
                break;
            }
        }

        if(!exists) return;

        //Disable placing door point, if it not avaliable
        ui->WarpSetEntrance->setEnabled(
            ((!edit->LvlData.doors[i].lvl_o) && (!edit->LvlData.doors[i].lvl_i)) ||
            ((edit->LvlData.doors[i].lvl_o) && (!edit->LvlData.doors[i].lvl_i))
        );
        //Disable placing door point, if it not avaliable
        ui->WarpSetExit->setEnabled(
            ((!edit->LvlData.doors[i].lvl_o) && (!edit->LvlData.doors[i].lvl_i)) ||
            (edit->LvlData.doors[i].lvl_i));

        bool iPlaced = edit->LvlData.doors[i].isSetIn;
        bool oPlaced = edit->LvlData.doors[i].isSetOut;

        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());

        //Unset placed point, if not it avaliable
        if(!(((!edit->LvlData.doors[i].lvl_o) && (!edit->LvlData.doors[i].lvl_i)) ||
             (edit->LvlData.doors[i].lvl_i)))
        {
            oPlaced = false;
            ui->WarpExitPlaced->setChecked(false);
            edit->LvlData.doors[i].ox = edit->LvlData.doors[i].ix;
            edit->LvlData.doors[i].oy = edit->LvlData.doors[i].iy;
        }

        edit->LvlData.doors[i].isSetIn = iPlaced;
        edit->LvlData.doors[i].isSetOut = oPlaced;

        edit->scene->m_history->addChangeWarpSettings((unsigned int)ui->WarpList->currentData().toInt(), HistorySettings::SETTING_LEVELEXIT, QVariant(extraData));
        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());
        edit->LvlData.meta.modified = true;
    }

}

void LvlWarpBox::on_WarpLevelEntrance_clicked(bool checked)
{
    if(lockWarpSetSettings) return;

    int WinType = mw()->activeChildWindow();
    if(WinType == MainWindow::WND_Level)
    {
        QList<QVariant> extraData;
        LevelEdit *edit = mw()->activeLvlEditWin();
        int i = 0;
        bool exists = false;
        for(i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                exists = true;
                extraData.push_back(checked);
                if(checked)
                {
                    extraData.push_back((int)edit->LvlData.doors[i].ix);
                    extraData.push_back((int)edit->LvlData.doors[i].iy);
                }
                edit->LvlData.doors[i].lvl_i = checked;
                break;
            }
        }

        if(!exists) return;

        //Disable placing door point, if it not avaliable
        ui->WarpSetEntrance->setEnabled(
            ((!edit->LvlData.doors[i].lvl_o) && (!edit->LvlData.doors[i].lvl_i)) ||
            ((edit->LvlData.doors[i].lvl_o) && (!edit->LvlData.doors[i].lvl_i)));
        //Disable placing door point, if it not avaliable
        ui->WarpSetExit->setEnabled(
            ((!edit->LvlData.doors[i].lvl_o) && (!edit->LvlData.doors[i].lvl_i)) ||
            (edit->LvlData.doors[i].lvl_i));

        bool iPlaced = edit->LvlData.doors[i].isSetIn;
        bool oPlaced = edit->LvlData.doors[i].isSetOut;

        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());

        //Unset placed point, if not it avaliable
        if(!(((!edit->LvlData.doors[i].lvl_o) && (!edit->LvlData.doors[i].lvl_i)) ||
             ((edit->LvlData.doors[i].lvl_o) && (!edit->LvlData.doors[i].lvl_i))))
        {
            iPlaced = false;
            ui->WarpEntrancePlaced->setChecked(false);
            edit->LvlData.doors[i].ix = edit->LvlData.doors[i].ox;
            edit->LvlData.doors[i].iy = edit->LvlData.doors[i].oy;
        }

        edit->LvlData.doors[i].isSetIn = iPlaced;
        edit->LvlData.doors[i].isSetOut = oPlaced;

        edit->scene->m_history->addChangeWarpSettings((unsigned int)ui->WarpList->currentData().toInt(), HistorySettings::SETTING_LEVELENTR, QVariant(extraData));
        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());
        edit->LvlData.meta.modified = true;
    }

}


void LvlWarpBox::on_WarpBrowseLevels_clicked()
{
    if(lockWarpSetSettings) return;

    QString dirPath;
    if(mw()->activeChildWindow() != MainWindow::WND_Level)
        return;

    LevelEdit *edit = mw()->activeLvlEditWin();
    if(!edit) return;

    dirPath = edit->LvlData.meta.path;

    if(edit->isUntitled())
    {
        QMessageBox::information(this,
                                 tr("Please save the file"),
                                 tr("Please save the file before selecting levels."),
                                 QMessageBox::Ok);
        return;
    }

    LevelFileList levelList(dirPath, ui->WarpLevelFile->text());
    if(levelList.exec() == QDialog::Accepted)
    {
        ui->WarpLevelFile->setText(levelList.SelectedFile);
        ui->WarpLevelFile->setModified(true);
        on_WarpLevelFile_editingFinished();
    }
}


void LvlWarpBox::on_WarpHideLevelEnterScreen_clicked(bool checked)
{
    int WinType = mw()->activeChildWindow();
    if(WinType == MainWindow::WND_Level)
    {
        LevelEdit *edit = mw()->activeLvlEditWin();

        for(int i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                edit->LvlData.doors[i].hide_entering_scene = checked;
                break;
            }
        }
        edit->scene->m_history->addChangeWarpSettings(ui->WarpList->currentData().toInt(), HistorySettings::SETTING_HIDE_LEVEL_ENTER_SCENE, QVariant(checked));
        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());
        edit->LvlData.meta.modified = true;
    }
}

void LvlWarpBox::on_WarpAllowNPC_IL_clicked(bool checked)
{
    int WinType = mw()->activeChildWindow();
    if(WinType == MainWindow::WND_Level)
    {
        LevelEdit *edit = mw()->activeLvlEditWin();

        for(int i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                edit->LvlData.doors[i].allownpc_interlevel = checked;
                break;
            }
        }
        edit->scene->m_history->addChangeWarpSettings(ui->WarpList->currentData().toInt(), HistorySettings::SETTING_ALLOWNPC_IL, QVariant(checked));
        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());
        edit->LvlData.meta.modified = true;
    }
}


void LvlWarpBox::on_WarpLevelFile_editingFinished()//_textChanged(const QString &arg1)
{
    if(lockWarpSetSettings) return;

    if(!ui->WarpLevelFile->isModified()) return;
    ui->WarpLevelFile->setModified(false);

    int WinType = mw()->activeChildWindow();
    if(WinType == MainWindow::WND_Level)
    {
        LevelEdit *edit = mw()->activeLvlEditWin();

        for(int i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                edit->LvlData.doors[i].lname = ui->WarpLevelFile->text();
                break;
            }
        }
        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());
        edit->LvlData.meta.modified = true;
    }
}
void LvlWarpBox::on_WarpToExitNu_valueChanged(int arg1)
{
    if(lockWarpSetSettings) return;

    int WinType = mw()->activeChildWindow();
    if(WinType == MainWindow::WND_Level)
    {
        QList<QVariant> warpToData;
        LevelEdit *edit = mw()->activeLvlEditWin();

        for(int i = 0; i < edit->LvlData.doors.size(); i++)
        {
            if(edit->LvlData.doors[i].meta.array_id == (unsigned int)ui->WarpList->currentData().toInt())
            {
                warpToData.push_back((int)edit->LvlData.doors[i].warpto);
                warpToData.push_back(arg1);
                edit->LvlData.doors[i].warpto = arg1;
                break;
            }
        }
        edit->scene->m_history->addChangeWarpSettings((unsigned int)ui->WarpList->currentData().toInt(), HistorySettings::SETTING_LEVELWARPTO, QVariant(warpToData));
        edit->scene->doorPointsSync((unsigned int)ui->WarpList->currentData().toInt());
        edit->LvlData.meta.modified = true;
    }
}


QComboBox *LvlWarpBox::getWarpList()
{
    return ui->WarpList;
}

void LvlWarpBox::removeItemFromWarpList(int index)
{
    ui->WarpList->removeItem(index);
}


void LvlWarpBox::setWarpRemoveButtonEnabled(bool isEnabled)
{
    ui->WarpRemove->setEnabled(isEnabled);
}

