#include <iostream>
#include <ostream>
#include <fstream>
#include <array>
#include <string>
#include <vector>
#include <stack>
#include <algorithm>
#include <exception>

using namespace std;

#define PAGE_SIZE 4096          // 4kb
#define MAX_DIGIT 3             // # of max digit - 100 being max value
#define TEMP_FILE_NAME "temp"   // default temporary filename



stack<string> partition_file_to(ifstream &, int );
void merge(stack <string> &, stack <string> &);
void merge_write_to_stream(ifstream &, ifstream &, ofstream &);
vector<int> parse(ifstream &, int);
void print_debug(int pass_id, int buffer_count, int temp_files_count);



int file_no = 0; // incremented upon creation of new file
int pass_count = 1;



int main(int argc, char * argv[])
{

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
    return TEMP_FILE_NAME + u + to_string(pass_count) + u + to_string(file_no ++);
}


/* merge files */
void merge( stack <string> & input_files, stack <string> & output_files ){

    pass_count++;

    // the last file in the stack is the sorted output file
    if(input_files.size() == 1){
        cout << "\nsorting done. \nsorted data saved to: " << input_files.top() << endl;
        input_files.pop();
        return;
    }

    string  output_fname;
   
    while( input_files.size() > 1){

    
        //string output_fname = generate_filename();
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
vector <int> parse(ifstream &input, int page_size){

    int count = 0;

    vector <int> buffer;
    buffer.reserve(page_size);

    char c;
    string string_buffer("");

    while(true){

        input.get(c);
        count ++;

        if (input.eof()  || (c == ',' && count >= (PAGE_SIZE - MAX_DIGIT)) ) 
        {
            if (!string_buffer.empty()){
                buffer.push_back(stoi(string_buffer));
            }
            break;

        }else if(c == ','){

            if (!string_buffer.empty()){
                buffer.push_back(stoi(string_buffer));
            }

            string_buffer.clear();

        }else if(c >= 48 && c <= 57 ){

            string_buffer.push_back(c);

        }
    }

    return buffer;
}

// merge vector buffer1 and vector buffer 2 and write the sorted output to stream.
void merge_write_to_stream(ifstream & input1, ifstream & input2, ofstream & output){

    int b1_i = 0, b2_i = 0;

    vector<int> buffer_in_1 = parse(input1, PAGE_SIZE);
    vector<int> buffer_in_2 = parse(input2, PAGE_SIZE);
    
    while (true)
    {

        if (b1_i >= buffer_in_1.size())
        {
            buffer_in_1 = parse(input1, PAGE_SIZE);
            b1_i = 0;
        }

        if (b2_i >= buffer_in_2.size())
        {
            buffer_in_2 = parse(input2, PAGE_SIZE);
            b2_i = 0;
        }

        if (buffer_in_1.empty() || buffer_in_2.empty())
        {
            break;
        }

        if (buffer_in_1[b1_i] <= buffer_in_2[b2_i])
        {
            output << buffer_in_1[b1_i] << ',';
            b1_i++;
        }
        else
        {
            output << buffer_in_2[b2_i] << ',';
            b2_i++;
        }
        }

        // write the rest of buffer1 into the stream
        while ( true )
        {

            if (b1_i >= buffer_in_1.size())
            {
                buffer_in_1 = parse(input1, PAGE_SIZE);
                b1_i = 0;
                
            }

            if ( buffer_in_1.empty() )
            {
                break;
            }

            output << buffer_in_1[b1_i] << ',';
            b1_i++;
        }

        // write the rest of buffer2 into the stream
        while ( true )
        {

            if (b2_i >= buffer_in_2.size())
            {
                buffer_in_2 = parse(input2, PAGE_SIZE);
                b2_i = 0;
            }

            if( buffer_in_2.empty() ){
                break;
            }

            output << buffer_in_2[b2_i] << ',';
            b2_i++;
        }

        output.flush();
}




/* writes the buffer to output stream*/
void write_buffer_to_file(vector <int> buffer, ofstream & out_stream){

    for(int i = 0; i < buffer.size(); i++){
        out_stream << buffer[i] << ',' ;
    } 

}

stack<string> partition_file_to(ifstream &source, int bytes_count )
{

    string filename(""); 
   // ofstream output("temp_ ", ofstream::out | ofstream::tr);
    vector<int> data;
    stack <string> temp_file_names;


    // parse file data to int and store int a buffer vector
    // sort the buffer vector
    // generate a uniqe file and write the buffer to the unique file.
    // Note: see parse and get_new_filename implementation for more details.
    data = parse(source, bytes_count);
    while (! data.empty() ){
        sort(data.begin(), data.end());
        filename = get_new_filename();    
        ofstream output(filename, ofstream::out | ofstream::trunc);
        write_buffer_to_file(data, output);
        output.close();
        data = parse(source, PAGE_SIZE);
        temp_file_names.push(filename);
    }

    return temp_file_names;

}