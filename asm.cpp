/*****************************************************************************
Title: Two Pass Assembler
Author: Saumya Sinha
Roll Number: 2201CS65
Declaration of Authorship
This cpp file, asm.cpp, is part of the assignment of CS209/CS210 at the
department of Computer Science and Engineering, IIT Patna.
*****************************************************************************/
#include<bits/stdc++.h>
using namespace std;

vector<string> mylines;
map<string,pair<int,int>> operations;//for storing mnemonics and their operand type
map<int,string> opcodes,operands,labels;//map to store {PC,labels/opcodes/operans}
map<int,string> original_operands;
string my_opcode,my_operand,my_label;
map<string,int> label_val;//to store values of label to use them as operands
vector<pair<int,int>> not_num;
vector<pair<int,int>> not_num_for_val;
vector<int> set_label;//for SET
vector<pair<int,string>> errors;//for storing differnt errors
set<string>temp_label;
map<string,bool> nxt_line;//to stpore if label is without opcode or not
vector<pair<string,string>> finalcodes;// to store the 8 nibble code
bool extra_operand=false;
int program_counter=0;
int max_line=0;
bool is_halt=false,is_infinity=false;//for warnings

void read_file(string file_name)
{
    ifstream myfile;
  myfile.open(file_name);
    while(myfile){
        string content;
        string temp;
        getline(myfile,content);
        temp=content;
        temp.erase(remove_if(temp.begin(),temp.end(),::isspace),temp.end());
       if(temp!=""){//removing empty lines
        mylines.push_back(content);
       }
    }
}

void initialize()
{
    // key-> instruction    value->{opcode,operand} 
    // data->opcode=-1
    // SET->opcode=-2
    // 0-> no operand
    // 1-> value
    // 2-> offset

    operations["data"]     = {-1, 1};
    operations["SET"]      = {-2, 1};
    operations["ldc"]      = {0, 1};
    operations["adc"]      = {1, 1};
    operations["ldl"]      = {2, 2};
    operations["stl"]      = {3, 2};
    operations["ldnl"]     = {4, 2};
    operations["stnl"]     = {5, 2};
    operations["add"]      = {6, 0};
    operations["sub"]      = {7, 0};
    operations["shl"]      = {8, 0};
    operations["shr"]      = {9, 0};
    operations["adj"]      = {10, 1};
    operations["a2sp"]     = {11, 0};
    operations["sp2a"]     = {12, 0};
    operations["call"]     = {13, 2};
    operations["return"]   = {14, 0};
    operations["brz"]      = {15, 2};
    operations["brlz"]     = {16, 2};
    operations["br"]       = {17, 2};
    operations["HALT"]     = {18, 0};

}

bool check_val(string s){//checks if the operand is label or numeric value
    bool value=true;
    char temp[s.length()];
    for(int i=0;i<s.length();i++){
        temp[i]=s[i];
    }
    long long l=strtol(temp,NULL,0); //converting string to integer
    if(s!="0" && l==0) return false;//strol gives 0 for "ajsj"
    return true;
}

