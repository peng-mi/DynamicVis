#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "GraphData.h"
#include "datamanager.h"
#include "PerformanceTimer.h"
#include "color.h"
#include "Frame.h"

#include "FilteredData.h"
#include "RawData.h"

namespace VIS
{

	extern "C" void kernel_function(t_LayoutParamter* layoutParameter,t_GraphNode* graph_node,t_GraphEdge* graph_edge);

	/******************************************************************************************************
	********************CGraphIData*******************************************************************
	******************************************************************************************************/

	CGraphIData::CGraphIData()
	{
		Init();
	}

	void CGraphIData::Init()
	{
		m_data.is_layout = false;
		m_data.graphInfo.is_total = false;
		m_data.graphInfo.is_merge = false;
		m_data.graphInfo.items[0] = -1;
		m_data.graphInfo.items[1] = -1;
		m_data.node = NULL;
		m_data.edge = NULL;
		m_data.graphPara.graphReady = false;
		m_data.graphPara.numNode = 0;
		m_data.graphPara.numEdge = 0;
		m_data.graphPara.boundary[0] = -1.0f;
		m_data.graphPara.boundary[1] = 1.0f;
		m_data.graphPara.boundary[2] = 1.0f;
		m_data.graphPara.baryCenter[0] =m_data.graphPara.baryCenter[1] =0.0f;  
		
		m_data.nodeInfo.clear();
		m_data.edgeInfo.clear();
		m_data.vertex_IdToValue.clear();
		m_data.extra_edges.clear();
		m_data.vertices.clear();
		m_data.records.clear();
		m_data.vertex_ValueToId.clear();
	}

	void  CGraphIData::CleanSelf()
	{
		if(m_data.node != NULL)
			delete [] m_data.node;
		if(m_data.edge != NULL)
			delete [] m_data.edge;
		Init();
	}

	void CGraphIData::UpdateSelf()
	{
		CDataManager* dm = GetDataManager();
		CGraphIData* idata = NULL;
		if(m_data.graphPara.graphReady)
		{
			//if this is the base color, we do not need to generate the compactlist
			for(unsigned int i = 0; i <dm->m_BasedIDataList.size(); i++)
			{
				idata = dynamic_cast<CGraphIData*>(dm->m_BasedIDataList[i]);
				if(idata != NULL)
				{
					if(idata == this)
						return;
					else
						break;
				}
			}
			PrepareRecords();
		}
	}

	void CGraphIData::IncrementalSelf()
	{
		if(m_data.graphPara.graphReady)
			PrepareRecords();
	}

	void CGraphIData::CreateGraph()
	{	
		PerformanceTimer _pTimer;
		_pTimer.StartTimer();
		
		PrepareRecords();//compactList;
		
		if(!m_data.graphInfo.is_merge)
			PrepareGraph();
		else
			PrepareMergeGraph();

		m_data.graphPara.graphReady = true;
		m_dataProcessingTime = (float)_pTimer.GetTimeElapsed();
	}

	
	void CGraphIData::UpdateVector(unsigned int _index)
	{
		CDataManager* dm = GetDataManager();
		t_uintvector vec;
		unsigned int idx_1 = m_data.graphInfo.items[0];
		unsigned int idx_2 = m_data.graphInfo.items[1];
		if( idx_1 == -1 || idx_2 == -1)
			return;
		pair<t_recordset::iterator,bool> ret;

		if(dm->m_RawData->m_item_desc[idx_1].num_values == 0) //numerical
			vec.push_back(_index);
		else
			vec.push_back(dm->m_RawData->GetDataValue(_index,idx_1));
		if(dm->m_RawData->m_item_desc[idx_2].num_values == 0) //numerical
			vec.push_back(_index);
		else
			vec.push_back(dm->m_RawData->GetDataValue(_index,idx_2));

		ret = m_data.records.insert(pair<t_uintvector,uint>(vec,1));
		if(ret.second == false)
			m_data.records[vec] = m_data.records[vec]+1;
	}

	void CGraphIData::PrepareRecords()
	{
		CDataManager* dm = GetDataManager();
		m_data.records.clear();
		if(m_data.graphInfo.is_total)
		{
			for(unsigned int i=0;i<dm->m_RawData->m_NumberOfRecords;i++)
				UpdateVector(i);
		}
		else
		{
			CFilteredData* parent = (CFilteredData*)m_parent[0];
			set<uint>::iterator set_it;
			for(set_it = parent->m_filteredData.begin(); set_it != parent->m_filteredData.end(); set_it++)
				UpdateVector(*set_it);
		}
	}

