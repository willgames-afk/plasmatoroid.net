#ifndef _MARKDOWN_H
#define _MARKDOWN_H 1

#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

/*
Rules
- At start of line:
	- Look at whitespace and compare to before (remember for later)
	- If we were in a list but now whitespace is less, close list
	- If we were in a list but now whitespace is more, open list
	- If we were in a list and whitespace is the same, and not matching list identifier, end list
	- If there's a list specifier and we're not in a list, start a list
	- If there's a list specifier and we are in a list, add list element

	- If #, ##, ###, ####, #####, or ######, start heading
	- If it's - + * or a number followed by `.` 
		- If we're not in a list, start it and start the li or ol
		- If we are already in a list, end previous element and start next one
	- If its >, start a blockquote
	- If its whitespace
		- If we're in a list, keep the list going

- At end of line:
	- End any headers, if there was one


*/

typedef struct {
	char* dat;
	size_t len;
	size_t cap;
} Buf;


void grow(Buf* b) {
	if (b->len < b->cap) return;
	while (b->len >= b->cap) {
		b->cap = b->cap*2 > 0 ? b->cap*2 : 64;
	}
	b->dat = realloc(b->dat, b->cap);
	assert(b->dat  && "Out of RAM!");
	//printf("Realloc'd good!\n");
}

void bputc(char d, Buf* b) {
	grow(b);
	b->dat[b->len++] = d;
}
void bputn(char* d, size_t c, Buf* b) {
	b->len += c;
	grow(b);
	memcpy(b->dat + (b->len - c), d, c);
}
void bputs(char* d, Buf* b) {
	bputn(d, strlen(d),b);
}

unsigned long hash(unsigned char* str, size_t len) {
	if (len == 0) {
		printf("Trying to hash str `%s`\n", str);
	} else {
		printf("Trying to hash str `%.*s`\n", len, str);
	}
	unsigned long hash = 5381;
	int c;
	if (len == 0) {
		while (c = *str++) {
			hash = ((hash << 5) + hash) + c; //fast hash*33 + c
		}
	} else {
		for (; len>0; len--) {
			hash = ((hash << 5) + hash) + (c = *str++); //fast hash*33 + c
			if (c == '\0') break;
		}
	}
	return hash;
}

typedef struct {
	char* start;
	size_t len;
} View;

#define chop(a, n) a->start += n; a->len -= n;


typedef struct {
	char* name;
	char* value;
} NTEntry;

typedef struct {
	Buf stringData;
	NTEntry* dat;
	size_t len;
	size_t cap;
} NameTable;

NTEntry* iaddnt(NTEntry* table, size_t cap, char* name, char* val) {
	unsigned long h = hash(name,0);
	h = h & (cap-1);
	while (table[h].name != NULL) {
		h = (h + 1) & (cap-1);
	}
	table[h] = (NTEntry){name, val};
	return &table[h];
}

void grownt(NameTable* nt) {
	size_t oldcap = nt->cap;
	if (nt->cap > 0) {
		if (nt->len < (nt->cap * 3 / 4)) return;
	}
	while(nt->len >= nt->cap) {
		nt->cap = nt->cap * 2 > 0 ? nt->cap * 2 : 64;
	}
	NTEntry* newDat = calloc(nt->cap, sizeof(NTEntry));
	assert(newDat != NULL && "Out of RAM!"); 

	for (size_t i=0;i<oldcap;i++) {
		if (nt->dat[i].name == NULL) continue;
		iaddnt(newDat, nt->cap, nt->dat[i].name, nt->dat[i].value);
	}

	free(nt->dat);
	nt->dat = newDat;
}

void lowercase(char* str) {
	for (int i=0; str[i]; i++) {
		str[i] = tolower(str[i]);
	}
}

// This function does copy the string `name`
// This function will auto-grow the table if needed
NTEntry* addnt(NameTable* nt, View name, char* val) {
	size_t oldlen = nt->stringData.len;
	bputn(name.start, name.len, &nt->stringData);
	bputc('\0',&nt->stringData);
	char* nameP = &nt->stringData.dat[oldlen];
	lowercase(nameP);
	grownt(nt);
	return iaddnt(nt->dat, nt->cap, nameP, val);
}

