int producer(char* port, char* fl, char* buffer,char* username, char* to, char* frameNum);
int encode(char *inData,char *len, char* fdOut_One,char* isCap,char* flag,char* username, char* to);
int decode(char** blocks);
int toUpper(char* str, char* len,char* fdIn_One);
int consumer(char* fdOut,char* fdIn,char* flag);
void addParity(char *inData, char* fdOut_One,char* isCap,char* flag,char* username, char* to);
void addHamming(char *inData, char* fdOut_One, char* isCap,char* flag,char* username, char* to);
void addCRC(char* inData, char* fdOut_One, char* isCap,char* flag,char* username, char* to);
void checkCRC(char* inData, char* fdIn_One,char* flag,char* file);
void removeHamming(char** characters);
void buildFrame(char *inData, char* fdOut_One,char* isCap,char* flag,char* username, char* to);
int deframe(char *inData, char* fdIn_One,char* flag,char* file);
int checkRemoveParity(char** characters);
int error(char *inData, char* len, char* fdOut_One,char* isCap,char* flag,char* username, char* to);
int server();
int client();

