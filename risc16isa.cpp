#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include<iomanip>
using namespace std;
string code = R"(lw     1,0,count    # load reg1 with 5 (uses symbolic address)
lw     2,1,2        # load reg2 with -1 (uses numeric address)
start:       add    1,1,2        # decrement reg1 -- could have been addi 1,1,-1
beq    0,1,1        # goto end of program when reg1==0
beq    0,0,start    # go back to the beginning of the loop
done:        halt                # end of program
count:       .fill  5
neg1:        .fill  -1
startAddr:   .fill  start        # will contain the address of start (2)
)";

/*
nop add 0,0,0
halt jalr 0,0
lli add X,X,imm6
*/
bool inkw = false;
bool innumber = false;
bool incomment = false;
bool inwhite = false;
bool isnegative = false;
string kw = "";
string lastkw = "";
string digit = "";
string label = "";
string instruction = "";
string rega = "";
string regb = "";
string regc = "";
bool indirective = false;
string* arr[] = { &rega,&regb,&regc };
int regindex = 0;
void feedkw(string kw) {
	lastkw = kw;
	if (indirective) {
		//printf("directive %s\n", kw.c_str());
		instruction = ".fill";
		indirective = false;
		regindex++;
	}
	if (regindex > 0&&regindex<4) {
		*arr[regindex - 1] = kw;
		//printf("symbolic %s\n",kw.c_str());
	}
}
void feedwhite() {
	if (!lastkw.empty()) {
		if (instruction.empty()&&(label!=lastkw)) {
			instruction = lastkw;
			
			regindex++;
		}
	}
}
void feedop(char c) {
	if (c == ':') {
		if (!lastkw.empty()) {
			label = lastkw;
			
		}
	}
	else if (c=='.') {
		indirective = true;
	}
	else if (c == ',') {
		regindex++;
	}
}
void feeddigit(string digit) {
	//int i = atoi(digit.c_str());
	//printf("i:%i\n", i);
	string neg = "";
	if (isnegative) {
		isnegative = false;
		neg = "-";
	}
	if (regindex == 1) {
		rega = neg + digit;
	}
	else if (regindex == 2) {
		regb = neg + digit;
	}
	else if (regindex == 3) {
		regc = neg +digit;
	}
}
struct Instruction {
	string label = "";
	string name="";
	string a="";
	string b="";
	string c="";
	int ia=0;
	int ib=0;
	int ic=0;
	uint16_t out = 0;
};
int progpointer = 0;
vector<Instruction> instructions;
void feednewline() {
	/*printf("%s: ", label.c_str());
	printf(" %s", instruction.c_str());
	printf(" %s, %s, %s \n", rega.c_str(),regb.c_str(),regc.c_str());*/
	instructions.push_back({ label,instruction,rega,regb,regc,0,0,0,0 });
	progpointer++;
	instruction = "";
	label = "";
	rega = "";
	regb = "";
	regc = "";
	regindex = 0;
}
void resetparser() {
	if (inkw) {
		feedkw(kw);
		kw = "";
		inkw = false;
	}
	if (innumber) {
		feeddigit(digit);
		digit = "";
		innumber = false;
	}
	if (inwhite) {
		inwhite = false;
		feedwhite();
	}
}
void parsechar(char c) {
	if (incomment) {
		if (c == '\n') {
			incomment = false;
		}
		else {

			return;
		}
	}
	if (isalpha(c)) {
		if (!inkw) {
			resetparser();
			inkw = true;
		}
		kw += c;
	}
	else if (isdigit(c)) {
		if (!innumber) {
			resetparser();
			innumber = true;
		}
		digit += c;
	}
	else {
		if (c == '#') {
			resetparser();
			incomment = true;
		}
		else if (c == ':') {
			resetparser();
			feedop(c);
		}
		else if (c == '.') {
			resetparser();
			feedop(c);
		}
		else if (c == ',') {
			resetparser();
			feedop(c);
		}
		else if (c == '\n') {
			resetparser();
			feednewline();
		}
		else if (c == '-') {
			resetparser();
			isnegative = true;
		}
		else {
			
			if (!inwhite) {
				resetparser();
				inwhite = true;
				
			}
		}
	}
}
void parse(string code) {
	for (size_t i = 0; i < code.length(); i++)
	{
		parsechar(code[i]);
	}
}
map<string, int> labels;
template< typename T >
std::string int_to_hex(T i)
{
	std::stringstream stream;
	stream << "0x"
		<< std::setfill('0') << std::setw(sizeof(T) * 2)
		<< std::hex << i;
	return stream.str();
}
void processinstructions() {
	for (size_t i = 0; i < instructions.size(); i++)
	{
		if (!instructions[i].label.empty()) {
			if (labels.count(instructions[i].label)) {
				printf("error label %s already defined\n", instructions[i].label.c_str());
			}
			else {
				labels[instructions[i].label] = i;
				//printf("setting label '%s' to %i \n", instructions[i].label.c_str(), i);
			}
		}
	}
	for (size_t i = 0; i < instructions.size(); i++)
	{
		if (!instructions[i].a.empty()) {
			if (!isalpha(instructions[i].a[0])) {
				instructions[i].ia = atoi(instructions[i].a.c_str());
			}
			else {
				if (!labels.count(instructions[i].a)) {
					printf("error label '%s' at rega is undefined\n", instructions[i].a.c_str());
				}
				else {
					instructions[i].ia = labels[instructions[i].a];
				}
			}
		}
		if (!instructions[i].b.empty()) {
			if (!isalpha(instructions[i].b[0])) {
				instructions[i].ib = atoi(instructions[i].b.c_str());
			}
			else {
				if (!labels.count(instructions[i].b)) {
					printf("error label %s at regb is undefined\n", instructions[i].b.c_str());
				}
				else {
					instructions[i].ib = labels[instructions[i].b];
				}
			}
		}
		if (!instructions[i].c.empty()) {
			if (!isalpha(instructions[i].c[0])) {
				instructions[i].ic = atoi(instructions[i].c.c_str());
			}
			else {
				if (!labels.count(instructions[i].c)) {
					printf("error label %s at regc is undefined\n", instructions[i].c.c_str());
				}
				else {
					instructions[i].ic = labels[instructions[i].c];
				}
			}
		}
		/*printf("%i\t %s: ", i, instructions[i].label.c_str());
		printf(" %s", instructions[i].name.c_str());
		printf(" %i, %i, %i \n",
			instructions[i].ia,
			instructions[i].ib,
			instructions[i].ic); */
		int8_t inst = -1;
		if (instructions[i].name.find("lw") == 0) {
			inst = 0b101;
		}
		else if (instructions[i].name.find("addi") == 0) {
			inst = 0b001;
		}
		else if (instructions[i].name.find("add") == 0) {
			inst = 0b000;
		}
		else if (instructions[i].name.find("sw") == 0) {
			inst = 0b100;
		}
		else if (instructions[i].name.find("beq") == 0) {
			inst = 0b110;
		}
		else if (instructions[i].name.find("jalr") == 0) {
			inst = 0b111;
		}
		else if (instructions[i].name.find("halt") == 0) {
			inst = 0b111;
		}
		if (instructions[i].name.find(".fill")==0) {
			int16_t out = instructions[i].ia;
			std::cout << int_to_hex(out);
			std::cout << endl;
			instructions[i].out = out;
		}
		else {
			uint8_t rega = instructions[i].ia;
			uint8_t regb = instructions[i].ib;
			int8_t imm = instructions[i].ic;
			if (instructions[i].name.find("beq") == 0) {
				if (!isdigit(instructions[i].c[0])) {
					imm = instructions[i].ic -i-1;
				}
			}
			uint16_t out = 0;
			out |= (inst & 0b111) << 13;
			out |= (rega & 0b111) << 10;
			out |= (regb & 0b111) << 7;
			out |= (imm & 0b1111111);
			std::cout << int_to_hex(out);
			std::cout << endl;
			instructions[i].out = out;
		}
	}
}

