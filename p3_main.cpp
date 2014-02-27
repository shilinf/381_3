#include "Record.h"
#include "Collection.h"
#include "Utility.h"
#include <string>
#include <iostream>
#include <fstream>
#include <cctype>
#include <utility>
#include <map>
#include <set>
#include <list>
#include <vector>
#include <algorithm>
#include <iterator>
#include <functional>
#include <new>

using std::string;
using std::cin; using std::cout; using std::endl;
using std::ifstream; using std::ofstream;
using std::isspace;
using std::make_pair;
using std::set; using std::list; using std::vector; using std::map;
using std::find_if; using std::lower_bound; using std::for_each;
using std::transform; using std::replace_if;
using std::ostream_iterator;
using std::bind; using std::mem_fn; using std::ref; using std::placeholders::_1;
using std::greater;

using Ordered_by_title_lib_t = set<Record *, Compare_Record_ptr_title>;
using Ordered_by_id_lib_t = vector<Record *>;
using Catalog_t = vector<Collection>;

struct Database_t {
    Catalog_t catalog;
    Ordered_by_title_lib_t library_ordered_by_title;
    Ordered_by_id_lib_t library_ordered_by_id;
};

using Command_fp_t = void (*)(Database_t&);
using Command_map_t = map<string, Command_fp_t>;

struct Title_exception {
	Title_exception(const char* msg_) : msg(msg_)
    {}
	const char* msg;
};

// functions to handle command
void find_Record_match_title(Database_t &database);
void print_Record_match_id(Database_t &database);
void print_Collection_match_name(Database_t &database);
void print_Records(Database_t &database);
void print_Catalog(Database_t &database);
void print_memory_allocation(Database_t &database);
void add_Record(Database_t &database);
void add_Collection(Database_t &database);
void add_Record_to_Collection(Database_t &database);
void modify_Record_rating(Database_t &database);
void delete_Record_from_Library(Database_t &database);
void delete_Collection_from_Catalog(Database_t &database);
void delete_Record_from_Collection(Database_t &database);
void clear_Library(Database_t &database);
void clear_Catalog(Database_t &database);
void clear_all_data(Database_t &database);
void save_all_data(Database_t &database);
void restore_all_data(Database_t &database);
void quit(Database_t &database);
void find_with_string(Database_t &database);
void list_ratings(Database_t &database);
void collection_statistics(Database_t &database);
void combine_collections(Database_t &database);
void modify_title(Database_t &database);

// helper functions
void load_command_map(Command_map_t& commands_map);
Ordered_by_title_lib_t::iterator read_title_probe_Record(Ordered_by_title_lib_t
                                                         &library);
string read_check_title();
void trim_title(string &title);
Ordered_by_title_lib_t::iterator probe_Record_by_title(string title,
                                            Ordered_by_title_lib_t &library);
string read_check_new_title_name(Ordered_by_title_lib_t &library);
Ordered_by_id_lib_t::iterator read_ID_probe_Record(Ordered_by_id_lib_t &library);
int read_and_check_integer();
Ordered_by_id_lib_t::iterator probe_Record_by_id(int id,
                                                 Ordered_by_id_lib_t &library);
Catalog_t::iterator read_check_collection_name(Catalog_t &catalog);
Catalog_t::iterator find_collection_iterator(Catalog_t &catalog,
                                             string &collection_name);
Catalog_t::iterator read_check_new_collection_name(Catalog_t &catalog,
                                                   string &new_collection_name);
void clear_Library_helper(Database_t &database);
void free_all_records(Ordered_by_title_lib_t &library);
void clear_all_data_helper(Database_t &database);
int read_int_from_file(ifstream &is);
void insert_new_Record(Database_t &database, Record *new_record);
bool compare_record_with_id(Record *record_ptr, int id);
void discard_input_remainder(void);

