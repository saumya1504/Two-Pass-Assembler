/*****************************************************************************
Title: Emulator
Author: Saumya Sinha
Roll Number: 2201CS65
Declaration of Authorship
This cpp file, emu.cpp, is part of the assignment of CS209/CS210 at the
department of Computer Science and Engineering, IIT Patna .
*****************************************************************************/

#include <iostream>
#include <fstream>
#include <vector>
#include <tuple>
#include <sstream>
#include <bitset>
#include <algorithm>
using namespace std;

vector<pair<int, int>> operand_opcode; //{operand,opcode}
vector<tuple<int, int, int, int>> trace;
int regA, regB, ProgramCounter, StackPointer; // reg1,reg2,Program cntr,Stack Pointer
bool halt_requested = false;
vector<int> memory(16777216); // memory  & (1 << 24 = 16777216)
vector<int> memory_locations; // stores the memory location that has been accesssed

// Function to convert an integer valueHexStr to a hexadecimal string
string intToHexString(int valueHexStr)
{
    stringstream ss;
    ss << hex << valueHexStr;
    return ss.str();
}

int binaryStringToInt(const string &binaryStr)
{
    int result = 0;
    for (char ch : binaryStr)
    {
        result = result * 2 + (ch - '0');
    }
    return result;
}

void read_bin(string FileName)
{
    ifstream input;
    input.open(FileName, ios::binary);
    char x;
    string code;
    int cntr = 0;
    int pc = 0;
    while (input.read((char *)&x, sizeof(x)))
    {
        cntr++; //counter incremented
        code += x;
        if (cntr == 34)
        { // 8 nibble code = code for set/data
            cntr = 0;
            string operand, opcode, check;

            // Extract set/data check (2 bits)
            check = "";
            for (int i = 32; i < 34; i++)
            {
                check += code[i];
            }

            // Extract opcode in binary (8 bits)
            opcode = "";
            for (int i = 24; i < 32; i++)
            {
                opcode += code[i];
            }

            // Extract operand in binary (24 bits)
            operand = "";
            for (int i = 0; i < 24; i++)
            {
                operand += code[i];
            }

            code.clear();

            if (check == "00")
            {
                switch (operand[0])
                {
                case '1':
                {
                    int i = 0;
                    while (i < 24)
                    {
                        if (operand[i] == '1')
                        {
                            operand[i] = '0';
                        }
                        else
                        {
                            operand[i] = '1';
                        }
                        i++;
                    }

                    operand_opcode.insert(operand_opcode.end(), make_pair((-1) * (binaryStringToInt(operand) + 1), binaryStringToInt(opcode)));
                    break;
                }
                default:
                    operand_opcode.insert(operand_opcode.end(), make_pair(binaryStringToInt(operand), binaryStringToInt(opcode)));
                    break;
                }
            }
            else
            { // if set/data is present
                memory_locations.insert(memory_locations.end(), pc);
                operand += opcode;
                if (operand[0] == '1') // Check if the number is negative (i.e., if the most significant bit is 1)
                {
                    // Perform one's complement operation to convert the negative number to positive
                    int i = 0;
                    while (i < 24)
                    {
                        if (operand[i] == '1')
                        {
                            operand[i] = '0';
                        }
                        else
                        {
                            operand[i] = '1';
                        }
                        ++i;
                    }
                    // If set/data is present, set memory to the two's complement of the operand
                    if (check == "01")
                        memory[pc] = (-1) * (binaryStringToInt(operand) + 1); // setting memory for data mnemonic

                    // Push the two's complement of the operand and opcode as a pair to operand_opcode vector
                    operand_opcode.insert(operand_opcode.end(), make_pair((-1) * (binaryStringToInt(operand) + 1), binaryStringToInt(opcode))); // 2's complement
                }
                else
                {
                    // If set/data is present, set memory to the operand valueHexStr
                    operand_opcode.insert(operand_opcode.end(), make_pair(binaryStringToInt(operand), binaryStringToInt(opcode)));
                    if (check == "01")
                        memory[pc] = binaryStringToInt(operand);
                }
            }
            pc++;
        }
    }
}

void modifyFileName(std::string &file, const std::string &appendString)
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