	void CGraphIData::PrepareGraph()
	{
		CDataManager* dm = GetDataManager();
		if(m_data.graphInfo.items[0] == -1 || m_data.graphInfo.items[1] == -1)
			return;

		m_data.graphPara.numEdge = m_data.records.size();
		m_data.edge = new t_GraphEdge[m_data.graphPara.numEdge*2];

		t_ref::iterator it_ref;
		pair<t_ref::iterator,bool> ret;
		unsigned int index = 0; 
		unsigned int offset = dm->m_RawData->m_item_desc[m_data.graphInfo.items[0]].num_values; //item_1 num
		m_data.offset = offset;
		m_data.vertices.resize(2);
		for(t_recordset::iterator it = m_data.records.begin(); it != m_data.records.end(); it++)
		{
			t_uintvector vec;
			vec.push_back(it->first[1]);
			vec.push_back(it->first[0]);
			m_data.extra_edges.insert(pair<t_uintvector,uint>(vec,it->second));

			ret = m_data.vertices[0].insert(pair<uint,uint>(it->first[0],1));
			if(ret.second == false)
				m_data.vertices[0][it->first[0]]++;
			ret = m_data.vertices[1].insert(pair<uint,uint>(it->first[1],1));
			if(ret.second == false)
				m_data.vertices[1][it->first[1]]++;
		}

		m_data.graphPara.numNode_1 = m_data.vertices[0].size();
		m_data.graphPara.numNode_2 = m_data.vertices[1].size();

		index = 0;
		for(it_ref = m_data.vertices[0].begin(); it_ref != m_data.vertices[0].end(); it_ref++)
		{
			m_data.vertex_IdToValue.insert(pair<unsigned int, unsigned int>(index,it_ref->first));
			m_data.vertex_ValueToId.insert(pair<unsigned int, unsigned int>(it_ref->first,index));
			index++;
		}
		for(it_ref = m_data.vertices[1].begin(); it_ref != m_data.vertices[1].end(); it_ref++)
		{
			m_data.vertex_IdToValue.insert(pair<uint,uint>(index,it_ref->first));
			m_data.vertex_ValueToId.insert(pair<uint,uint>(it_ref->first+offset,index));
			index++;
		}

		index = 0;
		for(t_recordset::iterator it = m_data.records.begin(); it != m_data.records.end(); it++)
		{
			m_data.edge[index].node = m_data.vertex_ValueToId[it->first[1] + offset];
			m_data.edge[index].weight =  it->second;
			index++;
		}
		for(t_recordset::iterator it = m_data.extra_edges.begin(); it != m_data.extra_edges.end(); it++)
		{
			m_data.edge[index].node = m_data.vertex_ValueToId[it->first[1]];
			m_data.edge[index].weight =  it->second;
			index++;
		}

		m_data.graphPara.numNode = m_data.vertices[0].size() + m_data.vertices[1].size();
		m_data.node = new t_GraphNode[m_data.graphPara.numNode];

		m_data.node[0].begin = 0;
		m_data.node[0].numEdge = m_data.vertices[0].begin()->second;
		m_data.node[0].pos[0] = ((float)rand()/RAND_MAX-0.5)*2.0f;
		m_data.node[0].pos[1] = ((float)rand()/RAND_MAX-0.5)*2.0f;
		m_data.node[0].stable = false;
		m_data.node[0].weight = 1.0f;

		index = 1;
		for(it_ref = m_data.vertices[0].begin(); it_ref != m_data.vertices[0].end(); it_ref++)
		{
			if(it_ref == m_data.vertices[0].begin())
				continue;

			m_data.node[index].begin = m_data.node[index-1].numEdge + m_data.node[index-1].begin;
			m_data.node[index].numEdge = it_ref->second;
			m_data.node[index].pos[0] = ((float)rand()/RAND_MAX-0.5)*2.0f;
			m_data.node[index].pos[1] = ((float)rand()/RAND_MAX-0.5)*2.0f;
			m_data.node[index].stable = false;
			m_data.node[index].weight = 1.0f;
			index++;
		}

		m_data.node[index].begin = m_data.node[index-1].numEdge + m_data.node[index-1].begin;
		m_data.node[index].numEdge = m_data.vertices[1].begin()->second;
		m_data.node[index].pos[0] = ((float)rand()/RAND_MAX-0.5)*2.0f;
		m_data.node[index].pos[1] = ((float)rand()/RAND_MAX-0.5)*2.0f;
		m_data.node[index].stable = false;
		m_data.node[index].weight = 1.0f;
		index++;

		for(it_ref = m_data.vertices[1].begin(); it_ref != m_data.vertices[1].end(); it_ref++)
		{
			if(it_ref == m_data.vertices[1].begin())
				continue;

			m_data.node[index].begin = m_data.node[index-1].numEdge + m_data.node[index-1].begin;
			m_data.node[index].numEdge = it_ref->second;
			m_data.node[index].pos[0] = ((float)rand()/RAND_MAX-0.5)*2.0f;
			m_data.node[index].pos[1] = ((float)rand()/RAND_MAX-0.5)*2.0f;
			m_data.node[index].stable = false;
			m_data.node[index].weight = 1.0f;
			index++;
		}
	}

