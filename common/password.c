#include <common.h>
#include <sha256.h>
#include <u-boot/md5.h>
#include <password.h>

#include <command.h>

static password_state_t lock_state = UNLOCKED;
static char erase_seq[] = "\b \b";

password_state_t
check_lock_status()
{
	return lock_state;
}

/********************************************
 *	remove_protection()
 *
 *	Remove password protection 
 */
int
remove_protection()
{

	setenv(PASSWORD_ENV_VAR, NULL);
	return (saveenv()? 1 : 0);

}

/********************************************
 *	set_protection()
 *
 *	Set / change password protection 
 */
int
set_protection(char *password, size_t password_length)
{

	char hash[256];
	memset(hash,0,256);
	process_password(password, strlen(password), hash);

	setenv(PASSWORD_ENV_VAR, hash);
	return (saveenv()? 1 : 0);

}

/********************************************
 *	setup_lock_status()
 *
 *	Control if a password is set as env
 *	Set the lock accordingly
 *	Return the lock status
 */
password_state_t
setup_lock_status()
{

	char *s = NULL;

	s = getenv(PASSWORD_ENV_VAR);

	lock_state = s ? LOCKED : UNLOCKED;

	return lock_state;

}

/********************************************
 *	unlock()
 *
 *	If the password supplied match, unlock the lock :) 
 */
password_state_t
unlock(char *password, size_t length)
{

	lock_state = verify_password(password, length) ? LOCKED : UNLOCKED;
	printf("password verified %s\n",lock_state?"locked":"unlocked");
	return lock_state;
}
#ifdef CONFIG_SHA256
/********************************************
 *	sha256()
 *
 *	Apply SHA256 to input  
 */
void
sha256(uint8_t * input, uint32_t length, uint8_t digest[32])
{

	sha256_context ctx;

	sha256_starts(&ctx);
	sha256_update(&ctx, input, length);
	sha256_finish(&ctx, digest);

	memset(&ctx, 0, sizeof (sha256_context));

}
#endif
/********************************************
 *	charify_digest()
 *
 *	Represent raw hex as ASCII  
 */
void
charify_digest(uint8_t digest[32], char *charified)
{

	int i;
	char *c = charified;

	for (i = 0; i < 32; i++) {
		sprintf(c, "%02x", digest[i]);
		c += 2;
	}
}

/********************************************
 *	verify_password()
 *
 *	Receive a password as input
 *	Process it
 *	Determine if the input and the stored password match
 */
int
verify_password(char *password, size_t length)
{

	char *original = NULL;
	char hash[256];
    memset(hash,0,256);
	original = getenv(PASSWORD_ENV_VAR);
	process_password(password, strlen(password), hash);

	return strncmp(hash, original,256);

}

/********************************************
 *	process_password()
 *
 *	Receive a password as input
 *	Hash it
 *	Represent the 32 bytes digest using a 256 bytes ASCII string
 */
void
process_password(char *password, size_t password_length, char *hash)
{

	uint8_t digest[32];
	char charified[256];
	uint8_t *input = (uint8_t *) password;
	uint32_t input_length = (uint32_t) password_length;

    memset(digest,0,32);
    memset(charified,0,256);
#ifdef CONFIG_SHA256
    sha256(input, input_length, digest);
#else CONFIG_MD5    
    md5(input,input_length,digest);
#endif    

	charify_digest(digest, charified);

	memcpy(hash, charified, 256);

}

static char *
del_char(char *buffer, char *p, int *colp, int *np, int plen)
{
	if (*np == 0) {
		return (p);
	}

	--p;
	puts(erase_seq);
	(*colp)--;

	(*np)--;
	return (p);
}

int
read_password(const char *const prompt, char *buffer, size_t buffer_length)
{

	char *p = buffer;

	char *p_buf = p;
	int n = 0;		/* buffer index         */
	int col;		/* output column cnt    */
	int plen = 0;		/* prompt length        */
	char c;

	if (prompt) {
		plen = strlen(prompt);
		puts(prompt);
	}
	col = plen;

	for (;;) {

		c = getc();

		/*
		 * Special character handling
		 */
		switch (c) {
		case '\r':	/* Enter                */
		case '\n':
			*p = '\0';
			puts("\r\n");
			return (p - p_buf);

		case '\0':	/* nul                  */
		case '\t':
			continue;

		case 0x03:	/* ^C - break           */
			p_buf[0] = '\0';	/* discard input */
			return (-1);

		case 0x08:	/* ^H  - backspace      */
		case 0x7F:	/* DEL - backspace      */
			p = del_char(p_buf, p, &col, &n, plen);
			continue;

		default:

			if (n < buffer_length - 2) {
				++col;	/* echo input           */
				putc('*');

				*p++ = c;
				++n;
			} else {	/* Buffer full          */
				putc('\a');
			}
		}
	}

}

/********************************************
 *	trap_loop ()
 *
 *	Lock console execution
 */
void
trap_loop()
{

	char buffer[BUFFER_MAX_LEN] = "\0";
	unsigned long long start,end,usleft;
	unsigned long long tmo = CONFIG_SYS_HZ;
	int next_delay;
    int pass_delay;
    next_delay = 1;
	if (setup_lock_status() == LOCKED) {

		while (1) {

			memset(buffer, 0, BUFFER_MAX_LEN);
			read_password("Enter password:", buffer,
				      BUFFER_MAX_LEN);

			if (!unlock(buffer, strlen(buffer)))
				return;
			pass_delay = next_delay;
            printf("password invalid,wait %d seconds...\n",pass_delay);
			while(pass_delay>0){
    			pass_delay--;
                start = get_ticks();
                do {
                    end = get_ticks();
                    usleft = (start < end) ? (end - start) : (end + ~start);
                } while (usleft < tmo);
                
            }
			next_delay *= 2;

		}
	}
}

