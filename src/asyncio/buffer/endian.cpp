#include "endian.h"



bool is_littleEndian(){
    unsigned int i = 1;
    return *(char*)&i != 0;
}



uint64_t to_big_endian64(uint64_t a){
    uint64_t b;
    unsigned char *src = (unsigned char *)&a;
    unsigned char *dst = (unsigned char *)&b;
 
    if (is_littleEndian()){
        dst[0] = src[7];
        dst[1] = src[6];
        dst[2] = src[5];
        dst[3] = src[4];
        dst[4] = src[3];
        dst[5] = src[2];
        dst[6] = src[1];
        dst[7] = src[0];
    }else{
        b = *(uint64_t *)&a;
    }
    return b;
}



uint64_t from_big_endian64(uint64_t a){
    return to_big_endian64(a);
}



uint64_t big_to_machine64(uint64_t a){
    if(is_littleEndian()){
        from_big_endian64(a);
    }else{
        return a;
    }
}



uint64_t host_to_network64(uint64_t a){
    return to_big_endian64(a);
}



uint64_t network_to_host64(uint64_t a){
    return big_to_machine64(a);
}

