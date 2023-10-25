int producer();
int encode(char *inData,char *len, char* fdOut_One,char* isCap,char* flag);
int decode(char** blocks);
int toUpper(char* str, char* len,char* fdIn_One);
int consumer(char* fdOut,char* fdIn,char* flag);
void addParity(char *inData, char* fdOut_One,char* isCap,char* flag);
void addHamming(char *inData, char* fdOut_One, char* isCap);
void addCRC(char* inData, char* fdOut_One, char* isCap);
void checkCRC(char* inData, char* fdOut_One, char* isCap);
void removeHamming(char** characters);
void buildFrame(char *inData, char* fdOut_One,char* isCap);
int deframe(char *inData, char* fdIn_One,char* flag);
int checkRemoveParity(char** characters);
int error(char *inData, char* len, char* fdOut_One,char* isCap,char* flag);