void executeOpcode() // Set the label on this line to the specified valueHexStr (rather than the PC). This is an choiceal extension,for which additional marks are available.
{
    for (; ProgramCounter < operand_opcode.size() && !halt_requested; ProgramCounter++)
    { // while PC doesnt end or HALT doesn't come
        int operand = operand_opcode[ProgramCounter].first;
        int opcode = operand_opcode[ProgramCounter].second;
        int temp = ProgramCounter;

        // mnemonics and their function decision according to the table
        if (opcode == 0) // ldc
        {                // Load accumulator with the valueHexStr specified
            regB = regA;
            regA = operand;
        }
        else if (opcode == 1) // adc
        {                     // Add the valueHexStr specified to the accumulator
            regA += operand;
        }
        else if (opcode == 2) // ldl
        {                     // Load locHexStral
            regB = regA;
            regA = memory[StackPointer + operand];
        }
        else if (opcode == 3) // stl
        {                     // Load locHexStral
            memory[StackPointer + operand] = regA;
            memory_locations.insert(memory_locations.end(), StackPointer + operand);
            regA = regB;
        }
        else if (opcode == 4) // ldnl
        {                     // Load non-locHexStral
            regA = memory[regA + operand];
        }
        else if (opcode == 5) // stnl
        {                     // store non-locHexStral
            memory[regA + operand] = regB;
            memory_locations.insert(memory_locations.end(), regA + operand);
        }
        else if (opcode == 6) // add
        {                     // addition
            regA = regA + regB;
        }
        else if (opcode == 7) // sub
        {                     // subtraction
            regA = regB - regA;
        }
        else if (opcode == 8) // shl
        {                     // shift left operation (A = B << A)
            int tempB = regB; // Create a copy of B
            for (int i = 0; i < regA; ++i)
            {
                tempB *= 2; // Equivalent to tempB = tempB * 2
            }
            regA = tempB; // Store the result in A
        }
        else if (opcode == 9) // shr
        {                     // shift right operation (A = B >> A)
            int tempB = regB; // Create a copy of B
            for (int i = 0; i < regA; ++i)
            {
                tempB /= 2; // Equivalent to tempB = tempB / 2
            }
            regA = tempB; // Store the result in A
        }
        else if (opcode == 10) // adj
        {                      // Adjust SP
            StackPointer = StackPointer + operand;
        }
        else if (opcode == 11) // a2sp
        {                      // Transfer A to SP
            StackPointer = regA;
            regA = regB;
        }
        else if (opcode == 12) // sp2a
        {                      // Transfer SP to A
            regB = regA;
            regA = StackPointer;
        }
        else if (opcode == 13) // call
        {                      // Call procedure
            regB = regA;
            regA = ProgramCounter;
            ProgramCounter += operand;
        }
        else if (opcode == 14) // ret
        {                      // Return from procedure
            ProgramCounter = regA;
            regA = regB;
        }
        else if (opcode == 15) // brz
        {                      // If accumulator is zero, branch to specified offset
            if (regA == 0)
                ProgramCounter = ProgramCounter + operand;
        }
        else if (opcode == 16) // brlz
        {                      // If accumulator is less than zero, branch to specified offset
            if (regA < 0)
                ProgramCounter = ProgramCounter + operand;
        }
        else if (opcode == 17) // br
        {                      // Branch to specified offset
            ProgramCounter = ProgramCounter + operand;
        }
        else if (opcode == 18) // halt
        {                      // Stop the emulator. This is not a `real' instruction, but needed to tell your emulator when to finish.
            halt_requested = true;
        }
        else
        {
            // Handle unsupported opcode
        }

        tuple<int, int, int, int> line;
        line = make_tuple(regA, regB, ProgramCounter, StackPointer); // making a tuple
        trace.insert(trace.end(), line);
        // PC++; // increasing PC
    }
}

signed main()
{
    cout << "Enter the name of file:" << endl;
    string file;
    cin >> file;
    read_bin(file); // read the binary file to get opcodes and operands
    executeOpcode();
    if (!halt_requested)
        cout << "WARNING: No HALT, exit program due to end of file\n\n"; // handling no HALT warning
    while (1)
    {
        cout << "type 1 : To see the trace line by line" << endl;
        cout << "type 2 : To see current Memory Dump" << endl;
        cout << "type 3 : To exit loop" << endl;
        int choice;
        cin >> choice;

        int terminate_loop = 0;
        switch (choice)
        {
        case 1:
        {
            cout << "Creating listing file \n";
            modifyFileName(file, "trace");
            ofstream tracefile;
            tracefile.open(file);
            int j = 0;
            for (auto it : trace)
            {
                tracefile << "Line = " << j + 1 << " A = " << get<0>(it) << " B = " << get<1>(it) << " PC = " << get<2>(it) << " SP = " << get<3>(it) << endl;
                j++;
            }
            break;
        }
        case 2:
        {
            modifyFileName(file, "memdump");
            ofstream mdfile;
            mdfile.open(file);

            cout << "Creating memory dump file \n";
            // prints only the memory that has been accessed
            for (int i = 0; i < memory_locations.size(); ++i)
            {
                // converting memory location and value to hexadecimal
                string locHexStr, valueHexStr;
                locHexStr = intToHexString(memory_locations[i]);
                valueHexStr = intToHexString(memory[memory_locations[i]]);
                // Pad locHexStr with leading zeros to make it 8 characters long
                while (locHexStr.size() < 8)
                {
                    locHexStr = "0" + locHexStr;
                }

                // Pad valueHexStr with leading zeros to make it 8 characters long
                while (valueHexStr.size() < 8)
                {
                    valueHexStr = "0" + valueHexStr;
                }

                mdfile << locHexStr << " " << valueHexStr << endl;
            }
            break;
        }
        case 3:
            terminate_loop++; // Exiting the loop and the program
            break;
        default:
            cout << "Command is Invalid" << endl;
        }
        if (terminate_loop > 0)
            break;
    }

    return 0;
}