void store(string my_line,int linenumber){//to separate labels,operands and opcodes
auto it=my_line.begin();
auto last=find(my_line.begin(),my_line.end(),';');//only reads upto ;

//seperating label,operand and mnemonic
if(find(my_line.begin(),last,':') !=last){
      for(;(*it)!=':';it++){
        if((*it)!=' '){
             my_label+=(*it);
        }
      }
      it++;
 }

     while(it!=last && (*it)==' ') it++;
     while(it!=last && (*it)!=' ') {my_opcode+=(*it); it++;}
     while(it!=last && (*it)==' ') it++;
     while(it!=last && (*it)!=' ') {my_operand+=(*it);it++;}
     while(it!=last && (*it)==' ') it++;
     if(it!=last){
          extra_operand=true;//checking for extra operand
     }

   //error handling
   if((my_label!="") && temp_label.find(my_label)!=temp_label.end()){
     errors.push_back({linenumber,"Duplicate label definiton"});
   }
   else if(my_label!="" && (my_label[0]>='0' && my_label[0]<='9')){
     errors.push_back({linenumber,"bogus label name"});// 9kal:
   }
   if(my_opcode!="" && operations.find(my_opcode)==operations.end()){
    errors.push_back({linenumber,"bogus mnemonic"});// aab 8
   }
   else if(my_opcode!=""){
    if(operations[my_opcode].second==0){
        if(my_operand!="") errors.push_back({linenumber,"unexpected operand"});// add 7
    }
    if(operations[my_opcode].second==1){
        if(my_operand=="") errors.push_back({linenumber,"missing operand"});
        else if(!check_val(my_operand)){//checking if the operand is numeric or not
           not_num_for_val.push_back({program_counter,linenumber});
        }
        if(!check_val(my_operand) && (my_opcode=="SET" || my_opcode=="data"))//handling non numeric operand for SET and data
        {
             errors.push_back({linenumber,"not a number"});
        }
    }
    if(operations[my_opcode].second==2){
        if(my_operand=="") errors.push_back({linenumber,"missing operand"});
        else if(!check_val(my_operand)){
            if((my_operand[0]>='0' && my_operand[0]<='9')){//label should not start with number
     errors.push_back({linenumber,"bogus label name"});
            }
           else not_num.push_back({program_counter,linenumber});
        }
    }
   }
   if(extra_operand) errors.push_back({linenumber,"extra on end of line"});// ldc 5 6
}

void print_listing(string filename){
    auto it= find(filename.begin(),filename.end(),'.');
    int indx=distance(filename.begin(),it)+1;
    filename.erase(indx);
    filename+="l";
    ofstream file;
    file.open(filename);

    for(int i=0;i<program_counter;i++){
        string temp_labell="";
        string temp_opcode="";
        string temp_operand="";
        temp_labell=labels[i];
        temp_opcode=opcodes[i];
        temp_operand=operands[i];
 
        string pc="";
              string lead_zeros="";
              stringstream s;
              s<<hex<<i;
              pc=s.str();//converting pc to hexadecimal
              for(int i=0;i<8-pc.length();i++){
                lead_zeros+="0";
              }
          
              pc=lead_zeros+pc;//to put leading zeroes
       
                if(temp_opcode=="HALT") is_halt=true; // checking whether halt is present or not
                if(temp_labell==original_operands[i] && temp_labell!="") {
                    is_infinity=true; //handling infinit loop-> label: ldc label
                }

              stringstream ss,s1;// for converting opcodes and operand to hexadecimal 
             
              if(temp_opcode!="SET" && temp_opcode!="data"){
                  int ops=operations[temp_opcode].first;
                  ss<<hex<<ops;
              }
             
               char oper[100];
            for(int j=0;j<100;j++) oper[j]='?';
             auto it=temp_operand.begin();
            for(int j=0;j<temp_operand.size();j++){
                oper[j]=*it;
                it++;
            }
    
           if(!check_val(temp_operand)){//handling if operand not numeric but a label
            int val =label_val[temp_operand];
             s1<<hex<<val;
             
           }
           else{
                int val=strtol(oper,NULL,0);
              s1<<hex<<val;
           }

           string code="";
           if(temp_opcode!="SET" && temp_opcode!="data"){
            string htp=s1.str();
            if(s1.str().size()>6){//handling for negative numbers..removing the first two most significant bits
                htp=s1.str().substr(s1.str().size()-6,6);
            }
            if(ss.str().length()!=2){
            code+=htp+"0"+ss.str();
           }
           else{
            code+=htp+ss.str();
           }
           }
           else{
            code+=s1.str();// for set and data no opcode
           }
           
           lead_zeros="";
           for(int i=0;i<8-code.length();i++){
                lead_zeros+="0";}
              code=lead_zeros+code;// the 8 nibble code

        if(nxt_line[temp_labell]){
            // cout<<pc<<" "<<code<<endl;
          file<<pc<<"          "<<temp_labell+":"<<endl;
          file<<pc<<" "<< code<<" "<<temp_opcode<<" "<<original_operands[i]<<endl;
        }
        else{
            // cout<<pc<<" "<<code<<endl;
          if(temp_labell!="")file<<pc<<" "<< code<<" "<<temp_labell+":"<<" "<<temp_opcode<<" "<<original_operands[i]<<endl;
          else file<<pc<<" "<< code<<" "<<temp_opcode<<" "<<original_operands[i]<<endl;
        }

     finalcodes.push_back({code,temp_opcode});//storing codes and mnemonic
    }
    if(!is_halt)  file<<"Warning : HALT is missing !!"<<endl;//if HALT is not present
    if(is_infinity) file<<"Warning : Infinity LOOP detected !!"<<endl;
}

