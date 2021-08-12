char* ArtDir = "/usr/local/share/sysfetchart";

char* Default[10] = {
	"38;5;10",
	"\x1b[38;5;8m  #==========#   ",
	"\x1b[38;5;8m  |\x1b[0m##########\x1b[38;5;8m|   ",
	"\x1b[38;5;8m  |\x1b[0m##########\x1b[38;5;8m|   ",
	"\x1b[38;5;8m  |\x1b[0m##########\x1b[38;5;8m|   ",
	"\x1b[38;5;8m  #==========#   ",
	"\x1b[38;5;8m     /####\\      ",
	"\x1b[38;5;237m================ ",
	"\x1b[38;5;237m|\x1b[38;5;1m@\x1b[38;5;10m\"\x1b[38;5;2m@\x1b[38;5;250m\"\"\"\"^=====\"\x1b[38;5;237m| ",
	"\x1b[38;5;237m================ "
};

#include <dirent.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <regex.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unistd.h>

#define RESET "\x1b[0m"

#define PROGNAME "sysfetch"
#define PROGVER "3.0"

char *OS, *KernelName, *KernelRelease, *Distro, *PrettyDistro, *Colour, *Username, *Hostname, *Model, *Uptime, *Shell, *CPU, *Mem, *ArtChoice;
char* Art[35];
char Info[9][149];

void Die(const char* Format, ...) {
	va_list VArgs;
	va_start(VArgs, Format);
	vfprintf(stderr, Format, VArgs);
	va_end(VArgs);
	exit(1);
}

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

