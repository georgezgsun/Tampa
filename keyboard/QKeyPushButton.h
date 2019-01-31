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

#include <QPushButton>

#ifndef _QKEYPUSHBUTTON_H
#define _QKEYPUSHBUTTON_H
        //sounds:
        #define CLICK_SOUND                     "/sounds/click.wav" // in future version of QT QSound will support loader from resource file
	//
	// colori e stile da utilizzare per la tastiera ed i tasti:

        #define DEFAULT_BACKGROUND_BUTTON	"background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #8C8F8C, stop: 1 #010101);"
        #define CHANGED_BACKGROUND_BUTTON	"background:lightgray;color:darkred;"
        //#define DEFAULT_BACKGROUND_BUTTON	"background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #f6f7fa, stop: 1 #dadbde);"
        //#define CHANGED_BACKGROUND_BUTTON	"background:lightblue;"

//        #define DEFAULT_STYLE_BUTTON            "color:white;border: 1px solid #010101;border-radius:1px;"   //6px
        #define DEFAULT_STYLE_BUTTON            "color:white;border: 1px solid #0A0A0A;border-radius:1px;"   //6px
        //#define DEFAULT_STYLE_BUTTON            "border: 1px solid #8f8f91;border-radius:5px;"
        #define STYLE_FOCUS_TEXT                "border: 1px solid red"

        //#define DEFAULT_STYLE_BUTTON		"color:white;border: 2px solid black;border-radius:5px;font-weight: bold;font-size:17px;"
        //#define DEFAULT_BACKGROUND_BUTTON	"background:gray;"
        //#define CHANGED_BACKGROUND_BUTTON	"background:red;"

        #define EMBEDDED_KEYBOARD               "font-size:14px"
        #define WIDTH_ZOOM_KEY                  20
        #define HEIGHT_ZOOM_KEY                 WIDTH_ZOOM_KEY
	//
	// tasti speciali utilizzati all'interno delle classi:
#ifndef KEY_TAB
        #define KEY_TAB				tr("TAB")
#endif
        #define KEY_ALT				tr("ALT")
        #define KEY_CAPS			tr("CAPS")
        #define KEY_CTRL_LEFT			tr("CTRL")
        #define KEY_CUT_LEFT			tr("cut")
#ifndef KEY_COPY			
        #define KEY_COPY			tr("copy")
#endif
#ifndef KEY_PASTE
        #define KEY_PASTE			tr("paste")
#endif
#ifndef KEY_BACKSPACE 
        #define KEY_BACKSPACE			tr("BACKSPACE")
#endif
        #define KEY_BACKSPACE_EMBEDDED		"<---"
        #define KEY_HIDECHAR                    tr("echo")
        #define KEY_HIDECHAR_EMBEDDED           tr("echo")
        #define KEY_CANC			tr("CANC-1")
        #define KEY_RETURN			tr("RETURN")
        #define KEY_SWITCH_NUM      tr("#12")
        #define KEY_SWITCH_ALPHA    tr("abc")
        #define KEY_HIDE_KEYBOARD   tr("CANC")
#ifndef KEY_SPACE 
        #define KEY_SPACE                       " "
#endif
        #define KEY_WIDTH_EMBEDDED              (480 / 10)
        #define KEY_HEIGHT_EMBEDDED             KEY_WIDTH_EMBEDDED
        //
        // macro parametrizzate per la riduzione del codice:
        #define IS_KEY(keyTextPressed, keyValCompare)   (keyTextPressed).contains((keyValCompare), Qt::CaseInsensitive)

        #define IS_TAB(text)                            IS_KEY(text, KEY_TAB)
        #define IS_ALT(text)                            IS_KEY(text, KEY_ALT)
        #define IS_CAPS(text)                           IS_KEY(text, KEY_CAPS)
        #define IS_CTRL_LEFT(text)                      IS_KEY(text, KEY_CTRL_LEFT)
        #define IS_CUT_LEFT(text)                       IS_KEY(text, KEY_CUT_LEFT)
        #define IS_BACK(text)                           IS_KEY(text, KEY_BACKSPACE)
        #define IS_BACK_EMB(text)                       IS_KEY(text, KEY_BACKSPACE_EMBEDDED)
        #define IS_CANC(text)                           IS_KEY(text, KEY_CANC)
        #define IS_RETURN(text)                         IS_KEY(text, KEY_RETURN)
        #define IS_SPACE(text)                          IS_KEY(text, KEY_SPACE)
        #define IS_PASSWORD(text)                       IS_KEY(text, KEY_HIDECHAR)
        #define IS_PASSWORD_EMB(text)                   IS_KEY(text, KEY_HIDECHAR_EMBEDDED)
        #define IS_COPY(text)                           IS_KEY(text, KEY_COPY)
        #define IS_PASTE(text)                          IS_KEY(text, KEY_PASTE)
        #define IS_SWITCH_NUM(text)                     IS_KEY(text, KEY_SWITCH_NUM)
        #define IS_SWITCH_ALPHA(text)                   IS_KEY(text, KEY_SWITCH_ALPHA)
        #define IS_HIDE_KEYBOARD(text)                  IS_KEY(text, KEY_HIDE_KEYBOARD)
        #define NO_SPECIAL_KEY(text)                    ((text).length() == 1)
	//
	//
	// definizione classe rappresentante un tasto:
	class QKeyPushButton : public QPushButton {
		Q_OBJECT
	
		public:
			QKeyPushButton(QWidget *parent = 0);
	
		private slots:
            void 		getKeyPress(void);
		
		signals:
            void 		pressedKey(void);

		private:
			QWidget		*m_parent;
                        QString         style_embedded;
                        int             m_oldYKey;
	
		protected:
			void 		mousePressEvent(QMouseEvent *event);
			void 		mouseReleaseEvent(QMouseEvent *event);
	};

#endif // _QKEYPUSHBUTTON_H
