char* ArtDir = "./art";

char* DefaultArt[10] = {
	"lime",
	"\x1b[38;5;8m  #==========#   ",
	"\x1b[38;5;8m  |\x1b[0m##########\x1b[38;5;8m|   ",
	"\x1b[38;5;8m  |\x1b[0m##########\x1b[38;5;8m|   ",
	"\x1b[38;5;8m  |\x1b[0m##########\x1b[38;5;8m|   ",
	"\x1b[38;5;8m  #==========#   ",
	"\x1b[38;5;8m     /####\\      ",
	"\x1b[38;5;237m================ ",
	"\x1b[38;5;237m|\x1b[38;5;1m@\x1b[38;5;250m\"\x1b[38;5;2m@\x1b[38;5;250m\"\"\"\"^=====\"\x1b[38;5;237m| ",
	"\x1b[38;5;237m================ "
};

#include <ctype.h>
#include <dirent.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __unix__
	#include <unistd.h>
	#include <regex.h>
#endif

#ifdef __linux__
	#include <sys/utsname.h>
	#include <sys/sysinfo.h>
	struct utsname UnixtimesystemInfo;
	struct sysinfo SystemInformation;
#endif

#define RESET "\x1b[0m"
#define LIME "\x1b[32m"
#define CYAN "\x1b[36m"
#define BRIGHTRED "\x1b[91m"
#define YELLOW "\x1b[93m"

#define VERSION "2.0"
#define PROGNAME "sysfetch"

char* OS;
char* Distro;
char InfoTable[9][255];

char* Art[30];
char* ArtChoice;
char* Colour;

void Die(const char* Format, ...) {
	va_list VArgs;
	va_start(VArgs, Format);
	vfprintf(stderr, Format, VArgs);
	va_end(VArgs);
	exit(1);
}

char *GetRegex(char *string, char *patrn, int *begin, int *end) { /* steal  */
	int i, w=0, len;                  
	char *word = NULL;
	regex_t rgT;
	regmatch_t match;
	regcomp(&rgT,patrn,REG_EXTENDED);
	if ((regexec(&rgT,string,1,&match,0)) == 0) {
    	*begin = (int)match.rm_so;
  		*end = (int)match.rm_eo;
    	len = *end-*begin;
    	word=malloc(len+1);
    	for (i=*begin; i<*end; i++) {
        	word[w] = string[i];
        	w++; 
    	}
    	word[w]=0;
  	}
	regfree(&rgT);
    return word;
}

char* ReplaceWord(const char* s, const char* oldW, const char* newW) {/* steal  */
    char* result;
    int i, cnt = 0;
    int newWlen = strlen(newW);
    int oldWlen = strlen(oldW);
    for (i = 0; s[i] != '\0'; i++) {
        if (strstr(&s[i], oldW) == &s[i]) {
            cnt++;
            i += oldWlen - 1;
        }
    }
    result = (char*)malloc(i + cnt * (newWlen - oldWlen) + 1);
    i = 0;
    while (*s) {
        if (strstr(s, oldW) == s) {
            strcpy(&result[i], newW);
            i += newWlen;
            s += oldWlen;
        }
        else
            result[i++] = *s++;
    }
    result[i] = '\0';
    return result;
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

char* GetLineFromFile(char* Path, char* Query) {
	char Buf[320];
	char* RetVal;
	FILE* File = fopen(Path, "r");
	if (!File) {
		return NULL;
	}
	if (strlen(Query) > 0) {
		while (fgets(Buf, sizeof(Buf), File)) {
			if (strstr(Buf, Query) != NULL) {
				RetVal = (char*) malloc(strlen(Buf) + 1);
				strcpy(RetVal, Buf);
				break;
			}	
		}
	} else { /* if query is nothing, this will get first line from file */
        fgets(Buf, sizeof(Buf), File);
        StrRemove(Buf, "\n");
        RetVal = (char*) malloc(strlen(Buf) + 1);
        strcpy(RetVal, Buf);
    }
	fclose(File);
	return RetVal;
}

char* ToLowerCase(char* S) {
	for (char *P=S; *P; P++) *P = tolower(*P);
	return S;
}

bool GetArtFile(char* Path) {
	bool Result = true;
	int i = 0;
	char* ProcessedString, Buffer[127];
	FILE* File = fopen(Path, "r");
	if (File) {
		while (fgets(Buffer, sizeof(Buffer), File)) {
			ProcessedString = (char*) malloc(strlen(Buffer) + 1);
			strcpy(ProcessedString, Buffer);
			StrRemove(ProcessedString, "\n");
			ProcessedString = ReplaceWord(ProcessedString, "COL", "\x1b");
			Art[i] = (char*) malloc(strlen(ProcessedString) + 1);
			strcpy(Art[i], ProcessedString);
			free(ProcessedString);
			i++;
		}
		fclose(File);
	} else {
		Result = false;
	}
	return Result;
}

void GetArt(char* Dist) {
	if (ArtChoice) {
		if (strcmp(ArtChoice, "default") == 0) {
			memcpy(Art, DefaultArt, sizeof(DefaultArt));
			return;
		} else {
			Dist = ArtChoice;	
		}
	}

	char* Path = (char*) malloc(strlen(Dist) + strlen(ArtDir) + 2);
	sprintf(Path, "%s/%s", ArtDir, Dist);

	if (!GetArtFile(Path)) {
		if (strcmp(OS, "Linux") == 0) {
			sprintf(Path, "%s/linux", ArtDir);
			if (!GetArtFile(Path)) {
				memcpy(Art, DefaultArt, sizeof(DefaultArt));
			}
		} else {
			memcpy(Art, DefaultArt, sizeof(DefaultArt));
		}
	}
}

char* GetColour() {
	char* Col;
	if (strcmp(Art[0], "lime") == 0) {
		Col = LIME;
	} else if (strcmp(Art[0], "cyan") == 0) {
		Col = CYAN;
	} else if (strcmp(Art[0], "red") == 0) {
		Col = BRIGHTRED;
	} else if (strcmp(Art[0], "yellow") == 0) {
		Col = YELLOW;
	} else {
		Col = RESET;
	}
	return Col;
}

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
	char* Dist, *PrettyName;
		#ifdef __linux__
			Dist = GetLineFromFile("/etc/os-release", "NAME=\"");
			if (Dist) {
				PrettyName = GetLineFromFile("/etc/os-release", "PRETTY_NAME");

				StrRemove(Dist, "NAME=\"");
				StrRemove(Dist, "\"\n");
				StrRemove(PrettyName, "PRETTY_NAME=\"");
				StrRemove(PrettyName, "\"\n");

				ToLowerCase(Dist);
				
				if (strstr(Dist, "gnu") != NULL) {
					StrRemove(Dist, " gnu/linux");
				} else if (strstr(Dist, "linux") != NULL) {
					StrRemove(Dist, " linux");		
				}

				GetArt(Dist);
				Colour = GetColour();

				sprintf(InfoTable[2], "%sOS%s: %s", Colour, RESET, PrettyName);				
			} else {
				Dist = "Unknown";
			}		
		#endif
	return Dist;	
}

