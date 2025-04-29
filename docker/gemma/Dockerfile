# Sử dụng image Python chính thức, hỗ trợ Apple Silicon
FROM python:3.11-slim

# Cài đặt các phụ thuộc hệ thống
# RUN apt-get update && apt-get install -y \
#     clang \
#     libclang-dev \
#     && rm -rf /var/lib/apt/lists/*

RUN pip install libclang

# Thiết lập thư mục làm việc
WORKDIR /app

# Sao chép requirements.txt và cài đặt thư viện Python
COPY requirements.txt .
#RUN pip install --no-cache-dir -r requirements.txt
#RUN pip install -r requirements.txt

# Sao chép mã nguồn
COPY generate_misra_data.py .
COPY fine_tune_gemma.py .
COPY check_misra.py .
COPY cpp_files/ ./cpp_files/

# Tạo thư mục output
RUN mkdir -p /app/output /app/cpp_files

# Lệnh mặc định khi chạy container
CMD ["bash", "-c", "python generate_misra_data.py && tail -f /dev/null"]
#CMD ["bash", "-c", "python generate_misra_data.py && python fine_tune_gemma.py && python check_misra.py"]
#CMD ["bash", "-c", "python generate_misra_data.py && python fine_tune_gemma.py && python check_misra.py && tail -f /dev/null"]