char* getnt(NameTable* nt, char* name, size_t len) {
	size_t i =0;
	char* copy = malloc(strlen(name)+1);
	if (copy == NULL) assert(0 && "Out of RAM :(");
	strcpy(copy, name);
	lowercase(copy);
	unsigned int sh = hash(copy,len) & (nt->cap - 1);
	if (nt->dat[sh+i].name == NULL) {
		free(copy);
		return NULL;
	}
	while (strncmp(copy, nt->dat[sh + i].name, len)) {
		i++;
		if (nt->dat[sh+i].name == NULL) {
			free(copy);
			return NULL;
		}
	}
	free(copy);
	return nt->dat[sh + i].value;
}

// Use when trying to detect blankness.
bool isBlank(unsigned char* c) {
	switch(c[0]) {
	case '\n':
	case '\r':
	case '\f': 
	case '\t':
	case '\v':
	case ' ': //ASCII spaces
		return true;
	case 0xE2:
		if (c[1] == '\x80') {
			if (c[2] >= '\x80' && c[2] <= '\x8B') return true; // EN/EM spaces and zero width space
			if (c[2] == '\xA8') return true; // Line sep
			if (c[2] == '\xA9') return true; // Paragraph sep
		} else if (c[1] == '\x81') {
			if (c[2] == '\x9F') return true; // Math space
		} else if (c[1] == '\xA0' && c[2] == '\x80') return 1; // Braille Blank
		return false;
	case 0xE3:
		if (c[1] == '\x80' && c[2] == '\x80') return true; // CJK Ideograph space
		return false;
	default: return false;
	}
}

// Use when trying to detect word boundaries.
// Returns true on breaking spaces and commas
bool isBreaking(unsigned char* c) {
	switch(c[0]) {
	case '\n':
	case '\r':
	case '\f': 
	case '\t':
	case '\v':
	case ',':
	case ' ': //ASCII spaces
		return true;
	case 0xE2:
		if (c[1] == '\x80') {
			if (c[2] >= '\x80' && c[2] <= '\x8B') return true; // EN/EM spaces and zero width space
			if (c[2] == '\xA8') return true; // Line sep
			if (c[2] == '\xA9') return true; // Paragraph sep
		} else if (c[1] == '\x81') {
			if (c[2] == '\x9F') return true; // Math space
		} else if (c[1] == '\xA0' && c[2] == '\x80') return 1; // Braille Blank
		return false;
	case 0xE3:
		if (c[1] == '\x80' && c[2] == '\x80') return true; // CJK Ideograph space
		return false;
	default: return false;
	}
}

void renderPlaintext(Buf prefix, View* input, FILE* out) {
	while (input->len > 0) {
		switch(*input->start) {
		case '<': fputs("&lt;", out); break;
		case '>': fputs("&gt;", out); break;
		case '&': fputs("&amp;", out); break;
		case '\n':
			input->start++;
			input->len--;
			while(input->start[0] == '\n') {
				input->start++;
				input->len--;
				fputs("<br>",out);
				if (input->len == 0) break;
			}
			// Match prefix
			size_t skipped = 0;
			for (size_t j=0;j<prefix.len;j++) {
				char n = input->start[j];
				skipped++;
				if (n == '\t') {
					if (prefix.dat[j++] != ' ') return;
					if (prefix.dat[j++] != ' ') return;
					if (prefix.dat[j++] != ' ') return;
					if (prefix.dat[j++] != ' ') return;
					continue;
				}
				if (n != prefix.dat[j]) return;
			}
			input->start += skipped;
			input->len -= skipped;
			fputs("<br>",out);
			continue;
		default: fputc(*input->start,out); break;
		}
		input->start++;
		input->len--;
	}
}

