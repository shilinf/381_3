#include "Collection.h"
#include "Record.h"
#include "Utility.h"
#include <string>
#include <fstream>
#include <iostream>

using std::endl;
using std::string;
using std::set;


Collection::Collection(std::ifstream& is, const set<Record*, Less_than_ptr<Record*>>& library)
{
    int num_records;
    if (!(is >> name >> num_records))
        throw Error("Invalid data found in file!");
    is.get(); // read the new line character
    for (int i = 0; i < num_records; i++) {
        string title;
        getline(is, title);
        Record probe(title);
        auto find_Record_item_iterator = library.find(&probe);
        if (find_Record_item_iterator == library.end())
            throw Error("Invalid data found in file!");
        add_member(*find_Record_item_iterator);
    }
}

void Collection::add_member(Record* record_ptr)
{
    if (is_member_present(record_ptr))
        throw Error("Record is already a member in the collection!");
    else
        members.insert(record_ptr);
}

bool Collection::is_member_present(Record* record_ptr) const
{
    return !(members.find(record_ptr) == members.end());
}

void Collection::remove_member(Record* record_ptr)
{
    if (is_member_present(record_ptr))
        members.erase(members.find(record_ptr));
    else
        throw Error("Record is not a member in the collection!");
}

void Collection::save(std::ostream& os) const
{
    os << name << " " << members.size() << endl;
    for (auto record_ptr : members)
        os << record_ptr->get_title() << endl;
}

std::ostream& operator<< (std::ostream& os, const Collection& collection)
{
    os << "Collection " << collection.name << " contains:";
    if (collection.empty())
        os << " None" << endl;
    else {
        os << endl;
        for (auto record_ptr : collection.members)
            os << *record_ptr <<endl;
    }
    return os;
}
