from uuid import uuid4
import GPUtil
from pdf2image import convert_from_bytes
from io import BytesIO
import numpy as np
from PIL import Image

def generate_unique_id():
    return str(uuid4())  

def convert_pdf_to_jpg():
    pass

def get_gpu_info():
    try:
        gpus = GPUtil.getGPUs()
        # print(gpus)
        gpu_info = []
        for gpu in gpus:
            gpu_dict = {
                "name": gpu.name,
                "memoryTotal": gpu.memoryTotal,
                "memoryFree": gpu.memoryFree,
                "memoryUsed": gpu.memoryUsed,
                # "GPU Memory Utilization": gpu.memoryUtil * 100,  # % sử dụng bộ nhớ
                # "GPU Utilization": gpu.gpuUtil,  # % sử dụng GPU
                "temperature": gpu.temperature,
                "driver": gpu.driver
                # "GPU UUID": gpu.uuid
            }
            gpu_info.append(gpu_dict)
        return gpu_info
    except Exception as e:
        print(e)
        return None
    


def convert_pdf_bytes_to_image_arrays(pdf_bytes):
    images = convert_from_bytes(pdf_bytes, dpi=300)
    img_array = np.array(images[0])
    return img_array


def convert_image_bytes_to_array(image_bytes):
    
    byte_stream = BytesIO(image_bytes)
    
    image = Image.open(byte_stream)
    
    img_array = np.array(image)
    
    return img_array