// Scans for reference declearations and adds them to the nametable.
// This only runs at most once and only triggers if a reference is used
// that has not previously been found
void scanRefs(View v, NameTable* namename) {
	// Look for newline followed by [

	View name = {0};
	int i=0;
	while(i<v.len) {
		notaname:
		for (i;i<v.len;i++) {
			if (v.start[i] == '[') {
				if ((i == 0) || v.start[i-1] == '\n') break;
			}
		}
		i++;
		name.start = & v.start[i];
		for (i; i<v.len;i++) {
			if (v.start[i] != ']') {
				name.len++;
				continue;
			}
			i++;
			if ((i == v.len) || v.start[i] != ':') {
				goto notaname;
			}
			break;
		}

		bool exists = getnt(namename, name.start, name.len) != NULL;
		if (exists) continue;

		i++;
		while(v.start[i] == ' ') {
			i++;
		}

		View link = {0};
		link.start = &v.start[i];

		while (link.start[link.len] != ' ') {
			if (link.start[link.len] == '\n') break;
			i++;
			link.len++;
		}

		char* url = malloc(link.len + 1);
		memcpy(url, link.start, link.len);
		url[link.len] = '\0';

		addnt(namename, name, url);
	}
}

#define MU_SUB 16
#define MU_SUP 8
#define MU_S 4
#define MU_EM 2
#define MU_STRONG 1

char* tags[] = {
	"strong",
	"em",
	"s",
	"sup",
	"sub"
};

// Order: sub contains sup which contains s, etc

void transitionMarkup(int markupOld, int markupNew, Buf* out) {
	int bit, needClose, needOpen;

	// step 1: close tags
	needClose = markupOld & ~markupNew;
	for (int i=0;i<5;i++) {
		bit = (1 << i);
		if ((markupOld & bit) && (needClose >= bit)) {
			bputs("</",out);
			bputs(tags[i], out);
			bputs(">",out);
			markupOld &= ~bit; //Record which tags were closed!
		}
	}
	// step 2: open tags
	needOpen = ~markupOld & markupNew;
	for (int i=4;i>=0;i--) {
		bit = (1<<i);
		if (!(needOpen & bit)) continue;
		bputs("<", out);
		bputs(tags[i],out);
		bputs(">",out);
	}
}

bool calcTransitions(View* v, Buf* out, char ts, bool wasSpace, int single_mode, int double_mode, int* newSingle, int* newDouble) {
	// step 1: Count *s (or tildes)
	unsigned int stars = 1;
	while(v->start[1] == ts) {
		stars++;
		v->start++;
		v->len--;
		if (v->len <= 0) return true;
	}
	bool mustEnd = isBreaking(v->start +1);
	bool mustStart = wasSpace;

	// step 2: output *s that don't do anything
	while(stars > 3) {
		bputc(ts, out);
		stars--;
	}
	if (mustStart && mustEnd) {
		for (int i=0;i<stars;i++) {
			bputc(ts, out);
		}
		return true;
	}

	// step 3: calculate changes in em/strong
	int newSingleMode = single_mode;
	int newDoubleMode = double_mode;
	if (mustStart) {
		if (stars & 1) newSingleMode++;
		if (stars & 2) newDoubleMode++;
	} else {
		if (stars & 1) {
			if (single_mode > 0) {
				newSingleMode--;
			} else if (!mustEnd) {
				newSingleMode++;
			} else {
				bputc(ts, out);
			}
		}
		if (stars & 2) {
			if (double_mode > 0) {
				newDoubleMode--;
			} else if (!mustEnd) {
				newDoubleMode++;
			} else if (single_mode >= 2) {
				newSingleMode -= 2;
			} else {
				bputc(ts,out);
				bputc(ts,out);
			}
		}
	}
	*newSingle = newSingleMode;
	*newDouble = newDoubleMode;
	return false;
}

