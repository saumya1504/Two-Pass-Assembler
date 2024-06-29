/*****************************************************************************
Title: Two Pass Assembler
Author: Saumya Sinha
Roll Number: 2201CS65
Declaration of Authorship
This cpp file, asm.cpp, is part of the assignment of CS209/CS210 at the
department of Computer Science and Engineering, IIT Patna.
*****************************************************************************/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <bitset>
#include <algorithm>
#define endl "\n"


using namespace std;

bool extra_operand = 0;
int program_counter = 0;
int max_line = 0;
int halt = 0, infinity_loop = 0; 

string opcode, operand, label;
set<string> temporary_label;

map<string, pair<int, int>> instructions;   // Stores mnemonics and their operand type
map<int, string> opcodes, operands, labels; // Stores {PC, labels/opcodes/operands}
map<int, string> original_operands;
map<string, bool> next_statement;   // Indicates if label is without opcode or not
map<string, int> label_values; // Stores valueues of labels to use them as operands

vector<pair<int, int>> not_numerics;
vector<pair<int, int>> not_numerics_for_value;
vector<int> LabelSet; // Stores PC for SET
vector<string> statements;
vector<pair<int, string>> errors;         // Stores different errors
vector<pair<string, string>> final_codes; // Stores the 8-nibble code

void removeWhitespace(std::string &str)
{
    // Iterate through the string and remove whitespace characters
    std::string result;
    for (char ch : str)
    {
        if (!isspace(ch))
        {
            result += ch;
        }
    }
    str = result;
}

void FileRead(string nameOfFile)
{
    ifstream My_File;
    My_File.open(nameOfFile);
    while (My_File)
    {
        string content, temporary;
        getline(My_File, content);
        temporary = content;
        removeWhitespace(temporary);
        if (temporary != "")
        { // eliminating empty statements
            statements.insert(statements.end(), content);
        }
    }
}

pair<int, int> makeInstruction(int opcode, int operandType)
{
    return make_pair(opcode, operandType);
}

long long stringToInteger(const char *str)
{
    long long ans = 0;
    bool negative = 0;
    const char *ptr = str;

    // Check for negative sign
    if (*ptr == '-')
    {
        negative = 1;
        ++ptr;
    }

    // Iterate through characters and calculate integer valueue
    while (*ptr != '\0')
    {
        ans = ans * 10 + (*ptr - '0');
        ++ptr;
    }

    // Apply sign if negative
    if (negative)
    {
        ans = -ans;
    }

    return ans;
}

int isLabelOrNumeric(string k)
{
    char temp[k.size()];
    int i = 0;
    while (i < k.size())
    {
        temp[i] = k[i];
        i++;
    }
    long long number = stringToInteger(temp); // converting string to integer
    if (k != "0" && number == 0)
    { // strtol gives 0 for non-numeric strings
        return 0;
    }
    return 1;
}

