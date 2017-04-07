
#include <unistd.h>

#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>

#include "main.hh"

ChatDialog::ChatDialog()
{
    // Create a UDP network socket
    mySocket = new NetSocket();
    if (!mySocket->bind()) {
        exit(-1);
    }
    connect(mySocket, SIGNAL(readyRead()), this, SLOT(recvData()));

    setWindowTitle("P2Papp");

    // Read-only text box where we display messages from everyone.
    // This widget expands both horizontally and vertically.
    textview = new QTextEdit(this);
    textview->setReadOnly(true);

    // Small text-entry box the user can enter messages.
    // This widget normally expands only horizontally,
    // leaving extra vertical space for the textview widget.
    //
    // You might change this into a read/write QTextEdit,
    // so that the user can easily enter multi-line messages.
    textline = new QLineEdit(this);

    // Lay out the widgets to appear in the main window.
    // For Qt widget and layout concepts see:
    // http://doc.qt.nokia.com/4.7-snapshot/widgets-and-layouts.html
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(textview);
    layout->addWidget(textline);
    textline->setFocus();
    setLayout(layout);

    // Register a callback on the textline's returnPressed signal
    // so that we can send the message entered by the user.
    connect(textline, SIGNAL(returnPressed()),
        this, SLOT(gotReturnPressed()));
}

void ChatDialog::gotReturnPressed()
{
    // Serialize
    QVariantMap msgMap;
    msgMap.insert(tr("ChatText"), textline->text());
    QByteArray msgByteArray;
    QDataStream msgStream(&msgByteArray, QIODevice::WriteOnly);
    msgStream << msgMap;

    // Send to network
    for (int portI = mySocket->getPortMin(); portI <= mySocket->getPortMax(); ++portI) {
        qint64 tsize = mySocket->writeDatagram(msgByteArray, QHostAddress::LocalHost, portI);
        if (tsize == -1) {
            exit(1);
        }
        qDebug() << "Sent to port: " << (quint16)(portI) << ", msg length:" << tsize;
    }

    qDebug() << "FIX: send message to other peers: " << textline->text();
    textview->append(textline->text());

    // Clear the textline to get ready for the next input message.
    textline->clear();
}

void ChatDialog::recvData() {
    while (mySocket->hasPendingDatagrams()) {
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
        QVariantMap msgMap; 
        QDataStream msgStream(&msgByteArray, QIODevice::ReadOnly);
        msgStream >> msgMap;
        qDebug() << "recv the map: " << msgMap;

        // Display
        QString stringRecv(msgMap.value("ChatText").toString());
        textview->append(stringRecv);
    }
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

