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


#include "widgetKeyBoard.h"
#if QT_VERSION > 0x040806
#include <QGuiApplication>
#else
#include <QDesktopWidget>
#include <QApplication>
#endif
#include <QLayout>
#include <QScreen>
#include <QKeyEvent>
#include <QDir>
#include <QDebug>
#include "debug.h"

bool widgetKeyBoard::m_created = false;


widgetKeyBoard::widgetKeyBoard(bool embeddedKeyboard, QWidget *activeForm/*, QWidget *parent*/)
    : /* QWidget(parent), */
      m_nextInput(NULL),
      m_activeWindow(activeForm),
      m_currentTextBox(NULL),
      m_embeddedKeyboard(embeddedKeyboard),
      m_echoMode(false),
      m_zoomFacilityEmbedded(false),
      m_enablePasswordEcho(false),
      m_capsActive(true),
     // m_player(QDir::currentPath() + CLICK_SOUND),
#if QT_VERSION > 0x040806
	m_clipboard(QGuiApplication::clipboard())
#else
	m_clipboard(QApplication::clipboard())
#endif
{
    this->m_display = NULL;
    this->m_alphaKeyBoard = NULL;
    this->m_alphaKeyLoBoard = NULL;
    this->m_numbKeyBoard = NULL;
    this->m_keyBoardLayout = NULL;
    this->m_clipboard->clear();
    this->setWindowIcon(QPixmap(":/TastieraVirtuale/logo"));
    m_created = false;

   // this->setWindowFlags(Qt::WindowTitleHint | Qt::CustomizeWindowHint);
}

widgetKeyBoard::~widgetKeyBoard()
{
}


void widgetKeyBoard::enableSwitchingEcho(bool status)
{
    this->m_enablePasswordEcho = status;
}

bool widgetKeyBoard::isEnabledSwitchingEcho(void)
{
    return (this->m_enablePasswordEcho);
}

/*inline */ bool widgetKeyBoard::isEmbeddedKeyboard(void)
{
    return (this->m_embeddedKeyboard);
}

void widgetKeyBoard::setZoomFacility(bool active)
{
    this->m_zoomFacilityEmbedded = active;
}

bool widgetKeyBoard::isZoomFacilityEnable(void)
{
    return (this->m_zoomFacilityEmbedded);
}

QKeyPushButton * widgetKeyBoard::createNewKey(QWidget *p, QString keyValue)
{
    QKeyPushButton *tmp = new QKeyPushButton(p);
    QString        style = QString(DEFAULT_STYLE_BUTTON) + QString(DEFAULT_BACKGROUND_BUTTON);
    QSize          imageDim;
    //int            width = 0, height = 0;
    int width = KEY_WIDTH_EMBEDDED;
    int height = width;

    if (this->isEmbeddedKeyboard() == true) {
        imageDim.setWidth(16);
        imageDim.setHeight(16);
    }
    else {
        imageDim.setWidth(32);
        imageDim.setHeight(32);
    }
    if (IS_PASSWORD(keyValue) == true || IS_PASSWORD_EMB(keyValue) == true) {
        tmp->setIconSize(imageDim);
        tmp->setIcon(QPixmap(":/TastieraVirtuale/normalEcho"));
    }
    else if (IS_PASTE(keyValue) == true) {
        tmp->setIconSize(imageDim);
        tmp->setEnabled(false);
        tmp->setIcon(QPixmap(":/TastieraVirtuale/paste"));
    }
    else if (IS_COPY(keyValue) == true) {
        tmp->setIconSize(imageDim);
        tmp->setIcon(QPixmap(":/TastieraVirtuale/copy"));
    }
    else if (IS_CUT_LEFT(keyValue) == true) {
        tmp->setIconSize(imageDim);
        tmp->setIcon(QPixmap(":/TastieraVirtuale/cut"));
    }
    else if(IS_HIDE_KEYBOARD(keyValue) == true) {
        tmp->setIconSize(imageDim);
        tmp->setIcon(QPixmap(":/TastieraVirtuale/logo"));
    }
    if (keyValue == "&")
        tmp->setText("&&");
    else
        tmp->setText(keyValue);
    if (this->isEmbeddedKeyboard() == true) {
        style += QString(EMBEDDED_KEYBOARD);
    }

    tmp->setObjectName(keyValue);
    tmp->setMinimumSize(width, height);
    tmp->setMaximumSize(width, height);
    tmp->setStyleSheet(style);
    tmp->setVisible(true);
    return (tmp);
}

