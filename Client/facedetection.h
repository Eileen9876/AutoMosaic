#ifndef FACEDETECTION_H
#define FACEDETECTION_H

#include <fstream>
#include <memory>
#include <vector>
#include "base64.h"
#include <WS2tcpip.h>

namespace imgproc {
using std::string;
using FaceLocation = std::vector<std::vector<int>>;

enum class DataType : short {
    WORD,
    IMAGE
};

struct Result {
    DataType type;
    string error;

    string word_data;
    FaceLocation image_data;

    Result() = default;
};

struct Data {
    DataType type;
    string content;
    int size;

    Data(DataType type, string content);

    string TypeToStr();
    string Info(); //Info : type + size, ex IMAGE 1024

    void InitialWordData();
    void InitialImageData();
};

struct ClientSocket {
    SOCKET s;
    ClientSocket();
    ~ClientSocket();
    void SetTimeOut(int timeout);
};

class Client {
public:
    Client(string server_ip, short server_port);
    Result TransmitData(Data data);
private:
    friend struct ClientStaticDestructor;

    sockaddr_in server_addr_;
    void SetServerAddr(string server_ip, short server_port);
    void ServerConnect(SOCKET s);

    static bool load_lib_;
    static void LoadWinsockLib();
    static void UnLoadWinsockLib();

    static string RecvFromServer(SOCKET s, int msg_size);
    static void SendToServer(SOCKET s, string msg);

    static void MsgCheck(string msg, string check_str);
    static Result GetResult(Data data, string result);
};

struct ClientStaticDestructor {
    ~ClientStaticDestructor();
};
}

#endif // FACEDETECTION_H
