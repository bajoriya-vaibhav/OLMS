#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

//client
int main(){
    struct sockaddr_in serv;
    int sd = socket(AF_INET,SOCK_STREAM,0);
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv.sin_port = htons(8080);
    connect(sd,(struct sockaddr *)&serv,sizeof(serv));
    while(1){
        char buf[1024] = {0};
        read(sd, buf, sizeof(buf));
        printf("%s\n", buf);
        if(buf[0] == 'E' && buf[1] == 'x' && buf[2] == 'i' && buf[3] == 't'){
            break;
        }
        memset(buf, 0, sizeof(buf));
        fgets(buf, sizeof(buf), stdin);
        write(sd, buf, sizeof(buf));
    }
    close(sd);
}