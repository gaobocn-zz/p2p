
#include <unistd.h>

#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>
#include <QHostInfo>

#include "main.hh"

const QString ChatDialog::TEXT_KEY = QString("ChatText");
const QString ChatDialog::ID_KEY = QString("Origin");
const QString ChatDialog::SEQNO_KEY = QString("SeqNo");
const QString ChatDialog::WANT_KEY = QString("Want");

const int ChatDialog::RM_TIMEOUT = 1000;
const int ChatDialog::ANTI_ENTROPY_INTERVAL = 10000;

ChatDialog::ChatDialog()
{
    // Create a UDP network socket
    mySocket = new NetSocket();
    if (!mySocket->bind()) {
        exit(-1);
    }
    connect(mySocket, SIGNAL(readyRead()), this, SLOT(recvData()));

    qsrand(time(NULL));
    this->myID = QHostInfo::localHostName() + "," + QString::number(mySocket->getPortNum()) + "," + QString::number(qrand());
    this->mySeqNo = 1;
    adID(myID);

    // init timer
    resendTimer = new QTimer(this);
    connect(resendTimer, SIGNAL(timeout()), this, SLOT(sendRM()));
    antiEntropyTimer = new QTimer(this);
    connect(antiEntropyTimer, SIGNAL(timeout()), this, SLOT(sendSM()));
    antiEntropyTimer->start(ANTI_ENTROPY_INTERVAL);

    // init UI
    setWindowTitle("P2Papp");
    setFixedSize(400, 400); // for testing
    textview = new QTextEdit(this);
    textview->setReadOnly(true);
    textline = new QLineEdit(this);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(textview);
    layout->addWidget(textline);
    textline->setFocus();
    setLayout(layout);

    textview->append("myID: " + this->myID);

    // Register a callback on the textline's returnPressed signal
    // so that we can send the message entered by the user.
    connect(textline, SIGNAL(returnPressed()),
        this, SLOT(gotReturnPressed()));
}

void ChatDialog::adID(const QString &id) {
    if (msgTable.find(id) != msgTable.end()) {
        qDebug() << "adID";
        exit(1);
    }
    this->msgTable.insert(id, QVector<QString>());
    this->msgNoTable.insert(id, (quint32)1);
}

void ChatDialog::addMsg(const QString &id, const QString &msg) {
    if (msgTable.find(id) == msgTable.end()) {
        qDebug() << "addMsg";
        exit(1);
    }
    msgTable[id].push_back(msg);
    msgNoTable.insert(id, msgNoTable[id].toUInt() + 1);
    // Display
    textview->append(id + ": " + msg);
}

void ChatDialog::gotReturnPressed()
{
    // Serialize
    QVariantMap sendMsgMap;
    sendMsgMap.insert(TEXT_KEY, textline->text());
    QByteArray msgByteArray;
    QDataStream msgStream(&msgByteArray, QIODevice::WriteOnly);
    msgStream << sendMsgMap;

    addMsg(myID, textline->text());

    makeRM(myID, msgTable[myID].size());

    // Send to network
    quint16 portNum = mySocket->pickNeighbor();
    sendRM(portNum);

    // Clear the textline to get ready for the next input message.
    textline->clear();
}

void ChatDialog::makeRM(QString &id, quint32 seqNo) {
    if (msgTable.find(id) == msgTable.end() || (quint32)msgTable[id].size() <= seqNo-1) {
        qDebug() << "makeRM: " << id << seqNo;
        exit(1);
    }
    rmMsg.clear();
    rmMsg.insert(TEXT_KEY, msgTable[id][seqNo-1]);
    rmMsg.insert(ID_KEY, id);
    rmMsg.insert(SEQNO_KEY, seqNo);
}

void ChatDialog::sendMsg(const QVariantMap &sendMsgMap, const quint16 &port) {
    // Serialize
    QByteArray msgByteArray;
    QDataStream msgStream(&msgByteArray, QIODevice::WriteOnly);
    msgStream << sendMsgMap;
    // Send
    qint64 tsize = mySocket->writeDatagram(msgByteArray, QHostAddress::LocalHost, port);
    if (tsize == -1) {
        exit(1);
    }
    // qDebug() << "Sent to port: " << (quint16)(port) << ", msg length:" << tsize;
}

