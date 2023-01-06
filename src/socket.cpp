#include "socket.h"
#include "context.h"

//registers socket to context
//assumes socket memory location is constant
void Socket::register_socket_to_context(Context& context){
    context.add(fd, [=](auto flag){
        if (flag.isRead())
        {
            this->reads_available();
        }
        if (flag.isWrite())
        {
            this->write_available();
        }
        if(flag.isError()){
            printf("Error\n");
        }
    });
}


void ListeningSocket::register_socket_to_context(Context& context){
    context.add(mFd, [=](Flag flag){
        if(flag.isRead()){
            this->accept_connection();
        }
    });
}
