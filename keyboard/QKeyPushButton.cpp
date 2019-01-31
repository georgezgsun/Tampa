/***************************************************************************
 *   Copyright (C) 2008 by Giovanni Romano                                 *
 *   Giovanni.Romano.76@gmail.com                                          *
 *                                                                         *
 *   This program may be distributed under the terms of the Q Public       *
 *   License as defined by Trolltech AS of Norway and appearing in the     *
 *   file LICENSE.QPL included in the packaging of this file.              *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 ***************************************************************************/


#include "QKeyPushButton.h"
#include "widgetKeyBoard.h"
#include <QKeyEvent>
#include <QCoreApplication>


QKeyPushButton::QKeyPushButton(QWidget *parent) : QPushButton(parent), m_parent(parent)
{
	this->setStyleSheet(QString(DEFAULT_STYLE_BUTTON) + QString(DEFAULT_BACKGROUND_BUTTON));
    connect(this, SIGNAL(pressedKey()), SLOT(getKeyPress()));
        if ((static_cast<widgetKeyBoard *> (parent))->isEmbeddedKeyboard() == true)
            this->style_embedded = QString(EMBEDDED_KEYBOARD);
}

void QKeyPushButton::getKeyPress()
{
	int 		keyCode = 0;
	QKeyEvent	*key = NULL;
	QString		text = this->text();
    if (text == "&&")
        text = "&";
	//
	// per tutti i car speciali tranne il CAPS (RETURN, ALT, SHIFT, ecc...) inoltra al componente widgetKeyBoard:
        if (NO_SPECIAL_KEY(text) == false && (IS_BACK(text) == true
                                              || IS_BACK_EMB(text) == true
                                              || IS_TAB(text) == true
                                              || IS_RETURN(text) == true
                                              || IS_CTRL_LEFT(text) == true
                                              || IS_ALT(text) == true
                                              || IS_CANC(text) == true
                                              || IS_CUT_LEFT(text) == true
                                              || IS_PASSWORD(text)
                                              || IS_PASSWORD_EMB(text)
                                              || IS_PASTE(text)
                                              || IS_COPY(text)
                                              || IS_CAPS(text)
                                              || IS_HIDE_KEYBOARD(text)
                                              || IS_SWITCH_ALPHA(text)
                                              || IS_SWITCH_NUM(text)))
		key = new QKeyEvent(QEvent::KeyPress, keyCode, Qt::NoModifier, text);
	else { // trattasi di caratteri stampabili
        key = new QKeyEvent(QEvent::KeyPress, keyCode, Qt::ShiftModifier, text);
	}
    ((widgetKeyBoard *) this->m_parent)->receiptChildKey(key, NULL);
    QCoreApplication::processEvents();
}


void QKeyPushButton::mousePressEvent(QMouseEvent * /*event */)
{
    widgetKeyBoard  *tmpKeyBoard = (widgetKeyBoard *) this->m_parent;
    QString         text = this->text();
    QString         defaultStyleButton = QString(DEFAULT_STYLE_BUTTON);
    QString         changedColorButton = QString(CHANGED_BACKGROUND_BUTTON);
    int signalFlag = 0;

    if (IS_CAPS(text) == true) {
        if (tmpKeyBoard->capsActive() == false)
            tmpKeyBoard->setCapsActive(true);
		else {
            tmpKeyBoard->setCapsActive(false);
		}
        signalFlag = 1;
	}
	else {
        this->setStyleSheet(defaultStyleButton + changedColorButton + this->style_embedded);
		this->repaint();
        signalFlag = 1;
	}
    if (tmpKeyBoard->isEmbeddedKeyboard() == true
            && tmpKeyBoard->isZoomFacilityEnable()
            && NO_SPECIAL_KEY(text)
            && text.trimmed().length() != 0) {
        tmpKeyBoard->setCursor(Qt::BlankCursor);

        //this->setStyleSheet(changedColorButton + defaultStyleButton);
        //this->repaint();
    }
    QCoreApplication::processEvents();
    if (signalFlag)
        emit pressedKey();
}

void QKeyPushButton::mouseReleaseEvent(QMouseEvent * /* event */)
{
    widgetKeyBoard  *tmpKeyBoard = (widgetKeyBoard *) this->m_parent;
    bool            pressedEcho = IS_PASSWORD(this->text()) == true || IS_PASSWORD_EMB(this->text()) == true;

    if (IS_CAPS(this->text()) == false && pressedEcho == false) {
        if (tmpKeyBoard->isEmbeddedKeyboard() == true
                && tmpKeyBoard->isZoomFacilityEnable()
                && NO_SPECIAL_KEY(this->text())
                && this->text().trimmed().length() != 0) {
            tmpKeyBoard->setCursor(Qt::ArrowCursor);
            QCoreApplication::processEvents();
        }        
        this->setStyleSheet(QString(DEFAULT_STYLE_BUTTON) + QString(DEFAULT_BACKGROUND_BUTTON) + this->style_embedded);
    }
    else if (pressedEcho == true
             && tmpKeyBoard->isEnabledSwitchingEcho() == false
             && tmpKeyBoard->currentTextBox()->echoMode() == QLineEdit::Normal)
        this->setStyleSheet(QString(DEFAULT_STYLE_BUTTON) + QString(DEFAULT_BACKGROUND_BUTTON) + this->style_embedded);
    this->repaint();
    tmpKeyBoard->soundClick();
}
