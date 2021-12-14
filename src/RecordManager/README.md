# Record Manager

所有的接口都大致實現,通過buffer來存取數據
**這次更改沒有調試!!!是針對index的調用做了一點更改**

```C++
	void createTableFile(const string& tablename);
	void dropTableFile(const string& tablename);
	int insertRecord(const string& tablename, const Tuple& tuple_in);
	void deleteRecord(const string& tablename);
	void deleteRecord(const string& tablename, const vector<Compare>& compare);
	void deleteRecord(const string& tablename, const Position& pos, const int& number);
	vector<Tuple> selectRecord(const string& tablename);
	vector<Tuple> selectRecord(const string& tablename, const vector<Compare>& compare);
	vector<Tuple> selectRecord(const string& tablename, const Position& pos, const int number = 1);
```

~~做了一點點調試(除buffer外的模块調用的部分還沒有進行調試)大致上沒有問題~~


## 定長記錄

int 用 32位, double 用64位, char(n)用n位

不足的直接在左方用空格補充

## test

表格是用三種數據類型各一個來組成, 其中名為“float”的是unique限制的

testRecordManager.cpp 中把所有接口都運行了一次，從結果上沒有問題