char* GetStringFromFile(char* Path, char* String) {
	char* Ret, Buf[512];
	FILE* File = fopen(Path, "r");
	if (File) {
		if (strlen(String) > 0) { 
			while(fgets(Buf, sizeof(Buf), File)) {
				if (strstr(Buf, String) != NULL) {
					Ret = (char*) malloc(strlen(Buf) + 1);
					strcpy(Ret, Buf);
					break;
				}
			}
		} else { /* get 1st line if String is nothing */
			fgets(Buf, sizeof(Buf), File);
			RemoveSubstring(Buf, "\n");
			Ret = (char*) malloc(strlen(Buf));
			strcpy(Ret, Buf);
		}
		fclose(File);
	}
	return Ret;
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

int CleanStringLen(char* Source) { /* removes newline and x1b escape sequence */
	int x, y;
	char* Ret = Source;
	char* Match = GetRegex(Ret, "\x1b(.*)m", &x, &y);
	if (Match)
		RemoveSubstring(Ret, Match);
	return strlen(Ret);
}

/* info gathering */
char* GetOS() {
	char* Ret;
	struct utsname Buf;
	if (uname(&Buf) < 0)
		Die("Can't get system name\n");
	
	KernelName = (char*) malloc(strlen(Buf.sysname) + 1);
	KernelRelease = (char*) malloc(strlen(Buf.release) + 1);
	strcpy(KernelName, Buf.sysname);
	strcpy(KernelRelease, Buf.release);

	if (strcmp(KernelName, "Linux") == 0 || strstr(KernelName, "GNU") != NULL)
		Ret = "Linux";
	else if ((strstr(KernelName, "BSD") != NULL || strcmp(KernelName, "DragonFly")) == 0)
		Ret = "BSD";
	else
		Die("Unknown OS '%s'\n", KernelName);
	return Ret;
}

char* GetDistro() {
	char* Ret;
	if (strcmp(OS, "Linux") == 0) {
		if ((Ret = GetStringFromFile("/etc/os-release", "ID"))) {
			Ret = RemoveSubstring(Ret, "\n");
			Ret = RemoveSubstring(Ret, "ID=");
			PrettyDistro = GetStringFromFile("/etc/os-release", "PRETTY");
			RemoveSubstring(PrettyDistro, "\"\n");
			RemoveSubstring(PrettyDistro, "PRETTY_NAME=\"");
		}
	} else if (strcmp(OS, "BSD") == 0) {
		Ret = KernelName;
		PrettyDistro = (char*) malloc(strlen(KernelName) + strlen(KernelRelease) + 2);
		snprintf(PrettyDistro, sizeof(PrettyDistro), "%s %s", KernelName, KernelRelease);
	}
	return Ret;
}

char* GetUserName() {
	return getenv("USER");
}

char* GetHostName() {
	char* Ret = (char*) malloc(64);
	gethostname(Ret, sizeof(Ret));
	return Ret;
}

char* GetModel() {
	char* Ret;
	if (strcmp(OS, "Linux") == 0) {
		if(!(Ret = GetStringFromFile("/sys/devices/virtual/dmi/id/product_name", ""))) {
			Ret = "Unknown";
		}
	}
	return Ret;
}

char* GetUptime() {
	char* Ret = (char*) malloc(32);
	struct sysinfo Sysinfo;
	if (sysinfo(&Sysinfo) == 0) {
		long UptimeS = Sysinfo.uptime;
		(UptimeS < 60) 
			? snprintf(Ret, sizeof(Ret), "%ld s", UptimeS)
			: snprintf(Ret, sizeof(Ret), "%ld min", UptimeS / 60);
	} else {
		Ret = "Unknown";
	}
	return Ret;
}

char* GetSh() {
	return getenv("SHELL");
}

char* GetCPU() {
	char* Ret;
	if (strcmp(OS, "Linux") == 0) {
		if((Ret = GetStringFromFile("/proc/cpuinfo", "model name"))) {
			RemoveSubstring(Ret, "model name\t: ");
			RemoveSubstring(Ret, "\n");
			if (strstr(Ret, "Intel") != NULL) {
				RemoveSubstring(Ret, "(R)");
				RemoveSubstring(Ret, "(TM)");
				RemoveSubstring(Ret, " CPU");
			};
		} else {
			Ret = "Unknown";
		}
	}
	return Ret;
}

char* GetMem() {
	char* Ret = (char*) malloc(64);
	struct sysinfo Sysinfo;
	if (sysinfo(&Sysinfo) == 0) {
		unsigned long TotMem = Sysinfo.totalram / (1024 * 1024);
		unsigned long FreeMem = (Sysinfo.totalram - Sysinfo.freeram) / (1024 * 1024);
		snprintf(Ret, 64, "%ld MiB / %ld MiB", FreeMem, TotMem);
	} else {
		Ret = "Unknown";
	}
	return Ret;
}

/* artistic things */
char* MakeSeparator(int Size) {
	char* Ret = (char*) malloc(Size);
	for (int i = 0; i < Size; i++)
		Ret[i] = '-';
	Ret[Size] = '\0';
	return Ret;
}

void GetArt(char* Target) {
	int i = 0;
	char Buf[255], Fname[strlen(ArtDir) + strlen(Target) + 6];
	snprintf(Fname, sizeof(Fname), "%s/%s", ArtDir, Target);
	FILE* File = fopen(Fname, "r");
	if (File && strcmp(Target, "default") != 0) {
		while (fgets(Buf, sizeof(Buf), File)) {
			Art[i] = (char*) malloc(sizeof(Buf));
			strcpy(Art[i], Buf);
			RemoveSubstring(Art[i], "\n");
			i++;
		}
		fclose(File);
	} else {
		/* special case for linux */
		snprintf(Fname, sizeof(Fname), "%s/linux", ArtDir); 
		if ((strcmp(Target, "default") != 0) && (strcmp(OS, "Linux") == 0) && ((File = fopen(Fname, "r")) != NULL)) {
			while (fgets(Buf, sizeof(Buf), File)) {
				Art[i] = (char*) malloc(sizeof(Buf));
				strcpy(Art[i], Buf);
				RemoveSubstring(Art[i], "\n");
				i++;
			}
			fclose(File);
		} else {
			for (int i = 0; i < sizeof(Default) / sizeof(Default[0]); i++ ) {
				Art[i] = (char*) malloc(strlen(Default[i]));
				strcpy(Art[i], Default[i]);
			}
		}
	}
}

void ProcessArt(char* Target) { /* Target is to skip processing if Target is 'default' */
	if (strcmp(Target, "default") != 0) {
		for (int i = 1; i < sizeof(Art) / sizeof(Art[0]); i++) {
			if(!Art[i])
				break;
			ReplaceAllOccurences(Art[i], "COL", "\x1b[");
		}
	}
	Colour = (char*) malloc(strlen(Art[0]) + 12);
	sprintf(Colour, "\x1b[%sm", Art[0]);
}

void GetInfo() {
	OS = GetOS();
	Distro = GetDistro();
	Username = GetUserName();
	Hostname = GetHostName();
	Model = GetModel();
	Uptime = GetUptime();
	Shell = GetSh();
	CPU = GetCPU();
	Mem = GetMem();
}

void PrintInfo() {
	char* Choice;
	(ArtChoice)
		? (Choice = ArtChoice)
		: (Choice = Distro);
	
	GetArt(Choice);
	ProcessArt(Choice);

	sprintf(Info[0], "%s%s%s@%s%s", Colour, Username, RESET, Colour, Hostname);
	sprintf(Info[1], "%s%s", RESET, MakeSeparator(strlen(Username) + strlen(Hostname) + 1));
	sprintf(Info[2], "%sOS: %s%s", Colour, RESET, PrettyDistro);
	sprintf(Info[3], "%sModel: %s%s", Colour, RESET, Model);
	sprintf(Info[4], "%sKernel: %s%s", Colour, RESET, KernelRelease);
	sprintf(Info[5], "%sUptime: %s%s", Colour, RESET, Uptime);
	sprintf(Info[6], "%sShell: %s%s", Colour, RESET, Shell);
	sprintf(Info[7], "%sCPU: %s%s", Colour, RESET, CPU);
	sprintf(Info[8], "%sMEM: %s%s", Colour, RESET, Mem);
	
	int ArtSize = 1; /* doing 'sizeof(Art) / sizeof(Art[0])' if array rows are small and contents are long, that results in incorrect size */
	while (Art[ArtSize]) /* may be not best practice to do this to get rows of art array but it's accurate */
		ArtSize++;

	if (ArtSize < 9) {
		int Spacerssize = CleanStringLen(Art[1]);
		char Spacer[Spacerssize + 1];
		for (int i = 0; i < (Spacerssize + 1)- 1; i++)
			Spacer[i] = ' ';
		Spacer[Spacerssize] = '\0';

		for (int i = 1; i < ArtSize; i++) {
			printf("%s  %s%s%s\n", Art[i], RESET, Info[i - 1], RESET);
		}
		for (int i = ArtSize; i < 9; i++) {
			printf("%s  %s\n", Spacer, Info[i - 1]);
		}
	} else {
		for (int i = 1; i < ArtSize; i++) {
			if (!Info[i - 1]) {
				printf("%s\n", Art[i]);
			} else {
				printf("%s  %s%s%s\n", Art[i], RESET, Info[i - 1], RESET);
			}
		}
	}
}

/* program sequence */
int main(int argc, char* argv[]) {

	if (argc > 1) { /* no need for getopt if only one option at time is */
		if (argv[1][0] == '-') {
			switch (argv[1][1]) {
				case 'v':
					Die("v%s\n", PROGVER);
					break;
				case 'D': {
					DIR* Dir = opendir(ArtDir);
					if (!Dir)
						Die("Missing directory '%s'\n", ArtDir);
					
					struct dirent* Entry;
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
				}
			}
		}
	}
	GetInfo();
	PrintInfo();
	return 0;
}
