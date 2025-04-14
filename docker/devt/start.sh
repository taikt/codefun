#!/bin/sh

# Khởi động MongoDB server ở chế độ background
mongod --fork --logpath /var/log/mongod.log
if [ $? -ne 0 ]; then
    echo "Failed to start MongoDB"
    exit 1
fi

# Khởi động SSH server ở chế độ background
/usr/sbin/sshd -D &
if [ $? -ne 0 ]; then
    echo "Failed to start SSH"
    exit 1
fi

# Chuyển đến thư mục React và chạy npm start ở foreground
cd /app/my-app || {
    echo "Cannot change to /app/my-app directory"
    exit 1
}
npm start &
tail -f /dev/null