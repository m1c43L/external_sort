#include <iostream>
#include <ostream>
#include <fstream>
#include <array>
#include <string>
#include <stack>
#include <algorithm>
#include <exception>
#include <stdio.h>
#include <string.h>
#include <list>

using namespace std;

#define PAGE_SIZE 4096          // 4kb
#define MAX_DIGIT 3             // # of max digit - 100 being max value
#define TEMP_FILE_NAME "temp"   // default temporary filename
#define FINAL_OUTPUT_FILE_NAME "sorted_age.txt" // final output filename



/* Function declarations */

stack<string> partition_file_to(ifstream &, int );

void merge(stack <string> &, stack <string> &);

void merge_write_to_stream(ifstream &, ifstream &, ofstream &);

bool parse(ifstream &, list <short int> & buffer, int);

void print_debug(int pass_id, int buffer_count, int temp_files_count);

bool is_buffer_full(list<short> &buffer);

void flush_output_buffer(ofstream &out_stream);


    int file_no = 0; // incremented upon creation of new file
    int pass_count = 0; // incremented upon each pass of merge.

list<short int> output_buffer;
list<short int> input_buffer_1;
list<short int> input_buffer_2;


int main(int argc, char * argv[])
{

    // 2 static stack for input and output.
    stack<string> temp_file_names;
    stack<string> temp_file_names_2;

    ifstream source(argv[argc - 1], ifstream::in);

    cout << "Sorting "<< argv[argc - 1] << " ...." << endl;

    temp_file_names = partition_file_to(source, PAGE_SIZE);

    print_debug(pass_count, 1, temp_file_names.size());
    cout << argv[argc - 1] << " is split and sorted to " << temp_file_names.size() << " files with " << PAGE_SIZE << " bytes size each." << endl << endl;

    

    cout << "merging files ...." << endl << endl;
    merge(temp_file_names, temp_file_names_2);

    cout << temp_file_names.size() << endl;
    cout << temp_file_names_2.size() << endl;

    return 0;
}


void print_debug(int pass_id, int buffer_count, int temp_files_count){
    cout << "( " << pass_id << ", " << buffer_count << ", " << temp_files_count << " )" << endl;
}

// generate a semi-random filename
string get_new_filename(){
    string u("_");
    return TEMP_FILE_NAME + u + "pass" + u + to_string(pass_count) + u + to_string(file_no ++);
}


/* merge files */
void merge( stack <string> & input_files, stack <string> & output_files ){

    pass_count++;

    // the last file in the stack is the sorted output file
    if(input_files.size() == 1){
        rename(input_files.top().c_str(), FINAL_OUTPUT_FILE_NAME);
        cout << "\nsorting done. \nsorted data saved to: " << FINAL_OUTPUT_FILE_NAME << endl;
        input_files.pop();
        return;
    }

    string  output_fname;
   
    while( input_files.size() > 1){

        ofstream output;
        output_fname = get_new_filename();
        output.open(output_fname, ofstream::out | ofstream::trunc);

        ifstream input_1(input_files.top());
        input_files.pop();
        ifstream input_2(input_files.top());
        input_files.pop();

        merge_write_to_stream(input_1, input_2, output);

        output.close();
        output_files.push(output_fname);
    
    }

    if (input_files.size() == 1){
        output_files.push( input_files.top() );
        input_files.pop();
    }

    print_debug(pass_count, 3,output_files.size());

    merge(output_files, input_files); // recursively call merge
}



