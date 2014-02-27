#include "Collection.h"
#include "Record.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <iterator>


using std::endl;
using std::string;
using std::set;
using std::for_each; using std::copy; using std::set_union;
using std::inserter;
using std::ref;


Collection::Collection(std::ifstream& is, const set<Record*, Compare_Record_ptr_title>& library)
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

Collection::Collection(const Collection& c1, const Collection& c2, const std::string& name_): name(name_)
{
    set_union(c1.members.begin(), c1.members.end(), c2.members.begin(), c2.members.end(), inserter(members, members.begin()));
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

void Collection::modify_member_title(Record *old_record, Record *new_record)
{
    if (is_member_present(old_record)) {
        members.erase(members.find(old_record));
        members.insert(new_record);
    }
}

void Collection::get_collection_statist(Collection_Statist &statist) const
{
    for_each(members.begin(), members.end(), ref(statist));
}

void Collection::save(std::ostream& os) const
{
    os << name << " " << members.size() << endl;
    for_each(members.begin(), members.end(), [&os](Record *record_ptr){os << record_ptr->get_title() << endl;});
}

std::ostream& operator<< (std::ostream& os, const Collection& collection)
{
    os << "Collection " << collection.name << " contains:";
    if (collection.empty())
        os << " None" << endl;
    else {
        os << endl;
        
        for_each(collection.members.begin(), collection.members.end(), [&os](Record *record_ptr){os << *record_ptr << endl;});
    }
    return os;
}