// Returns true on EOF
// This converts inline markdown elements (links, bold/italic, etc) into HTML.
bool renderInline(View* v, Buf* out, bool insideLink, bool insideHeading, View* trueEnd, NameTable* namename, unsigned int* footnoteCount) {
	int codemode = 0; // How many backticks to close a code block?
	int em_mode = 0; // how many nested ems?
	int strong_mode = 0; // how many nested strongs?
	int strike_mode = 0; // how many nested strikethrus?
	int sub_mode = 0; // how many nested subscripts?
	int sup_mode = 0; // how many nested superscripts?

	bool wasSpace = true;
	bool isSpace = true;

	bool embed = false;

	for (;;) {
		wasSpace = isSpace;
		isSpace = isBreaking(v->start);
		char n = v->start[0];
		if (n == '\n' && codemode < 3) break;
		switch(n) {
		case '#': // ignore hashtags at the end of a heading
			if (!insideHeading) {
				bputc(n, out);
				break;
			}
			int i = 0;
			while (n == '#') {
				if (i >= v->len) break; 
				n = v->start[++i];
			}
			if (i >= v->len) break;
			int ic = i-1;
			while (n == ' ') {
				if (i >= v->len) break; 
				n = v->start[++i];
			}
			if (i >= v->len) break;
			if (n == '\n') {
				i--;
				*trueEnd = *v;
				trueEnd->start += i;
				trueEnd->len -= i;
				break;
			}
			for (int i=0;i<ic;i++) {
				bputc('#', out);
				v->start++;
				v->len--;
			}
			break;
		case ']':
			if (codemode) {
				bputc(']', out);
				break;
			}
			if (insideLink) {
				v->start++;
				v->len--;
				return false;
			}
			bputc(']', out);
			break;
		case '\\':
			v->start++;
			v->len--;
			if (v->len <= 0) return true;
			n = v->start[0];
			bputc(n, out);
			break;
		case '<': bputs("&lt;", out); break;
		case '>': bputs("&gt;", out); break;
		case '&': bputs("&amp;", out); break;
		//TODO: convert nbsp to entity
		case '\n': bputs("<br>", out); break;
		case '*': 
			if (codemode) {
				bputc('*', out);
				break;
			}
			int newEmMode;
			int newStrongMode;
			if (calcTransitions(v, out, '*', wasSpace, em_mode, strong_mode, &newEmMode, &newStrongMode)) break;

			int old 
				= (strong_mode > 0 ? MU_STRONG : 0) 
				| (    em_mode > 0 ? MU_EM : 0)
				| (strike_mode > 0 ? MU_S : 0) 
				| (   sup_mode > 0 ? MU_SUP : 0)
				| (   sub_mode > 0 ? MU_SUB: 0);

			int new = (old & ~(MU_EM | MU_STRONG)) 
				| (newStrongMode > 0 ? MU_STRONG : 0) 
				| (    newEmMode > 0 ? MU_EM : 0);

			// step 4: use transition table to output HTML
			transitionMarkup(old, new, out);
	
			em_mode = newEmMode;
			strong_mode = newStrongMode;
			break;
		case '~':
			if (codemode) {
				bputc('~', out);
				break;
			}

			int newSubMode;
			int newStrikeMode;
			if (calcTransitions(v,out,'~', wasSpace, sub_mode, strike_mode, &newSubMode, &newStrikeMode)) break;
			old 
				= (strong_mode > 0 ? MU_STRONG : 0) 
				| (    em_mode > 0 ? MU_EM : 0)
				| (strike_mode > 0 ? MU_S : 0) 
				| (   sup_mode > 0 ? MU_SUP : 0)
				| (   sub_mode > 0 ? MU_SUB: 0);

			new = (old & ~(MU_SUB | MU_S)) 
				|    (newSubMode > 0 ? MU_SUB : 0) 
				| (newStrikeMode > 0 ? MU_S : 0);

			transitionMarkup(old, new, out);

			strike_mode = newStrikeMode;
			sub_mode = newSubMode;
			break;
		case '^':
			if (codemode || insideLink) {
				bputc('^',out);
				break;
			}

			break;
		case '!':
			if ((v->len < 2) || (v->start[1] != '[')) {
				bputc(n, out);
				break;
			}
			v->start++;
			v->len--;
			embed = true;
			//FALLTHRU
		case '[':
			if ((insideLink && !embed) || codemode) {
				bputc(n, out);
				embed = false;
				break;
			}
			v->start++;
			v->len--;
			Buf rendered = {0};
			renderInline(v, &rendered, true, insideHeading, trueEnd, namename, footnoteCount);

			//printf("v: %.*s", v->len, v->start);
			char mode = v->start[0];

			char close = ' ';
			if (mode == '(') close = ')';
			else if (mode == '[') close = ']';
			else if (rendered.len > 0 && rendered.dat[0] == '^') {
				//Footnote!
				bputs("<sup id=\"fnref:",out);
				bputn(rendered.dat+1, rendered.len-1, out);
				bputs("\"><a href=\"#fn:",out);
				bputn(rendered.dat+1, rendered.len-1, out);
				bputs("\" role=\"doc-noteref\">",out);
				char* num = malloc(32);
				sprintf(num, "%d", *footnoteCount);
				bputs(num, out);
				bputs("</a></sup>",out);
				embed = false;
				(*footnoteCount)++;
				break;
			} else {
				bputc('[',out);
				bputn(rendered.dat, rendered.len, out);
				bputc(']',out);
				embed = false;
				break;
			}
			v->start++;
			v->len--;
			Buf str = {0};
			while(v->start[0] != close) {
				bputc(v->start[0], &str);
				v->start++;
				v->len--;
			}
			bputc('\0',&str);

			char* url;
			if (mode == '[') {
				printf("Looking up...\n");
				if (str.len <= 1) {
					url = getnt(namename, rendered.dat, rendered.len);
				} else {
					url = getnt(namename, str.dat, str.len);
				}
				if (url == NULL) {
					url = "null :(";
				}
			} else {
				url = str.dat;
			}
			
			if (embed) {
				bputs("<img src=\"", out);
				bputs(url, out);
				bputs("\" alt=\"",out);
				bputn(rendered.dat, rendered.len, out);
				bputs("\">",out);
			} else {
				bputs("<a href=\"", out);
				bputs(url, out);
				bputs("\">",out);
				bputn(rendered.dat,rendered.len, out);
				bputs("</a>",out);
			}
			if (rendered.dat) {
				free(rendered.dat);
			}

			embed = false;
			break;
		case '`': 
			if (codemode == 0) {
				codemode = 1;
				while (v->start[1] == '`') {
					codemode++;
					v->start++;
					v->len--;
					if (v->len <= 0) return true;
				}
				//printf("Now searching for %d backticks\n", codemode);
				bputs("<code>",out);
				break;
			} else {
				int count = 1;
				while (v->start[count] == '`') {
					count++;
					if (count >= v->len) return true;
				}
				//printf("Got %d backticks\n", count);
				if (count == codemode) {
					v->start += count-1;
					v->len -= count-1;
					bputs("</code>",out);
					codemode = 0;
					break;
				}
			}
			// FALLTHROUGH to print backticks inside code blocks!
		default:
			bputc(n, out);
		}
		v->start++;
		v->len--;
		if (v->len <= 0) return true;
	}
	return false;
}


