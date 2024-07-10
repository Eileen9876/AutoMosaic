from Server import Server
from Log import Log

if __name__ == "__main__":
    
    server = Server("0.0.0.0", 8000)
    
    server.Create()
    
    server.Run()
    
    del server