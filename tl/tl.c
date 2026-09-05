#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#include "markdown.h"

typedef struct Buffer {
	char  *data;
	size_t len;
	size_t cap;
} Buffer;


//Default initial capacity of dynamic arrays
#define DA_INIT_CAP 16

#define da_append(da, item)                                                        \
	do {                                                                           \
		if ((da)->len >= (da)->cap) {                                              \
			(da)->cap = (da)->cap == 0 ? DA_INIT_CAP : (da)->cap * 2;              \
			(da)->data = realloc((da)->data, (da)->cap * sizeof(*(da)->data));     \
			assert((da)->data != NULL && "Out of memory :(");                      \
		}                                                                          \
                                                                                   \
		(da)->data[(da)->len++] = (item);                                          \
	} while (0)

#define da_append_many(da, new_items, new_items_count)                                    \
	do {                                                                                  \
		if ((da)->len + (new_items_count) > (da)->cap) {                                  \
			if ((da)->cap == 0) {                                                         \
				(da)->cap = DA_INIT_CAP;                                                  \
			}                                                                             \
			while ((da)->len + (new_items_count) > (da)->cap) {                           \
				(da)->cap *= 2;                                                           \
			}                                                                             \
			(da)->data = realloc((da)->data, (da)->cap*sizeof(*(da)->data));              \
			assert((da)->data != NULL && "Out of memory :(");                             \
		}                                                                                 \
		memcpy((da)->data + (da)->len, (new_items), (new_items_count)*sizeof(*(da)->data));   \
		(da)->len += new_items_count;                                                     \
	} while (0)

#define da_append_cstr(da, string) da_append_many(da, string, strlen(string))

#define nob_return_defer(value) do {result = (value); goto defer;} while(0)

bool read_entire_file(const char* path, Buffer *sv) {
	bool result = true;
	size_t buf_size = 1024;
	char* buf = realloc(NULL, buf_size);
	assert(buf != NULL && "Out of memory :(");

	FILE* f = fopen(path, "rb");
	if (f == NULL) {
		printf("Could not open file `%s` for reading: %s\n", path, strerror(errno)); 
		nob_return_defer(false);
	}

	size_t n = fread(buf, 1, buf_size, f);
	while(n > 0) {
		da_append_many(sv, buf, n);
		n = fread(buf, 1, buf_size, f);
	}
	if (ferror(f)) {
		printf("Could not read `%s`: %s\n", path, strerror(errno));
		nob_return_defer(false);
	}

defer: 
	free(buf);
	if (f) fclose(f);
	return result;
}

bool write_entire_file(const char* path, Buffer *buf) {
	bool result = true;
	FILE* f = fopen(path, "wb");
	if (f == NULL) {
		printf("Could not open file `%s` for writing: %s", path, strerror(errno));
		nob_return_defer(false);
	}
	fwrite(buf->data, 1, buf->len, f);
	if (ferror(f)) {
		printf("Could not write buffer to `%s`: %s", path, strerror(errno));
		nob_return_defer(false);
	}
	printf("Wrote file %s successfully!\n", path);
defer:
	if (f) fclose(f);
	return result;
}

