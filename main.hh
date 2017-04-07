#ifndef P2PAPP_MAIN_HH
#define P2PAPP_MAIN_HH

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QUdpSocket>
#include <QString>
#include <QDataStream>
#include <QHostAddress>
#include <QVariant>

#include "netsocket.hh"

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    ChatDialog();

public slots:
    void gotReturnPressed();
    void recvData();

private:
    QTextEdit *textview;
    QLineEdit *textline;
    NetSocket *mySocket;
};

#endif // P2PAPP_MAIN_HH
