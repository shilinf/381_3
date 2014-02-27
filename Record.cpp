#include "Record.h"
#include "Utility.h"
#include <string>
#include <fstream>
#include <iostream>

using std::string;
using std::endl;

int Record::ID_counter = 0;
int Record::ID_counter_backup = 0;

Record::Record(const string& medium_, const string& title_) : ID(++ID_counter), rate(0), title(title_), medium(medium_) {}

Record::Record(const string& title_) : ID(0), rate(0), title(title_) {}

Record::Record(int ID_) : ID(ID_), rate(0) {}

Record::Record(const Record &old, const std::string& title_) : ID(old.ID), rate(old.rate), title(title_), medium(old.medium) {}

Record::Record(std::ifstream& is)
{
    if (!(is >> ID >> medium >> rate && is.get() && getline(is, title)))
        throw Error("Invalid data found in file!");
    if (ID > ID_counter)
        ID_counter = ID;
}

void Record::set_rating(int rating_)
{
    if (rating_ >=1 && rating_ <= 5)
        rate = rating_;
    else
        throw Error("Rating is out of range!");
}

void Record::save(std::ostream& os) const
{
    os << ID << " " << medium << " " << rate << " " << title << std::endl;
}

std::ostream& operator<< (std::ostream& os, const Record& record)
{
    if (record.rate)
        os << record.ID << ": " << record.medium << " " << record.rate << " " << record.title;
    else
        os << record.ID << ": " << record.medium << " u " << record.title;
    return os;
}

std::ostream& operator<< (std::ostream& os, const Record* record_ptr)
{
    os << *record_ptr << endl;
    return os;
}
