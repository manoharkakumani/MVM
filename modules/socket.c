// // socket module

// #ifndef _WIN32
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <netdb.h>
// #else
// #include <winsock2.h>
// #include <ws2tcpip.h>
// #endif

// #include <string.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <stdio.h>
// #include <stdlib.h>

// #include "../datatypes/datatypes.h"
// #include "../vm.h"

// #define O_NONBLOCK  0x0004
// #define AF_INET     2
// #define SOCK_STREAM 1
// #define IPPROTO_TCP 6

// MODULE(socket);

// MyMoObject *socketfn(MVM *vm, uint argc, MyMoObject *argv[]){
//     if (argc != 3) {
//         runtimeError(vm, "TypeError: socket.socket() takes 3 arguments (%d given).", argc);
//         return NEW_EMPTY;
//     }
//     if (!IS_NUMBER(argv[0])) {
//         runtimeError(vm, "TypeError: must be <object 'int'>, not (%s)",getType(argv[0]));
//         return NEW_EMPTY;
//     }
//     if (!IS_NUMBER(argv[1])) {
//         runtimeError(vm, "TypeError: must be <object 'int'>, not (%s)",getType(argv[1]));
//         return NEW_EMPTY;
//     }
//     if (!IS_NUMBER(argv[2])) {
//         runtimeError(vm, "TypeError: must be <object 'int'>, not (%s)",getType(argv[2]));
//         return NEW_EMPTY;
//     }
//     int domain = NUMBER_VAL(argv[0]);
//     int type = NUMBER_VAL(argv[1]);
//     int protocol = NUMBER_VAL(argv[2]);
//     int sockfd = socket(domain, type, protocol);
//     if (sockfd == -1) {
//         runtimeError(vm, "Socket creation failed");
//         return NEW_EMPTY;
//     }
//     return NEW_INT(vm, sockfd);
// }

// MyMoObject *bindfn(MVM *vm, uint argc, MyMoObject *argv[]){
//     if (argc != 3) {
//         runtimeError(vm, "TypeError: socket.bind() takes 3 arguments (%d given).", argc);
//         return NEW_EMPTY;
//     }
//     if (!IS_NUMBER(argv[0])) {
//         runtimeError(vm, "TypeError: must be <object 'int'>, not (%s)",getType(argv[0]));
//         return NEW_EMPTY;
//     }
//     if (!IS_STRING(argv[1])) {
//         runtimeError(vm, "TypeError: must be <object 'str'>, not (%s)",getType(argv[1]));
//         return NEW_EMPTY;
//     }
//     if (!IS_NUMBER(argv[2])) {
//         runtimeError(vm, "TypeError: must be <object 'int'>, not (%s)",getType(argv[2]));
//         return NEW_EMPTY;
//     }
//     int sockfd = NUMBER_VAL(argv[0]);
//     char *ip = AS_CSTRING(argv[1]);
//     int port = NUMBER_VAL(argv[2]);
//     struct sockaddr_in address;
//     address.sin_family = AF_INET;
//     address.sin_addr.s_addr = inet_addr(ip);
//     address.sin_port = htons(port);
//     int addrlen = sizeof(address);
//     int bind_status = bind(sockfd, (struct sockaddr *)&address, addrlen);
//     if (bind_status == -1) {
//         runtimeError(vm, "Bind failed");
//         return NEW_EMPTY;
//     }
//     return NEW_INT(vm, bind_status);
// }

// MyMoObject *listenfn(MVM *vm, uint argc, MyMoObject *argv[]){
//     if (argc != 2) {
//         runtimeError(vm, "TypeError: socket.listen() takes 2 arguments (%d given).", argc);
//         return NEW_EMPTY;
//     }
//     if (!IS_NUMBER(argv[0])) {
//         runtimeError(vm, "TypeError: must be <object 'int'>, not (%s)",getType(argv[0]));
//         return NEW_EMPTY;
//     }
//     if (!IS_NUMBER(argv[1])) {
//         runtimeError(vm, "TypeError: must be <object 'int'>, not (%s)",getType(argv[1]));
//         return NEW_EMPTY;
//     }
//     int sockfd = NUMBER_VAL(argv[0]);
//     int backlog = NUMBER_VAL(argv[1]);
//     int listen_status = listen(sockfd, backlog);
//     if (listen_status == -1) {
//         runtimeError(vm, "Listen failed");
//         return NEW_EMPTY;
//     }
//     return NEW_INT(vm, listen_status);
// }