//Create directory, if it doesn't already exist.
bool make_dir_exist(const char* path) {
#ifdef _WIN32
	if (mkdir(path) < 0) {
#else
	if (mkdir(path, 0755) < 0) {
#endif
		if (errno == EEXIST) {
			return true; //OK if dir already exists.
		}
		printf("Could not create directory `%s`: %s\n", path, strerror(errno));
		return false;
	}
	return true;
}

void copy(const char* source, const char* dest) {
	FILE* from = fopen(source, "r");
	if (from < 0) return;
	FILE* to = fopen(dest, "wb");
	if (to < 0) goto cleanup;

	char buf[4996];
	ssize_t nread = 0;
	ssize_t nwritten = 0;
	while ((nread = fread(buf, 1, sizeof(buf), from)) > 0) {
		char* ptr = &buf[0];
		do {
			nwritten = fwrite(ptr, 1, nread, to);
			ptr += nwritten;
			nread -= nwritten;
		} while (nwritten > 0);
	}	
	fclose(to);

cleanup:
	fclose(from);
}



void empty_dir(const char* path) {
	struct dirent *ent;
	DIR *dir = opendir(path);
	if (!dir) {
		printf("Error, couldn't access build dir");
		exit(1);
	}
	while ((ent = readdir(dir)) != NULL) {
		if (ent->d_name[0] == '.') continue;
		char truePath[100] = "";
		strcat(truePath, path);
		strcat(truePath, ent->d_name);
		if (remove(truePath)) printf("Error removing %s\n", truePath);
		// TODO: delete old files recursively
	}
	closedir(dir);
}


//int c = 0;

void processTl(const char* sourcepath, const char* destpath, Buffer* content, Buffer* addToHead) {
	printf("Processing a TL: %s\n",sourcepath);
	Buffer body = {0};
	Buffer f = {0};
	if (!read_entire_file(sourcepath, &f)) return;
	da_append(&f, '\0');
	printf("Read entire file\n");

	Buffer head = {0};

	size_t lastOut = 0;
	bool hasTempl = false;

	if (strncmp("use ", f.data, 4) == 0) {
		while(f.data[lastOut] != '\0' && f.data[lastOut] != '\n') {
			lastOut++;
		}
		lastOut++;
		hasTempl = true;
	}

	size_t i=0;
	for (i=0; f.data[i] != '\0'; i++) {
		if (strncmp("<tl ", &f.data[i],4)) continue;
		printf("Found <tl> tag: ");
		da_append_many(&body, &f.data[lastOut], i-lastOut);
		lastOut = i;
		i += 4;
		if (!strncmp("content>", &f.data[i], 8)) {
			printf("content\n");
			i += 8;
			if (content != NULL) {
				//printf("file: `%.*s`", content->len, content->data);
				da_append_many(&body, content->data,content->len);
			}
		} else if (!strncmp("head>", &f.data[i], 5)) {
			i += 5;
			printf("head\n");
			da_append_many(&body, head.data, head.len);
			if (addToHead != NULL) {
				da_append_many(&body, addToHead->data,addToHead->len);
			}
		} else if (!strncmp("add>", &f.data[i], 4)) {
			printf("add\n");
			i += 4;
			size_t count = 1;
			while ((f.data[i] != '\0') && (count > 0)) {
				if (f.data[i] == '<') {
					if (f.data[i+1] == '/') {
						count--;
					} else {
						count++;
					}
				}
				if (count != 0) da_append(&head, f.data[i]);
				i++;
			}
			printf("NEW HEAD: `%.*s`\n", head.len, head.data );
		} else {
			printf("Unsupported, ignoring.\n");
		}
		while ((f.data[i] != '\0') && (f.data[i] != '>')) {
			i++;
		}
		lastOut = i+1;
	}

	da_append_many(&body, &f.data[lastOut], i-lastOut);
	lastOut = i;

	printf("file data: `%.64s`\n", f.data);

	if (hasTempl) {
		printf("Has Use\n");
		char path[200] = "./source/templates/";
		size_t i = strlen("./source/templates/");
		for (size_t j=4;j<100;j++) {
			if (f.data[j] == 0) break;
			if (f.data[j] == '\n') break;
			if (f.data[j] == '\r') break;
			path[i++] = f.data[j];
		}
		path[i] = '\0';
		strcat(path, ".tl");

		/*
		printf("Path: \"%s\"\n", path);
		c++;
		if (c > 1) {
			exit(1);
		}
*/
		processTl(path, destpath, &body, &head);
	} else {
		write_entire_file(destpath, &body);
	}
}

int main() {
	make_dir_exist("./build");
	empty_dir("./build/");

	struct dirent *ent;
	DIR *dir = opendir("./source/public");
	if (!dir) {
		printf("Error, couldn't access public dir");
		exit(1);
	}
	while ((ent = readdir(dir)) != NULL) {
		if (ent->d_name[0] == '.') continue;
		printf("%s: ",ent->d_name);
		
		char c;
		size_t i = 0;
		size_t extStart = 0;
		while (c = ent->d_name[i++]) {
			if (c == '.') extStart = i; 
		}

		char fullSource[100] = "./source/public/";
		char fullDest[100] = "./build/";
		strcat(fullSource, ent->d_name);

		if (!strcmp("md", &ent->d_name[extStart])) {
			printf("Markdown!\n");
			strncat(fullDest, ent->d_name, extStart);
			strcat(fullDest, "html");
		} else if (!strcmp("tl", &ent->d_name[extStart])) {
			printf("TL!\n");
			strncat(fullDest, ent->d_name, extStart);
			strcat(fullDest, "html");
			processTl(fullSource, fullDest, NULL, NULL);
		} else {
			strcat(fullDest, ent->d_name);
			copy(fullSource, fullDest);
			printf("Other!\n");
		}
	}
	closedir(dir);
}