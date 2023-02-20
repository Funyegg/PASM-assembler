//Since this will be used in an environment with my own OS, it can't use libraries. Basic IO is an exception
//Special thanks to disasm.pro and c9x.pro for helping me learn the opcodes!

#include <stdio.h>
//Constants and global vars
char allowed[][8] = {"MOV","CLI","STI","INT","OUTB","OUTW","PAD","DB","DW","DD","JMP","LABEL","ORG","CALL","RET","INC","DEC","ADD","SUB"}; //Allowed instructions
int inscount = 19; //Instruction count, since sizeof on arrays gets all fucky in freestanding environments
char oprule[] = {2,0,0,1,0,0,1,1,1,1,1,1,1,1,0,1,1,2,2}; //intended operand count for each
char registers8[][3] = {"AL","CL","DL","BL","AH","CH","DH","BH"}; // The available 8 bit registers
char labels[50][50] = {0}; //Available labels, can be up to 50 chars long
int labeldesc[50] = {0}; //Label location descriptors
char registers16[][3] = {"AX","CX","DX","BX","SP","BP","SI","DI"}; // The available 16 bit registers
int asmlen = 0;
int asmloc = 0;
int labeli = 0;

char compstr(char* str1, char* str2){
while(*str1 || *str2){
if(*str1 != *str2) return 0;
str1++;
str2++;
}
return 1;
}

//Putting a short into a file. Not the best solution
void putshort(unsigned short toput, FILE* fpoint){
short towrite[1] = {toput};
fwrite(towrite, 2, 1, fpoint);
}

int findregister(int* registeri, char* regist){
int mode = 0;
*registeri = 0;
while(!compstr(registers8[*registeri], regist) && *registeri<8) (*registeri)++;
//Something went wrong if it's at the 9th array entry: maybe they're looking for a 16 bit register?
if(*registeri==8){
mode = 1;
*registeri = 0;
while(!compstr(registers16[*registeri], regist) && *registeri<8) (*registeri)++;
}
return mode;
}

//Converting strings to integers.
int strtoint(char* ognumber){
int result = 0;
int i = 1;
char* toconv = ognumber;
char base = 10; //Assume base 10
//Finding out what base the string is in
if(*toconv == '0' && *(toconv+1) == 'x'){
base = 16;
ognumber+=2;
toconv+=2;
}
//Make sure to fix binary later
else if(*toconv == '0' && *(toconv+1) == 'b'){
base = 2;
ognumber+=2;
toconv+=2;
}

while(*toconv) toconv++;
toconv--; //Now at the last char of the string

//Read the numbers in ascending order
while(toconv>=ognumber){
*toconv-=48;
if(*toconv>41) *toconv-=39; //If a lowecase letter is used, subtract 39
else if(*toconv>9) *toconv-=7; //If an uppercase letter is used, subtract 7

result+=*toconv*i; //The character is now the appropriate digit. Multiply by i which is the column in the digit table
i*=base; //Multiply i by the base, to move up a column in the digit table
toconv--; //Go down
}
return result;
}