// MyMoObject *acceptfn(MVM *vm, uint argc, MyMoObject *argv[]){
//     if (argc != 1) {
//         runtimeError(vm, "TypeError: socket.accept() takes 1 argument (%d given).", argc);
//         return NEW_EMPTY;
//     }
//     if (!IS_NUMBER(argv[0])) {
//         runtimeError(vm, "TypeError: must be <object 'int'>, not (%s)",getType(argv[0]));
//         return NEW_EMPTY;
//     }
//     int sockfd = NUMBER_VAL(argv[0]);
//     struct sockaddr_in address;
//     int addrlen = sizeof(address);
//     int new_socket = accept(sockfd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
//     if (new_socket == -1) {
//         runtimeError(vm, "Accept failed");
//         return NEW_EMPTY;
//     }
//     return NEW_INT(vm, new_socket);
// }

// MyMoObject *connectfn(MVM *vm, uint argc, MyMoObject *argv[]){
//     if (argc != 3) {
//         runtimeError(vm, "TypeError: socket.connect() takes 3 arguments (%d given).", argc);
//         return NEW_EMPTY;
//     }
//     if (!IS_NUMBER(argv[0])) {
//         runtimeError(vm, "TypeError: must be <object 'int'>, not (%s)",getType(argv[0]));
//         return NEW_EMPTY;
//     }
//     if (!IS_STRING(argv[1])) {
//         runtimeError(vm, "TypeError: must be <object 'str'>, not (%s)",getType(argv[1]));
//         return NEW_EMPTY;
//     }
//     if (!IS_NUMBER(argv[2])) {
//         runtimeError(vm, "TypeError: must be <object 'int'>, not (%s)",getType(argv[2]));
//         return NEW_EMPTY;
//     }
//     int sockfd = NUMBER_VAL(argv[0]);
//     char *ip = STRING_VAL(argv[1]);
//     int port = NUMBER_VAL(argv[2]);
//     struct sockaddr_in address;
//     address.sin_family = AF_INET;
//     address.sin_addr.s_addr = inet_addr(ip);
//     address.sin_port = htons(port);
//     int addrlen = sizeof(address);
//     int connect_status = connect(sockfd, (struct sockaddr *)&address, addrlen);
//     if (connect_status == -1) {
//         runtimeError(vm, "Connect failed");
//         return NEW_EMPTY;
//     }
//     return NEW_INT(vm, connect_status);
// }

// MyMoObject *sendfn(MVM *vm, uint argc, MyMoObject *argv[]){
//     if (argc != 2) {
//         runtimeError(vm, "TypeError: socket.send() takes 2 arguments (%d given).", argc);
//         return NEW_EMPTY;
//     }
//     if (!IS_NUMBER(argv[0])) {
//         runtimeError(vm, "TypeError: must be <object 'int'>, not (%s)",getType(argv[0]));
//         return NEW_EMPTY;
//     }
//     if (!IS_STRING(argv[1])) {
//         runtimeError(vm, "TypeError: must be <object 'str'>, not (%s)",getType(argv[1]));
//         return NEW_EMPTY;
//     }
//     int sockfd = NUMBER_VAL(argv[0]);
//     char *buffer = STRING_VAL(argv[1]);
//     int send_status = send(sockfd, buffer, strlen(buffer), 0);
//     if (send_status == -1) {
//         runtimeError(vm, "Send failed");
//         return NEW_EMPTY;
//     }
//     return NEW_INT(vm, send_status);
// }

// MyMoObject *recvfn(MVM *vm, uint argc, MyMoObject *argv[]){
//     if (argc != 2) {
//         runtimeError(vm, "TypeError: socket.recv() takes 2 arguments (%d given).", argc);
//         return NEW_EMPTY;
//     }
//     if (!IS_NUMBER(argv[0])) {
//         runtimeError(vm, "TypeError: must be <object 'int'>, not (%s)",getType(argv[0]));
//         return NEW_EMPTY;
//     }
//     if (!IS_NUMBER(argv[1])) {
//         runtimeError(vm, "TypeError: must be <object 'int'>, not (%s)",getType(argv[1]));
//         return NEW_EMPTY;
//     }
//     int sockfd = NUMBER_VAL(argv[0]);
//     int buffer_size = NUMBER_VAL(argv[1]);
//     char buffer[buffer_size];
//     int recv_status = recv(sockfd, buffer, buffer_size, 0);
//     if (recv_status == -1) {
//         runtimeError(vm, "Recv failed");
//         return NEW_EMPTY;
//     }
//     return NEW_STRING(vm, buffer);
// }

