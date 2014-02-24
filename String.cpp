#include "String.h"
#include <cstring>
#include <utility>
#include <iostream>
#include <new>

using std::string;
using std::cout; using std::endl;


char String::a_null_byte = '\0';
int String::number = 0;
int String::total_allocation = 0;
bool String::messages_wanted = false;

String::String(const char* cstr_) : str_length(int(strlen(cstr_)))
{
    if (messages_wanted)
        cout << "Ctor: \"" << cstr_ << "\"" << endl;
    constructor_helper(cstr_);
}

String::String(const String& original) : str_length(original.size())
{
    if (messages_wanted)
        cout << "Copy ctor: \""<< original << "\"" << endl;
    constructor_helper(original.c_str());
}

String::String(const String& original, int i, int len) : str_length(len)
{
    constructor_helper(original.c_str()+i);
}

void String::constructor_helper(const char* cstr_)
{
    if (str_length) {
        str_allocation = str_length + 1;
        str = new char[str_allocation];
        strncpy(str, cstr_, str_length);
        str[str_length] = '\0';
    }
    else {
        str_allocation = 0;
        str = &a_null_byte;
    }
    ++number;
    total_allocation += str_allocation;
}

String::String(String&& original) noexcept :
str(&a_null_byte), str_length(0), str_allocation(0)
{
    if (messages_wanted)
        cout << "Move ctor: \"" << original << "\"" << endl;
    swap(original);
    ++number;
}

String::~String() noexcept
{
    if (messages_wanted)
        cout << "Dtor: \""  << str << "\"" << endl;
    --number;
    total_allocation -= str_allocation;
    if (!str)
        delete[] str;
}

String& String::operator= (const String& rhs)
{
    if (messages_wanted)
        cout << "Copy assign from String:  \"" << rhs << "\"" << endl;
    String temp(rhs);
    swap(temp);
    return *this;
}

String& String::operator= (const char* rhs)
{
    if (messages_wanted)
        cout << "Assign from C-string:  \"" << rhs << "\"" << endl;
    String temp(rhs);
    swap(temp);
    return *this;
}

String& String::operator= (String&& rhs) noexcept
{
    if (messages_wanted)
        cout << "Move assign from String:  \"" << rhs << "\"" << endl;
    swap(rhs);
    return *this;
}

char& String::operator[] (int i)
{
    if (i >= 0 && i < str_length)
        return *(str + i);
    else
        throw String_exception("Subscript out of range");
}

const char& String::operator[] (int i) const
{
    if (i >= 0 && i < str_length)
        return *(str + i);
    else
        throw String_exception("Subscript out of range");
}

String String::substring(int i, int len) const
{
    if (i >= 0 && len >= 0 && i <= str_length && (i + len) <= str_length)
        return String(*this, i, len);
    else
        throw String_exception("Substring bounds invalid");
}

void String::clear()
{
    String empty_string;
    swap(empty_string);
}

void String::remove(int i, int len)
{
    if (i >= 0 && len >= 0 && i <= str_length && (i + len) <= str_length) {
        str_length -= len;
        for (int index = i; index < str_length; index++)
            str[index] = str[index + len];
        str[str_length] = '\0';
    }
    else
        throw String_exception("Remove bounds invalid");
}

void String::insert_before(int i, const String& src)
{
    insert_before_helper(i, src.c_str());
}

String& String::operator += (char rhs)
{
    char cstr[2];
    cstr[0] = rhs;
    cstr[1] = '\0';
    insert_before_helper(str_length, cstr);
    return *this;
}

String& String::operator += (const char* rhs)
{
    insert_before_helper(str_length, rhs);
    return *this;
}

String& String::operator += (const String& rhs)
{
    insert_before(str_length, rhs);
    return *this;
}

void String::insert_before_helper(int i, const char *cstr)
{
    if (i < 0 || i > str_length)
        throw String_exception("Insertion point out of range");
    int cstr_size = int(strlen(cstr));
    if (str_allocation >= str_length + cstr_size + 1)
        copy_helper(i, str, cstr);
    else {
        int pre_allocation = str_allocation;
        str_allocation = 2 * (str_length + cstr_size + 1);
        total_allocation += str_allocation - pre_allocation;
        char *new_str = new char[str_allocation];
        for (int index = 0; index < i; index++)
            new_str[index] = str[index];
        copy_helper(i, new_str, cstr);
        if (!str)
            delete[] str;
        str = new_str;
    }
    str_length += cstr_size;
}

void String::copy_helper(int i, char* str_desti, const char* src_insert)
{
    int cstr_size = int(strlen(src_insert));
    // Copy characters after i to their destination from end to i,
    // to make sure no data is overwritten before copying when str_desti is
    // the same as str. The first copy character is null.
    for (int index = str_length; index >= i; --index)
        str_desti[index + cstr_size] = str[index];
    // copy characters in cstr to its destination
    for (int index = 0; index < cstr_size; index++)
        str_desti[index + i] = src_insert[index];
}

void String::swap(String& other) noexcept
{
    std::swap(str, other.str);
    std::swap(str_length, other.str_length);
    std::swap(str_allocation, other.str_allocation);
}

bool operator== (const String& lhs, const String& rhs)
{
    return !strcmp(lhs.c_str(), rhs.c_str());
}

bool operator!= (const String& lhs, const String& rhs)
{
    return strcmp(lhs.c_str(), rhs.c_str());
}

bool operator< (const String& lhs, const String& rhs)
{
    return (strcmp(lhs.c_str(), rhs.c_str()) < 0);
}

bool operator> (const String& lhs, const String& rhs)
{
    return (strcmp(lhs.c_str(), rhs.c_str()) > 0);
}

String operator+ (const String& lhs, const String& rhs)
{
    String concatenation(lhs);
    concatenation += rhs;
    return concatenation;
}

std::ostream& operator<< (std::ostream& os, const String& str)
{
    os << str.c_str();
    return os;
}

std::istream& operator>> (std::istream& is, String& str)
{
    str.clear();
    while (isspace(is.peek())) {
        if (is)
            is.get();
        else
            return is;
    }
    while (!isspace(is.peek())) {
        if (is)
            str += is.get();
        else
            return is;
    }
    return is;
}

std::istream& getline(std::istream& is, String& str)
{
    str.clear();
    while (is.peek() != EOF && is.peek() != '\n') {
        if (is)
            str += is.get();
        else
            return is;
    }
    return is;
}