void ChatDialog::sendRM(quint16 port) {
    if (port > 0) {
        this->rmDestPort = port;
    } else {
        // no port provided means its resend
        this->rmDestPort = mySocket->pickNeighbor();
    }
    qDebug() << "Send rumor msg to port:" << rmDestPort;
    sendMsg(rmMsg, rmDestPort);
    resendTimer->start(RM_TIMEOUT);
}

void ChatDialog::sendSM(quint16 port) {
    if (port > 0) {
        this->smDestPort = port;
    } else {
        // no port provided means its anti-entropy
        this->smDestPort = mySocket->pickNeighbor();
    }
    QVariantMap sendMsgMap;
    sendMsgMap.insert(WANT_KEY, msgNoTable);
    qDebug() << "Send status msg to port:" << smDestPort;
    sendMsg(sendMsgMap, smDestPort);
}

void ChatDialog::parseRM(const QVariantMap &recvMsgMap) {
    QString textRecv = recvMsgMap[TEXT_KEY].toString();
    QString idRecv = recvMsgMap[ID_KEY].toString();
    quint32 seqNoRecv = recvMsgMap[SEQNO_KEY].toUInt();
    if (msgNoTable.find(idRecv) == msgNoTable.end()) {
        adID(idRecv);
    }
    // will update seq vector if is expected msg
    if (seqNoRecv == msgNoTable[idRecv].toUInt()) {
        addMsg(idRecv, textRecv);
        makeRM(idRecv, msgTable[idRecv].size()); // prepare to forward this msg to others
    }
}

void ChatDialog::recvData() {
    resendTimer->stop();

    QByteArray msgByteArray;
    msgByteArray.resize(mySocket->pendingDatagramSize());
    QHostAddress senderAddr;
    quint16 senderPort;
    qint64 tsize = mySocket->readDatagram(msgByteArray.data(), mySocket->pendingDatagramSize(), &senderAddr, &senderPort);
    if (tsize == -1) {
        exit(1);
    }

    // De-serialize
    QVariantMap recvMsgMap; 
    QDataStream msgStream(&msgByteArray, QIODevice::ReadOnly);
    msgStream >> recvMsgMap;

    if (recvMsgMap.contains(WANT_KEY)) {
        // recved status msg
        QVariantMap recvMsgNoTable = recvMsgMap[WANT_KEY].toMap();
        qDebug() << "recved status msg" << recvMsgNoTable;
        QVariantMap::iterator it;
        for (it = recvMsgNoTable.begin(); it != recvMsgNoTable.end(); ++it) {
            QString tID = it.key();
            quint32 tSeqNo = it.value().toUInt();
            if (msgNoTable.find(tID) == msgNoTable.end()) {
                adID(tID);
            }
            if (tSeqNo > msgNoTable[tID].toUInt()) {
                // I need new msg
                sendSM(senderPort);
                return;
            } else if (tSeqNo < msgNoTable[tID].toUInt()) {
                // you need new msg
                makeRM(tID, tSeqNo);
                sendRM(senderPort);
                return;
            }
        }
        // all ids in recvMsgNoTable have the same seqNo in msgNoTable
        for (it = msgNoTable.begin(); it != msgNoTable.end(); ++it) {
            QString tID = it.key();
            quint32 tSeqNo = it.value().toUInt();
            if (recvMsgNoTable.find(tID) == recvMsgNoTable.end() && tSeqNo > 1) {
                // I have something you don't have
                makeRM(tID, 1);
                sendRM(senderPort);
                return;
            }
        }
        // msgNoTable and recvMsgNoTable are identical
        // nothing to be sent
        if (rmMsg.empty()) return;
        // flip coin
        quint16 portNum = mySocket->pickNeighbor();
        if (qrand() % 2) {
            sendRM(portNum);
        }
    } else if (recvMsgMap.contains(TEXT_KEY)) {
        // recved rumor msg
        qDebug() << "recved rumor msg";
        parseRM(recvMsgMap);
        sendSM(senderPort);
    } else {
        qDebug() << "recvData: Map format wrong";
        exit(1);
    }
}

/*
int haha(int argc, char **argv)
{
    // Initialize Qt toolkit
    QApplication app(argc,argv);

    // Create an initial chat dialog window
    ChatDialog dialog;
    dialog.show();

    // Enter the Qt main loop; everything else is event driven
    return app.exec();
}
*/