// MyMoObject *closefn(MVM *vm, uint argc, MyMoObject *argv[]){
//     if (argc != 1) {
//         runtimeError(vm, "TypeError: socket.close() takes 1 argument (%d given).", argc);
//         return NEW_EMPTY;
//     }
//     if (!IS_NUMBER(argv[0])) {
//         runtimeError(vm, "TypeError: must be <object 'int'>, not (%s)",getType(argv[0]));
//         return NEW_EMPTY;
//     }
//     int sockfd = NUMBER_VAL(argv[0]);
//     int close_status = close(sockfd);
//     if (close_status == -1) {
//         runtimeError(vm, "Close failed");
//         return NEW_EMPTY;
//     }
//     return NEW_INT(vm, close_status);
// }

// MyMoObject *setblockingfn(MVM *vm, uint argc, MyMoObject *argv[]){
//     if (argc != 2) {
//         runtimeError(vm, "TypeError: socket.setblocking() takes 2 arguments (%d given).", argc);
//         return NEW_EMPTY;
//     }
//     if (!IS_NUMBER(argv[0])) {
//         runtimeError(vm, "TypeError: must be <object 'int'>, not (%s)",getType(argv[0]));
//         return NEW_EMPTY;
//     }
//     if (!IS_BOOL(argv[1])) {
//         runtimeError(vm, "TypeError: must be <object 'bool'>, not (%s)",getType(argv[1]));
//         return NEW_EMPTY;
//     }
//     int sockfd = NUMBER_VAL(argv[0]);
//     int blocking = AS_BOOL(argv[1]);
//     int flags = fcntl(sockfd, F_GETFL, 0);
//     if (flags == -1) {
//         runtimeError(vm, "Get flags failed");
//         return NEW_EMPTY;
//     }
//     if (blocking) {
//         flags &= ~O_NONBLOCK;
//     } else {
//         flags |= O_NONBLOCK;
//     }
//     int setblocking_status = fcntl(sockfd, F_SETFL, flags);
//     if (setblocking_status == -1) {
//         runtimeError(vm, "Set blocking failed");
//         return NEW_EMPTY;
//     }
//     return NEW_INT(vm, setblocking_status);
// }

// MyMoObject *getblockingfn(MVM *vm, uint argc, MyMoObject *argv[]){
//     if (argc != 1) {
//         runtimeError(vm, "TypeError: socket.getblocking() takes 1 argument (%d given).", argc);
//         return NEW_EMPTY;
//     }
//     if (!IS_NUMBER(argv[0])) {
//         runtimeError(vm, "TypeError: must be <object 'int'>, not (%s)",getType(argv[0]));
//         return NEW_EMPTY;
//     }
//     int sockfd = NUMBER_VAL(argv[0]);
//     int flags = fcntl(sockfd, F_GETFL, 0);
//     if (flags == -1) {
//         runtimeError(vm, "Get flags failed");
//         return NEW_EMPTY;
//     }
//     if (flags & O_NONBLOCK) {
//         return NEW_BOOL(vm, false);
//     } else {
//         return NEW_BOOL(vm, true);
//     }
// }

// MyMoObject *gethostnamefn(MVM *vm, uint argc, MyMoObject *argv[]){
//     if (argc != 0) {
//         runtimeError(vm, "TypeError: socket.gethostname() takes 0 arguments (%d given).", argc);
//         return NEW_EMPTY;
//     }
//     char hostname[1024];
//     int gethostname_status = gethostname(hostname, 1024);
//     if (gethostname_status == -1) {
//         runtimeError(vm, "Get hostname failed");
//         return NEW_EMPTY;
//     }
//     return NEW_STRING(vm, hostname);
// }

// MyMoObject *gethostbynamefn(MVM *vm, uint argc, MyMoObject *argv[]){
//     if (argc != 1) {
//         runtimeError(vm, "TypeError: socket.gethostbyname() takes 1 argument (%d given).", argc);
//         return NEW_EMPTY;
//     }
//     if (!IS_STRING(argv[0])) {
//         runtimeError(vm, "TypeError: must be <object 'str'>, not (%s)",getType(argv[0]));
//         return NEW_EMPTY;
//     }
//     char *hostname = STRING_VAL(argv[0]);
//     struct hostent *host = gethostbyname(hostname);
//     if (host == NULL) {
//         runtimeError(vm, "Get host by name failed");
//         return NEW_EMPTY;
//     }
//     return NEW_STRING(vm, host->h_name);
// }