/* parse file content into int vector.*/
/* NOTE: content is limited by the size of page. */
/* an integer vector is returned that contains */
bool parse(ifstream &input, list <short> & buffer, int page_size){


    int character_count = 0;

    // input character placeholder
    char c;

    string string_buffer("");

    // reads character bytes per bytes.
    while(true){

        input.get(c);
        character_count ++;

        // when reading has reached either EOF or Page limit, break out of this loop
        if (input.eof()  || (c == ',' && character_count >= (PAGE_SIZE - MAX_DIGIT)) ) 
        {
            if (!string_buffer.empty()){
                buffer.push_back(stoi(string_buffer));
            }

            break;

        // integer is added to buffer.
        }else if(c == ','){ 
            if (!string_buffer.empty()){
                buffer.push_back(stoi(string_buffer));
                string_buffer.clear();
            }

        // filter numerical character
        }else if(c >= 48 && c <= 57 ){ 

            string_buffer.push_back(c);

        }else{
            cout << "Input Error: invalid input character" << endl;
        }
    }

    return !buffer.empty();

}

// merge vector buffer1 and vector buffer 2 and write the sorted output to stream.
void merge_write_to_stream(ifstream & input1, ifstream & input2, ofstream & output){
 
    // parameters: ifstream, vector buffer, int size
    parse(input1, input_buffer_1, PAGE_SIZE);
    parse(input2, input_buffer_2, PAGE_SIZE);

    while (true)
    {

        if ( input_buffer_1.empty() )
            parse(input1 ,input_buffer_1 ,PAGE_SIZE);
        
        if (input_buffer_2.empty())
            parse(input2, input_buffer_2, PAGE_SIZE);
        
        if (input_buffer_1.empty() || input_buffer_2.empty())
            break;

        // merge logic
        if ( input_buffer_1.front() <= input_buffer_2.front() )
        {
            output_buffer.push_back(input_buffer_1.front());
            input_buffer_1.pop_front();
        }
        else
        {
            output_buffer.push_back(input_buffer_2.front());
            input_buffer_2.pop_front();
        }

        // flush buffer when empty
        if(is_buffer_full(output_buffer))
            flush_output_buffer(output);
        

    }

        // process the rest of the input file 1
        while ( true )
        {

            if ( input_buffer_1.empty() )
            {
                bool file_done = parse(input1, input_buffer_1, PAGE_SIZE);

                if( !file_done){
                    break;
                }
            }

            output_buffer.push_back(input_buffer_1.back());
            input_buffer_1.pop_back();

            if (is_buffer_full(output_buffer))
            {
                flush_output_buffer(output);
            }
        }

        // process the rest of the input file 2
        while ( true )
        {

            if (input_buffer_2.empty())
            {
                bool file_done = parse(input2, input_buffer_2, PAGE_SIZE);

                if (!file_done)
                {
                    break;
                }
            }

            output_buffer.push_back(input_buffer_2.front());
            input_buffer_2.pop_front();

            if (is_buffer_full(output_buffer))
            {
                flush_output_buffer(output);
            }
        }

        flush_output_buffer(output);
}


bool is_buffer_full( list <short> & buffer){
    return buffer.size()  == (PAGE_SIZE / sizeof(short int)) - 1;
}


void flush_output_buffer(ofstream & out_stream){

    while (!output_buffer.empty())
    {
        out_stream << output_buffer.front() << ',';
        output_buffer.pop_front();
    }

    out_stream.flush();
}


/* writes the buffer to output stream*/
void write_buffer_to_file(list <short> & buffer, ofstream & out_stream){

    while( !buffer.empty() ){
        out_stream << buffer.front() << ',';
        buffer.pop_front();
    }

    out_stream.flush();
}



stack<string> partition_file_to(ifstream &source, int bytes_count )
{

    string filename(""); 
   // ofstream output("temp_ ", ofstream::out | ofstream::tr);
    list<short> data;
    stack <string> temp_file_names;


    // parse file data to int and store int a buffer vector
    // sort the buffer vector
    // generate a uniqe file and write the buffer to the unique file.
    // Note: see parse and get_new_filename implementation for more details.
    parse(source, data, bytes_count);
    while (! data.empty() ){
        data.sort();
        filename = get_new_filename();    
        ofstream output(filename, ofstream::out | ofstream::trunc);
        write_buffer_to_file(data, output);
        output.close();
        parse(source, data, PAGE_SIZE);
        temp_file_names.push(filename);
    }

    return temp_file_names;

}