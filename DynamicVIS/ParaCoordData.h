#pragma once
#include "header.h"
#include "Data.h"
#include "VBOData.h"
namespace VIS
{
	class CParaCoordIData : public CAbstractData
	{
	public:
		CParaCoordIData();
		CParaCoordIData(bool _gpu): m_compactNum(0), m_compactRecords(NULL),m_if_GPU(_gpu){};
		~CParaCoordIData();
		void UpdateSelf();
		void IncrementalSelf();
		void CleanSelf();
		void SetGPU(bool _gpu);
		

	public:
		t_uintvector m_selectedItems;
		t_recordset m_records;
		unsigned int* m_compactRecords;
		unsigned int  m_compactNum;
		bool		  m_if_GPU;
		//GPU data structures
		static bool	  m_loadData;
		
		

	private:
		void PrepareData();

		//GPU
		void GPU_Init();
		void GPU_TransferConstantData();
		void GPU_PrepareData();

	};


	class CParaCoordRData : public CAbstractData
	{
	public:
		CParaCoordRData(CCanvas* _canvas, bool _gpu);
		~CParaCoordRData();
		void UpdateSelf();
		void IncrementalSelf();
		void CleanSelf();
		void setCanvas();
		void SetGPU(bool _gpu);

	public:
		bool m_if_GPU;
		vector<t_ref> m_vertices;
		t_EdgeSet m_edges;
		CVBOData *m_vbo;

	private:
		CCanvas* m_canvas;

		void PrepareVertexEdge(CParaCoordIData* _idata, vector<t_ref>& _vertices, t_EdgeSet& _edges);
		void PrepareVBO();
		void PrepareData();
		void PrepareColorBuffer();
		t_color GetColor(CParaCoordIData* _idata);
		unsigned int GetDepth();
		//GPU
		void GPU_PrepareData();
		

		
	};

}