# 1. Build docker image
```
sudo docker build -t client_form_service .
```

# 2. Deployment
tải model AI: https://drive.google.com/file/d/1lBBBvRKPv9YotUPVeIKHvBu1zfeeK6ub/view?usp=sharing 
đẩy vào ocr_core/ocr_inference/weights/
* app: port 2010: -> -> có thể sửa lại expose trong dockerfile + compose
