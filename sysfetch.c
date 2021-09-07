char* Directory = "/usr/share/sysfetch";

char* DefaultArt[7] = {
	"32",
	"   .----.  ",
	"  /|\x1b[32m$_\x1b[0;1m  |  ",
	"  L|____|_ ",
	"/| .__    |",
	"L|________|",
	"  L\\______\\"
};

#include <dirent.h>
#include <regex.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <unistd.h>

char* OS = '\0',
    * Distro = '\0';

#define PROGNAME "sysfetch"
#define PROGVER "4.0"

#define RESET "\x1b[0m"
#define BOLD "\x1b[1m"

/* string operations */
char* RemoveSubstring(char *str, const char *sub) { /* stolen from stack */
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

int ReplaceAllOccurences(char* i_str, char* i_orig, char* i_rep) { /* steal */
   char l_before[150], l_after[150];
   char* l_p;
   int l_origLen;
   l_origLen = strlen(i_orig);

   while ((l_p = strstr(i_str, i_orig))) {
      sprintf(l_before, "%.*s", l_p - i_str, i_str);
      sprintf(l_after ,"%s" ,l_p + l_origLen);
      sprintf(i_str ,"%s%s%s" ,l_before ,i_rep ,l_after);
   }
   return strlen(i_str);
}

char* GetRegex(char *string, char *patrn, int *begin, int *end) { /* steal  */
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

char* GetStringFromFile(char* Path, char* String) {
	static char Buf[512];
	FILE* File = fopen(Path, "r");
	if (File) {
		if (strlen(String) > 0) { 
			while(fgets(Buf, sizeof(Buf), File)) {
				if (strstr(Buf, String) != NULL) {
					break;
				}
			}
		} else { /* get 1st line if String is nothing */
			fgets(Buf, sizeof(Buf), File);
			RemoveSubstring(Buf, "\n");
		}
		fclose(File);
	}
	return Buf;
}

unsigned long StringToULong(char* Source) {
	unsigned long Return = 0;
	for (unsigned int i = 0; i < strlen(Source); i++) {
		if (Source[i] < 48 || Source[i] > 57) /* skip non number characters */
			continue;
		Return = Return * 10 + (Source[i] - '0');
	}
	return Return;
}

int CleanStringLen(char* Source) { /* removes newline and x1b escape sequence */
	int x, y;
	char* Ret = Source;
	char* Match = GetRegex(Ret, "\x1b(.*)m", &x, &y);
	if (Match)
		RemoveSubstring(Ret, Match);
	return strlen(Ret);
}

/* other funtions */
void eprintf(const char* Format, ...) {
	va_list VArgs;
	va_start(VArgs, Format);
	vfprintf(stderr, Format, VArgs);
	va_end(VArgs);
	exit(1);
}

/* Information gahtering */
void GetOS(void) {
    char* Buf;
	struct utsname Buf2;
	if (uname(&Buf2) < 0)
		eprintf("Failed to buffer utsname\n");
		
	Buf = Buf2.sysname;
	
	if (strcmp(Buf, "Linux") == 0 ||
	    strstr(Buf, "GNU") != NULL
	) {
	    OS = "Linux";
	} else if (
		strstr(Buf, "BSD") != NULL ||
		strcmp(Buf, "DragonFly") == 0
	) {
		OS = "BSD";
	} else {
		eprintf("Unknown operating system '%s'\n", Buf);		
	}
}

void GetDistro(void) {
	if (strcmp(OS, "Linux") == 0) {
		if ((Distro = GetStringFromFile("/etc/os-release", "ID"))) {
			RemoveSubstring(Distro, "ID=");
			RemoveSubstring(Distro, "\n");
		}
	}
}

void PrintInfo(int n, char* Colour) {
	static char* CurrentUsername,
	             Hostname[64];	
	switch(n) {
		case 0: {
			CurrentUsername = getenv("USER");
			gethostname(Hostname, 64);
			printf("%s%s%s%s@%s%s%s%s\n", BOLD, Colour, CurrentUsername, RESET, BOLD, Colour, Hostname, RESET);
			break;
		}
		case 1: {
			char Separator[strlen(CurrentUsername) + strlen(Hostname) + 2];
			for (unsigned i = 0; i < (strlen(CurrentUsername) + strlen(Hostname) + 1); i++) {
				Separator[i] = '-';
			}
			Separator[strlen(CurrentUsername) + strlen(Hostname) + 1] = '\0';
			printf("%s\n", Separator);
			break;
		}
		case 2: {
			struct utsname Buffer;
			char* Buffer2,
			      Buffer3[256];
			if (uname(&Buffer) != 0)
				eprintf("Failed to buffer utsname\n");
			if (strcmp(OS, "Linux") != 1) {
				if ((Buffer2 = GetStringFromFile("/etc/os-release", "PRETTY"))) {
					RemoveSubstring(Buffer2 , "\"\n");
					RemoveSubstring(Buffer2, "PRETTY_NAME=\"");
					snprintf(Buffer3, 255, "%s %s", Buffer2, Buffer.machine);
				}
			}
			printf("%s%sOS%s: %s\n", BOLD, Colour, RESET, Buffer3);
			break;
		}
		case 3: {
			char* Buffer;
			if (strcmp(OS, "Linux") == 0) {
				if (!(Buffer = GetStringFromFile("/sys/devices/virtual/dmi/id/product_name", "")))
					Buffer = "Unknown";
			}
			printf("%s%sHost%s: %s\n", BOLD, Colour, RESET, Buffer);
			break;
		}
		case 4: {
			struct utsname Buffer;
			if (uname(&Buffer) != 0)
				eprintf("Failed to buffer utsname\n");
			printf("%s%sKernel%s: %s\n", BOLD, Colour, RESET, Buffer.release);
			break;
		}
		case 5: {
			if (strcmp(OS, "Linux") == 0) {
				struct stat Buffer;
				lstat("/sbin/init", &Buffer);
				if (S_ISLNK(Buffer.st_mode)) {
					printf("%s%sInit%s: %s\n", BOLD, Colour, RESET, realpath("/sbin/init", NULL));
				} else {
					printf("%s%sInit%s: System V\n", BOLD, Colour, RESET);
				}
			}
			break;
		}
		case 6: {
			char Buffer[256];
			if (strcmp(OS, "Linux") == 0) {
				struct sysinfo Sysinfo;
				if (sysinfo(&Sysinfo) != 0)
					eprintf("Failed to buffer sysinfo");		
				long UptimeS = Sysinfo.uptime;
				(UptimeS < 60) 
					? snprintf(Buffer, 255, "%ld s", UptimeS)
					: snprintf(Buffer, 255, "%ld min", UptimeS / 60);
			}
			printf("%s%sUptime%s: %s\n", BOLD, Colour, RESET, Buffer);
			break;
		}
		case 7: {
			printf("%s%sShell%s: %s\n", BOLD, Colour, RESET, getenv("SHELL"));
			break;
		}
		case 8: {
			char* Buffer;
			if (strcmp(OS, "Linux") == 0) {
				if ((Buffer = GetStringFromFile("/proc/cpuinfo", "model name"))) {
					RemoveSubstring(Buffer, "model name\t: ");
					RemoveSubstring(Buffer, "\n");
					RemoveSubstring(Buffer, "(R)");
					RemoveSubstring(Buffer, "(TM)");
					RemoveSubstring(Buffer, " CPU");
				} else {
					Buffer = "Unknown";
				}
			}
			printf("%s%sCPU%s: %s\n", BOLD, Colour, RESET, Buffer);
			break;
		}
		case 9: {
			char Buffer[256];
			if (strcmp(OS, "Linux") == 0) {
				struct sysinfo Sysinfo;
				if (sysinfo(&Sysinfo) != 0)
					eprintf("Failed to buffer sysinfo");
				unsigned long TotalMemMB = StringToULong(GetStringFromFile("/proc/meminfo", "MemTotal")) / 1024,
				              ShMemMB = StringToULong(GetStringFromFile("/proc/meminfo", "Shmem")) / 1024,
				              FreeMemMB = StringToULong(GetStringFromFile("/proc/meminfo", "MemFree")) / 1024,
				              BufferMemMB = StringToULong(GetStringFromFile("/proc/meminfo", "Buffers")) / 1024,
				              SReclaimMemMB = StringToULong(GetStringFromFile("/proc/meminfo", "SReclaimable")) / 1024,
				              CachedMemMB = StringToULong(GetStringFromFile("/proc/meminfo", "Cached")) / 1024;
				snprintf(Buffer, 255, "%ldMiB / %ldMiB", (TotalMemMB + ShMemMB) - FreeMemMB - BufferMemMB - CachedMemMB - SReclaimMemMB, TotalMemMB);
			}
			printf("%s%sMEM%s: %s\n", BOLD, Colour, RESET, Buffer);
			break;
		}
		case 10: {
			for (int i = 40; i < 48; i++)
				printf("\x1b[%dm   ", i);
			printf("%s\n", RESET);
			break;
		}
		case 11: {
			printf("%s", BOLD);
			for (int i = 100; i < 108; i++)
				printf("\x1b[%dm   ", i);
			printf("%s\n", RESET);
			break;
		}
		default: {
			printf("\n");
			break;
		}
	}
}

int PrintFromArtfile(char* Artfile) {
	int Return = 0;
	char Buffer[50];
	snprintf(Buffer, 50, "%s/%s", Directory, Artfile);
	FILE* File = fopen(Buffer, "r");
	if (File) {
		char Buffer2[256], 
		     Colour[30],
		     Spacer[128];
		int Counter = 0;
		while (fgets(Buffer2, 255, File) != NULL) {
			switch (Counter) {
				case 0: {
					RemoveSubstring(Buffer2, "\n");
					snprintf(Colour, 30, "\x1b[%sm", Buffer2);
					break;
				}
				case 1: {
					strncpy(Spacer, Buffer2, 127);
					RemoveSubstring(Spacer, "\n");

					int x, y;
					char* Match = GetRegex(Spacer, "COL(.*)m", &x, &y);
					if (Match)
						RemoveSubstring(Spacer, Match);

					for (int j = 0;; j++) {
						if (!Spacer[j])
							break;
						if (Spacer[j] != ' ' && Spacer[j] != '\0')
							Spacer[j] = ' ';
					}
					
					RemoveSubstring(Buffer2, "\n");
					ReplaceAllOccurences(Buffer2, "COL", "\x1b[");
					printf("%s%s%s  ", BOLD, Buffer2, RESET);
					PrintInfo(Counter - 1, Colour);
					break;
				}
				default: {
					RemoveSubstring(Buffer2, "\n");
					ReplaceAllOccurences(Buffer2, "COL", "\x1b[");
					printf("%s%s%s  ", BOLD, Buffer2, RESET);
					PrintInfo(Counter - 1, Colour);
					break;
				}
			}
			Counter++;
		}
		if (Counter < 12) {
			for (int i = Counter; i < 13; i++) {
				printf("%s  ", Spacer);
				PrintInfo(i - 1, Colour);
			}
		}
		fclose(File);
		exit(1);
	} else {
		Return = 1;
	}
	return Return;
}

void PrintDefault() {
	char Colour[25];
	int Counter = 1;
	snprintf(Colour, 25, "\x1b[%sm", DefaultArt[0]);
	int ArtSize = sizeof(DefaultArt) / sizeof(DefaultArt[0]);
	for (int i = 1; i < ArtSize; i++) {
		printf("%s%s%s  ", BOLD, DefaultArt[i], RESET);
		PrintInfo(i - 1, Colour);
		Counter++;
	}
	if (ArtSize < 13) {
		char Spacer[CleanStringLen(DefaultArt[1]) + 1];
		for (int i = 0; i < CleanStringLen(DefaultArt[1]); i++) {
			Spacer[i] = ' ';
		}
		Spacer[CleanStringLen(DefaultArt[1])] = '\0';
		for (int i = Counter; i < 13; i++) {
			printf("%s  ", Spacer);
			PrintInfo(i - 1, Colour);
		}
	}
}

void PrintResult(char* Artfile) {
	if (strcmp(Artfile, "default") == 0) {
		PrintDefault();
	} else {
		if (PrintFromArtfile(Artfile) != 0) {
			if (strcmp(OS, "Linux") == 0) {
				if (PrintFromArtfile("linux") != 0)
					PrintDefault();			
			} else {
				PrintDefault();
			}
		}
	}
}

int main(int argc, char* argv[]) {
	char* ArtChoice = '\0';
	if (argc > 1) {
		if (argv[1][0] == '-') {
			switch (argv[1][1]) {
				case 'V': {
					eprintf("%s\n", PROGVER);
					break;
				}
				case 'D': {
					struct dirent* Entry;
					DIR* Dir = opendir(Directory);
					if (!Dir)
						eprintf("Missing directory '%s'\n", Directory);
					puts("default");
					while ((Entry = readdir(Dir))) {
						if (strcmp(Entry->d_name, "..") == 0 || strcmp(Entry->d_name, ".") == 0)
							continue;
						printf("%s\n", Entry->d_name);
					}
					closedir(Dir);
					exit(0);
					break;
				}
				case 'd': {
					if (argv[2])
						ArtChoice = argv[2];
					break;
				}
 			}
		} else {
			eprintf("usage: %s [-d logo] [-D] [-V]\n", PROGNAME);
		}
	}
	GetOS();
	GetDistro();
	if (ArtChoice)
		PrintResult(ArtChoice);
	else 
		PrintResult(Distro);
	return 0;
}