void widgetKeyBoard::controlKeyEcho(QLineEdit *control)
{
    QString         textToSearch;
    QKeyPushButton  *echoControlKey;
    QString         tmpStyle;
    QPushButton     *pasteButton = this->findChild<QPushButton *>(KEY_PASTE);
    QPushButton     *cutButton = this->findChild<QPushButton *>(KEY_CUT_LEFT);
    QPushButton     *copyButton = this->findChild<QPushButton *>(KEY_COPY);

    if (this->isEmbeddedKeyboard()) {
        textToSearch = KEY_HIDECHAR_EMBEDDED;
        tmpStyle = QString(EMBEDDED_KEYBOARD);
    }
    else
        textToSearch = KEY_HIDECHAR;
    if (pasteButton != NULL && copyButton != NULL) { // support for copy/paste functionality
        if (control->echoMode() == QLineEdit::Normal) {
            cutButton->setEnabled(true);
            copyButton->setEnabled(true);
            if (this->m_clipboard->text().length() > 0)
                pasteButton->setEnabled(true);
            else
                pasteButton->setEnabled(false);
        }
        else if (control->echoMode() == QLineEdit::Password) {
            copyButton->setEnabled(false);
            cutButton->setEnabled(false);
            pasteButton->setEnabled(false);
        }
    }
    echoControlKey = this->findChild<QKeyPushButton *>(textToSearch);
    if (echoControlKey != NULL) { // support for password echo functionality            
        if (control->echoMode() == QLineEdit::Normal) {            
            echoControlKey->setStyleSheet(QString(DEFAULT_STYLE_BUTTON) + QString(DEFAULT_BACKGROUND_BUTTON) +  tmpStyle); // segnalato come attivo
            echoControlKey->setIcon(QPixmap(":/TastieraVirtuale/normalEcho"));
        }
        else if (control->echoMode() == QLineEdit::Password) {            
            echoControlKey->setStyleSheet(QString(DEFAULT_STYLE_BUTTON) + QString(CHANGED_BACKGROUND_BUTTON) + tmpStyle); // segnalato come attivo
            echoControlKey->setIcon(QPixmap(":/TastieraVirtuale/passwdEcho"));
        }
        echoControlKey->repaint();
#if QT_VERSION > 0x040806
        QGuiApplication::processEvents();
#else 
        QApplication::processEvents();
#endif
    }
}

