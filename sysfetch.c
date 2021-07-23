/*
 * sysfetch
 */
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "asciiart.h"

/* os specific declarations */
#ifdef __unix__
	#include <unistd.h>
#endif

#ifdef __linux__
	#include <sys/utsname.h>
	#include <sys/sysinfo.h>
	struct utsname UnixtimesystemInfo;
	struct sysinfo SystemInformation;
#endif

/* colour escape codes for vt100 emulators */
#define RESET "\x1b[0m"

/* variables */
#define VERSION "1.1"
#define PROGNAME "sysfetch"

char InfoTable[9][127];

char* OS;
char* Distro;

char* Ascii[30];
char* AsciiDistroChoice;

/* string operations */
char* GetLineFromFile(char* Path, char* Query) {
	char Buf[320];
	char* RetVal = malloc(sizeof(Buf));
	FILE* File = fopen(Path, "r");
	if (!File) {
		return NULL;
	}
	if (strlen(Query) > 0) {
		while (fgets(Buf, sizeof(Buf), File)) {
			if (strstr(Buf, Query) != NULL) {
				strcpy(RetVal, Buf);
				break;
			}	
		}
	} else { /* if query is nothing, this will get first line from file */
        fscanf(File, "%[^\n]", Buf);
        strcpy(RetVal, Buf);
    }
	fclose(File);
	return RetVal;
}

char* StrRemove(char *str, const char *sub) { /* stolen from stack */
    size_t len = strlen(sub);
    if (len > 0) {
        char *p = str;
        size_t size = 0;
        while ((p = strstr(p, sub)) != NULL) {
            size = (size == 0) ? (p - str) + strlen(p + len) + 1 : size - len;
            memmove(p, p + len, size - (p - str));
        }
    }
    return str;
}

char* ToLowerCase(char* s) { /* same as previous */
	for(char *p=s; *p; p++) *p=tolower(*p);
	return s;
}

/* other functions */
void Die(const char* Format, ...) {
	va_list VArgs;
	va_start(VArgs, Format);
	vfprintf(stderr, Format, VArgs);
	va_end(VArgs);
	exit(1);
}

/* info gathering */
char* GetOS() {
	char* OS;
	#ifdef __linux__
		OS = "Linux";
	#else
		OS = "Unknown";
	#endif
	return OS;
}

char* GetDistro() {
	char* Distro;
	#ifdef __linux__
		Distro = GetLineFromFile("/etc/os-release", "NAME=\"");
		if (Distro != NULL) {
			StrRemove(Distro, "NAME=\"");
			StrRemove(Distro, "\"\n");
			sprintf(InfoTable[2], "OS: %s", GetLineFromFile("/etc/os-release", "PRETTY_NAME"));
			StrRemove(InfoTable[2], "PRETTY_NAME=\"");
			StrRemove(InfoTable[2], "\"\n");
		} else {
			Distro = "Unknown";
		}
	#endif
	return Distro;
}

char* GetTitle() {
	char* Title;
	#ifdef __unix__
		char* Hostname = (char*) malloc(127);
		gethostname(Hostname, 127);
		char* Username = getenv("USER");
		Title = (char*) malloc(sizeof(Hostname) + sizeof(Username) + 1);
		sprintf(Title, "%s@%s", Username, Hostname);
	#endif
	return Title;
}

char* GetModel() {
	char* Model;
	#ifdef __linux__
		Model = GetLineFromFile("/sys/devices/virtual/dmi/id/product_name", "");
		if (Model == NULL) {
			Model = "Unknown";
		}
	#endif
	return Model;
}

char* GetKernel() {
	char* Kernel;
	#ifdef __unix__
		Kernel = UnixtimesystemInfo.release;
	#endif
	return Kernel;
}

char* GetUptime() {
	char* Uptime = (char*) malloc(42);
	#ifdef __linux__
		long UptimeS = SystemInformation.uptime;
		if (UptimeS < 60) {
			sprintf(Uptime, "%ld s", UptimeS); 
		} else {
			long UptimeM = UptimeS / 60;
			sprintf(Uptime, "%ld min", UptimeM);
		}
	#endif
	return Uptime;
}

char* GetShell() {
	char* Shell;
	#ifdef __unix__
		Shell = getenv("SHELL");
	#endif
	return Shell;
}

char* GetCPU() {
	char* Cpu;
	#ifdef __linux__
		Cpu = GetLineFromFile("/proc/cpuinfo", "model name");
		if (Cpu != NULL) {
			StrRemove(Cpu, "model name\t: ");
			if (strstr(Cpu, "Intel") != NULL) {
				StrRemove(Cpu, "(R)");
				StrRemove(Cpu, "(TM)");
				StrRemove(Cpu, "CPU ");
				StrRemove(Cpu, "\n");
			}
		} else {
			Cpu = "Unknown";
		}
	#endif
	return Cpu;
}

