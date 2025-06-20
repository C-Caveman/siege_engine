// Launch the client/server threads, clean up everything when they exit.

#include "client/client.h"
#include "server/server.h"
#include "netcode/netcode.h"
#include <unistd.h>
#include <pthread.h>

extern struct serverState server;

struct world test_world = {0};
struct client playerClient;
struct client clients[MAX_CLIENTS] = {0};
uint8_t animTick = 0;
uint32_t frameNumber = 0;

struct eventBufferFlat      serverEventBuffer = {0};
struct eventBufferCircular  serverEventListenBuffer = {0};

sem_t serverListenerCountMutex;
sem_t clientListenerCountMutex;

#define IP_ADDRESS_LEN 128
char serverAddress[IP_ADDRESS_LEN] = {0};//"127.0.0.1";//"10.0.0.33";
int serverPort = 1111;
struct inbox serverInbox = {0};
struct outbox outboxToClient = {0};
char clientAddress[IP_ADDRESS_LEN] = {0};//"10.0.0.33";//"10.0.0.33";
int clientPort = 2222;
struct inbox clientInbox = {0};
struct outbox outboxToServer = {0};

// Client logic in client.c
void* clientLoop();
void* clientListener();
pthread_t clientThread;
pthread_t clientListenerThread;
// Server logic in server.c
void* serverLoop();
void* serverListener();
pthread_t serverThread;
pthread_t serverListenerThread;

int main() {
    //
    // Initialize server:
    //
    sem_init(&clientListenerCountMutex, 0, 1);
    sem_init(&serverListenerCountMutex, 0, 1);
    applyConfig((char*)"config/config.txt");
    
    // Set up the networking sockets:
    getDefaultAddress('l', serverAddress, sizeof(serverAddress)); // Use the loopback address.
    getDefaultAddress('l', clientAddress, sizeof(clientAddress));
    inboxCreate(&serverInbox, serverPort, serverAddress);
    serverInbox.sendBuffer = (char*)serverEventBuffer.buffer;
    outboxCreate(&outboxToClient, clientPort, clientAddress, 2);
    inboxCreate(&clientInbox, clientPort, clientAddress);
    outboxCreate(&outboxToServer, serverPort, serverAddress, 1);
    
    // Demo recording:
    char demoFileName[] = "demos/demo001.bin";
    FILE* demoFile = 0;
    if (recordingDemo)
        demoFile = fopen(demoFileName, "wb");
    if (recordingDemo && !demoFile) {
        printf("*** Failed to create/open %s.\n", demoFileName);
        exit(-1);
    }
    
    // Demo playback:
    if (playingDemo)
        demoFile = fopen(demoFileName, "r");
    if (playingDemo && !demoFile) {
        printf("*** Failed to open %s.\n", demoFileName);
        exit(-1);
    }
    if (playingDemo && recordingDemo) {
        printf("*** playingDemo and recordingDemo at the same time is not allowed!\n");
        exit(-1);
    }
    int demoFileSize = 0;
    if (playingDemo) {
        /* Size of file */
        fseek(demoFile, 0, SEEK_END);
        demoFileSize = ftell(demoFile);
        fseek(demoFile, 0, SEEK_SET);
    }
    int numDemoEvents = demoFileSize / sizeof(serverEventBuffer.buffer[0]);
    int numDemoEventsRead = 0;
    uint32_t nextDemoFrameTime = 0;
    
    #define DEBUG_EVENT_SIZES 0
    #define TO_SIZE_PRINT(name, ...) printf("%32s: %3ld bytes long.\n", #name, sizeof(struct d##name));
    if (DEBUG_EVENT_SIZES) {
        printf("Event packet sizes:\n");
        EVENT_LIST(TO_SIZE_PRINT)
        printf("\n");
    }
    
    
    // Begin updating the game state:
    pthread_create(&serverThread, NULL, serverLoop, 0);
    while (!server.running)// TODO add SINGLEPLAYER variable for this
        SDL_Delay(5);
    // Begin accepting inputs and rendering the screen:
    pthread_create(&clientThread, NULL, clientLoop, 0);
    playerClient.running = true;
    pthread_create(&clientListenerThread, NULL, clientListener, 0);
    // Begin listening for server serverEvents:
    pthread_create(&serverListenerThread, NULL, serverListener, 0); // a thread is born!
    
    
    if (timeScale < 0.01)
        timeScale = 1;
    //
    //;;; GAME LOOP:
    //
    while (server.running) {
        // Record demo:
        if (recordingDemo && demoFile && serverEventBuffer.count > 0) {
            fwrite(serverEventBuffer.buffer, sizeof(serverEventBuffer.buffer[0]), serverEventBuffer.count, demoFile);
        }
        // Play demo:
        if (playingDemo && demoFile) {
            playerClient.player->sprites[PLAYER_CROSSHAIR].flags |= INVISIBLE;
            while (nextDemoFrameTime < curFrameStart && numDemoEventsRead < numDemoEvents && serverEventBuffer.count < EVENT_BUFFER_SIZE-2 && !feof(demoFile)) {
                // peek at the next event's FrameStart time
                int gotAnEvent = fread(&serverEventBuffer.buffer[serverEventBuffer.count], sizeof(serverEventBuffer.buffer[0]), 1, demoFile);
                if (gotAnEvent == 1 && serverEventBuffer.buffer[serverEventBuffer.count].type == eventFrameStart) {
                    nextDemoFrameTime = serverEventBuffer.buffer[serverEventBuffer.count].data.detFrameStart.time;
                }
                serverEventBuffer.count += (gotAnEvent == 1);
                numDemoEventsRead += 1;
            }
            if (numDemoEventsRead >= numDemoEvents) {
                printf("**** END OF DEMO!!!\n");
                server.running = false;
            }
        }
        
        if (playingDemo && numDemoEventsRead >= numDemoEvents)
            break;
        
        SDL_Delay(100);
    }
    printf("Server was running for %d seconds.\n", SDL_GetTicks() / 1000);
    if (demoFile)
        fclose(demoFile);
    cleanup_graphics();
    cleanup_audio();
    //pthread_join(serverListenerThread, 0);
    pthread_join(clientThread, 0);
    pthread_join(clientListenerThread, 0);
    pthread_join(serverThread, 0);
    pthread_join(serverListenerThread, 0);
    //TODO add a listenerShutdown event to join these properly!
    //pthread_cancel(serverListenerThread);
    sem_destroy(&clientListenerCountMutex);
    sem_destroy(&serverListenerCountMutex);
    inboxDestroy(&clientInbox);
    inboxDestroy(&serverInbox);
    return 0;
}
