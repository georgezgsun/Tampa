#ifndef UPLOAD_MGR_H
#define UPLOAD_MGR_H

#include <QWidget>
#include <QListWidgetItem>
#include "base_menu.h"

namespace Ui {
class uploadMgr;
}

class uploadMgr : public baseMenu
{
    Q_OBJECT

public:
    //explicit uploadMgr(QWidget *parent = 0);
    explicit uploadMgr(QWidget *parent, QFileInfoList& fil, const QListWidgetItem *twi);
    ~uploadMgr();

private:
    Ui::uploadMgr *ui;
    QFileInfoList m_fileInfoList;

    void initLists();
    void setInittoggleValues();

    void loadVideoFiles();

};

#endif // UPLOAD_MGR_H
