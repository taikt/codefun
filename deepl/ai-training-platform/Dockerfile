# Use the official NVIDIA CUDA base image
FROM local-harbor.savis.vn/savis/ai-training-platform:deployment_gpu_base

# RUN rm -rf /etc/localtime

# RUN ln -snf /usr/share/zoneinfo/Europe/London /etc/localtime && echo Europe/London > /etc/timezone

# RUN apt-get -qq update && apt-get -qq install -y --no-install-recommends \
# 	protobuf-compiler \
# 	python3.10 \
# 	python3-pip \
# 	python3-dev \
# 	build-essential 

# RUN apt install ffmpeg libsm6 libxext6 poppler-utils -y

# # Upgrade pip
# RUN pip3 install --upgrade pip

ENV CUDA_VERSION=11.7.1
ENV CUDNN_VERSION=8

# RUN mkdir app

WORKDIR /app

ENV PATH="/usr/local/cuda/bin:${PATH}"
ENV LD_LIBRARY_PATH="/usr/local/cuda/lib64:${LD_LIBRARY_PATH}"
RUN echo "$PATH" && echo "$LD_LIBRARY_PATH"

COPY requirements.txt requirements.txt


RUN pip3 install -r requirements.txt
# RUN pip3 uninstall ultralytics -y
COPY . ./
RUN yolo settings --sync False

COPY ocr_core ./


CMD ["sh","run.sh"]

