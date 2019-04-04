#include "video_setup.h"
#include "ui_video_setup.h"
#include "state.h"
#include "utils.h"
#include "illuminator.h"
#include "widgetKeyBoard.h"


videoSetup::videoSetup(QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::videoSetup)
{
    ui->setupUi(this);

    mConf = Utils::get().getConfiguration();
       int units = mConf.units;
       if (units  == 0)
           {
               ui->lb_pictureDist_2->setText("Feet");

           }
       else if ( units == 1)
       {
           ui->lb_pictureDist_2->setText("Meters");

       }
       else if (units  == 2)
       {
           ui->lb_pictureDist_2->setText("Feet");

       }


    this->initLists();
    this->buildHashTables();
    this->setInittoggleValues();


    state& v = state::get();
    v.setState(STATE_VIDEO_SETUP);
    m_listIndex = m_prevListIndex = 0;
#ifndef HH1
    m_command = CMD_NONE;
#else
    m_command = m_cmdList.at(m_listIndex);
#endif
}

videoSetup::~videoSetup()
{
    bool flag1 = false; // Assumed no modification


    // Image(s) per file
    if (mOldImagesIndex != m_imagesIndex)
    {
        mConf.imagesPerFile = (m_imagesIndex + 1);
        flag1 = true;
    }

    if (mOldPrebufferIndex != m_prebufferIndex)
    {   // Pre-event buffer changed
        if (!m_prebufferIndex)
            mConf.preBuf = 1;
        else if (m_prebufferIndex == 1)
            mConf.preBuf = 2;
        else
            mConf.preBuf = 5;
        flag1 = true;
    }
    if (mOldPostbufferIndex != m_postbufferIndex)
    {   // Pos-event buffer changed
        if (!m_postbufferIndex)
            mConf.postBuf = 5;
        else if (m_postbufferIndex == 1)
            mConf.postBuf = 10;
        else if (m_postbufferIndex == 2)
            mConf.postBuf = 15;
        else
            mConf.postBuf = 20;

        flag1 = true;
    }
   if(mConf.pictureDist != ui->le_pictureDist->text())
   {
       mConf.pictureDist= ui->le_pictureDist->text();
       flag1 = true;
   }

#ifdef HH1
    // Image Spacing
    if (mOldImageSpacingIndex != m_imageSpacingIndex)
    {
        mConf.imageSpacing = m_imageSpacingIndex;
        flag1 = true;
    }
#endif
    if (flag1 == true)
        Utils::get().setConfiguration(mConf);

    delete ui;
}

void videoSetup::initLists()
{

    m_list << ui->pb_cameraConfig
           << ui->pb_images
           << ui->pb_prebuffer
           << ui->pb_postbuffer
           << ui->le_pictureDist;

    m_cmdList << CMD_CAMERACONFIG
              << CMD_IMAGES
              << CMD_PREBUFFER
              << CMD_POSTBUFFER
              << CMD_PICTURDISTANCE;



    this->connectWidgetSigs();

}

void videoSetup::buildHashTables()
{
    m_imagesList << "1 Image/File" << "2 Images/File" << "3 Images/File";
   // m_resolutionList << "LOW Resolution" << "MED Resolution" << "HIGH Resolution";
    m_prebufferList << "1 Second" << "2 Seconds" << "5 Seconds";
    m_postbufferList << "5 Seconds" << "10 Seconds" << "15 Seconds" << "20 Seconds";
#ifdef HH1
    //m_framesList << "5 Frames/Sec" << "10 Frames/Sec" << "15 Frames/Sec";
    m_imageSpacingList << "Shorter" << "Normal" << "Longer";
#else
    m_framesList << "5 Frames/Sec" << "10 Frames/Sec" << "15 Frames/Sec" << "30 Frames/Sec";
#endif
}

void videoSetup::setInittoggleValues()
{
    mConf = Utils::get().getConfiguration();

    // Image(s) per file
    mOldImagesIndex = m_imagesIndex = (mConf.imagesPerFile - 1);

    // Check Pre-event Buffer
    switch (mConf.preBuf)
    {
        case 1:
            m_prebufferIndex = 0;
            break;
        case 2:
            m_prebufferIndex = 1;
            break;
        case 5:
            m_prebufferIndex = 2;
            break;
        default:
            m_prebufferIndex = 2;
    }
    mOldPrebufferIndex = m_prebufferIndex;

    // Check Post-event Buffer
    switch (mConf.postBuf)
    {
        case 5:
            m_postbufferIndex = 0;
            break;
        case 10:
            m_postbufferIndex = 1;
            break;
        case 15:
            m_postbufferIndex = 2;
            break;
        case 20:
            m_postbufferIndex = 3;
            break;
        default:
            m_postbufferIndex = 3;
    }
    mOldPostbufferIndex = m_postbufferIndex;
    ui->pb_images->setText(m_imagesList.at(m_imagesIndex));
    ui->pb_prebuffer->setText(m_prebufferList.at(m_prebufferIndex));
    ui->pb_postbuffer->setText(m_postbufferList.at(m_postbufferIndex));
    ui->le_pictureDist->setText( mConf.pictureDist);
   // mOldPictureDist = m_pictureDist;
#ifdef HH1
    // Dist Between Images
    mOldImageSpacingIndex = m_imageSpacingIndex = mConf.imageSpacing;

   // ui->pb_illuminator->setText(m_imageSpacingList.at(m_imageSpacingIndex));
#else
    // Ticket 21465: temporary to do this, Steven Cao, 8/31/2018
    ui->pb_frames->setEnabled(false);
    ui->pb_images->setEnabled(false);
    ui->pb_postbuffer->setEnabled(false);
    ui->pb_prebuffer->setEnabled(false);
    ui->pb_resolution->setEnabled(false);
#endif
}

void videoSetup::toggleValue(int cmd, int index, int /*f*/)
{
    switch (cmd) {
    case CMD_PREBUFFER:
    case CMD_POSTBUFFER:
        break;
    case CMD_PICTURDISTANCE:
        m_vkb->setNumKeyboard();
        break;
    default:
        baseMenu::toggleValue(cmd, index);

    }
}


void videoSetup::on_pb_images_clicked()
{
    if (++m_imagesIndex >= m_imagesList.size())
    {
        m_imagesIndex = 0;
    }
    ui->pb_images->setText(m_imagesList.at(m_imagesIndex));
}

void videoSetup::on_pb_prebuffer_clicked()
{
    if (++m_prebufferIndex >= m_prebufferList.size())
    {
        m_prebufferIndex = 0;
    }
    ui->pb_prebuffer->setText(m_prebufferList.at(m_prebufferIndex));
}

void videoSetup::on_pb_postbuffer_clicked()
{
    if (++m_postbufferIndex >= m_postbufferList.size())
    {
        m_postbufferIndex = 0;
    }
    ui->pb_postbuffer->setText(m_postbufferList.at(m_postbufferIndex));
}

void videoSetup::on_pb_illuminator_clicked()
{
#ifdef HH1
    if (++m_imageSpacingIndex >= m_imageSpacingList.size())
    {
        m_imageSpacingIndex = 0;
    }
    //ui->pb_illuminator->setText(m_imageSpacingList.at(m_imageSpacingIndex));
#endif
}