int main ()
{
    map<string, void(*)(Database_t &)> commands_map;
    load_command_map(commands_map);
    Database_t database;
    while (true) {
        cout << endl << "Enter command: ";
        char action, object;
        cin >> action;
        cin >> object;
        string command;
        command += action;
        command += object;
        try {
            if (command == "qq") {
                quit(database);
                return 0;
            }
            else {
                Command_fp_t cfp = commands_map[command];
                if (cfp)
                    cfp(database);
                else {
                    commands_map.erase(command);
                    cout << "Unrecognized command!" << endl;
                    discard_input_remainder();
                }
            }
        } catch (Error& error) {
            cout << error.msg << endl;
            discard_input_remainder();
        } catch (Title_exception& error) {
            cout << error.msg << endl;
        } catch (...) {
            cout << "Unknown exception caught" <<endl;
            return 1;
        }
    }
    return 0;
}

void load_command_map(Command_map_t& commands_map)
{
    commands_map.insert(make_pair("fr", find_Record_match_title));
    commands_map.insert(make_pair("pr", print_Record_match_id));
    commands_map.insert(make_pair("pc", print_Collection_match_name));
    commands_map.insert(make_pair("pL", print_Records));
    commands_map.insert(make_pair("pC", print_Catalog));
    commands_map.insert(make_pair("pa", print_memory_allocation));
    commands_map.insert(make_pair("ar", add_Record));
    commands_map.insert(make_pair("ac", add_Collection));
    commands_map.insert(make_pair("am", add_Record_to_Collection));
    commands_map.insert(make_pair("mr", modify_Record_rating));
    commands_map.insert(make_pair("dr", delete_Record_from_Library));
    commands_map.insert(make_pair("dc", delete_Collection_from_Catalog));
    commands_map.insert(make_pair("dm", delete_Record_from_Collection));
    commands_map.insert(make_pair("cL", clear_Library));
    commands_map.insert(make_pair("cC", clear_Catalog));
    commands_map.insert(make_pair("cA", clear_all_data));
    commands_map.insert(make_pair("sA", save_all_data));
    commands_map.insert(make_pair("rA", restore_all_data));
    commands_map.insert(make_pair("fs", find_with_string));
    commands_map.insert(make_pair("lr", list_ratings));
    commands_map.insert(make_pair("cs", collection_statistics));
    commands_map.insert(make_pair("cc", combine_collections));
    commands_map.insert(make_pair("mt", modify_title));
}

// find and print the specified record with the matching title
void find_Record_match_title(Database_t &database)
{
    auto library_iterator =
    read_title_probe_Record(database.library_ordered_by_title);
    cout << *library_iterator;
}

// print the specified record with the matching ID number
void print_Record_match_id(Database_t &database)
{
    auto library_iterator = read_ID_probe_Record(database.library_ordered_by_id);
    cout << *library_iterator;
}

// print collection - print each record in the collection with the
// specified name
void print_Collection_match_name(Database_t &database)
{
    Catalog_t::iterator catalog_iterator =
    read_check_collection_name(database.catalog);
    cout << *catalog_iterator;
}

// print all the records in the Library
void print_Records(Database_t &database)
{
    if (database.library_ordered_by_title.empty())
        cout << "Library is empty" << endl;
    else {
        cout << "Library contains " << database.library_ordered_by_title.size()
        << " records:" <<endl;
        copy(database.library_ordered_by_title.begin(),
             database.library_ordered_by_title.end(),
             ostream_iterator<const Record *>(cout));
    }
}

// print the Catalog - print all the collections in the Catalog
void print_Catalog(Database_t &database)
{
    if (database.catalog.empty())
        cout << "Catalog is empty" <<endl;
    else {
        cout << "Catalog contains " << database.catalog.size()
        << " collections:" <<endl;
        copy(database.catalog.begin(), database.catalog.end(),
             ostream_iterator<Collection &>(cout));
    }
}

