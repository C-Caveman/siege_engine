 #define MAX_BIND_NAME_LEN 64
struct namedKeyCode {
    char keyName[MAX_BIND_NAME_LEN];
    int keyCode;
};
extern struct namedKeyCode namedKeys[];
int keycodeFromBindName(char* bindName);
char* keyNameFromKeyCode(int keyCode);
