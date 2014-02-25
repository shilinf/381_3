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
#include <unordered_set>
#include <algorithm>
#include <iterator>
#include <functional>
#include <new>

using std::string;
using std::cin; using std::cout; using std::endl;
using std::ifstream; using std::ofstream;
using std::isspace;
using std::make_pair;
using std::set; using std::list; using std::vector; using std::map; using std::unordered_set;
using std::find_if; using std::lower_bound; using std::for_each; using std::transform; using std::unique_copy; using std::stable_sort;
using std::ostream_iterator;
using namespace::std::placeholders;
using std::bind; using std::mem_fn; using std::ref;


// compare two records by ID
struct Compare_Record_ID {
    bool operator() (const Record* p1, const Record* p2) const
        {return p1->get_ID() < p2->get_ID();}
};

using Ordered_by_title_lib_t = set<Record *, Less_than_ptr<Record *>>;
using Ordered_by_id_lib_t = vector<Record *>;
using Catalog_t = list<Collection>;

struct Database_t {
    Catalog_t catalog;
    Ordered_by_title_lib_t library_ordered_by_title;
    Ordered_by_id_lib_t library_ordered_by_id;
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



// function parameter for OC_apply_if_arg
bool check_record_in_Collection (Collection collection, Record *arg_ptr);

// function parameter for apply_if
bool check_Collection_empty (Collection collection);

// function parameter for apply
void free_Record(Record *record_ptr);

// helper functions
void handle_invalid_command(void);
void discard_input_remainder(void);
Catalog_t::iterator find_collection_iterator(Catalog_t &catalog);
Ordered_by_id_lib_t::iterator probe_Record_by_id(int id, Ordered_by_id_lib_t &library);
int read_and_check_integer();
string read_check_title();
void trim_title(string &title);
void clear_Library_helper(Database_t &database);
void clear_all_data_helper(Database_t &database);
Ordered_by_id_lib_t::iterator read_ID_probe_Record(Ordered_by_id_lib_t &library);
//Ordered_by_title_lib_t::iterator read_title_check_Record_exist(Ordered_by_title_lib_t &library);
Ordered_by_title_lib_t::iterator read_title_probe_Record(Ordered_by_title_lib_t &library);

Ordered_by_title_lib_t::iterator probe_Record_by_title(string title, Ordered_by_title_lib_t &library);

bool compare_record_with_id(Record *record_ptr, int id);

string read_check_new_collection_name(Database_t &database);

//void print_Record_helper(Record *record);

int main ()
{
    Database_t database;
    map<string, void(*)(Database_t &)> commands_map;
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
    while (true) {
        char action, object;
        cout << endl << "Enter command: ";
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
            else if (commands_map.find(command) == commands_map.end()) {
                handle_invalid_command();
            }
            else {
                commands_map[command](database);
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

// Print error message for invalid comamnd and read to new line
void handle_invalid_command(void)
{
    cout << "Unrecognized command!" << endl;
    discard_input_remainder();
}

// Read to new line
void discard_input_remainder(void)
{
    cin.clear();
    while (cin.get() != '\n')
        ;
}

// find and print the specified record with the matching title
void find_Record_match_title(Database_t &database)
{
    /*string title = read_check_title();
    Record probe(title);
    auto library_iterator = database.library_ordered_by_title.find(&probe);*/
    
    
    auto library_iterator = read_title_probe_Record(database.library_ordered_by_title);
    cout << **library_iterator << endl;

}

// print the specified record with the matching ID number
void print_Record_match_id(Database_t &database)
{
    //int id_input = read_and_check_integer();
    //auto library_iterator = probe_Record_by_id(id_input, database.library_ordered_by_id);
    auto library_iterator = read_ID_probe_Record(database.library_ordered_by_id);
    cout << **library_iterator << endl;
    
}

Ordered_by_id_lib_t::iterator read_ID_probe_Record(Ordered_by_id_lib_t &library)
{
    int id = read_and_check_integer();
    return probe_Record_by_id(id, library);
}


// print collection - print each record in the collection with the
// specified name
void print_Collection_match_name(Database_t &database)
{
    Catalog_t::iterator catalog_iterator = find_collection_iterator(database.catalog);
    cout << *catalog_iterator;
}

// print all the records in the Library
void print_Records(Database_t &database)
{
    if (database.library_ordered_by_title.empty())
        cout << "Library is empty" << endl;
    else {
        cout << "Library contains " << database.library_ordered_by_title.size() << " records:" <<endl;
        //for (auto record_ptr : database.library_ordered_by_title)
        //    cout << *record_ptr <<endl;
        for_each(database.library_ordered_by_title.begin(), database.library_ordered_by_title.end(), print_Record_helper);
    }
}


// print the Catalog - print all the collections in the Catalog
void print_Catalog(Database_t &database)
{
    if (database.catalog.empty())
        cout << "Catalog is empty" <<endl;
    else {
        cout << "Catalog contains " << database.catalog.size() << " collections:" <<endl;
        //for(auto iterator=database.catalog.begin(); iterator!=database.catalog.end(); ++iterator)
        //    cout << *iterator;
        
        copy(database.catalog.begin(), database.catalog.end(), ostream_iterator<Collection &>(cout));
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
    string title = read_check_title();
    //Record probe(title);
    //auto library_iterator = database.library_ordered_by_title.find(&probe);
    auto library_iterator = probe_Record_by_title(title, database.library_ordered_by_title);
    if (library_iterator != database.library_ordered_by_title.end()) {
        throw Title_exception("Library already has a record with this title!");
    }
    else {
        Record *new_record = new Record(medium, title);
        database.library_ordered_by_title.insert(new_record);
        database.library_ordered_by_id.insert(lower_bound(database.library_ordered_by_id.begin(), database.library_ordered_by_id.end(), new_record->get_ID(), compare_record_with_id), new_record);
        cout << "Record " << new_record->get_ID() << " added" << endl;
    }
}

bool compare_record_with_id(Record *record_ptr, int id)
{
    return record_ptr->get_ID() < id;
}

// add a collection with the specified name
void add_Collection(Database_t &database)
{
    string collection_name = read_check_new_collection_name(database);
    auto insert_position = find_if(database.catalog.begin(), database.catalog.end(), [&collection_name](Collection &collection){return collection_name < collection.get_name();});
    database.catalog.insert(insert_position, Collection(collection_name));
    cout << "Collection " << collection_name << " added" <<endl;
}

string read_check_new_collection_name(Database_t &database)
{
    string collection_name;
    cin >> collection_name;
    auto catalog_iterator = find_if(database.catalog.begin(), database.catalog.end(), [&collection_name](Collection &collection){return collection_name == collection.get_name();});
    if (catalog_iterator != database.catalog.end())
        throw Error("Catalog already has a collection with this name!");
    return collection_name;
}




// add a record to a specified collection
void add_Record_to_Collection(Database_t &database)
{
    Catalog_t::iterator catalog_iterator = find_collection_iterator(database.catalog);
    //int id_input = read_and_check_integer();
    //Ordered_by_id_lib_t::iterator library_iterator = probe_Record_by_id(id_input, database.library_ordered_by_id);
    Ordered_by_id_lib_t::iterator library_iterator = read_ID_probe_Record(database.library_ordered_by_id);
    catalog_iterator->add_member(*library_iterator);
    cout << "Member " << (*library_iterator)->get_ID() << " " << (*library_iterator)->get_title()
    <<" added" << endl;
}

// modify the rating of the specified record with the matching ID number
void modify_Record_rating(Database_t &database)
{
    //int id_input = read_and_check_integer();
    //Ordered_by_id_lib_t::iterator library_iterator = probe_Record_by_id(id_input, database.library_ordered_by_id);
    Ordered_by_id_lib_t::iterator library_iterator = read_ID_probe_Record(database.library_ordered_by_id);
    int rating_input = read_and_check_integer();
    (*library_iterator)->set_rating(rating_input);
    cout << "Rating for record " << (*library_iterator)->get_ID() << " changed to "
    << rating_input << endl;
}

// delete the specified record from the Library
void delete_Record_from_Library(Database_t &database)
{
    //string title = read_check_title();
    //Record probe(title);
    auto library_title_iterator = read_title_probe_Record(database.library_ordered_by_title);
    //if (library_title_iterator == database.library_ordered_by_title.end())
    //    throw Title_exception("No record with that title!");
    //if (apply_if_arg(database.catalog.begin(), database.catalog.end(), check_record_in_Collection,
    //                 *library_title_iterator))
        
        
    if (find_if(database.catalog.begin(), database.catalog.end(), bind(&Collection::is_member_present, _1, *library_title_iterator)) != database.catalog.end())
        throw Title_exception("Cannot delete a record that is a member of a collection!");
    int record_ID = (*library_title_iterator)->get_ID();
    Record *delete_pointer = * library_title_iterator;
    database.library_ordered_by_title.erase(library_title_iterator);
    database.library_ordered_by_id.erase(probe_Record_by_id(record_ID, database.library_ordered_by_id));
    delete delete_pointer;
    cout << "Record " << record_ID << " " << (*library_title_iterator)->get_title() << " deleted" << endl;
}

/*
bool check_record_in_Collection (Collection collection, Record *arg_ptr)
{
    return collection.is_member_present(arg_ptr);
}*/

// delete the specified collection from the Catalog
void delete_Collection_from_Catalog(Database_t &database)
{
    Catalog_t::iterator catalog_iterator = find_collection_iterator(database.catalog);
    string collection_name = catalog_iterator->get_name();
    database.catalog.erase(catalog_iterator);
    cout << "Collection " << collection_name << " deleted" << endl;
}

// delete the specified record as member of the a specified collection
void delete_Record_from_Collection(Database_t &database)
{
    Catalog_t::iterator catalog_iterator = find_collection_iterator(database.catalog);
    //int id_input = read_and_check_integer();
    //Ordered_by_id_lib_t::iterator library_iterator = probe_Record_by_id(id_input, database.library_ordered_by_id);
    Ordered_by_id_lib_t::iterator library_iterator = read_ID_probe_Record(database.library_ordered_by_id);
    catalog_iterator->remove_member(*library_iterator);
    cout << "Member " << (*library_iterator)->get_ID() << " " << (*library_iterator)->get_title()
    <<" deleted" << endl;
}

void clear_Library_helper(Database_t &database)
{
    //if (apply_if(database.catalog.begin(), database.catalog.end(), check_Collection_empty))
        
        
        
    if (find_if_not(database.catalog.begin(), database.catalog.end(), mem_fn(&Collection::empty)) != database.catalog.end())
        throw Error("Cannot clear all records unless all collections are empty!");
    //apply(database.library_ordered_by_title.begin(), database.library_ordered_by_title.end(), free_Record);
    
    for_each(database.library_ordered_by_title.begin(), database.library_ordered_by_title.end(), [](Record *record_ptr){free(record_ptr);});
    
    database.library_ordered_by_title.clear();
    database.library_ordered_by_id.clear();
    Record::reset_ID_counter();
}

// Clear the Library. If collections are not empty, throw exception
void clear_Library(Database_t &database)
{
    clear_Library_helper(database);
    cout <<"All records deleted" <<endl;
}

bool check_Collection_empty (Collection collection)
{
    return !collection.empty();
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
    //for (auto record_ptr : database.library_ordered_by_title)
    //    record_ptr->save(output_file);
    
    for_each(database.library_ordered_by_title.begin(), database.library_ordered_by_title.end(), [&output_file](Record *record_ptr){record_ptr->save(output_file);});
    output_file << database.catalog.size() << endl;
    //for (auto iterator = database.catalog.begin(); iterator != database.catalog.end(); ++iterator)
    //    iterator->save(output_file);
    for_each(database.catalog.begin(), database.catalog.end(), bind(&Collection::save, _1, ref(output_file)));
    
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
    Catalog_t local_catalog(database.catalog);
    Ordered_by_title_lib_t local_library_ordered_by_title(database.library_ordered_by_title);
    Ordered_by_id_lib_t local_library_ordered_by_id(database.library_ordered_by_id);
    database.catalog.clear();
    database.library_ordered_by_id.clear();
    database.library_ordered_by_title.clear();
    Record::save_ID_counter();
    Record::reset_ID_counter();
    try {
        int num_record;
        if (!(input_file >> num_record))
            throw Error("Invalid data found in file!");
        for (int i=0; i < num_record; i++) {
            Record *new_record = new Record(input_file);
            //insert considering duplicate with add record
            
            database.library_ordered_by_title.insert(new_record);
            database.library_ordered_by_id.insert(lower_bound(database.library_ordered_by_id.begin(), database.library_ordered_by_id.end(), new_record->get_ID(), compare_record_with_id), new_record);
        }
        int num_collection;
        if (!(input_file >> num_collection))
            throw Error("Invalid data found in file!");
        for (int i = 0; i < num_collection; i++) {
            Collection new_collection(input_file, database.library_ordered_by_title);
            database.catalog.push_back(new_collection);
        }
        input_file.close();
        //apply(local_library_ordered_by_title.begin(),
              //local_library_ordered_by_title.end(), free_Record);
        for_each(local_library_ordered_by_title.begin(), local_library_ordered_by_title.end(), [](Record *record_ptr){free(record_ptr);});

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

void free_Record(Record *record_ptr)
{
    delete record_ptr;
}

// clear all data (as in cA), and also destroy the Library and Catalog
// containers themselves, so that all memory is deallocated, and then terminate
void quit(Database_t &database)
{
    clear_all_data_helper(database);
    cout << "All data deleted" << endl;
    cout << "Done" << endl;
}

// Find the iterator from catalog with the specified collection name.
// throw exception if the collection doesn't exist
Catalog_t::iterator find_collection_iterator(Catalog_t &catalog)
{
    string collection_name;
    cin >> collection_name;
    //Collection find_temp(collection_name);
    //catalog_iterator = catalog.find(find_temp);
    auto catalog_iterator = find_if(catalog.begin(), catalog.end(), [&collection_name](Collection &collection){return collection_name == collection.get_name();});
    if (catalog_iterator == catalog.end())
        throw Error("No collection with that name!");
    return catalog_iterator;
}

// probe the library to find the iterator according to the input id
Ordered_by_id_lib_t::iterator probe_Record_by_id(int id, Ordered_by_id_lib_t &library)
{
    auto library_iterator = lower_bound(library.begin(), library.end(), id, compare_record_with_id);
    if (library_iterator != library.end() && (*library_iterator)->get_ID() == id)
        return library_iterator;
    else
        throw Error("No record with that ID!");
}

Ordered_by_title_lib_t::iterator probe_Record_by_title(string title, Ordered_by_title_lib_t &library)
{
    Record probe(title);
    return library.find(&probe);
}


Ordered_by_title_lib_t::iterator read_title_probe_Record(Ordered_by_title_lib_t &library)
{
    string title = read_check_title();
    //Record probe(title);
    //auto library_iterator = library.find(&probe);
    auto library_iterator = probe_Record_by_title(title, library);
    if (library_iterator == library.end())
        throw Title_exception("No record with that title!");
    return library_iterator;
}
/*
Ordered_by_title_lib_t::iterator read_title_check_Record_exist(Ordered_by_title_lib_t &library)
{
    auto library_iterator = read_title_probe_Record(library);
    if (library_iterator == library.end())
        throw Title_exception("No record with that title!");
    return library_iterator;
}*/



// Read in an integer and check whether it succeeds.
// throw exceptoin if read fails
int read_and_check_integer()
{
    int read_input;
    if (!(cin >> read_input))
        throw Error("Could not read an integer value!");
    return read_input;
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
// character (' '). Return 0 if the tile is empty after trim, 1 if not
void trim_title(string &title)
{
    transform(title.begin(), title.end(), title.begin(), [](char character){if(isspace(character))
                                                                                return ' ';
                                                                            else
                                                                                return character;});
    
        

    string trimmed_string(title.size(), ' ');
    unique_copy(title.begin(), title.end(), trimmed_string.begin(), [](char &a, char &b) {return a == b && a == ' ';});
    size_t beginning = trimmed_string.find_first_not_of(' ');
    if (beginning != string::npos)
        title = trimmed_string.substr(beginning, trimmed_string.find_last_not_of(' ') + 1 - beginning);
    
    
    
    
    
    /*
    int valid = 0, i = 0;
    while (i < title.size()) {
        while (i < title.size() && isspace(title[i]))
            title.erase(i, 1);
        while (i < title.size() && !isspace(title[i])) {
            ++i;
            valid = 1;
        }
         If title doesn't touch '\0', add one space to end of current string
        if (i < title.size())
            title[i++] = ' ';
    }
     If the last character before '\0' is space and there exists non-sapce
     character in the string, put the '\0' one position before to
     occupy the trailing whitespace.
    if (valid && isspace(title[title.size()-1]))
        title.erase(title.size()-1, 1);
    return valid;*/
}


void find_with_string(Database_t &database)
{
    string key_word;
    cin >> key_word;
    bool no_record_match = true;
    for_each(key_word.begin(), key_word.end(), tolower); // whether it change the original string???
    
    transform(key_word.begin(), key_word.end(), key_word.begin(), tolower);
    
    //for_each(database.library_ordered_by_title.begin(), database.library_ordered_by_title.end(), )
    
    //cout << key_word <<endl;
    //considering put compare into Record class
    for(Record *record_ptr: database.library_ordered_by_title) {
        string lower_title = record_ptr->get_title();
        transform(lower_title.begin(), lower_title.end(), lower_title.begin(), tolower);
        if (lower_title.find(key_word) != string::npos) {
            no_record_match = false;
            print_Record_helper(record_ptr);
        }
    }
    if (no_record_match)
        throw Error("No records contain that string!");
}

void list_ratings(Database_t &database)
{
    if (database.library_ordered_by_title.empty())
        cout << "Library is empty" << endl;
    else {
        size_t library_size = database.library_ordered_by_title.size();
        vector<Record *> library_ordered_by_rate(library_size);
        copy(database.library_ordered_by_title.begin(), database.library_ordered_by_title.end(),library_ordered_by_rate.begin());
        stable_sort(library_ordered_by_rate.begin(), library_ordered_by_rate.end(), [](Record* r1, Record *r2){return r1->get_rate() > r2->get_rate();});
        //cout << "Library contains " << library_size << " records:" <<endl;
        for_each(library_ordered_by_rate.begin(), library_ordered_by_rate.end(), print_Record_helper);
    }
}


void collection_statistics(Database_t &database)
{
    Collection_Statist statist;
    for_each(database.catalog.begin(), database.catalog.end(), bind(&Collection::get_collection_statist, _1, ref(statist)));
    size_t library_size = database.library_ordered_by_title.size();
    cout << statist.get_num_records_in_collection() <<" out of "<< library_size <<" Records appear in at least one Collection" << endl;
    cout << statist.get_num_frequent_records() <<" out of " << library_size <<" Records appear in more than one Collection" << endl;
    cout << "Collections contain a total of "<< statist.get_total_occurance() <<" Records" << endl;
}



void combine_collections(Database_t &database)
{
    Catalog_t::iterator catalog_iterator1 = find_collection_iterator(database.catalog);
    Catalog_t::iterator catalog_iterator2 = find_collection_iterator(database.catalog);
    string new_collection_name = read_check_new_collection_name(database);
    
    
    auto insert_position = find_if(database.catalog.begin(), database.catalog.end(), [&new_collection_name](Collection &collection){return new_collection_name < collection.get_name();});
    database.catalog.insert(insert_position, Collection(*catalog_iterator1, *catalog_iterator2, new_collection_name));
    cout << "Collections "<< catalog_iterator1->get_name() <<" and "<< catalog_iterator2->get_name() <<" combined into new collection "<< new_collection_name << endl;

}

void modify_title(Database_t &database)
{
    auto library_id_iterator = read_ID_probe_Record(database.library_ordered_by_id);
    string new_title = read_check_title();
    if (probe_Record_by_title(new_title, database.library_ordered_by_title) != database.library_ordered_by_title.end())
        throw Title_exception("Library already has a record with this title!");

    Record *old_record = *library_id_iterator;
    Record *new_record = new Record(**library_id_iterator, new_title);
    auto library_title_iterator = probe_Record_by_title(old_record->get_title(), database.library_ordered_by_title);
    
    
    
    database.library_ordered_by_title.erase(library_title_iterator);
    database.library_ordered_by_title.insert(new_record);
    database.library_ordered_by_id.erase(library_id_iterator);
    database.library_ordered_by_id.insert(lower_bound(database.library_ordered_by_id.begin(), database.library_ordered_by_id.end(), new_record->get_ID(), compare_record_with_id), new_record);
    
    for_each(database.catalog.begin(), database.catalog.end(), bind(&Collection::modify_title, _1, ref(old_record), ref(new_record)));
    
    cout <<"Title for record "<<old_record->get_ID()<< " changed to " << new_title <<endl;
}




