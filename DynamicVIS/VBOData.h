#pragma once

#include "GL/gl.h"

namespace VIS
{
	class CCanvas;
	class CVBOData
	{
	public:
		CVBOData(CCanvas* _canvas){Init();SetCanvas(_canvas);}
		~CVBOData(void){CleanData();}

		void Init();
		void CleanData();
		void CreateVBO();
		void BindVBO();
		void DestoryVBO();
		void PreKernel();
		void AfterKernel();
		void SetCanvas(CCanvas* _canvas){ m_canvas = _canvas;}

	private:
		void CreateVBOE(GLuint *vbo, unsigned int size);
		void DeleteVBOE(GLuint *vbo);
	public:
		bool	m_if_GPU;
		GLuint  m_vertexBufferId;
		GLuint  m_indexBufferId;
		GLuint  m_colorBufferId;
		float*	m_vertexData;
		unsigned int*   m_indexData;
		unsigned char*	m_colorData;
		unsigned int	m_vertexNum;
		unsigned int	m_indexNum;
		unsigned int	m_colorNum;
		unsigned int m_vertexFromat;
		unsigned int m_colorFormat;
		unsigned int m_indexFormat;
		CCanvas *m_canvas;

	};
}
