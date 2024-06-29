/*****************************************************************************
Title: Emulator
Author: Saumya Sinha
Roll Number: 2201CS65
Declaration of Authorship
This cpp file, emu.cpp, is part of the assignment of CS209/CS210 at the
department of Computer Science and Engineering, IIT Patna .
*****************************************************************************/

#include<bits/stdc++.h>
using namespace std;

vector<pair<int,int>>ops;//{operand,opcode}
vector<tuple<int,int,int,int>>trace;
int A,B,PC,SP;//reg1,reg2,Program counter,Stack Pointer
bool exitprg=false;
int memory[1<<24];//memory
vector<int> mem_locations;//stores the memory location that has been accesssed

void read_bin(string filename)
{
   ifstream input;
   input.open(filename,ios::binary);
   char x;
   string code;
   int counter=0;
   int pc=0;
   while(input.read((char*)&x,sizeof(x))){
       counter++;
       code+=x;
       if(counter==34){//8 nibble code = code for set/data
          counter=0;
          string oper=code.substr(0,24);//for operand in binary
          string opc=code.substr(24,8);//for opcode in binary
          string check=code.substr(32,2);// to check set/data
          code.clear();
          if(check=="00"){
          if(oper[0]=='1') // if number in negative
          {
            for(int i=0;i<24;i++){
            oper[i]=((oper[i]=='1')?'0':'1');
            }
            ops.push_back({(-1)*(stoi(oper,0,2)+1),stoi(opc,0,2)});//2's complement

          }
          else ops.push_back({stoi(oper,0,2),stoi(opc,0,2)});

          }
          else{//if set/data is present
            mem_locations.push_back(pc);
            oper+=opc;
            if(oper[0]=='1') // if number in negative
          {
            for(int i=0;i<24;i++){
            oper[i]=((oper[i]=='1')?'0':'1');//1's complement
            }
           if(check=="01")memory[pc]=(-1)*(stoi(oper,0,2)+1);//setting memory for data mnemonic
           ops.push_back({(-1)*(stoi(oper,0,2)+1),stoi(opc,0,2)});//2's complement
          }
          else {
            ops.push_back({stoi(oper,0,2),stoi(opc,0,2)});
            if(check=="01")memory[pc]=stoi(oper,0,2);
          }
          }
          pc++;
       }
   }
}

//All the mnemonics with their functions
void ldc(int value){                        
    B = A;
    A = value;
}
void adc(int value){
    A += value;
}
void ldl(int offset){
    B = A;
    A = memory[SP + offset];
}
void stl(int offset){
    memory[SP + offset] = A;
    mem_locations.push_back(SP+offset);
    A = B;
}
void ldnl(int offset){
    A = memory[A + offset];
}
void stnl(int offset){
    memory[A + offset] = B;
    mem_locations.push_back(A+offset);
}
void add(){
    A += B;
}
void sub(){
    A = B - A;
}
void shl(){
    A = B << A;
}
void shr(){ 
    A = B >> A;
}
void adj(int value){
    SP = SP + value;
}
void a2sp(){
    SP = A;
    A = B;
}
void sp2a(){
    B = A;
    A = SP;
}
void call(int offset){
    B = A;
    A = PC;
    PC += offset;
}
void ret(){
    PC = A;
    A = B;
}
void brz(int offset){
    if(A == 0) PC = PC + offset;
}
void brlz(int offset){
    if(A < 0) PC = PC + offset;
}
void br(int offset){ 
    PC = PC + offset;
}
void HALT(){
    exitprg=true;
}

void run_code(){
    while(PC<ops.size() && !exitprg){//while PC doesnt end or HALT doesn't come
        int operand=ops[PC].first;
        int opcode=ops[PC].second;
        int temp=PC;
        switch (opcode)//chosing mnemonic according to opcode
            {
            case 0 :
                ldc(operand);
                break;
            
            case 1 :
                adc(operand);
                break;
            
            case 2 :
                ldl(operand);
                break;

            case 3 :
                stl(operand);
                break;
            case 4 :
                ldnl(operand);
                break;        
            
            case 5 :
                stnl(operand);
                break;

            case 6 :
                add();
                break;

            case 7 :
                sub();
                break;

            case 8 :
                shl();
                break;

            case 9 :
                shr();
                break;

            case 10 :
                adj(operand);
                break;

            case 11 :
                a2sp();
                break;

            case 12 :
                sp2a();
                break;

            case 13 :
                call(operand);
                break;

            case 14:
                ret();
                break;

            case 15 :
                brz(operand);
                break;

            case 16 :
                brlz(operand);
                break;

            case 17 :
                br(operand);
                break;

            case 18 :
               HALT();
                break;                                        
            default:
                break;
            }

            tuple<int,int,int,int> line;
            line=make_tuple(A,B,PC,SP);// making a tuple
            trace.push_back(line);
            PC++;// increasing PC 
    }
}

signed main()
{
    cout<<"Enter the file name:"<<endl;
    string file;
    cin>>file;
    read_bin(file);//read the binary file to get opcodes and operands
    run_code();
    if(!exitprg) cout<<"WARNING: No HALT, exit program due to end of file\n\n";//handling no HALT warning
    while(true){
    cout<<"To see the trace line by line :  type 1" << endl;
    cout<<"To see current Memory Dump :     type 2" << endl;
    cout<<"To exit loop:                    type 3" <<endl;
    int option;
    cin>>option;
   
    if(option==1){
         cout<<"Creating listing file \n";
        auto it= find(file.begin(),file.end(),'.');
    int indx=distance(file.begin(),it)+1;
    file.erase(indx);
    file+="trace";
    ofstream tracefile;
    tracefile.open(file);
    for(int i=0;i<trace.size();i++){
        tracefile<<"Line = "<<i+1<<" A = "<<get<0>(trace[i])<<" B = "<<get<1>(trace[i])<<" PC = "<<get<2>(trace[i])<<" SP = "<<get<3>(trace[i])<<endl;
        }
    }
    else if(option==2){
        auto it= find(file.begin(),file.end(),'.');
    int indx=distance(file.begin(),it)+1;
    file.erase(indx);
    file+="memdump";
    ofstream mdfile;
    mdfile.open(file);

        cout<<"Creating memory dump file \n";
        //prints only the memory that has been accessed
        for(auto val:mem_locations){
            //converting memory location and value to hexadecimal
            stringstream ss,s;
            ss<<hex<<val;
            s<<hex<<memory[val];
            string loc,value;
            loc=ss.str();
            value=s.str();
            string lead_zeros="";
             for(int i=0;i<8-loc.length();i++){
                lead_zeros+="0";}
              loc=lead_zeros+loc;
              lead_zeros="";
              for(int i=0;i<8-value.length();i++){
                lead_zeros+="0";}
              value=lead_zeros+value;
            mdfile<<loc<<" "<<value<<endl;
        }

    }
    else if(option==3){
       break;
    }
    else{
        cout<<"Invalid command"<<endl;
    }

    }

return 0;
}
