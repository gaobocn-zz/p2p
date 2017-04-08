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
    void sendMsg(const QVariantMap &sendMsgMap, const quint16 &port);
    void sendRM() { sendMsg(rmMsg, rmDestPort); } // send rumor msg
    void sendSM(); // send status msg
    void makeRM(QString &id, quint32 seqNo); // make rumor msg
    // void makeSM(); // make status msg

public slots:
    void gotReturnPressed();
    void recvData();

private:
    QTextEdit *textview;
    QLineEdit *textline;

    NetSocket *mySocket;

    QString myID;
    quint32 mySeqNo;

    // <host id, [messages]>
    QMap<QString, QVector<QString> > msgTable;
    // msgNoTable[id] == msgTable[id].size() + 1
    QVariantMap msgNoTable;

    // Used by sendRM
    quint16 rmDestPort;
    QVariantMap rmMsg;

    // Used by sendSM
    quint16 smDestPort;
    QVariantMap smMsg;
};

#endif // P2PAPP_MAIN_HH
