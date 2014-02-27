#ifndef UTILITY_H
#define UTILITY_H
/* Utility functions, constants, and classes used by more than one other modules */

#include "Record.h"
#include <map>
#include <algorithm>

// a simple class for error exceptions - msg points to a C-string error message
struct Error {
	Error(const char* msg_ = "") :
		msg(msg_)
		{}
	const char* msg;
};

// compare two records by title
struct Compare_Record_ptr_title {
    bool operator() (const Record* p1, const Record* p2) const
    {return p1->get_title() < p2->get_title();}
};

class Collection_Statist {
public:
    Collection_Statist(): total_records_in_collection(0) {}
    void operator() (Record *record_ptr)
    {
        ++total_records_in_collection;
        int record_id = record_ptr->get_ID();
        std::map<int, int>::iterator find_iterator = records_appeared.find(record_id);
        if (find_iterator == records_appeared.end())
            records_appeared.insert(std::make_pair(record_id, 1));
        else
            ++find_iterator->second;
    }
    int get_total_occurance() const
    {return total_records_in_collection;}
    long get_num_frequent_records() const
    {
        return std::count_if(records_appeared.begin(), records_appeared.end(), [](const std::pair<int, int> &id_count){return id_count.second > 1;});
    }
    size_t get_num_records_in_collection() const
    {return records_appeared.size();}
private:
    int total_records_in_collection;
    std::map<int, int> records_appeared;
};

#endif
