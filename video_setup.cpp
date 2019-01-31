#include "video_setup.h"
#include "ui_video_setup.h"
#include "state.h"
#include "utils.h"
#include "illuminator.h"

videoSetup::videoSetup(QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::videoSetup)
{
    ui->setupUi(this);

    this->initLists();
    this->buildHashTables();
    this->setInittoggleValues();

	state& v = state::get();
    v.setState(STATE_VIDEO_SETUP);
    m_listIndex = m_prevListIndex = 0;
#ifdef HH1
    m_command = CMD_NONE;
#else
    m_command = m_cmdList.at(m_listIndex);
#endif
}

videoSetup::~videoSetup()
{
    bool flag1 = false; // Assumed no modification
    if (mOldFramesIndex != m_framesIndex)
    {// Frames changed
        if (m_framesIndex == 1)
            mConf.frames = 10;
        else if (m_framesIndex == 2)
            mConf.frames = 15;
        else if (m_framesIndex == 3)
            mConf.frames = 30;
        else
            mConf.frames = 0;
        flag1 = true;
    }

    // Resolution
    if (mOldResolutionIndex != m_resolutionIndex)
    {
        mConf.resolution = m_resolutionIndex;
        flag1 = true;
    }

    // Image(s) per file
    if (mOldImagesIndex != m_imagesIndex)
    {
        mConf.imagesPerFile = m_imagesIndex;
        flag1 = true;
    }

    if (mOldPrebufferIndex != m_prebufferIndex)
    {   // Pre-event buffer changed
        if (!m_prebufferIndex)
            mConf.preBuf = 1;
        else if (m_prebufferIndex == 1)
            mConf.preBuf = 2;
        else if (m_prebufferIndex == 2)
            mConf.preBuf = 5;
        else
            mConf.preBuf = 10;
        flag1 = true;
    }
    if (mOldPostbufferIndex != m_postbufferIndex)
    {   // Pre-event buffer changed
        if (!m_postbufferIndex)
            mConf.postBuf = 1;
        else if (m_postbufferIndex == 1)
            mConf.postBuf = 2;
        else if (m_postbufferIndex == 2)
            mConf.postBuf = 5;
        else
            mConf.postBuf = 10;
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
#ifndef HH1
    m_list << ui->pb_illuminator;

    m_cmdList << CMD_ILLIMINATOR;

    this->connectWidgetSigs();
#endif
}

void videoSetup::buildHashTables()
{
    m_imagesList << "1 Image/File" << "2 Images/File" << "3 Images/File";
    m_resolutionList << "LOW Resolution" << "MED Resolution" << "HIGH Resolution";
    m_prebufferList << "1 Second" << "2 Seconds" << "5 Seconds" << "10 Seconds";
    m_postbufferList << "1 Second" << "2 Seconds" << "5 Seconds" << "10 Seconds";
#ifdef HH1
    m_framesList << "5 Frames/Sec" << "10 Frames/Sec" << "15 Frames/Sec";
    m_imageSpacingList << "Shorter" << "Normal" << "Longer";
#else
    m_framesList << "5 Frames/Sec" << "10 Frames/Sec" << "15 Frames/Sec" << "30 Frames/Sec";
#endif
}

void videoSetup::setInittoggleValues()
{
    mConf = Utils::get().getConfiguration();

    // Check Frames
    switch (mConf.frames)
    {
        case 10:
            m_framesIndex = 1;
            break;
        case 15:
            m_framesIndex = 2;
            break;
        case 30:
            m_framesIndex = 3;
            break;
        default:
            m_framesIndex = 0;
    }
    mOldFramesIndex = m_framesIndex;

    // Resolution
    mOldResolutionIndex = m_resolutionIndex = mConf.resolution;

    // Image(s) per file
    mOldImagesIndex = m_imagesIndex = mConf.imagesPerFile;

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
            m_prebufferIndex = 3;
    }
    mOldPrebufferIndex = m_prebufferIndex;

    // Check Post-event Buffer
    switch (mConf.postBuf)
    {
        case 1:
            m_postbufferIndex = 0;
            break;
        case 2:
            m_postbufferIndex = 1;
            break;
        case 5:
            m_postbufferIndex = 2;
            break;
        default:
            m_postbufferIndex = 3;
    }

    ui->pb_frames->setText(m_framesList.at(m_framesIndex));
    ui->pb_resolution->setText(m_resolutionList.at(m_resolutionIndex));
    ui->pb_images->setText(m_imagesList.at(m_imagesIndex));
    ui->pb_prebuffer->setText(m_prebufferList.at(m_prebufferIndex));
    ui->pb_postbuffer->setText(m_postbufferList.at(m_postbufferIndex));

#ifdef HH1
    // Dist Between Images
    mOldImageSpacingIndex = m_imageSpacingIndex = mConf.imageSpacing;
    ui->pb_illuminator->setText(m_imageSpacingList.at(m_imageSpacingIndex));
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
    default:
        baseMenu::toggleValue(cmd, index);

    }
}

void videoSetup::on_pb_frames_clicked()
{
    if (++m_framesIndex >= m_framesList.size())
    {
        m_framesIndex = 0;
    }
    ui->pb_frames->setText(m_framesList.at(m_framesIndex));
}

void videoSetup::on_pb_resolution_clicked()
{
    if (++m_resolutionIndex >= m_resolutionList.size())
    {
        m_resolutionIndex = 0;
    }
    ui->pb_resolution->setText(m_resolutionList.at(m_resolutionIndex));
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
    ui->pb_illuminator->setText(m_imageSpacingList.at(m_imageSpacingIndex));
#endif
}
