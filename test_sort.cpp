#include <limits>
#include <ostream>
#include <fstream>
#include <algorithm>
#include <vector>

using namespace std;

vector<int> parse(ifstream &input, int page_size);
void write_buffer_to_file(vector<int> buffer, ofstream &out_stream);

    int main(int argc, char **argv)
{

    ifstream input(argv[1], ifstream::in);
    ofstream output(argv[2], ofstream::out | ofstream::trunc);

    vector <int> data = parse(input, numeric_limits<int>::max());

    sort(data.begin(), data.end());

    write_buffer_to_file(data, output);

}

/* writes the buffer to output stream*/
void write_buffer_to_file(vector<int> buffer, ofstream &out_stream)
{

    for (int i = 0; i < buffer.size(); i++)
    {
        out_stream << buffer[i] << ',';
    }
}

/* parse file content into int vector.*/
/* NOTE: content is limited by the size of page. */
vector<int> parse(ifstream &input, int page_size)
{

    int count = 0;

    vector<int> buffer;
    buffer.reserve(page_size);

    char c;
    string string_buffer("");

    while (true)
    {

        input.get(c);
        count++;

        if (input.eof() )
        {
            if (!string_buffer.empty())
            {
                buffer.push_back(stoi(string_buffer));
            }
            break;
        }
        else if (c == ',')
        {

            if (!string_buffer.empty())
            {
                buffer.push_back(stoi(string_buffer));
            }

            string_buffer.clear();
        }
        else if (c >= 48 && c <= 57)
        {

            string_buffer.push_back(c);
        }
    }

    return buffer;
}