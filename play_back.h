#ifndef PLAY_BACK_H
#define PLAY_BACK_H

#include <QWidget>
#include "base_menu.h"

namespace Ui {
class playBack;
}

class playBack : public baseMenu
{
    Q_OBJECT

public:
    explicit playBack(QWidget *parent = 0);
    ~playBack();

    void startPlay(void);
    void setFileName(QString &filename);

private slots:
    void on_pb_exit_clicked();

    void on_pb_restart_clicked();

    void on_pb_pause_clicked();

private:
    Ui::playBack *ui;

    void initLists();

    QString mFileName;

};

#endif // PLAY_BACK_H