// print memory allocations
void print_memory_allocation(Database_t &database)
{
    cout << "Memory allocations:" <<endl;
    cout << "Records: " << database.library_ordered_by_title.size() <<endl;
    cout << "Collections: " << database.catalog.size() << endl;
}

// add a record to the Library
void add_Record(Database_t &database)
{
    string medium;
    cin >> medium;
    string title = read_check_new_title_name(database.library_ordered_by_title);
    Record *new_record = new Record(medium, title);
    insert_new_Record(database, new_record);
    cout << "Record " << new_record->get_ID() << " added" << endl;
}

// add a collection with the specified name
void add_Collection(Database_t &database)
{
    string new_collection_name;
    auto insert_position = read_check_new_collection_name(database.catalog,
                                                          new_collection_name);
    database.catalog.insert(insert_position, Collection(new_collection_name));
    cout << "Collection " << new_collection_name << " added" <<endl;
}

// add a record to a specified collection
void add_Record_to_Collection(Database_t &database)
{
    Catalog_t::iterator catalog_iterator =
            read_check_collection_name(database.catalog);
    Ordered_by_id_lib_t::iterator library_iterator =
            read_ID_probe_Record(database.library_ordered_by_id);
    catalog_iterator->add_member(*library_iterator);
    cout << "Member " << (*library_iterator)->get_ID() << " "
        << (*library_iterator)->get_title() <<" added" << endl;
}

// modify the rating of the specified record with the matching ID number
void modify_Record_rating(Database_t &database)
{
    Ordered_by_id_lib_t::iterator library_iterator =
            read_ID_probe_Record(database.library_ordered_by_id);
    int rating_input = read_and_check_integer();
    (*library_iterator)->set_rating(rating_input);
    cout << "Rating for record " << (*library_iterator)->get_ID()
        << " changed to " << rating_input << endl;
}

// delete the specified record from the Library
void delete_Record_from_Library(Database_t &database)
{
    auto library_title_iterator =
        read_title_probe_Record(database.library_ordered_by_title);
    if (find_if(database.catalog.begin(), database.catalog.end(),
                bind(&Collection::is_member_present, _1,
                     *library_title_iterator)) != database.catalog.end())
        throw Title_exception("Cannot delete a record that is a member of a collection!");
    int record_ID = (*library_title_iterator)->get_ID();
    Record *record_ptr = * library_title_iterator;
    database.library_ordered_by_title.erase(library_title_iterator);
    database.library_ordered_by_id.erase(probe_Record_by_id(record_ID,
                                            database.library_ordered_by_id));
    cout << "Record " << record_ID << " " << record_ptr->get_title()
        << " deleted" << endl;
    delete record_ptr;
}

// delete the specified collection from the Catalog
void delete_Collection_from_Catalog(Database_t &database)
{
    auto catalog_iterator = read_check_collection_name(database.catalog);
    string collection_name = catalog_iterator->get_name();
    database.catalog.erase(catalog_iterator);
    cout << "Collection " << collection_name << " deleted" << endl;
}

// delete the specified record as member of the a specified collection
void delete_Record_from_Collection(Database_t &database)
{
    auto catalog_iterator = read_check_collection_name(database.catalog);
    auto library_iterator = read_ID_probe_Record(database.library_ordered_by_id);
    catalog_iterator->remove_member(*library_iterator);
    cout << "Member " << (*library_iterator)->get_ID() << " "
        << (*library_iterator)->get_title() << " deleted" << endl;
}

// Clear the Library. If collections are not empty, throw exception
void clear_Library(Database_t &database)
{
    clear_Library_helper(database);
    cout <<"All records deleted" <<endl;
}
void clear_Library_helper(Database_t &database)
{
    if (find_if_not(database.catalog.begin(), database.catalog.end(),
                    mem_fn(&Collection::empty)) != database.catalog.end())
        throw Error("Cannot clear all records unless all collections are empty!");
    void free_all_records(Ordered_by_title_lib_t &library);
    database.library_ordered_by_title.clear();
    database.library_ordered_by_id.clear();
    Record::reset_ID_counter();
}

