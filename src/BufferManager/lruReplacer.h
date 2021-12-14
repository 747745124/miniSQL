#pragma once

#include <list>
#include <vector>
#include <unordered_map>
typedef int frameId_t;

class LruReplacer
{
public:
	LruReplacer(int capacity)
	{
		this->capacity = capacity;
		this->lruMap.reserve(this->capacity);
	}
	~LruReplacer() = default;

	//若bufferpool已满，删除list最后的元素，删除成功返回true
	bool victim(frameId_t &frameId_)
	{
		if (lruList.size() == 0)
		{
			return false;
		}
		frameId_ = lruList.back();
		lruMap.erase(lruList.back());
		lruList.pop_back();
		return true;
	}
	//加pin并删除list和map中的记录
	void pin(frameId_t frame_id)
	{
		if (lruMap.count(frame_id))
		{
			lruList.erase(lruMap[frame_id]);
			lruMap.erase(frame_id);
		}
	}
	//解pin并增加list和map中的记录
	void unpin(frameId_t frame_id)
	{
		auto itor = lruMap.find(frame_id);
		if (itor == lruMap.end())
		{
			if (static_cast<int>(lruMap.size()) >= capacity)
			{
				while (static_cast<int>(lruMap.size()) >= capacity)
				{
					lruMap.erase(lruList.back());
					lruList.erase(prev(lruList.end()));
				}
			}
			lruList.push_front(frame_id);
			lruMap[frame_id] = lruList.begin();
		}
	}
	//返回list，map中的记录数，即bufferpool中记录的可替换page数
	int getSize()
	{
		return lruMap.size();
	}

private:
	//使用lruList储存bufferpool中可替换frame
	std::list<frameId_t> lruList;
	//使用lruMap,由frameId获得lruList迭代器
	std::unordered_map<frameId_t, std::list<frameId_t>::iterator> lruMap;
	//bufferpool的容量
	int capacity;
};
