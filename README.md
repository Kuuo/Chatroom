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

- `-a`: show all current online client ids and nicknames.
- `-q`: quit chat room.
- `-nn <new_nick_name>`: change nickname to `<new_nick_name>`.
- `-p <user_id> <msg>`: send private message `<msg>` to client `<user_id>`