//
// riceve i caratteri che sono stati digitati:
void widgetKeyBoard::receiptChildKey(QKeyEvent *event, QLineEdit *focusThisControl, bool reset)
{
    static QLineEdit    *nextInput = NULL;

    if (reset == true) { // reinizializza il controllo
        nextInput = this->getNextTextbox(focusThisControl, true);
        return;
    }

    if (nextInput == NULL)
        return;
//
    // inizia l'analisi del carattere ricevuto:
    QString newKey = event->text();
    QString tmpReceiptString = m_display->text();
    int     tmpPos = m_display->cursorPosition();

    if (NO_SPECIAL_KEY(newKey) == false) {
        if (IS_RETURN(newKey) == true || IS_TAB(newKey) == true) { // trattasi di TAB, si sposta alla text successiva
            nextInput->setText(m_display->text());
            nextInput = this->setDefaultTextStyle(nextInput);
            nextInput->deselect();
            emit cancelKeyBoard();

#if 0
            nextInput = this->getNextTextbox();
            this->controlKeyEcho(nextInput); // status of key echo here
            if (nextInput != NULL) {
                nextInput->setCursorPosition(nextInput->text().length()); // comment this line if you want caret position at current char inserted
                nextInput->setFocus(Qt::TabFocusReason);
            }
#endif
        }
        else if (IS_BACK(newKey) == true || IS_BACK_EMB(newKey) == true) { // trattasi di CANCELLARE carattere, elimina il carattere a sinistra
            if (tmpPos-1 >= 0) {
                tmpReceiptString = tmpReceiptString.remove(tmpPos-1, 1);
                m_display->setText(tmpReceiptString);
                m_display->setCursorPosition(tmpPos-1);
                if (tmpPos-2 >= 0)
                    m_display->setSelection(tmpPos-2, 1);
            }
        }
        else if (IS_CANC(newKey) == true) { // trattasi di CANC carattere, elimina il carattere a destra
             tmpReceiptString = tmpReceiptString.remove(tmpPos, 1);
             nextInput->setText(tmpReceiptString);
             nextInput->setCursorPosition(tmpPos);
             nextInput->setSelection(tmpPos-1, 1);
        }
        else if (IS_COPY(newKey) == true || IS_CUT_LEFT(newKey) == true) {
            QPushButton *button = this->findChild<QPushButton *>(KEY_PASTE);

            if (button != NULL) {
                if (nextInput->text().length() != 0) {
                    this->m_clipboard->setText(nextInput->text());
                    if (IS_CUT_LEFT(newKey) == true)
                        nextInput->setText("");
                    button->setEnabled(true);
                }
                else
                    button->setEnabled(false);
            }
        }
        else if (IS_PASTE(newKey) == true)
            nextInput->setText(this->m_clipboard->text());
        else if (IS_ALT(newKey) == true || IS_CTRL_LEFT(newKey) == true)
        {
            ; // non esegue nessuna operazione
        }
        else if (IS_HIDE_KEYBOARD(newKey)) {
            emit cancelKeyBoard();
        }
        else if (IS_SWITCH_ALPHA(newKey)) {
            if (this->m_capsActive == false)
                m_keyBoardLayout->setCurrentIndex(KB_ALPHA_LO_INDEX);
            else
                m_keyBoardLayout->setCurrentIndex(KB_ALPHA_HI_INDEX);
        }
        else if (IS_SWITCH_NUM(newKey)) {
            m_keyBoardLayout->setCurrentIndex(KB_NUM_INDEX);
        }
        else if (IS_CAPS(newKey)) {
            if (this->m_capsActive == false)
                m_keyBoardLayout->setCurrentIndex(KB_ALPHA_LO_INDEX);
            else
                m_keyBoardLayout->setCurrentIndex(KB_ALPHA_HI_INDEX);
        }
    }
    else { // si tratta di un carattere da visualizzare nella casella di testo corrente
        tmpReceiptString = tmpReceiptString.insert(tmpPos, newKey);
        m_display->setText(tmpReceiptString);
        m_display->setCursorPosition(tmpPos+1);
        m_display->setSelection(tmpPos, 1);
    }
}


void widgetKeyBoard::switchKeyEchoMode(QLineEdit *control)
{
    this->setStatusEchoMode(!this->statusEchoMode());
    if (this->statusEchoMode() == true)
        control->setEchoMode(QLineEdit::Password);
    else
        control->setEchoMode(QLineEdit::Normal);
    this->controlKeyEcho(control);
}


QLineEdit * widgetKeyBoard::getNextTextbox(QLineEdit *thisControl, bool reset)
{
    QLineEdit	*tmpInputFound = NULL;

    this->m_currentTextBox = NULL;
    if (this->m_activeWindow == NULL) { // nessuna finestra principale su cui operare
        m_nextInput = NULL;
        return (NULL);
    }
    if (thisControl != NULL) {
        if (this->m_activeWindow->findChild<QLineEdit *>(thisControl->objectName()) == 0) // verifies that thisControl is a child of m_activeWindow
            return (NULL);
        else // it's true!
            return (this->setCurrentTextStyle(thisControl));
    }
    //
    // cicla nella catena dei controlli in ordine di focus per ottenere la QEditLine
    if (m_nextInput == NULL || reset == true)
        m_nextInput = this->m_activeWindow->nextInFocusChain();
    else
        m_nextInput = m_nextInput->nextInFocusChain();
    do {
        if (m_nextInput->inherits("QLineEdit") == true) { // trovata una casella di testo da utilizzare
            tmpInputFound = dynamic_cast<QLineEdit *>(m_nextInput);
            return (this->setCurrentTextStyle(tmpInputFound));
        }
        m_nextInput = m_nextInput->nextInFocusChain();
    } while (m_nextInput != NULL);
    return (NULL);
}


