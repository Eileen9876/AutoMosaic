import socket
import threading
from ProcData import ProcData
from Log import Log

Log.Init()

class Server:
    __Threads = []

    def __init__(self, ip, port):
        self.__IP = ip
        self.__PORT = port


    def __del__(self):
        self.__ServerSocket.close()

        for thread in self.__Threads:
            thread.join()

        Log.LogInfo(f"Server {self.__IP, self.__PORT} Close Success")


    def Create(self, num_of_conn = 5):
        self.__ServerSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.__ServerSocket.bind((self.__IP, self.__PORT))
        self.__ServerSocket.listen(num_of_conn)

        Log.LogInfo(f"Server {self.__IP, self.__PORT} Create Success")


    def Run(self):
        Log.LogInfo(f"Server {self.__IP, self.__PORT} Start Accept")

        while 1:
            conn, address =  self.__ServerSocket.accept()

            Log.LogInfo(f"Connect Client {str(address)}")

            thread = threading.Thread(target = Server.__HandleRequest, args = (conn, address))

            self.__Threads.append(thread)

            thread.start()


    def __HandleRequest(conn, address):
        Log.LogInfo(f"Dispatch Client {str(address)}")

        handler = RequestHandler(conn, address)

        handler.Start()

        del handler

class RequestHandler:

    def __init__(self, conn, address):
        self.Socket = conn

        self.Address = address


    def __del__(self):
        self.Socket.close()

        Log.LogInfo(f"Client {str(self.Address)} Close Success")


    def Start(self):
        '''
        流程如下
        (1) 傳送訊號「CONNECT」，告知客戶端已分配交接對象。
        (2) 接收資料類型與資料大小，例如 "IMAGE 1024"。
        (3) 傳送訊號「OK」，告知客戶端開始傳送資料。
        (4) 接收資料內容。
        (3) 傳送訊號「RECV_END」，告知客戶端已接收完畢。
        (5) 處理資料。
        
        (8) 傳送訊號「PROC_END」，告知客戶端已處理完畢。
        (7) 接收訊號「OK」，確認可開始傳送訊息。
        (9) 傳送處理結果的資料大小。
        (10) 接收訊號「OK」，確認可開始傳送訊息。
        (11) 傳送處理結果的資料。
        '''
        BUFFER_SIZE = 2

        try:

            self.__Send("CONNECT")

            dataType, dataSize = self.__GetDataType()

            self.__Send("OK")

            dataContent = self.__Recv(dataSize)

            self.__Send("RECV_END")

            RequestHandler.__CheckMessage(self.__Recv(BUFFER_SIZE), "OK")

            result = RequestHandler.__ProcData(dataType, dataContent)

            self.__Send("PROC_END")

            RequestHandler.__CheckMessage(self.__Recv(BUFFER_SIZE), "OK")

            self.__Send(str(len(result)))

            RequestHandler.__CheckMessage(self.__Recv(BUFFER_SIZE), "OK")

            self.__Send(result)

        except Exception as ex:

            Log.LogError(ex)


    def __Send(self, message):
        self.Socket.send(message.encode())

        Log.LogInfo(f"Send to {str(self.Address)} {message}")


    def __Recv(self, size):
        message = self.Socket.recv(size).decode()

        Log.LogInfo(f"Receive from {str(self.Address)} {message}")

        return message


    def __GetDataType(self):
        # 返回資料類型與資料大小
        message = self.__Recv(25)

        data = message.split(" ")

        return data[0], int(data[1])
    

    def __CheckMessage(message, check_string):
        if message != check_string: 

            raise ValueError('接收訊號錯誤')
    

    def __ProcData(type, data):
        handler = ProcData(type, data)

        result = handler.Run()

        return result