void separateInstruction(const string &line,const int line_no)
{
    int last = -1; // initialize last index

    // Find the index of the semicolon
    for (int i = 0; i < line.size(); ++i)
    {
        if (line[i] == ';')
        {
            last = i;
            break;
        }
    }

    // if semicolon is not found, handle appropriately
    if (last == -1)
    {
        // handle error or do something else
    }

    // Rest of the code remains the same
    auto it = line.begin();
    // separating label, operand, and mnemonic
    bool foundColon = 0;
    for (; it != line.end() && it - line.begin() < last; ++it)
    {
        if (*it == ':')
        {
            foundColon = 1;
            break;
        }
    }

    if (foundColon)
    {
        while (*it != ':' && it - line.begin() < last)
        {
            if (*it != ' ')
            {
                label += *it;
            }
            ++it;
        }
        ++it;
    }

    for (; it != line.end() && it - line.begin() < last && (*it) == ' '; ++it)
        ;
    for (; it != line.end() && it - line.begin() < last && (*it) != ' '; ++it)
    {
        opcode += (*it);
    }
    for (; it != line.end() && it - line.begin() < last && (*it) == ' '; ++it)
        ;
    for (; it != line.end() && it - line.begin() < last && (*it) != ' '; ++it)
    {
        operand += (*it);
    }
    for (; it != line.end() && it - line.begin() < last && (*it) == ' '; ++it)
        ;

    if (it != line.end() && it - line.begin() < last)
    {
        extra_operand = 1; // checking for extra operand
    }
    // error handling
    if ((label != "") && temporary_label.find(label) != temporary_label.end())
    {
        errors.insert(errors.end(), {line_no, "Duplicate label definition"});
    }
    else if (label != "" && (label[0] >= '0' && label[0] <= '9'))
    {
        errors.insert(errors.end(), {line_no, "Bogus label name"}); // 9kal:
    }
    if (opcode != "" && instructions.find(opcode) == instructions.end())
    {
        errors.insert(errors.end(), {line_no, "Bogus mnemonic"}); // aab 8
    }

    else if (opcode != "")
    {
        switch (instructions[opcode].second)
        {
        case 0:
            if (!operand.empty())
            {
                errors.insert(errors.end(), {line_no, "Unexpected operand"}); // add 7
            }
            break;
        case 1:
            if (operand.empty())
            {
                errors.insert(errors.end(), {line_no, "Missing operand"});
            }
            else if (!isLabelOrNumeric(operand))
            {
                not_numerics_for_value.insert(not_numerics_for_value.end(), {program_counter, line_no});
                if (opcode == "SET" || opcode == "data")
                {
                    errors.insert(errors.end(), {line_no, "Not a number"});
                }
            }
            break;
        case 2:
            if (operand.empty())
            {
                errors.insert(errors.end(), {line_no, "Missing operand"});
            }
            else if (!isLabelOrNumeric(operand))
            {
                if (operand[0] >= '0' && operand[0] <= '9')
                {
                    errors.insert(errors.end(), {line_no, "Bogus label name"});
                }
                else
                {
                    not_numerics.insert(not_numerics.end(), {program_counter, line_no});
                }
            }
            break;
        default:
            // Handle default case (if needed)
            break;
        }
    }
    if (extra_operand)
        errors.insert(errors.end(), {line_no, "Extra on end of line"}); // ldc 5 6
}

string intToString(int valueue)
{
    stringstream ss;
    ss << valueue;
    return ss.str();
}

// Function to extract a substring from a string without using STL
string extractSubstring(const string &str, int startIndex, int length)
{
    string result;
    // Check if the starting index is within bounds
    if (startIndex >= 0 && startIndex + length <= str.size())
    {
        // Iterate through the characters and append to the result string
        for (int i = startIndex; i < startIndex + length; ++i)
        {
            result += str[i];
        }
    }
    return result;
}

void modifynameOfFile(std::string &file, const std::string &appendString)
{
    auto it = file.find('.');
    if (it != std::string::npos)
    {
        // Erase characters after the '.' character
        file.erase(it);
    }

    // Append the specified string to the modified file name
    file += appendString;
}

void PrintListing(string nameOfFile)
{
    modifynameOfFile(nameOfFile, "l");
    ofstream file;
    file.open(nameOfFile);

    for (int i = 0; i < program_counter; i++)
    {
        string temporay_label = labels[i], temporary_opcode = opcodes[i], temporary_operand = operands[i];

        string pc = "";
        stringstream s;
        s << hex << i;
        pc = s.str(); // converting pc to hexadecimal

        // Append leading zeros to ensure 8 characters
        while (pc.size() < 8)
        {
            pc = "0" + pc;
        }

        if (temporary_opcode == "HALT")
            halt = 1; // checking whether HALT is present or not
        if (temporay_label == original_operands[i] && temporay_label != "")
        {
            infinity_loop = 1; // handling infinite loop -> label: ldc label
        }

        stringstream ss, s1; // for converting opcodes and operands to hexadecimal

        if (temporary_opcode != "SET" && temporary_opcode != "data")
        {
            int ops = instructions[temporary_opcode].first;
            ss << hex << ops;
        }

        char oper[100];
        int j = 0;
        while (j < 100)
        {
            oper[j] = '?';
            ++j;
        }
        auto it = temporary_operand.begin();
        j = 0;
        while (j < temporary_operand.size())
        {
            oper[j] = *it;
            ++it;
            ++j;
        }

        if (!isLabelOrNumeric(temporary_operand))
        { // handling if operand not numeric but a label
            int value = label_values[temporary_operand];
            s1 << hex << value;
        }
        else
        {
            int value = stringToInteger(oper);
            s1 << hex << value;
        }

        string code = "";
        if (temporary_opcode != "SET" && temporary_opcode != "data")
        {
            string handle_neg = s1.str();
            if (s1.str().size() > 6)
            { // handling for negative numbers, removing the first two most significant bits
                string handle_neg = extractSubstring(s1.str(), s1.str().size() - 6, 6);
            }
            if (ss.str().size() != 2)
            {
                code += handle_neg + "0" + ss.str();
            }
            else
            {
                code += handle_neg + ss.str();
            }
        }
        else
        {
            code += s1.str(); // for SET and data, no opcode
        }

        // Append leading zeros to ensure 8 characters
        while (code.size() < 8)
        {
            code = "0" + code;
        }

        if (next_statement[temporay_label])
        {
            file << pc << "          " << temporay_label + ":" << endl;
            file << pc << " " << code << " " << temporary_opcode << " " << original_operands[i] << endl;
        }
        else
        {
            if (temporay_label != "")
                file << pc << " " << code << " " << temporay_label + ":"
                     << " " << temporary_opcode << " " << original_operands[i] << endl;
            else
                file << pc << " " << code << " " << temporary_opcode << " " << original_operands[i] << endl;
        }

        final_codes.push_back({code, temporary_opcode}); // storing codes and mnemonic
    }
    if (!halt)
        file << " : HALT is missing !!" << endl; // if HALT is not present
    if (infinity_loop)
        file << " : infinity_loop LOOP detected !!" << endl;
}

