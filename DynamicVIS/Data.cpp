#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include <GL/glew.h>
#include "Data.h"
#include "Frame.h"
#include "datamanager.h"
#include "PerformanceTimer.h"


namespace VIS
{
	/******************************************************************************************************
	********************CAbstractData***********************************************************************
	******************************************************************************************************/
	vector<CAbstractData*> CAbstractData::s_DataObjects;
	vector<CAbstractData*> CAbstractData::s_UpdateObjects;
	set<CAbstractData*> CAbstractData::s_NonExclusiveUpdateObjects;


	CAbstractData::CAbstractData(void) : m_dataProcessingTime(0.0f),m_dm(NULL)
	{
		s_DataObjects.push_back( this );
	}

	CAbstractData::~CAbstractData(void)
	{
		CleanSelf();
		vector<CAbstractData*>::iterator it;
		CAbstractData *dataobject;
		for ( it = s_DataObjects.begin(); it != s_DataObjects.end(); it++ )
		{
			dataobject = *it;
			if ( dataobject == this )
				break;
		}
		if ( it != s_DataObjects.end() )
		{
			CAbstractData::EraseFromParent(dataobject);
			CAbstractData::EraseFromChild(dataobject);
			s_DataObjects.erase( it );
		}
	}

	void CAbstractData::CleanAllObjects()
	{
		for ( unsigned int i = 0; i < s_DataObjects.size(); i++ )
			if ( s_DataObjects[i] != NULL )
				s_DataObjects[i]->CleanSelf();
	}

	void CAbstractData::EraseFromParent(CAbstractData* _dataobject)
	{
		vector<CAbstractData*>::iterator it;
		for ( unsigned int i = 0; i < _dataobject->m_parent.size(); i++ )
		{
			CAbstractData* _parent = _dataobject->m_parent[i];
			for ( it = _parent->m_child.begin(); it !=  _parent->m_child.end(); it++ )
				if ( *it ==  _dataobject )
					break;
			if ( it != _parent->m_child.end() ) // we found it.
				_parent->m_child.erase( it );
		}

		for ( unsigned int i = 0; i < _dataobject->m_exclusiveParents.size(); i++ )
		{
			CAbstractData* _parent = _dataobject->m_exclusiveParents[i];
			for ( it = _parent->m_exclusiveChildren.begin(); it !=  _parent->m_exclusiveChildren.end(); it++ )
				if ( *it ==  _dataobject )
					break;
			if ( it != _parent->m_exclusiveChildren.end() ) // we found it.
				_parent->m_exclusiveChildren.erase( it );
		}
	}

	void CAbstractData::GetAllParents(vector<CAbstractData*>& _vec, set<CAbstractData*>& _set, unsigned int _start, unsigned int _end)
	{
		if( _end >= _vec.size() || _end < _start )
			return; 

		for(unsigned int i = _start; i <=  _end; i++)
		{
			for(unsigned int j = 0; j < _vec[i]->m_parent.size(); j++)
				_set.insert(_vec[i]->m_parent[j]);
		}
	}

	void CAbstractData::EraseFromChild(CAbstractData* _dataobject)
	{
		vector<CAbstractData*>::iterator it;
		for ( unsigned int i = 0; i < _dataobject->m_child.size(); i++ )
		{
			CAbstractData* _child = _dataobject->m_child[i];
			for ( it = _child->m_parent.begin(); it !=  _child->m_parent.end(); it++ )
				if ( *it ==  _dataobject )
					break;
			if ( it != _child->m_parent.end() ) // we found it.
				_child->m_parent.erase( it );
		}

		for ( unsigned int i = 0; i < _dataobject->m_exclusiveChildren.size(); i++)
		{
			CAbstractData* _child = _dataobject->m_exclusiveChildren[i];
			for(it = _child->m_exclusiveParents.begin(); it != _child->m_exclusiveParents.end(); it++)
				if( *it == _dataobject)
					break;
			if(it != _child->m_exclusiveParents.end())
				_child->m_exclusiveParents.erase( it );
		}
	}

