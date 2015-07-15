#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include "user_list.h"

int CNT = 1024;
int srv_fd = -1;
int epoll_fd = -1;
int wsk = 0;
int PORT = 5089;
user** USERS;



/*typedef struct user {
	int fd;
	char* nick;
} user;

user** USERS;
*/

size_t receive_msg(char** msg, int fd);
void send_msg(int fd, const char* msg_w);
int accept_new_client();
void serve_client(int fd, uint32_t events, user_list* list);
void handle_login(const char* msg, size_t len, int fd, user_list* list);
//void get_users(user_list* list);

int main(int argc, const char *argv[])
{
	USERS = malloc(CNT * sizeof(user*));
	memset(USERS, 0, CNT * (sizeof(user*)));
	user_list* list = create_ul(CNT);

    int i = 0;

    struct sockaddr_in srv_addr;
    struct epoll_event e, es[CNT]; //e dany event  es[] tablica eventow do obslugi epoll-wielu klientow

    memset(&srv_addr, 0, sizeof(srv_addr)); // memset zeruje nam te strukturki
    memset(&e, 0, sizeof(e));

    srv_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (srv_fd < 0) {
        printf("Cannot create socket\n");
        return 1;
    }

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    srv_addr.sin_port = htons(PORT);
    if (bind(srv_fd, (struct sockaddr*) &srv_addr, sizeof(srv_addr)) < 0) {
        printf("Cannot bind socket\n");
        close(srv_fd);
        return 1;
    }

    if (listen(srv_fd, 1) < 0) {
        printf("Cannot listen\n");
        close(srv_fd);
        return 1;
    }

    epoll_fd = epoll_create(CNT);
    if (epoll_fd < 0) {
        printf("Cannot create epoll\n");
        close(srv_fd);
        return 1;
    }

    e.events = EPOLLIN;
    e.data.fd = srv_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, srv_fd, &e) < 0) {
        printf("Cannot add server socket to epoll\n");
        close(epoll_fd);
        close(srv_fd);
        return 1;
    }

    for(;;) {
        i = epoll_wait(epoll_fd, es, CNT, -1);
        if (i < 0) {
            printf("Canno5t wait for events\n");
            close(epoll_fd);
            close(srv_fd);
            return 1;
        }

        for (--i; i > -1; --i) {
            if (es[i].data.fd == srv_fd) { //pobiera nam deskryptor i jesli to bedzie deskryptor serwera, to wchodzi w if'a
                if (accept_new_client() != 0)
                	return 1; //shut down server
            } else {
            	serve_client(es[i].data.fd, es[i].events, list);
            }
        }
    }

	return 0;
}

int accept_new_client()
{
	int cli_fd = -1;
	struct sockaddr_in cli_addr;
	socklen_t cli_addr_len = sizeof(cli_addr);
	struct epoll_event e;
	memset(&cli_addr, 0, sizeof(cli_addr));

	cli_fd = accept(srv_fd, (struct sockaddr*) &cli_addr, &cli_addr_len); // i akceptuje

	if (cli_fd < 0) { //ustawia tam -1 dla klienta dla porzadku
		printf("Cannot accept client\n");
		close(epoll_fd);
		close(srv_fd);
		return 1;
	}

	e.events = EPOLLIN;
	e.data.fd = cli_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, cli_fd, &e) < 0) {
		printf("Cannot accept client\n");
		close(epoll_fd);
		close(srv_fd);
		return 1;
	}

	return 0;
}

void serve_client(int fd, uint32_t events, user_list* list)
{
	char *msg = 0;
	size_t len = 0;
	//send_msg(fd);
	if (events & EPOLLIN) {
		len = receive_msg(&msg, fd);
		if(len > 0) { //declaration
			if(msg[0] =='2') {
				printf("%s\n",msg);
				handle_login(msg, len, fd, list);
				//handle_login(msg, len, es[i].data.fd);
			}
			else if(msg[0] == '6') {
				int i = 0;
				size_t *tab_len;
				char** tab = list->print_users(list, &len);
				//for(;i < )

				//handle_userlist(msg, len, es[i].data.fd);
			}
		}
		free(msg);
	} else {
		close(fd);
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0);
		//TODO remove from user list
	}
}

size_t receive_msg(char** msg, int fd)
{
	size_t len = 0;
	if (read(fd,&len, sizeof(size_t)) != sizeof(size_t)) {
			return 0;
	}

	(*msg) = malloc((len+1) * sizeof(char));
	(*msg)[len] = 0;


	if (read(fd, (*msg), len) != len) {
		free(*msg);
		(*msg) = 0;
		return 0;
	}

	return len;
}

void send_msg(int fd, const char* msg_w){
	size_t len = strlen(msg_w);
	write(fd, &len, sizeof(size_t));
	write(fd, msg_w , len);
}

void handle_login(const char* msg, size_t len, int fd, user_list* list)
{
	//TODO:
	//		* create user
	//		* add user to user list
	//		* send ack ("1.0") if user added, send nack otherwise ("1.1.errromessage")
	//		* if nack sent then clear user (close its fd, remove from epoll and remove from user list if needed)

    char* temp = malloc((len-2)* sizeof(char));
    strncpy(temp, msg + 2, len - 2);
    printf("%s\n", temp);
    user* ptr = malloc(sizeof(user));
    ptr -> fd = fd;
    ptr -> nick = temp;
    int comm = list -> add(list, ptr);
    //printf("%d", comm);

    if(comm == 1){
    	char* back_communicate = malloc(30 * sizeof(char));
    	back_communicate = "1.1.user exists";
    	send_msg(fd, back_communicate);
    	//free(back_communicate);
    	//back_communicate = NULL;
    }
    if(comm == 0){
    	char* back_communicate = malloc(30 * sizeof(char));
    	back_communicate = "1.0";
    	send_msg(fd, back_communicate);
    	//printf("Nowa lista:\n");
    	//list->print_users(list);
    	//free(back_communicate);
    	//ack_communicate = NULL;
    }
}

//void get_users(user_list* list);