void bubble_sort(vector<pair<int, string>> &arr)
{
    int n = arr.size();
    for (int i = 0; i < n - 1; ++i)
    {
        for (int j = 0; j < n - i - 1; ++j)
        {
            if (arr[j].first > arr[j + 1].first)
            {
                // Swap elements if they are in the wrong order
                pair<int, string> temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

void ErrorPrint(string nameOfFile)
{
    modifynameOfFile(nameOfFile, "log");
    ofstream file;
    file.open(nameOfFile);
    bubble_sort(errors);
    for (int i = 0; i < errors.size(); i++)
    {
        file << "Error on line " << errors[i].first << " : " << errors[i].second << endl;
    }
}

// Function to convert a hexadecimal string to decimal (without STL)
unsigned int hexToDec(const string &hexString)
{
    unsigned int decimalvalueue = 0;
    for (char digit : hexString)
    {
        decimalvalueue *= 16;
        if (digit >= '0' && digit <= '9')
        {
            decimalvalueue += digit - '0';
        }
        else if (digit >= 'A' && digit <= 'F')
        {
            decimalvalueue += 10 + (digit - 'A');
        }
        else if (digit >= 'a' && digit <= 'f')
        {
            decimalvalueue += 10 + (digit - 'a');
        }
        else
        {
            // Invalueid hexadecimal digit encountered
            // Handle error or return appropriate valueue
        }
    }
    return decimalvalueue;
}

// Function to convert an integer to its binary representation (without STL)
string decimalToBinary(unsigned int decimalvalueue)
{
    string binaryString;
    while (decimalvalueue > 0)
    {
        binaryString = (decimalvalueue % 2 == 0 ? "0" : "1") + binaryString;
        decimalvalueue /= 2;
    }
    // Add leading zeros to ensure the binary string is 32 bits long
    while (binaryString.size() < 32)
    {
        binaryString = "0" + binaryString;
    }
    return binaryString;
}

void BinaryPrint(string nameOfFile)
{
    auto it = find(nameOfFile.begin(), nameOfFile.end(), '.');
    int index = distance(nameOfFile.begin(), it) + 1;
    nameOfFile.erase(index);
    nameOfFile += "o";

    ofstream file;
    file.open(nameOfFile, ios::binary);
    for (int i = 0; i < final_codes.size(); ++i)
    {
        unsigned int x = hexToDec(final_codes[i].first); // converting hex to decimal
        string ans = decimalToBinary(x);                 // Using bitset<32>(x).to_string() to get the binary

        // two extra bits for SET and data
        switch (final_codes[i].second[0])
        {
        case 'S':
            ans += "10"; // 10->SET
            break;
        case 'd':
            ans += "01"; // 01->data
            break;
        default:
            ans += "00"; // 00->no set/data
        }

        int j = 0;
        while (j < ans.size())
        {
            char x = ans[j];
            file.write((char *)&x, sizeof(x)); // writing in the binary file
            ++j;
        }

        // for each instruction writing a 34-bit binary number
    }
}

int stringToInt(const string &str)
{
    int ans = 0;
    int sign = 1;
    int i = 0;

    // Check for negative sign
    if (str[0] == '-')
    {
        sign = -1;
        i++;
    }

    // Convert each digit to integer
    for (; i < str.length(); ++i)
    {
        ans = ans * 10 + (str[i] - '0');
    }

    return sign * ans;
}

int main()
{
    cout << "Enter the name of file:" << endl;
    string nameOfFile;
    cin >> nameOfFile;

    FileRead(nameOfFile);

    // initialising the operations

    // key -> instruction, valueue -> {opcode, operand}
    // - Opcode valueues: -1 for "data", -2 for "SET", and integer valueues for other instructions.
    // - Operand types: 0 for no operand, 1 for valueue operand, and 2 for offset operand.

    instructions["data"] = makeInstruction(-1, 1);
    instructions["SET"] = makeInstruction(-2, 1);
    instructions["ldc"] = makeInstruction(0, 1);
    instructions["adc"] = makeInstruction(1, 1);
    instructions["ldl"] = makeInstruction(2, 2);
    instructions["stl"] = makeInstruction(3, 2);
    instructions["ldnl"] = makeInstruction(4, 2);
    instructions["stnl"] = makeInstruction(5, 2);
    instructions["add"] = makeInstruction(6, 0);
    instructions["sub"] = makeInstruction(7, 0);
    instructions["shl"] = makeInstruction(8, 0);
    instructions["shr"] = makeInstruction(9, 0);
    instructions["adj"] = makeInstruction(10, 1);
    instructions["a2sp"] = makeInstruction(11, 0);
    instructions["sp2a"] = makeInstruction(12, 0);
    instructions["call"] = makeInstruction(13, 2);
    instructions["return"] = makeInstruction(14, 0);
    instructions["brz"] = makeInstruction(15, 2);
    instructions["brlz"] = makeInstruction(16, 2);
    instructions["br"] = makeInstruction(17, 2);
    instructions["HALT"] = makeInstruction(18, 0);
    int line_no = 1;
    // first pass -> takes line by line code and separate label, opcode, and operand and look for errors
    for (int i = 0; i < statements.size(); ++i)
    {
        string line = statements[i];
        // line by line process of code
        extra_operand = 0;
        opcode = "";
        operand = "";
        label = "";

        separateInstruction(line, line_no);

        if (label != "")
        {
            labels[program_counter] = label;
            temporary_label.insert(label);
            label_values[label] = program_counter; // setting label valueue as PC
            if (opcode != "")
                next_statement[label] = 0; // checking if label is followed by opcode or not
            else
                next_statement[label] = 1;
        }

        if (opcode != "")
        {
            opcodes[program_counter] = opcode;
            operands[program_counter] = operand;
            original_operands[program_counter] = operand;
            if (opcode == "SET" || opcode == "data")
                LabelSet.push_back(program_counter); // handling SET and data separateInstructionly
            program_counter++;                         // increasing PC only when opcode is present
        }
        line_no++;
    }

    for (int i = 0; i < not_numerics.size(); ++i)
    {
        string temporary_oper = operands[not_numerics[i].first];
        if (temporary_label.find(temporary_oper) == temporary_label.end())
        {
            errors.push_back({line_no, "No such label"}); // error handling
        }
        else
        {
            operands[not_numerics[i].first] = intToString(label_values[temporary_oper] - not_numerics[i].first - 1); // for jump statements, storing label valueue as PC+1-PC(label)
        }
    }

    if (errors.size() == 0)
    {
        for (int i = 0; i < LabelSet.size(); ++i)
        {
            if (opcodes[LabelSet[i]] == "SET")
                label_values[labels[LabelSet[i]]] = stringToInt(operands[LabelSet[i]]); // only SET sets label valueue
        }

        for (int i = 0; i < not_numerics_for_value.size(); ++i)
        {
            string temporary_oper = operands[not_numerics_for_value[i].first];
            if (label_values.find(temporary_oper) == label_values.end())
            {
                errors.push_back({line_no, "NOT A NUMBER"});
            }
        }

        // second pass -> prints the listing file and machine-readable file and check for errors
        // if no errors are found then print the listing file and binary file
        cout << "Writing the listing and binary file !!" << endl;
        PrintListing(nameOfFile);
        BinaryPrint(nameOfFile);
    }

    // second pass -> prints the listing file and machine-readable file and check for errors
    // if errors are found then print log file
    if (errors.size() != 0)
    {
        cout << "Writing the log file !!" << endl;
        ErrorPrint(nameOfFile);
    }

    return 0;
}
