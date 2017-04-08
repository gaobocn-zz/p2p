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
    // send rumor msg, make it this way so it doesn't need parameter, can be used as SLOT
    // TODO: Add timeout
    void sendRM(quint16 port=0);
    void sendSM(quint16 port=0); // send status msg
    void makeRM(QString &id, quint32 seqNo); // make rumor msg
    // add a new id to msgTable
    void adID(const QString &id);
    // add a new msg to msgTable, update seq vector, display it.
    void addMsg(const QString &id, const QString &msg);
    // parse rumor msg
    void parseRM(const QVariantMap &recvMsgMap);

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

    static const QString TEXT_KEY;
    static const QString ID_KEY;
    static const QString SEQNO_KEY;

    static const QString WANT_KEY;
};

#endif // P2PAPP_MAIN_HH
