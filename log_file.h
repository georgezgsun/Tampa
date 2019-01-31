#ifndef LOG_FILE_H
#define LOG_FILE_H

#include <QWidget>
#include "base_menu.h"

namespace Ui {
class logFile;
}

class logFile : public baseMenu
{
    Q_OBJECT

public:
    explicit logFile(QWidget *parent = 0);
    ~logFile();

private slots:
   void on_pb_clearLog_clicked();

private:
    Ui::logFile *ui;

    void initLists(){}

};

#endif // LOG_FILE_H