typedef enum {
	NORMAL,
	BLOCKQUOTE,
	HEADING,
	THEMBREAK,
	BACKSLASH,
	FENCEBLOCK,
	CODEBLOCK, //Indented code block
	REFDEF,
	FOOTNOTE,
	EMBED,
	B_LIST,
	O_LIST='o',
	U_LIST='u'
} Element;

// Looks at the first char of v and figures out what markdown element it should be.
// It will consume input from V to leave V pointing at the first character of the content of that element.
// If the markdown element is a heading, it will give you a number indicating what level of heading it is.
// If the markdown element is a list, it will give you a character that holds the shape of the bullet (. for ordered lists)
Element classify(View* v, int* hlevelp, int* checked, View* ref) {
	char c = v->start[0];
	switch(c) {
	case '>': 
	case '\\': 
		v->start++;
		v->len--;
		break;
	}
	switch(c){
	case '>': return BLOCKQUOTE;
	/*case '`': 
		if (v->len < 3) return NORMAL;
		if (v->start[1] != '`') return NORMAL;
		if (v->start[2] != '`') return NORMAL;
		v->start+=3;
		v->len -=3;
		return FENCEBLOCK;
	*/
	case '-':
	case '*':
	case '_':
		int i=1;
		while (v->start[i] == ' ') { //skip optional spaces
			i++;
			if (i > v->len) break;
		}
		if (v->start[i] == c) { // find second *
			i++;
			while (v->start[i] == ' ') { //skip more optional spaces
				i++;
				if (i > v->len) break;
			}
			if (v->start[i] == c) { // find third *
				bool isHrule = true;
				while (v->start[++i] != '\n') {
					if (i > v->len) break;
					if (v->start[i] == c) continue;
					if (v->start[i] == ' ') continue;
					if (v->start[i] == '\r') continue;
					isHrule = false; 
					break;
				}
				if (isHrule) {
					v->start += i;
					v->len -= i;
					return THEMBREAK;
				}
			}
		}
		
		if (c == '_') { 
			return NORMAL;
		}

		//FALLTHRU in case of lists
	case '+':
		if (v->len < 2) return NORMAL;
		if (v->start[1] != ' ') return NORMAL;
		v->start +=2;
		v->len -= 2;
		while(v->start[0] == ' ' && v->len > 0) {
			v->start++;
			v->len--;
		}
		if (v->start[0] == '[') {
			if (v->start[1] == ']') {
				v->start+=2;
				v->len -=2;
				*checked = 1;
				return B_LIST;
			} else if (v->start[2] == ']') {
				if (v->start[1] == ' ') {
					v->start+=3;
					v->len-=3;
					*checked = 1;
					return B_LIST;
				} else {
					v->start+=3;
					v->len-=3;
					*checked = 2;
					return B_LIST;
				}
			}
		}
		return U_LIST;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		int digcount;
		for (digcount=1;digcount<v->len;digcount++) {
			c = v->start[digcount];
			if (c < '0' || c >= '9') break;
		}
		if (v->len - digcount < 2) return NORMAL;
		if (v->start[digcount] != '.') return NORMAL;
		if (v->start[digcount+1] != ' ') return NORMAL;
		v->start += digcount+2;
		v->len -= digcount+2;
		return O_LIST;
	case '#': 
		size_t hlevel; //Count #s
		for (hlevel = 1; hlevel < v->len; hlevel++) {
			if (v->start[hlevel] != '#') break;
		}

		bool empty = true;
		for (int i=hlevel;i<v->len;i++) {
			if (v->start[i] == '\n') break;
			if (v->start[i] == '\t') continue;
			if (v->start[i] == ' ') continue;
			empty = false;
			break;
		}
		if (empty) return NORMAL;
		if (v->start[hlevel] != ' ') return NORMAL;

		v->start += hlevel+1;
		v->len -= hlevel+1;
		if (hlevel > 6) hlevel = 6;
		*hlevelp = hlevel;
		return HEADING;
	case '[':
		View oldV = *v;
		v->start++;
		v->len--;
		ref->start = v->start;
		while (v->start[0] != ']') {
			v->start++;
			v->len--;
			ref->len++;
		}
		if (v->start[1] != ':') {
			*v = oldV;
			return NORMAL;
		}
		v->start += 2;
		v->len-=2;
		return REFDEF;
	default: return NORMAL;
	}
}


