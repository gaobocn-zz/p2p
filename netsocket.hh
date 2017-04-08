#ifndef P2PAPP_NETSOCKET_HH
#define P2PAPP_NETSOCKET_HH

#include <QUdpSocket>

class NetSocket : public QUdpSocket
{
    Q_OBJECT

public:
    NetSocket();

    // Bind this socket to a P2Papp-specific default port.
    bool bind();

    int getPortMin() { return myPortMin; }
    int getPortMax() { return myPortMax; }
    int getPortNum() { return myPortNum; }
    int pickNeighbor();

private:
    int myPortMin, myPortMax, myPortNum;
};

#endif // P2PAPP_NETSOCKET_HH
