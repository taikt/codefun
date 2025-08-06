import requests
import os
import json
from dotenv import load_dotenv
load_dotenv()

ADMIN_HOST = "http://10.0.20.212:9322" if not os.environ.get('ADMIN_HOST') else os.environ.get('ADMIN_HOST')


def get_train_data(form_code):
    try:
        FILE_ENDPOINT = "/CoreOCR/GetAllFileByTemplateOcr?code={}".format(form_code)
        payload = {}
        headers = {
        'accept': 'text/plain'
        }
        response = requests.request("GET", ADMIN_HOST+FILE_ENDPOINT, headers=headers, data=payload)
        # print(response)
        if response.status_code==200:
            # print(response.text)
            return json.loads(response.text)["data"]
        else:
            return None
    except:
        return None

def get_labels(form_code):
    try:
        LABEL_ENDPOINT = "/CoreOCR/GetAllFieldByTemplateOcr?code={}".format(form_code)
        payload = {}
        headers = {
        'accept': 'text/plain'
        }
        response = requests.request("GET", ADMIN_HOST+LABEL_ENDPOINT, headers=headers, data=payload)
        if response.status_code==200:
            return json.loads(response.text)["data"]
        else:
            return None
    except:
        return None
