#include "gl/glew.h"
//cuda
//#include <cutil_inline.h>    
//#include <cutil_gl_inline.h>
#include <cuda_gl_interop.h>
#include <cuda.h>
#include <cuda_runtime_api.h>
#include "VBOData.h"
#include "Frame.h"
namespace VIS
{

	void CVBOData::Init()
	{
		m_vertexBufferId = 0;
		m_indexBufferId = 0;
		m_colorBufferId = 0;

		m_indexNum = 0;
		m_vertexNum = 0;
		m_colorNum = 0;
		
		m_vertexData = NULL;
		m_indexData = NULL;
		m_colorData = NULL;

		m_vertexFromat = 0;
		m_colorFormat =0;
		m_indexFormat = 0;
	}

	void CVBOData::CleanData()
	{
		//DestoryVBO();
		if(!m_if_GPU)
		{
			if(m_vertexData != NULL)
				free(m_vertexData);
			if(m_indexData != NULL)
				free(m_indexData);
			if(m_colorData != NULL)
				free(m_colorData);

			Init();
		}
		Init();
	}

	void CVBOData::PreKernel()
	{
		m_canvas->SetCurrent();
		if(m_vertexNum > 0)
			cudaGLMapBufferObject((void**)&(m_vertexData), m_vertexBufferId);
		if(m_indexNum > 0)
			cudaGLMapBufferObject((void**)&(m_indexData), m_indexBufferId);
		if(m_colorNum > 0)
			cudaGLMapBufferObject((void**)&(m_colorData), m_colorBufferId);
	}

	void CVBOData::AfterKernel()
	{
		if(m_vertexNum > 0)
			cudaGLUnmapBufferObject(m_vertexBufferId);
		if(m_indexNum > 0)
			cudaGLUnmapBufferObject(m_indexBufferId);
		if(m_colorNum > 0)
			cudaGLUnmapBufferObject(m_colorBufferId);
	}

	void CVBOData::CreateVBOE(GLuint *vbo, unsigned int size)
	{
		glGenBuffers(1, vbo);
		glBindBuffer(GL_ARRAY_BUFFER, *vbo);
		// initialize buffer object
		glBufferData(GL_ARRAY_BUFFER, size, 0, GL_STATIC_DRAW);
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// register buffer object with CUDA
		cudaGLRegisterBufferObject(*vbo);
	}

	void CVBOData::DeleteVBOE(GLuint *vbo)
	{
		glBindBuffer(1, *vbo);
		glDeleteBuffers(1, vbo);
		if(m_if_GPU)
			cudaGLUnregisterBufferObject(*vbo);
		*vbo = NULL;
	}

	void CVBOData::BindVBO()
	{
		m_canvas->SetCurrent();
		DestoryVBO();
		if(m_if_GPU)
			CreateVBO();
		else
		{
			if(m_vertexNum !=0)
			{
				glGenBuffers(1,&m_vertexBufferId);
				glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
				glBufferData(GL_ARRAY_BUFFER, m_vertexNum*m_vertexFromat*sizeof(float), m_vertexData,GL_STATIC_DRAW);
			}

			if(m_indexNum != 0)
			{
				glGenBuffers(1, &m_indexBufferId);
				glBindBuffer(GL_ARRAY_BUFFER, m_indexBufferId);
				glBufferData(GL_ARRAY_BUFFER, m_indexNum*m_indexFormat*sizeof(unsigned int), m_indexData,GL_STATIC_DRAW);
			}

			if(m_colorNum !=0)
			{
				glGenBuffers(1, &m_colorBufferId);
				glBindBuffer(GL_ARRAY_BUFFER, m_colorBufferId);
				glBufferData(GL_ARRAY_BUFFER, m_colorNum*m_colorFormat*sizeof(unsigned char), m_colorData,GL_STATIC_DRAW);
			}
		}
	}

	void CVBOData::CreateVBO()
	{
		DestoryVBO();
		m_canvas->SetCurrent();
		if(m_vertexNum > 0)
			CreateVBOE(&(m_vertexBufferId), m_vertexNum*m_vertexFromat*sizeof(float));
		if(m_indexNum > 0)
			CreateVBOE(&(m_indexBufferId), m_indexNum*m_indexFormat*sizeof(unsigned int));
		if(m_colorNum > 0)
			CreateVBOE(&(m_colorBufferId), m_colorNum*m_colorFormat*sizeof(unsigned char));
	}

	void CVBOData::DestoryVBO()
	{
		if(m_vertexBufferId !=0)
			DeleteVBOE(&(m_vertexBufferId));
		if(m_indexBufferId !=0)
			DeleteVBOE(&(m_indexBufferId));
		if(m_colorBufferId !=0)
			DeleteVBOE(&(m_colorBufferId));
	}

}
