import requests
import os
import json
from dotenv import load_dotenv
load_dotenv()

MINIO_HOST = "http://10.0.20.212:9322" if not os.environ.get('MINIO_HOST') else os.environ.get('MINIO_HOST')
BUCKET_NAME = "mdm-ocr" if not os.environ.get('BUCKET_NAME') else os.environ.get('BUCKET_NAME')
MINIO_DOWNLOAD_ENDPOINT = "/api/v2/minio/download-object" if not os.environ.get('MINIO_DOWNLOAD_ENDPOINT') else os.environ.get('MINIO_DOWNLOAD_ENDPOINT')
MINIO_SAVE = "train_result/" if not os.environ.get('MINIO_SAVE') else os.environ.get('MINIO_SAVE')
# url = "http://10.0.2.18:30594/api/v2/minio/upload-many-object-path?bucketName=mdm-ocr"

def download_file(file_path, save_path):
    try:
        url = "{}{}?bucketName={}&objectName={}".format(MINIO_HOST,MINIO_DOWNLOAD_ENDPOINT,BUCKET_NAME, file_path)
        payload = {}
        files={}
        headers = {}
        base_name = os.path.basename(file_path)
        response = requests.request("GET", url, headers=headers, data=payload, files=files)

        # print(response.text)
        if response.status_code==200:
            final_path = os.path.join(save_path,base_name)
            f = open(final_path,'wb')
            f.write(response.content)
            return final_path
        return None
    except:
        return None