import logging

class Log:
    
    def Init(FORMAT = '%(asctime)s %(levelname)s: %(message)s'):
        logging.basicConfig(level=logging.INFO, filename='data/Log.log', filemode='a', format=FORMAT)
    
    def LogInfo(message):
        logging.info(message)

    def LogError(exception):
        logging.error("Catch an exception.", exc_info=True)