char* formatHeadingID(View rawHeading) {
	char* copy = malloc(rawHeading.len + 1);
	char* cp = copy;
	int i=0;
	bool wasDash = false;
	while (i < rawHeading.len) {
		if (rawHeading.start[i] != '-') break;
		wasDash = true;
		i++;
	}
	for (i;i<rawHeading.len;i++) {
		char c = tolower(rawHeading.start[i]);
		bool safe = 
			(c >= 'a' && c <= 'z')
			||(c >= '0' && c <= '9')
			|| c == '-';
		if (!safe) {
			c = '-';
		}
		if ((c=='-') && wasDash) continue;
		*(cp++) = c;
		wasDash = c == '-';
	}
	if (cp > copy) {
		if (*(cp-1) == '-') cp--;
	}
	*cp = '\0';
	return copy;
}

int getIndent(Buf* prefix, View* v, char* xtraspace) {
	char n;
	// Start of line: First match prefix
	for (int i=0;i<prefix->len;i++) {
		n = v->start[0];
		if (n == '\t') {
			if (prefix->dat[i++] != ' ') return -1;
			if (prefix->dat[i++] != ' ') return -1;
			if (prefix->dat[i++] != ' ') return -1;
			if (prefix->dat[i++] != ' ') return -1;
		}
		if (n != prefix->dat[i]) return -1;
		v->start++;
		v->len--;
		if (v->len <= 0) return -1;
	}

	// Next, find indent level
	int spaces = 0;
	for(;;) {
		n = v->start[0];
		if (n == ' ') {
			spaces += 1;
		} else if (n == '\t') {
			spaces += 4;
		} else {
			break;
		}
		v->start++;
		v->len--;
		if (v->len <= 0) return -1;
	}
	*xtraspace = spaces % 4;
	return spaces / 4;
}

