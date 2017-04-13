# p2p

An implementation of p2p messaging system using gossip protocol.

## Build and Run
./build.sh  
./p2p

## Implementation
No central server is needed, every peer receives the message and randomly forwards it to a neighbor, and there are anti-entropy mechanism to ensure every message can propagate to the whole system.


![FSM for the code](p2p_gossip.png)
