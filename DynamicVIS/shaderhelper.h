#ifndef _SHADERHELPER_H_
#define _SHADERHELPER_H_

extern "C"
{
char *textFileRead(const char *fn);
int textFileWrite(char *fn, char *s);
char*  printShaderInfoLog(GLuint obj);
char* printProgramInfoLog(GLuint obj);
}
#endif