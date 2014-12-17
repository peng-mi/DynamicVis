#include "glhelper.h"

void GetWorldCoordinate( float &_posX, float &_posY, int _screenX, int _screenY )
{
	float winX, winY, winZ;
	winX = (float) _screenX;
	winY = (float) _screenY;

	GLdouble projMatrix[16];
	GLdouble modelMatrix[16];
	GLint viewport[4];

	glGetIntegerv(GL_VIEWPORT,viewport);
	glGetDoublev (GL_MODELVIEW_MATRIX, modelMatrix);   
	glGetDoublev (GL_PROJECTION_MATRIX, projMatrix);

	winY = (float)viewport[3] - winY; 
	glReadPixels(winX, winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

	GLdouble posX, posY, posZ;
	gluUnProject( winX, winY, winZ, modelMatrix, projMatrix, viewport, &posX, &posY, &posZ);

	_posX = (float) posX; 
	_posY = (float) posY; 
}

int round_value( float _value, float _start, float _step )
{
	if ( _value - _start < 0.0f )
		return (int)((_value - _start - _step) / _step);
	else
		return (int)((_value - _start) / _step);
}