//ASSEMBLING FUNCTION
int assemble(char* tocompile, FILE* foutput){
char instruction[8] = {0}; // The instruction for this statement
char intendedops = 0; // The indended operand for this
char opcount = 0; //The ACTUAL operand count
char operands[][33] = {0,0,0}; //The ACTUAL operands. There can be up to 3 in assembly. They shouldn't exceed 33 characters

if(*tocompile == ';') return 0;

//Couple indexing variables
int i = 0;
int lastend = 0;
char found = 0;


//Find the instruction (which ends at the space or line break)
while(tocompile[i] != '\n' && tocompile[i] != ' '){
instruction[i] = tocompile[i];
i++;
lastend++;
}

//Find if the instruction is allowed. Then find the appropriate rules.
for(i=0;i<inscount;i++){
if(compstr(instruction, allowed[i])){
found=i+1;
intendedops = oprule[i];
break;
}
}

if(intendedops) lastend++; //Skip past the space (for finding operands, if there's any operands to find)

//return if instruction is not allowed.
if(!found) return 3;

//Save the operands.
while(tocompile[lastend] != '\n'){
i=0;
while(tocompile[lastend]!=' ' && tocompile[lastend]!='\n'){
operands[opcount][i] = tocompile[lastend];
lastend++;
i++;
}
operands[opcount][i] = 0; //Make sure the operand is terminated
opcount++;
if(tocompile[lastend] != '\n') lastend++; //Go forward a character, unless we hit the termination character (in this case a line break)
}

//Check if the amount of operands matches with the indended amount
if(opcount!=intendedops) return 2;

//Normally I hate IF cascades, but I couldn't be bothered with a big solution. If you have a better solution please fix this.

//MOV instruction, it's quite a big one
if(found == 1){
int registeri = 0;
char mode = 0;
char towrite[2] = {0,0};
int endloc = 0;

//Find the register's index
mode = (char)findregister(&registeri, operands[0]);

//The register they're looking for doesn't exist. return a code 1
if(registeri==8) return 1;

towrite[0] = registeri+0xB0+8*mode; //Now we have the MOV opcode

//Turn the end location into something the computer can understand
if(operands[1][0]!='_') endloc = strtoint(operands[1]);
else{
//Find the label
int labeli2 = 0;
while(!compstr(labels[labeli2], operands[1]) && labeli<50) labeli++;
if(labeli2==50) return 4; //Invalid label: code 4
endloc = labeldesc[labeli2];
}

//MOV byte if the mode is 0
if(!mode){
towrite[1] = (char)endloc; //Now we have the operand
fwrite(towrite, 1, 2, foutput); //Append the line (as bytes) to the file
asmlen+=2;
} 
//MOV a word otherwise (2 bytes)
else{
fputc(towrite[0], foutput);
putshort(endloc, foutput);
asmlen+=3;
}

} else if(found==2){
fputc(0xFA, foutput);
asmlen++;
}
else if(found==3) {
fputc(0xFB, foutput);
asmlen++;
}
else if(found==4){
fputc(0xCD, foutput);
fputc(strtoint(operands[0]), foutput);
asmlen+=2;
}
else if(found==5){
fputc(0xEE,foutput);
asmlen++;
}
else if(found==6){
fputc(0xEF,foutput);
asmlen++;
}
else if(found==7){
int count = strtoint(operands[0])-asmlen;
for(i=0; i<count;i++){
fputc(0,foutput);
asmlen++;
}
} else if(found==8){
fputc(strtoint(operands[0]), foutput);
asmlen++;
}
else if(found==9){
putshort(strtoint(operands[0]), foutput);
asmlen+=2;
}
else if(found==10){
putw(strtoint(operands[0]), foutput);
asmlen+=4;
} else if(found==11){
//Always an absolute jump to a register, for now
int registeri = 0;
while(!compstr(registers16[registeri], operands[0]) && registeri<8) registeri++; //find the 16 bit register
if(registeri==8) return 1; //return if invalid

//Write the instruction
fputc(0xFF, foutput);
fputc(registeri+0xE0, foutput);
asmlen+=2;

} else if(found == 12){
//Move the operand to the label array
for(i=0;operands[0][i];i++){
labels[labeli][i] = operands[0][i];
}
labeldesc[labeli] = asmlen+asmloc;
labeli++;
} else if(found == 13){
asmloc = strtoint(operands[0]); //ORG instruction
} else if(found == 14){
fputc(0xFF, foutput); //Call a register
int registeri = 0;
while(!compstr(registers16[registeri], operands[0]) && registeri<8) registeri++; //find the 16 bit register
if(registeri==8) return 1; // return if invalid

//write the instruction
fputc(0xD0 + registeri, foutput);
asmlen+=2;

} else if(found == 15){
//RET instruction
putshort(0xC366, foutput);
asmlen+=2;

} else if(found == 16){
//INC is pretty big too
int registeri = 0;
char mode = findregister(&registeri, operands[0]);
if(registeri == 8) return 1; //Return if it's an invalid register

if(!mode){
fputc(0xFE, foutput);
fputc(0xC0+registeri, foutput);
asmlen+=2;
} else{
fputc(0x40+registeri, foutput);
asmlen++;
}

} else if(found == 17){
//DEC is pretty big too
int registeri = 0;
char mode = findregister(&registeri, operands[0]);
if(registeri == 8) return 1; //Return if it's an invalid register

if(!mode){
fputc(0xFE, foutput);
fputc(0xC8+registeri, foutput);
asmlen+=2;
} else{
fputc(0x48+registeri, foutput);
asmlen++;
}

} else if(found == 18){
//Adding
int registeri = 0;
char mode = findregister(&registeri, operands[0]);
if(registeri == 8) return 1; //Return if it's an invalid register

if(!mode){
fputc(0x80, foutput);
fputc(0xC0+registeri, foutput);
fputc(strtoint(operands[1]), foutput);
asmlen+=3;
} else{
fputc(0x81, foutput);
fputc(0xC0+registeri, foutput);
putshort(strtoint(operands[1]), foutput);
asmlen+=4;
}
} else if(found == 19){
//There must be an easier way
int registeri = 0;
char mode = findregister(&registeri, operands[0]);
if(registeri == 8) return 1; //Return if it's an invalid register

if(!mode){
fputc(0x80, foutput);
fputc(0xE8+registeri, foutput);
fputc(strtoint(operands[1]), foutput);
asmlen+=3;
} else{
fputc(0x81, foutput);
fputc(0xE8+registeri, foutput);
putshort(strtoint(operands[1]), foutput);
asmlen+=4;
}
}
return 0;
}


//MAIN FUNCTION
int main(){
int i = 0;
int lncount = 1;
char instruction[100] = {0};
char filename[50] = {0};
char inbyte = 0;

//GET INPUT FILE
FILE* finputt;
printf("Input name>");
scanf("%s",filename);
finputt = fopen(filename,"r+");
if(!finputt){
printf("INPUT FILE ERROR. Maybe check if it exists?");
return 0;
}

//Get line count
for(inbyte = fgetc(finputt); inbyte != EOF; inbyte = fgetc(finputt)){
if(inbyte == '\n') lncount++;
}

//Re-open file for reading
finputt = fopen(filename,"r+");
if(!finputt){
printf("INPUT FILE ERROR. Maybe check if it exists?");
return 0;
}

//GET/MAKE OUTPUT FILE
printf("Output name>");
scanf("%s",filename);
FILE* foutputt = fopen(filename,"w+"); //Create/clear file
fclose(foutputt);
foutputt = fopen(filename,"a+"); //Open file for appending purposes
inbyte = fgetc(finputt);

for(i=0;i<lncount;i++){
int i2 = 0;

//If the line break leads to a file end, stop assembling
if(inbyte == EOF) break;

//Get line
for(inbyte; inbyte!='\r' && inbyte!='\n' && inbyte!=EOF; inbyte=fgetc(finputt)){
instruction[i2] = inbyte;
i2++;
}

//Skip forward
while(inbyte == '\r' || inbyte == '\n') inbyte = fgetc(finputt);

instruction[i2] = '\n';
instruction[i2+1] = '\0';

int errcode = assemble(instruction, foutputt);
if(errcode){
printf("ERROR ON LINE %i",i+1);
printf("\nCODE: %i", errcode);
break;
} else{
printf(instruction);
}
}
fclose(foutputt);
}
