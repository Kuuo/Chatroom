# Chatroom
A simple LAN chatroom implemented with pthread & socket.

**Development**

- OS: Ubuntu 16.04
- Language: C++ 11

# Usage

## Compile

Just `make`

## Run

**Server:**

```
./server <PORT_YOU_WANT>
```

**Client:**

```
./client <SERVER_IP> <SERVER_PORT>
```

## Special Commands

**Client:**

- `GET_CLIENT_LIST`: show all current online client ids.
