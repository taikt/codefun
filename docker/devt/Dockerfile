# Stage 1: Build stage cho Node.js và React
FROM node:alpine AS node-builder
WORKDIR /app
RUN npm install -g create-react-app \
    && npx create-react-app my-app \
    && cd my-app \
    && npm install

# Stage 2: Runtime stage - Image cuối cùng
FROM alpine:latest

# Thêm repository Alpine 3.9 để cài MongoDB
RUN echo "http://dl-cdn.alpinelinux.org/alpine/v3.9/main" >> /etc/apk/repositories && \
    echo "http://dl-cdn.alpinelinux.org/alpine/v3.9/community" >> /etc/apk/repositories && \
    apk update && apk add \
    openssh-server \
    nodejs \
    npm \
    mongodb=4.0.5-r0 \
    gcc \
    g++ \
    libstdc++ \
    python3 \
    && rm -rf /var/cache/apk/* \
    && mkdir /var/run/sshd \
    && mkdir -p /data/db \
    && echo "root:at" | chpasswd \
    && ssh-keygen -A

# Sao chép Node.js và project React từ node-builder
COPY --from=node-builder /usr/local /usr/local/
#COPY --from=node-builder /app/my-app /app/my-app
COPY start.sh /start.sh
# Cấu hình SSH
RUN sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config

# Thiết lập thư mục làm việc
WORKDIR /app/my-app

# Mở các port cần thiết
EXPOSE 22 27017 3000

# Tạo script khởi động
RUN echo '#!/bin/sh' > /start.sh && \
    echo 'mongod --fork --logpath /var/log/mongod.log || { echo "Failed to start MongoDB"; exit 1; }' >> /start.sh && \
    echo '/usr/sbin/sshd -D &' >> /start.sh && \
    echo 'cd /app/my-app || { echo "Cannot change to /app/my-app directory"; exit 1; }' >> /start.sh && \
    echo 'npm start &' >> /start.sh && \
    echo 'tail -f /dev/null' >> /start.sh && \
    chmod +x /start.sh

# Chạy script khởi động
CMD ["/start.sh"]