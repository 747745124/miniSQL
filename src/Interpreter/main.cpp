#include <iostream>
#include <string>

#include "Interpreter.h"

using namespace std;

int main()
{
    std::string s;

    Interpreter *in = new Interpreter();
    // in->setQuery("select id,name form student;");
    // in->setQuery("create table (id int primary key, name char(255), address
    // char(255));");
    in->setQuery(
        "create table person ( "
        "	height float unique,"
        "	pid int,"
        "name char(32),"
        "identity char(128) unique,"
        "age int unique,"
        "primary key(pid)");
    in->runQuery();

    in->setQuery("create index idx_height on person(height);");
    in->runQuery();

    in->setQuery("drop index idx_height;");
    in->runQuery();

    in->setQuery("drop table person;");
    in->runQuery();

    in->setQuery("select * from person where identity = \"Person15\";");
    in->runQuery();

    while (true)
    {
        cout << "minisql> ";
        string temp, q;
        while (getline(cin, temp))
        {
            bool run = false;
            if (temp.find(';') != string::npos)
            {
                temp.erase(temp.find_first_of(';') + 1);
                run = true;
            }
            q += temp + " ";

            if (run)
            {
                try
                {
                    in->setQuery(q);
                    int result = in->runQuery();
                    // quit;
                    if (result == 0)
                        exit(0);
                }
                catch (exception &e)
                {
                    cout << e.what() << endl;
                }
                q.clear();
                break;
            }
            else
            {
                cout << "      -> ";
            }
        }
    }
}