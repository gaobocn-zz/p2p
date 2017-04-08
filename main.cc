
#include <unistd.h>

#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>
#include <QHostInfo>

#include "main.hh"

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

    this->msgTable.insert(myID, QVector<QString>());
    this->msgNoTable.insert(myID, (quint32)1);

    // UI
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

void ChatDialog::gotReturnPressed()
{
    // Serialize
    QVariantMap sendMsgMap;
    sendMsgMap.insert(tr("ChatText"), textline->text());
    QByteArray msgByteArray;
    QDataStream msgStream(&msgByteArray, QIODevice::WriteOnly);
    msgStream << sendMsgMap;

    msgTable[myID].push_back(textline->text());
    msgNoTable.insert(myID, msgNoTable.value(myID).toInt() + 1);

    makeRM(myID, msgTable[myID].size());

    // Send to network
    for (int portI = mySocket->getPortMin(); portI <= mySocket->getPortMax(); ++portI) {
        if (portI == mySocket->getPortNum()) continue;
        this->rmDestPort = portI;
        sendRM();
        this->smDestPort = portI;
        sendSM();
    }

    textview->append(textline->text());

    // Clear the textline to get ready for the next input message.
    textline->clear();
}

void ChatDialog::makeRM(QString &id, quint32 seqNo) {
    if (msgTable.find(id) == msgTable.end() || (quint32)msgTable[id].size() <= seqNo-1) {
        qDebug() << "makeRM: " << id << seqNo;
        exit(1);
    }
    rmMsg.clear();
    rmMsg.insert(tr("ChatText"), msgTable[id][seqNo-1]);
    rmMsg.insert(tr("Origin"), id);
    rmMsg.insert(tr("SeqNo"), seqNo);
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
    qDebug() << "Sent to port: " << (quint16)(port) << ", msg length:" << tsize;
}

void ChatDialog::sendSM() {
    QVariantMap sendMsgMap;
    sendMsgMap.insert(tr("Want"), msgNoTable);
    sendMsg(sendMsgMap, smDestPort);
}

void ChatDialog::recvData() {
    // Recv from network
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

    if (recvMsgMap.contains("Want")) {
        qDebug() << recvMsgMap;
        return;
    }

    // Display
    QString stringRecv(recvMsgMap.value("ChatText").toString());
    textview->append(stringRecv);
}

int main(int argc, char **argv)
{
    // Initialize Qt toolkit
    QApplication app(argc,argv);

    // Create an initial chat dialog window
    ChatDialog dialog;
    dialog.show();

    // Enter the Qt main loop; everything else is event driven
    return app.exec();
}

