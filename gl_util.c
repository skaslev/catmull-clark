#include <stdio.h>
#include <stdarg.h>
#include <GL/glut.h>

void gl_printf(void *font, const char *format, ...)
{
	char str[256], *p;
	va_list args;

	va_start(args, format);
	vsnprintf(str, sizeof(str), format, args);
	va_end(args);

	for (p = str; *p; p++)
		glutBitmapCharacter(font, *p);
}
