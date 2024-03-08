#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/select.h>
#include <math.h>
#include <getopt.h>

const char *texmex_version_string = "0.2.1";

static const struct option long_options[] = {
	{"help", no_argument, NULL, 'h'},
	{"version", no_argument, NULL, 'V'},
	{"bin2text", required_argument, NULL, 'B'},
	{"text2bin", required_argument, NULL, 'b'},
	{"file2hex", required_argument, NULL, 'f'},
	{"file2int", required_argument, NULL, 'F'},
	{"hex2bin", required_argument, NULL, 'H'},
	{"int2bin", required_argument, NULL, 'I'},
	{"hex2int", required_argument, NULL, 'i'},
	{"int2hex", required_argument, NULL, 'j'},
	{"text2line", required_argument, NULL, 'l'},
	{"hex2text", required_argument, NULL, 't'},
	{"signed", no_argument, NULL, 's'},
	{"text2hex_escape", required_argument, NULL, 'X'},
	{"text2hex", required_argument, NULL, 'x'},
	{NULL, 0, NULL, 0}
};
static const char *short_options = "hVB:b:F:f:H:I:i:j:l:st:X:x:";

unsigned int use_signed, use_stdin, do_hex2int;
char *hex2int_arg;

void ShowHelp(void) {
printf("texmex options:\n\t-h, --help\n"
"\t-V, --version\n"
"\t-B, --bin2text 00001111\n"
"\t-b, --text2bin TEXT\n"
"\t-F, --file2hex FILENAME\n"
"\t-f, --file2int FILENAME\n"
"\t-H, --hex2bin FFEE8844\n"
"\t-I, --int2bin 255\n"
"\t-i, --hex2int FFEE8844\n"
"\t-l, --text2line PARAGRAPH\n"
"\t-s, --signed\n"
"\t-t, --hex2text \"AFDE08fa\"\n"
"\t-X, --text2hex_escape TEXT\n"
"\t-x, --text2hex TEXT }\n");
}

char *text2bin(char *text, int len) {
	char *str = (char *)malloc(len*8+1);
	memset(str, 0, len*8+1);
	char c = *text, mask = 0x80;
	unsigned int cnt, cnt2;
	for (cnt = 0; cnt < len; cnt++) {
		for (c = text[cnt], cnt2 = 0; cnt2 <= 7; cnt2++) {
			str[(cnt+1)*8-8+cnt2] = (c & mask)?'1':'0';
			c = (c << 1);
		}
	}

	return str;
}

char *bin2text(char *text, int len) {
	char *str = (char *)malloc(len/8+1);
	char c;
	unsigned int cnt, cnt2, cnt3;
	for (cnt = 0, cnt3 = 0; cnt < len; cnt+=8, cnt3++) {
		for (cnt2 = 0; cnt2 <= 7; cnt2++) {
			c = text[cnt+cnt2];
			if (c == '1') {
				if (cnt2 == 0)
					str[cnt3] = 128;
				else if (cnt2 == 1)
					str[cnt3] += 64;
				else if (cnt2 == 2)
					str[cnt3] += 32;
				else if (cnt2 == 3)
					str[cnt3] += 16;
				else if (cnt2 == 4)
					str[cnt3] += 8;
				else if (cnt2 == 5)
					str[cnt3] += 4;
				else if (cnt2 == 6)
					str[cnt3] += 2;
				else if (cnt2 == 7)
					str[cnt3] += 1;
			}
		}
	}

	return str;
}

void file2hex(char *filename, int len) {
	if (filename[len - 1] == '\n')
		filename[len - 1] = '\0';
    
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "texmex error: Cannot open %s: %s\n", filename, strerror(errno));
		return;
	}

	int c = 0;
	while (1) {
		c = fgetc(fp);
		if (c == -1)
			break;
		printf("%X:", c);
		fflush(stdout);
	}

	fclose(fp);
}

