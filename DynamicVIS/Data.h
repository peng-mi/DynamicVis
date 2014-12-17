#pragma once
#include <vector>
#include <set>
using namespace std;

#define PRECISION 0.000001
namespace VIS
{
	typedef enum
	{
		LEFT_DIR = 0,
		RIGHT_DIR,
		LARGE_DIR,
		SMALL_DIR,
		SAME_DIR,
		OTHERS,
	}t_IncrementalType;

	class CAbstractData;
	class CDataManager;
	class CCanvas;
	class CAbstractData
	{
	public:
		static vector<CAbstractData*> s_DataObjects;
		static vector<CAbstractData*> s_UpdateObjects;
		static set<CAbstractData*> s_NonExclusiveUpdateObjects;
		static void CleanAllObjects();
		static void DeleteObject(CAbstractData* _dataobject, bool _if_nonexclusive);
		static void EraseFromParent(CAbstractData* _dataobject);
		static void EraseFromChild(CAbstractData* _dataobject);
		static void EraseIsolatedData();
		static void PushStack(CAbstractData* _dataobject, bool _if_nonexclusive);
		static bool IsDeletedObject(CAbstractData* _dataobject);
		static void GetAllParents(vector<CAbstractData*>& _vec, set<CAbstractData*>& _set, unsigned int _start, unsigned int _end);
	
		CAbstractData(void);
		void Update();
		void Incremental();
		void SetDataManager(CDataManager* _dm){ m_dm = _dm;}
		CDataManager* GetDataManager();

		virtual ~CAbstractData(void);


		float m_dataProcessingTime;
		vector<CAbstractData*> m_parent;
		vector<CAbstractData*> m_child;
		vector<CAbstractData*> m_exclusiveParents;
		vector<CAbstractData*> m_exclusiveChildren;
	private:
		CDataManager* m_dm;
	protected:
		virtual void UpdateSelf(){}
		virtual void IncrementalSelf(t_IncrementalType){UpdateSelf();}
		virtual void IncrementalSelf(){ UpdateSelf();}
		virtual void CleanSelf(){}

	};

 
	void AddRelation(CAbstractData* _parent, CAbstractData* _child, bool is_exclusiveParent );

}