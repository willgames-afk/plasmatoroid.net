#ifndef _REGEX_H
#define _REGEX_H 1

typedef struct Transition {
	int start;
	char match; // match zero chars if 0
	int next;
} Transition;


void buildNFA(const char* regex) {
	Transition nfa[100];
	bool accepts[100];
	int t = 0;
	int state = 0;

	for (int i=0;regex[i];i++) {
		if (regex[i] == '|') {
			state--;
			continue;
		}
		if (regex[i] == '?') {
			nfa[t].match = '\0';
			nfa[t].start = state - 1;
			nfa[t].next = state;
		} else if (regex[i] == '+') {
			nfa[t].match = "\0";
			nfa[t].start = state;
			nfa[t].next = state - 1;
		} else if (regex[i] == '*') {
			nfa[t].match = '\0';
			nfa[t].start = state - 1;
			nfa[t++].next = state;
			nfa[t].match = "\0";
			nfa[t].start = state;
			nfa[t].next = state - 1;
		} else {
			nfa[t].match = regex[i];
			nfa[t].start = state;
			nfa[t].next = state+1;
			state++;
		}
		t++;
	}
}


// example regex: ab*c+d
// states:
// 0: a -> 1
// 1: b -> 2
// 1: _ -> 2
// 1: b -> 1
// 2: c -> 3
// 2: c -> 2
// 3: d -> 4
// 4(success)

// Test a regex against (the start of) a string
bool r_test(const char* str, const char* regex) {
	Worklist w = {0};
	int i = 0;
	switch (regex[i]) {
	case '?':
		//TODO: scan ahead to next token and add that as a forward worklist item
		//TODO: add the char after the ? as a forward worklist item
		wlAdd(&w, (Entry){str+1,false});
		wlAdd(&w, (Entry){ri_scanForward(str), false});
		break;
	case '*':
		wlAdd(&w, (Entry){str+1,true});
		wlAdd(&w, (Entry){str+1,false});
		break;
	case '|':
		wlAdd(&w, (Entry){str+1,false});
		break;
	}
}

// Scan a string, and return the location and length
// of the first match of the regex
r_scan() {

}


#endif