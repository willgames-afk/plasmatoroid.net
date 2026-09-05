
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include "markdown.h"

View readAll(char* file) {
	FILE* f;
	f = fopen(file,"r");
	if (f == NULL) return (View){0};
	struct stat st;
	if (stat(file, &st) != 0) return (View){0};
	char* buf = malloc(st.st_size + 2);
	if (buf == NULL) return (View){0};
	size_t count = fread(buf, 1, st.st_size, f);
	if (count != st.st_size) return (View){0};
	fclose(f);
	buf[st.st_size] = '\n';
	buf[st.st_size + 1] = '\0';
	return (View){buf, st.st_size+1};
}

int main() {
	View input = readAll("../source/public/tlc.md");
	if (input.start == NULL) return 1;
	FILE* out = fopen("test.html", "w");
	if (out == NULL) return 1;
	Buf mt = (Buf){0};
	View icpy = input;
	render(&mt, &icpy, out);
	fflush(out);
	free(input.start);
	fclose(out);
	return 0;
}