void file2int(char *filename, int len) {
	if (len > 0 && filename[len - 1] == '\n')
			filename[len - 1] = '\0';
    
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "texmex error: Cannot open %s: %s\n", filename, strerror(errno));
		return;
	}

	int c = 0;
	while (1) {
		c = fgetc(fp);
		if (c == -1)
			break;
		printf("%d:", c);
		fflush(stdout);
	}

	fclose(fp);
}

int hex2int(char hex) {
	switch (hex) {
	case '0':
		return 0;
	case '1':
		return 1;
	case '2':
		return 2;
	case '3':
		return 3;
	case '4':
		return 4;
	case '5':
		return 5;
	case '6':
		return 6;
	case '7':
		return 7;
	case '8':
		return 8;
	case '9':
		return 9;
	case 'A':
	case 'a':
		return 10;
	case 'B':
	case 'b':
		return 11;
	case 'C':
	case 'c':
		return 12;
	case 'D':
	case 'd':
		return 13;
	case 'E':
	case 'e':
		return 14;
	case 'F':
	case 'f':
		return 15;
	default:
		return 0;
	}
	return 0;
}

int hex2int32(char *hex, int len) {
	char fullcard[9];
	int x = strlen(hex)-1;
	int val = 0, cnt;
	memset(fullcard, 0, 9);
	for (cnt = 7; cnt >= 0; cnt--, x--) {
		if (x < 0)
			fullcard[cnt] = '0';
		else
			fullcard[cnt] = hex[x];
	}
	for (cnt = 7; cnt >= 0; cnt--) {
		if (cnt == 7)
			val += hex2int(fullcard[cnt]);
		else if (cnt == 6)
			val += hex2int(fullcard[cnt])*16;
		else if (cnt == 5)
			val += hex2int(fullcard[cnt])*16*16;
		else if (cnt == 4)
			val += hex2int(fullcard[cnt])*16*16*16;
		else if (cnt == 3)
			val += hex2int(fullcard[cnt])*16*16*16*16;
		else if (cnt == 2)
			val += hex2int(fullcard[cnt])*16*16*16*16*16;
		else if (cnt == 1)
			val += hex2int(fullcard[cnt])*16*16*16*16*16*16;
		else if (cnt == 0)
			val += hex2int(fullcard[cnt])*16*16*16*16*16*16*16;
	}

	return val;
}

unsigned int hex2uint32(char *hex, int len) {
	char fullcard[9];
	int x = strlen(hex)-1, cnt;
	unsigned int val = 0;
	memset(fullcard, 0, 9);
	for (cnt = 7; cnt >= 0; cnt--, x--) {
		if (x < 0)
			fullcard[cnt] = '0';
		else
			fullcard[cnt] = hex[x];
	}
	for (cnt = 7; cnt >= 0; cnt--) {
		if (cnt == 7)
			val += (unsigned int)hex2int(fullcard[cnt]);
		else if (cnt == 6)
			val += (unsigned int)hex2int(fullcard[cnt])*16;
		else if (cnt == 5)
			val += (unsigned int)hex2int(fullcard[cnt])*16*16;
		else if (cnt == 4)
			val += (unsigned int)hex2int(fullcard[cnt])*16*16*16;
		else if (cnt == 3)
			val += (unsigned int)hex2int(fullcard[cnt])*16*16*16*16;
		else if (cnt == 2)
			val += (unsigned int)hex2int(fullcard[cnt])*16*16*16*16*16;
		else if (cnt == 1)
			val += (unsigned int)hex2int(fullcard[cnt])*16*16*16*16*16*16;
		else if (cnt == 0)
			val += (unsigned int)hex2int(fullcard[cnt])*16*16*16*16*16*16*16;
	}

	return val;
}

char *int2bin(unsigned int val) {
	char *binstr = (char *)malloc(9);
	memset(binstr, 0, 9);
	unsigned int mask = 1, cnt = 7;
	while (1) {
		binstr[cnt] = (val & mask) ? '1':'0';
		if (cnt == 0)
			break;
		else		
			--cnt;
		val >>= 1;
	}
	return binstr;
}

