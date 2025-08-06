import requests
import os
import json
from dotenv import load_dotenv
load_dotenv()

MINIO_HOST = "http://10.0.2.18:30594" if not os.environ.get('MINIO_HOST') else os.environ.get('MINIO_HOST')
MINIO_UPLOAD_ENDPOINT = "/api/v2/minio/upload-many-object-path" if not os.environ.get('MINIO_UPLOAD_ENDPOINT') else os.environ.get('MINIO_UPLOAD_ENDPOINT')
BUCKET_NAME = "mdm-ocr" if not os.environ.get('BUCKET_NAME') else os.environ.get('BUCKET_NAME')
MINIO_SAVE = "train_result/" if not os.environ.get('MINIO_SAVE') else os.environ.get('MINIO_SAVE')
# url = "http://10.0.2.18:30594/api/v2/minio/upload-many-object-path?bucketName=mdm-ocr"

def update_file(file_path):
    url = "{}{}?bucketName={}".format(MINIO_HOST,MINIO_UPLOAD_ENDPOINT,BUCKET_NAME)
    print(url)
    payload = {'ObjectPath': MINIO_SAVE}
    base_name = os.path.basename(file_path)
    files=[
    ('files',(base_name,open(file_path,'rb'),'application/octet-stream'))
    ]
    headers = {}

    response = requests.request("POST", url, headers=headers, data=payload, files=files)

    print(response.text)
    if response.status_code==200:
        data = json.loads(response.text)["data"][0]
        name = data["objectName"]
        return name
    return None