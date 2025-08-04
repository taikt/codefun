#!/bin/bash
# Script tự động hóa huấn luyện Tesseract cho chữ viết tay
# Yêu cầu: đã cài tesseract-ocr, tesseract-ocr-dev, tesstrain

MODEL_NAME="myhandwriting"
LANG="vie"
START_MODEL="vie"
GT_DIR="ground-truth"
OUTPUT_DIR="output"

# Tạo thư mục ground-truth nếu chưa có
mkdir -p $GT_DIR
mkdir -p $OUTPUT_DIR

echo "Đặt ảnh và file .gt.txt vào thư mục $GT_DIR trước khi chạy script này."

git clone https://github.com/tesseract-ocr/tesstrain.git || true
cd tesstrain

make training MODEL_NAME=$MODEL_NAME LANG=$LANG START_MODEL=$START_MODEL GROUND_TRUTH_DIR=../$GT_DIR OUTPUT_DIR=../$OUTPUT_DIR

cd ..
echo "Huấn luyện xong. File traineddata nằm ở $OUTPUT_DIR/$MODEL_NAME.traineddata"
echo "Copy file này vào thư mục tessdata của Tesseract để sử dụng:"
echo "sudo cp $OUTPUT_DIR/$MODEL_NAME.traineddata /usr/share/tesseract-ocr/4.00/tessdata/"