char *hex4bit2bin(char hex) {
	char *binstr = (char *)malloc(5);
	memset(binstr, 0, 5);
	unsigned int val = hex2int(hex);
	unsigned int mask = 1;
	unsigned cnt = 3;
	while (1) {
		binstr[cnt] = (val & mask) ? '1':'0';
		val >>= 1;
		if (cnt == 0)
			break;
		--cnt;
	}
	return binstr;
}

char *hex2bin(char *hexstr, int len) {
	char *c = hexstr;
	char *binstr = (char *)malloc(len*8+1);
	memset(binstr, 0, len*8+1);
	int cnt = 0;
	while (1) {
		char *str = hex4bit2bin(*c++);
		strcat(binstr, str);
		free(str);
		
		++cnt;
		if (cnt >= len)
			break;
	}
	
	return binstr;
}

char hex2char(char hex[2]) {
	return hex2int(hex[0])*16 + hex2int(hex[1]);
}

void int2hex(unsigned int val) {
	printf("%X", val);
}

void hex2text(char *text, int len) {
	char *buffer = (char *)malloc(len+1);
	memset(buffer, 0, len+1);
	char cart[2];
	int cnt, iter;
	for (cnt = 0, iter = 0; cnt < len; cnt += 2, iter++) {
		cart[0] = *(text+cnt);
		cart[1] = *(text+cnt+1);
		*(buffer+iter) = hex2char(cart);
	}
	printf("%s", buffer);
	free(buffer);
}

void text2line(char *text, int len) {
	char *buffer = (char *)malloc(len+1);
	memset(buffer, 0, len+1);

	int cnt;
	for (cnt = 0; cnt < len; cnt++) {
		if (*(text+cnt) != '\n')
			*(buffer+cnt) = *(text+cnt);
		else
			*(buffer+cnt) = ' ';
	}

	printf("%s", buffer);
	free(buffer);
}

#define CHUNK_SIZE 8192
void ProcessStdin(char *option) {
	char chunk[CHUNK_SIZE];
	size_t bytes_read;

	// Based on the option, call the corresponding function
	if (strncmp(option, "-B", 2) == 0) {
		// Loop to read from stdin in chunks
		while ((bytes_read = fread(chunk, 1, CHUNK_SIZE, stdin)) > 0)
			bin2text(chunk, bytes_read);
	}
	else if (strncmp(option, "-b", 2) == 0) {
		while ((bytes_read = fread(chunk, 1, CHUNK_SIZE, stdin)) > 0) {
			char *str = text2bin(chunk, bytes_read);
			printf("%s", str);
			free(str);
		}
	}
	else if (strncmp(option, "-F", 2) == 0) {
		bytes_read = fread(chunk, 1, CHUNK_SIZE, stdin);
		file2hex(chunk, bytes_read);
	}
	else if (strncmp(option, "-f", 2) == 0) {
		bytes_read = fread(chunk, 1, CHUNK_SIZE, stdin);
		file2int(chunk, bytes_read);
	}
	else if (strncmp(option, "-H", 2) == 0) {
		while ((bytes_read = fread(chunk, 1, CHUNK_SIZE, stdin)) > 0) {
			char *str = hex2bin(chunk, bytes_read);
			printf("%s", str);
			free(str);
		}
	}
	else if (strncmp(option, "-I", 2) == 0) {
		fread(chunk, 1, CHUNK_SIZE, stdin);
		char *str = int2bin(atoi(chunk));
		printf("%s", str);
		free(str);
	}
	else if (strncmp(option, "-i", 2) == 0) {
		bytes_read = fread(chunk, 1, CHUNK_SIZE, stdin);
		printf("%d", hex2int32(chunk, bytes_read));
	}
	else if (strncmp(option, "-l", 2) == 0) {
		while ((bytes_read = fread(chunk, 1, CHUNK_SIZE, stdin)) > 0)
			text2line(chunk, bytes_read);
	}
	else if (strncmp(option, "-t", 2) == 0) {
		while ((bytes_read = fread(chunk, 1, CHUNK_SIZE, stdin)) > 0)
			hex2text(chunk, bytes_read);
	}
	else if (strncmp(option, "-X", 2) == 0) {
		while ((bytes_read = fread(chunk, 1, CHUNK_SIZE, stdin)) > 0) {
			for (int cnt = 0; cnt < bytes_read; cnt++)
				printf("\\x%X", chunk[cnt]);
		}
	}
	else if (strncmp(option, "-x", 2) == 0) {
		while ((bytes_read = fread(chunk, 1, CHUNK_SIZE, stdin)) > 0) {
			for (int cnt = 0; cnt < bytes_read; cnt++)
				printf("%02X", chunk[cnt]);
		}
	}
	printf("\n");

	// Check for errors in fread
	if (ferror(stdin)) {
		perror("texmex error reading from stdin");
		exit(1);
	}
	
	exit(0);
}

