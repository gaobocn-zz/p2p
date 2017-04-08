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

    quint16 getPortMin() { return myPortMin; }
    quint16 getPortMax() { return myPortMax; }
    quint16 getPortNum() { return myPortNum; }
    quint16 pickNeighbor();

private:
    quint16 myPortMin, myPortMax, myPortNum;
};

#endif // P2PAPP_NETSOCKET_HH
