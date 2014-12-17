#include <GL/glew.h>
#include <GL/gl.h>
#include <string.h>
#include <stdio.h>

//Function from: http://www.evl.uic.edu/aej/594/code/ogl.cpp
//Read in a textfile (GLSL program)
// we need to pass it as a string to the GLSL driver
char *textFileRead(const char *fn) 
{
	FILE *fp;
	char *content = NULL;

	int count=0;

	if (fn != NULL) {
		
		fp = fopen(fn,"rt");

		if (fp != NULL) {
      
        		fseek(fp, 0, SEEK_END);
        		count = ftell(fp);
        		rewind(fp);

			if (count > 0) {
				content = (char *)malloc(sizeof(char) * (count+1));
				count = fread(content,sizeof(char),count,fp);
				content[count] = '\0';
			}
			fclose(fp);
		
		}
	}

	return content;
}

//Function from: http://www.evl.uic.edu/aej/594/code/ogl.cpp
//Read in a textfile (GLSL program)
// we can use this to write to a text file
int textFileWrite(char *fn, char *s) 
{
	FILE *fp;
	int status = 0;

	if (fn != NULL) {
		fp = fopen(fn,"w");

		if (fp != NULL) {
			
			if (fwrite(s,sizeof(char),strlen(s),fp) == strlen(s))
				status = 1;
			fclose(fp);
		}
	}
	return(status);
}

//Got this from http://www.lighthouse3d.com/opengl/glsl/index.php?oglinfo
// it prints out shader info (debugging!)
char* printShaderInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog = NULL;
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		//strcpy( outstr, infoLog );
//		wxLogMessage("printShaderInfoLog: %s\n",infoLog);
        //free(infoLog);
	}
	return infoLog;
	//else{
	//	wxLogMessage("Shader Info Log: OK\n");
	//}
}

//Got this from http://www.lighthouse3d.com/opengl/glsl/index.php?oglinfo
// it prints out shader info (debugging!)
char* printProgramInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;
	glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		//strcpy( outstr, infoLog );
//		wxLogMessage("printProgramInfoLog: %s\n",infoLog);
        //free(infoLog);
    }
	return infoLog;
	//else{
	//	wxLogMessage("Program Info Log: OK\n");
	//}
}
