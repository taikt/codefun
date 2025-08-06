from pdf2image import convert_from_bytes
from io import BytesIO
import numpy as np
from PIL import Image

def convert_pdf_bytes_to_image_arrays(pdf_bytes):
    images = convert_from_bytes(pdf_bytes, dpi=300)
    img_array = np.array(images[0])

    return img_array


def convert_image_bytes_to_array(image_bytes):
    
    byte_stream = BytesIO(image_bytes)
    
    image = Image.open(byte_stream)
    
    img_array = np.array(image)
    
    return img_array




# if __name__ == '__main__':
#     # Mở file PDF và đọc dưới dạng byte
#     with open("example.pdf", "rb") as pdf_file:
#         pdf_bytes = pdf_file.read()

#     # Chuyển đổi PDF byte stream sang danh sách ảnh byte
#     image_bytes_list = convert_pdf_bytes_to_image_bytes(pdf_bytes)

#     # Lưu từng ảnh từ danh sách image byte stream để kiểm tra
#     for i, img_bytes in enumerate(image_bytes_list):
#         with open(f"page_{i + 1}.png", "wb") as img_file:
#             img_file.write(img_bytes)