	void CGraphIData::PrepareMergeGraph()
	{
		
		PrepareGraph();

		CDataManager* dm = GetDataManager();
		map<string,unsigned int> _nameNode_1,_nameNode_2;
		unsigned int id_1 = m_data.graphInfo.items[0];
		unsigned int id_2 = m_data.graphInfo.items[1];
		for(uint i=0; i<m_data.graphPara.numNode_1; i++)
		{
			string _name = dm->m_RawData->m_item_desc[id_1].value_names[m_data.vertex_IdToValue[i]];
			_nameNode_1.insert(pair<string, uint>(_name,m_data.vertex_IdToValue[i]));
		}
		for(uint i= m_data.graphPara.numNode_1; i< m_data.graphPara.numNode; i++)
		{
			string _name = dm->m_RawData->m_item_desc[id_2].value_names[m_data.vertex_IdToValue[i]];
			_nameNode_2.insert(pair<string, uint>(_name,m_data.vertex_IdToValue[i]));
		}

		map<uint,uint>& _duplicated = m_data.duplicatedS_D;
		map<uint,uint>& _dup2 = m_data.duplicatedD_S;
		_duplicated.clear();
		map<string,uint>::iterator str1_it;
		map<string,uint>::iterator str2_it;
		for(str1_it = _nameNode_1.begin(); str1_it != _nameNode_1.end(); str1_it++)
		{
			str2_it = _nameNode_2.find(str1_it->first);
			if( str2_it != _nameNode_2.end())
			{
				_duplicated.insert(pair<uint,uint>(str1_it->second, str2_it->second));
				_dup2.insert(pair<uint,uint>(str2_it->second,str1_it->second));
			}
		}

		if(_duplicated.size() ==0)
			return;

		//if have duplicated items
		t_GraphNode* _node;
		t_GraphEdge* _edge; 

		_node = new t_GraphNode[m_data.graphPara.numNode - _duplicated.size()];
		_edge = new t_GraphEdge[m_data.graphPara.numEdge*2];
		uint offset = dm->m_RawData->m_item_desc[m_data.graphInfo.items[0]].num_values;
		t_ref::iterator it_ref;
		uint index = 0;
		map<uint,uint>::iterator it_dup;
		for(it_ref = m_data.vertices[0].begin(); it_ref != m_data.vertices[0].end(); it_ref++)
		{
			_node[index].pos[0] = m_data.node[index].pos[0];
			_node[index].pos[1] = m_data.node[index].pos[1];
			_node[index].stable = m_data.node[index].stable;
			_node[index].weight = m_data.node[index].weight;

			it_dup = _duplicated.find(it_ref->first);
			if(it_dup == _duplicated.end())
				_node[index].numEdge = m_data.node[index].numEdge;

			else
			{
				uint du_id = m_data.vertex_ValueToId[it_dup->second + offset];
				_node[index].numEdge = m_data.node[index].numEdge + m_data.node[du_id].numEdge;
			}
			index++;
		}

		index = 0; //calculate the node begin;
		for(it_ref = m_data.vertices[0].begin(); it_ref != m_data.vertices[0].end(); it_ref++)
		{
			if(it_ref == m_data.vertices[0].begin())
				_node[index].begin =0;
			else
				_node[index].begin = _node[index-1].begin + _node[index-1].numEdge;
			index++;
		}

		index = m_data.graphPara.numNode_1;
		map<uint,uint> _node2map;
		for(it_ref = m_data.vertices[1].begin(); it_ref != m_data.vertices[1].end(); it_ref++)
		{
			if(_dup2.find(it_ref->first) != _dup2.end())//duplicated
				continue;
			_node2map.insert(pair<uint,uint>(it_ref->first,index));
			index++;
		}

		index = 0;
		uint index_edge =0;
		for(it_ref = m_data.vertices[0].begin(); it_ref != m_data.vertices[0].end(); it_ref++)
		{
			for(uint j=0; j<_node[index].numEdge; j++)
			{
				if(j<m_data.node[index].numEdge)//previous
				{
					_edge[index_edge].weight = m_data.edge[m_data.node[index].begin + j].weight;

					uint node_id_1 = m_data.edge[m_data.node[index].begin + j].node;
					uint node_id_2 = m_data.vertex_IdToValue[node_id_1];
					map<uint,uint>::iterator it_map = _dup2.find(node_id_2);
					if(it_map != _dup2.end())
						_edge[index_edge].node = m_data.vertex_ValueToId[it_map->second];
					else
						_edge[index_edge].node = _node2map[node_id_1];
					index_edge++;
				}
				else //the current
				{
					uint _tmp = j - m_data.node[index].numEdge;
					uint node_1 = m_data.vertex_IdToValue[index];
					uint node_2 = _duplicated[node_1];
					uint node_3 = m_data.vertex_ValueToId[node_2 + offset];
					_edge[index_edge].weight = m_data.edge[m_data.node[node_3].begin + _tmp].weight;
					_edge[index_edge].node  = m_data.edge[m_data.node[node_3].begin + _tmp].node;
					index_edge++;
				}
			}
			index++;
		}

		////////////////////////node 2
		index = m_data.graphPara.numNode_1;
		for(it_ref = m_data.vertices[1].begin(); it_ref != m_data.vertices[1].end(); it_ref++)
		{
			if(_dup2.find(it_ref->first) != _dup2.end())//duplicated
				continue;
			uint node_id = m_data.vertex_ValueToId[it_ref->first + offset];

			_node[index].numEdge = m_data.node[node_id].numEdge;
			_node[index].pos[0] = m_data.node[node_id].pos[0];
			_node[index].pos[1] = m_data.node[node_id].pos[1];
			_node[index].stable = m_data.node[node_id].stable;
			_node[index].weight = m_data.node[node_id].weight;
			index++;
		}
		m_data.graphPara.numNode_2 = index - m_data.graphPara.numNode_1;
		m_data.graphPara.numNode = index;
		index = m_data.graphPara.numNode_1;
		for(index = m_data.graphPara.numNode_1; index < m_data.graphPara.numNode; index++ )
			_node[index].begin = _node[index-1].begin + _node[index-1].numEdge;

		for(it_ref = m_data.vertices[1].begin(); it_ref != m_data.vertices[1].end(); it_ref++)
		{
			if(_dup2.find(it_ref->first) != _dup2.end())//duplicated
				continue;
			uint node_id = m_data.vertex_ValueToId[it_ref->first + offset];

			for(uint j=0; j<m_data.node[node_id].numEdge;j++)
			{
				_edge[index_edge].weight = m_data.edge[m_data.node[node_id].begin + j].weight;
				_edge[index_edge].node = m_data.edge[m_data.node[node_id].begin + j].node;
				index_edge++;
			}
		}

		delete [] m_data.node;
		delete [] m_data.edge;

		m_data.node = new t_GraphNode[m_data.graphPara.numNode];
		m_data.edge = new t_GraphEdge[m_data.graphPara.numEdge*2];

		for(uint i=0; i<m_data.graphPara.numNode; i++)
		{
			m_data.node[i].begin = _node[i].begin;
			m_data.node[i].numEdge = _node[i].numEdge;
			m_data.node[i].pos[0] = _node[i].pos[0];
			m_data.node[i].pos[1] = _node[i].pos[1];
			m_data.node[i].stable = _node[i].stable;
			m_data.node[i].weight = _node[i].weight;
		}

		for(uint  i=0; i<m_data.graphPara.numEdge*2; i++)
		{
			m_data.edge[i].node = _edge[i].node;
			m_data.edge[i].weight = _edge[i].weight;
		}

		delete [] _node;
		delete [] _edge;
		
	}


