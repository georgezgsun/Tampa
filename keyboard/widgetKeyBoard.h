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


#ifndef _WIDGETKEYBOARD_H
	
    #define _WIDGETKEYBOARD_H

    #include "QKeyPushButton.h"
    #include <QSound>
    #include <QLineEdit>
    #include <QClipboard>
    #include <QStackedLayout>
    #include <QWidget>

    #define KB_ALPHA_HI_INDEX   0
    #define KB_ALPHA_LO_INDEX   1
    #define KB_NUM_INDEX        2

    //
    //
    // classe che crea e controlla la tastiera:
    class widgetKeyBoard : public QWidget {
        Q_OBJECT

        public:
		widgetKeyBoard(bool embeddedKeyboard = false, QWidget *activeForm = NULL/*, QWidget *parent = NULL*/);
            ~widgetKeyBoard();

            void            createKeyboard(void);
            bool            isEmbeddedKeyboard(void);
            void            soundClick(void);
            void            setZoomFacility(bool active); // only valid for embedded keyboard
            bool            isZoomFacilityEnable(void);
            bool            statusEchoMode(void){return (this->m_echoMode);}
            void            setStatusEchoMode(bool echo) {this->m_echoMode = echo; }
            void            receiptChildKey(QKeyEvent *event, QLineEdit *focusThisControl, bool reset = false); // accoglie i tasti premuti
            QLineEdit *     currentTextBox(void) { return (this->m_currentTextBox);}
            void            switchKeyEchoMode(QLineEdit *control);
            void            enableSwitchingEcho(bool status); // if you don't want control echo from keyboard
            bool            isEnabledSwitchingEcho(void); // current status
            void            borderFrame(bool visible = true);
            bool            capsActive(void) { return this->m_capsActive; }
            void            setCapsActive(bool b) { this->m_capsActive = b; }
            void            setActiveForms(QLineEdit *l, QWidget *activeForm);

            void            setNumKeyboard(void) {m_keyBoardLayout->setCurrentIndex(KB_NUM_INDEX);}

        public slots:
            void            show(QWidget *activeForm);
            void            show(QLineEdit *l, QWidget *activeForm);
            void            hide(bool noChangeColor);
            void            focusThis(QLineEdit *control);

        signals:
            void            cancelKeyBoard();
            void            VKBHided();

        protected:
            virtual void    closeEvent(QCloseEvent * event);

        private:
         //   widgetKeyBoard(const widgetKeyBoard&);
          //  widgetKeyBoard& operator=(const widgetKeyBoard&);

            void            init_keyboard(QLineEdit *focusThisControl); // reinizializza le funzioni base della tastiera
            QLineEdit *     setCurrentTextStyle(QLineEdit *control);
            QLineEdit *     setDefaultTextStyle(QLineEdit *control);
            QKeyPushButton *createNewKey(QWidget *parent, QString keyValue);
            QLineEdit *		getNextTextbox(QLineEdit *thisControl = NULL, bool reset = false);
            void            controlKeyEcho(QLineEdit *control);
            void            createNumKeys(void);
            void            createAlphaHiKeys(void);
            void            createAlphaLoKeys(void);

        private:
            QLineEdit		m_noFocusPalette; // usata per ripristinare la linetext senza focus
            QWidget         *m_nextInput; // punta alla textbox attualmente in focus
            QWidget         *m_activeWindow;
            QLineEdit		*m_currentTextBox; // mantiene il riferimento alla casella correntemente in uso
            static bool     m_created;
            bool            m_embeddedKeyboard;
            bool            m_echoMode; // status of current text object for echo
            bool            m_zoomFacilityEmbedded;
            bool            m_enablePasswordEcho; // controls the possibility to change among normal/password echo
            bool            m_capsActive;
            //QSound          m_player;
            QClipboard      *m_clipboard;
            QLineEdit       *m_display;
            QWidget         *m_alphaKeyBoard;
            QWidget         *m_numbKeyBoard;
            QWidget         *m_alphaKeyLoBoard;
            QStackedLayout  *m_keyBoardLayout;
            QVBoxLayout     *m_kbViewLayout;
    };

#endif // _WIDGETKEYBOARD_H