char* GetTitle() {
	char* Title;
	#ifdef __unix__
		char* Hostname = (char*) malloc(64 * sizeof(char*));
		gethostname(Hostname, 64);
		char* Username = getenv("USER");
		Title = (char*) malloc(sizeof(Hostname) + sizeof(Username) + 1); // for some reason i get malloc corrupted top size if i dont multiply size of user and hostname by 2
		sprintf(Title, "%s%s%s@%s%s%s", Colour, Username, RESET, Colour, Hostname, RESET);
		free(Hostname);
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
	if (strcmp(OS, "Unknown") == 0)
		Die("Unknown OS\n");

	Distro = GetDistro();
	#ifdef __linux__
		uname(&UnixtimesystemInfo);
		sysinfo(&SystemInformation);
	#endif

	#ifdef __unix__
		strcpy(InfoTable[0], GetTitle());
		strcpy(InfoTable[1], "----------");
		sprintf(InfoTable[3], "%sHost%s: %s", Colour, RESET, GetModel()); 
		sprintf(InfoTable[4], "%sKernel%s: %s", Colour, RESET, GetKernel());
		sprintf(InfoTable[5], "%sUptime%s: %s", Colour, RESET, GetUptime());
		sprintf(InfoTable[6], "%sShell%s: %s", Colour,  RESET, GetShell());
		sprintf(InfoTable[7], "%sCPU%s: %s", Colour, RESET, GetCPU());
		sprintf(InfoTable[8], "%sMEM%s: %s", Colour, RESET, GetMem());
	#endif
}

void PrintInfo() {
	char* Spacer; /* used if ascii art is smaller than 9 lines */
	
	/* get art lines */
	int ArtLen;
	for (int i = 1;  ; i++) { /* get rows of ascii art */
		if (!Art[i]) {
			ArtLen = i - 1;
			break;
		}
	}
	
	/* if art is less than 9 lines */
	if (ArtLen < 9) {
		int X, Y;
		char* huh = (char*) malloc(strlen(Art[1]));
		strcpy(huh, Art[1]);
		char* EscapeCodePurge = GetRegex(huh, "\x1b(.*?)m", &X, &Y);
		if (EscapeCodePurge) {
			StrRemove(huh, EscapeCodePurge);	
		}
		Spacer = (char*) malloc(strlen(huh));
		int Idk = 0;
		for (unsigned int i = 0; i < strlen(huh); i++)
			Spacer[i] = ' ';

		#ifdef __unix__
			for (int i = 0; i < ArtLen; i++) {
				printf("%s  %s%s\n", Art[i + 1], RESET, InfoTable[i]);
				Idk++;
			}
			for (int i = Idk; i < 9; i++) {
				printf("%s  %s%s\n", Spacer, RESET, InfoTable[i]);
			}
		#endif
	} else {
		#ifdef __unix__
			for (int i = 0; i < 9; i++) {
				printf("%s  %s%s\n", Art[i + 1], RESET, InfoTable[i]);
			}
			if (Art[9]) {
				for (int i = 10; i < ArtLen + 1; i++)
					printf("%s\n", Art[i]);
			}
		#endif
	}
}

int main(int argc, char** argv) {
	if (argc > 1) {
		if (strcmp(argv[1], "-v") == 0) {
			Die("%s v%s\n", PROGNAME, VERSION);
		} else if (strcmp(argv[1], "-d") == 0) {
			if (argv[2]) {
				ArtChoice = argv[2];
			} else {
				Die("d\n");
			}	
		} else if (strcmp(argv[1], "-dl") == 0) { 
			DIR* Dir = opendir(ArtDir);
			if (Dir == NULL) {
				printf("Can't find entries in '%s', please be sure to specify correct path by editing 'ArtDir' variable or put files on the directory specified.\n", ArtDir);
				printf("default\n");
				Die("");
			}
			struct dirent* Entry;
			printf("default\n");
			while ((Entry = readdir(Dir))) {
				if (strcmp(Entry->d_name, "..") == 0 || strcmp(Entry->d_name, ".") == 0) {
					continue;
				}
				printf("%s\n", Entry->d_name);
			}
			closedir(Dir);
			Die("");
		} else {
			Die("usage: %s [-v] [-d distro]\n", PROGNAME);
		}
	}
	GetInfo();
	PrintInfo();
	return 0;
}
