import base64
import face_recognition

class ProcData:

    def __init__(self, type, data):
        self.__Type = type
        self.__Data = data

    def Run(self):
        switcher = {
            "WORD": self.__HandleWord,
            "IMAGE": self.__HandleImage
        }

        return switcher.get(self.__Type, lambda: "Invalid Data Type")()

    def __HandleWord(self):
        return self.__Data + " hi"

    def __HandleImage(self):
        decoded_bytes = base64.b64decode(self.__Data)

        filePath = 'tmp/image/01.jpg'

        with open(filePath, 'wb') as f:
            f.write(decoded_bytes)

        image = face_recognition.load_image_file(filePath)
        
        face_locations = face_recognition.face_locations(image)

        return str(face_locations)