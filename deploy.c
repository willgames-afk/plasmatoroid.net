#include <stdlib.h>
#include <stdio.h>

/* 
TODO: 
- install nginx if not already installed
- install git if not already installed
- pull from github
- run tl
*/

char* deps[] = {
	"nginx",
	"git",
	"openjdk-25-jre-headless"
}

char* cmd[1024];

int main() {
	bool missingDeps = false;
	for (int i=0;i<(sizeof(deps)/sizeof(deps[0]));i++) {
		sprintf(&cmd, "dpkg -s %s &>/dev/null", deps[i]);
		if (system(cmd) == 0) continue;
		if (!missingDeps) {
			missingDeps = true;
			printf("Missing Dependencies:\n");
		}
		printf("\t%s\n",deps[i]);
	}
	if (missingDeps) {
		printf("Please install (ie apt install <dep>)\n");
		return -1;
	}

}