QLineEdit * widgetKeyBoard::setCurrentTextStyle(QLineEdit *control)
{
    this->m_currentTextBox = control;
    control->setStyleSheet(STYLE_FOCUS_TEXT);
    return (control);
}

QLineEdit * widgetKeyBoard::setDefaultTextStyle(QLineEdit *control)
{
    control->setStyleSheet(this->m_noFocusPalette.styleSheet());
    return (control);
}

void widgetKeyBoard::init_keyboard(QLineEdit *focusThisControl)
{
    widgetKeyBoard::createKeyboard();
  
	if (focusThisControl) {
        QString s = focusThisControl->placeholderText();
        m_display->setPlaceholderText(s);
        m_display->setEchoMode(focusThisControl->echoMode());
        m_display->setText(focusThisControl->text());
        m_display->setAlignment(Qt::AlignTop);
    }
    this->m_currentTextBox = NULL;
    this->m_nextInput = focusThisControl;
    this->receiptChildKey(NULL, focusThisControl, true);
}

void widgetKeyBoard::soundClick(void)
{
    //this->m_player.play();
}

void widgetKeyBoard::show(QWidget *activeForm)
{
    this->m_activeWindow = activeForm;
    this->init_keyboard(NULL);
    if (this->windowState() == Qt::WindowMinimized)
        this->setWindowState(Qt::WindowNoState);
    this->setStatusEchoMode(false);
    this->m_clipboard->clear();
#if QT_VERSION > 0x040806
    this->move(20, QGuiApplication::screens()[0]->availableGeometry().height() - this->height() - 200);
#else
    QDesktopWidget *dt = QApplication::desktop();
    this->move(20, dt->availableGeometry().height() - this->height() - 200);
#endif
    this->borderFrame(true);
    QWidget::show();
}

void widgetKeyBoard::show(QLineEdit *l, QWidget *activeForm)
{
    this->m_activeWindow = activeForm;
    this->init_keyboard(l);
    if (this->windowState() == Qt::WindowMinimized)
        this->setWindowState(Qt::WindowNoState);
    this->setStatusEchoMode(false);
    this->m_clipboard->clear();
#if 0
#if QT_VERSION > 0x040806
    this->move(20, QGuiApplication::screens()[0]->availableGeometry().height() - this->height() - 200);
#else
    QDesktopWidget *dt = QApplication::desktop();
    this->move(20, dt->availableGeometry().height() - this->height() - 200);
#endif
#endif
    this->borderFrame(true);
    QWidget::show();
}

void widgetKeyBoard::setActiveForms(QLineEdit *l, QWidget *activeForm)
{
    if (this->m_activeWindow != activeForm)
        m_activeWindow = activeForm;

    if (this->m_currentTextBox != NULL)
        this->m_currentTextBox = this->setDefaultTextStyle(this->m_currentTextBox);

    this->init_keyboard(l);
}

void widgetKeyBoard::borderFrame(bool visible)
{
    if (visible == true)
    //    this->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::MSWindowsFixedSizeDialogHint);
    //else
        this->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::MSWindowsFixedSizeDialogHint | Qt::CustomizeWindowHint);
}


void widgetKeyBoard::focusThis(QLineEdit *control)
{
    if (this->m_activeWindow != NULL) {
        if (this->m_currentTextBox != NULL)
            this->m_currentTextBox = this->setDefaultTextStyle(this->m_currentTextBox);
        this->init_keyboard(control);
        this->controlKeyEcho(control);
    }
    else {
        ;// Non è attiva nessuna finestra su cui operare
    }
}

void widgetKeyBoard::closeEvent(QCloseEvent * event)
{
    event->ignore();
}


