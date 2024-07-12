from Server import Server
from Log import Log

if __name__ == "__main__":
    Log.Init()

    try:
        server = Server("0.0.0.0", 8000)
        
        server.Create()
        
        server.Run()

    except Exception as ex:
        Log.LogError(ex)
    
    finally:
        del server