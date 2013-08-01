//#ifndef _PASSWORD_H
#define _PASSWORD_H
#include <common.h>

//#define _USE_PASSWORD		1
#define PASSWORD_MAX_LEN	30
#define PASSWORD_ENV_VAR	"password"
#define	PASSWORD_HSH_LEN	32
#define BUFFER_MAX_LEN	PASSWORD_MAX_LEN+2

typedef enum {
	UNLOCKED = 0,
	LOCKED = 1
} password_state_t;

void trap_loop(void);

int remove_protection(void);
int verify_password(char *password, size_t length);
int set_protection(char *password, size_t password_length);
int read_password(const char *const prompt, char *buffer, size_t buffer_length);


void charify_digest(uint8_t digest[32], char *charified);
void process_password(char *password, size_t password_length, char *hash);

password_state_t check_lock_status(void);
password_state_t setup_lock_status(void);
password_state_t unlock(char *password, size_t length);
//#endif				/* _PASSWORD_H */