void widgetKeyBoard::hide(bool changeColor)
{
    try {
        if (changeColor == true)
            if (this->m_activeWindow != NULL && this->m_currentTextBox != NULL)
                this->m_currentTextBox = this->setDefaultTextStyle(this->m_currentTextBox);
        this->m_currentTextBox = NULL;
        this->m_activeWindow = NULL;
        setVisible(false);
        emit VKBHided();
    }
    catch (...) {
    }
}


void widgetKeyBoard::createKeyboard(void)
{
	if (widgetKeyBoard::m_created == true) {
	  DEBUG()<< "m_created already true" ;
	  return;
	}
    widgetKeyBoard::m_created = true;

    m_keyBoardLayout = new QStackedLayout ();
    m_keyBoardLayout->setMargin(0);

    this->createAlphaHiKeys();
    this->createAlphaLoKeys();
    this->createNumKeys();

    m_keyBoardLayout->setCurrentIndex(KB_ALPHA_HI_INDEX);

    m_kbViewLayout = new QVBoxLayout (this);
    m_kbViewLayout->setMargin(0);
    m_display = new QLineEdit();
   // m_display->setFixedHeight(80);
    m_display->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_display->setStyleSheet("color: #0a0a0a");
    m_kbViewLayout->addWidget(m_display, 1);
    m_kbViewLayout->addLayout(m_keyBoardLayout);

    //qDebug()<< "layout cout = " << m_keyBoardLayout->count();
}

void widgetKeyBoard::createAlphaHiKeys(void)
{
    m_alphaKeyBoard = new QWidget();
    QWidget *p = m_alphaKeyBoard;

    QKeyPushButton	*tmp = NULL;
    QVBoxLayout     *tmpVLayout = new QVBoxLayout(p);
    QHBoxLayout     *tmpLayout;
    QString         tmpStyle = QString::null;

    if (widgetKeyBoard::m_created != true) // tastiera già  creata: esce
                return;

    int halfWidth = KEY_WIDTH_EMBEDDED / 2;
    int oneAndHalfWidth = KEY_WIDTH_EMBEDDED + halfWidth;

    p = this;

    tmpLayout = new QHBoxLayout;
    tmpLayout->setSpacing(0);

    tmpLayout->addWidget(createNewKey(p, tr("Q")));
    tmpLayout->addWidget(createNewKey(p, tr("W")));
    tmpLayout->addWidget(createNewKey(p, tr("E")));
    tmpLayout->addWidget(createNewKey(p, tr("R")));
    tmpLayout->addWidget(createNewKey(p, tr("T")));
    tmpLayout->addWidget(createNewKey(p, tr("Y")));
    tmpLayout->addWidget(createNewKey(p, tr("U")));
    tmpLayout->addWidget(createNewKey(p, tr("I")));
    tmpLayout->addWidget(createNewKey(p, tr("O")));
    tmpLayout->addWidget(createNewKey(p, tr("P")));

    tmpVLayout->insertLayout(0, tmpLayout);

#if !defined(QT_ARCH_MACOSX)
    //tmpLayout->insertStretch(-1, 1);
#endif

    tmpLayout = new QHBoxLayout;
    tmpLayout->setSpacing(0);

    tmpLayout->addSpacing(halfWidth);
    tmpLayout->addWidget(createNewKey(p, tr("A")));
    tmpLayout->addWidget(createNewKey(p, tr("S")));
    tmpLayout->addWidget(createNewKey(p, tr("D")));
    tmpLayout->addWidget(createNewKey(p, tr("F")));
    tmpLayout->addWidget(createNewKey(p, tr("G")));
    tmpLayout->addWidget(createNewKey(p, tr("H")));
    tmpLayout->addWidget(createNewKey(p, tr("J")));
    tmpLayout->addWidget(createNewKey(p, tr("K")));
    tmpLayout->addWidget(createNewKey(p, tr("L")));
    tmpLayout->addSpacing(halfWidth);
    tmpVLayout->insertLayout(1, tmpLayout);

    //
    // Stampa linea della "Z":
    tmpLayout = new QHBoxLayout;
    tmpLayout->setSpacing(0);

    tmp = createNewKey(p, KEY_CAPS);
    tmp->setMaximumWidth(oneAndHalfWidth);
    tmp->setMinimumWidth(oneAndHalfWidth);
    if (this->isEmbeddedKeyboard() == true)
        tmpStyle = QString(EMBEDDED_KEYBOARD);
    tmp->setStyleSheet(QString(DEFAULT_STYLE_BUTTON) + QString(CHANGED_BACKGROUND_BUTTON) + tmpStyle); // segnalato come attivo
    tmpLayout->addWidget(tmp);

    tmpLayout->addWidget(createNewKey(p, tr("Z")));
    tmpLayout->addWidget(createNewKey(p, tr("X")));
    tmpLayout->addWidget(createNewKey(p, tr("C")));
    tmpLayout->addWidget(createNewKey(p, tr("V")));
    tmpLayout->addWidget(createNewKey(p, tr("B")));
    tmpLayout->addWidget(createNewKey(p, tr("N")));
    tmpLayout->addWidget(createNewKey(p, tr("M")));
    if (this->isEmbeddedKeyboard() == true)
        tmpStyle = QString(KEY_BACKSPACE_EMBEDDED);
    else
        tmpStyle = QString(KEY_BACKSPACE);
    tmp = createNewKey(p, tmpStyle);
    tmp->setMaximumWidth(oneAndHalfWidth);
    tmp->setMinimumWidth(oneAndHalfWidth);
    tmpLayout->addWidget(tmp);

    tmpVLayout->insertLayout(2, tmpLayout);

    //
    // Stampa linea dello SPACE:
    tmpLayout = new QHBoxLayout;
    tmpLayout->setSpacing(0);

    tmp = createNewKey(p, KEY_SWITCH_NUM);
    tmp->setMaximumWidth(KEY_WIDTH_EMBEDDED * 2);
    tmp->setMinimumWidth(KEY_WIDTH_EMBEDDED * 2);
    tmpLayout->addWidget(tmp);

    tmp = createNewKey(p, KEY_SPACE);
    tmp->setMaximumWidth(tmp->maximumWidth() * 6);
    tmp->setMinimumWidth(tmp->minimumWidth() * 6);
    tmpLayout->addWidget(tmp);

    tmp = createNewKey(p, KEY_RETURN);
    tmp->setMaximumWidth(tmp->maximumWidth() * 2);
    tmp->setMinimumWidth(tmp->minimumWidth() * 2);
    tmpLayout->addWidget(tmp);
    tmpVLayout->insertLayout(3, tmpLayout);

    tmpVLayout->setMargin(0);
    tmpVLayout->setSpacing(0);

    m_keyBoardLayout->insertWidget(KB_ALPHA_HI_INDEX, m_alphaKeyBoard);
    //m_keyBoardLayout->addWidget(m_alphaKeyBoard);
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}


