# Sử dụng image Ubuntu làm base
FROM ubuntu:22.04

# Cài đặt các công cụ cần thiết
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    python3 \
    python3-pip \
    clang \
    clang-tidy \
    cppcheck \
    && rm -rf /var/lib/apt/lists/*

# Cài đặt các thư viện bổ sung nếu cần
RUN pip3 install --no-cache-dir cmakelint

# Tạo thư mục làm việc
WORKDIR /workspace

# Đặt lệnh mặc định khi chạy container
#CMD ["/bin/bash"]
CMD ["bash", "-c", "tail -f /dev/null"]