// clear the Catalog: destroy all of the collections in the Catalog,
// and clear the Catalog
void clear_Catalog(Database_t &database)
{
    database.catalog.clear();
    cout << "All collections deleted" <<endl;
}

// clear all data: first clear the Catalog as in cC,
// then clear the Library as in cL
void clear_all_data(Database_t &database)
{
    clear_all_data_helper(database);
    cout << "All data deleted" <<endl;
}

void clear_all_data_helper(Database_t &database)
{
    database.catalog.clear();
    clear_Library_helper(database);
}

// save all data: write the Library and Catalog data to the named file
void save_all_data(Database_t &database)
{
    string file_name;
    cin >> file_name;
    ofstream output_file(file_name.c_str());
    if (!output_file.is_open())
        throw Error("Could not open file!");
    output_file << database.library_ordered_by_title.size() << endl;
    for_each(database.library_ordered_by_title.begin(),
             database.library_ordered_by_title.end(),
             bind(&Record::save, _1, ref(output_file)));
    output_file << database.catalog.size() << endl;
    for_each(database.catalog.begin(), database.catalog.end(),
             bind(&Collection::save, _1, ref(output_file)));
    output_file.close();
    cout << "Data saved" <<endl;
}

// restore all data - restore the Library and Catalog data from the file
// if fail, roll back the to the data before calling this command
void restore_all_data(Database_t &database)
{
    string file_name;
    cin >> file_name;
    ifstream input_file(file_name.c_str());
    if (!input_file.is_open())
        throw Error("Could not open file!");
    auto local_catalog(database.catalog);
    auto local_library_ordered_by_title(database.library_ordered_by_title);
    auto local_library_ordered_by_id(database.library_ordered_by_id);
    database.catalog.clear();
    database.library_ordered_by_id.clear();
    database.library_ordered_by_title.clear();
    Record::save_ID_counter();
    Record::reset_ID_counter();
    try {
        int num_record = read_int_from_file(input_file);
        for (int i=0; i < num_record; i++) {
            Record *new_record = new Record(input_file);
            insert_new_Record(database, new_record);
        }
        int num_collection = read_int_from_file(input_file);
        for (int i = 0; i < num_collection; i++) {
            Collection new_collection(input_file,
                                      database.library_ordered_by_title);
            database.catalog.insert(lower_bound(database.catalog.begin(),
                                                database.catalog.end(),
                                                new_collection), new_collection);
        }
        input_file.close();
        free_all_records(local_library_ordered_by_title);
        cout << "Data loaded" <<endl;
    } catch (Error &error) {
        clear_all_data_helper(database);
        database.library_ordered_by_title.swap(local_library_ordered_by_title);
        database.library_ordered_by_id.swap(local_library_ordered_by_id);
        database.catalog.swap(local_catalog);
        Record::restore_ID_counter();
        input_file.close();
        throw;
    }
}

int read_int_from_file(ifstream &is)
{
    int input_int;
    if (!(is >> input_int))
        throw Error("Invalid data found in file!");
    return input_int;
}

// clear all data (as in cA), and also destroy the Library and Catalog
// containers themselves, so that all memory is deallocated, and then terminate
void quit(Database_t &database)
{
    clear_all_data_helper(database);
    cout << "All data deleted" << endl;
    cout << "Done" << endl;
}

// function object class for command find_with_string
class Find_string {
public:
    Find_string() : find_status(false) {}
    void operator() (Record *record_ptr, string key_word)
    {
        string lower_title = record_ptr->get_title();
        transform(lower_title.begin(), lower_title.end(), lower_title.begin(),
                  tolower);
        if (lower_title.find(key_word) != string::npos) {
            find_status = true;
            cout << record_ptr;
        }
    }
    bool get_find_status()
    {return find_status;}
private:
    bool find_status;
};

