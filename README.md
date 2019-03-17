# Chatroom
A simple LAN chatroom implemented with pthread & socket.


# Usage

## Compile

```
g++ src/server.cpp -o server -lpthread -std=c++11
g++ src/client.cpp -o client -lpthread -std=c++11
```

or simply

```
bash compile.sh
```

## Run

**Server:**

```
./server <PORT_YOU_WANT>
```

**Client:**

```
./client <SERVER_IP> <SERVER_PORT>
```
