#include "facedetection.h"

namespace imgproc {

//--------- declare internal function  ---------//

int ToInt(char* c);
int ToInt(string str);
string ToStr(char* c, int size);
string ToStr(char* c);
string ToStr(int i);
std::unique_ptr<char[]> ToChar(string str);

int LengthOf(std::unique_ptr<char[]>& buf);
int GetFileSize(string path);
FaceLocation ConvertToList(string str);
std::unique_ptr<char[]> ReadFile(string path);


//--------------- initial value ---------------//

ClientStaticDestructor client_static_destructor;

//--------- define ClientStaticDestructor member ---------//

ClientStaticDestructor::~ClientStaticDestructor(){
    Client::UnLoadWinsockLib();
}

//------------ define Data function ------------//

Data::Data(DataType type_, std::string content_) :type(type_), content(content_) {
    switch (this->type) {
    case DataType::WORD:
        InitialWordData();
        break;
    case DataType::IMAGE:
        InitialImageData();
        break;
    default:
        throw "struct Data type err";
        break;
    }
}

string Data::TypeToStr() {
    switch (this->type) {
    case DataType::WORD: return "WORD";
    case DataType::IMAGE: return "IMAGE";
    default: return "UNKNOWN";
    }
}

string Data::Info() {
    return TypeToStr() + " " + ToStr(size);
}

void Data::InitialWordData() {
    size = content.length();
}

void Data::InitialImageData() {
    std::unique_ptr<char[]> file_content = ReadFile(content);
    int file_size = GetFileSize(content);
    content = base64_encode(file_content.get(), file_size);
    size = content.length();
}

//--------- define ClientSocket function ---------//

ClientSocket::ClientSocket() {
    this->s = socket(PF_INET, SOCK_STREAM, 0);
    if (this->s == INVALID_SOCKET) throw "can't create socket";
    SetTimeOut(1000);
}

ClientSocket::~ClientSocket() {
    closesocket(s);
}

void ClientSocket::SetTimeOut(int timeout) {
    TIMEVAL tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    setsockopt(this->s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(struct timeval));
    setsockopt(this->s, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(struct timeval));
}

//------------- define Client member -------------//

Client::Client(string server_ip, short server_port) {
    LoadWinsockLib();
    SetServerAddr(server_ip, server_port);
}

Result Client::TransmitData(Data data) {
    try {
        ClientSocket client_socket;

        ServerConnect(client_socket.s);

        /*
        流程如下
        (1) 接收伺服器訊號「CONNECT」，確認伺服器是否已分配交接對象。
        (2) 傳送資料類型與資料大小，例如 "IMAGE 1024"。
        (3) 接收伺服器訊號「OK」，確認可以開始傳送資料。
        (4) 根據資料類型做處理後並傳送資料內容。
        (5) 接收伺服器訊號「RECV_END」，表示伺服器已接收完畢。
        (6) 傳送「OK」。
        (7) 接收伺服器訊號「PROC_END」，表示伺服器已處理完畢。
        (8) 傳送「OK」，告知伺服器可開始傳送。
        (9) 接收伺服器處理結果的資料大小。
        (10) 傳送「OK」，告知伺服器可開始傳送。
        (11) 接收伺服器處理結果的資料。
        */
        MsgCheck(RecvFromServer(client_socket.s, 7), "CONNECT");

        SendToServer(client_socket.s, data.Info());

        MsgCheck(RecvFromServer(client_socket.s, 2), "OK");

        SendToServer(client_socket.s, data.content);

        MsgCheck(RecvFromServer(client_socket.s, 8), "RECV_END");

        SendToServer(client_socket.s, "OK");

        MsgCheck(RecvFromServer(client_socket.s, 8), "PROC_END");

        SendToServer(client_socket.s, "OK");

        int server_info_size = ToInt(RecvFromServer(client_socket.s, 20));

        SendToServer(client_socket.s, "OK");

        string server_info = RecvFromServer(client_socket.s, server_info_size);

        Result result = GetResult(data, server_info);

        return result;
    }
    catch (const char* msg){
        Result result;

        result.error = ToStr((char*)msg);

        return result;
    }
}

void Client::SetServerAddr(string server_ip, short server_port) {
    std::unique_ptr<char[]> ip = ToChar(server_ip);
    this->server_addr_.sin_family = AF_INET;
    this->server_addr_.sin_port = htons(server_port);
    inet_pton(AF_INET, ip.get(), &this->server_addr_.sin_addr.S_un.S_addr);
}

void Client::ServerConnect(SOCKET s) {
    int con = connect(s, (sockaddr*)&this->server_addr_, sizeof(this->server_addr_));
    if (con != 0) throw "can't connect to server";
}

bool Client::load_lib_ = false;

void Client::LoadWinsockLib() {
    if (load_lib_) return;

    WSADATA wsdata;
    int ws = WSAStartup(MAKEWORD(2, 2), &wsdata);
    if (ws != 0) throw "無法載入Winsock動態函式庫";
}

void Client::UnLoadWinsockLib() {
    if (!load_lib_) return;

    WSACleanup();
    load_lib_ = false;
}

string Client::RecvFromServer(SOCKET s, int msg_size) {
    string ret;
    int size = msg_size + 1; // +1 reserve null-terminator '\0'
    std::unique_ptr<char[]> buffer = std::make_unique<char[]>(size);
    memset(buffer.get(), 0, size);

    int recvbytes = recv(s, buffer.get(), size, 0);
    if (recvbytes == SOCKET_ERROR) {
        if(WSAGetLastError() == WSAETIMEDOUT) throw "server response timeout";
        throw "can't receive data from server";
    }

    ret = ToStr(buffer.get());
    return ret;
}

void Client::SendToServer(SOCKET s, std::string msg) {
    std::unique_ptr<char[]> buffer = ToChar(msg);
    int sendbytes = send(s, buffer.get(), LengthOf(buffer), 0);
    if (sendbytes == SOCKET_ERROR) throw "can't send data to server";
}

void Client::MsgCheck(string msg, string check_str) {
    if (msg.length() != check_str.length() || msg != check_str) throw "receive message err";//throw "doesn't receive【" + check_str + "】message";
}

Result Client::GetResult(Data data, std::string result) {
    Result ret;

    switch (data.type) {
    case DataType::WORD:
        ret.word_data = result;
        break;
    case DataType::IMAGE:
        ret.image_data = ConvertToList(result);
        break;
    default:
        throw "struct Data type err";
        break;
    }

    return ret;
}

//----------- define internal function  -----------//

int ToInt(char* c) {
    int ret = 0;
    for (int i = 0; i < strlen(c); i++) {
        ret *= 10;
        ret += c[i] - 48;
    }
    return ret;
}

int ToInt(string str) {
    return std::stoi(str);
}

string ToStr(char* c, int size) {
    return string(c, size);
}

string ToStr(char* c) {
    return string(c);
}

string ToStr(int i) {
    return std::to_string(i);
}

std::unique_ptr<char[]> ToChar(string str) {
    int size = str.length() + 1; // +1 reserve null-terminator '\0'
    std::unique_ptr<char[]> ret = std::make_unique<char[]>(size);
    ret[size - 1] = '\0';
    strcpy_s(ret.get(), size, str.c_str());
    return ret;
}

int LengthOf(std::unique_ptr<char[]>& buf) {
    return strlen(buf.get());
}

int GetFileSize(string path) {
    std::fstream f;

    f.open(path, std::ios::in | std::ios::binary);
    if (!f.is_open()) throw "can't open file : " + path;

    // get file size
    f.seekg(0, std::ios_base::end);
    int size = f.tellg();
    f.seekg(0, std::ios_base::beg);

    f.close();

    return size;
}

FaceLocation ConvertToList(string str) {
    FaceLocation list;
    std::vector<int> subList;
    std::string num = "";
    for (int i = 0; i < str.length(); i++) {
        if (str[i] == ')') {
            subList.push_back(ToInt(num));
            list.push_back(subList);
            num = "";
            subList.clear();
        }
        else if (str[i] == ',') {
            if (num == "") continue;
            subList.push_back(ToInt(num));
            num = "";
        }
        else if (str[i] >= 48 && str[i] <= 57) {
            num += str[i];
        }
    }
    return list;
}

std::unique_ptr<char[]> ReadFile(string path) {
    std::fstream f;

    f.open(path, std::ios::in | std::ios::binary);
    if (!f.is_open()) throw "can't open file : " + path;

    // get file size
    f.seekg(0, std::ios_base::end);
    int size = f.tellg();
    f.seekg(0, std::ios_base::beg);

    std::unique_ptr<char[]> buffer = std::make_unique<char[]>(size);
    f.read(buffer.get(), size);

    f.close();

    return buffer;
}
}