	void CAbstractData::EraseIsolatedData()
	{
		//erase isolated nodes
		for(vector<CAbstractData*>::iterator it = s_DataObjects.begin(); it != s_DataObjects.end(); )
		{
			if( (*it)->m_parent.size() == 0 && (*it)->m_child.size() == 0 )
			{
				delete (*it);
				it = s_DataObjects.erase(it);
			}
			else
				it++;
		}
	}

	void CAbstractData::DeleteObject(CAbstractData* _dataobject, bool _if_nonexclusive )
	{
		s_UpdateObjects.clear();
		PushStack(_dataobject, _if_nonexclusive);
		for(unsigned int i=0; i < s_UpdateObjects.size(); i++)
		{
			delete s_UpdateObjects[i];
			s_UpdateObjects[i] = NULL;
		}
		s_UpdateObjects.clear();

		for(set<CAbstractData*>::iterator it = s_NonExclusiveUpdateObjects.begin(); it != s_NonExclusiveUpdateObjects.end(); it++)
			(*it)->UpdateSelf();
		s_NonExclusiveUpdateObjects.clear();
	}

	void CAbstractData::PushStack(CAbstractData* _dataobject, bool _if_nonexclusive)
	{
		vector<CAbstractData*> *vec; 
		if(_if_nonexclusive)
			vec = &_dataobject->m_child;
		else
		{
			vec = &_dataobject->m_exclusiveChildren;
			for ( unsigned int i = 0; i<_dataobject->m_child.size(); i++ )
				s_NonExclusiveUpdateObjects.insert( _dataobject->m_child[i] );
		}

		s_UpdateObjects.push_back(_dataobject);
		for(unsigned int i=0; i< vec->size();i++)
		{
			if(!_if_nonexclusive)
				s_NonExclusiveUpdateObjects.erase((*vec)[i]);
			
			vector<CAbstractData*>::iterator _it;
			for(_it = s_UpdateObjects.begin(); _it != s_UpdateObjects.end(); _it++)
			{
				if(*_it == _dataobject->m_child[i])
				{
					s_UpdateObjects.erase(_it);
					break;
				}
			}
			PushStack((*vec)[i], _if_nonexclusive);
		}
	}

	bool CAbstractData::IsDeletedObject(CAbstractData* _dataobject)
	{
		unsigned int i = 0 ;
		for(; i < s_DataObjects.size(); i++)
		{
			if(_dataobject == s_DataObjects[i])
				break;
		}
		if( i == s_DataObjects.size())
			return true;
		else
			return false;
	}

	CDataManager* CAbstractData::GetDataManager()
	{
		if(m_dm != NULL)
			return m_dm;
		else
		{
			for(int i = 0; i < m_parent.size(); i++)
			{
				if( m_parent[i]->GetDataManager() != NULL)
					return m_parent[i]->GetDataManager();
			}
			return NULL;
		}
	}

 	void CAbstractData::Update()
	{
		s_UpdateObjects.clear();
		PushStack(this, true);
		for(uint i=0; i < s_UpdateObjects.size(); i++)
			s_UpdateObjects[i]->UpdateSelf();
	}

	void CAbstractData::Incremental()
	{	
		s_UpdateObjects.clear();
		PushStack(this, true);
		for(uint i=0; i < s_UpdateObjects.size(); i++)
			s_UpdateObjects[i]->IncrementalSelf();
	}

	void AddRelation(CAbstractData* _parent, CAbstractData* _child, bool is_exclusiveParent)
	{
		_parent->m_child.push_back( _child );
		_child->m_parent.push_back( _parent );
		if(is_exclusiveParent)
		{
			_child->m_exclusiveParents.push_back(_parent);
			_parent->m_exclusiveChildren.push_back(_child);
		}
	}

}