/*
Converts Markdown to HTML.

If the input source is weird or malformed, it will still try to render it.
*/
void render(Buf* prefix, View* input, FILE* out) {
	NameTable namename = {0};
	grownt(&namename);
	View v = *input;
	Buf toClose = {0};
	unsigned int footnoteCount = 1;
	
	int prevIndent = 0;
	bool inPara = false;
	bool list_addPara = false;
	char n = v.start[0];
	for (;;) {
		View startOfLine = v;
		char spaces;
		int indent = getIndent(prefix, &v, &spaces);
		if (indent == -1) goto cleanup;

		if (n == '\n') {
			// Handle blank lines
			if (inPara) {
				fprintf(out, "</p>\n");
				inPara = false;
			}
			//fprintf(out, "\n");
			v.start++;
			v.len--;
			if (v.len <= 0) goto cleanup;
			n = v.start[0];
			continue;
		}

		int hlevel = 0;
		int checked = 0;
		View ref = {0};
		
		// If indent level increases, and it's not because of a list, force a code block
		Element e = (indent - prevIndent) > 0 ? CODEBLOCK : classify(&v, &hlevel, &checked, &ref);
		//if (indent > 0 && e == HEADING) e = NORMAL; // Headings may not be indented


		if (e == REFDEF) {
			bool exists = getnt(&namename, ref.start, ref.len) != NULL;

			while(v.start[0] == ' ') {
				v.start++;
				v.len--;
			}

			View link = v;
			link.len = 0;

			while (link.start[link.len] != ' ') {
				if (link.start[link.len] == '\n') break;
				v.start++;
				v.len--;
				link.len++;
			}

			if (exists) break;

			char* url = malloc(link.len + 1);
			memcpy(url, link.start, link.len);
			url[link.len] = '\0';

			addnt(&namename, ref, url);
			
			goto done;
		}

		if (indent < prevIndent) {
			// If list indent went down, close li/ol/ul tags as necessary
			// We may need to close one more ol/ul tag, but that depends
			// on whether or not there's another list element to add to it
			fprintf(out, "</li>");
			for (int i=1; i<(prevIndent - indent); i++) {
				char g = toClose.dat[--toClose.len];
				if (g == B_LIST) {
					g = 'u';
				}
				fprintf(out, "</%cl></li>", g);
			}
			if (e != O_LIST && e != U_LIST && e != B_LIST) {
				char g = toClose.dat[--toClose.len];
				if (g == B_LIST) {
					g = 'u';
				}
				fprintf(out, "</%cl>", g);
			}
		} else if (e != NORMAL && inPara) { 
			// Non-paragraphs interrupt paragraphs
			fprintf(out, "</p>");
			inPara = false;
		}
		if (e != NORMAL) {
			list_addPara = true;
		}
		size_t oldPrefixLen;
		switch(e) {
		case THEMBREAK:
			fprintf(out, "<hr>\n");
			break;
		case CODEBLOCK:
			printf("\nCode Blockkin!\n");
			oldPrefixLen = prefix->len;
			for (int i=0;i<(indent-prevIndent + 1);i++) {
				bputn("    ", 4,prefix);
			}
			fprintf(out, "<pre><code>");
			renderPlaintext(*prefix, &v, out);
			fprintf(out, "</code></pre>");
			prefix->len = oldPrefixLen;
			continue; // JUMP TO TOP!!!
		case BLOCKQUOTE:
			oldPrefixLen = prefix->len;
			for (int i=0;i<indent;i++) {
				bputn("    ", 4,prefix);
			}
			for (int i=0;i<spaces;i++) {
				bputc(' ',prefix);
			}
			bputc('>',prefix);
			fprintf(out, "<blockquote>");
			v = startOfLine;
			render(prefix, &v, out);
			fprintf(out, "</blockquote>");
			prefix->len = oldPrefixLen;
			//printf("Encountered blockquote, stopping!\n");
			//exit(1);
			continue; // JUMP TO TOP!!!
		case O_LIST:
		case U_LIST:
		case B_LIST:
			indent++;
			if (indent > prevIndent) {
				// If this is the first list entry at this indentation, output ul or ol depending on element
				if (e == B_LIST) {
					fprintf(out, "<ul class=\"bullet\"><li>");
				} else {
					fprintf(out, "<%cl><li>", e); 
				}
				bputc(e, &toClose);
			} else if (toClose.dat[toClose.len-1] != e) {
				// If this is not the first list entry, but it's a different kind of list, switch types
				
				char g = toClose.dat[toClose.len-1];
				if (g == B_LIST) {
					g = 'u';
				}
				if (e == B_LIST) {
					fprintf(out, "</li></%cl>\n<ul class=\"bullet\"><li>", g);
				} else {
					fprintf(out, "</li></%cl>\n<%cl><li>", g, e);
				}
				toClose.dat[toClose.len-1] = e;
			} else {
				// If this is not the first list entry and it's the same kind of list, 
				// then the previous list entry was already closed and we just need to 
				// open a new one
				fprintf(out, "\n<li>");
			}
			list_addPara = false; //Reset to not wrapping things in this list in paras.
			if (!checked) break;
			checked--;
			fprintf(out, "<input type=\"checkbox\" disabled");
			if (checked) {
				fprintf(out, " checked");
			}
			fprintf(out,">");
			break;
		case NORMAL:
			if (!inPara && (toClose.len == 0 || list_addPara)) {
				fprintf(out, "<p>");
				inPara = true;
			} else {
				// If we're appending multiple lines into a paragraph, make sure there's spaces where there needs to be
				fprintf(out, " ");
			}
			break;
		}

		View rawHeading = v;
		View trueEnd = {0};

		Buf lbuf = {0};
		if (renderInline(&v,&lbuf,false,hlevel > 0, &trueEnd, &namename, &footnoteCount)) goto cleanup;
		bputc('\0', &lbuf);

		if (hlevel > 0) {
			if (trueEnd.start != NULL) {
				rawHeading.len = trueEnd.start - rawHeading.start;
			} else {
				rawHeading.len = v.start - rawHeading.start;
			}
			char* headingID = formatHeadingID(rawHeading);
			fprintf(out, "<h%d id=\"%s\">", hlevel, headingID);
			free(headingID);
		}


		//printf("\n\nno: %.*s", lbuf.len, lbuf.dat);
		fputs(lbuf.dat,out);
		if (lbuf.dat) {
			free(lbuf.dat);
		}

		if (hlevel > 0) {
			fprintf(out, "</h%d>\n", hlevel);
		}
	
		done:
		if (n == 0) break;
		v.start++;
		v.len--;
		if (v.len <= 0) goto cleanup;
		n = v.start[0];

		prevIndent = indent;
	}
	cleanup:
	if (inPara) {
		fprintf(out, "</p>\n");
	}
	free(namename.dat);
	*input = v;
}


/*
embed youtube
<iframe width="560" height="315" src="https://www.youtube.com/embed/mDhNQPt8An0?si=1r2pxG4TeK8tTmvu" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" referrerpolicy="strict-origin-when-cross-origin" allowfullscreen></iframe>
<iframe width="560" height="315" src="https://www.youtube.com/embed/mDhNQPt8An0?si=hkIV_3Tu9b-TEQyb" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" referrerpolicy="strict-origin-when-cross-origin" allowfullscreen></iframe>
<iframe width="560" height="315" src="https://www.youtube.com/embed/mDhNQPt8An0?si=TepADp4uBFktjWj9&amp;controls=0" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" referrerpolicy="strict-origin-when-cross-origin" allowfullscreen></iframe>

*/

#endif /* _MARKDOWN_H */