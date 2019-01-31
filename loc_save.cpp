#include "loc_save.h"
#include "ui_loc_save.h"
#include "utils.h"
#include "state.h"
#include "debug.h"

locSave::locSave(int type, QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::locSave),
    m_menuType (type)
{
  //  DEBUG() << "Entered";
  ui->setupUi(this);

    this->initLists();
    this->setInittoggleValues();

    if (m_menuType == CMD_LOC_LOAD)
        ui->lb_select->setText("LOAD LOCATION\nSELECT MEMORY SLOT:");

	state& v = state::get();
    v.setState(STATE_LEAF_MENU);
    m_command = m_cmdList.at(m_listIndex);

    connect(ui->lw_locList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(locItemClicked(QListWidgetItem*)));
    this->m_lwFocused = false;
    this->m_backupLoc.index = 0;
    this->m_backupTransitLoc.index = 0;
    this->loadLocsFromDB();
}

locSave::~locSave()
{
    //  DEBUG() << "Entered";
    this->disconnectWidgetSigs();
    delete ui;
}

void locSave::initLists()
{
    m_list << ui->lw_locList;

    m_cmdList << CMD_LOC_LIST;

    this->connectWidgetSigs();
}

void locSave::setInittoggleValues()
{

}

void locSave::toggleValue(int, int, int )
{
    int row = ui->lw_locList->currentRow();
//    if (ui->lw_locList->hasFocus() || row < 0)
    if (row < 0)
    {
        qDebug() << "row: " << row << "Focus: " << ui->lw_locList->hasFocus() << endl;
        return;
    }

    struct Location l;
    Utils& u = Utils::get();

    if (m_menuType == CMD_LOC_LOAD)
    {
        //set transit data
        l.index = m_locs[row].index;
        l.description = m_locs[row].description;
        l.speedLimit = m_locs[row].speedLimit;
        l.captureSpeed = m_locs[row].captureSpeed;
        l.roadCondition = m_locs[row].roadCondition;
        l.numberLanes = m_locs[row].numberLanes;
        u.setTransitData(CMD_LOC_LOAD, (DBStruct *)&l);
        u.getExitButton()->clicked();   // Clicked 'Exit' button
    }
    else if (m_menuType == CMD_LOC_SAVE)
    {
        //replace the location entry with transit data
        int retv = u.getTransitData(CMD_LOC_SAVE, (DBStruct *)&l);
        if (retv < 0)
        {
            qCritical() << "get transit location failed";
            return;
        }

        // 20002
        if (l.description.isNull() || l.description.isEmpty())
        {
            qCritical() << "Location cannot be empty";
            return;
        }
        else if (l.speedLimit.isNull() || l.speedLimit.isEmpty())
        {
            qCritical() << "Speed Limit cannot be empty";
            return;
        }
        else if (l.captureSpeed.isNull() || l.captureSpeed.isEmpty())
        {
            qCritical() << "Capture speed cannot be empty";
            return;
        }
        else if (l.roadCondition.isNull() || l.roadCondition.isEmpty())
        {
            qCritical() << "Road Condition cannot be empty";
            return;
        }
        else if (!l.numberLanes)
        {
            qCritical() << "Number of Lanes cannot be 0";
            return;
        }

        //backup data
        if (m_locs[row].index)
        {
            m_backupLoc.index = row + 1;
            m_backupLoc.description = m_locs[row].description;
            m_backupLoc.speedLimit = m_locs[row].speedLimit;
            m_backupLoc.captureSpeed = m_locs[row].captureSpeed;
            m_backupLoc.roadCondition = m_locs[row].roadCondition;
            m_backupLoc.numberLanes = m_locs[row].numberLanes;
        }
        m_backupTransitLoc.index = row + 1;
        m_backupTransitLoc.description = l.description;
        m_backupTransitLoc.speedLimit = l.speedLimit;
        m_backupTransitLoc.captureSpeed = l.captureSpeed;
        m_backupTransitLoc.roadCondition = l.roadCondition;
        m_backupTransitLoc.numberLanes = l.numberLanes;

        qDebug() << l.description;

        //update database and refresh display
        l.index = row + 1;
        if (m_locs[row].index)
            retv = u.db()->updateEntry(TBL_LOCATION, (DBStruct *)&l);
        else
            retv = u.db()->addEntry(TBL_LOCATION, (DBStruct *)&l);
        if (retv) {
            qCritical() << "update location entry failed (" << l.description << ")";
        }
        this->loadLocsFromDB();  //update the list
    }
    //save location
    u.setLocationIndex(l.index);
}