	void CGraphIData::GraphLayout()
	{
		PerformanceTimer _pTimer;
		_pTimer.StartTimer();

		float repSum  = 2.0f*m_data.graphPara.numEdge;
		float density =1.0f/repSum;
		m_data.graphPara.repFactor = density*pow(repSum,0.5f*(m_data.graphPara.attExponent - m_data.graphPara.repExponent));
		m_data.graphPara.graFactor = pow(m_data.graphPara.graScale,(m_data.graphPara.attExponent - m_data.graphPara.repExponent));
		m_data.graphPara.baryCenter[0] = m_data.graphPara.baryCenter[1] = 0.0f;
		if(m_data.graphPara.repFactor >10000.0f)
			m_data.graphPara.repFactor = 10000.0f;
		if(m_data.graphPara.graFactor<0.00001)
			m_data.graphPara.graFactor = 0.00001f;


		kernel_function(&(m_data.graphPara),m_data.node,m_data.edge);

		int index_max =0;
		int index_min =0;

		for (int i=0;i<m_data.graphPara.numNode;i++)
		{
			if (m_data.node[i].pos[0]>m_data.graphPara.boundary[1])
			{
				index_max = i;
				m_data.graphPara.boundary[1]=m_data.node[i].pos[0];
			}
			else if(m_data.node[i].pos[0]<m_data.graphPara.boundary[0])
			{
				index_min = i;
				m_data.graphPara.boundary[0]=m_data.node[i].pos[0];
			}

			if (m_data.node[i].pos[1]>m_data.graphPara.boundary[1])
			{
				index_max = i;
				m_data.graphPara.boundary[1]=m_data.node[i].pos[1];
			}
			else if(m_data.node[i].pos[1]<m_data.graphPara.boundary[0])
			{
				index_min = i;
				m_data.graphPara.boundary[0]=m_data.node[i].pos[1];
			}
		}

		m_data.graphPara.boundary[2] = m_data.graphPara.boundary[0]+m_data.graphPara.boundary[1]; 
		if(m_data.graphPara.boundary[2]>0)
			m_data.graphPara.boundary[2] = m_data.graphPara.boundary[1];
		else
			m_data.graphPara.boundary[2] = 0- m_data.graphPara.boundary[0];

		m_dataProcessingTime = (float)_pTimer.GetTimeElapsed();

	}