void find_with_string(Database_t &database)
{
    string key_word;
    cin >> key_word;
    transform(key_word.begin(), key_word.end(), key_word.begin(), tolower);
    Find_string find_str;
    for_each(database.library_ordered_by_title.begin(),
             database.library_ordered_by_title.end(),
             bind(ref(find_str), _1, ref(key_word)));
    if (!find_str.get_find_status())
        throw Error("No records contain that string!");
}

void list_ratings(Database_t &database)
{
    if (database.library_ordered_by_title.empty())
        cout << "Library is empty" << endl;
    else {
        map<int, list<Record *>, greater<int>> library_ordered_by_rate;
        for_each(database.library_ordered_by_title.begin(),
                 database.library_ordered_by_title.end(),
                 [&library_ordered_by_rate](Record *record_ptr)
        {library_ordered_by_rate[record_ptr->get_rate()].push_back(record_ptr);});
        for (auto list_rating : library_ordered_by_rate)
            copy(list_rating.second.begin(), list_rating.second.end(),
                     ostream_iterator<const Record *>(cout));
    }
}

void collection_statistics(Database_t &database)
{
    Collection_Statist statist;
    for_each(database.catalog.begin(), database.catalog.end(),
             bind(&Collection::get_collection_statist, _1, ref(statist)));
    size_t library_size = database.library_ordered_by_title.size();
    cout << statist.get_num_records_in_collection() <<" out of "<< library_size
        <<" Records appear in at least one Collection" << endl;
    cout << statist.get_num_frequent_records() <<" out of " << library_size
        <<" Records appear in more than one Collection" << endl;
    cout << "Collections contain a total of "<< statist.get_total_occurance()
        <<" Records" << endl;
}

void combine_collections(Database_t &database)
{
    Catalog_t::iterator catalog_iterator1 = read_check_collection_name(database.catalog);
    Catalog_t::iterator catalog_iterator2 = read_check_collection_name(database.catalog);
    string collection1_name = catalog_iterator1->get_name();
    string collection2_name = catalog_iterator2->get_name();
    string new_collection_name;
    auto insert_position = read_check_new_collection_name(database.catalog, new_collection_name);
    database.catalog.insert(insert_position, Collection(*catalog_iterator1, *catalog_iterator2, new_collection_name));
    cout << "Collections "<< collection1_name<<" and "<< collection2_name
        <<" combined into new collection "<< new_collection_name << endl;
}

void modify_title(Database_t &database)
{
    auto library_id_iterator = read_ID_probe_Record(database.library_ordered_by_id);
    string new_title = read_check_new_title_name(database.library_ordered_by_title);
    Record *old_record = *library_id_iterator;
    Record *new_record = new Record(*old_record, new_title);
    auto library_title_iterator = probe_Record_by_title(old_record->get_title(),
                                            database.library_ordered_by_title);
    database.library_ordered_by_title.erase(library_title_iterator);
    database.library_ordered_by_id.erase(library_id_iterator);
    insert_new_Record(database, new_record);
    for_each(database.catalog.begin(), database.catalog.end(),
             bind(&Collection::modify_member_title, _1,
                  ref(old_record), ref(new_record)));
    cout <<"Title for record "<<old_record->get_ID()<< " changed to "
        << new_title <<endl;
    delete old_record;
}

// read title and probe the title in library
Ordered_by_title_lib_t::iterator read_title_probe_Record(Ordered_by_title_lib_t
                                                         &library)
{
    string title = read_check_title();
    auto library_iterator = probe_Record_by_title(title, library);
    if (library_iterator == library.end())
        throw Title_exception("No record with that title!");
    return library_iterator;
}

// read new title name and check whether the name has already exist.
string read_check_new_title_name(Ordered_by_title_lib_t &library)
{
    string title = read_check_title();
    if (probe_Record_by_title(title, library) != library.end())
        throw Title_exception("Library already has a record with this title!");
    return title;
}