void locSave::locItemClicked(QListWidgetItem *)
{
    m_lwFocused = true;
    this->setIndexAndCmd(ui->lw_locList);
    //baseMenu::setCmd(ui->lw_locList);
    this->setLWFocus();
}

int locSave::loadLocsFromDB()
{
    QListWidget *lw = ui->lw_locList;
    struct Location l;
    int ct = 0;
    int retv;
	userDB *m_userDB;
	Utils& uu = Utils::get();
    m_userDB = uu.db();

    for (int i = 0; i <MAX_LOCATION_ENTRIES; i++) {
        m_locs[i].index = 0;
        m_locs[i].description.clear();
        //memset(&m_locs[i], 0, sizeof(Location));
    }

    ct = m_userDB->queryEntry(TBL_LOCATION, (DBStruct *)&l, QRY_ALL_ENTRIES);
    //    DEBUG() << "total locations " << ct;
    lw->clear();

    for (int i = 0; i < ct; i++)
    {
        retv = m_userDB->getNextEntry(TBL_LOCATION, (DBStruct *)&l);
        if (!retv && l.index <= MAX_LOCATION_ENTRIES)
        {
            m_locs[l.index-1].index = l.index;
            m_locs[l.index-1].description = l.description;
            m_locs[l.index-1].speedLimit = l.speedLimit;
            m_locs[l.index-1].captureSpeed = l.captureSpeed;
            m_locs[l.index-1].roadCondition = l.roadCondition;
            m_locs[l.index-1].numberLanes = l.numberLanes;
        }
    }

    //display the list
    for (int i = 0; i < MAX_LOCATION_ENTRIES; i++) {
        QString s = QString("%1\t").arg(i+1, 2, 10);
        s = s + m_locs[i].description;
        //s = s + QString("aaaa");
        QListWidgetItem *item = new QListWidgetItem(s);
        lw->addItem(item);
    }

	//    DEBUG() << "current location " << lw->currentRow();
	//    DEBUG() << "count " << lw->count();
    return 0;
}

void locSave::exeDownSelect() {
    int checkAgain = 0;
    QWidget *p = m_list.at(m_listIndex);
    if (p == ui->lw_locList) {
        int row = ui->lw_locList->currentRow();
        row++;
        if (row < ui->lw_locList->count()) {
            m_lwFocused = true;
            ui->lw_locList->setCurrentRow(row);
            //this->enableButtons(true);
            return;
        } else {
            m_lwFocused = false;
            goto baseSlot;
        }
    } else
        checkAgain = 1;

baseSlot:
    //this->enableButtons(false);
    baseMenu::exeDownSelect();

    if (checkAgain)
        setLWFocus();
}

void locSave::exeUpSelect() {
    int checkAgain = 0;
    QWidget *p = m_list.at(m_listIndex);
    if (p == ui->lw_locList) {
        int row = ui->lw_locList->currentRow();
        row--;
        if (row < 0) {
            m_lwFocused = false;
            goto baseSlot;
        }
        m_lwFocused = true;
        ui->lw_locList->setCurrentRow(row);
        //this->enableButtons(true);
        return;
    } else
        checkAgain = 1;

baseSlot:
    //this->enableButtons(false);
    baseMenu::exeUpSelect();

    if (checkAgain) {
        setLWFocus();
    }
}

void locSave::setLWFocus()
{
    QWidget *p = m_list.at(m_listIndex);
    if (p == ui->lw_locList) {
        m_lwFocused = true;
        int row = ui->lw_locList->currentRow();
        if (row < 0 || row >= ui->lw_locList->count())
            row = 0;
        ui->lw_locList->setCurrentRow(row);
        //this->enableButtons(true);
    }
}

void locSave::enableButtons(bool )
{

}

void locSave::exeCancelClick()
{
    Utils& u = Utils::get();
   if (m_menuType == CMD_LOC_LOAD) {
       //clear transit data
       u.clearTransitData();
   } else if (m_menuType == CMD_LOC_SAVE) {
       //restore original data
       if (m_backupLoc.index) {
           u.db()->updateEntry(TBL_LOCATION, (DBStruct *)&m_backupLoc);
           m_backupLoc.index = 0;
       } else if (m_backupTransitLoc.index) {
           u.db()->deleteEntry(TBL_LOCATION, (DBStruct *)&m_backupTransitLoc);
           m_backupTransitLoc.index = 0;
       }
       this->loadLocsFromDB();

       //restore transit data
       u.setTransitData(CMD_LOC_SAVE, (DBStruct *)&m_backupTransitLoc);
   }
   ui->lw_locList->setCurrentRow(-1);
   m_lwFocused = false;
   ui->lw_locList->clearFocus();
}