	/******************************************************************************************************
	********************CGraphRData*******************************************************************
	******************************************************************************************************/

	CGraphRData::CGraphRData(CCanvas* _canvas)
	{
		m_vbo = new CVBOData(_canvas);
	}
	CGraphRData::~CGraphRData()
	{
		
		if( m_vbo != NULL)
		{
			m_vbo->CleanData();
			delete m_vbo;
			m_vbo = NULL;
		}
	}


	void  CGraphRData::CleanSelf()
	{
		CGraphIData* idata = NULL;
		if(m_parent.size() > 1)
		{
			if(m_vbo != NULL)
			m_vbo->CleanData();
		}
		else
		{
			idata = dynamic_cast<CGraphIData*>(m_parent[0]);
			assert(idata != NULL);
			if(idata->m_data.graphPara.numNode != m_vbo->m_vertexNum || idata->m_data.is_layout)
				m_vbo->CleanData();
		}
	}

	void CGraphRData::PrepareVBO()
	{
		CGraphIData* idata = NULL;
		idata = dynamic_cast<CGraphIData*>(m_parent[0]);
		assert(idata != NULL);

		unsigned int order = 0;
		for(unsigned int i = 0; i < m_vbo->m_canvas->m_RDataList.size(); i++)
		{
			order = i;
			if(m_vbo->m_canvas->m_RDataList[i] == this)
				break;
		}
		
		//highlight dataset
		if(m_parent.size() >1)
		{
			CGraphIData* ex_parent = NULL;
			ex_parent = dynamic_cast<CGraphIData*>(m_parent[1]);
			assert(ex_parent);

			if(ex_parent->m_data.records.size() ==0)
				return;
			
			set<unsigned int> node_1, node_2;
			vector<pair<unsigned int, unsigned int>> records;
			t_ref::iterator it_res;
			if(idata->m_data.vertices.size() != 2)
				return;
		
			for(t_recordset::iterator it = ex_parent->m_data.records.begin(); it != ex_parent->m_data.records.end(); it++)
			{
				it_res = idata->m_data.vertices[0].find(it->first[0]);
				if(it_res == idata->m_data.vertices[0].end())
					continue;

				it_res = idata->m_data.vertices[1].find(it->first[1]);
				if(it_res == idata->m_data.vertices[1].end())
					continue;
				records.push_back(pair<unsigned int, unsigned int>(it->first[0], it->first[1]));
			}
			
			for(int i = 0; i < records.size(); i++)
			{
				node_1.insert(records[i].first);
				node_2.insert(records[i].second);
			}
			
			m_vbo->m_vertexNum = node_1.size() + node_2.size();
			m_vbo->m_vertexFromat = 3;
			m_vbo->m_vertexData = (float*)malloc(m_vbo->m_vertexFromat*m_vbo->m_vertexNum*sizeof(float));

			map<unsigned int, unsigned int> edges_1;
			map<unsigned int, unsigned int> edges_2;
			unsigned int idx = 0;
			for(set<unsigned int>::iterator it = node_1.begin(); it != node_1.end(); it++)
			{
				unsigned int index = idata->m_data.vertex_ValueToId[*it];
				m_vbo->m_vertexData[idx*3] = idata->m_data.node[index].pos[0];
				m_vbo->m_vertexData[idx*3 +1] = idata->m_data.node[index].pos[1];
				m_vbo->m_vertexData[idx*3 +2] = order*0.1f;
				edges_1.insert(pair<unsigned int, unsigned int>(*it, idx));
				idx++;
			}
			for(set<unsigned int>::iterator it = node_2.begin(); it != node_2.end(); it++)
			{
				unsigned int index;
				it_res = idata->m_data.duplicatedD_S.find(*it);
				if(it_res ==  idata->m_data.duplicatedD_S.end())
				{
					if(idata->m_data.vertex_ValueToId.find(*it + idata->m_data.offset) != idata->m_data.vertex_ValueToId.end())
						index = idata->m_data.vertex_ValueToId[*it + idata->m_data.offset];
					else
					{
						//errors
						int error =0;
						assert(error != 0);
					}
				}
				else
				{
					if(idata->m_data.vertex_ValueToId.find(it_res->second) != idata->m_data.vertex_ValueToId.end())
						index = idata->m_data.vertex_ValueToId[it_res->second];
					else
					{
						//errors
						int error =0;
						assert(error != 0);
					}
				}

				m_vbo->m_vertexData[idx*3] = idata->m_data.node[index].pos[0];
				m_vbo->m_vertexData[idx*3 +1] = idata->m_data.node[index].pos[1];
				m_vbo->m_vertexData[idx*3 +2] = order*0.1f;
				edges_2.insert(pair<unsigned int, unsigned int>(*it,idx));
				idx++;
			}

			m_vbo->m_indexNum = records.size()*2;
			m_vbo->m_indexData = (unsigned int*)malloc(m_vbo->m_indexNum*sizeof(unsigned int));
			for(unsigned int i = 0; i < records.size(); i++)
			{
				m_vbo->m_indexData[2*i] = edges_1[records[i].first];
				m_vbo->m_indexData[2*i +1] = edges_2[records[i].second];
			}

		}
		//background dataset
		else
		{
			if(idata->m_data.graphPara.numNode != m_vbo->m_vertexNum)
			{
				m_vbo->m_vertexNum = idata->m_data.graphPara.numNode;
				m_vbo->m_vertexFromat = 3;
				m_vbo->m_vertexData = (float*)malloc(m_vbo->m_vertexNum*m_vbo->m_vertexFromat*sizeof(float));

				for(unsigned int i = 0; i < idata->m_data.graphPara.numNode; i++)
				{
					m_vbo->m_vertexData[3*i] = idata->m_data.node[i].pos[0];
					m_vbo->m_vertexData[3*i+1] = idata->m_data.node[i].pos[1];
					m_vbo->m_vertexData[3*i+2] = order*0.1f;
				}

				m_vbo->m_indexNum = idata->m_data.records.size()*2;
				m_vbo->m_indexData = (unsigned int*)malloc(m_vbo->m_indexNum*sizeof(unsigned int));
				unsigned int idx = 0;
				for(unsigned int i = 0; i < idata->m_data.graphPara.numNode_1; i++)
				{
					for(unsigned int j = 0; j < idata->m_data.node[i].numEdge; j++)
					{
						m_vbo->m_indexData[2*idx] = i;
						m_vbo->m_indexData[2*idx+1] = idata->m_data.edge[idata->m_data.node[i].begin + j].node;
						idx++;
					}
				}
			}
		}
		m_vbo->BindVBO();
	}

	void CGraphRData::UpdateSelf()
	{
		PerformanceTimer _pTimer;
		_pTimer.StartTimer();

		CleanSelf();
		PrepareVBO();

		m_dataProcessingTime = (float)_pTimer.GetTimeElapsed();
	}

	void CGraphRData::IncrementalSelf()
	{
		UpdateSelf();
	}


}