void CheckStdin(void) {
	fd_set read_fds;
	struct timeval timeout;

	// Initialize the file descriptor set to include stdin (0)
	FD_ZERO(&read_fds);
	FD_SET(STDIN_FILENO, &read_fds); // STDIN_FILENO is 0

	// Set timeout to 0, which makes select non-blocking
	timeout.tv_sec = 0;  // seconds
	timeout.tv_usec = 250000; // microseconds

	// Check if data is available on stdin
	int ret = select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &timeout);

	if (ret == -1) {
		perror("select()");
		exit(1);
	} else if (ret == 0) {
		// No data available on stdin
		use_stdin = 0;
	} else {
		// There is data to process
		use_stdin = 1;
	}
}

int main(int argc, char **argv) {
	CheckStdin();
	if (use_stdin && argc > 2) {
		printf("texmex error: Only one option can be processed for stdin.\n");
		exit(1);
	}
	else if (use_stdin && argc < 2) {
		printf("texmex error: An option is required to process stdin data.\n");
		ShowHelp();
		exit(1);
	}
	else if (use_stdin)
		ProcessStdin(argv[1]);

	int c, cnt;
	while (1) {
		c = getopt_long(argc, argv, short_options, long_options, NULL);
		if (c == -1)
			break;
		switch (c) {
		case 'h':
			ShowHelp();
			exit(0);
			break;
		case 'V':
			printf("texmex %s\n", texmex_version_string);
			exit(0);
			break;
		case 'b':
			printf("%s\n", text2bin(optarg, strlen(optarg)));
			break;
		case 'B':
			printf("%s\n", bin2text(optarg, strlen(optarg)));
			break;
		case 'F':
			file2hex(optarg, strlen(optarg));
			break;
		case 'f':
			file2int(optarg, strlen(optarg));
			break;
		case 'H':
			printf("%s\n", hex2bin(optarg, strlen(optarg)));
			break;
		case 'I':
			printf("%s\n", int2bin(atoi(optarg)));
			break;
		case 'i':
			do_hex2int = 1;
			hex2int_arg = optarg;
			break;
		case 'j':
			int2hex(atoi(optarg));
			break;
		case 'l':
			text2line(optarg, strlen(optarg));
			exit(0);
			break;
		case 's':
			use_signed = 1;
			break;
		case 't':
			hex2text(optarg, strlen(optarg));
			break;
		case 'X':
			for (cnt = 0; cnt < strlen(optarg); cnt++) {
				printf("\\x%X", optarg[cnt]);
			}
			printf("\n");
			break;
		case 'x':
			for (cnt = 0; cnt < strlen(optarg); cnt++) {
				printf("%02X", optarg[cnt]);
			}
			printf("\n");
			break;
		}
	}

	if (do_hex2int) {
		if (use_signed)
			printf("%d\n", hex2int32(hex2int_arg, strlen(hex2int_arg)));
		else
			printf("%u\n", hex2uint32(hex2int_arg, strlen(hex2int_arg)));
	}

	return 0;
}