void run() {
	int progcounter = 0;
	int8_t registers[8] = { 0 };
	uint8_t memory[1024] = { 0 };
	bool running = true;
	while (running) {
		printf("%i -> ", progcounter);
		for (size_t i = 0; i < 8; i++)
		{
			printf("%i: %i\t ",i, registers[i]);
		}
		
		if (progcounter >= instructions.size()) {
			
			return;
		}
		uint16_t inst = instructions[progcounter].out;
		uint8_t type = (inst >> 13) & 0b111;
		uint8_t rega = (inst >> 10) & 0b111;
		uint8_t regb = (inst >> 7) & 0b111;
		int8_t imm = (inst) & 0b1111111;
		//printf("imm %i ", imm);
		imm |= ((imm >> 6) & 1) << 7;
		if (type == 0b101) {
			
			registers[rega] = instructions[registers[regb] + imm].out;
		}
		if (type == 0b000) {
			uint8_t regc = (inst) & 0b111;
			registers[rega] = registers[regb] + registers[regc];
		}
		if (type == 0b110) {
			if (registers[rega] == registers[regb]) {
				
				progcounter = progcounter+ imm;
			}
		}
		if (type == 0b111) {
			if (rega == 0 && regb == 0) {
				return;
			}
		}
		printf("  %i %i\n",type,  imm);
		progcounter++;
	}
}
int main()
{
	parse(code);
	processinstructions();
	run();
}
