#ifndef UTILITY_H
#define UTILITY_H
/* Utility functions, constants, and classes used by more than one other modules */

#include "Record.h"
#include <map>
#include <iostream>
#include <algorithm>

// a simple class for error exceptions - msg points to a C-string error message

struct Error {
	Error(const char* msg_ = "") :
		msg(msg_)
		{}
	const char* msg;
};

struct Title_exception {
	Title_exception(const char* msg_) : msg(msg_)
    {}
	const char* msg;
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
        return std::count_if(records_appeared.begin(), records_appeared.end(), [](std::pair<int, int> id_count){return id_count.second > 1;});
    }
    size_t get_num_records_in_collection() const
    {return records_appeared.size();}
private:
    int total_records_in_collection;
    std::map<int, int> records_appeared;
};

// Compare two objects (passed by const&) using T's operator<
template<typename T>
struct Less_than_ref {
	bool operator() (const T& t1, const T& t2) const {return t1 < t2;}
};

// Compare two pointers (T is a pointer type) using *T's operator<
template<typename T>
struct Less_than_ptr {
	bool operator()(const T p1, const T p2) const {return *p1 < *p2;}
};

//void print_Record_helper(Record *record, std::ostream& os = std::cout);
void print_Record_helper(Record *record);

#endif
