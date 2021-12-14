#include "RecordManager.h"
#include <iostream>

using namespace std;

void output(vector<Data> info, vector<Tuple> temp);

int main()
{
	Data test[3];
	vector<Data> info;
	vector<Tuple> temp;
	vector<Compare> compare(3);
	Position pos;
	RecordManager recordmanager;
	int count = 0;

	srand((int)time(0));
	//	recordmanager.createTableFile("test");	
	 recordmanager.deleteRecord("test");
	//	recordmanager.dropTableFile("test");

	int integer = 0;
	double doublenum =0;
	while (count <= 50) //²åÈë50‚€value ÓÃµ½2‚€page
	{
		Tuple tuple;
		count++;
		test[0].type = -1; test[0].datai = integer++;
		test[1].type = 0; test[1].dataf = doublenum;
		test[2].type = 20; test[2].datas = "test";
		doublenum += 0.1;

		for (int i = 0; i < 3; i++) tuple.addData(test[i]);

		recordmanager.insertRecord("test", tuple);
	}

	cout << "select *" << endl;
	temp = recordmanager.selectRecord("test");
	output(info, temp);
	cout << endl;

	cout << "select int <= 10 and float > 0 and string = test"<<endl;
	compare[0].attrName = "int"; compare[0].data.datai = 10; compare[0].data.type = -1; compare[0].type = CompareType::LE;
	compare[1].attrName = "float"; compare[1].data.dataf = 0.0; compare[1].data.type = 0; compare[1].type = CompareType::G;
	compare[2].attrName = "string"; compare[2].data.datas = "test"; compare[2].data.type = 4; compare[2].type = CompareType::E;
	temp = recordmanager.selectRecord("test", compare);
	output(info, temp);
	cout << endl;

	cout << "select pageid = 1, offset = 116, number = 1" << endl;
	pos.pageId = 1; pos.offset = 116;
	temp = recordmanager.selectRecord("test", pos);
	output(info, temp); 
	cout << endl;

	cout << "select pageid = 0, offset = 0, number = 5" << endl;
	pos.pageId = 0; pos.offset = 0;
	temp = recordmanager.selectRecord("test", pos, 5);
	output(info, temp);
	cout << endl;

	cout << "delect int <= 10 and float > 0 and string = test" << endl;
	recordmanager.deleteRecord("test", compare);
	temp = recordmanager.selectRecord("test", compare);
	output(info, temp);
	cout << endl;

	cout << "delete pageid = 0, offset = 0, number = 1" << endl;
	pos.pageId = 0; pos.offset = 0;
	recordmanager.deleteRecord("test", pos, 1);
	temp = recordmanager.selectRecord("test", pos, 5);
	output(info, temp);
	cout << endl;

	cout << "select *" << endl;
	temp = recordmanager.selectRecord("test");
	output(info, temp);
	cout << endl;

	//recordmanager.dropTableFile("test");

}

void output(vector<Data> info, vector<Tuple> temp)
{
	for (int i = 0; i < temp.size(); i++)
	{
		cout << i+1 << ": ";
		info = temp[i].getData();
		for (int j = 0; j < (int)info.size(); j++)
		{
			switch (info[j].type) {
			case(-1): cout << info[j].datai << " "; break;
			case(0):	cout << info[j].dataf << " "; break;
			default: cout << info[j].datas << " "; break;
			}		}
		cout << endl;
	}
}