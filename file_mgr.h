#ifndef FILE_MGR_H
#define FILE_MGR_H

#include <QWidget>
#include <QDir>
#include <QFileInfo>
#include <QListWidgetItem>

#include "base_menu.h"

#define DEFAULT_VEDIO_PATH  "/mnt/mmc/ipnc"
#define DEFAULT_FILTER      QDir::AllEntries
#define DEFAULT_SORT_FLAG   QDir::Name | QDir::IgnoreCase
#define ARROW_UP    1
#define ARROW_DOWN  2

//FM_STATE values
#define FM_SELECT_NEW   1
#define FM_SELECT_LAST  2
#define FM_SELECT_ALL   3

namespace Ui {
class fileMgr;
}

class fileMgr : public baseMenu
{
    Q_OBJECT

public:
    explicit fileMgr(QWidget *parent = 0);
    ~fileMgr();

	static fileMgr& get() {
	  static fileMgr instance;
	  return instance;
    }

    void toggleValue(int, int, int f=0);

    //reimplement virutal slots
    void exeUpSelect();
    void exeDownSelect();
    QFileInfoList& fileInfoList() { return m_FIList; }
    const QListWidgetItem *currTWItem() { return m_currTWItem; }
    void uploadUSB(); 

private slots:
    void twItemChanged();
    void setCmd();

    void on_pb_print_clicked();

private:
    Ui::fileMgr *ui;

    QFileInfoList m_FIList;
    QListWidgetItem *m_currTWItem;
    QList <QListWidgetItem *> m_listItemList;

    int m_fmState;

    void initLists();
    void setInittoggleValues();

    QDir m_videoDir;        // Video files path
    void setVideoPath(void);
    void loadVideoFiles();  //load files from video dir

    bool m_twFocused;       //tree widget in focus
    void setTWFocus(int Flag=ARROW_DOWN);      //set the Tree Widget's focus when the TW is selected
    void enableButtons(bool);
};

#endif // FILE_MGR_H