// Read title and check whether its valid.
// throw exception if the title is invalid.
string read_check_title()
{
    string title;
    getline(cin, title);
    trim_title(title);
    if (!title.size())
        throw Title_exception("Could not read a title!");
    return title;
}

// Remove leading or trailing whitespace of the title passed in, and all
// embedded consecutive whitespace characters shrunk to a single
// character (' ').
void trim_title(string &title)
{
    title.erase(unique(title.begin(), title.end(), [](char a, char b)
                       {return isspace(a) && isspace(b);}), title.end());
    replace_if(title.begin(), title.end(),
               static_cast<int (*)(int)>(isspace), ' ');
    size_t beginning = title.find_first_not_of(' ');
    if (beginning != string::npos)
        title = title.substr(beginning,
                             title.find_last_not_of(' ') + 1 - beginning);
    else
        title = "";
}

Ordered_by_title_lib_t::iterator probe_Record_by_title(string title,
                                            Ordered_by_title_lib_t &library)
{
    Record probe(title);
    return library.find(&probe);
}

Ordered_by_id_lib_t::iterator read_ID_probe_Record(Ordered_by_id_lib_t &library)
{
    int id = read_and_check_integer();
    return probe_Record_by_id(id, library);
}

// Read in an integer and check whether it succeeds.
// throw exceptoin if read fails
int read_and_check_integer()
{
    int read_input;
    if (!(cin >> read_input))
        throw Error("Could not read an integer value!");
    return read_input;
}

// probe the library to find the iterator according to the input id
Ordered_by_id_lib_t::iterator probe_Record_by_id(int id,
                                                 Ordered_by_id_lib_t &library)
{
    auto library_iterator = lower_bound(library.begin(), library.end(), id,
                                        compare_record_with_id);
    if (library_iterator != library.end() && (*library_iterator)->get_ID() ==id)
        return library_iterator;
    else
        throw Error("No record with that ID!");
}

Catalog_t::iterator read_check_collection_name(Catalog_t &catalog)
{
    string collection_name;
    auto catalog_iterator = find_collection_iterator(catalog, collection_name);
    if (catalog_iterator != catalog.end() && catalog_iterator->get_name() ==
        collection_name)
        return catalog_iterator;
    else
        throw Error("No collection with that name!");
}

// Read new collection. Throw exception if collection already exists.
Catalog_t::iterator read_check_new_collection_name(Catalog_t &catalog,
                                                   string &new_collection_name)
{
    auto catalog_iterator = find_collection_iterator(catalog,
                                                     new_collection_name);
    if (catalog_iterator != catalog.end() && catalog_iterator->get_name() ==
        new_collection_name)
        throw Error("Catalog already has a collection with this name!");
    return catalog_iterator;
}

Catalog_t::iterator find_collection_iterator(Catalog_t &catalog,
                                             string &collection_name)
{
    cin >> collection_name;
    Collection probe(collection_name);
    auto catalog_iterator = lower_bound(catalog.begin(), catalog.end(), probe);
    return catalog_iterator;
}

void free_all_records(Ordered_by_title_lib_t &library)
{
    for_each(library.begin(), library.end(), [](Record *record_ptr)
                                                    {free(record_ptr);});
}

void insert_new_Record(Database_t &database, Record *new_record)
{
    database.library_ordered_by_title.insert(new_record);
    database.library_ordered_by_id.insert(
        lower_bound(database.library_ordered_by_id.begin(),
                    database.library_ordered_by_id.end(),
                    new_record->get_ID(), compare_record_with_id), new_record);
}

bool compare_record_with_id(Record *record_ptr, int id)
{
    return record_ptr->get_ID() < id;
}

// Read to new line
void discard_input_remainder(void)
{
    cin.clear();
    while (cin.get() != '\n')
        ;
}
