#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <getopt.h>

const char *texmex_version_string = "0.1.11";

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

unsigned int use_signed;

char *text2bin(char *text) {
	char *str = (char *)malloc(strlen(text)*8+1);
	memset(str, 0, strlen(text)*8+1);
	char c = *text, mask = 0x80;
	unsigned int cnt, cnt2;
	for (cnt = 0; cnt < strlen(text); cnt++) {
		for (c = text[cnt], cnt2 = 0; cnt2 <= 7; cnt2++) {
			str[(cnt+1)*8-8+cnt2] = (c & mask)?'1':'0';
			c = (c << 1);
		}
	}

	return str;
}

char *bin2text(char *text) {
	char *str = (char *)malloc(strlen(text)/8+1);
	char c;
	unsigned int cnt, cnt2, cnt3;
	for (cnt = 0, cnt3 = 0; cnt < strlen(text); cnt+=8, cnt3++) {
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

void file2hex(char *filename) {
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

void file2int(char *filename) {
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

int hex2int32(char *hex) {
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

unsigned int hex2uint32(char *hex) {
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

char *hex2bin(char *hexstr) {
	char *c = hexstr;
	char *binstr = (char *)malloc(1024),
		*buffer = (char *)malloc(1024);
	memset(binstr, 0, 1024);
	while (1) {
		sprintf(buffer, "%s", binstr);
		sprintf(binstr, "%s%s", buffer, hex4bit2bin(*c++));
		if (*c == '\0')
			break;
	}
	return binstr;
}

char hex2char(char hex[2]) {
	return hex2int(hex[0])*16 + hex2int(hex[1]);
}

void int2hex(unsigned int val) {
	printf("%X\n", val);
}

void hex2text(char *text) {
	char *buffer = (char *)malloc(strlen(text)+1);
	memset(buffer, 0, strlen(text)+1);
	char cart[2];
	int cnt, iter;
	for (cnt = 0, iter = 0; cnt < strlen(text); cnt += 2, iter++) {
		cart[0] = *(text+cnt);
		cart[1] = *(text+cnt+1);
		*(buffer+iter) = hex2char(cart);
	}
	printf("%s\n", buffer);
}

void text2line(char *text) {
	char *buffer = (char *)malloc(strlen(text));

	int cnt;
	for (cnt = 0; cnt < strlen(text); cnt++) {
		if (*(text+cnt) != '\n')
			*(buffer+cnt) = *(text+cnt);
		else
			*(buffer+cnt) = ' ';
	}

	printf("%s\n", buffer);
	free(buffer);
}

int main(int argc, char **argv) {
	int c, cnt;
	while (1) {
		c = getopt_long(argc, argv, short_options, long_options, NULL);
		if (c == -1)
			break;
		switch (c) {
		case 'h':
			printf("texmex options:\n\t-h, --help\n"
				"\t-V, --version\n"
				"\t-l, --text2line PARAGRAPH\n"
				"\t-t, --hex2text \"AFDE08fa\"\n"
				"\t-B, --bin2text 00001111\n"
				"\t-b, --text2bin TEXT\n"
				"\t-F, --file2hex FILENAME\n"
				"\t-f, --file2int FILENAME\n"
				"\t-H, --hex2bin FFEE8844\n"
				"\t-I, --int2bin 255\n"
				"\t-i, --hex2int FFEE8844\n"
				"\t-s, --signed\n"
				"\t-X, --text2hex_escape TEXT\n"
				"\t-x, --text2hex TEXT }\n");
			exit(0);
			break;
		case 'V':
			printf("texmex %s\n", texmex_version_string);
			exit(0);
			break;
		case 'b':
			printf("%s\n", text2bin(optarg));
			break;
		case 'B':
			printf("%s\n", bin2text(optarg));
			break;
		case 'F':
			file2hex(optarg);
			break;
		case 'f':
			file2int(optarg);
			break;
		case 'H':
			printf("%s\n", hex2bin(optarg));
			break;
		case 'I':
			printf("%s\n", int2bin(atoi(optarg)));
			break;
		case 'i':
			if (use_signed)
				printf("%d\n", hex2int32(optarg));
			else
				printf("%u\n", hex2uint32(optarg));
			break;
		case 'j':
			int2hex(atoi(optarg));
			break;
		case 'l':
			text2line(optarg);
			exit(0);
			break;
		case 's':
			use_signed = 1;
			break;
		case 't':
			hex2text(optarg);
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
}

