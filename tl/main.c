#include "fcgiapp.h"

int main() {
	int res;
	res = stat("/run/t.sock")

	res = FCGX_Init();
	if (res) return res;
	int fd = FCGX_OpenSocket("/run/t.sock",10); // 10 pending connections max
	if (fd < 0) return fd;
	if (dup2(fd,0) < 0) return -1; // Replace STDIN with FCGX input pipe

	FCGX_Stream *in, *out, *err;
	FCGX_ParamArray envp;
	int counter = 0;
	while (FCGX_Accept(&in, &out, &err, &envp) >= 0) {

		if (res) continue;
		FCGX_FPrintF(r.out, 
			"Content-type: text/html\r\n"
			"\r\n"
			"<!DOCTYPE html>\n"
			"<html lang=en>\n"
			"<head>\n"
			"</head>\n"
			"<body>\n"
			"<h1>Hi!</h1>\n"
			"<p>Counter: %d</p>\n"
			"<h2>File Content</h2>\n"
			"%s"
			"</body>\n"
			"</html>\n"
			, counter, 
		);
		FCGX_Finish_r(&r);
	}
	return 0;
}