import cv2
from PIL import Image
import numpy as np

def preprocess_image(image_path):
    # Đọc ảnh
    img = cv2.imread(image_path)
    # Chuyển sang grayscale
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    # Tăng độ tương phản
    _, thresh = cv2.threshold(gray, 150, 255, cv2.THRESH_BINARY_INV | cv2.THRESH_OTSU)
    # Lưu ảnh đã xử lý
    processed_path = image_path.replace('.png', '_processed.png')
    cv2.imwrite(processed_path, thresh)
    return processed_path

if __name__ == "__main__":
    import sys
    processed = preprocess_image(sys.argv[1])
    print(f"Processed image saved to: {processed}")
