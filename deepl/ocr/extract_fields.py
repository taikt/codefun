import pytesseract
from PIL import Image

def extract_fields(image_path):
    img = Image.open(image_path)
    # Giả sử đã cắt vùng, ví dụ: ho_ten, nam_sinh, que_quan, noi_o
    # Thực tế cần dùng OpenCV để xác định vùng, ở đây demo dùng toàn bộ ảnh
    ho_ten = pytesseract.image_to_string(img, lang='vie')
    nam_sinh = pytesseract.image_to_string(img, lang='vie')
    que_quan = pytesseract.image_to_string(img, lang='vie')
    noi_o = pytesseract.image_to_string(img, lang='vie')
    return {
        'ho_ten': ho_ten,
        'nam_sinh': nam_sinh,
        'que_quan': que_quan,
        'noi_o': noi_o
    }

if __name__ == "__main__":
    import sys
    fields = extract_fields(sys.argv[1])
    print(fields)