void widgetKeyBoard::createAlphaLoKeys(void)
{
    m_alphaKeyLoBoard = new QWidget();
    QWidget *p = m_alphaKeyLoBoard;

    QKeyPushButton	*tmp = NULL;
    QVBoxLayout     *tmpVLayout = new QVBoxLayout(p);
    QHBoxLayout     *tmpLayout;
    QString         tmpStyle = QString::null;

    if (widgetKeyBoard::m_created != true) // tastiera già  creata: esce
                return;

    int halfWidth = KEY_WIDTH_EMBEDDED / 2;
    int oneAndHalfWidth = KEY_WIDTH_EMBEDDED + halfWidth;

    p = this;

	tmpLayout = new QHBoxLayout;	
    tmpLayout->setSpacing(0);

    tmpLayout->addWidget(createNewKey(p, tr("q")));
    tmpLayout->addWidget(createNewKey(p, tr("w")));
    tmpLayout->addWidget(createNewKey(p, tr("e")));
    tmpLayout->addWidget(createNewKey(p, tr("r")));
    tmpLayout->addWidget(createNewKey(p, tr("t")));
    tmpLayout->addWidget(createNewKey(p, tr("y")));
    tmpLayout->addWidget(createNewKey(p, tr("u")));
    tmpLayout->addWidget(createNewKey(p, tr("i")));
    tmpLayout->addWidget(createNewKey(p, tr("o")));
    tmpLayout->addWidget(createNewKey(p, tr("p")));

    tmpVLayout->insertLayout(0, tmpLayout);

#if !defined(QT_ARCH_MACOSX)
    //tmpLayout->insertStretch(-1, 1);
#endif

	//
	// Stampa linea della "A":
	tmpLayout = new QHBoxLayout;	
    tmpLayout->setSpacing(0);

    tmpLayout->addSpacing(halfWidth);
    tmpLayout->addWidget(createNewKey(p, tr("a")));
    tmpLayout->addWidget(createNewKey(p, tr("s")));
    tmpLayout->addWidget(createNewKey(p, tr("d")));
    tmpLayout->addWidget(createNewKey(p, tr("f")));
    tmpLayout->addWidget(createNewKey(p, tr("g")));
    tmpLayout->addWidget(createNewKey(p, tr("h")));
    tmpLayout->addWidget(createNewKey(p, tr("j")));
    tmpLayout->addWidget(createNewKey(p, tr("k")));
    tmpLayout->addWidget(createNewKey(p, tr("l")));
    tmpLayout->addSpacing(halfWidth);
    tmpVLayout->insertLayout(1, tmpLayout);

	//	
	// Stampa linea della "Z":
    tmpLayout = new QHBoxLayout;
    tmpLayout->setSpacing(0);

    tmp = createNewKey(p, KEY_CAPS);
    tmp->setMaximumWidth(oneAndHalfWidth);
    tmp->setMinimumWidth(oneAndHalfWidth);
    if (this->isEmbeddedKeyboard() == true)
        tmpStyle = QString(EMBEDDED_KEYBOARD);
    tmpLayout->addWidget(tmp);

    tmpLayout->addWidget(createNewKey(p, tr("z")));
    tmpLayout->addWidget(createNewKey(p, tr("x")));
    tmpLayout->addWidget(createNewKey(p, tr("c")));
    tmpLayout->addWidget(createNewKey(p, tr("v")));
    tmpLayout->addWidget(createNewKey(p, tr("b")));
    tmpLayout->addWidget(createNewKey(p, tr("n")));
    tmpLayout->addWidget(createNewKey(p, tr("m")));
    if (this->isEmbeddedKeyboard() == true)
        tmpStyle = QString(KEY_BACKSPACE_EMBEDDED);
    else
        tmpStyle = QString(KEY_BACKSPACE);
    tmp = createNewKey(p, tmpStyle);
    tmp->setMaximumWidth(oneAndHalfWidth);
    tmp->setMinimumWidth(oneAndHalfWidth);
    tmpLayout->addWidget(tmp);

	tmpVLayout->insertLayout(2, tmpLayout);

    //
	// Stampa linea dello SPACE:
	tmpLayout = new QHBoxLayout;
    tmpLayout->setSpacing(0);

    tmp = createNewKey(p, KEY_SWITCH_NUM);
    tmp->setMaximumWidth(KEY_WIDTH_EMBEDDED * 2);
    tmp->setMinimumWidth(KEY_WIDTH_EMBEDDED * 2);
	tmpLayout->addWidget(tmp);

    tmp = createNewKey(p, KEY_SPACE);
    tmp->setMaximumWidth(tmp->maximumWidth() * 6);
    tmp->setMinimumWidth(tmp->minimumWidth() * 6);
	tmpLayout->addWidget(tmp);

    tmp = createNewKey(p, KEY_RETURN);
    tmp->setMaximumWidth(tmp->maximumWidth() * 2);
    tmp->setMinimumWidth(tmp->minimumWidth() * 2);
	tmpLayout->addWidget(tmp);
	tmpVLayout->insertLayout(3, tmpLayout);

    tmpVLayout->setMargin(0);
    tmpVLayout->setSpacing(0);

    m_keyBoardLayout->insertWidget(KB_ALPHA_LO_INDEX, m_alphaKeyLoBoard);
}

