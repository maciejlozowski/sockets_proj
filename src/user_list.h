/*
 * user_list.h
 *
 *  Created on: Jul 15, 2015
 *      Author: student
 */

#ifndef SRC_USER_LIST_H_
#define SRC_USER_LIST_H_

#include <stdlib.h>

typedef struct user{
	int fd;
	char* nick;
}user;

typedef struct ul_ctx ul_ctx;

typedef struct user_list{
	ul_ctx* ctx;
	int(*add)(struct user_list* self,user* u);
	int(*rm_by_fd)(struct user_list* self,int fd);
	user*(*find_by_fd)(struct user_list* self, int fd);
	char**(*print_users)(struct user_list* self);
}user_list;

user_list* create_ul(size_t size);
void destroy_ul(user_list* ul);


#endif /* SRC_USER_LIST_H_ */