char* GetMem() {
	char* Mem = (char*) malloc(32);
	#ifdef __linux__
		unsigned long TMem = SystemInformation.totalram / (1024 * 1024);
		unsigned long UMem = (SystemInformation.totalram  - SystemInformation.freeram) / (1024 * 1024);
		sprintf(Mem, "%lu MiB / %lu MiB", UMem,  TMem);
	#endif
	return Mem;
}

void GetInfo() {
	OS = GetOS();
	if (strcmp(OS, "Unknown") == 0) {
		Die("Unknown OS");
	}
	#ifdef __linux__
		uname(&UnixtimesystemInfo);
		sysinfo(&SystemInformation);
	#endif
	Distro = GetDistro();
	strcpy(InfoTable[0], GetTitle());
	strcpy(InfoTable[1], "--------");
	sprintf(InfoTable[3], "Host: %s", GetModel());
	sprintf(InfoTable[4], "Kernel: %s", GetKernel());
	sprintf(InfoTable[5], "Uptime: %s", GetUptime());
	sprintf(InfoTable[6], "Shell: %s", GetShell());
	sprintf(InfoTable[7], "CPU: %s", GetCPU());
	sprintf(InfoTable[8], "MEM: %s", GetMem());
}

/* printing information */
void SetAscii() {
	char* Dist;
	if (AsciiDistroChoice) {
		Dist = AsciiDistroChoice;
	} else {
		Dist = Distro;
		ToLowerCase(Dist);
		if (strstr(Dist, "gnu") != NULL) {
			StrRemove(Dist, " gnu/linux");
		} else if (strstr(Dist, "linux") != NULL) {
			StrRemove(Dist, " linux");		
		}
	}
	/* warning, a chain of if statements */
	if (strcmp(OS, "Linux") == 0) {
		if (strcmp(Dist, "arch") == 0) {
			memcpy(Ascii, arch, sizeof(arch));
		} else if (strcmp(Dist, "artix") == 0) {
			memcpy(Ascii, artix, sizeof(artix));
		} else if (strcmp(Dist, "tux") == 0 || strcmp(Dist, "linux") == 0) {
			memcpy(Ascii, tux, sizeof(tux));
		} else {
			memcpy(Ascii, tux, sizeof(tux));
		}
	}
}

void PrepInfo() {
	char* Spacer;/* used if ascii art is smaller than 9 lines */
	SetAscii();
	int ArtLen;
	for (int i = 0; ; i++) { // get rows of ascii art
		if (!Ascii[i]) {
			ArtLen = i;
			break;	
		}
	}
	if (ArtLen < 9) {
		Spacer = (char*) malloc(strlen(Ascii[0]));
		int Idk = 0;
		for (unsigned int i = 0; i < strlen(Ascii[0]); i++)
			Spacer[i] = ' ';
		for (int i = 0; i < ArtLen; i++) {
			#ifdef __linux__
				printf("%s\t%s%s\n", Ascii[i], RESET, InfoTable[i]);
			#else 
				printf("%s\t%s\n", Ascii[i], InfoTable[i]);
			#endif
			Idk++;
		}
		for (int i = Idk; i < 9; i++) {
			#ifdef __linux
				printf("%s\t%s%s\n", Spacer, RESET, InfoTable[i]);
			#else
				printf("%s\t%s\n", Spacer, InfoTable[i]);
			#endif
		}
		free(Spacer);
		#ifdef __linux__
			printf(RESET);
		#endif
	} else {
		for (int i = 0; i < 9; i++) {
			#ifdef __linux__
				printf("%s\t%s%s\n", Ascii[i], RESET, InfoTable[i]);
			#else
				printf("%s\t%s\n", Ascii[i], InfoTable[i]);
			#endif
		}
		if (Ascii[9]) {
			for (int i = 9; i < ArtLen; i++)
				printf("%s\n", Ascii[i]);
		}
		#ifdef __linux__
			printf(RESET);
		#endif
	}
}
/* main sequence */
int main(int argc, char** argv) {
	if (argc > 1) {
		if (strcmp(argv[1], "-v") == 0) {
			Die("%s\n", VERSION);
		} else if (strcmp(argv[1], "-d") == 0) {
			if (argv[2]) {
				for (unsigned int i = 0; i < sizeof(Distros) / sizeof(Distros[0]); i++) {
					if (strcmp(argv[2], Distros[i]) == 0) {
						AsciiDistroChoice = Distros[i];
						break;
					}
				}
				if (!AsciiDistroChoice) {
					Die("invalid selection. To see aviable choices for '-d', run '%s -dl'\nusage: %s [-v] [-d distro]\n", PROGNAME, PROGNAME);
				}
			} else {
				Die("invalid selection. To see aviable choices for '-d', run '%s -dl'\nusage: %s [-v] [-d distro]\n", PROGNAME, PROGNAME);
			}
		} else if (strcmp(argv[1], "-dl") == 0) {
			for (unsigned int i = 0; i < sizeof(Distros) / sizeof(Distros[0]); i++) {
				printf("%s\n", Distros[i]);
			}
			Die("");
		} else {
			Die("usage: %s [-v] [-d distro]\n", PROGNAME);
		}
	}
	GetInfo();
	PrepInfo();
	return 0;
}