void widgetKeyBoard::createNumKeys()
{
    m_numbKeyBoard = new QWidget();
    QWidget *p = m_numbKeyBoard;

    QKeyPushButton  *tmp = NULL;
    QVBoxLayout     *tmpVLayout = new QVBoxLayout (p);
    QHBoxLayout     *tmpLayout;
    QString         tmpStyle = QString::null;

    if (widgetKeyBoard::m_created != true) // tastiera già  creata: esce
                return;

    int halfWidth = KEY_WIDTH_EMBEDDED / 2;
    int oneAndHalfWidth = KEY_WIDTH_EMBEDDED + halfWidth;

    p = this;

    tmpLayout = new QHBoxLayout;
    tmpLayout->setSpacing(0);
    for (short i = 49; i <= 57; i++) {
        tmpLayout->addWidget(createNewKey(p, QChar(i)));
    }
    tmpLayout->addWidget(createNewKey(p, tr("0")));
    tmpVLayout->insertLayout(0, tmpLayout);

    tmpLayout = new QHBoxLayout;
    tmpLayout->setSpacing(0);
    tmpLayout->addWidget(createNewKey(p, tr("\\")));
    tmpLayout->addWidget(createNewKey(p, tr("!")));
    tmpLayout->addWidget(createNewKey(p, tr("@")));
    tmpLayout->addWidget(createNewKey(p, tr("#")));
    tmpLayout->addWidget(createNewKey(p, tr("$")));
    tmpLayout->addWidget(createNewKey(p, tr("%")));
    tmpLayout->addWidget(createNewKey(p, tr("&")));
    tmpLayout->addWidget(createNewKey(p, tr("+")));
    tmpLayout->addWidget(createNewKey(p, tr("?")));
    tmpLayout->addWidget(createNewKey(p, tr("/")));
    tmpVLayout->insertLayout(1, tmpLayout);

    tmpLayout = new QHBoxLayout;
    tmpLayout->setSpacing(0);
    tmpLayout->addWidget(createNewKey(p, tr("*")));
    tmpLayout->addWidget(createNewKey(p, tr("_")));
    tmpLayout->addWidget(createNewKey(p, tr("\"")));
    tmpLayout->addWidget(createNewKey(p, tr("'")));
    tmpLayout->addWidget(createNewKey(p, tr("(")));
    tmpLayout->addWidget(createNewKey(p, tr(")")));
    tmpLayout->addWidget(createNewKey(p, tr("-")));
    tmpLayout->addWidget(createNewKey(p, tr(":")));
    if (this->isEmbeddedKeyboard() == true)
        tmpStyle = QString(KEY_BACKSPACE_EMBEDDED);
    else
        tmpStyle = QString(KEY_BACKSPACE);
    tmp = createNewKey(p, tmpStyle);
    tmp->setMaximumWidth(tmp->maximumWidth() * 2);
    tmp->setMinimumWidth(tmp->minimumWidth() * 2);
    tmpLayout->addWidget(tmp);
    tmpVLayout->insertLayout(2, tmpLayout);

    tmpLayout = new QHBoxLayout;
    tmpLayout->setSpacing(0);
    tmp = createNewKey(p, KEY_SWITCH_ALPHA);
    tmp->setMaximumWidth(KEY_WIDTH_EMBEDDED * 2);
    tmp->setMinimumWidth(KEY_WIDTH_EMBEDDED * 2);
    tmpLayout->addWidget(tmp);
    tmpLayout->addWidget(createNewKey(p, tr(";")));
    tmpLayout->addWidget(createNewKey(p, tr(",")));

    tmp = createNewKey(p, KEY_SPACE);
    tmp->setMaximumWidth(tmp->maximumWidth() * 3.5);
    tmp->setMinimumWidth(tmp->minimumWidth() * 3.5);
    tmpLayout->addWidget(tmp);

    tmpLayout->addWidget(createNewKey(p, tr(".")));

    tmp = createNewKey(p, KEY_RETURN);
    tmp->setMaximumWidth(oneAndHalfWidth);
    tmp->setMinimumWidth(oneAndHalfWidth);
    tmpLayout->addWidget(tmp);
    tmpVLayout->insertLayout(3, tmpLayout);

    tmpVLayout->setMargin(0);
    tmpVLayout->setSpacing(0);

    m_keyBoardLayout->insertWidget(KB_NUM_INDEX, m_numbKeyBoard);
    //m_keyBoardLayout->addWidget(m_numbKeyBoard);
}
