#include <mi.h>


bool file_exists(const char *fpath)
{
    struct stat sb;
    return (stat(fpath, &sb) == 0) && (S_ISREG(sb.st_mode));
}

bool dir_exists(const char *folder)
{
	struct stat sb;
	return (stat(folder, &sb) == 0) && (S_ISDIR(sb.st_mode));
}

int min(int a, int b) {
    return (a > b) ? b : a;
}

int max(int a, int b) {
    return (a < b) ? b : a;
}

bool isprintable(char c) 
{
    return isalnum(c) || ispunct(c) || isspace(c);
}
char *string_dup(char *str) {
    char *s = calloc(1, strlen(str) + 1); 
    size_t i = 0;

    while (str[i]) {
        s[i] = str[i];
        i++;
    }

    s[i] = 0;
    return s; 
}

int decode_utf8(uint32_t utf8_bytes, char *str)
{
    printf("> DECODE 0x%x\n", utf8_bytes);
	if(utf8_bytes > 1114112 || ( utf8_bytes >= 0xd800 && utf8_bytes <= 0xdfff ) ){
		//encodes U+fffd; replacement character
		str[0] = (char)0xef;
		str[1] = (char)0xbf;
		str[2] = (char)0xbd;
		str[3] = 0;
		return 3;
	}

	// just like 7-bit ascii
	if(utf8_bytes < 128){
		str[0] = utf8_bytes;
		str[1] = '\0';
		return 1;
	}

	uint32_t len = 0;
	if(utf8_bytes  < 2048){
		len = 2;
	}else{
		if(utf8_bytes  < 65636){
			len = 3;
		} else {
			len = 4;
		}
	}
	str[len] = '\0';

	// set the bits at the start to indicate number of bytes
	unsigned char m = TEC_MOST_SIG_BIT;
	uint32_t i = 0;
	str[0] = 0;
	while(i < len){
		str[0] |= m;
		i += 1;
		m >>= 1;
	}

	//set the most significant bits in the other bytes
	i = 1;
	while(i < len){
		str[i] = (char)TEC_MOST_SIG_BIT;
		i += 1;
	}

	//fill in the codepoint
	int j = len - 1;
	while(j >= 0){
		m = 1;
		i = 0;
		while(utf8_bytes && i < 6){
			if(utf8_bytes % 2){
				str[j] |= m;
			}
			utf8_bytes  >>= 1;
			m <<= 1;
			i += 1;
		}
		j -= 1;
	}

	return (3);
}

void string_clean(char *s) 
{
    size_t i = strlen(s);

    if (s == NULL) return;
    if (!*s) return;

    printf("START => |%s|\n", s);

    // remove spaces from the end. 
    while (i > 1 && isspace(s[i - 1])) {
        s[i - 1] = 0x00;
        printf("|%s|\n", s);
        i--;
    }

    // remove spaces from the start.
    while (isspace(*s))
    {
        printf("|%s|\n", s);
        s++;
    }
    
    printf("FINAL => |%s|\n", s);
}

char *resolve_path(char *src, char *dest)
{
    char *delim = "/";

    if (!src || !dest) {
        return NULL;
    }

    char *buffer = calloc(1, strlen(src) + strlen(dest) + 2);
    if (!strcmp(dest, "..")) {
        // TODO: remove one entry back. /include/x/y => /include/x    
        size_t last = strlen(src) - 1;

        do {
            last--;
        } while (last > 0 && src[last] != *delim);
        
        if (src[last] == *delim) last--;
        buffer = strncpy(buffer, src, last + 1);
        return buffer;
    }    

    if (!strcmp(dest, ".")) {
        // TODO: do nothing
        buffer = strcpy(buffer, src);
        return buffer;
    }

    buffer = strcpy(buffer, src);
    buffer = strcat(buffer, delim);
    buffer = strcat(buffer, dest);
    return (buffer);
}

int make_dir(char *path)
{
    int status = 0;

    if (!path) return status;
    if (strcmp(path, ".") == 0 || strcmp(path, "..") == 0) return status;
    if (dir_exists(path)) return status;

    status = mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    return status;
}

Result *make_prompt_buffer(int x, int y, size_t w)
{
    Result *result = malloc(sizeof(Result));
    bool deleted = false;
    result->data  = malloc(LINE_SZ);
    memset(result->data, 0, LINE_SZ);

    result->type  = SUCCESS;
    result->etype = NONE;

    int buffer_idx = 0;
    int size = 0, byte = 0;
    
    mvchgat (y, 0, w, A_NORMAL, BLUE_PAIR, NULL);
    move(y, x + buffer_idx);
    if (result == NULL || result->data == NULL)
        return NULL;
    
    while ((byte = getch())) { 
        switch(byte) {
            case ESC: {
                result->type = ERROR;
                result->etype = EXIT_SIG;
                return result;
            } break;
            case NL: {
                if (size == 0) {
                    result->type = ERROR;
                    result->etype = EMPTY_BUFF;
                    result->data = strcpy(result->data, "File path/name can not be empty!");
                    return result;
                }
                
                result->type = OK;
                return result;
            } break;
            case KEY_BACKSPACE: {
                if (size > 0) {
                    memmove(
                        result->data + buffer_idx - 1,
                        result->data + buffer_idx,
                        size - buffer_idx
                    );
                    
                    result->data[size] = 0;
                    size--;
                    buffer_idx--;
                    deleted = true;
                }
                result->data[size] = 0;
            } break;
            case KEY_RIGHT: {
                if (buffer_idx < size) buffer_idx++; 
            } break;
            case KEY_LEFT: {
                if (buffer_idx > 0) buffer_idx--; 
            } break;
			case KEY_HOME: {
                buffer_idx = 0;
            } break;
			case KEY_END: {
                buffer_idx = size;
			} break;
            default: {
                if (isprintable(byte)) {
                    memmove(result->data + buffer_idx + 1,
                        result->data + buffer_idx,
                        size - buffer_idx
                    );

                    result->data[buffer_idx++] = byte;
                    size++;
                }
            } break;
        }

        for (int i = 0; i < size; ++i)
            mvaddch (y, x + i, result->data[i]);

        
        // 
        if (deleted) {
            delch();
            deleted = false;
        }

        mvchgat (y, 0, w, A_NORMAL, BLUE_PAIR, NULL);
        move(y, x + buffer_idx);
    }

    if (result->type == OK) {
        // NOTE: This call cleans up the buffer from any trilling or leading white spaces,
        string_clean(result->data);
    }

    return result;
}

