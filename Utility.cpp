#include "Utility.h"
#include "Record.h"
#include <iostream>

/*
void print_Record_helper(Record *record, std::ostream& os)
{
    os << *record << std::endl;
}*/
void print_Record_helper(Record *record)
{
    std::cout << *record << std::endl;
}