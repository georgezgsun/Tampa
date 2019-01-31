#ifndef VIDEO_SETUP_H
#define VIDEO_SETUP_H

#include <QWidget>

#include "base_menu.h"

namespace Ui {
class videoSetup;
}

class videoSetup : public baseMenu
{
    Q_OBJECT

public:
    explicit videoSetup(QWidget *parent = 0);
    ~videoSetup();

    void toggleValue(int, int, int f=0);

private slots:
    void on_pb_frames_clicked();

    void on_pb_resolution_clicked();

    void on_pb_images_clicked();

    void on_pb_prebuffer_clicked();

    void on_pb_postbuffer_clicked();

    void on_pb_illuminator_clicked();

private:
    Ui::videoSetup *ui;

    void initLists();
    void setInittoggleValues();
    void buildHashTables();

    QStringList m_framesList;
    QStringList m_resolutionList;
    QStringList m_imagesList;
    QStringList m_prebufferList;
    QStringList m_postbufferList;
    SysConfig mConf;
    int m_framesIndex;
    int m_resolutionIndex;
    int m_imagesIndex;
    int m_prebufferIndex;
    int m_postbufferIndex;
    int mOldFramesIndex;
    int mOldResolutionIndex;
    int mOldImagesIndex;
    int mOldPrebufferIndex;
    int mOldPostbufferIndex;
#ifdef HH1
    QStringList m_imageSpacingList;
    int m_imageSpacingIndex;
    int mOldImageSpacingIndex;
#endif
};

#endif // VIDEO_SETUP_H
