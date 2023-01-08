#include "socket.h"
#include "context.h"

//registers socket to context
//assumes socket memory location is constant
void Socket::register_socket_to_context(Context& context){
    context.add_socket(fd, [=](Flag flag, Context& context, size_t socket_i){
      

        if (flag.isRead())
        {
            this->reads_available();
        }
        if (flag.isWrite())
        {
            this->write_available();
        }

        Flag f;
        f.setReadIf(this->wants_to_read());
        f.setWriteIf(this->wants_to_write());
        context.set_subscription(socket_i, f);

        if(flag.isError()){
            printf("Error\n");
        }
    });
}


void ListeningSocket::register_socket_to_context(Context& context){
    context.add_socket(mFd, [=](Flag flag, Context& context, size_t socket_i){
        if(flag.isRead()){
            this->accept_connection();
        }
    });
}