void print_error(string filename){//for printing errors
     auto it= find(filename.begin(),filename.end(),'.');
    int indx=distance(filename.begin(),it)+1;
    filename.erase(indx);
    filename+="log";
    ofstream file;
    file.open(filename);
     sort(errors.begin(),errors.end());
    for(auto err:errors){
        file<<"Error on line "<<err.first<<" : "<<err.second<<endl;
    }
}

void print_binary(string filename){
   auto it= find(filename.begin(),filename.end(),'.');
    int indx=distance(filename.begin(),it)+1;
    filename.erase(indx);
    filename+="o";

    ofstream file;
    file.open(filename,ios::binary);
    for(auto val:finalcodes){
         unsigned int x =  stoul(val.first, nullptr, 16) ;//converting hex to decimal
  string result = bitset<32>(x).to_string();//Using bitset<32>(x).to_string() to get the binary
        
        //adding two check bits for SET and data
     if(val.second=="SET")  result+="10";//10->SET
     else if(val.second=="data") result+="01";//01->data
     else result+="00"; //00->no set/data

        for(auto i=result.begin();i!=result.end();i++){
            char x=*i;
            file.write((char *)&x , sizeof(x));//writing in the binary file
         }
        //for each instruction writing a 34 bit binary number 
    }
}

signed main()
{
  cout<<"Enter the file name:"<<endl;
  string file_name;
  cin>>file_name;
  
  read_file(file_name);
  initialize();
  int linenumber=1;
  //first pass-> takes line by line code and seperate label, opcode and operand and look for errors
 for(auto my_line:mylines){//line by line process of code
    extra_operand=false;
    my_opcode="";
    my_operand="";
    my_label="";

    store(my_line,linenumber);
    
    if(my_label!=""){
        labels[program_counter]=my_label;
        temp_label.insert(my_label);
        label_val[my_label]=program_counter;//setting label value as PC
        if(my_opcode!="") nxt_line[my_label]=false;// checking if label is followed by opcode or not
        else nxt_line[my_label]=true;
    }

    if(my_opcode!=""){
        opcodes[program_counter]=my_opcode;
        operands[program_counter]=my_operand;
        original_operands[program_counter]=my_operand;
        if(my_opcode=="SET" || my_opcode=="data") set_label.push_back(program_counter);//handling SET and data separately
        program_counter++;//incresing PC only when opcode is present
    }
    linenumber++;
}

for(auto v:not_num){
    string temp_oper=operands[v.first];
    if(find(temp_label.begin(),temp_label.end() ,temp_oper)==temp_label.end()){
        errors.push_back({linenumber,"no such label"});//error handling
    }
    else{
       operands[v.first]=to_string(label_val[temp_oper]-v.first-1);//for jump statements storing label value as PC+1-Pc(label) 
    }
}


if(errors.size()==0)
{
    for(auto v:set_label){
    if(opcodes[v]=="SET") label_val[labels[v]]=stoi(operands[v]);//only SET sets label value
}
    for(auto v:not_num_for_val){
    string temp_oper=operands[v.first];
    if(label_val.find(temp_oper)==label_val.end()){
        errors.push_back({linenumber,"not a number"});
    }
}}

//second pass-> prints the listing file and machine-readable file and check for errors
if(errors.size()==0){//if no errors print the listing file and binary file
    cout<<"Writing the listing and binary file !!"<<endl;
    print_listing(file_name);
    print_binary(file_name);
}
else{//if errors print log file
    cout<<"Writing the log file !!"<<endl;
    print_error(file_name);
}
return 0;
}
